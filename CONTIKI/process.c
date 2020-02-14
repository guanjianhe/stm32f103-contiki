#include "process.h"

struct process *process_list = NULL;
struct process *process_current = NULL;

static process_event_t lastevent;

struct event_data
{
    process_event_t ev;  /* typedef unsigned char process_event_t; */
    process_data_t data; /* typedef void *process_data_t; */
    struct process *p;
};

static process_num_events_t nevents, fevent;
static struct event_data events[PROCESS_CONF_NUMEVENTS];

static volatile unsigned char poll_requested;

#define PROCESS_STATE_NONE 0    /* 进程已退出，但还没从进程链表删除 */
#define PROCESS_STATE_RUNNING 1 /* 进程需要执行，现在处于阻塞状态 */
#define PROCESS_STATE_CALLED 2  /* 进程被调用 */

static void call_process(struct process *p, process_event_t ev,
                         process_data_t data);

#define DEBUG 0

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
process_event_t process_alloc_event(void)
{
    return lastevent++;
}
/*---------------------------------------------------------------------------*/
void process_start(struct process *p, process_data_t data)
{
    struct process *q;

    /* 确保线程没有重复添加 */
    for (q = process_list; q != p && q != NULL; q = q->next)
        ;

    /* 如果要添加进来的线程已经在列表中，则退出 */
    if (q == p)
    {
        return;
    }

    /* 
     * 把线程添加到列表中 
     * process_list是个指针，它指向第一个进程  
     */
    p->next = process_list;
    process_list = p;
    /* 初始化状态，有3种状态 */
    p->state = PROCESS_STATE_RUNNING;

    PT_INIT(&p->pt); /* 初始化协程 */
    /* 调试信息 */
    PRINTF("process: starting '%s'\r\n", PROCESS_NAME_STRING(p));

    /* 将初始化事件同步到线程 */
    process_post_synch(p, PROCESS_EVENT_INIT, data);
}
/*---------------------------------------------------------------------------*/
static void exit_process(struct process *p, struct process *fromprocess)
{
    register struct process *q;
    struct process *old_current = process_current;

    PRINTF("process: exit_process '%s'\r\n", PROCESS_NAME_STRING(p));

    /* 确保要退出的线程再链表内 */
    for (q = process_list; q != p && q != NULL; q = q->next)
        ;

    /* 如果不在链表内，则退出 */
    if (q == NULL)
    {
        return;
    }

    if (process_is_running(p))
    {
        /* 线程不处于PROCESS_STATE_NONE状态 */
        p->state = PROCESS_STATE_NONE;

        /* 向所有进程发布同步事件，以通知他们该进程即将退出。*/
        for (q = process_list; q != NULL; q = q->next)
        {
            if (p != q)
            {
                call_process(q, PROCESS_EVENT_EXITED, (process_data_t)p);
            }
        }

        /* p != fromprocess加这个条件避免重复运行，只能在别的进程中被退出才会再运行一遍 */
        if (p->thread != NULL && p != fromprocess)
        {
            /* 将退出事件发布到将要退出的进程。*/
            process_current = p;
            /* 再执行一遍函数 */
            p->thread(&p->pt, PROCESS_EVENT_EXIT, NULL);
        }
    }

    /* 要退出的进程再链表头，则更新下链表，指向下个节点 */
    if (p == process_list)
    {
        process_list = process_list->next;
    }
    else
    {
        /* 不在链表头 */
        for (q = process_list; q != NULL; q = q->next)
        {
            if (q->next == p)
            {
                q->next = p->next;
                break;
            }
        }
    }

    process_current = old_current;
}
/*---------------------------------------------------------------------------*/
static void call_process(struct process *p, process_event_t ev,
                         process_data_t data)
{
    int ret;

#if DEBUG

    if (p->state == PROCESS_STATE_CALLED)
    {
        printf("process: process '%s' called again with event %02X\r\n",
               PROCESS_NAME_STRING(p), ev);
    }

#endif /* DEBUG */

    /* 判断状态是否是PROCESS_STATE_RUNNING并且函数指针不为空 */
    if ((p->state & PROCESS_STATE_RUNNING) &&
        p->thread != NULL)
    {
        PRINTF("process: calling process '%s' with event %02X\r\n", PROCESS_NAME_STRING(p),
               ev);
        /* 当前运行的指针赋值 */
        process_current = p;
        /* 状态赋值为运行态 */
        p->state = PROCESS_STATE_CALLED;
        /* 执行函数 */
        ret = p->thread(&p->pt, ev, data);
        /* 返回值是否退出以及事件是退出事件 */
        if (ret == PT_EXITED ||
            ret == PT_ENDED ||
            ev == PROCESS_EVENT_EXIT)
        {
            exit_process(p, p);
        }
        else
        {
            /* 重新赋值状态为PROCESS_STATE_RUNNING */
            p->state = PROCESS_STATE_RUNNING;
        }
    }
}
/*---------------------------------------------------------------------------*/
void process_exit(struct process *p)
{
    exit_process(p, PROCESS_CURRENT());
}
/*---------------------------------------------------------------------------*/
void process_init(void)
{
    lastevent = PROCESS_EVENT_MAX;
    nevents = fevent = 0;
    process_current = process_list = NULL;
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static void do_poll(void)
{
    struct process *p;

    poll_requested = 0;

    /* Call the processes that needs to be polled. */
    for (p = process_list; p != NULL; p = p->next)
    {
        if (p->needspoll)
        {
            p->state = PROCESS_STATE_RUNNING;
            p->needspoll = 0;
            call_process(p, PROCESS_EVENT_POLL, NULL);
        }
    }
}
/*---------------------------------------------------------------------------*/
static void do_event(void)
{
    process_event_t ev;
    process_data_t data;
    struct process *receiver;
    struct process *p;

    /*
     * If there are any events in the queue, take the first one and walk
     * through the list of processes to see if the event should be
     * delivered to any of them. If so, we call the event handler
     * function for the process. We only process one event at a time and
     * call the poll handlers inbetween.
     */

    if (nevents > 0)
    {

        /* There are events that we should deliver. */
        ev = events[fevent].ev;

        data = events[fevent].data;
        receiver = events[fevent].p;

        /* Since we have seen the new event, we move pointer upwards
           and decrease the number of events. */
        fevent = (fevent + 1) % PROCESS_CONF_NUMEVENTS;
        --nevents;

        /* If this is a broadcast event, we deliver it to all events, in
           order of their priority. */
        if (receiver == PROCESS_BROADCAST)
        {
            for (p = process_list; p != NULL; p = p->next)
            {

                /* If we have been requested to poll a process, we do this in
                   between processing the broadcast event. */
                if (poll_requested)
                {
                    do_poll();
                }

                call_process(p, ev, data);
            }
        }
        else
        {
            /* This is not a broadcast event, so we deliver it to the
            specified process. */
            /* If the event was an INIT event, we should also update the
            state of the process. */
            if (ev == PROCESS_EVENT_INIT)
            {
                receiver->state = PROCESS_STATE_RUNNING;
            }

            /* Make sure that the process actually is running. */
            call_process(receiver, ev, data);
        }
    }
}
/*---------------------------------------------------------------------------*/
int process_run(void)
{
    /* Process poll events. */
    if (poll_requested)
    {
        do_poll();
    }

    /* Process one event from the queue */
    do_event();

    return nevents + poll_requested;
}
/*---------------------------------------------------------------------------*/
int process_nevents(void)
{
    return nevents + poll_requested;
}
/*---------------------------------------------------------------------------*/
int process_post(struct process *p, process_event_t ev, process_data_t data)
{
    /* 临时变量，用来标识当前要传递的事件放在哪里 */
    process_num_events_t snum;

    /* 当前没有其他线程在运行，换句话说当前传递者不是线程 */
    if (PROCESS_CURRENT() == NULL)
    {
        PRINTF("process_post: NULL process posts event %d to process '%s', nevents %d\r\n",
               ev, PROCESS_NAME_STRING(p), nevents);
    }
    else
    {
        PRINTF("process_post: Process '%s' posts event %02X to process '%s', nevents %d\r\n",
               PROCESS_NAME_STRING(PROCESS_CURRENT()), ev,
               p == PROCESS_BROADCAST ? "<broadcast>" : PROCESS_NAME_STRING(p), nevents);
    }

    /* 还未处理的线程总数已经满了（放不进来），退出 */
    if (nevents == PROCESS_CONF_NUMEVENTS)
    {
#if DEBUG

        if (p == PROCESS_BROADCAST)
        {
            printf("soft panic: event queue is full when broadcast event %02X was posted from %s\r\n",
                   ev, PROCESS_NAME_STRING(process_current));
        }
        else
        {
            printf("soft panic: event queue is full when event %02X was posted to %s from %s\r\n",
                   ev, PROCESS_NAME_STRING(p), PROCESS_NAME_STRING(process_current));
        }

#endif /* DEBUG */
        return PROCESS_ERR_FULL;
    }

    /* 计算下标，fevent是将要处理的事件下标，nevets是总为处理下标
	PROCESS_CONF_NUMEVENTS 是总的能存放为处理的事件总数 */
    snum = (process_num_events_t)(fevent + nevents) % PROCESS_CONF_NUMEVENTS;
    events[snum].ev = ev;
    events[snum].data = data;
    events[snum].p = p;
    /* 总数+1 */
    ++nevents;

    return PROCESS_ERR_OK;
}
/*---------------------------------------------------------------------------*/
void process_post_synch(struct process *p, process_event_t ev,
                        process_data_t data)
{
    /* 定义一个指针变量，用来存储当前线程的指针 */
    struct process *caller = process_current;
    /* 调用执行线程 */
    call_process(p, ev, data);
    /* 再赋值回来 */
    process_current = caller;
}
/*---------------------------------------------------------------------------*/
void process_poll(struct process *p)
{
    if (p != NULL)
    {
        if (p->state == PROCESS_STATE_RUNNING ||
            p->state == PROCESS_STATE_CALLED)
        {
            p->needspoll = 1;
            poll_requested = 1;
        }
    }
}
/*---------------------------------------------------------------------------*/
int process_is_running(struct process *p)
{
    return p->state != PROCESS_STATE_NONE;
}
/*---------------------------------------------------------------------------*/
