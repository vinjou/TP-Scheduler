#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACK_SIZE     1024
#define PRIORITY       5
#define WAKER_PRIORITY (PRIORITY - 1)
#define ITERATIONS     10

static int shared_counter;

/* TODO: q1 - declare the semaphore (initial count 1, limit 1) */
K_SEM_DEFINE(sem_start_critical, 1, 1);

static void print_word(const char *word)
{
    for (const char *c = word; *c != '\0'; c++) {
        printk("%c", *c);
        k_sleep(K_MSEC(5));
    }
    printk("\n");
}

static void waker_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    int cnt = 20;

    while (cnt--)
    {
        k_msleep(200);
        k_sem_give(&sem_start_critical);
    }
}

static void worker(const char *name)
{
    for (int i = 0; i < ITERATIONS; i++) {
        /* TODO: q1 - enter the critical section */
        k_sem_take(&sem_start_critical, K_FOREVER);
        int local = shared_counter;

        print_word(name);
        k_sleep(K_MSEC(5));

        shared_counter = local + 1;

        /* TODO: q1 - leave the critical section */
        k_sem_give(&sem_start_critical);

        k_sleep(K_MSEC(1));
    }
}

static void ping_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    worker("Ping");
}

static void pong_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    worker("Pong");
}

/* TODO: q2 - waker thread entry: k_sem_give() every 20 ms, 20 times.
 *            This thread never calls k_sem_take().
 */

K_THREAD_STACK_DEFINE(ping_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(pong_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(waker_stack, STACK_SIZE);
/* TODO: q2 - reserve a stack for the waker thread */

static struct k_thread ping_thread_data;
static struct k_thread pong_thread_data;
static struct k_thread waker_thread_data;
/* TODO: q2 - declare the waker thread control block */

int main(void)
{
    k_tid_t ping_tid, pong_tid, waker_tid;

    printk("=== Semaphore lab ===\n");

    ping_tid = k_thread_create(&ping_thread_data,
                   ping_stack, K_THREAD_STACK_SIZEOF(ping_stack),
                   ping_entry,
                   NULL, NULL, NULL,
                   PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(ping_tid, "ping");

    pong_tid = k_thread_create(&pong_thread_data,
                   pong_stack, K_THREAD_STACK_SIZEOF(pong_stack),
                   pong_entry,
                   NULL, NULL, NULL,
                   PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(pong_tid, "pong");

    waker_tid = k_thread_create(&waker_thread_data,
                   waker_stack, K_THREAD_STACK_SIZEOF(waker_stack),
                   waker_entry,
                   NULL, NULL, NULL,
                   WAKER_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(waker_tid, "waker");

    /* TODO: q2 - create the waker thread at WAKER_PRIORITY */

    k_thread_join(ping_tid, K_FOREVER);
    k_thread_join(pong_tid, K_FOREVER);
    k_thread_join(waker_tid, K_FOREVER);

    printk("counter = %d (expected %d)\n", shared_counter, 2 * ITERATIONS);
    printk("=== Done ===\n");
    return 0;
}