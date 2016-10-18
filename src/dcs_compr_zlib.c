#include <zlib.h>

typedef struct {
    gzFile fp;
} dcs_zlib_ctx;


int dcs_zlib_open(void *ctx, const char *file, const char *mode)
{
    dcs_zlib_ctx  *zctx = ctx;

    if (ctx == NULL) return -1;

    zctx->fp = gzopen(file, mode);
    if (zctx->fp == NULL) return -1;

#ifdef HAVE_GZBUFFER
    return gzbuffer(zctx->fp, 1048576);
#else
    return 0;
#endif
}

int dcs_zlib_dopen(void *ctx, const int fd, const char *mode)
{
    dcs_zlib_ctx  *zctx = ctx;

    if (ctx == NULL) return -1;

    zctx->fp = gzdopen(fd, mode);
    if (zctx->fp == NULL) return -1;

#ifdef HAVE_GZBUFFER
    return gzbuffer(zctx->fp, 1048576);
#else
    return 0;
#endif
}


int dcs_zlib_read(void *ctx, unsigned char *bytes, size_t *len, size_t cap)
{
    if (ctx == NULL || bytes == NULL || len == NULL || cap < 1) return -1;

    dcs_zlib_ctx  *zctx = ctx;

    int res = gzread(zctx->fp, bytes, cap);
    if (res < 0) {
        *len = 0;
        return -1;
    }
    // TODO: handle errors here with gzerror

    *len = res;
    return 0;
}

int dcs_zlib_write(void *ctx, unsigned char *bytes, size_t len)
{
    if (ctx == NULL || bytes == NULL) return -1;

    dcs_zlib_ctx  *zctx = ctx;

    int res = gzwrite(zctx->fp, bytes, len);
    if (res == 0) {
        return -1;
    }
    // TODO: handle errors here with gzerror
    return 0;
}

int dcs_zlib_flush(void *ctx)
{
    if (ctx == NULL) return -1;

    dcs_zlib_ctx  *zctx = ctx;
    int res = gzflush(zctx->fp, Z_SYNC_FLUSH);
    if (res != Z_OK) return -1;
    return 0;
}

int dcs_zlib_close(void *ctx)
{
    if (ctx == NULL) return -1;

    dcs_zlib_ctx  *zctx = ctx;
    int res = gzclose(zctx->fp);
    if (res != Z_OK) return -1;
    return 0;
}

