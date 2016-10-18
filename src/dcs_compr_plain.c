#include <stdio.h>

typedef struct {
    FILE *fp;
} dcs_plain_ctx;

int dcs_plain_open(void *ctx, const char *file, const char *mode)
{
    dcs_plain_ctx  *zctx = ctx;

    if (ctx == NULL) return -1;

    zctx->fp = fopen(file, mode);
    if (zctx->fp == NULL) return -1;

    return 0;
}

int dcs_plain_dopen(void *ctx, const int fd, const char *mode)
{
    dcs_plain_ctx  *zctx = ctx;

    if (ctx == NULL) return -1;

    zctx->fp = fdopen(fd, mode);
    if (zctx->fp == NULL) return -1;

    return 0;
}

int dcs_plain_read(void *ctx, unsigned char *bytes, size_t *len, size_t cap)
{
    if (ctx == NULL || bytes == NULL || len == NULL || cap < 1) return -1;

    dcs_plain_ctx  *zctx = ctx;

    int res = fread(bytes, 1, cap, zctx->fp);
    if (ferror(zctx->fp) != 0) {
        *len = 0;
        return -1;
    }
    *len = res;
    return 0;
}

int dcs_plain_write(void *ctx, unsigned char *bytes, size_t len)
{
    if (ctx == NULL || bytes == NULL) return -1;

    dcs_plain_ctx  *zctx = ctx;

    fwrite(bytes, 1, len, zctx->fp);
    if (ferror(zctx->fp) != 0) {
        return -1;
    }
    return 0;
}

int dcs_plain_flush(void *ctx)
{
    if (ctx == NULL) return -1;

    dcs_plain_ctx  *zctx = ctx;
    int res = fflush(zctx->fp);
    if (res != 0) return -1;
    return 0;
}

int dcs_plain_close(void *ctx)
{
    if (ctx == NULL) return -1;

    dcs_plain_ctx  *zctx = ctx;
    int res = fclose(zctx->fp);
    if (res != 0) return -1;
    return 0;
}

