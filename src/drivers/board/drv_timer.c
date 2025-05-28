/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-22     Jesven       first version
 */

#include <rthw.h>
#include <rtthread.h>
#include <stdint.h>

#include "board.h"
#include "bsp_timer.h"
#include "bsp_external_interrupts.h"
#include "interrupt.h"
#include "components.h"
#include "rtdef.h"
#include "drv_timer.h"
#include "psp_interrupts_eh1.h"
#include "psp_api.h"

#define TIMER01_HW_BASE                 0x10011000
#define TIMER23_HW_BASE                 0x10012000

#define TIMER_LOAD(hw_base)             __REG32(hw_base + 0x00)
#define TIMER_VALUE(hw_base)            __REG32(hw_base + 0x04)
#define TIMER_CTRL(hw_base)             __REG32(hw_base + 0x08)
#define TIMER_CTRL_ONESHOT              (1 << 0)
#define TIMER_CTRL_32BIT                (1 << 1)
#define TIMER_CTRL_DIV1                 (0 << 2)
#define TIMER_CTRL_DIV16                (1 << 2)
#define TIMER_CTRL_DIV256               (2 << 2)
#define TIMER_CTRL_IE                   (1 << 5)        /* Interrupt Enable (versatile only) */
#define TIMER_CTRL_PERIODIC             (1 << 6)
#define TIMER_CTRL_ENABLE               (1 << 7)

#define TIMER_INTCLR(hw_base)           __REG32(hw_base + 0x0c)
#define TIMER_RIS(hw_base)              __REG32(hw_base + 0x10)
#define TIMER_MIS(hw_base)              __REG32(hw_base + 0x14)
#define TIMER_BGLOAD(hw_base)           __REG32(hw_base + 0x18)

#define TIMER_LOAD(hw_base)             __REG32(hw_base + 0x00)
#define TIMER_VALUE(hw_base)            __REG32(hw_base + 0x04)
#define TIMER_CTRL(hw_base)             __REG32(hw_base + 0x08)
#define TIMER_CTRL_ONESHOT              (1 << 0)
#define TIMER_CTRL_32BIT                (1 << 1)
#define TIMER_CTRL_DIV1                 (0 << 2)
#define TIMER_CTRL_DIV16                (1 << 2)
#define TIMER_CTRL_DIV256               (2 << 2)
#define TIMER_CTRL_IE                   (1 << 5)        /* Interrupt Enable (versatile only) */
#define TIMER_CTRL_PERIODIC             (1 << 6)
#define TIMER_CTRL_ENABLE               (1 << 7)

#define TIMER_INTCLR(hw_base)           __REG32(hw_base + 0x0c)
#define TIMER_RIS(hw_base)              __REG32(hw_base + 0x10)
#define TIMER_MIS(hw_base)              __REG32(hw_base + 0x14)
#define TIMER_BGLOAD(hw_base)           __REG32(hw_base + 0x18)

#define SYS_CTRL                        __REG32(REALVIEW_SCTL_BASE)
#define TIMER_HW_BASE                   REALVIEW_TIMER2_3_BASE

void rt_hw_timer_isr(void)
{
    RT_DEBUG_LOG(RT_DEBUG_IRQ, ("irq is going to enter"));
    /* enter interrupt */
    //rt_interrupt_enter();
    pspDisableInterruptNumberMachineLevel(D_PSP_INTERRUPTS_MACHINE_TIMER);

    rt_tick_increase();

    pspEnableInterruptNumberMachineLevel(D_PSP_INTERRUPTS_MACHINE_TIMER);

    /* Activates Core's timer with the calculated period */
    //pspTimerCounterSetupAndRun(E_MACHINE_TIMER, 500000);
    
    /* leave interrupt */
    //rt_interrupt_leave();
}

void enterInt(){
    rt_kprintf("aaaa");
}
//extern int rtosalHandleEcall();

int rt_hw_timer_init(void)
{
    rt_kprintf("creating timer");

    // bspInitializeGenerationRegister(0);
    // bspRoutTimer(E_TIMER_TO_IRQ3);
    // bspSetTimerDurationMsec(1);
    // bspStartTimer();
    
    pspInterruptsSetVectorTableAddress(&psp_vect_table);//D_PSP_MEIVT_NUM);

    pspRegisterInterruptHandler((pspInterruptHandler_t)rt_hw_timer_isr,E_MACHINE_TIMER_CAUSE);

    //pspInterruptsEnable();

    //pspEnableInterruptNumberMachineLevel(D_PSP_INTERRUPTS_MACHINE_TIMER);
    
    return 0;
}
//INIT_BOARD_EXPORT(rt_hw_timer_init);
//volatile const init_fn_t *fn_ptr = &__rt_init_rt_hw_timer_init;
// void timer_init(int timer, unsigned int preload)
// {
//     uint32_t val;

//     if (timer == 0) 
//     {
//         /* Setup Timer0 for generating irq */
//         val = TIMER_CTRL(TIMER01_HW_BASE);
//         val &= ~TIMER_CTRL_ENABLE;
//         val |= (TIMER_CTRL_32BIT | TIMER_CTRL_PERIODIC | TIMER_CTRL_IE);
//         TIMER_CTRL(TIMER01_HW_BASE) = val;

//         TIMER_LOAD(TIMER01_HW_BASE) = preload;

//         /* enable timer */
//         TIMER_CTRL(TIMER01_HW_BASE) |= TIMER_CTRL_ENABLE;
//     } 
//     else 
//     {
//         /* Setup Timer1 for generating irq */
//         val = TIMER_CTRL(TIMER23_HW_BASE);
//         val &= ~TIMER_CTRL_ENABLE;
//         val |= (TIMER_CTRL_32BIT | TIMER_CTRL_PERIODIC | TIMER_CTRL_IE);
//         TIMER_CTRL(TIMER23_HW_BASE) = val;

//         TIMER_LOAD(TIMER23_HW_BASE) = preload;

//         /* enable timer */
//         TIMER_CTRL(TIMER23_HW_BASE) |= TIMER_CTRL_ENABLE;
//     }
// }

void timer_clear_pending(int timer)
{
    if (timer == 0)
    {
        TIMER_INTCLR(TIMER01_HW_BASE) = 0x01;
    } 
    else
    {
        TIMER_INTCLR(TIMER23_HW_BASE) = 0x01;
    }
}
