#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <error.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cmocka.h>

#include "dcs_util.h"
#include "dcs_compr.h"
#include "dcs_stream.h"

#define TESTING_BUFSIZE (1<<10) // 1KiB

static const char filename[4096] = "/dev/shm/test.dat";
static const char textline[100] = "A line of text %zu\n";

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

int
mktesttext(size_t lines)
{
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) return -1;

    for (size_t i = 0; i < lines; i++) {
        fprintf(fp, textline, i % 10);
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
            assert_non_null(stream);
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
            assert_true(roundtrip[length] == '\0');
            assert_int_equal(strlen(roundtrip), length);
            assert_string_equal(roundtrip, orig);

            // Try reading again, should fail & set eof
            assert_int_equal(dcs_read(stream, roundtrip, length), 0);
            assert_int_equal(dcs_eof(stream), 1);

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

void test_stream_getuntil(void **ctx)
{
    dcs_stream *stream;
    ssize_t res = 0;
    const size_t numlines = TESTING_BUFSIZE;
    const unsigned char delim = '\n';

    size_t bufsize = TESTING_BUFSIZE;
    char *buf = calloc(1, bufsize + 1);
    char *expect = calloc(1, bufsize + 1);
    assert_non_null(buf);
    assert_non_null(expect);

    res = mktesttext(numlines);
    assert_int_equal(res, 0);

    stream = dcs_open(filename, "r", DCS_PLAIN);
    assert_non_null(stream);
    dcs_setbufsize(stream, TESTING_BUFSIZE);

    for (size_t l = 0; l < numlines; l++) {
        snprintf(expect, bufsize, textline, l % 10);
        res = dcs_getuntil(stream, &buf, &bufsize, delim);
        assert_string_equal(buf, expect);
        assert_int_equal(res, strlen(expect));
        assert_non_null(buf);
        assert_true(bufsize >= TESTING_BUFSIZE);
        assert_int_equal(buf[res-1], delim);
        assert_int_equal(buf[res], 0);
    }

    res = dcs_getuntil(stream, &buf, &bufsize, delim);
    assert_int_equal(res, 0);
    assert_int_equal(dcs_eof(stream), 1);
    // These should not have been changed
    assert_non_null(buf);
    assert_true(bufsize >= TESTING_BUFSIZE);
    assert_string_equal(buf, expect);

    assert_int_equal(dcs_eof(stream), 1);
    assert_int_equal(dcs_close(stream), 0);
    assert_null(stream);
    free(buf);
    free(expect);
}

void test_stream_getuntil_bigline(void **ctx)
{
    dcs_stream *stream;
    ssize_t res = 0;
    const unsigned char delim = '\n';

    size_t bufsize = TESTING_BUFSIZE * 2 + TESTING_BUFSIZE - 1;
    char *buf = calloc(1, bufsize + 1);
    char *expect = calloc(1, bufsize + 1);
    assert_non_null(buf);
    assert_non_null(expect);

    FILE *fp = fopen(filename, "w");
    assert_non_null(fp);

    for (size_t i = 0; i < bufsize - 2; i++) {
        // Extend the string
        expect[i] = '0' + i % 10;
        expect[i+1] = '\n';

        // Write test file
        fflush(fp);
        assert_int_equal(truncate(filename, 0), 0);
        rewind(fp);
        assert_true(fputs(expect, fp) > 0);
        fflush(fp);

        stream = dcs_open(filename, "r", DCS_PLAIN);
        assert_non_null(stream);
        dcs_setbufsize(stream, TESTING_BUFSIZE);

        res = dcs_getuntil(stream, &buf, &bufsize, delim);
        assert_int_equal(res, strlen(expect));
        assert_string_equal(buf, expect);

        // Check that realloc etc worked sensibly
        assert_non_null(buf);
        assert_true(bufsize >= TESTING_BUFSIZE * 2);
        assert_int_equal(buf[res-1], delim);
        assert_int_equal(buf[res], 0);
    }
    fclose(fp);

    free(buf);
    free(expect);
}


dcs_comp_algo dcs_guess_compression_type(const char *filename);
void test_stream_guess_algo(void **ctx)
{
    const char *files[] = {
        "file.txt", "file.txt.gz", ".hidden", "path/to/file.txt",
        "path/to/file.gz", "file.zst", "file.bz2", "file."
    };
    const dcs_comp_algo algos[] = {
        DCS_PLAIN, DCS_GZIP, DCS_PLAIN, DCS_PLAIN, DCS_GZIP,
        DCS_ZSTD, DCS_BZIP2, DCS_PLAIN
    };
    const size_t nfiles = sizeof(files) / sizeof(*files);
    assert_int_equal(sizeof(algos) / sizeof(*algos), nfiles);

    for (size_t fileidx = 0; fileidx < nfiles; fileidx++) {
        const char * file = files[fileidx];
        const dcs_comp_algo expect = algos[fileidx];

        dcs_comp_algo got = dcs_guess_compression_type(file);
        assert_int_equal(got, expect);
    }
}

void test_stream_flush(void **ctx)
{
    const dcs_comp_algo algos[] = {DCS_PLAIN, DCS_GZIP};
    const size_t nalgos = sizeof(algos) / sizeof(*algos);

    for (size_t algoidx = 0; algoidx < nalgos; algoidx++) {
        dcs_comp_algo algo = algos[algoidx];
        const char string[] = "String to be flushed\n";
        char got[100] = "";
        const size_t length = strlen(string);


        /***********
        *  Write  *
        ***********/
        // Open stream, check struct members initialised
        dcs_stream *wstream = dcs_open(filename, "w", algo);
        assert_non_null(wstream);

        // resize buffer, check that worked
        assert_int_equal(dcs_setbufsize(wstream, TESTING_BUFSIZE), 0);

        // Write buffer
        assert_int_equal(dcs_write(wstream, string, length), length);
        assert_int_equal(wstream->pos, length);

        // Flush buffer
        assert_int_equal(dcs_flush(wstream), 0);
        assert_non_null(wstream);
        assert_int_equal(wstream->len, 0);
        assert_int_equal(wstream->pos, 0);

        /**********
        *  Read  *
        **********/
        // Open file for reading, check struct
        dcs_stream *rstream = dcs_open(filename, "r", algo);
        assert_non_null(rstream);

        // Get string
        assert_int_equal(dcs_read(rstream, got, length), length);
        assert_int_equal(strlen(got), length);
        assert_string_equal(got, string);

        assert_int_equal(dcs_close(wstream), 0);
        assert_null(wstream);
        if (dcs_close(rstream) != 0) {
            puts(strerror(errno));
            assert_true(false);
        }
        assert_null(rstream);
    }
}

const struct CMUnitTest suite_stream[] = {
    cmocka_unit_test_teardown(test_stream_readwrite_roundtrip, remove_testfile),
    cmocka_unit_test_teardown(test_stream_bad_read, remove_testfile),
    cmocka_unit_test_teardown(test_stream_getc_ungetc, remove_testfile),
    cmocka_unit_test_teardown(test_stream_getuntil, remove_testfile),
    cmocka_unit_test_teardown(test_stream_getuntil_bigline, remove_testfile),
    cmocka_unit_test_teardown(test_stream_flush, remove_testfile),
    cmocka_unit_test(test_stream_guess_algo),
};
