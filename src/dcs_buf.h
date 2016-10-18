#ifndef DCS_BUF_H_KD9UNYLZ
#define DCS_BUF_H_KD9UNYLZ

#include <stdlib.h>
#include <stddef.h>
#include "dcs_util.h"
#include "dcs_compr.h"


typedef struct dcs_buffer_s {
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
} dcs_buf;

/* Allocates a buffer object
 * @capacity The buffer's capacity, in bytes
 *
 * @return A pointer to a buffer object, or NULL on failure
 */
dcs_buf *dcs_buf_create(size_t capacity);

/* Initialises a buffer object in-place
 * @buf A pointer to a buffer object, typically on the stack
 * @capacity The buffer's capacity, in bytes
 *
 * @return 0 on success, otherwise -1
 */
int dcs_buf_init(dcs_buf *buf, size_t capacity);


int dcs_buf_read(dcs_buf *buf, void *dest, size_t size);
int dcs_buf_write(dcs_buf *buf, const void *src, size_t size);

int dcs_buf_getc(dcs_buf *buf, unsigned char *chr);
int dcs_buf_putc(dcs_buf *buf, unsigned char chr);

int dcs_buf_getuntil(dcs_buf *buf, unsigned char *out, unsigned char end);
int dcs_buf_puts(dcs_buf *buf, const char *str);

int dcs_buf_flush(dcs_buf *buf);
int dcs_buf_fill(dcs_buf *buf);


/* Destroys internal members of a buffer object in-place
 * @buf A pointer to a buffer object
 *
 * @return 0 on success, otherwise -1
 */
void dcs_buf_deinit(dcs_buf *buf);

void _dcs_buf_free(dcs_buf *buf);
/* Frees and sets to NULL a pointer to a buffer object
 * @buf A pointer to a buffer object on the heap.
 *
 * Note that `buf` is set to NULL after freeing
 */
#define dcs_buf_free(buf) ({if (buf != NULL) _dcs_buf_free(buf); buf = NULL;})

#endif /* end of include guard: DCS_BUF_H_KD9UNYLZ */
