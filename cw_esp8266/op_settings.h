// 设置操作
#ifndef _OP_SETTINGS_H
#define _OP_SETTINGS_H
#include <EEPROM.h>
#include "var_settings.h"

void get_settings(String item);
void write_num_value(int num_value, int s_addr, int data_b_count);
void save_settings(String item);
void settings_default();

/* 
设置参数所在地址
变量名				 地址	  字节
bee_freq			0-4		5字节
s_play_my_code		5 		1字节
s_show_code		 	6 		1字节
u_time		 		7-10 	4字节
play_u_time			11-14 	4字节
shake_time			15-18 	4字节
flag_net			19 		1字节
s_key_type			20 		1字节
s_auto_key_type		21 		1字节

 */

// 保存设置参数
void save_settings(String item)
{
	
	EEPROM.begin(22);
	// 音调频率
	if (item == "bee_freq" || item == "all")
	{
		String temp_str = String(bee_freq);
		int str_len = temp_str.length() + 1;
		char temp_c[str_len];
		temp_str.toCharArray(temp_c, str_len);
		for (int i = 4; i >= 0; i--)
		{
			int num = i - str_len + 2;
			if (num <= 0)
			{
				num = -num;
				EEPROM.write(i, temp_c[num]);
			}
			else
			{
				EEPROM.write(i, '0');
			}
		}
	}
	if(item == "s_key_type" || item == "all") {
		char value = '0';
		if(s_key_type) {
			value = '1';
		}
		EEPROM.write(20, value);
	}
	if(item == "s_auto_key_type" || item == "all") {

		char value = '0';
		if(s_auto_key_type) {
			value = '1';
		}
		EEPROM.write(21, value);
	}
	// 收到自己发出的电码是否播放
	if (item == "s_play_my_code" || item == "all")
	{
		char value = '0';
		if (s_play_my_code)
		{
			value = '1';
		}
		EEPROM.write(5, value);
	}
	// 是否翻译电码
	if (item == "s_show_code" || item == "all")
	{
		char value = '0';
		if (s_show_code)
		{
			value = '1';
		}
		EEPROM.write(6, value);
	}

	// 拍发电码的单位时间
	if (item == "u_time" || item == "all")
	{
		write_num_value(u_time, 7, 4);
	}

	// 播放电码的单位时间
	if (item == "play_u_time" || item == "all")
	{
		write_num_value(play_u_time, 11, 4);
	}
	// 防抖动
	if (item == "shake_time" || item == "all")
	{
		write_num_value(shake_time, 15, 4);
	}

	// 是否连接网络
	if (item == "flag_net" || item == "all")
	{
		char value = '0';
		if (flag_net)
		{
			value = '1';
		}
		EEPROM.write(19, value);
	}

	EEPROM.commit();

	// get_settings("all");
}


void get_settings(String item)
{
	EEPROM.begin(22);
	// 音调频率
	if (item == "bee_freq" || item == "all")
	{
		String str_bee_freq = "";
		for (int i = 4; i >= 0; i--)
		{
			char temp_c = EEPROM.read(i);
			str_bee_freq += temp_c;
		}
		bee_freq = str_bee_freq.toInt();
	}
	// 按键类型
	if(item == "s_key_type" || item == "all")  {
		char value = EEPROM.read(20);
		if(value == '1') {
			s_key_type = true;
		}
		else {
			s_key_type = false;
		}
	}
	// 按键方向
	if(item == "s_auto_key_type" || item == "all") {
		char value = EEPROM.read(21);
		if(value == '1') {
			s_auto_key_type = true;
		}
		else {
			s_auto_key_type = false;
		}
	}
	// 收到自己发出的电码后是否播放 地址5
	if (item == "s_play_my_code" || item == "all")
	{
		char value = EEPROM.read(5);
		if (value == '1')
		{
			s_play_my_code = true;
		}
		else if (value = '0')
		{
			s_play_my_code = false;
		}
	}

	// 是否翻译电码
	if (item == "s_show_code" || item == "all")
	{
		char value = EEPROM.read(6);
		if (value == '1')
		{
			s_show_code = true;
		}
		else if (value == '0')
		{
			s_show_code = false;
		}
	}

	// 拍发的单位时间
	if (item == "u_time" || item == "all")
	{
		String str = "";
		for (int i = 3; i >= 0; i--)
		{
			char temp_c = EEPROM.read(i + 7);
			str += temp_c;
		}
		u_time = str.toInt();
	}

	// 播放的单位时间
	if (item == "play_u_time" || item == "all")
	{
		String str = "";
		for (int i = 3; i >= 0; i--)
		{
			char temp_c = EEPROM.read(i + 11);
			str += temp_c;
		}
		play_u_time = str.toInt();
	}

	// 防抖动时长
	if (item == "shake_time" || item == "all")
	{
		String str = "";
		for (int i = 3; i >= 0; i--)
		{
			char temp_c = EEPROM.read(i + 15);
			str += temp_c;
		}
		shake_time = str.toInt();
	}

	// 是否连接网络
	if (item == "flag_net" || item == "all")
	{
		char value = EEPROM.read(19);
		if (value == '1')
		{
			flag_net = true;
		}
		else if (value == '0')
		{
			flag_net = false;
		}
	}
}

// num_value 要写入的值，整数
// s_addr 开始的地址
// data_b_count 预留几个字节存储
void write_num_value(int num_value, int s_addr, int data_b_count)
{
	String temp_str = String(num_value);
	int str_len = temp_str.length() + 1;
	char temp_c[str_len];
	temp_str.toCharArray(temp_c, str_len);
	for (int i = data_b_count - 1; i >= 0; i--)
	{
		int num = i - str_len + 2;
		if (num <= 0)
		{
			num = -num;
			EEPROM.write(i + s_addr, temp_c[num]);
		}
		else
		{
			EEPROM.write(i + s_addr, '0');
		}
	}
}


// 重置设置为默认值
void settings_default()
{
	bee_freq = 900;
	s_play_my_code = true;
	s_show_code = true;
	u_time = 150;
	play_u_time = 120;
	shake_time = 30;
	flag_net = true;
	s_key_type = true;
	s_auto_key_type = true;
	save_settings("all");
}






#endif