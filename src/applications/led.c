#include <rtthread.h>
#include <rthw.h>
#include "board.h"
#include "appdef.h"
#include <sys/unistd.h>

#define SegEn_ADDR      0x80001038
#define SegDig_ADDR     0x8000103C

#define GPIO_SWs        0x80001400
#define GPIO_LEDs       0x80001404
#define GPIO_INOUT      0x80001408
#define RGPIO_INTE      0x8000140C
#define RGPIO_PTRIG     0x80001410
#define RGPIO_CTRL      0x80001418
#define RGPIO_INTS      0x8000141C

#define RPTC_CNTR       0x80001200
#define RPTC_HRC        0x80001204
#define RPTC_LRC        0x80001208
#define RPTC_CTRL       0x8000120c

#define Select_INT      0x80001018

#define READ_GPIO(dir) (*(volatile unsigned *)dir)
#define WRITE_GPIO(dir, value)             \
  {                                        \
    (*(volatile unsigned *)dir) = (value); \
  }


void terminate_led(void){
    
  }

/** led 线程 :Seg显示0x02111314(我的学号末位数字），同时LED灯双数位置和单数位置交替
*/
void led_sample()
{
    WRITE_GPIO(SegEn_ADDR, 0x80);
    WRITE_GPIO(SegDig_ADDR, 0x02111315);
    WRITE_GPIO(GPIO_LEDs,0x20207717);
    uid_t count = 0;
    while (1)
    {
      if (count % 2 == 0)
      {
        WRITE_GPIO(GPIO_LEDs, READ_GPIO(GPIO_SWs) >> 16);
      }
      else
      {
        WRITE_GPIO(GPIO_LEDs, 0x5555);
      }
      count ++;
    }

}


 /* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(led_sample, led sample);