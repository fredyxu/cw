#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>

#include <EEPROM.h>

#include "font.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 d(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
U8G2_FOR_ADAFRUIT_GFX u;

WiFiClient esp_client;
PubSubClient client(esp_client);


// 蜂鸣器正极引脚
#define pin_bee 16
// 按键的引脚
#define pin_key 2

// 编码器引脚
#define pin_sw 12
#define pin_dt 14
#define pin_clk 0

// client id 的长度
#define client_id_len 32

// 呼号
String callsign = "ZClub";
// 路由器SSID
const char *ssid = "***";
// 路由器密码
const char *password = "***";
// MQTT服务器
const char *mqtt_server = "cw.funner.pub";
// MQTT服务器端口
const int port = 1128;
// TOPIC
const char *topic_name = "cw";
// client id 随机生成
char client_id[client_id_len];

/* 用户设置 */
// 播放电码的时候是否翻译。
bool s_show_code = true;
// 播放自己的代码
bool s_play_my_code = false;
/*
关于电码的参数设置
*/
// 字符的数量，用于遍历
#define arr_len 36
// 滴的最长时长，单位毫秒
int u_time = 150;
// 防止抖动的忽略时间，少于这个时间的按压会忽略。单位：毫秒
int shake_time = 30;
// 播放电码的单位时长
int play_u_time = 120;
// 莫尔斯码
String c[] = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", "-----"};
// 对应字符
char w[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};

// 单次敲击的时间记录
unsigned long s_time = 0;
unsigned long e_time = 0;
// 字母，单词间隔时间变量
unsigned long cs_time = 0;

// 蜂鸣器频率
int bee_freq = 800;

// 单词敲击的记录开始符号
int flag_rcd = 0;

// 字符记录开始符号
int flag_letter = 0;

// 检查空格标识
int flag_space = 0;

// 发送代码标识
int flag_send = 0;

// 显示屏刷新标识
bool flag_d_ref = 1;

// 是否连接网络
bool flag_net = false;


// 单词键入的电码
String key_code = "";
// 一次键入的全部电码
String send_code = "";

// 编码器相关参数
// 测试用
// int count = 0;
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



// 显示屏显示内容
#define d_line_word_11_num 8
#define d_line_word_7_num 16
#define d_menu_item_num 9
#define d_menu_page_item_num 4

String oled_time_arg = "";
String d_text_code = "                ";
String d_text_letter = "                ";
String d_text_info = "";
String d_text_rec_code = "                ";
String d_text_rec_letter = "                ";

int d_home_line[] = {13, 23, 33, 43, 53, 63};

int d_menu_line[d_menu_page_item_num] = {13, 29, 45, 61};



String d_menu_item_text[] = {"返回", "调整音调", "是否播放", "是否转换", "拍发时长", "播放时长", "防抖时长", "连接网络", "重置为默认设置"};

String d_menu_item_code[] = {"back", "tzyd", "sfbf", "sfzh", "pfsc", "bfsc", "fdsc", "net", "default"};


// 显示屏状态 home 正常显示屏幕， menu 菜单页面
String d_status = "home";


// 当前选中项目的代码。用来作为后续操作的识别
String d_menu_current_item_code = "";
// 当前选中项目的序号
int d_menu_current_item_num = 0;
// 当前选中项目
String d_menu_selected_item_code = "";


void setup()
{
	// 初始化引脚
	pinMode(pin_bee, OUTPUT);
	pinMode(pin_key, INPUT_PULLUP);
	pinMode(pin_sw, INPUT_PULLUP);
	pinMode(pin_dt, INPUT_PULLUP);
	pinMode(pin_clk, INPUT_PULLUP);


	// 初始化串口
	Serial.begin(115200);
	Serial.println();

	// 生成client id
	create_client_id();

	// 初始化显示屏
	d_init();

	// 读取设置
	get_settings("all");
}



