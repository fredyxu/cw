#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// 蜂鸣器正极引脚
#define pin_bee 14
// 按键的引脚
#define pin_key 12
// client id 的长度
#define client_id_len 32

// 呼号
String callsign = "ZClub";
// 路由器SSID
const char* ssid = "***";
// 路由器密码
const char* password = "***";
// MQTT服务器
const char* mqtt_server = "cw.funner.pub";
// MQTT服务器端口
const int port = 1128;
// TOPIC
const char* topic_name = "cw";
// client id 随机生成
char client_id[client_id_len];

/* 用户设置 */
// 播放电码的时候是否翻译。
bool s_show_code = true;
// 播放自己的代码
bool s_play_my_code = false;

WiFiClient esp_client;
PubSubClient client(esp_client);

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


void setup()
{
	// 初始化串口
	Serial.begin(115200);
	Serial.println();

	// 生成client id
	create_client_id();

	// 初始化针脚
	pinMode(pin_bee, OUTPUT);
	pinMode(pin_key, INPUT_PULLUP);

	Serial.print("尝试连接WiFi");
	// WiFi初始化
	WiFi.begin(ssid, password);
	while(WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("连接成功");

	// 初始化MQTT
	// 设置服务器
	client.setServer(mqtt_server, port);
	// 设置回调
	client.setCallback(mqtt_callback);

	
}


void loop() {
	if(!client.connected()) {
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
}

// 生成client id
void create_client_id() {
	randomSeed(analogRead(0));
	for(int i = 0; i < client_id_len; i++) {
		int rand_num = random(arr_len);
		client_id[i] = char(w[rand_num]);
	}
}

// 处理MQTT消息
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
	String rec_msg = "";
	String rec_callsign = "";
	int flag_rcd_callsign = 1;
	Serial.print("[MSG] ");
	for(int i = 0; i < length; i ++) {
		rec_msg += (char)payload[i];
		// 记录发送者的呼号
		if((char)payload[i] != ':' && flag_rcd_callsign != 0) {
			rec_callsign += (char)payload[i];
		}
		else {
			flag_rcd_callsign = 0;
		}
	}
	Serial.println(rec_msg);
	// 如果呼号不是自己的，则播放
	if(rec_callsign != String(callsign) || s_play_my_code) {
		play_code(rec_msg);
	}
}


// 连接MQTT
void reconnect() {
	Serial.print("尝试连接MQTT...");
	while(!client.connected()) {
		// char* cs;
		// callsign.toCharArray(cs, callsign.length());
		if(client.connect(client_id)) {
			client.subscribe(topic_name);
			Serial.println("连接成功");
		}
		else {
			Serial.print(".");
			delay(500);
		}
	}
}

// 从串口读取信息
void check_serial_input() {
	if(Serial.available() > 0) {
		int flag_add = 1;
		String temp_read_code = "";
		String temp_comm_code = "";
		while(Serial.available()) {
			char temp_get_c = Serial.read();
			temp_comm_code += temp_get_c;
			// 在字符中查找输入内容，如果有，则添加相应电码，如果没有，则直接添加到读取数据中。
			for(int i = 0; i < arr_len; i ++) {
				flag_add = 1;
				if(w[i] == temp_get_c) {
					temp_read_code += c[i] + " ";
					flag_add = 0;
					break;
				}
			}
			if(flag_add) {
				temp_read_code += temp_get_c;
			}

			delay(1);
		}
		if(temp_comm_code.substring(0, 3) == "set") {
			run_comm(temp_comm_code);
		}
		else if(temp_comm_code == "?" || temp_comm_code == "？") {
			show_help();
		}
		else {
			Serial.println();
			Serial.print(callsign);
			Serial.print(temp_read_code);
			send_code = temp_read_code;
			send_code_to_server();
		}
	}
}
void show_help() {
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
}

void run_comm(String comm) {
	Serial.println();
	// 设置是否自动翻译
	if(comm == "set_show_code_yes") {
		s_show_code = true;
		Serial.println("设置成功 - 自动识别代码");
	}
	else if(comm == "set_show_code_no") {
		s_show_code = false;
		Serial.println("设置成功 - 不自动识别代码");
	}
	// 设置呼号
	else if(comm.substring(0, 13) == "set_callsign_") {
		callsign = comm.substring(13, comm.length());
		Serial.print("呼号重新设置为：");
		Serial.println(callsign);
	}
	// 播放自己的电码
	else if (comm == "set_play_my_code_yes") {
		s_play_my_code = true;
		Serial.println("设置成功 - 播放自己的代码");
	}
	else if(comm == "set_play_my_code_no") {
		s_play_my_code = false;
		Serial.println("设置成功 - 不播放自己的代码");
	}
	// 设置单位时间
	else if(comm.substring(0, 14) == "set_unit_time_") {
		u_time = comm.substring(14, comm.length()).toInt();
		Serial.println("设置成功 - 单位时间: " + comm.substring(14, comm.length()) + "毫秒");

	}
	// 设置播放的单位时间
	else if(comm.substring(0, 19) == "set_play_unit_time_") {
		play_u_time = comm.substring(19, comm.length()).toInt();
		Serial.println("设置成功 - 播放单位时间: " + comm.substring(19, comm.length()) + "毫秒");
	}
	// 设置防抖时长
	else if(comm.substring(0, 15) == "set_shake_time_") {
		shake = comm.substring(15, comm.length()).toInt();
		Serial.println("设置成功 - 防抖时长: " + comm.substring(15, comm.length()) + "毫秒");
	}
}

// 播放电码
void play_code(String p_code) {
	String rcd_code = "";
	for(int i = 0; i < p_code.length(); i ++) {
		if(p_code[i] == '.') {
			digitalWrite(pin_bee, 1);
			delay(play_u_time);
			digitalWrite(pin_bee, 0);
			delay(play_u_time * 0.2 );
			rcd_code += p_code[i];
		}
		else if(p_code[i] == '-') {
			digitalWrite(pin_bee, 1);
			delay(play_u_time * 2);
			digitalWrite(pin_bee, 0);
			delay(play_u_time * 0.2);
			rcd_code += p_code[i];
		}
		else if(p_code[i] == ' ') {
			delay(play_u_time);
			if(rcd_code != "" && s_show_code) {
				check_code(rcd_code);
			}
			rcd_code = "";
		}
		else if(p_code[i] == '/') {
			delay(play_u_time * 2);
			if(rcd_code != "" && s_show_code) {
				Serial.print("/");
				check_code(rcd_code);
			}
			rcd_code = "";
		}
		// 将最后一个代码也送去播放
		if(i == p_code.length() -1 && rcd_code != "" && s_show_code) {
			check_code(rcd_code);
			rcd_code = "";
		}
	}
	Serial.println();
}
// 识别字符
void check_letter() {
	// 如果输入字符检查标识开启，则检查字符
	if(flag_letter == 1) {
		int diff_letter = millis() - cs_time;
		if(diff_letter > u_time * 1 && key_code != "") {
			check_code(key_code);
			send_code += key_code + " ";
			key_code = "";
			flag_letter = 0;
			// 检查空格标识开启
			flag_space = 1;
			cs_time = millis();
		}
	}

	// 空格检查
	if(flag_space == 1) {
		int diff_space = millis() - cs_time;
		if(diff_space > u_time * 3) {
			// 将发送代码的最后一个空格去掉，换成 '/'
			// 经测试，这种方法影响性能。等待更高性能的方法
			// send_code.substring(0, send_code.length()-1);
			send_code += "/";

			// Serial.println();
			flag_space = 0;
			flag_send = 1;
			cs_time = millis();
		}
	}

	// 检查发送电码
	if(flag_send == 1) {
		int diff_time = millis() - cs_time;
		if(diff_time > u_time * 10 && send_code != "") {
			send_code_to_server();
		}
	}
}

// 发送电码
void send_code_to_server() {
	Serial.println();
	Serial.print("正在发送...");

	String send_msg = String(callsign) + ":" + send_code;
	int msg_length = send_msg.length();
	if(msg_length > 0) {
		char msg_arr[msg_length + 1];
		send_msg.toCharArray(msg_arr, msg_length + 1);
		client.publish(topic_name, msg_arr);
	}
	Serial.println("完成");
	// Serial.println(send_msg);
	// play_code(send_code);
	send_code = "";
}


// 检查按键是否按下
void check_key_press() {
	// 按键按下开始记录时间
	if(digitalRead(pin_key) == 0 && flag_rcd == 0) {
		flag_letter = 0;
		flag_space = 0;
		flag_rcd = 1;
		s_time = millis();
		digitalWrite(pin_bee, 1);
	}
}

// 检查按键是否释放
void check_key_release() {
	if(digitalRead(pin_key) == 1 && flag_rcd == 1) {
		digitalWrite(pin_bee, 0);
		e_time = millis();
		unsigned long diff_time = e_time - s_time;

		if (diff_time > shake_time) {
			if(diff_time < u_time) {
				key_code += ".";
				Serial.print(".");
			}
			else if(diff_time >= u_time) {
				key_code += "-";
				Serial.print("-");
			}
			flag_rcd = 0;
			
			cs_time = millis();
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
			Serial.print(" ");
			Serial.print(w[i]);
			Serial.print(" ");
			return true;
		}
	}
	Serial.print("*");
	return false;
}

