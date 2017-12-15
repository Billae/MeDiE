#ifndef __REQUEST_H__
#define __REQUEST_H__

#include<json.h>

json_object* create_request(char* key);
json_object* create_request_create(char* key, char* data);

#endif
