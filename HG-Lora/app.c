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
#include "log.h"

#include "hwuart.h"
#if (!defined PLATFORM_EFM32GG_STK3700 && !defined PLATFORM_EFM32HG_STK3400 && !defined PLATFORM_EZR32LG_WSTK6200A)
	#error Mismatch between the configured platform and the actual platform.
#endif

#include "userbutton.h"
#include "platform_sensors.h"
#include "platform_lcd.h"
// HCI wrapper file
#include "iM880A_RadioInterface.h"
#include "CRC16.h"

#define SENDALP
//#define PASSTROUGH
//------------------------------------------------------------------------------
//
//	Section Defines
//
//------------------------------------------------------------------------------
// SLIP Protocol Characters
#define SLIP_END					0xC0
#define	SLIP_ESC					0xDB
#define	SLIP_ESC_END				0xDC
#define	SLIP_ESC_ESC				0xDD

#define TX_LENGTH               13

#define POWERUP_DELAY           500     // ms

#define MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE 100
#define ALP_CMD_HANDLER_ID 'D'
//------------------------------------------------------------------------------
//
//	Section Typedefs
//
//------------------------------------------------------------------------------
typedef enum
{
    SEND_MSG                        = 0x0001,
    POWER_UP                        = 0x0002,
    MSG_RECEIVED                    = 0x0004,

}TMainEventCode;

typedef enum {
    SESSION_STATE_IDLE = 0x00,
    SESSION_STATE_DORMANT = 0x01,
    SESSION_STATE_PENDING = 0x02,
    SESSION_STATE_ACTIVE = 0x03,
    SESSION_STATE_DONE = 0x04,
} session_state_t;
typedef struct {
    union {
        uint8_t raw;
        struct {
            session_state_t session_state : 3;
            uint8_t _rfu : 2;
            bool retry : 1;
            bool missed : 1;
            bool nls : 1;
        };
    };
} d7asp_state_t;
typedef struct {
    union {
        uint8_t addressee_ctrl;
        struct {
            uint8_t addressee_ctrl_access_class : 4;
            bool addressee_ctrl_virtual_id : 1;
            bool addressee_ctrl_has_id : 1; // TODO: in spec v1.0 this is defined as UCAST but will be changed in next revision
            uint8_t _rfu : 2;
        };
    };
    uint8_t addressee_id[8]; // TODO assuming 8 byte id for now
} d7atp_addressee_t;

typedef struct {
    d7asp_state_t status;
    uint8_t fifo_token;
    uint8_t request_id;
    uint8_t response_to;
    d7atp_addressee_t* addressee;
} d7asp_result_t;
//------------------------------------------------------------------------------
//
//	Section RAM
//
//------------------------------------------------------------------------------

uint32_t mainEvent = 0;

uint8_t txBuffer[TX_LENGTH] = {'"','H','e','l','l','o',' ','W','o','l','d','!','"'};


