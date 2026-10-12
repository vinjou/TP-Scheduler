#include <errno.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#define STACK_SIZE 1024
#define PRIORITY   5
#define ITERATIONS 10

// TODO: Declarer le bit  EVENT_BUTTON
#define EVENT_BUTTON BIT(0)

/* TODO: déclarer et initialiser l’objet k_event `app_events` */
K_EVENT_DEFINE(app_events);

static const struct gpio_dt_spec button =
	GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

static struct gpio_callback button_callback_data;

K_THREAD_STACK_DEFINE(ping_stack, STACK_SIZE);
static struct k_thread ping_thread_data;

static void ping_work(void)
{
	for (int i = 0; i < ITERATIONS; i++) {
		printk("Ping\n");
		k_sleep(K_MSEC(100));
	}
}

static void button_pressed(const struct device *port,
			   struct gpio_callback *callback,
			   gpio_port_pins_t pins)
{
	ARG_UNUSED(port);
	ARG_UNUSED(callback);
	ARG_UNUSED(pins);

	/* TODO: poster EVENT_BUTTON sur `app_events` */
	k_event_post(&app_events, EVENT_BUTTON);
}

static void ping_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		/* TODO: attendre EVENT_BUTTON de maniere SAFE, puis lancer ping_work() */
		k_event_wait(&app_events, EVENT_BUTTON, true, K_FOREVER); // Clear before waiting
		ping_work();
	}
}

int main(void)
{
	k_event_init(&app_events);

	if (!device_is_ready(button.port)) {
		printk("Erreur : le contrôleur GPIO du bouton n’est pas prêt\n");
		return -ENODEV;
	}

	if (gpio_pin_configure_dt(&button, GPIO_INPUT) < 0) {
		printk("Erreur : configuration du bouton impossible\n");
		return -EIO;
	}

	gpio_init_callback(&button_callback_data,
			   button_pressed,
			   BIT(button.pin));

	if (gpio_add_callback(button.port, &button_callback_data) < 0) {
		printk("Erreur : impossible d’ajouter la callback GPIO\n");
		return -EIO;
	}

	if (gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE) < 0) {
		printk("Erreur : configuration de l’interruption impossible\n");
		return -EIO;
	}

	k_tid_t ping_tid = k_thread_create(&ping_thread_data,
					   ping_stack,
					   K_THREAD_STACK_SIZEOF(ping_stack),
					   ping_entry,
					   NULL, NULL, NULL,
					   PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(ping_tid, "ping");

	printk("=== Events lab ===\n");
	printk("Appuyez sur le bouton pour lancer Ping.\n");

	return 0;
}