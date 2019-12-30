// 引脚定义
#include "pin.h"
// 设置参数
#include "var_settings.h"
// 莫尔斯电码
#include "var_morse.h"
// 时间操作工具
#include "time_tools.h"
// 蜂鸣器操作
#include "op_bee.h"
// 显示
#include "display.h"
// 电码处理
#include "op_code.h"
// 网络连接
#include "net.h"
// MQTT连接
#include "mqtt.h"
// 存储设置
#include "op_settings.h"
// 编码器操作
#include "op_rot.h"
// 串口显示
#include "op_ser.h"
// 手动电键
#include "key_check.h"
// 自动电键
#include "auto_key_check.h"

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


void setup()
{
	// 初始化引脚
	pinMode(pin_bee, OUTPUT);
	pinMode(pin_key, INPUT_PULLUP);
	pinMode(pin_sw, INPUT_PULLUP);
	pinMode(pin_dt, INPUT_PULLUP);
	pinMode(pin_clk, INPUT_PULLUP);

	// 自动键引脚
	pinMode(pin_key_di, INPUT_PULLUP);
	pinMode(pin_key_da, INPUT_PULLUP);

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
	if (flag_net)
	{
		if (!client.connected() && flag_net)
		{
			if (WiFi.status() != WL_CONNECTED)
			{
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

	// 检查按键
	// 手动建
	if (s_key_type)
	{
		// 手动按键
		check_key_press();
		// 检查按键是否释放
		check_key_release();
	}
	// 自动键
	else
	{
		auto_key_check_press();
	}
	// 检测串口输入的数据
	check_serial_input();

	// 检查是否要刷新屏幕
	if (flag_d_ref == 1)
	{
		flag_d_ref = 0;
		d_home_ref();
	}
	// 检查电位器是否被按动
	check_btn();

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