void loop()
{
	if(flag_net) {
		if (!client.connected() && flag_net)
		{
			if(WiFi.status() != WL_CONNECTED) {
				init_net();
			}
			// 设置服务器
			client.setServer(mqtt_server, port);
			// 设置回调
			client.setCallback(mqtt_callback);

			reconnect();
		}
		client.loop();
	}
	// 检查字符是否输入完成，停止输入的时间超过一个单位时间的1.2倍则代表此次字符输入完成，将输入的内容进行识别。
	check_letter();
	// 检查按键是否按下，按下则开始记录此次输入。
	check_key_press();
	// 检查按键是否释放
	check_key_release();
	// 检测串口输入的数据
	check_serial_input();

	// 检查是否要刷新屏幕
	if(flag_d_ref == 1) {
		flag_d_ref = 0;
		d_home_ref();
	}
	// 检查电位器是否被按动
	check_btn();

}

// 连接WIFI
void init_net() {
	// WiFi.mode(WIFI_STA);
	Serial.print("尝试连接WiFi");

	d_home_update("info", "WiFi...");
	// WiFi初始化
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED && flag_net)
	{
		check_btn();
		delay(100);
		Serial.print(".");
	}
	Serial.println("连接成功");
	d_home_update("info", "WiFi...Done");

	// reconnect();
}

void check_btn() {
	// pin_sw为0，则按下选择按钮
	if (!digitalRead(pin_sw)) //读取到按钮按下并且计数值不为0时把计数器清零
	{
		sw_pressed();
	}
	int clk_val = digitalRead(pin_clk);//读取CLK引脚的电平
	int dt_val = digitalRead(pin_dt);//读取DT引脚的电平

	// CLK与之前保存的电平不同，则有转动。
	if (last_clk != clk_val) {
		// 转动计数自增
		b_count++;
		// 新的值保存起来，一边检测下一次转动。
		last_clk = clk_val;
		// 检查是顺时针转动还是逆时针转动
		// CLK与DT值不同，则为顺时针转动，否则为逆时针转动。将此次检查到的转动方向记录下面，并自增相对应的变量。用于判断此组转动用户是希望顺时针转动还是逆时针。可能因为震动的关系，转动方向时常不准。故以计数并判断方向
		if(clk_val != dt_val) {
			add_count++;
		}
		else {
			min_count ++;
		}
		// 如果转动计数到达了预设值，则视为有效转动。进行相关操作
		if(b_count >= d_count) {
			b_count = 0;
			// 正向转动
			if(add_count > min_count) {
				rot_do("add");
			}
			else {
				rot_do("min");
			}
			add_count = 0;
			min_count = 0;
		}
	}
}

// 编码器转动
void rot_do(String item) {
	// 如果是在菜单首页转动了编码器。并且没有选中项目。则上下选择菜单项目
	if(d_status == "menu" && d_menu_selected_item_code == "") {
		// 顺时针向下滚动选择
		if(item == "add") {
			if(d_menu_current_item_num < d_menu_item_num - 1) {
				d_menu_current_item_num ++;
			}
			// 如果超过了
			else if(d_menu_current_item_num >= d_menu_item_num -1) {
				d_menu_current_item_num = 0;
			}
			
		}
		// 逆时针向上滚动选择
		else if (item == "min") {
			if(d_menu_current_item_num > 0) {
				d_menu_current_item_num --;
			}
			else if(d_menu_current_item_num <= 0) {
				d_menu_current_item_num = d_menu_item_num - 1;
			}
		}
	}
	// 如果在菜单界面，并且有选中项目，则调节项目的值
	else if(d_status == "menu" && d_menu_selected_item_code != "") {
		// 调节音调
		if(d_menu_selected_item_code == "tzyd") {
			// 顺时针增加
			if(item == "add") {
				bee_freq += 50;
			}
			else if(item == "min" && bee_freq > 0) {
				bee_freq -= 50;
			}
			else if(item == "min" && bee_freq <= 0)  {
				bee_freq = 0;
			}
		}
		// 是否播放自己的电码
		else if(d_menu_selected_item_code == "sfbf") {
			s_play_my_code = !s_play_my_code;
		}
		// 是否将电码转换为字符
		else if(d_menu_selected_item_code == "sfzh") {
			s_show_code = !s_show_code;
		}
		// 拍发时长
		else if(d_menu_selected_item_code == "pfsc") {
			if(item == "add") {
				u_time += 10;
			}
			else if(item == "min" && u_time >= 10) {
				u_time -= 10;
			}
			else if(item == "min" && u_time <= 10) {
				u_time = 10;
			}
		}
		// 播放时长
		else if(d_menu_selected_item_code == "bfsc") {
			if(item == "add") {
				play_u_time += 10;
			}
			else if(item == "min" && play_u_time >= 10) {
				play_u_time -= 10;
			}
			else if(item == "min" && play_u_time <= 10) {
				play_u_time = 10;
			}
		}

		// 防抖时长
		else if(d_menu_selected_item_code == "fdsc") {
			if(item == "add") {
				shake_time += 5;
			}
			else if(item == "min" && shake_time >= 5) {
				shake_time -= 5;
			}
			else if(item == "min" && shake_time <= 0) {
				shake_time = 0;
			}
		}

		// 是否连接WiFi
		else if(d_menu_selected_item_code == "net") {
			flag_net = !flag_net;
			if(flag_net == false) {
				WiFi.mode(WIFI_OFF);
			}
			if(flag_net == true) {
				init_net();
			}
		}
		
		save_settings("all");
	}

	d_menu_ref();
}



