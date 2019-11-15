#include "params.h"
#include "httpserver.h"
#include "multipart.h"

#include <osapi.h>
#include <mem.h>


#define FAVICON_SIZE	495
#define FAVICON_FLASH_SECTOR	0x200	
#define HTML_HEADER \
	"<!DOCTYPE html><html>" \
	"<head><title>ESP8266 Firstboot config</title></head><body>\r\n" 
#define HTML_FOOTER "\r\n</body></html>\r\n"

#define HTML_INDEX \
	HTML_HEADER \
	"<h4>Welcome to %s Web Administration</h4><br />" \
	"<a href=\"/params\">Params</a><br />" \
	HTML_FOOTER

#define HTML_FORM \
	HTML_HEADER \
	"<form action=\"/params\" method=\"post\">" \
	"<h4>Settings</h4>" \
	"name: <input name=\"name\" value=\"%s\"/><br/>" \
	"AP PSK: <input name=\"ap_psk\" value=\"%s\"/><br/>" \
	"SSID: <input name=\"ssid\" value=\"%s\"/><br/>" \
	"PSK: <input name=\"psk\" value=\"%s\"/><br/>" \
	"<input type=\"submit\" value=\"Reboot\" />" \
	"</form>" \
	HTML_FOOTER

static Params *params;
static ETSTimer ff;

#define BUFFSIZE	2048

static Multipart mp;
static char buff[BUFFSIZE];
static RingBuffer rb = {BUFFSIZE, 0, 0, buff};
static MultipartField *cf = NULL;




static ICACHE_FLASH_ATTR
void _update_params_field(const char *field, const char *value) {

	char *target;
	if (os_strcmp(field, "name") == 0) {
		target = (char*)&params->device_name;
	}
	else if (os_strcmp(field, "ap_psk") == 0) {
		target = (char*)&params->ap_psk;
	}
	else if (os_strcmp(field, "ssid") == 0) {
		target = (char*)&params->station_ssid;
	}
	else if (os_strcmp(field, "psk") == 0) {
		target = (char*)&params->station_psk;
	}
	else return;
	os_strcpy(target, value);
}


static ICACHE_FLASH_ATTR
void webadmin_get_params(Request *req, char *body, uint32_t body_length, 
		uint32_t more) {

	char buffer[1024];
	int len = os_sprintf(buffer, HTML_FORM, 
			params->device_name, 
			params->ap_psk, 
			params->station_ssid, 
			params->station_psk);
	httpserver_response_html(req, HTTPSTATUS_OK, buffer, len);
}


static ICACHE_FLASH_ATTR
void webadmin_set_params(Request *req, char *body, uint32_t body_length, 
		uint32_t more) {
	body[body_length] = 0;
	httpserver_parse_querystring(body, _update_params_field);  
	if (!params_save(params)) {
		httpserver_response_notok(req, HTTPSTATUS_SERVERERROR);
		return;
	}
	system_restart();
}


static ICACHE_FLASH_ATTR
void webadmin_favicon(Request *req, char *body, uint32_t body_length, 
		uint32_t more) {
	
	char buffer[4 * 124];
	int result = spi_flash_read(
			FAVICON_FLASH_SECTOR * SPI_FLASH_SEC_SIZE,
			(uint32_t*) buffer,
			4 * 124
		);
	if (result != SPI_FLASH_RESULT_OK) {
		os_printf("SPI Flash write failed: %d\r\n", result);
		httpserver_response_notok(req, HTTPSTATUS_SERVERERROR);
		return;
	}
	httpserver_response(req, HTTPSTATUS_OK, "image/x-icon", buffer, 495, NULL, 0);
}


static ICACHE_FLASH_ATTR
void webadmin_index(Request *req, char *body, uint32_t body_length, 
		uint32_t more) {
	char buffer[1024];
	int len = os_sprintf(buffer, HTML_INDEX, params->device_name);
	httpserver_response_html(req, HTTPSTATUS_OK, buffer, len);
}


static HttpRoute routes[] = {
	{"POST", 	"/params",			webadmin_set_params				},
	{"GET",  	"/params",			webadmin_get_params				},
	{"GET",  	"/favicon.ico",		webadmin_favicon				},
	{"GET",  	"/",				webadmin_index					},
	{ NULL }
};


int ICACHE_FLASH_ATTR
webadmin_start(Params *_params) {
	params = _params;
	httpserver_init(80, routes);
	return OK;
}


void ICACHE_FLASH_ATTR
webadmin_stop() {
	httpserver_stop();
}

