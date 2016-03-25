/*
 * utilities.c
 *
 * Holds commonly used functions
 */

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "utilities.h"

volatile uint32_t msTicks = 0;
#define DEBOUNCE_TIME		100  // ms

bool pressed1 = false;
int debounce_pressed_timeout1 = 0;
int debounce_released_timeout1 = 0;

bool pressed2 = false;
int debounce_pressed_timeout2 = 0;
int debounce_released_timeout2 = 0;

bool pressed3 = false;
int debounce_pressed_timeout3 = 0;
int debounce_released_timeout3 = 0;

bool pressed4 = false;
int debounce_pressed_timeout4 = 0;
int debounce_released_timeout4 = 0;

void delay(uint32_t milliseconds)
{
	uint32_t start = msTicks;
	while ((start + milliseconds) > msTicks)
		;
}

// Pass in the elapsed time before it times out, returns the future time
int32_t set_timeout_ms(int32_t timeout_ms)
{
	return msTicks + timeout_ms;
}

// Check to see if the future time has elapsed.
int32_t expired_ms(int32_t timeout_ms)
{
	if (timeout_ms < msTicks)
	{
		return true;
	}
	return false;
}

// If a button is low, that means it is pressed, so return true
bool get_button(int number)
{
	if( number == 1)
	{
		 if(! (GPIO_PinInGet(BUTTON_PORT, SET_BUTTON1_PIN)) ) {               // If PB0 is pressed
		    	if (pressed1 == false && expired_ms(debounce_released_timeout1))
		    				{


							pressed1 = true;
							debounce_pressed_timeout1 = set_timeout_ms(DEBOUNCE_TIME);
							return true;
		    				}
		    	//GPIO_PinOutSet(LED_PORT, LED1);                        // Turn on LED1
		    }else{
		    	if (pressed1 == true && expired_ms(debounce_pressed_timeout1))
		    				{
		    					// You could start some process here on the release event
		    					// and it would be immediate

		    					pressed1 = false;
		    					debounce_released_timeout1 = set_timeout_ms(DEBOUNCE_TIME);
		    					return false;
		    				}
		      //GPIO_PinOutClear(LED_PORT, LED1);                      // Turn off LED1
		    }
		 return false;
	}
	if( number == 2)
	{
			 if(! (GPIO_PinInGet(BUTTON_PORT, SET_BUTTON2_PIN)) ) {               // If PB0 is pressed
			    	if (pressed2 == false && expired_ms(debounce_released_timeout2))
			    				{


								pressed2 = true;
								debounce_pressed_timeout2 = set_timeout_ms(DEBOUNCE_TIME);
								return true;
			    				}
			    	//GPIO_PinOutSet(LED_PORT, LED1);                        // Turn on LED1
			    }else{
			    	if (pressed2 == true && expired_ms(debounce_pressed_timeout2))
			    				{
			    					// You could start some process here on the release event
			    					// and it would be immediate

			    					pressed2 = false;
			    					debounce_released_timeout2 = set_timeout_ms(DEBOUNCE_TIME);
			    					return false;
			    				}
			      //GPIO_PinOutClear(LED_PORT, LED1);                      // Turn off LED1
			    }
			 return false;
		}
	if( number == 3)
		{
				 if(! (GPIO_PinInGet(BUTTON_PORT2, SET_BUTTON1_PIN2)) ) {               // If PB0 is pressed
				    	if (pressed3 == false && expired_ms(debounce_released_timeout3))
				    				{
									pressed3 = true;
									debounce_pressed_timeout3 = set_timeout_ms(DEBOUNCE_TIME);
									return true;
				    				}
				    	//GPIO_PinOutSet(LED_PORT, LED1);                        // Turn on LED1
				    }else{
				    	if (pressed3 == true && expired_ms(debounce_pressed_timeout3))
				    				{
				    					pressed3 = false;
				    					debounce_released_timeout3 = set_timeout_ms(DEBOUNCE_TIME);
				    					return false;
				    				}
				      //GPIO_PinOutClear(LED_PORT, LED1);                      // Turn off LED1
				    }
				 return false;
			}
	if( number == 4)
		{
				 if(! (GPIO_PinInGet(BUTTON_PORT2, SET_BUTTON2_PIN2)) ) {               // If PB0 is pressed
				    	if (pressed4 == false && expired_ms(debounce_released_timeout4))
				    				{
									pressed4 = true;
									debounce_pressed_timeout4 = set_timeout_ms(DEBOUNCE_TIME);
									return true;
				    				}
				    	//GPIO_PinOutSet(LED_PORT, LED1);                        // Turn on LED1
				    }else{
				    	if (pressed4 == true && expired_ms(debounce_pressed_timeout4))
				    				{
				    					pressed4 = false;
				    					debounce_released_timeout4 = set_timeout_ms(DEBOUNCE_TIME);
				    					return false;
				    				}
				      //GPIO_PinOutClear(LED_PORT, LED1);                      // Turn off LED1
				    }
				 return false;
			}
}

