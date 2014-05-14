

#include "anypio.h"
#include "anytimer.h"

void main(void) {
	anytimer_every(CT16B0, 50, anypio_led_toggle);	
}

