#include <rtthread.h>  
#include <rthw.h>  
#include "board.h"  
#include "appdef.h"  
#include <sys/unistd.h>  
  
#define SegEn_ADDR      0x80001038  
#define SegDig_ADDR     0x8000103C  
#define GPIO_SWs        0x80001400  
#define GPIO_LEDs       0x80001404  
  
#define READ_GPIO(dir) (*(volatile unsigned *)dir)  
#define WRITE_GPIO(dir, value)             \  
  {                                        \  
    (*(volatile unsigned *)dir) = (value); \  
  }  
  
// 录制相关数据结构  
#define MAX_RECORD_STEPS 50  
typedef struct {  
    rt_uint8_t note;        // 音符  
    rt_uint8_t tempo;       // 节拍  
    rt_uint32_t duration;   // 持续时间(ms)  
} record_step_t;  
  
static record_step_t recorded_sequence[MAX_RECORD_STEPS];  
static rt_uint8_t record_count = 0;  
static rt_uint8_t is_recording = 0;  
static rt_uint8_t is_playing_record = 0;  
static rt_uint32_t record_start_time = 0;  
static rt_uint32_t last_record_time = 0;  
  
void terminate_led(void){  
    // 清理录制数据  
    record_count = 0;  
    is_recording = 0;  
    is_playing_record = 0;  
}  
  