// 编码器的按钮被按下
void sw_pressed(){
	// 两次按键间隔要超过300毫秒，否则视为抖动
	if(millis() - sw_check_time > 300) {
		sw_check_time = millis();
		// 当前如果显示的是默认首页，则显示菜单首页
		if(d_status == "home") {
			d_menu_ref();
		}
		else if(d_status == "menu") {
			// 选中返回项。显示首页
			if(d_menu_current_item_code == "back") {
				d_home_ref();
			}
			else if(d_menu_current_item_code == "default") {
				settings_default();
				d_home_ref();
			}
			// 如果选中项目为空，则当前项目转变为选中项目
			else if(d_menu_selected_item_code == "") {
				d_menu_selected_item_code = d_menu_current_item_code;
				d_menu_ref();
			}
			// 如果已有选中项目，则表示已经设置了该项目，设置好当前值，并将当前选项移除。
			else if(d_menu_selected_item_code != "") {
				d_menu_selected_item_code = "";
				d_menu_ref();
			}
		}
		
	}
}


// 重置设置为默认值
void settings_default() {
	bee_freq = 900;
	s_play_my_code = true;
	s_show_code = true;
	u_time = 150;
	play_u_time = 120;
	shake_time = 30;
	flag_net = true;
	save_settings("all");
}


/* 
设置参数所在地址
bee_freq : 0-4 5字节
s_play_my_code: 5 1字节
s_show_code: 6 1字节
u_time: 7-10 4字节
play_u_time: 11-14 4字节
shake_time: 15-18 4字节
flag_net: 19 1字节
 */

// 保存设置参数
void save_settings(String item) {
	EEPROM.begin(20);
	// 音调频率
	if(item == "bee_freq" || item == "all") {
		String temp_str = String(bee_freq);
		int str_len = temp_str.length() + 1;
		char temp_c[str_len];
		temp_str.toCharArray(temp_c, str_len);
		for(int i = 4; i >= 0; i--) {
			int num = i - str_len + 2;
			if(num <= 0 ) {
				num = -num;
				EEPROM.write(i, temp_c[num]);
			}
			else {
				EEPROM.write(i, '0');
			}
		}
	}
	// 收到自己发出的电码是否播放
	if(item == "s_play_my_code" || item == "all") {
		char value = '0';
		if(s_play_my_code) {
			value = '1';
		}
		EEPROM.write(5, value);
	}
	// 是否翻译电码
	if(item == "s_show_code" || item == "all") {
		char value = '0';
		if(s_show_code) {
			value = '1';
		}
		EEPROM.write(6, value);
	}

	// 拍发电码的单位时间
	if(item == "u_time" || item == "all") {
		write_num_value(u_time, 7, 4);
	}

	// 播放电码的单位时间
	if(item == "play_u_time" || item == "all") {
		write_num_value(play_u_time, 11, 4);
	}
	// 防抖动
	if(item == "shake_time" || item == "all") {
		write_num_value(shake_time, 15, 4);
	}

	// 是否连接网络
	if(item == "flag_net" || item == "all") {
		char value = '0';
		if(flag_net) {
			value = '1';
		}
		EEPROM.write(19, value);
	}

	EEPROM.commit();

	get_settings("all");
}