//------------------------------------------------------------------------------
//
//	CbRxIndication
//
//------------------------------------------------------------------------------
void alp_cmd_handler_output_unsollicited_response(d7asp_result_t d7asp_result, uint8_t *alp_command, uint8_t alp_command_size, hw_rx_metadata_t* rx_meta)
{
    uint8_t data[1 + sizeof(hw_rx_metadata_t) + MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE] = { 0x00 };
    uint8_t* ptr = data;
    // TODO transmit ALP interface ID as well?
    // TODO rx meta
    (*ptr) = d7asp_result.status.raw; ptr++;
    (*ptr) = d7asp_result.fifo_token; ptr++;
    (*ptr) = d7asp_result.request_id; ptr++;
    (*ptr) = d7asp_result.response_to; ptr++;
    (*ptr) = d7asp_result.addressee->addressee_ctrl; ptr++;
    uint8_t address_len = d7asp_result.addressee->addressee_ctrl_virtual_id? 2 : 8; // TODO according to spec this can be 1 byte as well?
    memcpy(ptr, d7asp_result.addressee->addressee_id, address_len); ptr += address_len;
    memcpy(ptr, alp_command, alp_command_size); ptr+= alp_command_size;

    shell_return_output(ALP_CMD_HANDLER_ID, data, ptr - data);
}
static void
CbRxIndication     (uint8_t* rxMsg,uint8_t  length,TRadioFlags rxFlags)
{
    if(rxFlags == trx_RXDone)
    {
        mainEvent |= MSG_RECEIVED;  // Radio Msg received
        //Send Original Message with SLIP and with CRC
#ifdef SENDALP
        //console_print_byte(32); // RETURN_FILE_DATA(32)
        //console_print_byte(100);
        //console_print_byte(0);
        //console_print_byte(length);

        d7atp_addressee_t address;
        address.addressee_ctrl_has_id = 1;
        address.addressee_ctrl_virtual_id = 0;
        address.addressee_id[0] = 0x00;
        address.addressee_id[1] = 0x00;
        address.addressee_id[2] = 0x00;
        address.addressee_id[3] = 0x00;
        address.addressee_id[4] = 0x00;
        address.addressee_id[5] = rxMsg[6];
        address.addressee_id[6] = rxMsg[8];
        address.addressee_id[7] = rxMsg[7];
        d7asp_state_t state;
        state.raw = 0;
        //create d7ap
        d7asp_result_t alp;
        alp.status = state;
        alp.fifo_token = 0x00;
        alp.request_id = 0x00;
        alp.response_to = 0x00;
        alp.addressee = &address;

        uint8_t alp_command[length+4];
        alp_command[0] = 32;
        alp_command[1] = 100;
        alp_command[2] = 0;
        alp_command[3] = length;
        for(int i=0;i<length;i++)
        	alp_command[i+4] = rxMsg[i];
        alp_cmd_handler_output_unsollicited_response(alp, alp_command, length+4, 0);
        //console_print_bytes(rxMsg,length);
#else
        //console_print_byte(0xC0);
        //console_print_bytes(rxMsg,length);
        //console_print_byte(0xC0);
#endif





    }
}


//------------------------------------------------------------------------------
//
//	CbTxIndication
//
//------------------------------------------------------------------------------
static void
CbTxIndication     (TRadioMsg*     txMsg,
                    TRadioFlags     txFlags)
{
    if(txFlags == trx_TXDone)
    {
        // TX was successfull
    }
    else
    {
        //  error
    }
}

uint8_t app_mode_status = 0xFF;
uint8_t app_mode_status_changed = 0x00;
uint8_t app_mode = 0;
//uart_handle_t* lora;

bool handeling_lora_message = false;
uint8_t lora_message_count = 0;
uint8_t lora_message[50];
uint8_t lora_message_length = 0;

// Toggle different operational modes
void userbutton_callback(button_id_t button_id)
{
	console_print("Button Pressed\n");
	//uart_send_string(lora, "Button Pressed\n");
	#ifdef PLATFORM_EFM32GG_STK3700
		lcd_write_string("Butt %d", button_id);
	#else
		lcd_write_string("button: %d\n", button_id);
	#endif
}
void console_rx_cb(uint8_t byte){
	#ifdef PASSTROUGH
		uart_send_byte(lora, byte);
	#else
		//uart_send_byte(lora, byte);
	#endif
}

void bootstrap()
{
	//log_print_string("Device booted\n");
	//console_print("Device booted\n");
	console_set_rx_interrupt_callback(&console_rx_cb);
	console_rx_interrupt_enable();
	//lora = uart_init(3, 115200, 1);
	//uart_enable(lora);
	//uart_set_rx_interrupt_callback(lora, &lora_rx_cb);
	//uart_rx_interrupt_enable(lora);


	// Initialize radio driver
    iM880A_Init();

	// Register callback functions for receive / send
    iM880A_RegisterRadioCallbacks(CbRxIndication, CbTxIndication);

    sched_register_task((&iM880A_Configure));
    timer_post_task_delay(&iM880A_Configure, TIMER_TICKS_PER_SEC * 500);
	 // Set configuration parameters
	 //iM880A_Configure();

    ubutton_register_callback(0, &userbutton_callback);
    ubutton_register_callback(1, &userbutton_callback);

    //sched_register_task((&execute_sensor_measurement));

    //timer_post_task_delay(&execute_sensor_measurement, TIMER_TICKS_PER_SEC * 1);

    lcd_write_string("LORA");
}



