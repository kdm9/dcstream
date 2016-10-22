#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "dcs_util.h"
#include "dcs_compr.h"
#include "dcs_stream.h"

#define TESTING_BUFSIZE (1<<10) // 1KiB

static char filename[4096] = "/dev/shm/test.dat";

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


void test_stream_readwrite_roundtrip(void **ctx)
{
    const size_t lengths[] = {1000, TESTING_BUFSIZE, TESTING_BUFSIZE + 10,
                              5 * TESTING_BUFSIZE};
    const size_t nlengths = sizeof(lengths) / sizeof(*lengths);

    const dcs_comp_algo algos[] = {DCS_PLAIN, DCS_GZIP};
    const size_t nalgos = sizeof(algos) / sizeof(*algos);

    for (size_t algoidx = 0; algoidx < nalgos; algoidx++) {
        dcs_comp_algo algo = algos[algoidx];
        for (size_t lengthidx = 0; lengthidx < nlengths; lengthidx++) {
            size_t length = lengths[lengthidx];

            char *orig = malloc(length + 1);
            assert_non_null(orig);
            orig[length] = '\0';

            for (size_t i = 0; i < length; i++) {
                orig[i] = '0' + (char)(i % 10);
            }

            char *roundtrip = malloc(length + 1);
            assert_non_null(roundtrip);
            roundtrip[length] = '\0';

            /***********
            *  Write  *
            ***********/
            // Open stream, check struct members initialised
            dcs_stream *stream = dcs_open(filename, "w", algo);
            assert_non_null(stream->compr);
            assert_non_null(stream->compr->ctx);
            assert_non_null(stream->buf);
            assert_false(stream->read);
            assert_int_equal(stream->cap, DCS_BUFSIZE);
            assert_int_equal(stream->pos, 0);
            assert_int_equal(stream->len, 0);

            // resize buffer, check that worked
            assert_int_equal(dcs_setbufsize(stream, TESTING_BUFSIZE), 0);
            assert_int_equal(stream->cap, TESTING_BUFSIZE);
            assert_non_null(stream->buf);

            // Write buffer
            assert_int_equal(dcs_write(stream, orig, length), length);

            // Close buffer
            assert_int_equal(dcs_close(stream), 0);
            assert_null(stream);


            /**********
            *  Read  *
            **********/
            // Open file for reading, check struct
            stream = dcs_open(filename, "r", algo);
            assert_non_null(stream->compr);
            assert_non_null(stream->compr->ctx);
            assert_true(stream->read);
            assert_int_equal(stream->cap, DCS_BUFSIZE);
            assert_int_equal(stream->pos, 0);
            assert_int_equal(stream->len, 0);

            // resize buffer, check that worked
            assert_int_equal(dcs_setbufsize(stream, TESTING_BUFSIZE), 0);
            assert_int_equal(stream->cap, TESTING_BUFSIZE);
            assert_non_null(stream->buf);

            assert_int_equal(dcs_read(stream, roundtrip, length), length);
            assert_int_equal(dcs_eof(stream), 1);
            assert_true(roundtrip[length] == '\0');
            assert_int_equal(strlen(roundtrip), length);
            assert_string_equal(roundtrip, orig);

            assert_int_equal(dcs_close(stream), 0);
            assert_null(stream);
            free(roundtrip);
            free(orig);
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


void test_stream_getc_ungetc(void **ctx)
{
    const size_t length = 10;
    dcs_stream *stream;

    assert_int_equal(mktestdat(length), 0);
    stream = dcs_open(filename, "r", DCS_PLAIN);
    assert_non_null(stream);

    assert_int_equal(stream->prevous_getc, -1);
    for (size_t i = 0; i < length; i++) {
        int expect = '0' + (char)(i % 10);

        // getc, check result
        assert_int_equal(dcs_getc(stream), expect);
        assert_int_equal(stream->pos, i + 1);
        assert_int_equal(stream->prevous_getc, expect);

        // Ungetc, should reset state
        assert_int_equal(dcs_ungetc(stream), 0);
        assert_int_equal(stream->pos, i);
        assert_int_equal(stream->prevous_getc, -1);

        // Ungetc, should fail
        assert_int_equal(dcs_ungetc(stream), -1);
        assert_int_equal(stream->pos, i);

        // re-get c to advance
        assert_int_equal(dcs_getc(stream), expect);
        assert_int_equal(stream->pos, i + 1);
    }
    // Read should fail, without advancing
    assert_int_equal(stream->pos, stream->len);
    assert_int_equal(dcs_getc(stream), -1);
    assert_int_equal(stream->pos, stream->len);
}


const struct CMUnitTest suite_stream[] = {
    cmocka_unit_test_teardown(test_stream_readwrite_roundtrip, remove_testfile),
    cmocka_unit_test_teardown(test_stream_bad_read, remove_testfile),
    cmocka_unit_test_teardown(test_stream_getc_ungetc, remove_testfile),
};
