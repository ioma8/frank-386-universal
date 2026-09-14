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

### Windows 95

Windows 95 OSR2 has reached graphical Setup, but installation and booting an
installed Windows desktop have **not yet been verified**. See [STATUS.md](STATUS.md)
for the installation checkpoint and remaining checks.

Use a **500 MiB** hard-disk image: the current image-size limit is
503.5078125 MiB, so 512 MiB and 1 GiB images are rejected. The tested setup uses
32 MiB RAM, `--cpu 6`, the OSR2 ISO, and its extracted El Torito boot floppy:

```bash
# From the repository root; supply your own Windows 95 installation media.
./desktop/frank386 \
  --fda "$HOME/frank-disks/win95-boot.img" \
  --hda "$HOME/frank-disks/win95-500.img" \
  --cda "/path/to/Win95 OSR2.ISO" \
  -m 32 --cpu 6
```

Keep the boot floppy attached while restarting Setup; the newly formatted HDD
may otherwise report `Invalid system disk`. Once Setup makes the HDD bootable,
use `--fda ""` to boot from it. Never run two emulator instances against the same
writable disk image.

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
