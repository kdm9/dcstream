#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>

#include "dcs_stream.h"

#define BUFFSIZE 1<<12

int main(int argc, char *argv[])
{
    int ret = EXIT_FAILURE;
    dcs_stream *in = NULL, *out = NULL;

    in = dcs_dopen(STDIN_FILENO, "r", DCS_PLAIN);
    if (in == NULL) goto end;
    out = dcs_open(argv[1], "w", DCS_PLAIN);
    if (out == NULL) goto end;

    unsigned char buf[BUFFSIZE];

    while (1) {
        ssize_t bytesread = dcs_read(in, buf, BUFFSIZE);
        if (bytesread < 0) goto end;
        ssize_t byteswrote = dcs_write(out, buf, bytesread);
        if (byteswrote < 0 || bytesread != byteswrote) goto end;
        if (dcs_eof(in)) break;
    }

    ret = EXIT_SUCCESS;
end:
    if (in != NULL) dcs_close(in);
    if (out != NULL) dcs_close(out);
    return ret;
}
