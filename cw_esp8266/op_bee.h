// ESP8266蜂鸣器控制
#ifndef _OP_BEE_H
#define _OP_BEE_H

#include "var_settings.h"
#include "pin.h"

void bee(bool s) {
	if(s) {
		tone(pin_bee, bee_freq);
	}
	else {
		noTone(pin_bee);
	}
}

#endif