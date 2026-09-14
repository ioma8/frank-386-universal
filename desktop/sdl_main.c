/*
 * frank-386 desktop frontend — SDL2 window, keyboard and mouse.
 *
 * The emulator core is the same code the RP2350 build runs; this file
 * replaces the Pico SDK platform layer with a window on the host.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "pc.h"
#include "i8042.h"
#include "pico/time.h"   /* host repeating-timer shim: host_timers_poll() */
#include <hardware/timer.h>   /* time_us_64() from the host HAL */

extern PC *pc;   /* defined by the host HAL, used by adlib.c */

typedef struct {
	SDL_Window   *win;
	SDL_Renderer *ren;
	SDL_Texture  *tex;
	PC           *pc;
	uint8_t      *fb;      /* software render target (vga_refresh writes here) */
	int           width, height;
	int           scale;
} Console;

/* AT set-1 scancodes. Extended keys are encoded 0xE000 | code, which is
 * what ps2_put_keycode() expects (i8042.c). */
static const uint16_t at_scancode[SDL_NUM_SCANCODES] = {
	[SDL_SCANCODE_ESCAPE] = 0x01,
	[SDL_SCANCODE_1] = 0x02, [SDL_SCANCODE_2] = 0x03, [SDL_SCANCODE_3] = 0x04,
	[SDL_SCANCODE_4] = 0x05, [SDL_SCANCODE_5] = 0x06, [SDL_SCANCODE_6] = 0x07,
	[SDL_SCANCODE_7] = 0x08, [SDL_SCANCODE_8] = 0x09, [SDL_SCANCODE_9] = 0x0A,
	[SDL_SCANCODE_0] = 0x0B,
	[SDL_SCANCODE_MINUS] = 0x0C, [SDL_SCANCODE_EQUALS] = 0x0D,
	[SDL_SCANCODE_BACKSPACE] = 0x0E, [SDL_SCANCODE_TAB] = 0x0F,
	[SDL_SCANCODE_Q] = 0x10, [SDL_SCANCODE_W] = 0x11, [SDL_SCANCODE_E] = 0x12,
	[SDL_SCANCODE_R] = 0x13, [SDL_SCANCODE_T] = 0x14, [SDL_SCANCODE_Y] = 0x15,
	[SDL_SCANCODE_U] = 0x16, [SDL_SCANCODE_I] = 0x17, [SDL_SCANCODE_O] = 0x18,
	[SDL_SCANCODE_P] = 0x19,
	[SDL_SCANCODE_LEFTBRACKET] = 0x1A, [SDL_SCANCODE_RIGHTBRACKET] = 0x1B,
	[SDL_SCANCODE_RETURN] = 0x1C, [SDL_SCANCODE_LCTRL] = 0x1D,
	[SDL_SCANCODE_A] = 0x1E, [SDL_SCANCODE_S] = 0x1F, [SDL_SCANCODE_D] = 0x20,
	[SDL_SCANCODE_F] = 0x21, [SDL_SCANCODE_G] = 0x22, [SDL_SCANCODE_H] = 0x23,
	[SDL_SCANCODE_J] = 0x24, [SDL_SCANCODE_K] = 0x25, [SDL_SCANCODE_L] = 0x26,
	[SDL_SCANCODE_SEMICOLON] = 0x27, [SDL_SCANCODE_APOSTROPHE] = 0x28,
	[SDL_SCANCODE_GRAVE] = 0x29, [SDL_SCANCODE_LSHIFT] = 0x2A,
	[SDL_SCANCODE_BACKSLASH] = 0x2B,
	[SDL_SCANCODE_Z] = 0x2C, [SDL_SCANCODE_X] = 0x2D, [SDL_SCANCODE_C] = 0x2E,
	[SDL_SCANCODE_V] = 0x2F, [SDL_SCANCODE_B] = 0x30, [SDL_SCANCODE_N] = 0x31,
	[SDL_SCANCODE_M] = 0x32,
	[SDL_SCANCODE_COMMA] = 0x33, [SDL_SCANCODE_PERIOD] = 0x34,
	[SDL_SCANCODE_SLASH] = 0x35, [SDL_SCANCODE_RSHIFT] = 0x36,
	[SDL_SCANCODE_KP_MULTIPLY] = 0x37, [SDL_SCANCODE_LALT] = 0x38,
	[SDL_SCANCODE_SPACE] = 0x39, [SDL_SCANCODE_CAPSLOCK] = 0x3A,
	[SDL_SCANCODE_F1] = 0x3B, [SDL_SCANCODE_F2] = 0x3C, [SDL_SCANCODE_F3] = 0x3D,
	[SDL_SCANCODE_F4] = 0x3E, [SDL_SCANCODE_F5] = 0x3F, [SDL_SCANCODE_F6] = 0x40,
	[SDL_SCANCODE_F7] = 0x41, [SDL_SCANCODE_F8] = 0x42, [SDL_SCANCODE_F9] = 0x43,
	[SDL_SCANCODE_F10] = 0x44,
	[SDL_SCANCODE_NUMLOCKCLEAR] = 0x45, [SDL_SCANCODE_SCROLLLOCK] = 0x46,
	[SDL_SCANCODE_KP_7] = 0x47, [SDL_SCANCODE_KP_8] = 0x48,
	[SDL_SCANCODE_KP_9] = 0x49, [SDL_SCANCODE_KP_MINUS] = 0x4A,
	[SDL_SCANCODE_KP_4] = 0x4B, [SDL_SCANCODE_KP_5] = 0x4C,
	[SDL_SCANCODE_KP_6] = 0x4D, [SDL_SCANCODE_KP_PLUS] = 0x4E,
	[SDL_SCANCODE_KP_1] = 0x4F, [SDL_SCANCODE_KP_2] = 0x50,
	[SDL_SCANCODE_KP_3] = 0x51, [SDL_SCANCODE_KP_0] = 0x52,
	[SDL_SCANCODE_KP_PERIOD] = 0x53,
	[SDL_SCANCODE_F11] = 0x57, [SDL_SCANCODE_F12] = 0x58,
	/* extended */
	[SDL_SCANCODE_KP_ENTER] = 0xE01C, [SDL_SCANCODE_RCTRL] = 0xE01D,
	[SDL_SCANCODE_KP_DIVIDE] = 0xE035, [SDL_SCANCODE_RALT] = 0xE038,
	[SDL_SCANCODE_HOME] = 0xE047, [SDL_SCANCODE_UP] = 0xE048,
	[SDL_SCANCODE_PAGEUP] = 0xE049, [SDL_SCANCODE_LEFT] = 0xE04B,
	[SDL_SCANCODE_RIGHT] = 0xE04D, [SDL_SCANCODE_END] = 0xE04F,
	[SDL_SCANCODE_DOWN] = 0xE050, [SDL_SCANCODE_PAGEDOWN] = 0xE051,
	[SDL_SCANCODE_INSERT] = 0xE052, [SDL_SCANCODE_DELETE] = 0xE053,
	[SDL_SCANCODE_PRINTSCREEN] = 99,  /* special-cased in ps2_put_keycode */
};

