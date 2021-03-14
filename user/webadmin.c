#include "params.h"
#include "status.h"
#include "webadmin.h"
#include "httpd.h"
#include "uns.h"

#include <upgrade.h>
#include <osapi.h>
#include <mem.h>


static char buff[128];
static uint32_t bufflen = 0;


void reboot_fotamode_cb() {
    system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
    system_upgrade_reboot();
}


static ICACHE_FLASH_ATTR
httpd_err_t reboot_fotamode(struct httpd_session *s) {
    uint8_t image = system_upgrade_userbin_check();
    bufflen = os_sprintf(buff, "Rebooting to %s mode...\r\n",
        image == UPGRADE_FW_BIN1? "APP": "FOTA");
    status_update(500, 500, 1, reboot_fotamode_cb);
    return HTTPD_RESPONSE_TEXT(s, HTTPSTATUS_OK, buff, bufflen);
}


static ICACHE_FLASH_ATTR
httpd_err_t demo_download(struct httpd_session *s) {
    httpd_err_t err = httpd_response_start(s, HTTPSTATUS_OK, NULL, 0, 
            HTTPHEADER_CONTENTTYPE_BINARY, 0, HTTPD_FLAG_STREAM);
    if (err) {
        return err;
    }
    
    err = httpd_send(s, "Foo"CR, 5);
    if (err) {
        return err;
    }
    err = httpd_send(s, "Bar"CR, 5);
    if (err) {
        return err;
    }
    err = httpd_send(s, "Baz"CR, 5);
    if (err) {
        return err;
    }
    err = httpd_send(s, "Qux"CR, 5);
    if (err) {
        return err;
    }
    httpd_response_finalize(s, HTTPD_FLAG_CLOSE);
    return HTTPD_OK;
}


static struct httpd_session *downloader;
static struct httpd_multipart *uploader;


static ICACHE_FLASH_ATTR
httpd_err_t _multipart_stream_cb(struct httpd_multipart *m, char *data, 
        size16_t len, bool lastchunk, bool finish) {
    httpd_err_t err;
    
    uploader = m;
    CHK("CB: %dB l: %d f: %d", len, lastchunk, finish);
    
    if (downloader && len) {
        err = httpd_send(downloader, data, len);
        if (err) {
            return err;
        }
    }
    if (finish){
        CHK("Response uploader");
        err = HTTPD_RESPONSE_TEXT(m->session, HTTPSTATUS_OK, "Ok"CR, 4);
        if(err) {
            return err;
        }
        if (downloader != NULL) {
            CHK("Finalize downloader");
            httpd_response_finalize(downloader, HTTPD_FLAG_CLOSE);
            downloader = NULL;
        }
    }
    return HTTPD_OK;
}


static ICACHE_FLASH_ATTR
httpd_err_t demo_download_chunk_sent(struct httpd_session *s) {
    httpd_err_t err;
    size16_t available = HTTPD_RESP_LEN(s);
    //CHK("SENT CB: avail: %d", available);
    if (available) {
        return HTTPD_OK;
    }
    
    if (s->status >= HTTPD_SESSIONSTATUS_CLOSING) {
        uploader = NULL;
    }
    if (uploader != NULL) {
        if(!HTTPD_SCHEDULE(HTTPD_SIG_RECVUNHOLD, 
                    (os_param_t) uploader->session)) {
            return HTTPD_ERR_TASKQ_FULL;
        }
    }
    return HTTPD_OK;
}

static ICACHE_FLASH_ATTR
httpd_err_t demo_download_stream(struct httpd_session *s) {
    s->sentcb = demo_download_chunk_sent;
    httpd_err_t err = httpd_response_start(s, HTTPSTATUS_OK, NULL, 0, 
            HTTPHEADER_CONTENTTYPE_BINARY, 0, HTTPD_FLAG_STREAM);
    if (err) {
        return err;
    }
    
    downloader = s;
    return HTTPD_OK;
}


