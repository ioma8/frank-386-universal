# frank-386 desktop build (macOS/Linux) — SDL2 window.
#
# Builds the portable emulator core with the host HAL. See desktop/README.md.

ROOT    := $(dir $(lastword $(MAKEFILE_LIST)))/..

CC      := clang
CXX     := clang++
SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LIBS   := $(shell sdl2-config --libs)
SLIRP_CFLAGS := $(shell pkg-config --cflags slirp) -I$(shell pkg-config --variable=includedir slirp)
SLIRP_LIBS   := $(shell pkg-config --libs slirp)

CFLAGS  := -O2 -g -std=gnu11 -Wall -Wno-unused-function -Wno-unused-variable \
           -Wno-missing-field-initializers -Wno-parentheses \
           -DFRANK_HOST=1 -DSOUND_FREQUENCY=44100 -Wno-macro-redefined \
           -include $(ROOT)/desktop/port.h \
           -I$(ROOT)/src -I$(ROOT)/src/emu8950 \
           -I$(ROOT)/desktop/fake -I$(ROOT)/desktop/hostff \
           $(SDL_CFLAGS) $(SLIRP_CFLAGS) -DUSE_SLIRP -DI386_ENABLE_MMX -DI386_ENABLE_SSE -DI386_ENABLE_SSE2 -DI386_ENABLE_SSE3 -DI386_ENABLE_SSSE3 -DUSE_EMU8950_OPL -DEMU8950_SLOT_RENDER=1 -DEMU8950_NO_RATECONV=1 -DEMU8950_NO_WAVE_TABLE_MAP=1 -DEMU8950_NO_TLL=1 -DEMU8950_NO_FLOAT=1 -DEMU8950_NO_TIMER=1 -DEMU8950_NO_TEST_FLAG=1 -DEMU8950_SIMPLER_NOISE=1 -DEMU8950_SHORT_NOISE_UPDATE_CHECK=1 -DEMU8950_LINEAR_SKIP=1 -DEMU8950_LINEAR_END_OF_NOTE_OPTIMIZATION -DEMU8950_NO_PERCUSSION_MODE=1 -DEMU8950_LINEAR=1

CXXFLAGS := -O2 -g -std=gnu++17 -fno-exceptions -fno-rtti \
            -include $(ROOT)/desktop/port.h \
            -I$(ROOT)/src -I$(ROOT)/src/emu8950 \
            -I$(ROOT)/desktop/fake -I$(ROOT)/desktop/hostff -DUSE_EMU8950_OPL -DEMU8950_SLOT_RENDER=1 -DEMU8950_NO_RATECONV=1 -DEMU8950_NO_WAVE_TABLE_MAP=1 -DEMU8950_NO_TLL=1 -DEMU8950_NO_FLOAT=1 -DEMU8950_NO_TIMER=1 -DEMU8950_NO_TEST_FLAG=1 -DEMU8950_SIMPLER_NOISE=1 -DEMU8950_SHORT_NOISE_UPDATE_CHECK=1 -DEMU8950_LINEAR_SKIP=1 -DEMU8950_LINEAR_END_OF_NOTE_OPTIMIZATION -DEMU8950_NO_PERCUSSION_MODE=1 -DEMU8950_LINEAR=1

LIBS    := $(SDL_LIBS) $(SLIRP_LIBS) -lm

CORE := \
	$(ROOT)/src/i386.c \
	$(ROOT)/src/pc.c \
	$(ROOT)/src/vga.c \
	$(ROOT)/src/misc.c \
	$(ROOT)/src/i8042.c \
	$(ROOT)/src/i8254.c \
	$(ROOT)/src/i8257.c \
	$(ROOT)/src/i8259.c \
	$(ROOT)/src/ide.c \
	$(ROOT)/src/fdd.c \
	$(ROOT)/src/disk.c \
	$(ROOT)/src/adlib.c \
	$(ROOT)/src/sb16.c \
	$(ROOT)/src/pcspk.c \
	$(ROOT)/src/sn76489.c \
	$(ROOT)/src/dss.c \
	$(ROOT)/src/pci.c \
	$(ROOT)/src/ne2000.c \
	$(ROOT)/src/fpu.c \
	$(ROOT)/src/ini.c \
	$(ROOT)/src/gameport.c \
	$(ROOT)/src/emu8950/emu8950.c \
	$(ROOT)/src/emu8950/emuadpcm.c

HOST := desktop/host_hal.c desktop/hostff/ff.c desktop/sdl_main.c
CPP  := $(ROOT)/src/emu8950/slot_render.cpp

OBJS := $(patsubst $(ROOT)/%.c,build/%.o,$(CORE)) \
        build/emu8950/slot_render.o \
        $(patsubst %.c,build/%.o,$(HOST))

frank386: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

build/%.o: $(ROOT)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/emu8950/slot_render.o: $(ROOT)/src/emu8950/slot_render.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build frank386

.PHONY: clean
