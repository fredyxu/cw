// 蜂鸣器正极引脚
#define pin_bee 14
// 按键的引脚
#define pin_key 12

// 呼号
String callsign = "";
/*
关于电码的参数设置
*/
// 字符的数量，用于遍历
#define arr_len 36
// 滴的最长时长，单位毫秒
#define u_time 150
// 防止抖动的忽略时间，少于这个时间的按压会忽略。单位：毫秒
#define check_time 30

#define play_u_time 120
// 莫尔斯码
String c[] = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", "-----"};
// 对应字符
String w[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};

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
	Serial.begin(115200);
	Serial.println();
	// 初始化针脚
	pinMode(pin_bee, OUTPUT);
	pinMode(pin_key, INPUT_PULLUP);
}


void loop() {
	// 检查字符是否输入完成，停止输入的时间超过一个单位时间的1.2倍则代表此次字符输入完成，将输入的内容进行识别。
	check_letter();
	// 检查按键是否按下，按下则开始记录此次输入。
	check_key_press();
	// 检查按键是否释放
	check_key_release();
	// 检测串口输入的数据
	check_serial_input();
}


// 从串口读取电码
void check_serial_input() {
	if(Serial.available() > 0) {
		int flag_add = 1;
		String temp_read_code = "";
		while(Serial.available()) {
			char temp_get_c = Serial.read();
			// 在字符中查找输入内容，如果有，则添加相应电码，如果没有，则直接添加到读取数据中。
			for(int i = 0; i < arr_len; i ++) {
				if(w[i] == String(temp_get_c)) {
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
		Serial.print(callsign + " - INPUT CODE: ");
		Serial.println(temp_read_code);
		play_code(temp_read_code);
	}
}

// 播放电码
void play_code(String p_code) {
	for(int i = 0; i < p_code.length(); i ++) {
		if(p_code[i] == '.') {
			digitalWrite(pin_bee, 1);
			delay(play_u_time);
			digitalWrite(pin_bee, 0);
			delay(play_u_time * 0.2 );
		}
		else if(p_code[i] == '-') {
			digitalWrite(pin_bee, 1);
			delay(play_u_time * 2);
			digitalWrite(pin_bee, 0);
			delay(play_u_time * 0.2);
		}
		else if(p_code[i] == ' ') {
			delay(play_u_time);
		}
		else if(p_code[i] == '/') {
			delay(play_u_time * 2);
		}
	}
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

			Serial.println();
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
	String send_msg = callsign + ":" + send_code;
	Serial.println(send_msg);
	play_code(send_code);
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

		if (diff_time > check_time) {
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
			Serial.println(w[i]);
			return true;
		}
	}
	Serial.println("*");
	return false;
}

