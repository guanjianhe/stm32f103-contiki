#include "stm32f10x.h"
#include "etimer.h"
#include "process.h"
#include "clock.h"
#include "autostart.h"
#include "led.h"
#include "stdio.h"

static process_event_t event_data_ready;

PROCESS(LEDRED, "ledred");
PROCESS(LEDYELLOW, "ledyellow");
PROCESS(print_hello_process, "Hello");
PROCESS(print_world_process, "world");

AUTOSTART_PROCESSES(&LEDYELLOW, &LEDRED, &print_hello_process,
                    &print_world_process);

PROCESS_THREAD(print_world_process, ev, data)
{
    PROCESS_BEGIN();

    printf("***print world process start***\r\n");

    while (1)
    {
        PROCESS_WAIT_EVENT_UNTIL(ev == event_data_ready);
        printf("world\r\n");
    }

    PROCESS_END();
}

PROCESS_THREAD(print_hello_process, ev, data)
{
    PROCESS_BEGIN();
    static struct etimer timer;

    etimer_set(&timer, CLOCK_SECOND / 10);

    printf("***print hello process start***\r\n");

    event_data_ready = process_alloc_event();

    while (1)
    {
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

        printf("Hello ");

        process_post(&print_world_process, event_data_ready, NULL);

        etimer_reset(&timer);
    }

    PROCESS_END();
}

PROCESS_THREAD(LEDRED, ev, data)
{
    PROCESS_BEGIN();
    static struct etimer timer;
    etimer_set(&timer, CLOCK_SECOND * 3);

    while (1)
    {
        PROCESS_WAIT_EVENT();
        leds_toggle(LED_RED);
        etimer_reset(&timer);
    }

    PROCESS_END();
}

PROCESS_THREAD(LEDYELLOW, ev, data)
{
    PROCESS_BEGIN();
    static struct etimer timer;
    etimer_set(&timer, CLOCK_SECOND * 1);

    while (1)
    {
        PROCESS_WAIT_EVENT();
        leds_toggle(LED_YELLOW);
        etimer_reset(&timer);
    }

    PROCESS_END();
}

extern void bsp_init(void);

int main(void)
{
    bsp_init();                           /* 外设初始化 */
    clock_init();                         /* 系统时钟初始化 */
    process_init();                       /* 进程初始化 */
    process_start(&etimer_process, NULL); /* 启动系统进程 */
    autostart_start(autostart_processes); /* 启动用户进程 */

    while (1)
    {
        do
        {
        } while (process_run() > 0);
    }
}
