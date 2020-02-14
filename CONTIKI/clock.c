#include "clock.h"
#include "etimer.h"

#include "stm32f10x.h"

#define DEBUG 0
#if DEBUG
    #include <stdio.h>
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif

static volatile clock_time_t current_clock = 0;
static volatile unsigned long current_seconds = 0;
static unsigned int second_countdown = CLOCK_SECOND;

void clock_init(void)
{
    if (SysTick_Config(SystemCoreClock / CLOCK_SECOND))
    {
        while (1);
    }
}

clock_time_t clock_time(void)
{
    return current_clock;
}

void SysTick_Handler(void)
{
    current_clock++;

    if (etimer_pending() && etimer_next_expiration_time() <= current_clock)
    {
        etimer_request_poll();
    }

    if (--second_countdown == 0)
    {
        current_seconds++;
        second_countdown = CLOCK_SECOND;
    }
}

unsigned long clock_seconds(void)
{
    return current_seconds;
}
