/* Host shim for pico/time.h — the only piece adlib.c needs is the
 * repeating-timer API that drives OPL rendering. */
#ifndef _HOST_PICO_TIME_H
#define _HOST_PICO_TIME_H
#include "pico.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct repeating_timer {
	bool (*callback)(struct repeating_timer *);
	void *user_data;
	int64_t delay_us;
	int64_t next_us;
	int     active;
} repeating_timer_t;

bool add_repeating_timer_us(int64_t delay_us,
			    bool (*callback)(repeating_timer_t *),
			    void *user_data, repeating_timer_t *out);
bool cancel_repeating_timer(repeating_timer_t *t);

/* Service due callbacks. Called from the host event loop. */
void host_timers_poll(void);

#endif
