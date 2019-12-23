#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <U8g2lib.h>

// 关于显示屏的相关参数设置
U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u(U8G2_R0, /* clock=*/2, /* data=*/0, /* cs=*/16, /* dc=*/5, /* reset=*/4);
// U8G2_SH1106_128X64_NONAME_F_SW_I2C u(U8G2_R0, /* clock=*/2, /* data=*/0);

WiFiClient esp_client;
PubSubClient client(esp_client);

// oled显示的总行数
#define oled_line_num 6
#define oled_line_word_11_num 8
#define oled_line_word_7_num 16

// 显示屏显示内容
String oled_callsign = "";
String oled_time_arg = "";
String oled_code = "                ";
String oled_word = "                ";
String oled_info = "";
String oled_rec_code = "                ";
String oled_rec_word = "                ";
int oled_line_bottom[oled_line_num] = {13, 23, 33, 43, 53, 63};

// oled刷新标识
int flag_oled_ref = 0;

// 蜂鸣器正极引脚
#define pin_bee 10
// 按键的引脚
#define pin_key 14

// 编码器引脚
#define pin_sw 15
#define pin_dt 13
#define pin_clk 12


// client id 的长度
#define client_id_len 32


// 呼号
String callsign = "BG7YXY";

// 路由器SSID
const char *ssid = "BiliGo";
// 路由器密码
const char *password = "08980898";
// MQTT服务器
const char *mqtt_server = "cw.funner.pub";
// MQTT服务器端口
const int port = 1128;
// TOPIC
const char *topic_name = "test";
// client id 随机生成
char client_id[client_id_len];

/* 用户设置 */
// 播放电码的时候是否翻译。
bool s_show_code = true;
// 播放自己的代码
bool s_play_my_code = true;



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

// 单词键入的电码
String key_code = "";
// 一次键入的全部电码
String send_code = "";

// 测试用参数
bool last_status = false; 


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
	u.begin();
	u.enableUTF8Print(); // enable UTF8 support for the Arduino print() function
	u.setFontDirection(0);
	// 设置字体
	// set_font('m');
	// u.clearBuffer();

	oled_ref();

	Serial.print("尝试连接WiFi");
	oled_info = "WiFi...";
	oled_ref();
	// WiFi初始化
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}
	Serial.println("连接成功");
	oled_info = "WiFi...Done";
	oled_ref();

	// 初始化MQTT
	// 设置服务器
	client.setServer(mqtt_server, port);
	// 设置回调
	client.setCallback(mqtt_callback);
}

void loop()
{
	if (!client.connected())
	{
		reconnect();
	}
	client.loop();
	// 检查字符是否输入完成，停止输入的时间超过一个单位时间的1.2倍则代表此次字符输入完成，将输入的内容进行识别。
	check_letter();
	// 检查按键是否按下，按下则开始记录此次输入。
	check_key_press();
	// 检查按键是否释放
	check_key_release();
	// 检测串口输入的数据
	check_serial_input();

	// 检查是否要刷新屏幕
	if(flag_oled_ref == 1) {
		flag_oled_ref = 0;
		oled_ref();
	}
	// 检查电位器是否被按动
	// check_btn();

}

// void check_btn() {
// 	bool new_status = !digitalRead(pin_sw);
// 	if(new_status != last_status) {
// 		Serial.println(new_status? "pressed": "released");
// 		last_status = new_status;
// 	}

