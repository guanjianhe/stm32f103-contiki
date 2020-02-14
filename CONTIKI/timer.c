#include "timer.h"

/*---------------------------------------------------------------------------*/
void timer_set(struct timer *t, clock_time_t interval)
{
    t->interval = interval;  /* 设置间隔时间 */
    t->start = clock_time(); /* 获取当前时间 */
}
/*---------------------------------------------------------------------------*/
void timer_reset(struct timer *t)
{
    t->start += t->interval; /* 以相同的间隔重置计时器 */
}
/*---------------------------------------------------------------------------*/
void timer_restart(struct timer *t)
{
    t->start = clock_time(); /* 获取当前时间 */
}
/*---------------------------------------------------------------------------*/
int timer_expired(struct timer *t)
{
    /* 注意这边+1了，所以下面不能写成<= */
    clock_time_t diff = (clock_time() - t->start) + 1;
    return t->interval < diff; /* 检查是否到期 */
}
/*---------------------------------------------------------------------------*/
clock_time_t timer_remaining(struct timer *t)
{
    return t->start + t->interval - clock_time(); /* 计算还有多久到期 */
}
/*---------------------------------------------------------------------------*/
