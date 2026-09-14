/*
 * Host build stand-in for FatFs (ff.h).
 *
 * The disk layer (disk.c/ide.c/fdd.c) was rewritten for the RP2350 to go
 * through FatFs on an SD card. On a desktop there is no SD card — images
 * are ordinary files — so this header maps the small FatFs surface the
 * disk layer actually uses onto POSIX stdio.
 *
 * Paths are resolved relative to the root directory named by the
 * FRANK_ROOT environment variable (default "."). So a guest image at
 * "386/dos622.img" is opened as "$FRANK_ROOT/386/dos622.img".
 */
#ifndef _HOST_FF_H
#define _HOST_FF_H

#include <stdint.h>
#include <stdio.h>

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef uint64_t       QWORD;
typedef unsigned int   UINT;
typedef char           TCHAR;
typedef uint64_t       FSIZE_t;
typedef uint64_t       LBA_t;

typedef enum {
	FR_OK = 0,
	FR_DISK_ERR, FR_INT_ERR, FR_NOT_READY, FR_NO_FILE, FR_NO_PATH,
	FR_INVALID_NAME, FR_DENIED, FR_EXIST, FR_INVALID_OBJECT,
	FR_WRITE_PROTECTED, FR_INVALID_DRIVE, FR_NOT_ENABLED,
	FR_NO_FILESYSTEM, FR_MKFS_ABORTED, FR_TIMEOUT, FR_LOCKED,
	FR_NOT_ENOUGH_CORE, FR_TOO_MANY_OPEN_FILES, FR_INVALID_PARAMETER
} FRESULT;

/* Open mode flags — same values as FatFs so call sites are unchanged */
#define FA_READ          0x01
#define FA_WRITE         0x02
#define FA_OPEN_EXISTING 0x00
#define FA_CREATE_NEW    0x04
#define FA_CREATE_ALWAYS 0x08
#define FA_OPEN_ALWAYS   0x10
#define FA_OPEN_APPEND   0x30

typedef struct {
	void *fs;            /* non-NULL while the file is open */
} FFOBJID;

typedef struct FIL_s {
	FFOBJID obj;
	FILE  *fp;
	char   path[1024];   /* resolved host path */
	int    readonly;
} FIL;

typedef struct {
	int dummy;
} FATFS;

/* FatFs exposes f_size as a macro over the FIL internals; here it is a
 * real function so the struct layout stays private. */
FSIZE_t f_size(FIL *fp);

FRESULT f_open(FIL *fp, const TCHAR *path, BYTE mode);
FRESULT f_close(FIL *fp);
FRESULT f_read(FIL *fp, void *buff, UINT btr, UINT *br);
FRESULT f_write(FIL *fp, const void *buff, UINT btw, UINT *bw);
FRESULT f_lseek(FIL *fp, FSIZE_t ofs);
FRESULT f_sync(FIL *fp);

#endif /* _HOST_FF_H */
