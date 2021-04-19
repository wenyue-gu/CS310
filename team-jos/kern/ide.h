#include <inc/types.h>
#include <inc/assert.h>

#define SECTSIZE 512 // bytes per disk sector
#define MAX_RW  255 // max number of sectors that can be read in a single chunk
#define FS_START_SECTOR 2000 // be sure you give enough space for kernel

/* ide.c */
int	ide_read(uint32_t secno, void *dst, size_t nsecs);
int	ide_write(uint32_t secno, const void *src, size_t nsecs);