// num_value 要写入的值，整数
// s_addr 开始的地址
// data_b_count 预留几个字节存储
void write_num_value(int num_value, int s_addr, int data_b_count) {
	String temp_str = String(num_value);
	int str_len = temp_str.length() + 1;
	char temp_c[str_len];
	temp_str.toCharArray(temp_c, str_len);
	for(int i = data_b_count - 1; i >= 0; i--) {
		int num = i - str_len + 2;
		if(num <= 0 ) {
			num = -num;
			EEPROM.write(i + s_addr, temp_c[num]);
		}
		else {
			EEPROM.write(i + s_addr, '0');
		}
	}
}

void get_settings(String item) {
	EEPROM.begin(20);
	// 音调频率
	if(item == "bee_freq" || item == "all") {
		String str_bee_freq = "";
		for(int i = 4; i >= 0; i--) {
			char temp_c = EEPROM.read(i);
			str_bee_freq += temp_c;
		}
		bee_freq = str_bee_freq.toInt();
	}
	// 收到自己发出的电码后是否播放 地址5
	if(item == "s_play_my_code" || item == "all") {
		char value = EEPROM.read(5);
		if(value == '1') {
			s_play_my_code = true;
		}
		else if(value = '0') {
			s_play_my_code = false;
		}
	}

	// 是否翻译电码
	if(item == "s_show_code" || item == "all") {
		char value = EEPROM.read(6);
		if(value == '1') {
			s_show_code = true;
		}
		else if(value == '0') {
			s_show_code = false;
		}
	}

	// 拍发的单位时间
	if(item == "u_time" || item == "all") {
		String str = "";
		for(int i = 3; i >= 0; i--) {
			char temp_c = EEPROM.read(i + 7);
			str += temp_c;
		}
		u_time = str.toInt();
	}

	// 播放的单位时间
	if(item == "play_u_time" || item == "all") {
		String str = "";
		for(int i = 3; i >= 0; i--) {
			char temp_c = EEPROM.read(i + 11);
			str += temp_c;
		}
		play_u_time = str.toInt();
	}

	// 防抖动时长
	if(item == "shake_time" || item == "all") {
		String str = "";
		for(int i = 3; i >= 0; i--) {
			char temp_c = EEPROM.read(i + 15);
			str += temp_c;
		}
		shake_time = str.toInt();
	}

	// 是否连接网络
	if(item == "flag_net" || item == "all") {
		char value = EEPROM.read(19);
		if(value == '1') {
			flag_net = true;
		}
		else if(value == '0') {
			flag_net = false;
		}
	}
}

// 生成client id
void create_client_id()
{
	randomSeed(analogRead(0));
	for (int i = 0; i < client_id_len; i++)
	{
		int rand_num = random(arr_len);
		client_id[i] = char(w[rand_num]);
	}
}

// 处理MQTT消息
void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
	String rec_msg = "";
	String rec_callsign = "";
	int flag_rcd_callsign = 1;
	Serial.print("[MSG] ");
	for (int i = 0; i < length; i++)
	{
		rec_msg += (char)payload[i];
		// 记录发送者的呼号
		if ((char)payload[i] != ':' && flag_rcd_callsign != 0)
		{
			rec_callsign += (char)payload[i];
		}
		else
		{
			flag_rcd_callsign = 0;
		}
	}
	Serial.println(rec_msg);
	// 如果呼号不是自己的，则播放
	if (rec_callsign != String(callsign) || s_play_my_code)
	{
		play_code(rec_msg);
	}
}

