/**
 * frank-386 - i386 PC Emulator for RP2350
 *
 * Stub implementations for disabled features (FPU).
 * Sound and NE2000 emulation are enabled.
 *
 * Copyright (c) 2026 Mikhail Matveev <xtreme@rh1.tech>
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

//=============================================================================
// FPU Stubs (if FPU is disabled)
//=============================================================================

#ifdef NO_FPU
typedef struct FPU FPU;

FPU *fpu_new(void) {
    return NULL;
}

int fpu_exec1(FPU *f, uint32_t op) {
    (void)f;
    (void)op;
    return 0;  // Not handled
}

int fpu_exec2(FPU *f, uint32_t op, uint32_t addr, uint8_t *mem) {
    (void)f;
    (void)op;
    (void)addr;
    (void)mem;
    return 0;  // Not handled
}
#endif
