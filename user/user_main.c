// Internal 
#include "common.h"
#include "user_config.h"
#include "partition.h"
#include "wifi.h"
#include "params.h" 
#include "status.h"
#include "uns.h"
#include "interrupt.h"

// SDK
#include <ets_sys.h>
#include <osapi.h>
#include <mem.h>
#include <user_interface.h>
#include <driver/uart.h>
#include <upgrade.h>
#include <c_types.h>
#include <ip_addr.h> 
#include <espconn.h>


static bool configured;
static Params params;


static ICACHE_FLASH_ATTR 
void reboot_appmode() {
	system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
	system_upgrade_reboot();
}


void wifi_connect_cb(uint8_t status) {
    if(status == STATION_GOT_IP) {
        char hostname[UNS_HOSTNAME_MAXLEN];
        os_sprintf(hostname, "%s.%s", params.zone, params.name);
        uns_init(hostname);
        INFO("WIFI Connected to: %s\r\n", params.station_ssid);
        wifi_ap_stop();

        if (params.apploaded && (REBOOTDELAY > 0)) {
            INFO("Reboot in %d seconds\r\n", REBOOTDELAY);
            status_update(500, 500, REBOOTDELAY, reboot_appmode);
        }
    } 
    else {
        uns_deinit();    
        INFO("WIFI Disonnected from: %s\r\n", params.station_ssid);
        wifi_ap_start();
    }
}

//static ICACHE_FLASH_ATTR
//void startweb() {
//	webadmin_start();
//}
//static ICACHE_FLASH_ATTR
//void stopweb() {
//	webadmin_stop();
//    status_update(500, 500, 10, startweb);
//}

ICACHE_FLASH_ATTR
void boothello() {
    INFO(__name__" version: "__version__"\r\n");
    INFO("My full name is: %s.%s\r\n", params.zone, params.name);
    if (!configured) {
        INFO(
            "Connect to WIFI Access point: %s, "
            "open http://192.168.43.1 to configure me.\r\n",
            params.name
        );
    }
    status_update(1000, 1000, INFINITE, NULL);
    //status_update(1000, 1000, 10, stopweb);

    /* Web UI */
	webadmin_start();

}


void user_init(void) {
    //uart_init(BIT_RATE_115200, BIT_RATE_115200);
    uart_div_modify(UART0, UART_CLK_FREQ / BIT_RATE_115200);
    uart_rx_intr_disable(UART0);
    uart_rx_intr_disable(UART1);
    
    /* Uncomment and edit the interrupt.c to configure interrupts */
    //interrupt_init();

	configured = params_load(&params);
	if (!configured) {
		ERROR("Cannot load Params\r\n");
		if(!params_defaults(&params)) {
			ERROR("Cannot save params\r\n");
			return;
		}
	}
    
    PARAMS_PRINT(params);
	
    // Status LED
    status_init();

    /* Start WIFI */
    wifi_start(&params, wifi_connect_cb);

    // Disable wifi led before infrared
    wifi_status_led_uninstall();

    status_update(100, 500, 5, boothello);
}


ICACHE_FLASH_ATTR 
void user_pre_init(void) {
    if(!system_partition_table_regist(at_partition_table, 
				sizeof(at_partition_table)/sizeof(at_partition_table[0]),
				SPI_FLASH_SIZE_MAP)) {
		ERROR("system_partition_table_regist fail");
		while(1);
	}
}

