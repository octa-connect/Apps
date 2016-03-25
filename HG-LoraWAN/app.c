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
//#include <stdio.h>
//#include <stdlib.h>

#include "hwadc.h"

#include "d7ap_stack.h"
#include "fs.h"
#include "hwuart.h"
#if (!defined PLATFORM_EFM32GG_STK3700 && !defined PLATFORM_EFM32HG_STK3400 && !defined PLATFORM_OCTA_GATEWAY)
	#error Mismatch between the configured platform and the actual platform.
#endif

#include "userbutton.h"
//#include "platform_sensors.h"
//#include "platform_lcd.h"
// HCI wrapper file
#include "iM880A_RadioInterface.h"
#include "CRC16.h"
#include "console.h"
#include "em_gpio.h"
#include "em_cmu.h"
#define SENSOR_UPDATE	TIMER_TICKS_PER_SEC * 1
#define SENDALP
//#define PASSTROUGH

//------------------------------------------------------------------------------
//
//	Section Defines
//
//------------------------------------------------------------------------------

#define TX_LENGTH                       16
#define POWERUP_DELAY                   500         // ms
#define TX_DELAY                        10000       // ms
#define DEVICE_ADDR                     0x00000002


// select activation method
//
#define ACTIVATION_METHOD_DIRECT        1
//#define ACTIVATION_METHOD_OTA           1

// select telegram type
//
//#define UNCONFIRMED_DATA_TELEGRAMS      1
#define CONFIRMED_DATA_TELEGRAMS        1

#define MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE 100
#define ALP_CMD_HANDLER_ID 'D'

uart_handle_t* sigfox;
//------------------------------------------------------------------------------
//
//	Section Typedefs
//
//------------------------------------------------------------------------------

typedef enum
{
    POWER_UP                        = 0x0001,
    MSG_SENT                        = 0x0002,
    MSG_RECEIVED                    = 0x0004,
    ACK_RECEIVED                    = 0x0008,

}TMainEventCode;

//------------------------------------------------------------------------------
//
//	Section RAM
//
//------------------------------------------------------------------------------

uint32_t mainEvent = 0;


static uint8_t txBuffer[TX_LENGTH] = {0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

#if defined(ACTIVATION_METHOD_DIRECT)

// Direct Activation Parameters

static uint8_t nwkSessionKey[KEY_LEN] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

static uint8_t appSessionKey[KEY_LEN] = {0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00 };

#else

// OTA Parameters

static uint8_t appEUI[EUI_LEN]        = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22 };

static uint8_t deviceEUI[EUI_LEN]     = {0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28 };

static uint8_t deviceKey[KEY_LEN]     = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 };

#endif

//------------------------------------------------------------------------------
//
//	CbDevMgmtHCIResponse
//
//------------------------------------------------------------------------------
//!
//! @brief: Handle HCI response messages of SAP DEVMGMT_ID
//!
//------------------------------------------------------------------------------
static void
CbDevMgmtHCIResponse(uint8_t          msgID,
                     uint8_t*         msg,
                     uint8_t          length)
{
    switch(msgID)
    {
        case    DEVMGMT_MSG_PING_RSP:
                // handle ping response here
                break;

        default:
                // handle unsupported MsgIDs here
                break;
    }
}

//------------------------------------------------------------------------------
//
//	CbMsgIndication
//
//------------------------------------------------------------------------------
//!
//! @brief: Handle TX/RX/ACK radio messages
//!
//------------------------------------------------------------------------------
static void
CbMsgIndication(uint8_t*          msg,
                uint8_t           length,
                TRadioFlags     trxFlags)
{
    if(trxFlags == trx_RXDone)
    {
        // Radio Msg received
        mainEvent |= MSG_RECEIVED;
    }
    else if (trxFlags == trx_TXDone)
    {
        // TX was successfull
        mainEvent |= MSG_SENT;
    }
    else if (trxFlags == trx_ACKDone)
    {
        // Ack received
        mainEvent |= ACK_RECEIVED;
    }
}

