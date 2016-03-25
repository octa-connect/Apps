/*
 * utilities.h
 *
 *  Created on: Apr 20, 2015
 *      Author: David
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_

#define LED_PORT					gpioPortA
#define LED0_PIN					8
#define LED1_PIN					9
#define LED2_PIN					10


#define RGB0_PORT					gpioPortB
#define RGB0_BLUE_PIN				3
#define	RGB0_GREEN_PIN				4
#define	RGB0_RED_PIN				5

#define RGB1_PORT					gpioPortC
#define RGB1_BLUE_PIN				2
#define	RGB1_GREEN_PIN				3
#define	RGB1_RED_PIN				4

#define BUTTON_PORT					gpioPortA
#define SET_BUTTON1_PIN				7
#define SET_BUTTON2_PIN				11

#define BUTTON_PORT2					gpioPortB
#define SET_BUTTON1_PIN2				9
#define SET_BUTTON2_PIN2				10


#define DEBUG_BREAK		__asm__("BKPT #0");

void delay(uint32_t milliseconds);
int32_t set_timeout_ms(int32_t timeout_ms);
int32_t expired_ms(int32_t timeout_ms);
bool get_button();
void set_led(int number, bool level);
void set_rgbled(int number, bool red, bool green, bool blue);
void setup_utilities();


#endif /* UTILITIES_H_ */
