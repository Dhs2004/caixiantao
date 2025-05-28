/*
* SPDX-License-Identifier: Apache-2.0
* Copyright 2019 Western Digital Corporation or its affiliates.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http:*www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
/**
* @file   main.c
* @author Nati Rapaport
* @date   23.10.2019
* @brief  main C file of the demonstration application
*/

/**
* include files
*/
#include "board.h"
#include <rthw.h>
#include "rtthread.h"
#include "appdef.h"
/**
* definitions
*/

/**
* macros
*/

/**
* types
*/

/**
* local prototypes
*/

/**
* external prototypes
*/

/**
* global variables
*/


volatile rt_thread_t Start;
volatile int exit_flag = 0;  // 全局退出标志
void startApp();  // 前向声明
void thread_sample(void);
void timeslice_sample(void);
void timer_sample(void);
void msgq_sample(void);
void interrupt_sample(void);
void mempool_sample(void);
void mutex_sample(void);
void led_sample(void);

void printInfo(){
    rt_kprintf("=========================================================\n");
    rt_kprintf("|                   Process Selection                    |\n");
    rt_kprintf("=========================================================\n");
    rt_kprintf("| SW0-3: Select a process sample                         |\n");
    rt_kprintf("| SW15=1: Confirm selection                              |\n");
    rt_kprintf("| SW13=1: Terminate the current process sample           |\n");
    rt_kprintf("=========================================================\n");
    rt_kprintf("| 0000: thread_sample()                                  |\n");
    rt_kprintf("| 0001: timeslice_sample()                               |\n");
    rt_kprintf("| 0010: timer_sample()                                   |\n");
    rt_kprintf("| 0011: msgq_sample()                                    |\n");
    rt_kprintf("| 0100: mem_sample()                                     |\n");
    rt_kprintf("| 0101: interrupt_sample()                               |\n");
    rt_kprintf("| 0110: mutex_sample()                                   |\n");
    rt_kprintf("| 0111: led_sample()                                     |\n");
    rt_kprintf("=========================================================\n");
}


// 示例进程类型枚举
typedef enum {
    THREAD_SAMPLE,
    TIMESLICE_SAMPLE,
    TIMER_SAMPLE,
    MSGQ_SAMPLE,
    INTERRUPT_SAMPLE,
    MEM_SAMPLE,
    MUTEX_SAMPLE,
    LED_SAMPLE,
    NO_SAMPLE
} SampleType;

SampleType current_sample = NO_SAMPLE;  // 用于存储当前示例进程类型
rt_thread_t example_thread = RT_NULL;  // 用于存储示例线程句柄
rt_thread_t monitor_thread = RT_NULL;  // 用于存储监控线程句柄

void monitor_thread_entry(void* parameter) {
    while (1) {
        // 检测 sw13 是否置1
        if (((READ_GPIO(GPIO_SWs) >> 16) >> 13) & 0x1) {
            rt_kprintf("sw13 is set, suspending example thread and restarting startApp.\n");
            if (example_thread != RT_NULL) {
                rt_thread_delete(example_thread);   // 删除当前示例线程
                switch (current_sample) {           // 根据当前用例信息删除对应创建的进程
                    case THREAD_SAMPLE:
                        terminate_thread();
                        break;
                    case TIMESLICE_SAMPLE:
                        terminate_timeslice();
                        break;
                    case TIMER_SAMPLE:
                        terminate_timer();
                        break;
                    case MSGQ_SAMPLE:
                        terminate_msgq();
                        break;
                    case INTERRUPT_SAMPLE:
                        terminate_interrupt();
                        break;
                    case MEM_SAMPLE:
                        terminate_mem();
                        break;
                    case MUTEX_SAMPLE:
                        terminate_mutex();
                        break;
                    case LED_SAMPLE:
                        terminate_led();
                        break;
                    default:
                        break;
                }
                example_thread = RT_NULL;           // 重置示例线程句柄
                current_sample = NO_SAMPLE;         // 重置进程类型
            }
            // 重启 startApp 线程
            rt_thread_t Start = rt_thread_create("start",
                                startApp, RT_NULL,
                                THREAD_STACK_SIZE,
                                THREAD_PRIORITY-1, THREAD_TIMESLICE);
            if (Start != RT_NULL)
                rt_thread_startup(Start);
            return;  // 退出监控线程
        }
        rt_thread_delay(100);
    }
}

