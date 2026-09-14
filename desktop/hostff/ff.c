/* Host build stand-in for FatFs — see hostff/ff.h for why. */
#include "ff.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* FatFs paths are '/'-rooted, optionally "N:/". Map them under FRANK_ROOT.
 *
 * disk.c prefixes every disk image name with "386/", so an absolute
 * host path passed by the user arrives here as "386//abs/path"; that
 * case is detected and used verbatim. */
static void resolve(char *out, size_t outsz, const TCHAR *path)
{
	const char *root = getenv("FRANK_ROOT");
	if (!root || !*root) root = ".";
	if (!path) path = "";

	if (path[0] && path[1] == ':') path += 2;   /* drive prefix "0:" */
	if (strncmp(path, "386//", 5) == 0) path += 4;
	if (path[0] == '/') { snprintf(out, outsz, "%s", path); return; }
	while (*path == '/' || *path == '\\') path++;
	snprintf(out, outsz, "%s/%s", root, path);
}

/* The disk layer only ever needs the volume object to exist. */
FATFS fs;

FSIZE_t f_size(FIL *fp)
{
	if (!fp || !fp->fp) return 0;
	long cur = ftell(fp->fp);
	if (cur < 0) return 0;
	if (fseek(fp->fp, 0, SEEK_END) != 0) return 0;
	long end = ftell(fp->fp);
	fseek(fp->fp, cur, SEEK_SET);
	return end < 0 ? 0 : (FSIZE_t)end;
}

FRESULT f_open(FIL *fp, const TCHAR *path, BYTE mode)
{
	if (!fp) return FR_INVALID_PARAMETER;
	memset(fp, 0, sizeof(*fp));
	fp->readonly = (mode & FA_WRITE) == 0;
	resolve(fp->path, sizeof(fp->path), path);

	const char *fm;
	if (mode & FA_CREATE_ALWAYS) {
		fm = (mode & FA_READ) ? "wb+" : "wb";
	} else if (mode & FA_CREATE_NEW) {
		fm = (mode & FA_READ) ? "w+b" : "wb";
	} else if (mode & FA_OPEN_APPEND) {
		fm = (mode & FA_READ) ? "a+b" : "ab";
	} else if (mode & FA_OPEN_ALWAYS) {
		fm = (mode & FA_WRITE) ? "r+b" : "rb";
	} else if (mode & FA_WRITE) {
		fm = "r+b";
	} else {
		fm = "rb";
	}

	fp->fp = fopen(fp->path, fm);
	/* FA_OPEN_ALWAYS means "create if missing" */
	if (!fp->fp && (mode & FA_OPEN_ALWAYS) && (mode & FA_WRITE))
		fp->fp = fopen(fp->path, "w+b");
	if (!fp->fp) return FR_NO_FILE;
	fp->obj.fs = fp->fp;   /* disk.c tests this to see if a file is open */
	return FR_OK;
}

FRESULT f_close(FIL *fp)
{
	if (!fp || !fp->fp) return FR_INVALID_OBJECT;
	fclose(fp->fp);
	fp->fp = NULL;
	fp->obj.fs = NULL;
	return FR_OK;
}

FRESULT f_read(FIL *fp, void *buff, UINT btr, UINT *br)
{
	if (br) *br = 0;
	if (!fp || !fp->fp) return FR_INVALID_OBJECT;
	size_t n = fread(buff, 1, btr, fp->fp);
	if (br) *br = (UINT)n;
	return FR_OK;
}

FRESULT f_write(FIL *fp, const void *buff, UINT btw, UINT *bw)
{
	if (bw) *bw = 0;
	if (!fp || !fp->fp) return FR_INVALID_OBJECT;
	if (fp->readonly) return FR_DENIED;
	size_t n = fwrite(buff, 1, btw, fp->fp);
	if (bw) *bw = (UINT)n;
	return n == btw ? FR_OK : FR_DISK_ERR;
}

FRESULT f_lseek(FIL *fp, FSIZE_t ofs)
{
	if (!fp || !fp->fp) return FR_INVALID_OBJECT;
	return fseeko(fp->fp, (off_t)ofs, SEEK_SET) == 0 ? FR_OK : FR_DISK_ERR;
}

FRESULT f_sync(FIL *fp)
{
	if (!fp || !fp->fp) return FR_INVALID_OBJECT;
	return fflush(fp->fp) == 0 ? FR_OK : FR_DISK_ERR;
}
