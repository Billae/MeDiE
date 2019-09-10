#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <czmq.h>
#include <json.h>
#include <string.h>
#include <errno.h>

#include "request.h"
#include "client_api.h"


#ifdef DISTRIBUTION_SH
    #include "distribution_sh_c.h"
    #include "protocol_sh.h"
#endif

#ifdef DISTRIBUTION_DH
    #include "distribution_dh_c.h"
    #include "protocol_dh.h"
#endif

#ifdef DISTRIBUTION_INDEDH
    #include "distribution_indedh_c.h"
    #include "protocol_indedh.h"
#endif

#ifdef DISTRIBUTION_WINDOWED
    #include "distribution_windowed_c.h"
    #include "protocol_windowed.h"
#endif

/* path in pcocc*/
#define HOST_PATH "/home/billae/prototype_MDS/etc/hosts.cfg"
/*path in ocre*/
//#define HOST_PATH "/ccc/home/cont001/ocre/billae/prototype_MDS/etc/hosts.cfg"

#define max_id_size 21


static int nb_servers;
static zsock_t **servers;


int client_init_context(int nb_srv)
{
    nb_servers = 0;
    servers = malloc(nb_srv*sizeof(zsock_t *));
    if (servers == NULL) {
        int err = errno;
        fprintf(stderr,
            "Client API:init_context: servers init alloc error: %s\n",
                strerror(err));
        return 0;
    }

    FILE *fd = fopen(HOST_PATH, "r");
    if (fd == NULL) {
        int err = errno;
        fprintf(stderr, "Client API:init_context: open hosts file error: %s\n",
            strerror(err));
        return 0;
    }


    char *id_srv;
    id_srv = malloc(max_id_size*sizeof(*id_srv));
    if (id_srv == NULL) {
        int err = errno;
        fprintf(stderr, "Client API:init_context: id_srv malloc error: %s\n",
            strerror(err));
        return 0;
    }

    while (nb_servers < nb_srv && fgets(id_srv, max_id_size, fd) != NULL) {
        char *positionEntree = strchr(id_srv, '\n');
        if (positionEntree != NULL)
            *positionEntree = '\0';
        servers[nb_servers] = client_init_connexion(id_srv);
        if (servers[nb_servers] == NULL) {
            fprintf(stderr, "Client API:init_context: init_connexion error\n");
            free(id_srv);
            return -nb_servers;
        }
        nb_servers++;
    }

    if (nb_servers < nb_srv) {
        if (!feof(fd)) {
            int err = errno;
            fprintf(stderr,
                "Client API:init_context: read hosts file error: %s\n",
                strerror(err));
            free(id_srv);
            return -nb_servers;
        } else {
            fprintf(stderr,
                "Client API:init_context: not enough servers in server list\n");
            return -nb_servers;
        }
    }

    int rc = distribution_init(nb_servers);
    if (rc != 0) {
        fprintf(stderr,
            "Client API:init_context: distribution init failed: %s\n",
                strerror(rc));
        return -nb_servers;
    }

    free(id_srv);
    fclose(fd);
    return nb_servers;
}



void client_finalize_context()
{
    distribution_finalize();
    int i;
    for (i = 0; i < nb_servers; i++)
        zsock_destroy(&servers[i]);
    free(servers);
}


zsock_t *client_init_connexion(const char *id_srv)
{
    char *socket;
    if (asprintf(&socket, "tcp://%s:%d", id_srv, Client_port) == -1) {
        int err = errno;
        fprintf(stderr,
            "Client API:init_connexion: format zmq socket name error: %s\n",
            strerror(err));
        return NULL;
    }

    zsock_t *sock = zsock_new_req(socket);
    if (sock == NULL) {
        fprintf(stderr, "Client API:init_connexion: ");
        fprintf(stderr, "create zmq socket %s error\n", socket);
        return NULL;
    }
    free(socket);

    return sock;
}


