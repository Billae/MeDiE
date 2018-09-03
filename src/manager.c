#include <stdio.h>
#include <stdlib.h>
#include <czmq.h>
#include <errno.h>
#include <pthread.h>

#include "protocol.h"
#include "mlt.h"
#include "eacl.h"


/*TO DO*/
int init_manager()
{

}


/*TO DO*/
int manager_receive_eacl(struct eacl *new_list)
{

}

/*TO DO*/
int manager_calculate_relab()
{

}


/*TO DO*/
int *manager_balance_load(int current_load, int goal_load, int *list)
{

}


/*TO DO*/
void *thread_manager(void *args)
{

}


/*TO DO*/
void *thread_timer(void *args)
{

}


/*TO DO*/
int main(int argc, char *argv[])
{
    char *socket;
    //a changer pour pcocc
    asprintf(&socket, "tcp://192.168.129.25:%d", Mlt_port);

    printf("trying to connect to %s\n", socket);
    zsock_t *pub;
    pub = zsock_new_pub(socket);
    if (pub == NULL) {
            fprintf(stderr,
                "create zmq socket error\n");
            return -1;
    }

    //while (1) {
    int i;
    for (i = 0; i < 10; i++) {
        zstr_send(pub, "falala");
        printf("manager\n");
        sleep(1);
    }

    /*cleaning*/
    zsock_destroy(&pub);

    return 0;
}