static void redraw(void *opaque, int x, int y, int w, int h)
{
	Console *c = opaque;
	int pitch = c->width * 4;
	if (w <= 0 || h <= 0) {
		SDL_UpdateTexture(c->tex, NULL, c->fb, pitch);
	} else {
		SDL_Rect r = { x, y, w, h };
		SDL_UpdateTexture(c->tex, &r, c->fb + (y * c->width + x) * 4, pitch);
	}
	SDL_RenderClear(c->ren);
	SDL_RenderCopy(c->ren, c->tex, NULL, NULL);
	SDL_RenderPresent(c->ren);
}

static void poll(void *opaque)
{
	Console *c = opaque;
	SDL_Event ev;

	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
		case SDL_QUIT:
			exit(0);
		case SDL_KEYDOWN:
		case SDL_KEYUP: {
			uint16_t code = at_scancode[ev.key.keysym.scancode];
			if (code)
				ps2_put_keycode(c->pc->kbd,
						ev.type == SDL_KEYDOWN, code);
			break;
		}
		case SDL_MOUSEMOTION:
			if (c->pc->mouse_enabled)
				ps2_mouse_event(c->pc->mouse,
						ev.motion.xrel, -ev.motion.yrel,
						0, 0);
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP: {
			if (!c->pc->mouse_enabled) break;
			Uint32 st = SDL_GetMouseState(NULL, NULL);
			int b = 0;
			if (st & SDL_BUTTON(SDL_BUTTON_LEFT))   b |= 1;
			if (st & SDL_BUTTON(SDL_BUTTON_RIGHT))  b |= 2;
			if (st & SDL_BUTTON(SDL_BUTTON_MIDDLE)) b |= 4;
			ps2_mouse_event(c->pc->mouse, 0, 0, 0, b);
			break;
		}
		default:
			break;
		}
	}
}

