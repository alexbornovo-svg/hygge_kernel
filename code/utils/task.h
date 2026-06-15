#pragma once
#include "../include/types.h"

#include "regs.h"

typedef enum { TASK_READY, TASK_RUNNING, TASK_ZOMBIE } task_state_t;

typedef struct task {
    uint32_t pid;
    uint32_t esp;
    uint32_t stack_base;
    task_state_t state;
    struct task* next;
} task_t;

void init_scheduler(void);
void task_create(void (*entry_point)(void));
void schedule(registers_t* regs);
task_t* get_current_task(void);
task_t* scheduler_next(registers_t* regs);