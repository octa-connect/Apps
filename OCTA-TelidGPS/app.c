/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hwleds.h"
#include "hwsystem.h"
#include "scheduler.h"
#include "timer.h"
#include "assert.h"
#include "platform.h"

#include <stdio.h>
#include <stdlib.h>

#include "hwlcd.h"
#include "hwadc.h"
#include "d7ap_stack.h"
#include "fs.h"
#include "log.h"

#include "gps.h"
#include "nmea.h"

#if (!defined PLATFORM_EFM32GG_STK3700 && !defined PLATFORM_EFM32HG_STK3400 && !defined PLATFORM_OCTA_GATEWAY)
	#error Mismatch between the configured platform and the actual platform.
#endif

#include "userbutton.h"
//#include "platform_sensors.h"
//#include "platform_lcd.h"

#define SENSOR_FILE_ID           0x40
#define SENSOR_FILE_SIZE         10
#define ACTION_FILE_ID           0x41

#define SENSOR_UPDATE	TIMER_TICKS_PER_SEC * 2

// Toggle different operational modes
void userbutton_callback(button_id_t button_id)
{
	// userbutton callback
}

void execute_sensor_measurement()
{
	uint8_t sensor_values[10];
	gps_position_dd_t position = gps_get_position_dd();
	uint16_t vdd = hw_get_battery();

	log_print_string("GPS: %d - %d", (int16_t) position.latitude, (int16_t)  position.longitude);
	log_print_string("Batt %d mV", vdd);

    uint8_t *pointer =  (uint8_t*) sensor_values;
    memcpy(pointer, (uint8_t*) &position, 8);
    memcpy(pointer+8, (uint8_t*) &vdd, 2);

    fs_write_file(SENSOR_FILE_ID, 0, (uint8_t*)&sensor_values,10);

    timer_post_task_delay(&execute_sensor_measurement, SENSOR_UPDATE);
}

void init_user_files()
{
    // file 0x40: contains our sensor data + configure an action file to be executed upon write
    fs_file_header_t file_header = (fs_file_header_t){
        .file_properties.action_protocol_enabled = 1,
        .file_properties.action_file_id = ACTION_FILE_ID,
        .file_properties.action_condition = ALP_ACT_COND_WRITE,
        .file_properties.storage_class = FS_STORAGE_VOLATILE,
        .file_properties.permissions = 0, // TODO
        .length = SENSOR_FILE_SIZE
    };

    fs_init_file(SENSOR_FILE_ID, &file_header, NULL);

    // configure file notification using D7AActP: write ALP command to broadcast changes made to file 0x40 in file 0x41
    // first generate ALP command consisting of ALP Control header, ALP File Data Request operand and D7ASP interface configuration
    alp_control_t alp_ctrl = {
        .group = false,
        .response_requested = false,
        .operation = ALP_OP_READ_FILE_DATA
    };

    alp_operand_file_data_request_t file_data_request_operand = {
        .file_offset = {
            .file_id = SENSOR_FILE_ID,
            .offset = 0
        },
        .requested_data_length = SENSOR_FILE_SIZE,
    };

    d7asp_fifo_config_t d7asp_fifo_config = {
        .fifo_ctrl_nls = false,
        .fifo_ctrl_stop_on_error = false,
        .fifo_ctrl_preferred = false,
        .fifo_ctrl_state = SESSION_STATE_PENDING,
        .qos = {
            .qos_ctrl_resp_mode = SESSION_RESP_MODE_NONE
        },
        .dormant_timeout = 0,
        .start_id = 0,
        .addressee = {
            .addressee_ctrl_has_id = false,
            .addressee_ctrl_virtual_id = false,
            .addressee_ctrl_access_class = 0,
            .addressee_id = 0
        }
    };

    // finally, register D7AActP file
    fs_init_file_with_D7AActP(ACTION_FILE_ID, &d7asp_fifo_config, &alp_ctrl, (uint8_t*)&file_data_request_operand);
}

void bootstrap()
{
	log_print_string("Device booted\n");

    dae_access_profile_t access_classes[1] = {
        {
            .control_scan_type_is_foreground = false,
            .control_csma_ca_mode = CSMA_CA_MODE_UNC,
            .control_number_of_subbands = 1,
            .subnet = 0x00,
            .scan_automation_period = 0,
            .transmission_timeout_period = 0xFF, // TODO compressed time value
            .subbands[0] = (subband_t){
                .channel_header = {
                    .ch_coding = PHY_CODING_PN9,
                    .ch_class = PHY_CLASS_NORMAL_RATE,
#ifdef PLATFORM_EZR32LG_WSTK6200A
                    .ch_freq_band = PHY_BAND_868
#else
                    .ch_freq_band = PHY_BAND_433
#endif
                },
                .channel_index_start = 16,
                .channel_index_end = 16,
                .eirp = 10,
                .ccao = 0
            }
        }
    };

    fs_init_args_t fs_init_args = (fs_init_args_t){
        .fs_user_files_init_cb = &init_user_files,
        .access_profiles_count = 1,
        .access_profiles = access_classes
    };

    d7ap_stack_init(&fs_init_args, NULL, false);

    //initSensors();
	gps_init();

    ubutton_register_callback(0, &userbutton_callback);
    ubutton_register_callback(1, &userbutton_callback);

    sched_register_task((&execute_sensor_measurement));

    timer_post_task_delay(&execute_sensor_measurement, TIMER_TICKS_PER_SEC);

    char id_hex[30];
    uint64_t id = hw_get_unique_id();
    sprintf(id_hex, "ID: %.8x%.8x\n", id<<8, id);
    log_print_string(id_hex);

	
	gps_activate();
}