static void usage(const char *argv0)
{
	fprintf(stderr,
		"Usage: %s [options]\n"
		"  --root DIR        image root (default: FRANK_ROOT or .)\n"
		"  --bios FILE       main BIOS (default: 386/bios.bin)\n"
		"  --vga-bios FILE   VGA BIOS  (default: 386/vgabios.bin)\n"
		"  --fda NAME        floppy A: image, inside the 386/ image dir\n"
		"  --fdb FILE        floppy B: image\n"
		"  --hda FILE        hard disk C: image\n"
		"  --hdb FILE        hard disk D: image\n"
		"  --cda FILE        CD-ROM E: image\n"
		"  -m MB             guest RAM in MB (default 8)\n"
		"  --vga-mem KB      VGA memory in KB (default 256)\n"
		"  --cpu N           CPU generation 3/4/6 (default 4)\n"
		"  --no-fpu          disable x87\n"
		"  --scale N         window scale (default 2)\n"
		"  --type SCRIPT     type SCRIPT into the guest (see below)\n"
		"  --type-delay SEC  wait SEC seconds before typing (default 20)\n"
		"  --net-config FILE SLIRP network configuration (optional)\n"
		"\nSCRIPT: characters are typed as keys. Backslash escapes:\n"
		"  \\r Enter   \\e Esc   \\t Tab   \\1..\\9 F1..F9   \\0 F10\n"
		"  \\- F11   \\= F12   \\~ pause 3 s   \\\\ literal backslash\n",
		argv0);
}

/*
 * Scripted keystrokes, for driving a DOS installer non-interactively.
 * Key numbers are AT set-1 / raw Linux input codes, which is what
 * ps2_put_keycode() accepts (see the mapping in i8042.c).
 */
typedef struct {
	const char *script;
	int         idx;
	/* one script item may need several key events (Shift, key, ...) */
	uint8_t     code[4];
	uint8_t     down[4];
	int         n, i;
	uint64_t    next_us;
	int         done;
} Typist;

