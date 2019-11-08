#include <U8g2lib.h>

// 蜂鸣器正极引脚
#define pin_bee 14
// 按键的引脚
#define pin_key 12


/*
关于电码的参数设置
*/
// 字符的数量，用于遍历
#define arr_len 36
// 滴的最长市场，单位纳秒
#define u_time 150000
// 防止抖动的忽略时间，少于这个时间的按压会忽略。
#define check_time 30000
// 莫尔斯码
String c[] = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", "-----"};
// 对应字符
String w[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};

// 单词敲击的记录开始符号
int flag_rcd = 0;

// 字符记录开始符号
int flag_letter = 0;

// 检查空格标识
int flag_space = 0;

String key_code = "";
// 单次敲击的时间记录
unsigned long s_time = 0;
unsigned long e_time = 0;

// 每个字符的敲击时间记录
unsigned long cs_time = 0;

String lcd_content = "";



// 关于显示屏的相关参数设置
U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u(U8G2_R0, /* clock=*/2, /* data=*/0, /* cs=*/16, /* dc=*/5, /* reset=*/4);



void setup()
{
	// 初始化串口，用于测试
	Serial.begin(115200);
	// 初始化针脚
	pinMode(pin_bee, OUTPUT);
	pinMode(pin_key, INPUT_PULLUP);


	// 初始化LCD
	u.begin();
	u.enableUTF8Print();
	// u.setFont(u8g2_font_wqy12_t_gb2312);
	u.setFont(u8g2_font_ncenB14_tr);
	u.setFontDirection(0);
	u.clearBuffer();
}


void loop() {
	// 检查字符是否输入完成，停止输入的时间超过一个单位时间的1.2倍则代表此次字符输入完成，将输入的内容进行识别。
	check_letter();
	// 检查按键是否按下，按下则开始记录此次输入。
	check_key_press();
	// 检查按键是否释放
	check_key_release();

}
// 识别字符
void check_letter() {
	// 如果输入字符检查标识开启，则检查字符
	if(flag_letter == 1) {
		int diff_letter = micros() - cs_time;
		if(diff_letter > u_time * 1 && key_code != "") {
			check_code(key_code);
			key_code = "";
			flag_letter = 0;
			// 检查空格标识开启
			flag_space = 1;
			cs_time = micros();
		}
	}

	// 空格检查
	if(flag_space == 1) {
		int diff_space = micros() - cs_time;
		if(diff_space > u_time * 3) {
			lcd_bottom_s(" ");
			flag_space = 0;
		}
	}
}


// 检查按键是否按下
void check_key_press() {
	// 按键按下开始记录时间
	if(digitalRead(pin_key) == 0 && flag_rcd == 0) {
		flag_letter = 0;
		flag_space = 0;
		flag_rcd = 1;
		s_time = micros();
		digitalWrite(pin_bee, 1);
	}
}

// 检查按键是否释放
void check_key_release() {
	if(digitalRead(pin_key) == 1 && flag_rcd == 1) {
		digitalWrite(pin_bee, 0);
		e_time = micros();
		unsigned long diff_time = e_time - s_time;

		if (diff_time > check_time) {
			if(diff_time < u_time) {
				key_code += ".";
			}
			else if(diff_time >= u_time) {
				key_code += "-";
			}
			flag_rcd = 0;
			Serial.println(key_code);
			cs_time = micros();
			flag_letter = 1;
		}
		else {
			flag_rcd = 0;
			flag_letter = 1;
		}
	}
}




bool check_code(String code) {
	for (int i = 0 ; i < arr_len; i ++) {
		if(code == c[i]) {
			lcd_bottom_s(w[i]);
			return true;
		}
	}
	lcd_bottom_s("*");
	return false;
}


// LCD相关操作
void lcd_bottom_s(String s) {
	if(lcd_content.length() > 7) {
		lcd_content = lcd_content.substring(1);
	}
	lcd_content += s;
	u.clearBuffer();
	u.setCursor(0, 64);
	u.print(lcd_content);
	u.sendBuffer();
}