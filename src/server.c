#include <stdio.h>
#include <stdlib.h>
#include <czmq.h>

int main(void)
{
    zsock_t *pull = zsock_new_pull("inproc://example");

    char *string = zstr_recv(pull);
    puts (string);
    zstr_free(&string);

    zsock_destroy(&pull);

    return 0;
}