// 连接MQTT
void reconnect()
{
	if(flag_net) {
		Serial.print("尝试连接MQTT...");
		d_home_update("info", "MQTT...");
		while (!client.connected() && flag_net)
		{
			check_btn();
			if (client.connect(client_id) && flag_net)
			{
				client.subscribe(topic_name);
				Serial.println("连接成功");
				d_home_update("info", "MQTT...Done");
			}
			// else
			// {
				// Serial.print(".");
				// delay(100);
			// }
		}
	}
}

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
		else if (temp_comm_code == "?" || temp_comm_code == "？")
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
	else if(comm.substring(0, 13) == "set_bee_freq_") {
		bee_freq = comm.substring(13, comm.length()).toInt();
		Serial.println("设置成功 - 蜂鸣器频率为" + comm.substring(13, comm.length()).toInt());
	}
}

// 播放电码
void play_code(String p_code)
{
	String rcd_code = "";
	for (int i = 0; i < p_code.length(); i++)
	{
		if (p_code[i] == '.')
		{
			tone(pin_bee, bee_freq);
			int s = millis();
			d_home_update("rec_code", ".");
			delay(play_u_time - (millis() - s));
			noTone(pin_bee);
			delay(play_u_time * 0.2);
			rcd_code += p_code[i];
			
		}
		else if (p_code[i] == '-')
		{
			tone(pin_bee, bee_freq);
			int s = millis();
			d_home_update("rec_code", "-");
			delay(play_u_time * 2 - (millis() - s));
			noTone(pin_bee);
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
			send_code += key_code + " ";
			// OLED上显示一个空格
			d_text_code += " ";
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
			// 将发送代码的最后一个空格去掉，换成 '/'
			// 经测试，这种方法影响性能。等待更高性能的方法
			// send_code.substring(0, send_code.length()-1);
			send_code += "/";
			// 在显示屏上显示斜杠
			d_text_code += "/";
			d_text_letter += " ";
			flag_d_ref = 1;
			flag_space = 0;
			flag_send = 1;
			cs_time = millis();
		}
	}

	// 检查发送电码
	if (flag_send == 1)
	{
		int diff_time = millis() - cs_time;
		if (diff_time > u_time * 10 && send_code != "")
		{
			send_code_to_server();
		}
	}
}

// 发送电码
void send_code_to_server()
{
	if(flag_net) {
		Serial.println();
		Serial.print("正在发送...");
		String send_msg = String(callsign) + ":" + send_code;
		int msg_length = send_msg.length();
		if (msg_length > 0)
		{
			char msg_arr[msg_length + 1];
			send_msg.toCharArray(msg_arr, msg_length + 1);
			client.publish(topic_name, msg_arr);
		}
		Serial.println("完成");
		d_text_info = "Sent";
	}
	else {
		Serial.println("完成");
		d_text_info = "Done";
	}
	flag_d_ref = 1;

	send_code = "";
}

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
		tone(pin_bee, bee_freq);
	}
}

