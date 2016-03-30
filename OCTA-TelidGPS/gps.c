// GPS over serial connection handling
// author: Christophe VG <contact@christophe.vg>

#include <string.h>
#include <stdbool.h>

#include "types.h"
#include "fifo.h"
#include "debug.h"
#include "errors.h"
#include "scheduler.h"

#include "console.h"
#include "nmea.h"
#include "gps.h"


//#define DEBUG_GPS

#ifdef PLATFORM_EFM32GG_STK3700
#define GPS_UART		1
#define GPS_LOCATION	3
#endif

#ifdef PLATFORM_OCTA_GATEWAY
#define GPS_UART		4
#define GPS_LOCATION	1
#endif

#define GPS_BAUDRATE	9600


#define GPS_BUFFER_SIZE 512

static uint8_t gps_buffer[GPS_BUFFER_SIZE] = { 0 };
static fifo_t  gps_fifo;

static bool _active = false;

static void _process_gps_fifo() {
  while(fifo_get_size(&gps_fifo) > 0) {
    uint8_t byte;
    fifo_pop(&gps_fifo, &byte, 1);
    // feed GPS input to NMEA parser
    nmea_parse(byte);
#ifdef DEBUG_GPS
    console_print_byte(byte);
#endif
  }
  // continue processing
  sched_post_task(&_process_gps_fifo);
}

static void _gps_cb(uint8_t data) {
  if(! _active ) { return; }
  error_t err;
  err = fifo_put(&gps_fifo, &data, 1); assert(err == SUCCESS);
  if(!sched_is_scheduled(&_process_gps_fifo)) {
    sched_post_task(&_process_gps_fifo);
  }
}

static uart_handle_t* uart;

bool gps_init() {
  uart = uart_init(GPS_UART, GPS_BAUDRATE, GPS_LOCATION);
  uart_enable(uart);
  fifo_init(&gps_fifo, gps_buffer, sizeof(gps_buffer));
  uart_set_rx_interrupt_callback(uart, &_gps_cb);
  uart_rx_interrupt_enable(uart);
  sched_register_task(&_process_gps_fifo);
  return true;
}

void gps_activate() {
  _active = true;
}

void gps_deactivate() {
  _active = false;
}

// decimal degrees = degrees + minutes/60 + seconds/3600
// N/E are positive, S/W negative

gps_position_dd_t gps_get_position_dd() {
  gps_position_dd_t dd;
  nmea_position_t   pos = nmea_get_position();
  dd.latitude = pos.latitude.deg + pos.latitude.min/60.0;
  if(pos.latitude.ns == 's') {
    dd.latitude *= -1;
  }
  dd.longitude = pos.longitude.deg + pos.longitude.min/60.0;
  if(pos.longitude.ew == 'w') {
    dd.longitude *= -1;
  }
  return dd;
}
