# FRANK 386

A small i386 PC emulator for RP2350 boards, with VGA/HDMI, DOS disk images,
PS/2/USB input, and PC/AdLib/Sound Blaster audio.

Based on [Tiny386](https://github.com/hchunhui/tiny386).

## Desktop build

The SDL2 desktop port runs on macOS and Linux.

```bash
brew install sdl2 libslirp       # macOS
cd desktop
make
./frank386 --fda "" --hda "$HOME/frank-disks/msdos622.img"
```

The emulator expects these BIOS files by default:

```text
386/bios.bin
386/vgabios.bin
```

Use `--fda`, `--hda`, `--cda`, and `--root` to attach other images. Absolute
paths are accepted. The emulated NE2000 uses libslirp; pass `--net-config FILE`
for an optional SLIRP config. `--vga-mem KB` enables larger desktop VGA memory;
`--cpu 6` exposes the emulated MMX/SSE-family instructions. `--type 'ver\r'`
sends scripted keystrokes to the guest.

The tested MS-DOS 6.22 image boots directly to `C:\>`:

```text
Starting MS-DOS...
C:\>
```

## Firmware build

Requires the [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)
and an ARM GCC toolchain.

```bash
export PICO_SDK_PATH=/path/to/pico-sdk
./build.sh
```

See `build.sh` for board and USB options.

## License

MIT. See [LICENSE](LICENSE).
