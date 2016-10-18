#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

/*******************************************************************************
*                                 test_stream                                 *
*******************************************************************************/

extern void test_stream_write(void **ctx);
extern void test_stream_read(void **ctx);
extern int remove_testfile(void **ctx);
const struct CMUnitTest suite_stream[] = {
    cmocka_unit_test_teardown(test_stream_read, remove_testfile),
    cmocka_unit_test_teardown(test_stream_write, remove_testfile),
};


/*******************************************************************************
*                                    MAIN                                     *
*******************************************************************************/

int main(int argc, char *argv[])
{
    int ret = 0;
    ret |= cmocka_run_group_tests_name("stream", suite_stream, NULL, NULL);
    return ret;
}
