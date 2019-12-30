// 手动电键
#ifndef _KEY_CHECK_H
#define _KEY_CHECK_H
// 单次键入的时长记录
// 开始
unsigned long s_time = 0;
// 结束
unsigned long e_time = 0;


// 函数声明
void check_key_press();
void check_key_release();

// 检查按键是否按下
void check_key_press()
{
	// 按键按下开始记录时间
	if (digitalRead(pin_key) == 0 && flag_rcd == 0)
	{
		flag_letter = 0;
		flag_space = 0;
		flag_rcd = 1;
		s_time = millis();
		bee(1);
		
	}
}

// 检查按键是否释放
void check_key_release()
{
	if (digitalRead(pin_key) == 1 && flag_rcd == 1)
	{
		bee(0);
		e_time = millis();
		int diff_time = e_time - s_time;
		if (diff_time > shake_time)
		{
			d_text_info = "";
			flag_d_ref = 1;
			if (diff_time < u_time)
			{
				key_code += ".";
				Serial.print(".");
				// OLED上显示.
				d_text_code += ".";
			}
			else if (diff_time >= u_time)
			{
				key_code += "-";
				Serial.print("-");
				// OLED上显示-
				d_text_code += "-";
			}
			flag_rcd = 0;
			cs_time = millis();
			flag_letter = 1;
		}
		else
		{
			flag_rcd = 0;
			flag_letter = 1;
		}
	}
}



#endif