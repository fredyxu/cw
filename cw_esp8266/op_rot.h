// 编码器操作
#ifndef _OP_ROT_H
#define _OP_ROT_H

// 编码器相关参数
// 测试用
// clk的值，用于确定电位器是否被旋转
int last_clk = 0;
// 防止抖动，调整编码器的灵敏度
// b_count 旋转时计数。
// d_count 计数累计到此值则计入一次有效输入。
int b_count = 0;
int d_count = 5;
// 确定是正向旋转还是逆向旋转，顺时针定义为增加，逆时针定义为减少。
// 通过记录每一次旋转确定是顺时针还是逆时针来确定最后有效的输入是增加还是减少。因为存在或抖动或其他原因，无法以最后一次旋转的情况来判断是增加还是减少。
int add_count = 0;
int min_count = 0;

int sw_check_time = 0;

extern bool s_key_type;

// 函数声明
void rot_do(String item);
void sw_pressed();
void check_btn();


// 函数定义
void check_btn()
{
	// pin_sw为0，则按下选择按钮
	if (!digitalRead(pin_sw)) //读取到按钮按下并且计数值不为0时把计数器清零
	{
		sw_pressed();
	}
	int clk_val = digitalRead(pin_clk); //读取CLK引脚的电平
	int dt_val = digitalRead(pin_dt);   //读取DT引脚的电平

	// CLK与之前保存的电平不同，则有转动。
	if (last_clk != clk_val)
	{
		// 转动计数自增
		b_count++;
		// 新的值保存起来，一边检测下一次转动。
		last_clk = clk_val;
		// 检查是顺时针转动还是逆时针转动
		// CLK与DT值不同，则为顺时针转动，否则为逆时针转动。将此次检查到的转动方向记录下面，并自增相对应的变量。用于判断此组转动用户是希望顺时针转动还是逆时针。可能因为震动的关系，转动方向时常不准。故以计数并判断方向
		if (clk_val != dt_val)
		{
			add_count++;
		}
		else
		{
			min_count++;
		}
		// 如果转动计数到达了预设值，则视为有效转动。进行相关操作
		if (b_count >= d_count)
		{
			b_count = 0;
			// 正向转动
			if (add_count > min_count)
			{
				rot_do("add");
			}
			else
			{
				rot_do("min");
			}
			add_count = 0;
			min_count = 0;
		}
	}
}

// 编码器转动
void rot_do(String item)
{
	// 如果是在菜单首页转动了编码器。并且没有选中项目。则上下选择菜单项目
	if (d_status == "menu" && d_menu_selected_item_code == "")
	{
		// 顺时针向下滚动选择
		if (item == "add")
		{
			if (d_menu_current_item_num < d_menu_item_num - 1)
			{
				d_menu_current_item_num++;
			}
			// 如果超过了
			else if (d_menu_current_item_num >= d_menu_item_num - 1)
			{
				d_menu_current_item_num = 0;
			}
		}
		// 逆时针向上滚动选择
		else if (item == "min")
		{
			if (d_menu_current_item_num > 0)
			{
				d_menu_current_item_num--;
			}
			else if (d_menu_current_item_num <= 0)
			{
				d_menu_current_item_num = d_menu_item_num - 1;
			}
		}
	}
	// 如果在菜单界面，并且有选中项目，则调节项目的值
	else if (d_status == "menu" && d_menu_selected_item_code != "")
	{
		
		// 调节音调
		if (d_menu_selected_item_code == "tzyd")
		{
			// 顺时针增加
			if (item == "add")
			{
				bee_freq += 50;
			}
			else if (item == "min" && bee_freq > 0)
			{
				bee_freq -= 50;
			}
			else if (item == "min" && bee_freq <= 0)
			{
				bee_freq = 50;
			}
		}

		// 设置按键类型
		else if (d_menu_selected_item_code == "djlx") {
			s_key_type = !s_key_type;
		}
		// 按键方向
		else if (d_menu_selected_item_code == "ajfx") {
			s_auto_key_type = !s_auto_key_type;
		}
		// 是否播放自己的电码
		else if (d_menu_selected_item_code == "sfbf")
		{
			s_play_my_code = !s_play_my_code;
		}
		// 是否将电码转换为字符
		else if (d_menu_selected_item_code == "sfzh")
		{
			s_show_code = !s_show_code;
		}
		// 拍发时长
		else if (d_menu_selected_item_code == "pfsc")
		{
			if (item == "add")
			{
				u_time += 10;
			}
			else if (item == "min" && u_time > 50)
			{
				u_time -= 10;
			}
			else if (item == "min" && u_time <= 50)
			{
				u_time = 50;
			}
		}
		// 播放时长
		else if (d_menu_selected_item_code == "bfsc")
		{
			if (item == "add")
			{
				play_u_time += 10;
			}
			else if (item == "min" && play_u_time >= 10)
			{
				play_u_time -= 10;
			}
			else if (item == "min" && play_u_time <= 10)
			{
				play_u_time = 10;
			}
		}

		// 防抖时长
		else if (d_menu_selected_item_code == "fdsc")
		{
			if (item == "add")
			{
				shake_time += 5;
			}
			else if (item == "min" && shake_time >= 5)
			{
				shake_time -= 5;
			}
			else if (item == "min" && shake_time <= 0)
			{
				shake_time = 0;
			}
		}

		// 是否连接WiFi
		else if (d_menu_selected_item_code == "net")
		{
			flag_net = !flag_net;
			if (flag_net == false)
			{
				WiFi.mode(WIFI_OFF);
			}
			if (flag_net == true)
			{
				init_net();
			}
		}

		save_settings("all");
	}

	d_menu_ref();
}

// 编码器的按钮被按下
void sw_pressed()
{
	// 两次按键间隔要超过300毫秒，否则视为抖动
	if (millis() - sw_check_time > 300)
	{
		sw_check_time = millis();
		// 当前如果显示的是默认首页，则显示菜单首页
		if (d_status == "home")
		{
			d_menu_ref();
		}
		else if (d_status == "menu")
		{
			// 选中返回项。显示首页
			if (d_menu_current_item_code == "back")
			{
				d_home_ref();
			}
			else if (d_menu_current_item_code == "default")
			{
				settings_default();
				d_home_ref();
			}
			// 如果选中项目为空，则当前项目转变为选中项目
			else if (d_menu_selected_item_code == "")
			{
				d_menu_selected_item_code = d_menu_current_item_code;
				d_menu_ref();
			}
			// 如果已有选中项目，则表示已经设置了该项目，设置好当前值，并将当前选项移除。
			else if (d_menu_selected_item_code != "")
			{
				d_menu_selected_item_code = "";
				d_menu_ref();
			}
		}
	}
}
#endif