/*
 * Host build HAL — the handful of platform functions the emulator core
 * expects, implemented with ordinary POSIX calls.
 *
 * On the RP2350 these live in main.c/platform_rp2350.c and talk to the
 * Pico SDK (PSRAM at 0x11000000, SD card, hardware timers).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "i386.h"
#include "pc.h"
#include "pico/time.h"

/* Global PC instance, referenced by adlib.c. On the RP2350 this is
 * defined in main.c. */
PC *pc = NULL;

/* Video memory window: the emulated VGA RAM. On the RP2350 this is a
 * fixed 256 KB SRAM buffer; here it is just a static array. */
uint8_t gfx_buffer[256ul << 10];

uint64_t time_us_64(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000000ull + (uint64_t)ts.tv_nsec / 1000ull;
}

uint32_t time_us_32(void)
{
	return (uint32_t)time_us_64();
}

uint32_t get_uticks(void)
{
	return time_us_32();
}

/* The RP2350 returns a pointer into PSRAM here. There is no fixed
 * address to return on a host, so a plain heap allocation stands in. */
void *pcmalloc(long size)
{
	void *p = calloc(1, (size_t)size);
	if (!p) {
		fprintf(stderr, "pcmalloc(%ld) failed\n", size);
		abort();
	}
	return p;
}

int load_rom(void *phys_mem, const char *file, uword addr, int backward)
{
	FILE *fp = fopen(file, "rb");
	if (!fp) {
		fprintf(stderr, "load_rom: cannot open %s: %s\n",
			file, strerror(errno));
		abort();
	}
	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);
	rewind(fp);
	if (backward)
		fread((uint8_t *)phys_mem + addr - len, 1, len, fp);
	else
		fread((uint8_t *)phys_mem + addr, 1, len, fp);
	fclose(fp);
	fprintf(stderr, "load_rom: %s -> 0x%06x, %ld bytes\n",
		file, (unsigned)addr, len);
	return (int)len;
}

/* Optional guest-to-host redirector; not wired up on the host build. */
void netredirect_init(CPUI386 *cpu, int enable)
{
	(void)cpu;
	(void)enable;
}

/*
 * Pico repeating timers, used here only by adlib.c to drive OPL
 * rendering at 1 kHz. The host has no ISR context, so due callbacks are
 * run from the frontend's event loop via host_timers_poll().
 */
#define HOST_TIMER_MAX 16
static repeating_timer_t *timers[HOST_TIMER_MAX];

bool add_repeating_timer_us(int64_t delay_us,
			    bool (*callback)(repeating_timer_t *),
			    void *user_data, repeating_timer_t *out)
{
	int slot = -1;
	for (int i = 0; i < HOST_TIMER_MAX; i++) {
		if (!timers[i]) { slot = i; break; }
	}
	if (slot < 0 || !out) return false;

	out->callback  = callback;
	out->user_data = user_data;
	out->delay_us  = delay_us;
	out->next_us   = (int64_t)time_us_64() + delay_us;
	out->active    = 1;
	timers[slot]   = out;
	return true;
}

bool cancel_repeating_timer(repeating_timer_t *t)
{
	for (int i = 0; i < HOST_TIMER_MAX; i++) {
		if (timers[i] == t) { timers[i] = NULL; t->active = 0; return true; }
	}
	return false;
}

void host_timers_poll(void)
{
	int64_t now = (int64_t)time_us_64();
	for (int i = 0; i < HOST_TIMER_MAX; i++) {
		repeating_timer_t *t = timers[i];
		if (!t || !t->active) continue;
		if (now < t->next_us) continue;
		/* Reschedule before the callback: it may render for a while
		 * and we do not want to re-enter it on the next poll. */
		t->next_us = now + t->delay_us;
		t->callback(t);
	}
}
