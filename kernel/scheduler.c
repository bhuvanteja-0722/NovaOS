#include <stdint.h>

#define NOVA_SCHED_MAX 16u
#define NOVA_SCHED_QUANTUM 10u

struct nova_sched_entry {
    uint32_t pid;
    uint32_t state;
    uint32_t ticks_left;
};

static struct nova_sched_entry run_queue[NOVA_SCHED_MAX];
static uint32_t run_count;
static uint32_t current_index;

void scheduler_init(void) {
    run_count = 0;
    current_index = 0;
}

uint32_t scheduler_add(uint32_t pid) {
    if (pid == 0 || run_count >= NOVA_SCHED_MAX) {
        return 0;
    }
    run_queue[run_count].pid = pid;
    run_queue[run_count].state = 1;
    run_queue[run_count].ticks_left = NOVA_SCHED_QUANTUM;
    ++run_count;
    return 1;
}

void scheduler_tick(void) {
    if (run_count == 0) {
        return;
    }
    if (run_queue[current_index].ticks_left > 0) {
        --run_queue[current_index].ticks_left;
    }
    if (run_queue[current_index].ticks_left == 0) {
        run_queue[current_index].ticks_left = NOVA_SCHED_QUANTUM;
        current_index = (current_index + 1) % run_count;
    }
}

uint32_t scheduler_current_pid(void) {
    return run_count == 0 ? 0 : run_queue[current_index].pid;
}

uint32_t scheduler_count(void) {
    return run_count;
}

uint32_t scheduler_quantum(void) {
    return NOVA_SCHED_QUANTUM;
}
