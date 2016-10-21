#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "test_stream.c"


/*******************************************************************************
*                                    MAIN                                     *
*******************************************************************************/

int main(int argc, char *argv[])
{
    int ret = 0;
    ret |= cmocka_run_group_tests_name("stream", suite_stream, NULL, NULL);
    return ret;
}
