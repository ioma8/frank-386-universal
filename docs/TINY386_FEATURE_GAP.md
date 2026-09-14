# Tiny386 feature comparison

Checked `hchunhui/tiny386` at commit [`4b7133b`](https://github.com/hchunhui/tiny386/tree/4b7133be75b8baf6db71f63172d7cbc87af10517) against this fork.

## Features missing or incomplete in this fork

| Feature | Upstream evidence | Fork status |
|---|---|---|
| NE2000 ISA network card | [`ne2000.c`](https://github.com/hchunhui/tiny386/blob/4b7133be75b8baf6db71f63172d7cbc87af10517/ne2000.c), included by [`Makefile`](https://github.com/hchunhui/tiny386/blob/4b7133be75b8baf6db71f63172d7cbc87af10517/Makefile) | Ported to `src/` and wired at `0x300`, IRQ 9. |
| Host networking for the NE2000 | Upstream supports SLIRP and TAP backends in `ne2000.c` | Desktop uses libslirp; firmware uses the emulation without a host backend. `--net-config` is supported. |
| AMD64 long mode | [`amd64.c`](https://github.com/hchunhui/tiny386/blob/4b7133be75b8baf6db71f63172d7cbc87af10517/amd64.c) and `USE_AMD64` in the upstream Makefile | Not present. The fork is limited to the i386/i486/i586-style core. |
| SSE/SSE2/SSE3/SSSE3 | Upstream [`simd.inc`](https://github.com/hchunhui/tiny386/blob/4b7133be75b8baf6db71f63172d7cbc87af10517/simd.inc) and SIMD compiler flags | Ported and enabled in desktop and firmware builds. Use a CPU generation exposing SIMD (desktop `--cpu 6`). |
| KVM acceleration | Upstream [`kvm.c`](https://github.com/hchunhui/tiny386/blob/4b7133be75b8baf6db71f63172d7cbc87af10517/kvm.c) and `USE_CPUABS` support | Not buildable in this fork; only conditional references remain. |
| VGA memory above 256 KB | Upstream allocates `conf->vga_mem_size` and documents 2 MB configurations | Desktop allocates the configured size (`--vga-mem KB`); RP2350 remains fixed at 256 KB because its scanout buffer is fixed. |
| `fill_cmos` compatibility switch | Upstream supports `fill_cmos=0/1` for Windows 9x versus Windows NT compatibility | Ported; defaults to `1` and accepts `fill_cmos=0` in configuration. |
| Non-SDL2 host targets | Upstream has rawdraw, SDL 1.2, Windows, WebAssembly, and ESP32 ports | This fork has a new SDL2 desktop port and RP2350 firmware, but not those upstream ports. |

## Partially present

Direct Linux boot is still implemented in `src/pc.c` through `linuxstart`, `kernel`, `initrd`, and `cmdline` configuration fields. However, the fork does not include a `linuxstart.bin` payload, and the SDL2 command-line frontend does not expose those options.

## Features this fork adds or retains

- SDL2 desktop window and host-file disk images
- RP2350 board, VGA/HDMI, PS/2, USB HID, and SD-card integration
- Runtime disk/settings UI
- Tandy 3-voice, Covox, DSS, and MPU-401 support
- H: network redirector support

The main remaining upstream gaps are **AMD64 long mode**, **KVM acceleration**, and the non-SDL host frontends. The rest are optional platform features.