/* Returns the unshifted key for a character, or -1. */
static int base_key(char ch)
{
	static const uint8_t k_digit[10] = { 11, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	static const uint8_t k_alpha[26] = {
		30, 48, 46, 32, 18, 33, 34, 35, 23, 36, 37, 38, 50,
		49, 24, 25, 16, 19, 31, 20, 22, 47, 17, 45, 21, 44
	};
	if (ch >= 'a' && ch <= 'z') return k_alpha[ch - 'a'];
	if (ch >= 'A' && ch <= 'Z') return k_alpha[ch - 'A'];
	if (ch >= '0' && ch <= '9') return k_digit[ch - '0'];
	switch (ch) {
	case ' ':  return 57;  /* space */
	case '.':  return 52;
	case '-':  return 12;
	case '/':  return 53;
	case ';':  return 39;
	case '\'': return 40;
	case '\\': return 43;
	case '=':  return 13;
	case ',':  return 51;
	case '[':  return 26;
	case ']':  return 27;
	case '`':  return 41;
	default:   return -1;
	}
}

/* Characters that need Shift, mapped to the key underneath. */
static int shifted_key(char ch, int *out_key)
{
	switch (ch) {
	case ':': *out_key = 39; return 1;   /* Shift+; */
	case '!': *out_key = 2;  return 1;
	case '@': *out_key = 3;  return 1;
	case '#': *out_key = 4;  return 1;
	case '$': *out_key = 5;  return 1;
	case '%': *out_key = 6;  return 1;
	case '^': *out_key = 7;  return 1;
	case '&': *out_key = 8;  return 1;
	case '*': *out_key = 9;  return 1;
	case '(': *out_key = 10; return 1;
	case ')': *out_key = 11; return 1;
	case '_': *out_key = 12; return 1;
	case '+': *out_key = 13; return 1;
	case '<': *out_key = 51; return 1;
	case '>': *out_key = 52; return 1;
	case '?': *out_key = 53; return 1;
	case '{': *out_key = 26; return 1;
	case '}': *out_key = 27; return 1;
	case '|': *out_key = 43; return 1;
	case '"': *out_key = 40; return 1;
	case '~': *out_key = 41; return 1;
	default:  return 0;
	}
}

static int special_key(char ch)
{
	switch (ch) {
	case 'r': return 0x1C;              /* Enter */
	case 'e': return 0x01;              /* Esc */
	case 't': return 0x0F;              /* Tab */
	case '1': return 0x3B; case '2': return 0x3C; case '3': return 0x3D;
	case '4': return 0x3E; case '5': return 0x3F; case '6': return 0x40;
	case '7': return 0x41; case '8': return 0x42; case '9': return 0x43;
	case '0': return 0x44;              /* F10 */
	case '-': return 0x57;              /* F11 */
	case '=': return 0x58;              /* F12 */
	case '\\': return 43;
	default:  return -1;
	}
}

/* Load the key events for the next script item. 0 = nothing left. */
static int typist_load(Typist *t)
{
	for (;;) {
		char ch = t->script[t->idx];
		if (ch == 0) return 0;

		if (ch == '\\') {
			char esc = t->script[t->idx + 1];
			if (esc == '~') {                 /* pause, not a key */
				t->idx += 2;
				t->next_us = time_us_64() + 3000000ull;
				return -1;
			}
			int k = special_key(esc);
			t->idx += 2;
			if (k < 0) continue;
			t->code[0] = (uint8_t)k; t->down[0] = 1;
			t->code[1] = (uint8_t)k; t->down[1] = 0;
			t->n = 2; t->i = 0;
			return 1;
		}

		int base = -1;
		int is_shift = shifted_key(ch, &base);
		if (!is_shift) base = base_key(ch);
		if (base < 0) { t->idx++; continue; }
		if (ch >= 'A' && ch <= 'Z') is_shift = 1;

		t->idx++;
		if (is_shift) {
			t->code[0] = 0x2A; t->down[0] = 1;   /* Left Shift */
			t->code[1] = (uint8_t)base; t->down[1] = 1;
			t->code[2] = (uint8_t)base; t->down[2] = 0;
			t->code[3] = 0x2A; t->down[3] = 0;
			t->n = 4;
		} else {
			t->code[0] = (uint8_t)base; t->down[0] = 1;
			t->code[1] = (uint8_t)base; t->down[1] = 0;
			t->n = 2;
		}
		t->i = 0;
		return 1;
	}
}

static void typist_tick(Typist *t, PC *pc)
{
	if (t->done || !t->script || !pc->kbd) return;
	if (time_us_64() < t->next_us) return;

	while (t->i >= t->n) {
		int r = typist_load(t);
		if (r == 0) { t->done = 1; return; }   /* script finished */
		if (r < 0) return;                      /* paused */
	}

	ps2_put_keycode(pc->kbd, t->down[t->i], t->code[t->i]);
	t->i++;
	/* Shift is held across the key, so only pace actual presses. */
	t->next_us = time_us_64() + (t->down[t->i - 1] ? 20000ull : 90000ull);
}

static void dump_text(PC *pc)
{
	if (!pc->vga || vga_get_mode(pc->vga) != 1) {
		fprintf(stderr, "|<not in text mode: %d>|\n", pc->vga ? vga_get_mode(pc->vga) : -1);
		return;
	}
	uint32_t sa = vga_get_start_addr(pc->vga);
	int cols = vga_get_text_cols(pc->vga);
	uint8_t *ram = (uint8_t *)pc->vga->vga_ram;
	fprintf(stderr, "--- guest text screen (%d cols) ---\n", cols);
	for (int r = 0; r < 25; r++) {
		char line[200];
		int n = 0;
		for (int cx = 0; cx < cols && n < 199; cx++) {
			uint16_t a = *(uint16_t *)(ram + ((sa + r * cols + cx) * 4));
			line[n++] = (a & 0xff) ? (char)(a & 0xff) : ' ';
		}
		line[n] = 0;
		fprintf(stderr, "|%s|\n", line);
	}
}

int main(int argc, char *argv[])
{
	static PCConfig conf;
	static char bios[1024] = "386/bios.bin";
	static char vgabios[1024] = "386/vgabios.bin";
	static char fda[1024] = "fdd0.img";  /* name inside the image dir, like config.ini */
	const char *type_script = NULL;
	const char *net_config = NULL;
	int type_delay_s = 20;
	int scale = 2;

	memset(&conf, 0, sizeof(conf));
	conf.mem_size = 8 * 1024 * 1024;
	conf.vga_mem_size = 256 * 1024;
	conf.width = 720;
	conf.height = 480;
	conf.cpu_gen = 4;
	conf.fpu = 1;
	conf.fill_cmos = 1;
	conf.bios = bios;
	conf.vga_bios = vgabios;
	conf.fdd[0] = fda;

	for (int i = 1; i < argc; i++) {
		const char *a = argv[i];
		const char *v = (i + 1 < argc) ? argv[i + 1] : NULL;
#define NEED(x) do { if (!v) { fprintf(stderr, "%s needs a value\n", x); return 1; } } while (0)
		if (!strcmp(a, "--root")) {
			NEED(a); setenv("FRANK_ROOT", v, 1); i++;
		} else if (!strcmp(a, "--bios")) {
			NEED(a); snprintf(bios, sizeof(bios), "%s", v); i++;
		} else if (!strcmp(a, "--vga-bios")) {
			NEED(a); snprintf(vgabios, sizeof(vgabios), "%s", v); i++;
		} else if (!strcmp(a, "--fda")) {
			NEED(a); snprintf(fda, sizeof(fda), "%s", v); i++;
		} else if (!strcmp(a, "--fdb")) {
			NEED(a); conf.fdd[1] = v; i++;
		} else if (!strcmp(a, "--hda")) {
			NEED(a); conf.ata[0] = v; i++;
		} else if (!strcmp(a, "--hdb")) {
			NEED(a); conf.ata[1] = v; i++;
		} else if (!strcmp(a, "--cda")) {
			NEED(a); conf.ata[2] = v; conf.iscd[2] = 1; i++;
		} else if (!strcmp(a, "-m")) {
			NEED(a); conf.mem_size = atol(v) * 1024 * 1024; i++;
		} else if (!strcmp(a, "--vga-mem")) {
			NEED(a); conf.vga_mem_size = atol(v) * 1024; i++;
		} else if (!strcmp(a, "--cpu")) {
			NEED(a); conf.cpu_gen = atoi(v); i++;
		} else if (!strcmp(a, "--no-fpu")) {
			conf.fpu = 0;
		} else if (!strcmp(a, "--scale")) {
			NEED(a); scale = atoi(v); i++;
		} else if (!strcmp(a, "--type")) {
			NEED(a); type_script = v; i++;
		} else if (!strcmp(a, "--type-delay")) {
			NEED(a); type_delay_s = atoi(v); i++;
		} else if (!strcmp(a, "--net-config")) {
			NEED(a); net_config = v; i++;
		} else if (!strcmp(a, "-h") || !strcmp(a, "--help")) {
			usage(argv[0]); return 0;
		} else {
			usage(argv[0]); return 1;
		}
#undef NEED
	}

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return 1;
	}

	Console *c = calloc(1, sizeof(Console));
	c->width = conf.width;
	c->height = conf.height;
	c->scale = scale;
	c->win = SDL_CreateWindow("FRANK 386",
				  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
				  c->width * scale, c->height * scale,
				  SDL_WINDOW_SHOWN);
	if (!c->win) {
		fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
		return 1;
	}
	c->ren = SDL_CreateRenderer(c->win, -1, SDL_RENDERER_ACCELERATED);
	if (!c->ren) {
		fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
		return 1;
	}
	c->tex = SDL_CreateTexture(c->ren, SDL_PIXELFORMAT_ARGB8888,
				   SDL_TEXTUREACCESS_STREAMING,
				   c->width, c->height);
	if (!c->tex) {
		fprintf(stderr, "SDL_CreateTexture: %s\n", SDL_GetError());
		return 1;
	}

	c->fb = calloc((size_t)c->width * c->height, 4);
	if (net_config) ne2000_set_config_file(net_config);
	pc = pc_new(redraw, poll, c, c->fb, &conf);
	c->pc = pc;
	SDL_SetRelativeMouseMode(SDL_TRUE);

	load_bios_and_reset(pc);
	pc->boot_start_time = get_uticks();

	/* FRANK_SHOT=file.bmp dumps the guest screen once, a few seconds
	 * after boot. Used to verify a headless-ish run actually renders. */
	/* FRANK_TRACE=1 prints, every 3 s, how fast the guest is executing
	 * and what is on its text screen. Used to debug a headless run. */
	int trace = getenv("FRANK_TRACE") != NULL;
	uint64_t trace_at = trace ? (uint64_t)get_uticks() + 3000000ull : 0;
	long trace_cycles = 0;


	const char *shot_path = getenv("FRANK_SHOT");
	const char *shot_sec = getenv("FRANK_SHOT_SEC");
	uint64_t shot_delay = shot_sec ? (uint64_t)atoll(shot_sec) * 1000000ull : 4000000ull;
	uint64_t shot_at = shot_path ? (uint64_t)get_uticks() + shot_delay : 0;

	Typist typist;
	memset(&typist, 0, sizeof(typist));
	typist.script = type_script;
	typist.next_us = time_us_64() + (uint64_t)type_delay_s * 1000000ull;

	for (;;) {
		pc_step(pc);
		pc_vga_step(pc);
		poll(c);
		host_timers_poll();
		typist_tick(&typist, pc);

		if (trace && (uint64_t)get_uticks() >= trace_at) {
			long cyc = cpui386_get_cycle(pc->cpu);
			trace_at = (uint64_t)get_uticks() + 3000000ull;
			uint32_t cs = 0, ip = 0;
			int halt = 0;
			cpui386_get_state(pc->cpu, &cs, &ip, &halt);
			fprintf(stderr, "[trace] %5.2f MIPS  CS=%04lx IP=%08lx halt=%d\n",
				(cyc - trace_cycles) / 3000000.0,
				(unsigned long)cs, (unsigned long)ip, halt);
			trace_cycles = cyc;
			dump_text(pc);
		}


		if (shot_path && shot_at && (uint64_t)get_uticks() >= shot_at) {
			SDL_Surface *s = SDL_CreateRGBSurfaceWithFormatFrom(
				c->fb, c->width, c->height, 32, c->width * 4,
				SDL_PIXELFORMAT_ARGB8888);
			if (s) {
				SDL_SaveBMP(s, shot_path);
				SDL_FreeSurface(s);
				fprintf(stderr, "wrote %s\n", shot_path);
			}
			dump_text(pc);
			shot_at = 0;
		}
	}
	return 0;
}
