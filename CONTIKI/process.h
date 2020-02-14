#ifndef PROCESS_H_
#define PROCESS_H_

#include "pt.h"

#define PROCESS_CONF_NO_PROCESS_NAMES 0

#ifndef NULL
#define NULL 0
#endif /* NULL */

typedef unsigned char process_event_t;
typedef void *process_data_t;
typedef unsigned char process_num_events_t;

/* û�д��� */
#define PROCESS_ERR_OK 0
/* ������� */
#define PROCESS_ERR_FULL 1

#define PROCESS_NONE NULL

/* ����¼����� */
#define PROCESS_CONF_NUMEVENTS 32

#define PROCESS_EVENT_NONE 0x80
#define PROCESS_EVENT_INIT 0x81
#define PROCESS_EVENT_POLL 0x82
#define PROCESS_EVENT_EXIT 0x83
#define PROCESS_EVENT_SERVICE_REMOVED 0x84
#define PROCESS_EVENT_CONTINUE 0x85
#define PROCESS_EVENT_MSG 0x86
#define PROCESS_EVENT_EXITED 0x87
#define PROCESS_EVENT_TIMER 0x88
#define PROCESS_EVENT_COM 0x89
#define PROCESS_EVENT_MAX 0x8a

#define PROCESS_BROADCAST NULL

#define PROCESS_BEGIN() PT_BEGIN(process_pt)
#define PROCESS_END() PT_END(process_pt)
#define PROCESS_WAIT_EVENT() PROCESS_YIELD()
#define PROCESS_WAIT_EVENT_UNTIL(c) PROCESS_YIELD_UNTIL(c)
#define PROCESS_YIELD() PT_YIELD(process_pt)
#define PROCESS_YIELD_UNTIL(c) PT_YIELD_UNTIL(process_pt, c)
#define PROCESS_WAIT_UNTIL(c) PT_WAIT_UNTIL(process_pt, c)
#define PROCESS_WAIT_WHILE(c) PT_WAIT_WHILE(process_pt, c)
#define PROCESS_EXIT() PT_EXIT(process_pt)

#define PROCESS_THREAD(name, ev, data)                            \
    static PT_THREAD(process_thread_##name(struct pt *process_pt, \
                                           process_event_t ev,    \
                                           process_data_t data))

#define PROCESS_NAME(name) extern struct process name

#if PROCESS_CONF_NO_PROCESS_NAMES
#define PROCESS(name, strname)      \
    PROCESS_THREAD(name, ev, data); \
    struct process name = {NULL,    \
                           process_thread_##name}
#else
#define PROCESS(name, strname)            \
    PROCESS_THREAD(name, ev, data);       \
    struct process name = {NULL, strname, \
                           process_thread_##name}
#endif

struct process
{
    struct process *next;               /* ָ���¸��ڵ� */
#if PROCESS_CONF_NO_PROCESS_NAMES       /* �꿪�أ��Ƿ�Ҫ������ */
#define PROCESS_NAME_STRING(process) "" /* ��Ҫ�����������Կ��ַ������ */
#else
    const char *name; /* ������ָ�� */
#define PROCESS_NAME_STRING(process) (process)->name /* �궨�壬ָ������� */
#endif
    PT_THREAD((*thread)(struct pt *, process_event_t, process_data_t)); /* ����ָ�� */
    /* ptЭ�̱�����������ת */
    struct pt pt;
    /* state�ǽ���״̬����3��״̬��needspoll�����ȼ���ת��־λ */
    unsigned char state, needspoll;
};

void process_start(struct process *p, process_data_t data);
int process_post(struct process *p, process_event_t ev,
                 process_data_t data);
void process_post_synch(struct process *p,
                        process_event_t ev, process_data_t data);
void process_exit(struct process *p);

#define PROCESS_CURRENT() process_current
extern struct process *process_current;

process_event_t process_alloc_event(void);

/* ����Խ�����ѯ */
void process_poll(struct process *p);

void process_init(void);
int process_run(void);
int process_is_running(struct process *p);
/* ��ȡ�ȴ�������¼��� */
int process_nevents(void);

extern struct process *process_list;

#define PROCESS_LIST() process_list

#endif /* PROCESS_H_ */