// 检查按键是否释放
void check_key_release()
{
	if (digitalRead(pin_key) == 1 && flag_rcd == 1)
	{
		noTone(pin_bee);
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

// 1 是自己输入
// 0 是接收来的
bool check_code(String code, int to)
{
	for (int i = 0; i < arr_len; i++)
	{
		if (code == c[i])
		{
			Serial.print(" ");
			Serial.print(w[i]);
			Serial.print(" ");
			// 在显示屏上显示
			if(to == 1) {
				d_text_letter += w[i];
			}
			else if(to == 0) {
				d_text_rec_letter += w[i];
			}
			return true;
		}
	}
	Serial.print("*");
	if(to == 1) {
		d_text_letter += "*";
	}
	else if(to == 0) {
		d_text_rec_letter += "*";
	}
	return false;
}



/*****************************************************************
 * 
 * 显示部分
 * 
 * 采用 SSD1306 I2C 128*64 OLED 屏幕
 * 										
 * ***************************************************************/



void d_init() {
	if (!d.begin(SSD1306_SWITCHCAPVCC, 0x3C))
	{ // Address 0x3C for 128x64
		Serial.println(F("SSD1306 allocation failed"));
		for (;;)
			; // Don't proceed, loop forever
	}
	u.begin(d);
}


void d_home_update(String item, String info_text) {
	if(item == "info") {
		d_text_info = info_text;
	}

	else if(item == "rec_code") {
		d_text_rec_code += info_text;
	}

	// 刷新首页
	d_home_ref();
}



bool set_font(char size) {
	if(size == 's') {
		u.setFont(u8g2_font_pxplusibmcgathin_8r);
		return true;
	}
	else if (size == 'm') {
		u.setFont(u8g2_font_profont17_mr);
		return true;
	}
	else if(size == 'c') {
		u.setFont(u8g2_font_c_13);
		// u.setFont(u8g2_font_pxplusibmcgathin_8r);
		return true;
	}
}



// 首页显示刷新
void d_home_ref() {
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

// 菜单页面刷新
void d_menu_ref() {
	d_status = "menu";
	
	
	d_menu_current_item_code = d_menu_item_code[d_menu_current_item_num];
	d.clearDisplay();
	// 获取当前页数
	int current_page = d_menu_current_item_num / d_menu_page_item_num;

	// 确定显示的页面内容
	int item_s_num = current_page * d_menu_page_item_num;
	int item_e_num;
	// 计算当前页面显示的项目的结束序号
	if(d_menu_item_num - item_s_num > d_menu_page_item_num) {
		item_e_num = item_s_num + d_menu_page_item_num;
	}
	else {
		item_e_num = d_menu_item_num;
	}
	
	int j = 0;
	for(int i = item_s_num; i < item_e_num; i ++ ) {
		// u.setForegroundColor(1);
		
		// 如果当前项目是选中项目，则外面再画个圈
		if(d_menu_item_code[i] == d_menu_selected_item_code) {
			d.drawLine(0, d_menu_line[j] +1 , 128, d_menu_line[j] +1, 1);
		}
		// 如果该项目是当前项目，则前面画个点
		else if(i == d_menu_current_item_num) {
			d.fillRect(0, d_menu_line[j] - 7, 4, 4, 1);
		}
		set_font('c');
		u.setCursor(6, d_menu_line[j]);
		u.print(d_menu_item_text[i]);

		// 显示该项目的当前值
		String value = "";
		// 音调
		if(d_menu_item_code[i] == "tzyd") {
			value = String(bee_freq);
		}
		// 是否播放自己的diamante
		else if(d_menu_item_code[i] == "sfbf") {
			if(s_play_my_code) {
				value = "Yes";
			}
			else {
				value = "No";
			}
		}
		// 是否转换电码为字符
		else if(d_menu_item_code[i] == "sfzh") {
			if(s_show_code) {
				value = "Yes";
			}
			else {
				value = "No";
			}
		}
		// 拍发电码的单位时长
		else if(d_menu_item_code[i] == "pfsc") {
			value = String(u_time);
		}
		// 播放电码的单位时长
		else if(d_menu_item_code[i] == "bfsc") {
			value = String(play_u_time);
		}
		// 防抖时长
		else if(d_menu_item_code[i] == "fdsc") {
			value = String(shake_time);
		}
		// 呼号
		else if(d_menu_item_code[i] == "hh") {
			value = callsign;
		}

		// 是否连接网络
		else if(d_menu_item_code[i] == "net") {
			if(flag_net) {
				value = "Yes";
			}
			else {
				value = "No";
			}
		}
		

		if(value != "") {
			set_font('s');
			int str_width = u.getUTF8Width(value.c_str());
			u.setCursor(128 - 4 - str_width, d_menu_line[j] - 2);
			u.print(value);
		}

		j++;
	}

	d.display();
}
