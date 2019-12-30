#ifndef _MQTT_H
#define _MQTT_H

#include <PubSubClient.h>

WiFiClient esp_client;
PubSubClient client(esp_client);

// client id 的长度
#define client_id_len 32
// client id 随机生成
char client_id[client_id_len];

extern const char *mqtt_server;
extern const int port;
extern const char *topic_name;


extern void check_btn();

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
	if (flag_net)
	{
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

// 发送电码
void send_code_to_server()
{
	if (flag_net)
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
		d_text_info = "Sent";
	}
	else
	{
		Serial.println("完成");
		d_text_info = "Done";
	}
	flag_d_ref = 1;

	send_code = "";
}

#endif
