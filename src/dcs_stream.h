#ifndef DCS_STREAM_H_SXLTXFQZ
#define DCS_STREAM_H_SXLTXFQZ

#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include "dcs_compr.h"

#ifndef DCS_BUFSIZE
#define DCS_BUFSIZE (1<<20) // 1Mib
#endif

typedef struct {
    // buffer content
    unsigned char *buf;
    // capacity of buffer
    size_t cap;
    // current length of content in buffer
    size_t len;
    // current position of cursor within buffer
    size_t pos;
    // Last char obtained via getc
    int prevous_getc;
    // Is the underlying fp at eof?
    bool fp_eof;
    // File is reading. False for writing;
    bool read;
    // Compressed source/sink
    dcs_compr compr;
} dcs_stream;


/* Open @file in mode @mode
 *
 * @file Filename to open. 
 * @mode File mode. Must be "r" or "w". Any trailing characters ignored, but are
 *       passed directly to the underlying IO/compression library's `open`.
 * @algo Compression algo. When reading, pass DCS_UNKNOWN to guess compression
 *       type from file extension. Must not be DCS_UNKNOWN if writing.
 */
dcs_stream *dcs_open(const char *file, const char *mode, dcs_comp_algo algo);


/* Open @file in mode @mode
 *
 * @file Filename to open. 
 * @mode File mode. Must be "r" or "w". Any trailing characters ignored, but are
 *       passed directly to the underlying IO/compression library's `open`.
 * @algo Compression algo. If DCS_UNKNOWN is given, no detection is attempted
 *       and DCS_PLAIN is assumed. 
 */
dcs_stream *dcs_dopen(int fd, const char *mode, dcs_comp_algo algo);

/* Close @dcs, destroying all data strucutres
 */
int _dcs_close(dcs_stream *stream);
#define dcs_close(st) ({int res=0; if (st != NULL){res = _dcs_close(st);} st = NULL; res;})


int dcs_read(dcs_stream *stream, void *dest, size_t size);
int dcs_write(dcs_stream *stream, const void *src, size_t size);

int dcs_getc(dcs_stream *stream);
int dcs_putc(dcs_stream *stream, unsigned char chr);

ssize_t dcs_getuntil(dcs_stream *stream, unsigned char *out, unsigned char end, size_t cap);
int dcs_puts(dcs_stream *stream, const char *str);

int dcs_flush(dcs_stream *stream);






#endif /* end of include guard: DCS_STREAM_H_SXLTXFQZ */
