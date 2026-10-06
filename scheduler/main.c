#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#define MAX_TASKS 10
#define MSEC_IN_NSEC 1000000

typedef struct {
    const char *name;
    uint32_t period_ms;
    uint32_t max_runs;
    uint64_t last_run_ms;
    uint32_t run_count;
    void (*func)(void);
} task_t;

static task_t tasks[MAX_TASKS];
static int task_count = 0;

uint64_t get_time_ms(void) {
    struct timespec tspec;
    clock_gettime(CLOCK_REALTIME, &tspec);

    return (tspec.tv_nsec/MSEC_IN_NSEC);
}

void task_register(const char *name, uint32_t period_ms, uint32_t max_runs, void (*func)(void)) {
    
    if (task_count <= MAX_TASKS)
    {
        task_t new_task = {
        .name = name,
        .period_ms = period_ms,
        .max_runs = max_runs,
        .run_count = 0,
        .last_run_ms = 0,
        .func = func,
        };

        tasks[task_count] = new_task;
        task_count++;
    }
    else
    {
        printf("Error : Maximum number of task reached. Task %s was dropped.", name);
    }

}

void ms_sleep(int ms)
{
    struct timespec tspec = {
        .tv_sec = (time_t)0,
        .tv_nsec = ms*MSEC_IN_NSEC,
    };

    nanosleep(&tspec, NULL);
}

void task_1_handler(void) {
    printf("-> Task 1 logic executed\n");
}

void task_2_handler(void) {
    printf("-> Task 2 logic executed\n");
}

int main(void) {
    task_register("SensorTask", 100, 12, task_1_handler); // Runs 12 times
    task_register("LoggerTask", 500, 2, task_2_handler); // Runs 2 time

    while (true) {
        ms_sleep(100);
    }

    return 0;
}
