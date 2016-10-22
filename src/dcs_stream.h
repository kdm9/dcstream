#ifndef DCS_STREAM_H_SXLTXFQZ
#define DCS_STREAM_H_SXLTXFQZ

#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>


typedef struct dcs_compr_s dcs_compr;

typedef enum dcs_comp_algo_e{
    DCS_UNKNOWN = 0,
    // Uncompressed
    DCS_PLAIN,
    // Compressed using Zstandard version 1.0 or greater
    DCS_ZSTD,
    // Compressed using gzip, or the zlib library
    DCS_GZIP,
    // Compressed using bzip2, or the bzip2 library
    DCS_BZIP2
} dcs_comp_algo;


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
    dcs_compr *compr;
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

/* Close `stream`, destroying all data strucutres
 */
int _dcs_close(dcs_stream *stream);
#define dcs_close(st) ({int res=0; if (st != NULL){res = _dcs_close(st);} st = NULL; res;})

/* Sets the buffer size of `stream` to `size`.
 *
 * @stream Open stream whose buffer should be resized.
 * @size New buffer size
 *
 * @return 0 on success, or -1 on error. Existing buffers are preserved if
 *         resizing fails
 *
 * **Note:** This function must be called immediately after `open`, before the
 * buffer has been filled.
 */
int dcs_setbufsize(dcs_stream *stream, size_t size);


/* Read @size bytes from @stream into @dest
 *
 * @stream Stream from which to read
 * @dest Destination for content to be read
 * @size Number of bytes to read.
 *
 * @return Number of items read, or -1 on error;
 */
ssize_t dcs_read(dcs_stream *stream, void *dest, size_t size);

/* Write @size bytes from @src into @stream
 *
 * @stream Stream to be written to
 * @src Source for content to be written
 * @size Number of bytes to write.
 *
 * @return Number of items written, or -1 on error;
 */
ssize_t dcs_write(dcs_stream *stream, const void *src, size_t size);

int dcs_getc(dcs_stream *stream);
int dcs_ungetc(dcs_stream *stream);

ssize_t dcs_getuntil(dcs_stream *stream, unsigned char *out, unsigned char end, size_t cap);
int dcs_puts(dcs_stream *stream, const char *str);

int dcs_flush(dcs_stream *stream);

inline int
dcs_eof(dcs_stream *stream)
{
    if (stream == NULL || ! stream->read) return -1;
    if (stream->fp_eof && stream->pos == stream->len) return 1;
    else return 0;
}


#endif /* end of include guard: DCS_STREAM_H_SXLTXFQZ */
