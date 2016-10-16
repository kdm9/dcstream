#include "dcs_buf.h"

dcs_buf *dcs_buf_create(size_t capacity)
{
    return NULL;
}

int dcs_buf_init(dcs_buf *buf, size_t capacity)
{
    return -1;
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
