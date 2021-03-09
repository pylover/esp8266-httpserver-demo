#include "params.h"
//#include "multipart.h"
//#include "querystring.h"
#include "status.h"
#include "debug.h"
#include "webadmin.h"
#include "httpd.h"
#include "uns.h"

#include <upgrade.h>
#include <osapi.h>
#include <mem.h>



//static Params *params;
static char buff[128];
static uint32_t bufflen = 0;
//
//
//void reboot_fotamode() {
//    system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
//    system_upgrade_reboot();
//}
//
//
//static ICACHE_FLASH_ATTR
//void app_reboot(struct httpd_request *req, char *body, uint32_t body_length, 
//        uint32_t more) {
//    char buffer[256];
//    uint8_t image = system_upgrade_userbin_check();
//    int len = os_sprintf(buffer, "Rebooting to %s mode...\r\n",
//        image == UPGRADE_FW_BIN1? "app": "FOTA");
//    httpd_response_text(req, HTTPSTATUS_OK, buffer, len);
//    status_update(500, 500, 1, reboot_fotamode);
//}


/* Under Test */
//
//#define BUFFSIZE	2048
//
//static Multipart mp;
//static char rbbuff[BUFFSIZE];
//static RingBuffer rb = {BUFFSIZE, 0, 0, rbbuff};
//
//
//static
//void _small_multipartcb(MultipartField *f, char *body, Size len, bool last) {
//    char b[128];
//    os_snprintf(b, len, "%s", body);
//    bufflen += os_sprintf(buff + bufflen, "%s=%s ", f->name, b);
//}
//
//
//ICACHE_FLASH_ATTR
//void webadmin_small_multipart(struct httpd_request *req, char *body, 
//        uint32_t body_length, uint32_t more) {
//
//	int err;
//    //os_printf("L: %d, more: %d\n", body_length, more);
//
//	if (body_length <= 0) {
//		return;
//	}
//	
//	if (mp.status == MP_IDLE) {
//		err = mp_init(&mp, req->contenttype, _small_multipartcb);
//		if (err != MP_OK) {
//			os_printf("Cannot init multipart: %d\r\n", err);
//			goto badrequest;
//		}
//        bufflen = 0;
//		rb_reset(&rb);
//        status_update(1000, 1000, INFINITE, NULL);
//	}
//	
//	espconn_recv_hold(req->conn);
//	if ((err = rb_safepush(&rb, body, body_length)) == RB_FULL) {
//		goto badrequest;
//	}
//    
//    err = mp_feedbybuffer(&mp, &rb);
//	espconn_recv_unhold(req->conn);
//	switch (err) {
//		case MP_DONE:
//			goto done;
//
//		case MP_MORE:
//			return;
//
//		default:
//			goto badrequest;
//	}
//
//done:
//	mp_close(&mp);
//	httpd_response_text(req, HTTPSTATUS_OK, buff, bufflen);
//    bufflen = 0;
//	return;
//
//badrequest:
//    bufflen = 0;
//	mp_close(&mp);
//    status_update(100, 100, 3, NULL);
//	httpd_response_notok(req, HTTPSTATUS_BADREQUEST);
//}
//
//
//
static ICACHE_FLASH_ATTR
void _form_cb(struct httpd_session *s, const char *field, 
        const char *value) {
    bufflen += os_sprintf(buff + bufflen, "%s=%s ", field, value);
}

//static ICACHE_FLASH_ATTR
//err_t demo_urlencoded(struct httpd_session *s) {
//    err_t err;
//    uint32_t more = HTTPD_REQUESTBODY_REMAINING(s);
//    DEBUG("more: %u"CR, more);
//    if (more) {
//        return HTTPD_OK;
//    }
//    
//    DEBUG("Starting response: %u"CR, more);
//    err = httpd_form_urlencoded_parse(s, _urlencoded_cb);
//    if (err) {
//        return err;
//    }
//    err = httpd_response_text(req, HTTPSTATUS_OK, buff, bufflen);
//    bufflen = 0;
//    return err;
//}

static ICACHE_FLASH_ATTR
err_t demo_querystring(struct httpd_session *s) {
    err_t err;
    bufflen = 0;
    httpd_querystring_parse(s, _form_cb);
    return httpd_response_text(s, HTTPSTATUS_OK, buff, bufflen);
}


static ICACHE_FLASH_ATTR
err_t demo_favicon(struct httpd_session *s) {
    #define FAVICON_SIZE    495

    #if SPI_SIZE_MAP == 2
    #define FAVICON_FLASH_SECTOR    0x77    
    #elif SPI_SIZE_MAP == 6
    #define FAVICON_FLASH_SECTOR    0x200    
    #endif
   
    char buffer[4 * 124];
    int result = spi_flash_read(
            FAVICON_FLASH_SECTOR * SPI_FLASH_SEC_SIZE,
            (uint32_t*) buffer,
            4 * 124
        );
    if (result != SPI_FLASH_RESULT_OK) {
        os_printf("SPI Flash write failed: %d\r\n", result);
        httpd_response_internalservererror(s);
        return;
    }
    return httpd_response(s, HTTPSTATUS_OK, NULL, 0, 
            HTTPHEADER_CONTENTTYPE_ICON, buffer, FAVICON_SIZE);
}


static ICACHE_FLASH_ATTR
err_t demo_headersecho(struct httpd_session *s) {
    return httpd_response(s, HTTPSTATUS_OK, s->request.headers, 
            s->request.headerscount, NULL, NULL, 0, false);
}



static ICACHE_FLASH_ATTR
err_t demo_index(struct httpd_session *s) {
    return httpd_response_text(s, HTTPSTATUS_OK, "Index", 5);
}


static struct httpd_route routes[] = {
//    {"FOTA",     "/",                app_reboot                      },
//    {"UPLOAD",   "/multipart",       webadmin_small_multipart        },
//    {"POST",     "/urlencoded",      demo_urlencoded             },
    {"ECHO",     "/queries",         demo_querystring            },
    {"ECHO",     "/headers",         demo_headersecho            },
    {"GET",      "/favicon.ico",     demo_favicon                },
    {"GET",      "/",                demo_index                  },
    { NULL }
};


ICACHE_FLASH_ATTR
int webadmin_start() {
    err_t err;
    err = httpd_init(routes);
    if (err) {
        ERROR("Cannot init httpd: %d\r\n", err);
    }
    return OK;
}


ICACHE_FLASH_ATTR
void webadmin_stop() {
    httpd_deinit();
}