//------------------------------------------------------------------------------
//
//	CbLoRaWANHCIResponse
//
//------------------------------------------------------------------------------
//!
//! @brief: Handle HCI response messages of SAP LORAWAN_ID
//!
//------------------------------------------------------------------------------
static void
CbLoRaWANHCIResponse(uint8_t          msgID,
                     uint8_t*         msg,
                     uint8_t          length)
{
    switch(msgID)
    {
        case    LORAWAN_MSG_ACTIVATE_DEVICE_RSP:
                break;

        case    LORAWAN_MSG_SET_JOIN_PARAM_RSP:
                break;

        case    LORAWAN_MSG_SEND_UDATA_RSP:
                break;

        case    LORAWAN_MSG_SEND_CDATA_RSP:
                break;

        case    LORAWAN_MSG_GET_STATUS_RSP:
                break;

        default:
                // handle unsupported MsgIDs here
                break;
    }
}

#define SENSOR_FILE_ID           0x40
#define SENSOR_FILE_SIZE         8
#define ACTION_FILE_ID           0x41

#define APP_MODE_LEDS		1
#define APP_MODE_LCD		1 << 1
#define APP_MODE_CONSOLE	1 << 2

uint8_t app_mode_status = 0xFF;
uint8_t app_mode_status_changed = 0x00;
uint8_t app_mode = 0;
void send_message()
{
    //Sigfox
    uart_send_string(sigfox,"AT");
    uart_send_byte(sigfox, 0x0D);//CR
	uart_send_byte(sigfox, 0x0A);//CR
    
    //LoraWAN
#if defined(UNCONFIRMED_DATA_TELEGRAMS)

        // Unreliable Data Transmission
        if(iM880A_SendUDataTelegram(txBuffer, TX_LENGTH) != WiMODLR_RESULT_OK)
               ; // handle faults

#elif defined(CONFIRMED_DATA_TELEGRAMS)

        // Confirmed Data Transmission
        if(iM880A_SendCDataTelegram(txBuffer, TX_LENGTH) != WiMODLR_RESULT_OK)
               ; // handle faults

#endif
    //Dash7
    float internal_temp = hw_get_internal_temperature();
	  //lcd_write_temperature(internal_temp*10, 1);

	fs_write_file(SENSOR_FILE_ID, 0, (uint8_t*)&internal_temp, sizeof(internal_temp)); // File 0x40 is configured to use D7AActP trigger an ALP action which broadcasts this file data on Access Class 0


}
// Toggle different operational modes
void userbutton_callback(button_id_t button_id)
{
	send_message();
	
	//uart_send_string(lora, "Button Pressed\n");
	#ifdef PLATFORM_EFM32GG_STK3700
		lcd_write_string("Butt %d", button_id);
	#else
		//lcd_write_string("button: %d\n", button_id);
	#endif
}
void console_rx_cb(uint8_t byte){
	#ifdef PASSTROUGH
		uart_send_byte(lora, byte);
	#else
		//uart_send_byte(lora, byte);
	#endif
}

