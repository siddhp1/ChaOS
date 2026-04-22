#ifndef WAIT_H
#define WAIT_H

#include "kernel/task.h"

extern struct task* wait_queue;

void task_wait(struct task* task);
void unwait(void);
void unwait_all(void);

#endif
