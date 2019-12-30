#ifndef _OP_CODE_H
#define _OP_CODE_H
// 函数声明
// 识别字符
bool check_code(String code, int to);
// 识别字符
void check_letter();
// 播放电码
void play_code(String p_code);

// 识别字符
void check_letter()
{
	// 如果输入字符检查标识开启，则检查字符
	if (flag_letter == 1)
	{
		int diff_letter = millis() - cs_time;
		if (diff_letter > u_time * 1 && key_code != "")
		{
			cs_time = millis();
			check_code(key_code, 1);
			// send_code += key_code + " ";
			// OLED上显示一个空格
			// d_text_code += " ";
			// 在显示屏的最下一行，显示空格
			key_code = "";
			flag_letter = 0;
			// 检查空格标识开启
			flag_space = 1;
		}
	}

	// 空格检查
	if (flag_space == 1)
	{
		int diff_space = millis() - cs_time;
		if (diff_space > u_time * 3)
		{
			// 加上分割用的空格
			send_code += " ";
			// 在显示屏上显示斜杠
			d_text_code += " ";
			d_text_letter += " ";
			flag_d_ref = 1;
			flag_space = 0;
			flag_send = 1;
			cs_time = millis();
		}
	}
}

// 1 是自己输入
// 0 是接收来的
bool check_code(String code, int to)
{
	for (int i = 0; i < arr_len; i++)
	{
		if (code == c[i])
		{
			Serial.print(w[i]);
			Serial.print(" ");
			// 在显示屏上显示
			if (to == 1)
			{
				d_text_letter += w[i];
				d_text_code += " ";
				
			}
			else if (to == 0)
			{
				d_text_rec_letter += w[i];
				d_text_rec_code += " ";
			}
			return true;
		}
	}
	Serial.print("*");
	if (to == 1)
	{
		d_text_letter += "*";
		d_text_code += " ";
	}
	else if (to == 0)
	{
		d_text_rec_letter += "*";
		d_text_rec_code += " ";
	}
	return false;
}

// 播放电码
void play_code(String p_code)
{
	String rcd_code = "";
	for (int i = 0; i < p_code.length(); i++)
	{
		if (p_code[i] == '.')
		{
			bee(true);
			int s = millis();
			d_home_update("rec_code", ".");
			delay(play_u_time - (millis() - s));
			bee(false);
			delay(play_u_time * 0.2);
			rcd_code += p_code[i];
		}
		else if (p_code[i] == '-')
		{
			bee(true);
			int s = millis();
			d_home_update("rec_code", "-");
			delay(play_u_time * 2 - (millis() - s));
			bee(false);
			delay(play_u_time * 0.2);
			rcd_code += p_code[i];
		}
		else if (p_code[i] == ' ')
		{
			int s = millis();
			d_text_rec_code += ' ';
			if (rcd_code != "" && s_show_code)
			{
				check_code(rcd_code, 0);
			}
			d_home_ref();
			delay(play_u_time - (millis() - s));
			rcd_code = "";
		}
		else if (p_code[i] == '/')
		{
			int s = millis();
			d_text_rec_code += '/';
			d_text_rec_letter += ' ';

			if (rcd_code != "" && s_show_code)
			{
				Serial.print("/");
				check_code(rcd_code, 0);
			}
			d_home_ref();
			delay(play_u_time * 2 - (millis() - s));
			rcd_code = "";
		}
		// 将最后一个代码也送去播放
		if (i == p_code.length() - 1 && rcd_code != "" && s_show_code)
		{
			check_code(rcd_code, 0);
			rcd_code = "";
		}
	}
	Serial.println();
}



#endif