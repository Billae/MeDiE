#include<stdio.h>
#include<stdlib.h>
#include<czmq.h>

int main (void)
{
    zsock_t *req = zsock_new_req("tcp://10.252.0.1:7410");

    zstr_send(req, "hello");
    char* string = zstr_recv(req);
    puts(string);
    zstr_free(&string);
    zsock_destroy(&req);
    return 0;
}
