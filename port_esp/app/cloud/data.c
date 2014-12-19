/******************************************************************************
 * Copyright 2014 Nodelua.org (chenliang)
 *
*******************************************************************************/

#include <osapi.h>
#include <os_type.h>
#include "user_interface.h"
#include <espconn.h>
#include <mem.h>
#include "cloud/data.h"


char *_miid;
char *_security;

ip_addr_t dummy_ip;
int _active = 0;

typedef struct {
	char *buf;
	http_response_callback cb;
	void *cb_arg;
} HttpReq;

char * ICACHE_FLASH_ATTR http_parse_response(char *pdata, int *status)
{
	char *lenStr = strstr(pdata, "\r\n\r\n");
	if (lenStr != NULL)
	{
		char *lenStrEnd = strstr(lenStr+4, "\r\n");
		if (lenStrEnd != NULL)
		{
			int len = strtol(lenStr+4, NULL, 16);
			char *code = lenStrEnd+2;
			*(code+len) = 0;
			DEBUG_MSG("http resp(len=%d):\n%s\n", len, code);
			*status = 200; //TODO:
			return code;
		}
		else
		{
			__printf("parse http response err1\n");
		}
	}
	else
	{
		__printf("parse http response err2: %s\n", pdata);
	}
	*status = -1;
	return NULL;
}

/**
  * @brief  Client received callback function.
  * @param  arg: contain the ip link information
  * @param  pdata: received data
  * @param  len: the lenght of received data
  * @retval None
  */
static void ICACHE_FLASH_ATTR cloud_data_on_recv_cb(void *arg, char *pdata, unsigned short len)
{
	struct espconn *pConn = (struct espconn *)arg;

	int status = -1;
	char *res = http_parse_response(pdata, &status);

	HttpReq *req = pConn->reverse;
	if (req->cb)
	{
		req->cb(status, res, req->cb_arg);
		req->cb = NULL;
	}

	//TODO: concat more then one package
	espconn_disconnect(pConn);
}

/**
  * @brief  Tcp client disconnect success callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR cloud_data_on_discon_cb(void *arg)
{
    struct espconn *pConn = (struct espconn *)arg;

	HttpReq *req = pConn->reverse;

	if (req->cb)
	{
		req->cb(-1, NULL, req->cb_arg);
		req->cb = NULL;
	}

	os_free(req->buf);
	os_free(req);
	os_free(pConn->proto.tcp);
	os_free(pConn);
	_active --;

    DEBUG_MSG("tcp client disconnect(active=%d)\n", _active);
}

static void ICACHE_FLASH_ATTR cloud_data_on_recon_cb(void *arg, sint8 errType)
{
    struct espconn *pConn = (struct espconn *)arg;

    DEBUG_MSG("tcp client reconnect\n");
}

/**
  * @brief  Tcp client connect success callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR cloud_data_on_connect_cb(void *arg)
{
    struct espconn *pConn = (struct espconn *)arg;
	HttpReq *req = pConn->reverse;

    DEBUG_MSG("tcp client connect\n");

    espconn_regist_disconcb(pConn, cloud_data_on_discon_cb);
    espconn_regist_recvcb(pConn, cloud_data_on_recv_cb);////////
    //espconn_regist_sentcb(pConn, tcpclient_sent_cb);///////

	espconn_sent(pConn, req->buf, os_strlen(req->buf));
}

static void ICACHE_FLASH_ATTR cloud_data_on_dns_found(const char *host, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;
    if (ipaddr != NULL)
    {
        char buf[16];
        char *p = (char*)&(ipaddr->addr);
        os_memcpy(pespconn->proto.tcp->remote_ip, &(ipaddr->addr), 4);
		_active ++;
        DEBUG_MSG("connection to "IPSTR"...(active=%d)\n", p[0], p[1], p[2], p[3], _active);

        espconn_connect(pespconn);
    }
	else
	{
		__printf("dns failed!\n");
		espconn_gethostbyname(pespconn, host, &dummy_ip, cloud_data_on_dns_found);
	}
}

void ICACHE_FLASH_ATTR http_get (const char *host, char *send, http_response_callback cb, void *cb_arg)
{
	struct espconn *pFetchConn;

	HttpReq *req = (HttpReq*)os_malloc(sizeof(HttpReq));
	req->buf = (char *)os_malloc(os_strlen(send)+1);
	os_strcpy(req->buf, send);
	req->cb = cb;
	req->cb_arg = cb_arg;

    pFetchConn = (struct espconn *)os_zalloc(sizeof(struct espconn));
	pFetchConn->reverse = req;
    pFetchConn->type = ESPCONN_TCP;
    pFetchConn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));

    pFetchConn->proto.tcp->local_port = espconn_port();

    espconn_regist_connectcb(pFetchConn, cloud_data_on_connect_cb);
    espconn_regist_reconcb(pFetchConn, cloud_data_on_recon_cb);
    pFetchConn->proto.tcp->remote_port = 80;
    espconn_gethostbyname(pFetchConn, host, &dummy_ip, cloud_data_on_dns_found);
}

void ICACHE_FLASH_ATTR cloud_data_append (const char *cloudid, const char *v0, const char *v1, const char *v2, http_response_callback cb, void *cb_arg)
{
	char buf0[512], buf1[512];
	os_sprintf(buf1, CLOUD_APPEND_URL, _miid, _security, cloudid, v0, v1, v2);
	os_sprintf(buf0, HTTP_QUERY, buf1);

	http_get(CLOUD_HOST, buf0, cb, cb_arg);
}
