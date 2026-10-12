#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACK_SIZE 1024
#define PRIORITY   5
#define ITERATIONS 10

static void ping_entry(void *p1, void *p2, void *p3);
static void pong_entry(void *p1, void *p2, void *p3);

/* TODO: reserve one stack per thread */
K_THREAD_STACK_DEFINE(stack_ping, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_pong, STACK_SIZE);

/* TODO: declare one thread control block per thread */
struct k_thread thread_ping;
struct k_thread thread_pong;

/* TODO: q2 - declare the semaphores */
K_SEM_DEFINE(sem_ping_done, 0, 1);
K_SEM_DEFINE(sem_pong_done, 1, 1); // Init value = 1 so that thread_ping does not wait on first run
K_SEM_DEFINE(sem_thread_returned, 0, 2); 

static void ping_entry(void *p1, void *p2, void *p3)
{
	printk("Ping start\n");
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	for (int i = 0; i < ITERATIONS; i++) {
		/* TODO: q2 - wait for your turn */
		k_sem_take(&sem_pong_done, K_FOREVER);
		printk("Ping\n");

		/* TODO: q2 - hand the turn over */
		k_sem_give(&sem_ping_done);
	}

	k_sem_give(&sem_thread_returned);
}

static void pong_entry(void *p1, void *p2, void *p3)
{
	printk("Pong start\n");
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	for (int i = 0; i < ITERATIONS; i++) {
		/* TODO: q2 - wait for your turn */
		k_sem_take(&sem_ping_done, K_FOREVER);
		printk("Pong\n");

		/* TODO: q2 - hand the turn over */
		k_sem_give(&sem_pong_done);
	}

	k_sem_give(&sem_thread_returned);
}

int main(void)
{
	printk("=== Threads lab ===\n");

	/* TODO: create both threads */
	k_tid_t tid_ping = k_thread_create(&thread_ping, stack_ping, K_THREAD_STACK_SIZEOF(stack_ping), ping_entry, NULL, NULL, NULL, PRIORITY, 0, K_NO_WAIT);
	k_tid_t tid_pong = k_thread_create(&thread_pong, stack_pong, K_THREAD_STACK_SIZEOF(stack_pong), pong_entry, NULL, NULL, NULL, PRIORITY, 0, K_NO_WAIT);

	k_thread_start(tid_ping);
	k_thread_start(tid_pong);

	/* TODO: wait for both threads to finish */
	k_sem_take(&sem_thread_returned, K_FOREVER);

	printk("=== Done ===\n");
	return 0;
}
