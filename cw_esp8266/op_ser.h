// 串口显示
#ifndef _OP_SER_H
#define _OP_SER_H

// 函数声明
void check_serial_input();
void show_help();
void run_comm(String comm);

// 从串口读取信息
void check_serial_input()
{
	if (Serial.available() > 0)
	{
		int flag_add = 1;
		String temp_read_code = "";
		String temp_comm_code = "";
		while (Serial.available())
		{
			char temp_get_c = Serial.read();
			temp_comm_code += temp_get_c;
			// 在字符中查找输入内容，如果有，则添加相应电码，如果没有，则直接添加到读取数据中。
			for (int i = 0; i < arr_len; i++)
			{
				flag_add = 1;
				if (w[i] == temp_get_c)
				{
					temp_read_code += c[i] + " ";
					flag_add = 0;
					break;
				}
			}
			if (flag_add)
			{
				temp_read_code += temp_get_c;
			}

			delay(1);
		}
		if (temp_comm_code.substring(0, 3) == "set")
		{
			run_comm(temp_comm_code);
		}
		else if (temp_comm_code == "help")
		{
			show_help();
		}
		else
		{
			Serial.println();
			Serial.print(callsign);
			Serial.print(temp_read_code);
			send_code = temp_read_code;
			send_code_to_server();
		}
	}
}
void show_help()
{
	Serial.println();
	Serial.println("参数设置命令：");
	Serial.println("set_show_code_yes		- 播放电码时自动翻译成字符");
	Serial.println("set_show_code_no		- 播放电码时不翻译成字符");
	Serial.println("set_play_my_code_yes		- 播放自己输入的电码声音");
	Serial.println("set_play_my_code_no		- 不播放自己输入的电码声音");
	Serial.println("set_callsign_ + 呼号		- 设置呼号");
	Serial.println("set_unit_time_ + 整数		- 设置拍发电码的单位时间（单位：毫秒。默认150毫秒）");
	Serial.println("set_play_unit_time_ + 整数	- 设置播放电码的单位时间（单位：毫秒。默认120毫秒）");
	Serial.println("set_shake_time_ + 整数 		- 设置防抖时长（单位：毫秒。默认30毫秒）");
	Serial.println("set_bee_freq_ + 整数		- 设置蜂鸣器频率");
}

void run_comm(String comm)
{
	Serial.println();
	// 设置是否自动翻译
	if (comm == "set_show_code_yes")
	{
		s_show_code = true;
		Serial.println("设置成功 - 自动识别莫尔斯电码");
	}
	else if (comm == "set_show_code_no")
	{
		s_show_code = false;
		Serial.println("设置成功 - 不自动识别莫尔斯电码");
	}
	// 设置呼号
	else if (comm.substring(0, 13) == "set_callsign_")
	{
		callsign = comm.substring(13, comm.length());
		Serial.print("呼号重新设置为：");
		Serial.println(callsign);
	}
	// 播放自己的电码
	else if (comm == "set_play_my_code_yes")
	{
		s_play_my_code = true;
		Serial.println("设置成功 - 播放自己发送的莫尔斯电码");
	}
	else if (comm == "set_play_my_code_no")
	{
		s_play_my_code = false;
		Serial.println("设置成功 - 不播放自己发送的莫尔斯电码");
	}
	// 设置单位时间
	else if (comm.substring(0, 14) == "set_unit_time_")
	{
		u_time = comm.substring(14, comm.length()).toInt();
		Serial.println("设置成功 - 拍发单位时间: " + comm.substring(14, comm.length()) + "毫秒");
	}
	// 设置播放的单位时间
	else if (comm.substring(0, 19) == "set_play_unit_time_")
	{
		play_u_time = comm.substring(19, comm.length()).toInt();
		Serial.println("设置成功 - 播放单位时间: " + comm.substring(19, comm.length()) + "毫秒");
	}
	// 设置防抖时长
	else if (comm.substring(0, 15) == "set_shake_time_")
	{
		shake_time = comm.substring(15, comm.length()).toInt();
		Serial.println("设置成功 - 防抖时长: " + comm.substring(15, comm.length()) + "毫秒");
	}
	// 设置蜂鸣器频率
	else if (comm.substring(0, 13) == "set_bee_freq_")
	{
		bee_freq = comm.substring(13, comm.length()).toInt();
		Serial.println("设置成功 - 蜂鸣器频率为" + comm.substring(13, comm.length()).toInt());
	}
}

#endif