/** This application illustrates usage of the I2C interface. It reads a ams TCS3471 RGB color sensor
and turns the LED on and off based on the measured brightness. This example requires a TCS3471
connected to the I2C port (SCL = PIO0_4, SDA = PIO0_5). Add 3.3V pullup resistors to both lines.
See TCS3471 datasheet and NXP's I2C spec and notes for details. */

#include "anykey/anykey.h"
#include "pn532.h"

#define LED_PORT 0
#define LED_PIN 7
#define KEY_PORT 0
#define KEY_PIN 1

void sleep(uint32_t len) {
	volatile uint32_t i;
	while (i < len) { i++; }
}

uint16_t status = 0;


void main () {
	sleep(100000); //For some reason, this seems to be a good thing to do

	any_gpio_set_dir (LED_PORT, LED_PIN, OUTPUT);
	any_gpio_write   (LED_PORT, LED_PIN, false);
	any_gpio_set_dir (KEY_PORT, KEY_PIN, INPUT);
	ANY_GPIO_SET_PULL (KEY_PORT, KEY_PIN, PULL_UP);


	SYSCON_StartSystick_10ms();

	//Initialize PN532
	PN532_Error err ;

	sleep(10000000);
	err = PN532_Init();
	if (err) {
		status = 0x80 + err;
		return;
	}
/*
	sleep(10000000);
	uint32_t firmwareVersion;
	err = PN532_GetFirmwareVersion(&firmwareVersion);
	if (err) status = 0x100 + err;
	else status = (firmwareVersion >> 0) & 0xff;
	return;

	int count = 1;
	err = PN532_ListCards(&count, PN532_Mod_TypeA106kbps);
	if (err) status = 0x100 + err;
	else status = count;
	return;
*/
	err = PN532_SAMConfiguration();
	if (err) {
		status = 0x100 + err;
		return;
	}	

	err = PN532_SAMConfiguration();
	if (err) {
		status = 0x100 + err;
		return;
	}	

	status = 0x1ff;

	int count = 1;
	err = PN532_ListCards(&count, PN532_Mod_TypeA106kbps);
	if (err) status = 0x100 + err;
	any_gpio_write (LED_PORT, LED_PIN, count > 0);
	return;

	uint8_t errCode = 0;
	bool fieldPresent = false;
	uint8_t numTargets = 0;
	err = PN532_GetGeneralStatus(&errCode, &fieldPresent, &numTargets);
	if (err) status = 0x100 + err;
	else if (errCode) status = 0x80 + errCode;
	else if (numTargets) status = 0x180 + numTargets;
	else status = fieldPresent ? 0x1ff : 0x001;

}

void systick() {
	const int phaseLen = 70;
	const int onPhaseLen = 50;
	const int offPhaseLen = 10;
	static int counter = 0;
	counter = (counter+1) % (11*phaseLen);
	int bit = counter / phaseLen;
	int phase = counter % phaseLen;
	int phaseRef = (bit < 9) ? ((status & (0x100 >> bit)) ? onPhaseLen : offPhaseLen) : 0;
//	any_gpio_write (LED_PORT, LED_PIN, phase < phaseRef);
}