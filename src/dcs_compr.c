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

int dcs_compr_open(dcs_compr *compr, const char *file, const char *mode, dcs_comp_algo algo)
{
    if (compr == NULL) return -1;

    if (dcs_compr_init(compr, algo) != 0) return -1;

    return compr->open(compr->ctx, file, mode);
}


int dcs_compr_dopen(dcs_compr *compr, const int fd, const char *mode, dcs_comp_algo algo)
{
    if (compr == NULL) return -1;

    if (dcs_compr_init(compr, algo) != 0) return -1;

    return compr->dopen(compr->ctx, fd, mode);
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

int dcs_compr_close(dcs_compr *compr)
{
    if (compr == NULL) return -1;

    return compr->close(compr->ctx);
}