static ICACHE_FLASH_ATTR
httpd_err_t demo_multipart_stream(struct httpd_session *s) {
    return httpd_form_multipart_parse(s, _multipart_stream_cb);
}


static ICACHE_FLASH_ATTR
httpd_err_t _multipart_cb(struct httpd_multipart *m, char *data, 
        size16_t len, bool lastchunk, bool finish) {
    char tmp[HTTPD_MP_CHUNK];
    if (finish) {
        CHK("Finish");
        return HTTPD_RESPONSE_TEXT(m->session, HTTPSTATUS_OK, buff, bufflen);
    }

    if (lastchunk) {
        CHK("len: %d", len);
        os_strncpy(tmp, data, len);
        tmp[len] = 0;
        bufflen += os_sprintf(buff + bufflen, "%s=%s ", m->field, tmp);
    }
    return HTTPD_OK;
}


static ICACHE_FLASH_ATTR
httpd_err_t demo_multipart(struct httpd_session *s) {
    bufflen = 0;
    return httpd_form_multipart_parse(s, _multipart_cb);
}


static ICACHE_FLASH_ATTR
httpd_err_t _form_cb(struct httpd_session *s, const char *field, 
        const char *value) {
    bufflen += os_sprintf(buff + bufflen, "%s=%s ", field, value);
    return HTTPD_OK;
}


static ICACHE_FLASH_ATTR
httpd_err_t demo_urlencoded(struct httpd_session *s) {
    err_t err;
    uint32_t more = HTTPD_REQUESTBODY_REMAINING(s);
    if (more) {
        return HTTPD_OK;
    }
    
    bufflen = 0;
    err = httpd_form_urlencoded_parse(s, _form_cb);
    if (err) {
        return err;
    }
    return HTTPD_RESPONSE_TEXT(s, HTTPSTATUS_OK, buff, bufflen);
}

static ICACHE_FLASH_ATTR
httpd_err_t demo_querystring(struct httpd_session *s) {
    err_t err;
    bufflen = 0;
    err = httpd_querystring_parse(s, _form_cb);
    if (err) {
        return err;
    }
    return HTTPD_RESPONSE_TEXT(s, HTTPSTATUS_OK, buff, bufflen);
}


static ICACHE_FLASH_ATTR
httpd_err_t demo_favicon(struct httpd_session *s) {
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
        HTTPD_RESPONSE_INTERNALSERVERERROR(s);
        return;
    }
    return httpd_response(s, HTTPSTATUS_OK, NULL, 0, 
            HTTPHEADER_CONTENTTYPE_ICON, buffer, FAVICON_SIZE, 
            HTTPD_FLAG_NONE);
}


static ICACHE_FLASH_ATTR
httpd_err_t demo_headersecho(struct httpd_session *s) {
    return httpd_response(s, HTTPSTATUS_OK, s->request.headers, 
            s->request.headerscount, NULL, NULL, 0, false);
}



static ICACHE_FLASH_ATTR
httpd_err_t demo_index(struct httpd_session *s) {
    return HTTPD_RESPONSE_TEXT(s, HTTPSTATUS_OK, "Index", 5);
}


static struct httpd_route routes[] = {
    {"DOWNLOAD",   "/multipartstreams",    demo_download_stream   },
    {"UPLOAD",     "/multipartstreams",    demo_multipart_stream  },
    {"ECHO",       "/multipartforms",      demo_multipart         },
    {"ECHO",       "/urlencodedforms",     demo_urlencoded        },
    {"ECHO",       "/queries",             demo_querystring       },
    {"ECHO",       "/headers",             demo_headersecho       },
    {"GET",        "/favicon.ico",         demo_favicon           },
    {"DOWNLOAD",   "/",                    demo_download          },
    {"GET",        "/",                    demo_index             },
    {"FOTA",       "/",                    reboot_fotamode        },
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

