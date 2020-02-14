#include "etimer.h"

static struct etimer *timerlist;
static clock_time_t next_expiration;

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

PROCESS(etimer_process, "Event timer");
/*
static char process_thread_etimer_process(struct pt *process_pt, process_event_t ev, 
                                        process_data_t data); 
struct process etimer_process = {0, "Event timer", process_thread_etimer_process}
*/
/*---------------------------------------------------------------------------*/
static void update_time(void)
{
    clock_time_t tdist;
    clock_time_t now;
    struct etimer *t;

    if (timerlist == NULL)
    {
        next_expiration = 0;
    }
    else
    {
        now = clock_time();
        t = timerlist;

        tdist = t->timer.start + t->timer.interval - now;

        for (t = t->next; t != NULL; t = t->next)
        {
            if (t->timer.start + t->timer.interval - now < tdist)
            {
                tdist = t->timer.start + t->timer.interval - now;
            }
        }

        next_expiration = now + tdist;
    }
}
/*---------------------------------------------------------------------------*/
/*
static char process_thread_etimer_process(struct pt *process_pt, process_event_t ev, 
                                          process_data_t data)
*/
PROCESS_THREAD(etimer_process, ev, data)
{
    struct etimer *t, *u;

    PROCESS_BEGIN();

    timerlist = NULL;

    while (1)
    {
        PROCESS_YIELD();

        if (ev == PROCESS_EVENT_EXITED)
        {
            struct process *p = data;

            while (timerlist != NULL && timerlist->p == p)
            {
                timerlist = timerlist->next;
            }

            if (timerlist != NULL)
            {
                t = timerlist;

                while (t->next != NULL)
                {
                    if (t->next->p == p)
                    {
                        t->next = t->next->next;
                    }
                    else
                    {
                        t = t->next;
                    }
                }
            }

            continue;
        }
        else if (ev != PROCESS_EVENT_POLL)
        {
            continue;
        }

    again:

        u = NULL;

        for (t = timerlist; t != NULL; t = t->next)
        {
            if (timer_expired(&t->timer))
            {
                if (process_post(t->p, PROCESS_EVENT_TIMER, t) == PROCESS_ERR_OK)
                {

                    /* Reset the process ID of the event timer, to signal that the
                       etimer has expired. This is later checked in the
                       etimer_expired() function. */
                    t->p = PROCESS_NONE;

                    if (u != NULL)
                    {
                        u->next = t->next;
                    }
                    else
                    {
                        timerlist = t->next;
                    }

                    t->next = NULL;
                    update_time();
                    goto again;
                }
                else
                {
                    etimer_request_poll();
                }
            }

            u = t;
        }
    }

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void etimer_request_poll(void)
{
    process_poll(&etimer_process);
}
/*---------------------------------------------------------------------------*/
static void add_timer(struct etimer *timer)
{
    struct etimer *t;

    etimer_request_poll();

    if (timer->p != PROCESS_NONE)
    {
        for (t = timerlist; t != NULL; t = t->next)
        {
            if (t == timer)
            {
                /* Timer already on list, bail out. */
                timer->p = PROCESS_CURRENT();
                update_time();
                return;
            }
        }
    }

    /* Timer not on list. */
    timer->p = PROCESS_CURRENT();
    timer->next = timerlist;
    timerlist = timer;

    update_time();
}
/*---------------------------------------------------------------------------*/
void etimer_set(struct etimer *et, clock_time_t interval)
{
    timer_set(&et->timer, interval);
    add_timer(et);
}
/*---------------------------------------------------------------------------*/
void etimer_reset_with_new_interval(struct etimer *et, clock_time_t interval)
{
    timer_reset(&et->timer);
    et->timer.interval = interval;
    add_timer(et);
}
/*---------------------------------------------------------------------------*/
void etimer_reset(struct etimer *et)
{
    timer_reset(&et->timer);
    add_timer(et);
}
/*---------------------------------------------------------------------------*/
void etimer_restart(struct etimer *et)
{
    timer_restart(&et->timer);
    add_timer(et);
}
/*---------------------------------------------------------------------------*/
void etimer_adjust(struct etimer *et, int timediff)
{
    et->timer.start += timediff;
    update_time();
}
/*---------------------------------------------------------------------------*/
int etimer_expired(struct etimer *et)
{
    return et->p == PROCESS_NONE;
}
/*---------------------------------------------------------------------------*/
clock_time_t etimer_expiration_time(struct etimer *et)
{
    return et->timer.start + et->timer.interval;
}
/*---------------------------------------------------------------------------*/
clock_time_t etimer_start_time(struct etimer *et)
{
    return et->timer.start;
}
/*---------------------------------------------------------------------------*/
int etimer_pending(void)
{
    return timerlist != NULL;
}
/*---------------------------------------------------------------------------*/
clock_time_t etimer_next_expiration_time(void)
{
    return etimer_pending() ? next_expiration : 0;
}
/*---------------------------------------------------------------------------*/
void etimer_stop(struct etimer *et)
{
    struct etimer *t;

    /* First check if et is the first event timer on the list. */
    if (et == timerlist)
    {
        timerlist = timerlist->next;
        update_time();
    }
    else
    {
        /* Else walk through the list and try to find the item before the
           et timer. */
        for (t = timerlist; t != NULL && t->next != et; t = t->next)
            ;

        if (t != NULL)
        {
            /* We've found the item before the event timer that we are about
            to remove. We point the items next pointer to the event after
             the removed item. */
            t->next = et->next;

            update_time();
        }
    }

    /* Remove the next pointer from the item to be removed. */
    et->next = NULL;
    /* Set the timer as expired */
    et->p = PROCESS_NONE;
}
/*---------------------------------------------------------------------------*/