void set_led(int number, bool level)
{
	if (number == 0)
	{	if(level)
			GPIO_PinOutSet(LED_PORT, LED0_PIN);
		else
		    GPIO_PinOutClear(LED_PORT, LED0_PIN);
	}

	if (number == 1)
	{
		if(level)
			GPIO_PinOutSet(LED_PORT, LED1_PIN);
		else
			GPIO_PinOutClear(LED_PORT, LED1_PIN);
	}
	if (number == 2)
	{
		if(level)
			GPIO_PinOutSet(LED_PORT, LED2_PIN);
		else
			GPIO_PinOutClear(LED_PORT, LED2_PIN);
	}
}
void set_rgbled(int number, bool red, bool green, bool blue)
{
	if (number == 0)
	{	if(red)
			GPIO_PinOutSet(RGB0_PORT, RGB0_RED_PIN);
		else
		    GPIO_PinOutClear(RGB0_PORT, RGB0_RED_PIN);
		if(green)
			GPIO_PinOutSet(RGB0_PORT, RGB0_GREEN_PIN);
		else
			GPIO_PinOutClear(RGB0_PORT, RGB0_GREEN_PIN);
		if(blue)
			GPIO_PinOutSet(RGB0_PORT, RGB0_BLUE_PIN);
		else
			GPIO_PinOutClear(RGB0_PORT, RGB0_BLUE_PIN);
	}

	if (number == 1)
		{	if(red)
				GPIO_PinOutSet(RGB1_PORT, RGB1_RED_PIN);
			else
			    GPIO_PinOutClear(RGB1_PORT, RGB1_RED_PIN);
			if(green)
				GPIO_PinOutSet(RGB1_PORT, RGB1_GREEN_PIN);
			else
				GPIO_PinOutClear(RGB1_PORT, RGB1_GREEN_PIN);
			if(blue)
				GPIO_PinOutSet(RGB1_PORT, RGB1_BLUE_PIN);
			else
				GPIO_PinOutClear(RGB1_PORT, RGB1_BLUE_PIN);
		}
}


void setup_utilities()
{
	//GPIO_PinModeSet(LED_PORT, LED0_PIN, gpioModePushPull, 0);      // Configure LED0 pin as digital output (push-pull)
	GPIO_PinModeSet(LED_PORT, LED0_PIN, gpioModePushPullDrive, 0);
	GPIO_PinModeSet(LED_PORT, LED1_PIN, gpioModePushPullDrive, 0); // Configure LED1 pin as digital output (push-pull) with drive-strength to lowest setting
	GPIO_PinModeSet(LED_PORT, LED2_PIN, gpioModePushPullDrive, 0);
	GPIO_DriveModeSet(LED_PORT, gpioDriveModeLow);          // Set DRIVEMODE to lowest setting (0.5 mA) for all LEDs configured with alternate drive strength

	GPIO_PinModeSet(RGB0_PORT, RGB0_RED_PIN, gpioModeWiredAnd, 1);
	GPIO_PinModeSet(RGB0_PORT, RGB0_GREEN_PIN, gpioModeWiredAnd, 1); // Configure LED1 pin as digital output (push-pull) with drive-strength to lowest setting
	GPIO_PinModeSet(RGB0_PORT, RGB0_BLUE_PIN, gpioModeWiredAnd, 1);

	GPIO_PinModeSet(RGB1_PORT, RGB1_RED_PIN, gpioModeWiredAnd, 1);
	GPIO_PinModeSet(RGB1_PORT, RGB1_GREEN_PIN, gpioModeWiredAnd, 1); // Configure LED1 pin as digital output (push-pull) with drive-strength to lowest setting
	GPIO_PinModeSet(RGB1_PORT, RGB1_BLUE_PIN, gpioModeWiredAnd, 1);

	GPIO_PinModeSet(BUTTON_PORT, SET_BUTTON1_PIN, gpioModeInputPull, 1);   // Configure PB0 as input with pull-up enabled
	GPIO_PinModeSet(BUTTON_PORT, SET_BUTTON2_PIN, gpioModeInputPull, 1);   // Configure PB1 as input with pull-up enabled

	GPIO_PinModeSet(BUTTON_PORT2, SET_BUTTON1_PIN2, gpioModeInputPull, 1);   // Configure PB0 as input with pull-up enabled
	GPIO_PinModeSet(BUTTON_PORT2, SET_BUTTON2_PIN2, gpioModeInputPull, 1);   // Configure PB1 as input with pull-up enabled
	if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000))
	{
		DEBUG_BREAK;
	}
}

void SysTick_Handler(void)
{
	msTicks++;
}