void startApp(){
    printInfo();
    while(1){
        if((READ_GPIO(GPIO_SWs) >> 16 >> 14) == 2){
            switch (READ_GPIO(GPIO_SWs) >> 16)
            {
                case 0x8000:
                    example_thread = rt_thread_create("thread_sample", (void (*)(void*))thread_sample, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
                    current_sample = THREAD_SAMPLE;
                    break;
                case 0x8001:
                    example_thread = rt_thread_create("timeslice_sample", (void (*)(void*))timeslice_sample, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
                    current_sample = TIMESLICE_SAMPLE;
                    break;
                case 0x8002:
                    example_thread = rt_thread_create("timer_sample", (void (*)(void*))timer_sample, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
                    current_sample = TIMER_SAMPLE;
                    break;
                case 0x8003:
                    example_thread = rt_thread_create("msgq_sample", (void (*)(void*))msgq_sample, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
                    current_sample = MSGQ_SAMPLE;
                    break;
                case 0x8004:
                    // rt_kprintf("8004");
                    example_thread = rt_thread_create("mempool_sample", (void (*)(void*))mempool_sample, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
                    current_sample = MEM_SAMPLE;
                    break;
                case 0x8005:
                    example_thread = rt_thread_create("interrupt_sample", (void (*)(void*))interrupt_sample, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
                    current_sample = INTERRUPT_SAMPLE;
                    break;
                case 0x8006:
                    example_thread = rt_thread_create("mutex_sample", (void (*)(void*))mutex_sample, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
                    current_sample = MUTEX_SAMPLE;
                    break;
                case 0x8007:
                    example_thread = rt_thread_create("led_sample", (void (*)(void*))led_sample, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
                    current_sample = LED_SAMPLE;
                    break;
                default:
                    break;
            }

            if (example_thread != RT_NULL) {
                rt_thread_startup(example_thread);

                // 创建监控线程
                monitor_thread = rt_thread_create("monitor",
                                        monitor_thread_entry, RT_NULL,
                                        THREAD_STACK_SIZE,
                                        THREAD_PRIORITY, THREAD_TIMESLICE);
                if (monitor_thread != RT_NULL){}
                    rt_thread_startup(monitor_thread);
                
                // 分离当前线程
                // rt_thread_detach(rt_thread_self());
                return;  // 返回以结束当前线程
            }
        }
        rt_thread_delay(100);
    }
    pspEnableInterruptNumberMachineLevel(D_PSP_INTERRUPTS_MACHINE_TIMER);
}

int main(void)
{	
  WRITE_GPIO(GPIO_LEDs,0x11110001);
  WRITE_GPIO(SegEn_ADDR, 0x10);
  WRITE_GPIO(SegDig_ADDR, 0x02111314);
  M_PSP_WRITE_REGISTER_32(SegEn_ADDR, 0x00110000);
  M_PSP_WRITE_REGISTER_32(SegDig_ADDR, 0x02111314);
/*创建线程示例*/

  Start = rt_thread_create("start",
                            startApp, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY-1, THREAD_TIMESLICE);
  /* 如果获得线程控制块，启动这个线程 */
  if (Start != RT_NULL)
    rt_thread_startup(Start);
  /*开启中断，最好创建一个线程后再中断，否则可能报错*/
  pspEnableInterruptNumberMachineLevel(D_PSP_INTERRUPTS_MACHINE_TIMER);
  return 0;
}


