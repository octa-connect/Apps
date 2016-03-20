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
#if (!defined PLATFORM_EFM32GG_STK3700 && !defined PLATFORM_EFM32HG_STK3400 && !defined PLATFORM_EZR32LG_WSTK6200A)
	#error Mismatch between the configured platform and the actual platform.
#endif

#include "userbutton.h"
#include "platform_sensors.h"
//#include "platform_lcd.h"
// HCI wrapper file
#include "iM880A_RadioInterface.h"
#include "CRC16.h"

#include "em_leuart.h"
#include "em_gpio.h"
#include "em_cmu.h"

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
        //alp_cmd_handler_output_unsollicited_response(alp, alp_command, length+4, 0);
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
    	//lcd_write_string("TX");
    }
    else
    {
        //  error
    	//lcd_write_string("Error");
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

// Toggle different operational modes
void userbutton_callback(button_id_t button_id)
{
	iM880A_SendRadioTelegramwithadress("Button pressed", 14,0xFF,0xFFFF);
	leuart_send_string("AT$SS= 54 45 41 4D 20 50 49 4F");
	leuart_send_byte(0x0D);//CR
	leuart_send_byte(0x0A);//CR
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
#ifdef PLATFORM_EFM32GG_STK3700
  float internal_temp = hw_get_internal_temperature();
  lcd_write_temperature(internal_temp*10, 1);

  uint32_t vdd = hw_get_battery();


  fs_write_file(SENSOR_FILE_ID, 0, (uint8_t*)&internal_temp, sizeof(internal_temp)); // File 0x40 is configured to use D7AActP trigger an ALP action which broadcasts this file data on Access Class 0
#endif

#if (defined PLATFORM_EFM32HG_STK3400  || defined PLATFORM_EZR32LG_WSTK6200A)
  //lcd_clear();
  float internal_temp = hw_get_internal_temperature();
  //lcd_write_string("Int T: %2d.%d C\n", (int)internal_temp, (int)(internal_temp*10)%10);
  //log_print_string("Int T: %2d.%d C\n", (int)internal_temp, (int)(internal_temp*10)%10);

  uint32_t rhData;
  uint32_t tData;
  getHumidityAndTemperature(&rhData, &tData);

  //lcd_write_string("Ext T: %d.%d C\n", (tData/1000), (tData%1000)/100);
  //log_print_string("Temp: %d.%d C\n", (tData/1000), (tData%1000)/100);

  //lcd_write_string("Ext H: %d.%d\n", (rhData/1000), (rhData%1000)/100);
  //log_print_string("Hum: %d.%d\n", (rhData/1000), (rhData%1000)/100);

  uint32_t vdd = hw_get_battery();

  //lcd_write_string("Batt %d mV\n", vdd);
  //log_print_string("Batt: %d mV\n", vdd);

  //TODO: put sensor values in array

  uint8_t sensor_values[8];
  uint16_t *pointer =  (uint16_t*) sensor_values;
  *pointer++ = (uint16_t) (internal_temp * 10);
  *pointer++ = (uint16_t) (tData /100);
  *pointer++ = (uint16_t) (rhData /100);
  *pointer++ = (uint16_t) (vdd /10);

  fs_write_file(SENSOR_FILE_ID, 0, (uint8_t*)&sensor_values,8);
#endif

  timer_post_task_delay(&execute_sensor_measurement, TIMER_TICKS_PER_SEC * 1);
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

//leuart
/* Defining the LEUART1 initialization data */
LEUART_Init_TypeDef leuart0Init =
{
  .enable   = leuartDisable,     /* Activate data reception on LEUn_RX pin. */
  .refFreq  = 0,                  /* Inherit the clock frequenzy from the LEUART clock source */
  .baudrate = 9600,               /* Baudrate = 9600 bps */
  .databits = leuartDatabits8,    /* Each LEUART frame containes 8 databits */
  .parity   = leuartNoParity,     /* No parity bits in use */
  .stopbits = leuartStopbits1,    /* Setting the number of stop bits in a frame to 2 bitperiods */
};
void LEUART0_IRQHandler(void)
{
    char ucRxData = LEUART0->RXDATA;
    //LEUART0->TXDATA = ucRxData;
    // debugger printf
    //printf("IRQ: %c\n",ucRxData);
}

void initLeuart(void)
{
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
	CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);
	CMU_ClockEnable(cmuClock_CORELE,true);
	CMU_ClockEnable(cmuClock_LEUART0,cmuClkDiv_1);
	CMU_ClockEnable(cmuClock_LEUART0, true);


  /* Enable GPIO for LEUART1. RX is on C7 */
//  GPIO_PinModeSet(3,            /* Port D */
//                  5,                    /* Port number */
//                  gpioModeInputPull,    /* Pin mode is set to input only, with pull direction given bellow */
//                  1);                   /* Pull direction is set to pull-up */
  // configure UART TX pin as digital output, initialize high since UART TX
    // idles high (otherwise glitches can occur)
    assert(hw_gpio_configure_pin((pin_id_t){ .port = 3, .pin =  4}, false, gpioModePushPull, 0) == SUCCESS);
    // configure UART RX pin as input (no filter)
    assert(hw_gpio_configure_pin((pin_id_t){ .port = 3, .pin =  5}, false, gpioModeInput, 0) == SUCCESS);

    LEUART_Reset(LEUART0);
	/* Initialize the LEUART */
	LEUART_Init(LEUART0, &leuart0Init);
    LEUART0->ROUTE = LEUART_ROUTE_TXPEN | LEUART_ROUTE_RXPEN| LEUART_ROUTE_LOCATION_LOC0;

    /* Clear previous RX interrupts */
	LEUART_IntClear(LEUART0, LEUART_IF_RXDATAV);
	NVIC_ClearPendingIRQ(LEUART0_IRQn);
	/* Enable RX interrupts */
	LEUART_IntEnable(LEUART0, LEUART_IF_RXDATAV);
	NVIC_EnableIRQ(LEUART0_IRQn);
    /* enable interrupt on every char */
    LEUART0->IEN = LEUART_IEN_RXDATAV;
    LEUART_Enable(LEUART0, leuartEnable);
}

void leuart_send_byte(uint8_t data) {
  while (!(LEUART0->STATUS & LEUART_STATUS_TXBL));
  //while(!(LEUART0->STATUS & (1 << 6))); // wait for TX buffer to empty
  LEUART0->TXDATA = data;
}

void leuart_send_bytes(void const *data, size_t length) {
	for(uint8_t i=0; i<length; i++)	{
		leuart_send_byte(((uint8_t const*)data)[i]);
	}
}

void leuart_send_string(const char *string) {
  leuart_send_bytes(string, strnlen(string, 100));
}
void bootstrap()
{
	//log_print_string("Device booted\n");

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

	    initSensors();
	//console_set_rx_interrupt_callback(&console_rx_cb); //NO CONSOLE
	//console_rx_interrupt_enable();						//NO CONSOLE
	//lora = uart_init(3, 115200, 1);
	//uart_enable(lora);
	//uart_set_rx_interrupt_callback(lora, &lora_rx_cb);
	//uart_rx_interrupt_enable(lora);


	// Initialize radio driver
    iM880A_Init();

	// Register callback functions for receive / send
    iM880A_RegisterRadioCallbacks(CbRxIndication, CbTxIndication);

    initLeuart();
    sched_register_task((&iM880A_Configure));
    timer_post_task_delay(&iM880A_Configure, TIMER_TICKS_PER_SEC);


    /* Setup LEUART with DMA */
    //setupLeuartDma();
    ubutton_register_callback(0, &userbutton_callback);
    ubutton_register_callback(1, &userbutton_callback);

    //sched_register_task((&execute_sensor_measurement));

    //timer_post_task_delay(&execute_sensor_measurement, TIMER_TICKS_PER_SEC * 2);

    //lcd_write_string("Lora Sensor\n");
}



