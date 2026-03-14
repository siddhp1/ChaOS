#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#include "kernel/task.h"

void add_child(struct task* parent, struct task* child);
void remove_child(struct task* parent, struct task* child);
struct task* find_child_by_pid(struct task* parent, int32_t pid);

#endif
