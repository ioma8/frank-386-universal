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

See `build.sh` for board and USB options. The default board is M2. This tree
includes upstream firmware v1.08.

### HDMI

HDMI output is real HDMI (not DVI): data islands, AVI and audio InfoFrames, and
44.1 kHz stereo. It works with USB HDMI capture cards as well as monitors and
TVs. AdLib, Sound Blaster, the PC speaker, and the other sound devices play
over I2S or PWM, and also over HDMI when HDMI is the output.

### PSRAM

The firmware expects 8MB QSPI PSRAM on the RP2350's second QSPI chip select
(XIP CS1: GPIO47 on RP2350B, GPIO19 or GPIO8 on RP2350A), sharing the bus with
the flash. PSRAM wired as a plain SPI device on other GPIOs is not supported.
If the part does not answer at the configured clock, bring-up retries slower
clocks. On the PC layout it also tries the other candidate chip select (GPIO8
or GPIO47) before giving up, which covers both the PICO-PC carrier and the
Olimex RP2350pc.

### Z2 USB

USB HID builds for Z2 run the USB host on the board's PIO-USB USB-C socket
(GPIO28/29, via [Pico-PIO-USB](https://github.com/sekigon-gonnoc/Pico-PIO-USB)).
Plug the keyboard, mouse, or a hub there. The native USB socket is then a USB
serial console (CDC), alongside a UART console on GPIO0 (TX) / GPIO1 (RX) at
115200 baud. That UART uses the Z2 PS/2 mouse pins, so PS/2 is dropped in a
USB HID build. Build with `-DZ2_PIO_USB=OFF` to use the native socket for the
host instead (no USB console then).

### Flash Quad Enable

Some non-Winbond flash parts (for example Puya on some Waveshare
RP2350-PiZero batches) ship with the Quad Enable bit cleared, which makes
PSRAM boards fail once overclocked. The firmware checks the bit on every boot
and sets it once if needed.

### Console commands

On the serial console (USB CDC or UART, 115200 baud):

| Command | Action |
|---------|--------|
| `~BOOT~` | Reboot into BOOTSEL, ready for `picotool load -x` |
| `~RST~` | Reboot |
| `~STAT~` | HDMI counters: frames, longest line IRQ, output restarts, HDMI audio underruns/drops |

Every software reboot stops core 1 and takes the PSRAM out of QPI mode first,
so the boot ROM can read the flash after a warm reset.

If the platform config file is missing, the firmware also reads the legacy
`386/config.ini` path.

## License

MIT. See [LICENSE](LICENSE). Upstream v1.06–v1.08 vendors
[Pico-PIO-USB](https://github.com/sekigon-gonnoc/Pico-PIO-USB) 0.7.2 (MIT,
`drivers/pio_usb`) and the HDMI packet encoder in `drivers/hdmi/hstx_packet.c`
(public domain; see `drivers/hdmi/LICENSE.pico_hdmi`). The RP2350 platform
code by DnCraptor included here is MIT; no GPL Quake or QuakeGeneric code is
used.
