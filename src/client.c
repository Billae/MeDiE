#include<stdio.h>
#include<stdlib.h>
#include<czmq.h>

int main (void)
{
    zsock_t *push = zsock_new_push("inproc://example");

    sleep(10);
    printf("avant");
    zstr_send(push, "hello, World");
   
    sleep(10);
    printf("apres");
    zsock_destroy(&push);
    return 0;
}
