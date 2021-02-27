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


static Params *params;
static char buff[64];
static uint32_t bufflen = 0;


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
void _fieldcb(const char *field, const char *value) {
    os_printf("%s %s\n", field, value);
    bufflen += os_sprintf(buff + bufflen, "%s=%s ", field, value);
}


static ICACHE_FLASH_ATTR
void webadmin_urlencoded(struct httpd_request *req, char *body, uint32_t len, 
        uint32_t more) {
    if (more) {
        return;
    }
    body[len] = 0;
    os_printf("%s\n", body);
    querystring_parse(body, _fieldcb);  
    os_printf("%s len: %d\n", buff, bufflen);
    httpd_response_text(req, HTTPSTATUS_OK, buff, bufflen);
    bufflen = 0;
}


static ICACHE_FLASH_ATTR
void webadmin_favicon(struct httpd_request *req, char *body, uint32_t len, 
        uint32_t more) {
    
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
    httpd_response(req, HTTPSTATUS_OK, HTTPHEADER_CONTENTTYPE_ICON, buffer, 
            FAVICON_SIZE, NULL, 0);
}


static ICACHE_FLASH_ATTR
void webadmin_index(struct httpd_request *req, char *body, 
        uint32_t body_length, uint32_t more) {
    char buffer[64];
    int len = os_sprintf(buffer, "%s Index", params->name);
    httpd_response_text(req, HTTPSTATUS_OK, buffer, len);
}


static struct httproute routes[] = {
    {"FOTA",     "/",                app_reboot                      },


    // Under test
    {"POST",     "/urlencoded",      webadmin_urlencoded             },
    {"GET",      "/favicon.ico",     webadmin_favicon                },
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