void execute_sensor_measurement()
{
#if (defined PLATFORM_EFM32GG_STK3700 || defined PLATFORM_OCTA_GATEWAY)
  float internal_temp = hw_get_internal_temperature();
  //lcd_write_temperature(internal_temp*10, 1);

  uint32_t vdd = hw_get_battery();


  fs_write_file(SENSOR_FILE_ID, 0, (uint8_t*)&internal_temp, sizeof(internal_temp)); // File 0x40 is configured to use D7AActP trigger an ALP action which broadcasts this file data on Access Class 0
#endif

#if (defined PLATFORM_EFM32HG_STK3400  || defined PLATFORM_EZR32LG_WSTK6200A)
  char str[30];

  float internal_temp = hw_get_internal_temperature();
  //sprintf(str, "Int T: %2d.%d C", (int)internal_temp, (int)(internal_temp*10)%10);
  //lcd_write_line(2,str);
  //log_print_string(str);

  uint32_t rhData;
  uint32_t tData;
  getHumidityAndTemperature(&rhData, &tData);

  //sprintf(str, "Ext T: %d.%d C", (tData/1000), (tData%1000)/100);
  //lcd_write_line(3,str);
  //log_print_string(str);

  //sprintf(str, "Ext H: %d.%d", (rhData/1000), (rhData%1000)/100);
  //lcd_write_line(4,str);
  //log_print_string(str);

  uint32_t vdd = hw_get_battery();

  //sprintf(str, "Batt %d mV", vdd);
 // lcd_write_line(5,str);
  //log_print_string(str);

  //TODO: put sensor values in array

  uint8_t sensor_values[8];
  uint16_t *pointer =  (uint16_t*) sensor_values;
  *pointer++ = (uint16_t) (internal_temp * 10);
  *pointer++ = (uint16_t) (tData /100);
  *pointer++ = (uint16_t) (rhData /100);
  *pointer++ = (uint16_t) (vdd /10);

  fs_write_file(SENSOR_FILE_ID, 0, (uint8_t*)&sensor_values,8);
#endif

  timer_post_task_delay(&execute_sensor_measurement, TIMER_TICKS_PER_SEC);
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



static d7asp_init_args_t d7asp_init_args;

void iM880A_setup()
{
	// Test connection
	  iM880A_PingRequest();

	#if defined(ACTIVATION_METHOD_DIRECT)

	    // Direct Device Activation
	    //
//	    iM880A_DirectDeviceActivation(DEVICE_ADDR, nwkSessionKey, appSessionKey);

	#else

	    // Wireless Network Activation (OTA)
	    //
	    iM880A_SetJoinParameters(appEUI, deviceEUI, deviceKey);

	    iM880A_JoinNetworkRequest();

	#endif

}



void Sigfox_ProcessRxByte(uint8_t Byte)
{
    console_print_byte(Byte);
}
void bootstrap()
{
    console_set_rx_interrupt_callback(console_rx_cb);
	//console_print("Device booted\n");
    //console_print("ID: ");
	uint8_t id[8];
	uint64_t id64 = hw_get_unique_id();
	// convert from an unsigned long int to a 4-byte array
	 id[0] = (uint8_t)((id64 >> 56) & 0xFF) ;
	 id[1] = (uint8_t)((id64 >> 48) & 0xFF) ;
	 id[2] = (uint8_t)((id64 >> 40) & 0XFF);
	 id[3] = (uint8_t)((id64 >> 32)& 0XFF);
	 id[4] = (uint8_t)((id64 >> 24) & 0xFF) ;
	 id[5] = (uint8_t)((id64 >> 16) & 0xFF) ;
	 id[6] = (uint8_t)((id64 >> 8) & 0XFF);
	 id[7] = (uint8_t)((id64 & 0XFF));
	console_print_bytes(id,8);
    console_print("\n");
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

	    d7ap_stack_init(&fs_init_args, 0, false);

	    //initSensors();


	// Initialize radio driver
    iM880A_Init();

	// Register callback functions for receive / send
    iM880A_RegisterRadioCallbacks(CbMsgIndication, CbLoRaWANHCIResponse, CbDevMgmtHCIResponse);

    sched_register_task((&iM880A_setup));
    timer_post_task_delay(&iM880A_setup, TIMER_TICKS_PER_SEC * 1);

    sigfox = uart_init(1, 9600, 3);
	uart_enable(sigfox);
	uart_set_rx_interrupt_callback(sigfox, &Sigfox_ProcessRxByte);
	uart_rx_interrupt_enable(sigfox);

    /* Setup LEUART with DMA */
    ubutton_register_callback(0, &userbutton_callback);
    ubutton_register_callback(1, &userbutton_callback);

}



