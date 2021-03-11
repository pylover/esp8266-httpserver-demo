#include "params.h"

#include <user_interface.h>
#include <osapi.h>


ICACHE_FLASH_ATTR 
bool params_save(Params* params) {
	params->magic = PARAMS_MAGIC;
	return system_param_save_with_protect(PARAMS_SECTOR, params, 
			sizeof(Params));
}


ICACHE_FLASH_ATTR 
bool params_load(Params* params) {
	bool ok = system_param_load(PARAMS_SECTOR, 0,
			params, sizeof(Params));
	return ok && params->magic == PARAMS_MAGIC;
}


bool ICACHE_FLASH_ATTR 
params_defaults(Params* params) {
    os_memset(params, 0, sizeof(Params));
	os_sprintf(params->zone, PARAMS_DEFAULT_ZONE);
	os_sprintf(params->name, PARAMS_DEFAULT_NAME);
	params->ap_psk[0] = 0;
	params->station_ssid[0] = 0;
	params->station_psk[0] = 0;
    params->apploaded = 0;
	return params_save(params);
}


