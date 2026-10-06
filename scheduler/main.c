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
static int active_task_count = 0;

uint64_t get_time_ms(void) {
    struct timespec tspec;
    clock_gettime(CLOCK_REALTIME, &tspec);

    return (uint64_t)(tspec.tv_nsec/MSEC_IN_NSEC);
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

        printf("Register task %s, id = %d \n", name, task_count);
    }
    else
    {
        printf("Error : Maximum number of task reached. Task %s was dropped. \n", name);
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

void print_task_stats(int task_id)
{
    task_t t = tasks[task_id-1];
    printf("| Run %02d/%02d, delay %d ms \n", t.run_count, t.max_runs, t.last_run_ms - get_time_ms());
}

void task_1_handler(void) {
    printf("-> Task 1 logic executed ");
    print_task_stats(1);
}

void task_2_handler(void) {
    printf("-> Task 2 logic executed ");
    print_task_stats(2);
}

int main(void) {
    task_register("SensorTask", 100, 12, task_1_handler); // Runs 12 times
    task_register("LoggerTask", 500, 2, task_2_handler); // Runs 2 time

    active_task_count = task_count;

    while (true) {

        for (int i = 0; i < task_count; i++)
        {
            // Check if tasks[i] period has run out, OR if task has never been run
            if ((tasks[i].last_run_ms - get_time_ms() > tasks[i].period_ms) || tasks[i].last_run_ms == 0)
            {
                // Check for remaining runs
                if(tasks[i].run_count < tasks[i].max_runs)
                {
                    // Execute function
                    tasks[i].func();

                    // Update last_run_ms
                    tasks[i].last_run_ms = get_time_ms();

                    // Increment run_count
                    tasks[i].run_count++;

                    // Max run count has been reached -> decrement active task counter
                    if (tasks[i].run_count == tasks[i].max_runs)
                    {
                        active_task_count--;
                    }
                }
            }
        }

        if (!active_task_count)
        {
            printf("All tasks have been executed. Exiting...");
            break;
        }
    }

    return 0;
}
