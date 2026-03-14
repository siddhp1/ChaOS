#ifndef REAPER_H
#define REAPER_H

#include "kernel/task.h"

void task_zombie(struct task* task);
void reap_zombies(void);

#endif
