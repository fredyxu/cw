// SSD1306 I2C 4引脚 OLED显示屏
#ifndef _DISPLAY_SSD1306_I2C_H
#define _DISPLAY_SSD1306_I2C_H

#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>

#include "font.h"


String oled_time_arg = "";
String d_text_code = "                ";
String d_text_letter = "                ";
String d_text_info = "";
String d_text_rec_code = "                ";
String d_text_rec_letter = "                ";

// 显示屏显示内容
#define d_line_word_11_num 8
#define d_line_word_7_num 16
#define d_menu_item_num 11
#define d_menu_page_item_num 4

// 显示屏参数
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1


Adafruit_SSD1306 d(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
U8G2_FOR_ADAFRUIT_GFX u;

int d_home_line[] = {13, 23, 33, 43, 53, 63};

int d_menu_line[d_menu_page_item_num] = {13, 29, 45, 61};

String d_menu_item_text[] = {
	"返回",
	"调整音调",
	"电键类型",
	"按键方向",
	"是否播放",
	"是否转换",
	"拍发时长",
	"播放时长",
	"防抖时长",
	"连接网络",
	"重置为默认设置"};

String d_menu_item_code[] = {
	"back",
	"tzyd",
	"djlx",
	"ajfx",
	"sfbf",
	"sfzh",
	"pfsc",
	"bfsc",
	"fdsc",
	"net",
	"default"};

// 显示屏状态 home 正常显示屏幕， menu 菜单页面
String d_status = "home";
// 当前选中项目的代码。用来作为后续操作的识别
String d_menu_current_item_code = "";
// 当前选中项目的序号
int d_menu_current_item_num = 0;
// 当前选中项目
String d_menu_selected_item_code = "";


// 呼号
extern String callsign;


void d_init()
{
	if (!d.begin(SSD1306_SWITCHCAPVCC, 0x3C))
	{
		Serial.println(F("SSD1306 allocation failed"));
	}
	u.begin(d);
}

bool set_font(char size)
{
	if (size == 's')
	{
		u.setFont(u8g2_font_pxplusibmcgathin_8r);
		return true;
	}
	else if (size == 'm')
	{
		u.setFont(u8g2_font_profont17_mr);
		return true;
	}
	else if (size == 'c')
	{
		u.setFont(u8g2_font_c_13);
		// u.setFont(u8g2_font_pxplusibmcgathin_8r);
		return true;
	}
}

// 首页显示刷新
void d_home_ref()
{
	d_status = "home";
	// int s = millis();
	d.clearDisplay();

	// 第一行 呼号
	set_font('m');
	u.setCursor(0, d_home_line[0]);
	u.print(callsign);

	// 更换字体
	set_font('s');
	// 第二行 接收的代码
	u.setCursor(0, d_home_line[1]);
	d_text_rec_code = d_text_rec_code.substring(d_text_rec_code.length() - d_line_word_7_num);
	u.print(d_text_rec_code);

	// 第三行 接收的文字
	u.setCursor(0, d_home_line[2]);
	d_text_rec_letter = d_text_rec_letter.substring(d_text_rec_letter.length() - d_line_word_7_num);
	u.print(d_text_rec_letter);

	// 第四行 发送的代码
	u.setCursor(0, d_home_line[3]);
	d_text_code = d_text_code.substring(d_text_code.length() - d_line_word_7_num);
	u.print(d_text_code);

	// 第5行 发送的文字
	u.setCursor(0, d_home_line[4]);
	d_text_letter = d_text_letter.substring(d_text_letter.length() - d_line_word_7_num);
	u.print(d_text_letter);

	// 第6行
	u.setCursor(0, d_home_line[5]);
	u.print(d_text_info);

	d.display();

	// int e = millis();
	// Serial.println("s = " + String(s) + "   e = " + String(e) + "  e - s = " + String(e - s));
}

void d_home_update(String item, String info_text)
{
	if (item == "info")
	{
		d_text_info = info_text;
	}

	else if (item == "rec_code")
	{
		d_text_rec_code += info_text;
	}
	// 刷新首页
	d_home_ref();
}

// 菜单页面刷新
void d_menu_ref()
{
	d_status = "menu";

	d_menu_current_item_code = d_menu_item_code[d_menu_current_item_num];
	d.clearDisplay();
	// 获取当前页数
	int current_page = d_menu_current_item_num / d_menu_page_item_num;

	// 确定显示的页面内容
	int item_s_num = current_page * d_menu_page_item_num;
	int item_e_num;
	// 计算当前页面显示的项目的结束序号
	if (d_menu_item_num - item_s_num > d_menu_page_item_num)
	{
		item_e_num = item_s_num + d_menu_page_item_num;
	}
	else
	{
		item_e_num = d_menu_item_num;
	}

	int j = 0;
	for (int i = item_s_num; i < item_e_num; i++)
	{
		// u.setForegroundColor(1);

		// 如果当前项目是选中项目，则外面再画个圈
		if (d_menu_item_code[i] == d_menu_selected_item_code)
		{
			d.drawLine(0, d_menu_line[j] + 1, 128, d_menu_line[j] + 1, 1);
		}
		// 如果该项目是当前项目，则前面画个点
		else if (i == d_menu_current_item_num)
		{
			d.fillRect(0, d_menu_line[j] - 7, 4, 4, 1);
		}
		set_font('c');
		u.setCursor(6, d_menu_line[j]);
		u.print(d_menu_item_text[i]);

		// 显示该项目的当前值
		String value = "";
		// 音调
		if (d_menu_item_code[i] == "tzyd")
		{
			value = String(bee_freq);
		}
		else if(d_menu_item_code[i] == "djlx") {
			if(s_key_type == true) {
				value = "Manual";
			}
			else {
				value = "Auto";
			}
		}
		else if(d_menu_item_code[i] == "ajfx") {
			if(s_auto_key_type) {
				value = "L-Di";
			}
			else {
				value = "R-Di";
			}
		}
		// 是否播放自己的diamante
		else if (d_menu_item_code[i] == "sfbf")
		{
			if (s_play_my_code)
			{
				value = "Yes";
			}
			else
			{
				value = "No";
			}
		}
		// 是否转换电码为字符
		else if (d_menu_item_code[i] == "sfzh")
		{
			if (s_show_code)
			{
				value = "Yes";
			}
			else
			{
				value = "No";
			}
		}
		// 拍发电码的单位时长
		else if (d_menu_item_code[i] == "pfsc")
		{
			value = String(u_time);
		}
		// 播放电码的单位时长
		else if (d_menu_item_code[i] == "bfsc")
		{
			value = String(play_u_time);
		}
		// 防抖时长
		else if (d_menu_item_code[i] == "fdsc")
		{
			value = String(shake_time);
		}
		// 呼号
		else if (d_menu_item_code[i] == "hh")
		{
			value = callsign;
		}

		// 是否连接网络
		else if (d_menu_item_code[i] == "net")
		{
			if (flag_net)
			{
				value = "Yes";
			}
			else
			{
				value = "No";
			}
		}

		if (value != "")
		{
			set_font('s');
			int str_width = u.getUTF8Width(value.c_str());
			u.setCursor(128 - 4 - str_width, d_menu_line[j] - 2);
			u.print(value);
		}

		j++;
	}

	d.display();
}

#endif