void led_sample()  
{  
    WRITE_GPIO(SegEn_ADDR, 0xFF);  // 启用所有数码管  
      
    static rt_uint8_t current_note = 0;  // 当前音符 (0-7对应Do-Si)  
    static rt_uint8_t tempo = 4;         // 节拍速度  
    static rt_uint8_t playing = 0;       // 播放状态  
    static rt_uint32_t last_switch_state = 0;  
    static rt_uint32_t note_counter = 0;  
    static rt_uint8_t playback_index = 0;  
    static rt_uint32_t playback_start_time = 0;  
      
    // 音符频率表  
    const rt_uint16_t notes[] = {262, 294, 330, 349, 392, 440, 494, 523};  
      
    while (1)  
    {  
        rt_uint32_t switch_state = READ_GPIO(GPIO_SWs) >> 16;  
        rt_uint32_t current_time = rt_tick_get() * 1000 / RT_TICK_PER_SECOND;  
          
        // 检测开关状态变化  
        if (switch_state != last_switch_state) {  
            rt_uint32_t changed_bits = switch_state ^ last_switch_state;  
              
            // SW0-2: 选择音符 (3位二进制，0-7)  
            if (changed_bits & 0x0007) {  
                rt_uint8_t new_note = switch_state & 0x0007;  
                  
                // 如果正在录制，记录音符变化  
                if (is_recording && record_count < MAX_RECORD_STEPS) {  
                    rt_uint32_t duration = 0;  
                    if (record_count > 0) {  
                        duration = current_time - last_record_time;  
                    }  
                      
                    recorded_sequence[record_count].note = new_note;  
                    recorded_sequence[record_count].tempo = tempo;  
                    recorded_sequence[record_count].duration = duration;  
                    record_count++;  
                    last_record_time = current_time;  
                      
                    // 录制状态LED指示  
                    WRITE_GPIO(GPIO_LEDs, 0xF000 | (1 << new_note));  
                } else {  
                    current_note = new_note;  
                    // 在数码管显示当前音符  
                    WRITE_GPIO(SegDig_ADDR, 0x02110000 | (current_note << 4) | current_note);  
                    // LED显示音符位置  
                    WRITE_GPIO(GPIO_LEDs, 1 << current_note);  
                }  
            }  
              
            // SW3: 播放/暂停  
            if (changed_bits & 0x0008) {  
                if (switch_state & 0x0008) {  
                    if (!is_recording) {  
                        playing = !playing;  
                        if (playing) {  
                            WRITE_GPIO(GPIO_LEDs, 0xFFFF);  // 全亮表示播放  
                        } else {  
                            WRITE_GPIO(GPIO_LEDs, 0x0000);  // 全灭表示暂停  
                        }  
                    }  
                }  
            }  
              
            // SW4-6: 调节节拍速度 (3位二进制，1-8)  
            if (changed_bits & 0x0070) {  
                rt_uint8_t new_tempo = ((switch_state >> 4) & 0x0007) + 1;  
                  
                // 如果正在录制，记录节拍变化  
                if (is_recording && record_count < MAX_RECORD_STEPS) {  
                    rt_uint32_t duration = current_time - last_record_time;  
                    recorded_sequence[record_count].note = current_note;  
                    recorded_sequence[record_count].tempo = new_tempo;  
                    recorded_sequence[record_count].duration = duration;  
                    record_count++;  
                    last_record_time = current_time;  
                } else {  
                    tempo = new_tempo;  
                    // 在数码管显示节拍速度  
                    WRITE_GPIO(SegDig_ADDR, 0x02111300 | tempo);  
                }  
            }  
              
            // SW7: 录制模式控制  
            if (changed_bits & 0x0080) {  
                if (switch_state & 0x0080) {  
                    if (!is_recording && !is_playing_record) {  
                        // 开始录制  
                        is_recording = 1;  
                        record_count = 0;  
                        record_start_time = current_time;  
                        last_record_time = current_time;  
                          
                        // 录制开始LED提示  
                        for (int i = 0; i < 3; i++) {  
                            WRITE_GPIO(GPIO_LEDs, 0xFF00);  
                            rt_thread_delay(100);  
                            WRITE_GPIO(GPIO_LEDs, 0x00FF);  
                            rt_thread_delay(100);  
                        }  
                        WRITE_GPIO(SegDig_ADDR, 0x02111300 | 0x0E); // 显示E表示录制  
                          
                    } else if (is_recording) {  
                        // 停止录制  
                        is_recording = 0;  
                          
                        // 录制完成LED提示  
                        for (int i = 0; i < 5; i++) {  
                            WRITE_GPIO(GPIO_LEDs, 0xAAAA);  
                            rt_thread_delay(100);  
                            WRITE_GPIO(GPIO_LEDs, 0x5555);  
                            rt_thread_delay(100);  
                        }  
                        WRITE_GPIO(SegDig_ADDR, 0x02111300 | record_count); // 显示录制步数  
                    }  
                }  
            }  
              
            // SW8: 播放录制内容 (如果有SW8的话)  
            if (changed_bits & 0x0100) {  
                if (switch_state & 0x0100) {  
                    if (record_count > 0 && !is_recording) {  
                        is_playing_record = !is_playing_record;  
                        if (is_playing_record) {  
                            playback_index = 0;  
                            playback_start_time = current_time;  
                            WRITE_GPIO(GPIO_LEDs, 0x0F0F); // 播放录制内容指示  
                        } else {  
                            WRITE_GPIO(GPIO_LEDs, 0x0000);  
                        }  
                    }  
                }  
            }  
              
            last_switch_state = switch_state;  
        }  
          
        // 播放录制内容逻辑  
        if (is_playing_record && record_count > 0) {  
            if (playback_index < record_count) {  
                record_step_t *step = &recorded_sequence[playback_index];  
                rt_uint32_t elapsed = current_time - playback_start_time;  
                  
                if (elapsed >= step->duration || playback_index == 0) {  
                    // 播放当前步骤  
                    current_note = step->note;  
                    tempo = step->tempo;  
                      
                    // 显示当前播放的音符和节拍  
                    WRITE_GPIO(SegDig_ADDR, 0x02110000 | (step->note << 4) | step->tempo);  
                    WRITE_GPIO(GPIO_LEDs, (1 << step->note) | (step->tempo << 8));  
                      
                    playback_index++;  
                    playback_start_time = current_time;  
                }  
            } else {  
                // 录制内容播放完毕，循环播放  
                playback_index = 0;  
                playback_start_time = current_time;  
            }  
        }  
        // 正常播放逻辑  
        else if (playing && !is_recording) {  
            note_counter++;  
              
            // 根据节拍速度控制音符持续时间  
            if (note_counter >= (500 / tempo)) {  
                // 模拟音符播放 - 通过LED模式变化表示  
                rt_uint16_t freq = notes[current_note];  
                rt_uint32_t led_pattern = freq % 0xFFFF;  
                WRITE_GPIO(GPIO_LEDs, led_pattern);  
                  
                // 在数码管显示当前播放的音符和频率  
                WRITE_GPIO(SegDig_ADDR, 0x02110000 | (freq & 0xFFFF));  
                  
                note_counter = 0;  
                  
                // 自动切换到下一个音符（简单的音阶播放）  
                current_note = (current_note + 1) % 8;  
            }  
        }  
          
        rt_thread_delay(10);  // 主循环延时  
    }  
}