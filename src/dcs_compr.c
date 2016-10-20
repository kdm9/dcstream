#include "dcs_util.h"
#include "dcs_compr.h"

#include "dcs_compr_plain.c"
#include "dcs_compr_zlib.c"

inline int
dcs_compr_init(dcs_compr *compr, dcs_comp_algo algo)
{
    if (compr == NULL) return -1;

    switch (algo) {
        case DCS_GZIP:
            compr->ctx = malloc(sizeof(dcs_zlib_ctx));
            if (compr->ctx == NULL) return -1;
            compr->open = &dcs_zlib_open;
            compr->dopen = &dcs_zlib_dopen;
            compr->read = &dcs_zlib_read;
            compr->write = &dcs_zlib_write;
            compr->flush = &dcs_zlib_flush;
            compr->close = &dcs_zlib_close;
            compr->read = &dcs_zlib_read;
            break;
        case DCS_PLAIN:
            compr->ctx = malloc(sizeof(dcs_plain_ctx));
            if (compr->ctx == NULL) return -1;
            compr->open = &dcs_plain_open;
            compr->dopen = &dcs_plain_dopen;
            compr->read = &dcs_plain_read;
            compr->write = &dcs_plain_write;
            compr->flush = &dcs_plain_flush;
            compr->close = &dcs_plain_close;
            compr->read = &dcs_plain_read;
            break;
        default:
            return -1;
    }
    return 0;
}

dcs_compr *
dcs_compr_open(const char *file, const char *mode, dcs_comp_algo algo)
{
    dcs_compr *compr = calloc(1, sizeof(*compr));
    if (compr == NULL) return NULL;

    if (dcs_compr_init(compr, algo) != 0) return NULL;
    if (compr->open(compr->ctx, file, mode) != 0) return NULL;
    return compr;
}


dcs_compr *
dcs_compr_dopen(const int fd, const char *mode, dcs_comp_algo algo)
{
    dcs_compr *compr = calloc(1, sizeof(*compr));
    if (compr == NULL) return NULL;

    if (dcs_compr_init(compr, algo) != 0) return NULL;
    if (compr->dopen(compr->ctx, fd, mode) != 0) return NULL;
    return compr;
}

int dcs_compr_read(dcs_compr *compr, unsigned char *bytes, size_t *len, size_t cap)
{
    if (compr == NULL) return -1;

    return compr->read(compr->ctx, bytes, len, cap);
}

int dcs_compr_write(dcs_compr *compr, unsigned char *bytes, size_t len)
{
    if (compr == NULL) return -1;

    return compr->write(compr->ctx, bytes, len);
}

int dcs_compr_flush(dcs_compr *compr)
{
    if (compr == NULL) return -1;

    return compr->flush(compr->ctx);
}

int _dcs_compr_close(dcs_compr *compr)
{
    if (compr == NULL) return -1;

    int res = compr->close(compr->ctx);
    dcs_free(compr->ctx);
    dcs_free(compr);
    return res;
}
