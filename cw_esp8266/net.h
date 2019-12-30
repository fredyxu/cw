// ESP8266 wifi

#ifndef _NET_ESP8266_H
#define _NET_ESP8266_H

#include <ESP8266WiFi.h>

extern void d_home_update();
// extern void check_btn();
extern const char *ssid;
extern const char *password;

// 连接WIFI
void init_net()
{
	// WiFi.mode(WIFI_STA);
	Serial.print("尝试连接WiFi");

	d_home_update("info", "WiFi...");
	// WiFi初始化
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED && flag_net)
	{
		// check_btn();
		delay(100);
		Serial.print(".");
	}
	Serial.println("连接成功");
	d_home_update("info", "WiFi...Done");

	// reconnect();
}

#endif