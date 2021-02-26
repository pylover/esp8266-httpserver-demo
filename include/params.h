#ifndef PARAMS_H
#define PARAMS_H

#include "debug.h"
#include "partition.h"

#include <c_types.h>
#include <user_interface.h>


#define PARAMS_SECTOR SYSTEM_PARTITION_PARAMS_ADDR / 4096 

#define PARAMS_MAGIC 'Q'
#define PARAMS_DEFAULT_ZONE "home"
#define PARAMS_DEFAULT_NAME "NewDevice"

#define PARAMS_PRINT(p) INFO("\r\n%s.%s, ssid: %s psk: %s ap-psk: %s\r\n", \
			p.zone, \
			p.name, \
			p.station_ssid, \
			p.station_psk, \
			p.ap_psk \
		)

#ifndef PARAMS_ZONE_MAXLEN
#define PARAMS_ZONE_MAXLEN  32
#endif

#ifndef PARAMS_NAME_MAXLEN
#define PARAMS_NAME_MAXLEN  32
#endif


typedef struct {
	 char zone[PARAMS_ZONE_MAXLEN];
	 char name[PARAMS_NAME_MAXLEN];
	 char ap_psk[32];
	 char station_ssid[32];
	 char station_psk[32];
     char apploaded;
	 char magic;
} Params;


bool params_save(Params* params);
bool params_load(Params* params);
bool params_defaults(Params* params);

#endif

