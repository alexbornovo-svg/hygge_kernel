#include "task.h"
#include "../mm/kmalloc.h"

static task_t* task_list = (void *)0;
static task_t* current_task = (void *)0;
static uint32_t next_pid = 1;

void init_scheduler(void)
{
    task_list = (void *)0;
    current_task = (void *)0;
    next_pid = 1;
}

void task_create(void (*entry_point)(void))
{
    task_t* task = (task_t*)kmalloc(sizeof(task_t));

    if (task == (void *)0)
    {
        return;
    }

    uint8_t* stack = (uint8_t*)kmalloc(4096);

    if (stack == (void *)0)
    {
        return;
    }

    task->pid = next_pid++;
    task->stack_base = (uint32_t)stack;
    task->state = TASK_READY;
    task->next = (void *)0;

    uint32_t* stack_top = (uint32_t*)(stack + 4096);

    *(--stack_top) = 0x202;                  // eflags
    *(--stack_top) = 0x08;                   // cs
    *(--stack_top) = (uint32_t)entry_point;  // eip

    task->esp = (uint32_t)stack_top;

    if (task_list == (void *)0)
    {
        task_list = task;
        current_task = task;
        return;
    }

    task_t* cur = task_list;

    while (cur->next != (void *)0)
    {
        cur = cur->next;
    }

    cur->next = task;
}

void schedule(registers_t* regs)
{
    (void)regs;

    if (current_task == (void *)0)
    {
        return;
    }

    task_t* next = current_task->next;

    if (next == (void *)0)
    {
        next = task_list;
    }

    if (next == (void *)0)
    {
        return;
    }

    current_task = next;
}

task_t* get_current_task(void)
{
    return current_task;
}

task_t* scheduler_next(registers_t* regs)
{
    (void)regs;

    task_t* next = current_task->next;

    if (next == (void *)0)
    {
        next = task_list;
    }

    current_task = next;

    return current_task;
}