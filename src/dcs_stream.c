#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>

#include "dcs_util.h"
#include "dcs_compr.h"
#include "dcs_stream.h"


/*******************************************************************************
*                             Helper declarations                             *
*******************************************************************************/

int dcs_fileext(const char *filename, char *extbuf, size_t extbuflen);
dcs_comp_algo dcs_guess_compression_type(const char *filename);


/*******************************************************************************
*                              Stream Open/Close                              *
*******************************************************************************/

static inline int
_dcs_fillbuf(dcs_stream *stream)
{
    if (stream == NULL || !stream->read) return -1;
    int res = dcs_compr_read(stream->compr, stream->buf, &stream->len, stream->cap);
    if (stream->len < stream->cap) {
        stream->fp_eof = true;
    }
    stream->pos = 0;
    stream->prevous_getc = -1; // ungetc now impossible
    return res;
}

static inline int
_dcs_writebuf(dcs_stream *stream)
{
    if (stream == NULL) return -1;
    int res = dcs_compr_write(stream->compr, stream->buf, stream->len);
    stream->len = 0;
    stream->pos = 0;
    return res;
}

static inline dcs_stream *
_dcs_init(const char *mode)
{
    bool read = true;
    if (mode == NULL) return NULL;
    if (mode[0] == 'r') read = true;
    else if (mode[0] == 'w') read = false;
    else return NULL;

    dcs_stream *stream = malloc(sizeof(*stream));
    if (stream == NULL) return NULL;

    stream->buf = calloc(1, DCS_BUFSIZE);
    if (stream->buf == NULL) {
        dcs_free(stream);
        return NULL;
    }
    stream->len = 0;
    stream->pos = 0;
    stream->cap = DCS_BUFSIZE;
    stream->prevous_getc = -1;
    stream->fp_eof = false;
    stream->read = read;
    return stream;
}


dcs_stream *dcs_open(const char *file, const char *mode, dcs_comp_algo algo)
{
    if (file == NULL || mode == NULL) return NULL;

    // Guess compression type. If still unknown, bail.
    if (algo == DCS_UNKNOWN) algo = dcs_guess_compression_type(file);
    if (algo == DCS_UNKNOWN) return NULL;

    dcs_stream *stream = _dcs_init(mode);

    dcs_compr *compr = dcs_compr_open(file, mode, algo);
    if (compr == NULL) {
        dcs_free(stream->buf);
        dcs_free(stream);
        return NULL;
    }
    stream->compr = compr;
    return stream;
}


/* Open @file in mode @mode
 *
 * @file Filename to open.
 * @mode File mode. Must be "r" or "w". Any trailing characters ignored, but are
 *       passed directly to the underlying IO/compression library's `open`.
 * @algo Compression algo. If DCS_UNKNOWN is given, no detection is attempted
 *       and DCS_PLAIN is assumed.
 */
dcs_stream *dcs_dopen(int fd, const char *mode, dcs_comp_algo algo)
{
    if (mode == NULL || algo == DCS_UNKNOWN) return NULL;

    dcs_stream *stream = _dcs_init(mode);

    dcs_compr *compr = dcs_compr_dopen(fd, mode, algo);
    if (compr == NULL) {
        dcs_free(stream->buf);
        dcs_free(stream);
        return NULL;
    }
    stream->compr = compr;

    return stream;
}

/* Close @dcs, destroying all data strucutres
 */
int _dcs_close(dcs_stream *stream)
{
    if (stream == NULL) return -1;

    if (stream->len > 0) {
        _dcs_writebuf(stream);
    }
    int res = dcs_compr_close(stream->compr);

    dcs_free(stream->buf);
    dcs_free(stream);
    return res;
}

int dcs_setbufsize(dcs_stream *stream, size_t size)
{
    if (stream == NULL || size == 0) return -1;

    // If buffer has been filled, exit
    if (stream->pos != 0 || stream->len != 0 || stream->fp_eof) return -1;

    unsigned char *newbuf = calloc(1, size);
    if (newbuf == NULL) return -1;

    dcs_free(stream->buf);
    stream->buf = newbuf;
    stream->cap = size;

    return 0;
}

