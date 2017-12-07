#include<stdio.h>
#include<stdlib.h>
#include<czmq.h>
#include<json.h>
#include<string.h>

int main (void)
{
    /*pour les VM pcocc*/
    //zsock_t *req = zsock_new_req("tcp://10.252.0.1:7410");
    /*pour ocre*/
    zsock_t *req = zsock_new_req("tcp://192.168.129.25:7410");
  
    //on cree une requete et on l'envoie
    json_object *request = json_object_new_object();
    json_object *str = json_object_new_string("Hello");
    json_object_object_add(request, "req", str);
    
    const char* req_c = json_object_to_json_string(request);
    zstr_send(req, req_c);

    //on clean
    json_object_object_del(request, "req");
    json_object_put(request);

    //on recupere la reponse
    char* string = zstr_recv(req);
    //puts(string);
    json_object *reply = json_tokener_parse(string);
    zstr_free(&string);

    //on verifie la reponse
    json_object *rep;
    if(!json_object_object_get_ex(reply, "rep", &rep))
        printf("Error: no key found\n");
    if(strcmp(json_object_get_string(rep),"World")==0)
        printf("Win client: World reçu !\n");
    
    //printf("%s %d",json_object_get_string(rep), json_object_get_string_len(rep));
    
    //on clean
    json_object_object_del(reply, "req");
    json_object_object_del(reply, "rep");
    if(json_object_put(reply) != 1)
        printf("error free reply");

    zsock_destroy(&req);

    return 0;
}
