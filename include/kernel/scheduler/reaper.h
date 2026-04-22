#ifndef REAPER_H
#define REAPER_H

#include "kernel/task.h"

#define REAP_TICKS 100

void task_zombie(struct task* task);
void reap_zombies(void);

#endif
