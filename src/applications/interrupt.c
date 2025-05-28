/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <rthw.h>
#include <rtthread.h>
#include "board.h"
#include "appdef.h"


/* 同时访问的全局变量 */
static rt_uint32_t cnt;
static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;

void thread_entry(void *parameter)
{
    rt_uint32_t no;
    rt_uint32_t level;

    no = (rt_uint32_t) parameter;
    while (1)
    {
        /* 关闭全局中断 */
        level = rt_hw_interrupt_disable();
        cnt += no;
        /* 恢复全局中断 */
        rt_hw_interrupt_enable(level);

        rt_kprintf("protect thread[%d]'s counter is %d\n", no, cnt);
        rt_thread_mdelay(no * 10);
    }
}

/* 用户应用程序入口 */
int interrupt_sample(void)
{


    /* 创建 t1 线程 */
    tid1 = rt_thread_create("thread1", thread_entry, (void *)10,
                              THREAD_STACK_SIZE,
                              THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);


    /* 创建 t2 线程 */
    tid2 = rt_thread_create("thread2", thread_entry, (void *)20,
                              THREAD_STACK_SIZE,
                              THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);

    return 0;
}

// 终止和删除所有线程
void terminate_interrupt(void) {
    if (tid1 != RT_NULL) {
        rt_thread_delete(tid1);
        tid1 = RT_NULL;
        //rt_kprintf("Thread 1 terminated.\n");
    }

    if (tid2 != RT_NULL) {
        rt_thread_delete(tid2);
        tid2 = RT_NULL;
        //rt_kprintf("Thread 2 terminated.\n");
    }
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(interrupt_sample, interrupt sample);


