#ifndef AUTOSTART_H_
#define AUTOSTART_H_

#include "process.h"

/* 注意，这个宏只能在和main函数同一个文件中包含，把所有要自启动的process的地址都写进来 */
#define AUTOSTART_PROCESSES(...) \
    struct process *const autostart_processes[] = {__VA_ARGS__, NULL}

extern struct process *const autostart_processes[];

void autostart_start(struct process *const processes[]);
void autostart_exit(struct process *const processes[]);

#endif /* AUTOSTART_H_ */
