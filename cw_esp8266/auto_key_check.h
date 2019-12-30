#ifndef _AUTO_KEY_CHECK_H
#define _AUTO_KEY_CHECK_H


// 电码间隔
int sp_time = u_time / 2;
// 滴的时长
int di_time = u_time / 2;
// 哒的时长
int da_time = u_time * 1.5;

void auto_key_check_press();
void auto_key_input_code(bool s);

void auto_key_check_press()
{
	// 检查滴
	if(digitalRead(pin_key_di) == 0) {
		auto_key_input_code(s_auto_key_type);
	}
	if(digitalRead(pin_key_da) == 0) {
		auto_key_input_code(!s_auto_key_type);
	}
	if(digitalRead(pin_key_di) == 1 && digitalRead(pin_key_da) == 1 && flag_letter == 0) {
		cs_time = millis();
		flag_letter = 1;
	}

}

// s = true  键入di
// s = false 键入da
void auto_key_input_code(bool s) {
	flag_letter = 0;
	flag_space = 0;

	if(s) {
		key_code += ".";
		d_text_code += ".";
		Serial.print(".");
		bee(true);
		tt(true);
		d_home_ref();
		delay(di_time - tt(false));
		bee(false);
		delay(sp_time);
	}
	else  {
		key_code += "-";
		d_text_code += "-";
		Serial.print("-");
		bee(true);
		tt(true);
		d_home_ref();
		delay(da_time - tt(false));
		bee(false);
		delay(sp_time);
	}
	
	
}


#endif