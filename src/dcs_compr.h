#ifndef DCS_COMPR_H_CRY3YU7D
#define DCS_COMPR_H_CRY3YU7D


#include "dcs_util.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>


typedef enum {
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

typedef struct dcs_compr_s {
    void *ctx;
    int (*open)(void *ctx, const char *file, const char *mode);
    int (*dopen)(void *ctx, const int fd, const char *mode);
    int (*read)(void *ctx, unsigned char *bytes, size_t *len, size_t cap);
    int (*write)(void *ctx, unsigned char *bytes, size_t len);
    int (*flush)(void *ctx);
    int (*close)(void *ctx);
} dcs_compr;

int dcs_compr_open(dcs_compr *compr, const char *file, const char *mode, dcs_comp_algo algo);
int dcs_compr_dopen(dcs_compr *compr, const int fd, const char *mode, dcs_comp_algo algo);
int dcs_compr_read(dcs_compr *compr, unsigned char *bytes, size_t *len, size_t cap);
int dcs_compr_write(dcs_compr *compr, unsigned char *bytes, size_t len);
int dcs_compr_flush(dcs_compr *compr);
int dcs_compr_close(dcs_compr *compr);


#endif /* end of include guard: DCS_COMPR_H_CRY3YU7D */
