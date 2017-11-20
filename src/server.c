#include <stdio.h>
#include <stdlib.h>
#include <czmq.h>

int main(void)
{
    zsock_t *rep = zsock_new_rep("tcp://10.252.0.1:7410");

    char *string = zstr_recv(rep);
    puts (string);
    zstr_free(&string);

    zstr_send(rep,"World");
    zsock_destroy(&rep);

    return 0;
}
