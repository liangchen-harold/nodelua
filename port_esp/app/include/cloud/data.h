#ifndef _CLOUD_DATA_H_
#define _CLOUD_DATA_H_


#define CLOUD_HOST      "api.gulumao.cn"
#define HTTP_QUERY      "GET %s HTTP/1.1\r\nHost: "CLOUD_HOST"\r\n\r\n"
#define miid "6"
#define security "9eeec357d65a9ef4ef79e4473e96cb88"
#define CLOUD_FETCH_URL "/api/module/code?miid=%s&security=%s"
#define CLOUD_APPEND_URL "/api/cloud/data/append?miid=%s&security=%s&tiid=%s&v0=%s&v1=%s&v2=%s"


typedef void (* http_response_callback)(int status, char *data);


void http_get (const char *host, char *send, http_response_callback cb);
void cloud_data_append (const char *cloudid, const char *v0, const char *v1, const char *v2);

#endif
