#ifndef DCS_COMPR_H_CRY3YU7D
#define DCS_COMPR_H_CRY3YU7D


#include "dcs_stream.h"

struct dcs_compr_s {
    void *ctx;
    int (*open)(void *ctx, const char *file, const char *mode);
    int (*dopen)(void *ctx, const int fd, const char *mode);
    int (*read)(void *ctx, unsigned char *bytes, size_t *len, size_t cap);
    int (*write)(void *ctx, unsigned char *bytes, size_t len);
    int (*flush)(void *ctx);
    int (*close)(void *ctx);
    dcs_comp_algo algo;
};

dcs_compr *dcs_compr_open(const char *file, const char *mode, dcs_comp_algo algo);
dcs_compr *dcs_compr_dopen(const int fd, const char *mode, dcs_comp_algo algo);
int dcs_compr_read(dcs_compr *compr, unsigned char *bytes, size_t *len, size_t cap);
int dcs_compr_write(dcs_compr *compr, unsigned char *bytes, size_t len);
int dcs_compr_flush(dcs_compr *compr);
int _dcs_compr_close(dcs_compr *compr);
#define dcs_compr_close(compr) __extension__ ({int res=0;                \
        if (compr != NULL){res = _dcs_compr_close(compr);} \
        compr = NULL; res;})


#endif /* end of include guard: DCS_COMPR_H_CRY3YU7D */
