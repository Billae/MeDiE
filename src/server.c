#include <stdio.h>
#include <stdlib.h>
#include <czmq.h>
#include <json.h>
#include <string.h>

int main(void)
{
    /*for pcocc VM*/
    //zsock_t *rep = zsock_new_rep("tcp://10.252.0.1:7410");
    
    /*for ocre*/
    zsock_t *rep = zsock_new_rep("tcp://192.168.129.25:7410");
    
    //receiving request
    char *string = zstr_recv(rep);
    //puts (string);
    json_object *request = json_tokener_parse(string);
    zstr_free(&string);

    //checking 
    json_object *req;
    if(!json_object_object_get_ex(request, "data", &req))
        printf("Error: no key found\n");
    if(strcmp(json_object_get_string(req), "Hello")==0)
        printf("Win serveur: Hello reçu !\n");

    //printf("%s %d",json_object_get_string(req), json_object_get_string_len(req));
    
    //creating reply and send
    json_object *reply = json_object_new_string("World");
    json_object_object_add(request, "rep", reply);
    
    const char* rep_c = json_object_to_json_string(request); 
    zstr_send(rep,rep_c);
    
    //cleaning
    json_object_object_del(request, "req");
    json_object_object_del(request, "rep");
    if(json_object_put(request) != 1)
        printf("error free request");

    zsock_destroy(&rep);

    return 0;
}
