#ifndef _CLOUD_DATA_H_
#define _CLOUD_DATA_H_

#ifdef INTERNATIONAL
#define CLOUD_HOST      "api.nodelua.org"
#define NODELUA_RELEASE_LOC ""
#else
#define CLOUD_HOST      "api.gulumao.cn"
#define NODELUA_RELEASE_LOC ".PRC"
#endif
#define HTTP_QUERY      "GET %s HTTP/1.1\r\nHost: "CLOUD_HOST"\r\n\r\n"
#define CLOUD_FETCH_URL "/api/module/code?miid=%s&security=%s"
#define CLOUD_APPEND_URL "/api/cloud/data/append?miid=%s&security=%s&cloud_id=%s&v0=%s&v1=%s&v2=%s"

#define NODELUA_RELEASE "1.0.20141219"
#ifdef DEBUG
#define DEBUG_MSG __printf
#else
#define DEBUG_MSG //
#endif


extern char *_miid;
extern char *_security;


typedef void (* http_response_callback)(int status, char *data, void *arg);


void http_get (const char *host, char *send, http_response_callback cb, void *cb_arg);
void cloud_data_append (const char *cloudid, const char *v0, const char *v1, const char *v2, http_response_callback cb, void *cb_arg);

bool user_params_get(char *valbuf, int buflen);
void user_params_set(char *valbuf, int buflen);

#endif