int client_request(json_object *request, json_object **reply)
{
    int rc = 0;

    /*call the distribution processing*/
    rc = distribution_pre_send(request);
    if (rc != 0) {
        fprintf(stderr,
            "Client API:request: distribution pre_send error\n");
        goto fail;
    }

    /*getting the server ID*/
    json_object *host;
    if (!json_object_object_get_ex(request, "id_srv", &host))
        fprintf(stderr,
            "Client API:request: json extract error: no key \"host\" found\n");
    errno = 0;
    int srv_id = json_object_get_int(host);
    if (errno == EINVAL) {
        fprintf(stderr, "Client API:request: get server id error\n");
        goto fail;
    }

    /*sending request*/
    const char *req_c = json_object_to_json_string(request);
    zstr_send(servers[srv_id], req_c);

    /*cleaning json object sent*/
    if (json_object_put(request) != 1)
        fprintf(stderr, "Client API:request: free request error\n");


    /*receiving reply*/
    char *string = zstr_recv(servers[srv_id]);
    *reply = json_tokener_parse(string);
    zstr_free(&string);

    /*processing reply*/
    json_object *rep_flag;
    if (!json_object_object_get_ex(*reply, "repFlag", &rep_flag)) {
        fprintf(stderr,
            "Client API:request: json extract error: no key \"repFlag\" found\n");
        goto fail;
    }

    int rep_rc;
    if (strcmp(json_object_get_string(rep_flag), "done") == 0)
        rep_rc = 0;
    else if (strcmp(json_object_get_string(rep_flag), "update&retry") == 0)
        rep_rc = -EAGAIN;
    else if (strcmp(json_object_get_string(rep_flag), "wait&retry") == 0) {
        reply = NULL;
        return -EALREADY;
    } else
        goto fail;

    /*call the distribution processing*/
    rc = distribution_post_receive(*reply);
    if (rc != 0) {
        fprintf(stderr,
            "Client API:request: distribution post_receive error\n");
         /*cleaning*/
        if (json_object_put(*reply) != 1)
            fprintf(stderr, "Client API:request: free reply error\n");
        return -1;
    }
    return rep_rc;

fail:
    reply = NULL;
    return -1;
}


int client_request_create(const char *key, const char *data)
{
    int rc = 0;

    json_object *request = create_request_create(key, data);
    if (request == NULL) {
        fprintf(stderr, "client api:request_create: create_request failed\n");
        return -1;
    }

    json_object *reply = NULL;
    rc = client_request(request, &reply);

    /*cleaning*/
    if (json_object_put(reply) != 1)
        fprintf(stderr, "client api:request_create: free reply error\n");

    return rc;
}


int client_request_read(const char *key, char **return_value)
{
    int rc = 0;

    json_object *request = create_request_read(key);
    if (request == NULL) {
        fprintf(stderr, "client api:request_read: create_request failed\n");
        return -1;
    }

    json_object *reply = NULL;
    rc = client_request(request, &reply);
    if (reply == NULL) {
        fprintf(stderr, "client api:request_read: no reply found\n");
        return -1;
    }


    json_object *rep_value;
    if (!json_object_object_get_ex(reply, "getValue", &rep_value)) {
        fprintf(stderr,
            "Client API:request_read: json extract error: no key \"getValue\" found\n");
        return -1;
    }

    if (asprintf(return_value, "%s", json_object_get_string(rep_value)) == -1)
        fprintf(stderr, "Client API:request_read: asprintf error\n");

    /*cleaning*/
    if (json_object_put(reply) != 1)
        fprintf(stderr, "client api:request_read: free reply error\n");

    return rc;
}


int client_request_update(const char *key, const char *data)
{
    int rc = 0;

    json_object *request = create_request_update(key, data);
    if (request == NULL) {
        fprintf(stderr, "client api:request_update: create_request failed\n");
        return -1;
    }

    json_object *reply = NULL;
    rc = client_request(request, &reply);

    /*cleaning*/
    if (json_object_put(reply) != 1)
        fprintf(stderr, "client api:request_update: free reply error\n");

    return rc;
}


int client_request_delete(const char *key)
{
    int rc = 0;

    json_object *request = create_request_delete(key);
    if (request == NULL) {
        fprintf(stderr, "client api:request_delete: create_request failed\n");
        return -1;
    }

    json_object *reply = NULL;
    rc = client_request(request, &reply);

    /*cleaning*/
    if (json_object_put(reply) != 1)
        fprintf(stderr, "client api:request_delete: free reply error\n");

    return rc;
}