/*******************************************************************************
*                               Read and Write                                *
*******************************************************************************/

ssize_t dcs_read(dcs_stream *stream, void *dest, size_t size)
{
    if (stream == NULL || dest == NULL || ! stream->read) return -1;

    size_t read = 0;
    uint8_t *bdest = dest;
    if (stream->pos == stream->len) {
        if (_dcs_fillbuf(stream) != 0) return -1;
    }
    for (;read < size && stream->pos != stream->len;) {
        size_t tocpy = dcs_size_min(stream->len - stream->pos, size - read);
        memcpy(bdest + read, stream->buf + stream->pos, tocpy);
        stream->pos += tocpy;
        read += tocpy;
        if (stream->pos == stream->len) {
            if (_dcs_fillbuf(stream) != 0) return -1;
        }
    }
    stream->prevous_getc = -1;
    return read;
}

ssize_t dcs_write(dcs_stream *stream, const void *src, size_t size)
{
    if (stream == NULL || src == NULL || stream->read) return -1;

    size_t wrote = 0;
    int res = 0;
    const uint8_t *bsrc = src;
    for (; wrote < size; ) {
        size_t tocpy = dcs_size_min(stream->cap - stream->pos, size - wrote);
        memcpy(stream->buf + stream->pos, bsrc + wrote, tocpy);
        stream->pos += tocpy;
        stream->len = stream->pos;
        wrote += tocpy;
        if (stream->pos == stream->len) {
            res = _dcs_writebuf(stream);
            if (res != 0) {
                return -1;
            }
        }
    }
    return wrote;
}


int dcs_getc(dcs_stream *stream)
{
    if (stream == NULL || !stream->read) return -1;
    if (stream->pos == stream->len) {
        // refill buffer if empty
        if (_dcs_fillbuf(stream) != 0) return -1;
    }
    if (stream->pos < stream->len) {
        int chr = stream->buf[stream->pos++];
        stream->prevous_getc = chr;
        return chr;
    }
    return -1;
}

int dcs_ungetc(dcs_stream *stream)
{
    if (stream == NULL || !stream->read) return -1;
    if (stream->prevous_getc < 0) return -1;
    if (stream->buf[stream->pos - 1] == stream->prevous_getc) {
        stream->pos--;
        stream->prevous_getc = -1;
        return 0;
    }
    return -1;
}


/*******************************************************************************
*                                   Helpers                                   *
*******************************************************************************/

int
dcs_fileext(const char *filename, char *extbuf, size_t extbuflen)
{
    if (filename == NULL || extbuf == NULL || extbuflen == 0) return -1;

    char *lastdot = strrchr(filename, '.');
    if (lastdot == NULL) {
        // No extension
        strncpy(extbuf, "", extbuflen);
    } else {
        strncpy(extbuf, lastdot, extbuflen);
    }
    return 0;
}

dcs_comp_algo
dcs_guess_compression_type(const char *filename)
{
    int res = 0;

    // If file is stdin, we only support plain file IO
    if (strcmp(filename, "-") == 0 || strcmp(filename, "/dev/stdin") == 0) {
        return DCS_PLAIN;
    }

    // Stat file
    struct stat statres;
    res = stat(filename, &statres);
    if (res == 0) {
        // What, someone gave us a directory?
        if (S_ISDIR(statres.st_mode)) {
            return DCS_UNKNOWN;
        }

        // If file is a stream or socket, we only support plain IO
        if (S_ISFIFO(statres.st_mode) || S_ISSOCK(statres.st_mode)) {
            return DCS_PLAIN;
        }
    }

    // Get the file extension
    char extbuf[4096] = "";
    res = dcs_fileext(filename, extbuf, 4096);
    if (res != 0) return DCS_UNKNOWN;

    // Guess the file type from the extension. Yes, I'm that lazy right now.
    if (strcmp(extbuf, ".gz") == 0) {
        return DCS_GZIP;
    } else if (strcmp(extbuf, ".bz2") == 0) {
        return DCS_BZIP2;
    } else if (strcmp(extbuf, ".zst") == 0) {
        return DCS_ZSTD;
    } else {
        return DCS_PLAIN;
    }
}
