#include "params.h"
#include "multipart.h"
#include "querystring.h"
#include "status.h"
#include "debug.h"
#include "webadmin.h"
#include "httpd.h"
#include "uns.h"
#include "http.h"

#include <upgrade.h>
#include <osapi.h>
#include <mem.h>


#define FAVICON_SIZE    495

#if SPI_SIZE_MAP == 2
#define FAVICON_FLASH_SECTOR    0x77    
#elif SPI_SIZE_MAP == 6
#define FAVICON_FLASH_SECTOR    0x200    
#endif

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
    "zone: <input name=\"zone\" value=\"%s\"/><br/>" \
    "name: <input name=\"name\" value=\"%s\"/><br/>" \
    "AP PSK: <input name=\"ap_psk\" value=\"%s\"/><br/>" \
    "SSID: <input name=\"ssid\" value=\"%s\"/><br/>" \
    "PSK: <input name=\"psk\" value=\"%s\"/><br/>" \
    "<input type=\"submit\" value=\"Reboot\" />" \
    "</form>" \
    HTML_FOOTER

static Params *params;


void discovercb(struct unsrecord *rec, void *arg) {
    char buffer[128];
    struct httpd_request *req = (struct httpd_request *) arg;
    int len = os_sprintf(buffer, "%s "IPSTR"\n", rec->fullname, 
            IP2STR(&rec->address));
    httpd_response_text(req, HTTPSTATUS_OK, buffer, len);
}


static ICACHE_FLASH_ATTR
void webadmin_uns_discover(struct httpd_request *req, char *body, 
        uint32_t bodylen, uint32_t more) {
    char *pattern = rindex(req->path, '/') + 1;
    uns_discover(pattern, discovercb, req);
}


static
void httpcb(int status, char *body, void *arg) {
    struct httpd_request *req = (struct httpd_request *) arg;
    httpd_response_text(req, HTTPSTATUS_OK, body, strlen(body));
}


static ICACHE_FLASH_ATTR
void webadmin_sysinfo(struct httpd_request *req, char *body, 
        uint32_t body_length, uint32_t more) {
    int len;
    char buffer[512];
    if (strlen(req->path) <= 1) {
        len = os_sprintf(buffer, "%d Free mem: %d.\n", 
            system_get_time() / 1000000,
            system_get_free_heap_size()
        );
        httpd_response_text(req, HTTPSTATUS_OK, buffer, len);
        return;
    }
    
    char *pattern = rindex(req->path, '/');
    pattern++;
    DEBUG("Trying UNS for: %s\n", pattern);
    http_nobody_uns(pattern, "INFO", "/", httpcb, req);
}


void reboot_fotamode() {
    system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
    system_upgrade_reboot();
}


static ICACHE_FLASH_ATTR
void app_reboot(struct httpd_request *req, char *body, uint32_t body_length, 
        uint32_t more) {
    char buffer[256];
    uint8_t image = system_upgrade_userbin_check();
    int len = os_sprintf(buffer, "Rebooting to %s mode...\r\n",
        image == UPGRADE_FW_BIN1? "app": "FOTA");
    httpd_response_text(req, HTTPSTATUS_OK, buffer, len);
    status_update(500, 500, 1, reboot_fotamode);
}


static ICACHE_FLASH_ATTR
void _update_params_field(const char *field, const char *value) {

    char *target;
    if (os_strcmp(field, "zone") == 0) {
        target = (char*)&params->zone;
    }
    else if (os_strcmp(field, "name") == 0) {
        target = (char*)&params->name;
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
void webadmin_get_params(struct httpd_request *req, char *body, 
        uint32_t body_length, uint32_t more) {

    char buffer[1024];
    int len = os_sprintf(buffer, HTML_FORM, 
            params->zone, 
            params->name, 
            params->ap_psk, 
            params->station_ssid, 
            params->station_psk);
    httpd_response_html(req, HTTPSTATUS_OK, buffer, len);
}


static ICACHE_FLASH_ATTR
void webadmin_set_params(struct httpd_request *req, char *body, 
        uint32_t body_length, uint32_t more) {

    body[body_length] = 0;
    querystring_parse(body, _update_params_field);  
    if (!params_save(params)) {
        httpd_response_notok(req, HTTPSTATUS_SERVERERROR);
        return;
    }
    system_restart();
}


static ICACHE_FLASH_ATTR
void webadmin_favicon(struct httpd_request *req, char *body, 
        uint32_t body_length, uint32_t more) {
    
    char buffer[4 * 124];
    int result = spi_flash_read(
            FAVICON_FLASH_SECTOR * SPI_FLASH_SEC_SIZE,
            (uint32_t*) buffer,
            4 * 124
        );
    if (result != SPI_FLASH_RESULT_OK) {
        os_printf("SPI Flash write failed: %d\r\n", result);
        httpd_response_notok(req, HTTPSTATUS_SERVERERROR);
        return;
    }
    httpd_response(req, HTTPSTATUS_OK, "image/x-icon", buffer, FAVICON_SIZE, 
            NULL, 0);
}


static ICACHE_FLASH_ATTR
void webadmin_index(struct httpd_request *req, char *body, 
        uint32_t body_length, uint32_t more) {
    char buffer[1024];
    int len = os_sprintf(buffer, HTML_INDEX, params->name);
    httpd_response_html(req, HTTPSTATUS_OK, buffer, len);
}


static struct httproute routes[] = {
    {"DISCOVER", "/uns",             webadmin_uns_discover           },
    {"POST",     "/params",          webadmin_set_params             },
    {"GET",      "/params",          webadmin_get_params             },
    {"GET",      "/favicon.ico",     webadmin_favicon                },
    {"FOTA",      "/",                app_reboot                      },
    {"INFO",     "/",                webadmin_sysinfo                },
    {"GET",      "/",                webadmin_index                  },
    { NULL }
};


static struct httpd httpd;


ICACHE_FLASH_ATTR
int webadmin_start(Params *_params) {
    err_t err;
    params = _params;
    err = httpd_init(&httpd, routes);
    if (err) {
        ERROR("Cannot init httpd: %d\r\n", err);
    }
    return OK;
}


ICACHE_FLASH_ATTR
void webadmin_stop() {
    httpd_stop(&httpd);
}

