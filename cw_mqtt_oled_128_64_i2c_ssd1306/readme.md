# 使用OLED作为信息输出

## 接线方法

### - OLED显示屏
### OLED  -  ESP8266

GND - GND


VCC - 3.3V


SCL - D1(GPIO 5)


SDA - D2(GPIO 4)




### - 开关的两个引脚
### KEY - ESP8266

1 - D4(GPIO 2)
 
2 - GND
 




### - 无源蜂鸣器 
### BEE - ESP8266

GND - GND

VCC - 3.3V

I/O - D0(GPIO 16)

### - 编码器
### ENCODER - ESP8266
GND - GND

\+ - 3.3V

SW - D6(GPIO 12)

DT - D5(GPIO 14)

CLK - D3(GPIO 0)



## 依赖的库
ESP8266WiFi.h

PubSubClient.h

Adafruit_SSD1306.h

U8g2_for_Adafruit_GFX.h

EEPROM.h


## 关于font.h
font.h为自定义的字体文件，使用的是 文泉驿开源字体( http://wenq.org/wqy2/index.cgi?FontGuide )。

感谢字体作者的无私奉献。




