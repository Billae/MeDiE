#include<stdio.h>
#include<stdlib.h>
#include<json.h>
#include"distribution.h"
#include"request.h"



json_object* create_request(char* key)
{
    json_object *request = json_object_new_object();
    
    json_object *req_key = json_object_new_string(key);
    json_object_object_add(request, "key", req_key);
    
    return request;

}


json_object* create_request_create(char* key, char* data)
{
    json_object *request = create_request(key);
    
    /*request type: 1 -> create*/
    json_object *type = json_object_new_int(1);
    json_object *str = json_object_new_string(data);
    
    json_object_object_add(request, "data", str);
    json_object_object_add(request, "type", type);

    //find the server with the distribution method 
    json_object *hostname = json_object_new_string(AssignSrvByKey(key));
    json_object_object_add(request, "id_srv", hostname);

    return request;
}
