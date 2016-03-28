// GPS implementation, using _only_ the RX part of (the default) UART(0)
// author: Christophe VG <contact@christophe.vg>

#ifndef __GPS_H
#define __GPS_H

#include "nmea.h"

bool gps_init(void);

//void gps_firmware_version(void);

// wrappers around nmea parser
#define gps_position_t    nmea_position_t
#define gps_have_position nmea_have_position
#define gps_get_position  nmea_get_position

typedef struct gps_position_dd {
  float latitude;
  float longitude;
} gps_position_dd_t;

gps_position_dd_t gps_get_position_dd(void);

void gps_activate(void);
void gps_deactivate(void);

#endif
