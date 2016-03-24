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
#include "console.h"
#include "d7ap_stack.h"
#include "fs.h"
#include "hwuart.h"
#if (!defined PLATFORM_EFM32GG_STK3700 && !defined PLATFORM_EFM32HG_STK3400 && !defined PLATFORM_OCTA_GATEWAY)
	#error Mismatch between the configured platform and the actual platform.
#endif

#include "userbutton.h"
#include "em_gpio.h"
#include "em_cmu.h"

uart_handle_t* uart;

// Toggle different operational modes
void userbutton_callback(button_id_t button_id)
{
	uart_send_string(uart, "AT");
    uart_send_byte(uart, 0x0D);//CR
	uart_send_byte(uart, 0x0A);//CR
}

void uart_ProcessRxByte(uint8_t Byte)
{
    console_print_byte(Byte);
}
void console_rx_cb(uint8_t Byte)
{
    uart_send_byte(uart, Byte);
}

void bootstrap()
{
    console_set_rx_interrupt_callback(console_rx_cb);
	console_print("Device booted\n");
	uart = uart_init(0, 9600, 0); //Define Ports, Baudrate and pins
    uart_enable(uart);
	uart_set_rx_interrupt_callback(uart, &uart_ProcessRxByte);
	uart_rx_interrupt_enable(uart);
    
    /* Setup LEUART with DMA */
    ubutton_register_callback(0, &userbutton_callback);
    ubutton_register_callback(1, &userbutton_callback);

}



