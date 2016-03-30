#include "em_device.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_system.h"
#include "em_chip.h"    // required for CHIP_Init() function
#include "utilities.h"

int led = 0;
int rgb1 = 0;
int rgb2 = 0;
int main() {
  CHIP_Init();                                               // This function addresses some chip errata and should be called at the start of every EFM32 application (need em_system.c)

  CMU_ClockEnable(cmuClock_GPIO, true);                      // Enable GPIO peripheral clock



  bool changed = false;
  setup_utilities();
  set_led(0,true);
  set_led(1,true);
  set_led(2,true);
  while(1) {
    if(get_button(1) ) {               // If PB0 is pressed
		led++;
		if(led > 7)
			led = 0;
		set_led(0,(led & ( 1 << 0 )) >> 0);
		set_led(1,(led & ( 1 << 1 )) >> 1);
		set_led(2,(led & ( 1 << 2 )) >> 2);
    }
    if(get_button(2) ) {               // If PB1 is pressed
    	if(led == 0)
    		led = 7;
    	else
    		led--;
    	set_led(0,(led & ( 1 << 0 )) >> 0);
    	set_led(1,(led & ( 1 << 1 )) >> 1);
    	set_led(2,(led & ( 1 << 2 )) >> 2);
    }
    if(get_button(3) ) {               // If PB0 is pressed
    		rgb1++;
    		if(rgb1 > 7)
    			rgb1 = 0;
    		set_rgbled(0,(rgb1 & ( 1 << 0 )) >> 0,(rgb1 & ( 1 << 1 )) >> 1,(rgb1 & ( 1 << 2 )) >> 2);
        }
    if(get_button(4) ) {               // If PB1 is pressed
        	rgb2++;
        	if(rgb2 > 7)
        	    rgb2 = 0;
        	set_rgbled(1,(rgb2 & ( 1 << 0 )) >> 0,(rgb2 & ( 1 << 1 )) >> 1,(rgb2 & ( 1 << 2 )) >> 2);
        }

    if(changed){
    switch (led) {
    case 0:
    	set_led(0,true);
    	set_led(1,false);
    	set_led(2,false);
    	changed = false;
    	break;
    case 1:
    	set_led(0,false);
    	set_led(1,true);
    	set_led(2,false);
        changed = false;
        break;
    case 2:
    	set_led(0,false);
    	set_led(1,false);
    	set_led(2,true);
        changed = false;
        break;
    default:
    	set_led(0,false);
    	set_led(1,false);
    	set_led(2,false);
    	changed = false;
    	break;
    }
    }
  }
}