// }

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
	Serial.print("尝试连接MQTT...");
	oled_info = "MQTT...";
	oled_ref();
	while (!client.connected())
	{
		// char* cs;
		// callsign.toCharArray(cs, callsign.length());
		if (client.connect(client_id))
		{
			client.subscribe(topic_name);
			Serial.println("连接成功");
			oled_info = "MQTT...Done";
			oled_ref();
		}
		else
		{
			Serial.print(".");
			delay(500);
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
		Serial.println("设置成功 - 自动识别代码");
	}
	else if (comm == "set_show_code_no")
	{
		s_show_code = false;
		Serial.println("设置成功 - 不自动识别代码");
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
		Serial.println("设置成功 - 播放自己的代码");
	}
	else if (comm == "set_play_my_code_no")
	{
		s_play_my_code = false;
		Serial.println("设置成功 - 不播放自己的代码");
	}
	// 设置单位时间
	else if (comm.substring(0, 14) == "set_unit_time_")
	{
		u_time = comm.substring(14, comm.length()).toInt();
		Serial.println("设置成功 - 单位时间: " + comm.substring(14, comm.length()) + "毫秒");
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
			// digitalWrite(pin_bee, 1);
			tone(pin_bee, bee_freq);
			int s = millis();
			oled_rec_code += '.';
			oled_ref();
			delay(play_u_time - (millis() - s));
			// digitalWrite(pin_bee, 0);
			noTone(pin_bee);
			delay(play_u_time * 0.2);
			rcd_code += p_code[i];
			
		}
		else if (p_code[i] == '-')
		{
			// digitalWrite(pin_bee, 1);
			tone(pin_bee, bee_freq);
			int s = millis();
			oled_rec_code += '-';
			oled_ref();
			delay(play_u_time * 2 - (millis() - s));
			// digitalWrite(pin_bee, 0);
			noTone(pin_bee);
			delay(play_u_time * 0.2);
			rcd_code += p_code[i];
		}
		else if (p_code[i] == ' ')
		{
			int s = millis();
			oled_rec_code += ' ';
			if (rcd_code != "" && s_show_code)
			{
				check_code(rcd_code, 0);
			}
			oled_ref();
			delay(play_u_time - (millis() - s));
			rcd_code = "";
		}
		else if (p_code[i] == '/')
		{
			int s = millis();
			oled_rec_code += '/';
			oled_rec_word += ' ';
			
			
			if (rcd_code != "" && s_show_code)
			{
				Serial.print("/");
				check_code(rcd_code, 0);
			}
			oled_ref();
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
	// 最后增加一个空格。
	// oled_word += " ";
	// oled_code += " ";
	oled_ref();
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
			oled_ref();
			send_code += key_code + " ";
			// OLED上显示一个空格
			oled_code += " ";
			// oled_word += " ";


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
			oled_code += "/";
			oled_word += " ";
			flag_oled_ref = 1;

			// Serial.println();
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
	oled_info = "Sent";
	flag_oled_ref = 1;

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
		// digitalWrite(pin_bee, 1);
		tone(pin_bee, bee_freq);
	}
}

// 检查按键是否释放
void check_key_release()
{
	if (digitalRead(pin_key) == 1 && flag_rcd == 1)
	{
		// digitalWrite(pin_bee, 0);
		noTone(pin_bee);
		e_time = millis();
		unsigned long diff_time = e_time - s_time;

		if (diff_time > shake_time)
		{
			oled_info = "";
			if (diff_time < u_time)
			{
				key_code += ".";
				Serial.print(".");

				// OLED上显示.
				oled_code += ".";
				flag_oled_ref = 1;

			}
			else if (diff_time >= u_time)
			{
				key_code += "-";
				Serial.print("-");
				// OLED上显示-
				oled_code += "-";
				flag_oled_ref = 1;
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
				oled_word += w[i];
			}
			else if(to == 0) {
				oled_rec_word += w[i];
			}
			return true;
		}
	}
	Serial.print("*");
	if(to == 1) {
		oled_word += '*';
	}
	else if(to == 0) {
		oled_rec_word += '*';
	}
	return false;
}


// 刷新显示屏显示内容
void oled_ref() {

	u.clearBuffer();
	// 第一行 呼号
	set_font('m');
	oled_callsign = callsign;
	u.setCursor(0, oled_line_bottom[0]);
	u.print(oled_callsign);

	// 更换字体
	set_font('s');
	// 第二行 接收的代码
	u.setCursor(0, oled_line_bottom[1]);
	oled_rec_code = oled_rec_code.substring(oled_rec_code.length() - oled_line_word_7_num);
	u.print(oled_rec_code);

	// 第三行 接收的文字
	u.setCursor(0, oled_line_bottom[2]);
	oled_rec_word = oled_rec_word.substring(oled_rec_word.length() - oled_line_word_7_num);
	u.print(oled_rec_word);

	// 第四行 发送的代码
	u.setCursor(0, oled_line_bottom[3]);
	oled_code = oled_code.substring(oled_code.length() - oled_line_word_7_num);
	u.print(oled_code);

	// 第5行 发送的文字
	u.setCursor(0, oled_line_bottom[4]);
	oled_word = oled_word.substring(oled_word.length() - oled_line_word_7_num);
	u.print(oled_word);

	// 第6行
	u.setCursor(0, oled_line_bottom[5]);
	u.print(oled_info);

	u.sendBuffer();
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
}

void oled_menu_ref() {
	u.clearBuffer();
	set_font('m');
	u.setCursor(0, 0);
	u.print("设置菜单：");
	u.sendBuffer();
}