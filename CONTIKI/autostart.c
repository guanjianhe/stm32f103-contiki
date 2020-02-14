#include "autostart.h"

#define DEBUG 0

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
void autostart_start(struct process *const processes[])
{
    int i;

    for (i = 0; processes[i] != NULL; ++i)
    {
        process_start(processes[i], NULL);
        PRINTF("autostart_start: starting process '%s'\r\n", processes[i]->name);
    }
}
/*---------------------------------------------------------------------------*/
void autostart_exit(struct process *const processes[])
{
    int i;

    for (i = 0; processes[i] != NULL; ++i)
    {
        process_exit(processes[i]);
        PRINTF("autostart_exit: stopping process '%s'\n", processes[i]->name);
    }
}
/*---------------------------------------------------------------------------*/
