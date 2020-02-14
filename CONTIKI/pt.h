#ifndef PT_H_
#define PT_H_

#if 1
typedef unsigned short lc_t;

#define LC_INIT(s) s = 0;

#define LC_RESUME(s) \
    switch (s)       \
    {                \
    case 0:

#define LC_SET(s) \
    s = __LINE__; \
    case __LINE__:

#define LC_END(s) }

#else
typedef void *lc_t;

#define LC_INIT(s) s = NULL

#define LC_RESUME(s)   \
    do                 \
    {                  \
        if (s != NULL) \
        {              \
            goto *s;   \
        }              \
    } while (0)

#define LC_SET(s) \
    do            \
    {             \
        ({ __label__ resume; resume: (s) = &&resume; });    \
    } while (0)

#define LC_END(s)

#endif

struct pt
{
    lc_t lc;
};

#define PT_WAITING 0
#define PT_YIELDED 1
#define PT_EXITED 2
#define PT_ENDED 3

#define PT_INIT(pt) LC_INIT((pt)->lc)

#define PT_THREAD(name_args) char name_args

#define PT_BEGIN(pt)            \
    {                           \
        char PT_YIELD_FLAG = 1; \
        if (PT_YIELD_FLAG)      \
        {                       \
            ;                   \
        }                       \
        LC_RESUME((pt)->lc)

#define PT_END(pt)     \
    LC_END((pt)->lc);  \
    PT_YIELD_FLAG = 0; \
    PT_INIT(pt);       \
    return PT_ENDED;   \
    }

/* condition为真时执行用户程序，否则跳出 */
#define PT_WAIT_UNTIL(pt, condition) \
    do                               \
    {                                \
        LC_SET((pt)->lc);            \
        if (!(condition))            \
        {                            \
            return PT_WAITING;       \
        }                            \
    } while (0)

/* condition为真时跳出，否则执行用户程序 */
#define PT_WAIT_WHILE(pt, cond) PT_WAIT_UNTIL((pt), !(cond))

#define PT_WAIT_THREAD(pt, thread) PT_WAIT_WHILE((pt), PT_SCHEDULE(thread))

#define PT_SPAWN(pt, child, thread)     \
    do                                  \
    {                                   \
        PT_INIT((child));               \
        PT_WAIT_THREAD((pt), (thread)); \
    } while (0)

#define PT_RESTART(pt)     \
    do                     \
    {                      \
        PT_INIT(pt);       \
        return PT_WAITING; \
    } while (0)

#define PT_EXIT(pt)       \
    do                    \
    {                     \
        PT_INIT(pt);      \
        return PT_EXITED; \
    } while (0)

#define PT_SCHEDULE(f) ((f) < PT_EXITED)

#define PT_YIELD(pt)            \
    do                          \
    {                           \
        PT_YIELD_FLAG = 0;      \
        LC_SET((pt)->lc);       \
        if (PT_YIELD_FLAG == 0) \
        {                       \
            return PT_YIELDED;  \
        }                       \
    } while (0)

#define PT_YIELD_UNTIL(pt, cond)             \
    do                                       \
    {                                        \
        PT_YIELD_FLAG = 0;                   \
        LC_SET((pt)->lc);                    \
        if ((PT_YIELD_FLAG == 0) || !(cond)) \
        {                                    \
            return PT_YIELDED;               \
        }                                    \
    } while (0)

#endif /* PT_H_ */
