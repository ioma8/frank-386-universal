/* Host build shim for <pico.h> — see desktop/README.md */
#ifndef _HOST_PICO_H
#define _HOST_PICO_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#define __not_in_flash(...)
#define __not_in_flash_func(f) f
#define __time_critical_func(f) f
#define __no_inline_not_in_flash_func(f) f
typedef unsigned int uint;
#endif
