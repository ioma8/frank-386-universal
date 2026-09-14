/*
 * Host build portability shim — force-included into every translation
 * unit by desktop/Makefile.
 *
 * These macros come from the Pico SDK (<pico.h> / <pico/platform.h>) on
 * the RP2350. On the host build the emulator core is compiled without
 * RP2350_BUILD, so the SDK headers are never pulled in and the macros
 * would be undefined. Placing code in RAM is meaningless on a host, so
 * every one of them expands to nothing (except __always_inline, which
 * really is a compiler attribute).
 */
#ifndef FRANK_HOST_PORT_H
#define FRANK_HOST_PORT_H

#ifndef __not_in_flash
#define __not_in_flash(...)
#endif
#ifndef __not_in_flash_func
#define __not_in_flash_func(f) f
#endif
#ifndef __time_critical_func
#define __time_critical_func(f) f
#endif
#ifndef __no_inline_not_in_flash_func
#define __no_inline_not_in_flash_func(f) f
#endif
#ifndef __always_inline
#define __always_inline inline __attribute__((always_inline))
#endif
#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif
#ifndef DRAM_ATTR
#define DRAM_ATTR
#endif

/* RP2350 SDK helper. On a host the plain multiply is already "fast". */
#ifndef __fast_mul
#define __fast_mul(a, b) ((a) * (b))
#endif

/* Audio sample rate — a CMake compile definition on the RP2350 build. */
#ifndef SOUND_FREQUENCY
#define SOUND_FREQUENCY 44100
#endif

/* ARM memory barrier; a compiler barrier is the host equivalent. */
#ifndef __dmb
#define __dmb() __asm__ __volatile__("" ::: "memory")
#endif

#endif /* FRANK_HOST_PORT_H */
