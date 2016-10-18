#include "dcs_buf.h"


dcs_buf *dcs_buf_create(size_t capacity)
{
    dcs_buf *buf = malloc(sizeof(*buf));
    if (buf == NULL) return NULL;

    int res = dcs_buf_init(buf, capacity);
    if (res != 0) {
        dcs_free(buf);
        return NULL;
    }
    return buf;
}

int dcs_buf_init(dcs_buf *buf, size_t capacity)
{
    if (buf == NULL || capacity < 1L<<20) return -1;

    buf->buf = calloc(capacity, 1);
    if (buf->buf == NULL) {
        return -1;
    }
    buf->cap = capacity;
    buf->len = 0;
    buf->pos = 0;
    buf->prevous_getc = -1;
    return 0;
}

void dcs_buf_deinit(dcs_buf *buf)
{
    if (buf == NULL) return;
    dcs_free(buf->buf);
    buf->cap = 0;
    buf->len = 0;
    buf->pos = 0;
}

void _dcs_buf_free(dcs_buf *buf)
{
    dcs_buf_deinit(buf);
    dcs_free(buf);
}
