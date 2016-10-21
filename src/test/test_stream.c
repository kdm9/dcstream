#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "dcs_util.h"
#include "dcs_compr.h"
#include "dcs_stream.h"

static const char filename[] = "/dev/shm/test.dat";

int
mktestdat(size_t len)
{
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) return -1;

    for (size_t i = 0; i < len; i++) {
        fprintf(fp, "%zu", i % 10);
    }
    fclose(fp);
    return 0;
}

int remove_testfile(void **ctx)
{
    return remove(filename);
}

/*******************************************************************************
*                                    TESTS                                    *
*******************************************************************************/


void test_stream_read(void **ctx)
{
    const size_t lengths[] = {1000, DCS_BUFSIZE, DCS_BUFSIZE + 1000};
    const size_t nlengths = sizeof(lengths) / sizeof(*lengths);

    const dcs_comp_algo algos[] = {DCS_UNKNOWN, DCS_PLAIN, DCS_GZIP};
    const size_t nalgos = sizeof(algos) / sizeof(*algos);

    for (size_t algoidx = 0; algoidx < nalgos; algoidx++) {
        dcs_comp_algo algo = algos[algoidx];
        for (size_t lengthidx = 0; lengthidx < nlengths; lengthidx++) {
            size_t length = lengths[lengthidx];

            assert_int_equal(mktestdat(length), 0);

            dcs_stream *stream = dcs_open(filename, "r", algo);
            assert_non_null(stream->compr);
            assert_non_null(stream->compr->ctx);
            assert_true(stream->read);
            assert_int_equal(stream->cap, DCS_BUFSIZE);
            assert_int_equal(stream->pos, 0);
            assert_int_equal(stream->len, dcs_size_min(length,  DCS_BUFSIZE));

            char *buf = malloc(length + 1);
            assert_non_null(buf);
            buf[length] = '\0';
            assert_int_equal(dcs_read(stream, buf, length), length);
            assert_int_equal(dcs_eof(stream), 1);
            assert_true(buf[length] == '\0');
            assert_int_equal(strlen(buf), length);

            for (size_t i = 0; i < length; i++) {
                assert_true(buf[i] == '0' + (char)(i % 10));
            }
            assert_int_equal(dcs_close(stream), 0);
            assert_null(stream);
            free(buf);
        }
    }
}

void test_stream_bad_read(void **ctx)
{
    const size_t length = 100;

    char *buf = malloc(length + 1);
    assert_non_null(buf);
    buf[length] = '\0';

    // Read on write file
    dcs_stream *stream = dcs_open(filename, "w", DCS_PLAIN);
    assert_false(stream->read);
    assert_int_equal(dcs_read(stream, buf, length), -1);
    assert_int_equal(dcs_close(stream), 0);

    // Read longer than the file length
    assert_int_equal(mktestdat(length * 1.5), 0);
    stream = dcs_open(filename, "r", DCS_PLAIN);
    assert_int_equal(dcs_read(stream, buf, length), length);
    assert_int_equal(dcs_eof(stream), 0);
    assert_int_equal(dcs_read(stream, buf, length), length * 0.5);
    assert_int_equal(dcs_eof(stream), 1);
    assert_int_equal(dcs_close(stream), 0);

    free(buf);
}


void test_stream_write(void **ctx)
{

    //const size_t lengths[] = {1000, DCS_BUFSIZE, DCS_BUFSIZE + 1000};
    const size_t lengths[] = {1000, DCS_BUFSIZE, DCS_BUFSIZE + 1000};
    const size_t nlengths = sizeof(lengths) / sizeof(*lengths);

    const dcs_comp_algo algos[] = {DCS_UNKNOWN, DCS_PLAIN, DCS_GZIP};
    const size_t nalgos = sizeof(algos) / sizeof(*algos);

    for (size_t algoidx = 0; algoidx < nalgos; algoidx++) {
        dcs_comp_algo algo = algos[algoidx];

        for (size_t lengthidx = 0; lengthidx < nlengths; lengthidx++) {
            size_t length = lengths[lengthidx];

            assert_int_equal(mktestdat(length), 0);

            dcs_stream *stream = dcs_open(filename, "w", algo);
            assert_non_null(stream->compr);
            assert_non_null(stream->compr->ctx);
            assert_false(stream->read);
            assert_int_equal(stream->cap, DCS_BUFSIZE);
            assert_int_equal(stream->pos, 0);
            assert_int_equal(stream->len, 0);

            char *buf = malloc(length + 1);
            assert_non_null(buf);
            buf[length] = '\0';
            for (size_t i = 0; i < length; i++) {
                buf[i] = '0' + (char)(i % 10);
            }

            assert_int_equal(dcs_write(stream, buf, length), length);

            for (size_t i = 0; i < length; i++) {
                assert_true(buf[i] == '0' + (char)(i % 10));
            }
            assert_int_equal(dcs_close(stream), 0);
            assert_null(stream);
            free(buf);
        }
    }
}
