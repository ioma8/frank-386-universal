# Windows 95 installation status

## Current result

Windows 95 OSR2 boots and reaches the graphical Setup license agreement. **Installation is not complete, and an installed Windows desktop has not been verified.**

The latest emulator process was left running. A `y` keystroke was sent using macOS System Events, but its effect has not been verified.

## Confirmed findings

- The hard-disk image limit in `src/disk.c` is `0x1f782000` bytes: **503.5078125 MiB**, not 527 MiB. Both the original 1 GiB image and the replacement 512 MiB image exceed it.
- A separate **500 MiB** image is accepted. Setup successfully wrote an MBR with signature `55aa`; this resolves the previously observed absence of disk writes for this image.
- Graphical Setup renders correctly. The earlier blank screen occurred around Setup's reboot; it did not establish a VGA defect.
- After partitioning/formatting, booting without a floppy reached `Invalid system disk` on the HDD rather than restarting CD Setup.
- Extracting the ISO's El Torito 1.44 MB boot image and supplying it as `--fda` successfully returns to graphical Setup.

## Local files

- ISO: `/Users/jakubkolcar/Downloads/Win95 OSR2(1).ISO`
- Working installation disk: `/Users/jakubkolcar/frank-disks/win95-500.img`
- Extracted boot floppy: `/Users/jakubkolcar/frank-disks/win95-boot.img`
- Previous rejected 512 MiB disk: `/Users/jakubkolcar/frank-disks/win95.img`
- Latest run log: `/tmp/w95-floppy.log`
- Latest run PID file: `/tmp/w95-floppy.pid`
- Verified graphical license-screen capture: `/tmp/w95-floppy.bmp`
- Subsequent host screenshot, not yet inspected: `/tmp/w95-screen.png`

Do not recreate or truncate the working installation disk when resuming.

## Code changes in this checkpoint

- `src/pc.c`: attach an IDE HDD only when `insertdisk()` succeeds, avoiding attachment of a closed file after image rejection.
- `src/ide.c`: added an IRQ assertion when starting a sector write. **Its necessity has not been isolated or regression-tested**; successful installation progress with a valid disk does not prove this change is required.
- Temporary FF/VGA tracing was removed before the latest runs.
- Desktop build succeeds. This checkpoint records installation progress, not a verified Windows 95 compatibility release.

## Build and run

```sh
make -C desktop -j8
./desktop/frank386 \
  --fda "$HOME/frank-disks/win95-boot.img" \
  --hda "$HOME/frank-disks/win95-500.img" \
  --cda '/Users/jakubkolcar/Downloads/Win95 OSR2(1).ISO' \
  -m 32 --cpu 6
```

Check for the already-running process before launching another instance against the same writable disk.

## Next steps

1. Inspect the running Setup window and continue from the license agreement. macOS System Events can send keystrokes to the `frank386` process; the command returned successfully, but guest reception remains unverified.
2. Complete Setup using the user's supplied product key. The key is not recorded here and is not available in the compacted conversation context.
3. Remove the boot floppy for the installed-system reboot, retaining the CD if Setup still needs installation files.
4. Verify a Windows 95 desktop and a subsequent successful cold boot from the HDD.
5. Isolate whether the IDE IRQ change is needed; retain only justified fixes and add a regression check for rejected-disk attachment.

Remaining CPU/device compatibility issues are **unknown**, not confirmed. Do not claim Windows 95 is fully working until installation and HDD boot are verified.
