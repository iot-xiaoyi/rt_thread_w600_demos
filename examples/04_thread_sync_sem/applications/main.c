/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-02-13     tyx          first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>

static rt_thread_t led_tid = RT_NULL;

/********************** led thread **********************/
/* using BLUE LED in RGB */
#define LED_PIN     (23)

static void led_thread_entry(void *arg)
{
    unsigned int count = 1;
    /* set LED pin mode to output */
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    while (count > 0)
    {
        /* led on */
        rt_pin_write(LED_PIN, PIN_LOW);
        rt_kprintf("led on, count: %d\n", count);
        rt_thread_mdelay(500);

        /* led off */
        rt_pin_write(LED_PIN, PIN_HIGH);
        rt_kprintf("led off\n");
        rt_thread_mdelay(500);

        count++;
        if (10 == count)
        {
            break;
        }
    }
}

/*
    生产者消费者例程：
    ①生产者线程：获取到空位后，产生一个数字，循环放入数组中，然后释放一个满位。
    ②消费者线程：获取到满位后，读取数组内容并相加，然后释放一个空位。
 */
#define THREAD_PRIORITY         10
#define THREAD_TIMESLICE        5

#define MAXSEM 5
// 放置生产者
rt_uint32_t array[MAXSEM];

// 指向生产者、消费者在array数组中读写位置
static rt_uint32_t set = 0, get = 0;

static rt_thread_t producer_tid = RT_NULL;
static rt_thread_t consumer_tid = RT_NULL;

static rt_sem_t sem_lock;
static rt_sem_t sem_empty, sem_full;

/********************** producter thread **********************/
void producer_thread_entry(void *args)
{
    int cnt = 0;

    while (cnt < 10)
    {
        //获取一个空位置
        rt_sem_take(sem_empty, RT_WAITING_FOREVER);

        //修改array内容，上锁
        rt_sem_take(sem_lock, RT_WAITING_FOREVER);
        array[set % MAXSEM] = cnt + 1;
        rt_kprintf("the producer generates a number: %d\n", array[set % MAXSEM]);
        set++;
        rt_sem_release(sem_lock);

        //发布一个满位
        rt_sem_release(sem_full);
        cnt++;

        rt_thread_mdelay(20);
    }
    
    rt_kprintf("the producter exit!\r\n");
}

/********************** consumer thread **********************/
void consumer_thread_entry(void *args)
{
    rt_uint32_t sum = 0;

    while (1)
    {
        //获取一个满位
        rt_sem_take(sem_full, RT_WAITING_FOREVER);
        // 上锁操作数据
        rt_sem_take(sem_lock, RT_WAITING_FOREVER);
        sum += array[get % MAXSEM];
        rt_kprintf("the consumer[%d] get a number: %d\n", (get % MAXSEM), array[get % MAXSEM]);
        get++;
        rt_sem_release(sem_lock);

        // 释放一个空位置
        rt_sem_release(sem_empty);

        // 生产者生产到10个数码，停止，消费者也要停止
        if (10 == get)
        {
            break;
        }

        rt_thread_mdelay(50);
    }
    
    rt_kprintf("the consumer sum is:%d\r\n", sum);
    rt_kprintf("the consumer exit!\r\n");
}

int main(void)
{
    // sem init
    sem_empty = rt_sem_create("empty", MAXSEM, RT_IPC_FLAG_FIFO);
    if (RT_NULL != sem_empty)
    {
        rt_kprintf("create done. ynamic semaphore value = 5.\r\n");
    }
    sem_full = rt_sem_create("full", 0, RT_IPC_FLAG_FIFO);
    sem_lock = rt_sem_create("lock", 1, RT_IPC_FLAG_FIFO);

    // create led_thread
    led_tid = rt_thread_create("led_tid", led_thread_entry, RT_NULL, 1024, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (RT_NULL != led_tid)
    {
        rt_thread_startup(led_tid);
    }else
    {
        rt_kprintf("create dynamic led thread error!!!\r\n");
    }
    

    // create producer_thread
    producer_tid = rt_thread_create("producer_tid", producer_thread_entry, RT_NULL, 1024, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (RT_NULL != producer_tid)
    {
        rt_thread_startup(producer_tid);   
    }else
    {
        rt_kprintf("create dynamic producer thread error!!!\r\n");
    }
    

    // create consumer_thread
    consumer_tid = rt_thread_create("consumer_tid", consumer_thread_entry, RT_NULL, 1024, THREAD_PRIORITY-1, THREAD_TIMESLICE);
    if (RT_NULL != consumer_tid)
    {
        rt_thread_startup(consumer_tid);
    }else
    {
        rt_kprintf("create dynamic led thread error!!!\r\n");
    }
    

    return 0;
}
