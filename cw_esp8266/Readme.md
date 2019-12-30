# 此版本使用的OLED显示屏为SSD1306 I2C 4引脚版本。

## 接线方法

### OLED
| OLED | ESP8266 |
| :---: | :---: |
| VCC | 3.3V |
| GND | GND |
|SCL|D1(GPIO 5)|
|SDA|D2(GPIO 4)|


### 电键（KEY）
|KEY|ESP8266|
|:---:|:---:|
|1 | D4(GPIO 2)|
|2|D7(GPIO 13)|
|GND|GND|

#### 说明：如使用手动电键或开关，则只需要一条导线接D4(GPIO 2)另一条接地即可。

### 编码器
|ENCODER|ESP8266|
|:---:|:---:|
|GND|GND|
|+|3.3V|
|SW|D6(GPIO 12)|
|DT|D5(GPIO 14)|
|CLK|D3(GPIO 0)|

### 依赖的库
ESP8266WiFi.h 

PubSubClient.h 

Adafruit_SSD1306.h 

U8g2_for_Adafruit_GFX.h 

EEPROM.h

### 关于font.h
font.h为自定义的字体文件，使用的是 文泉驿开源字体( http://wenq.org/wqy2/index.cgi?FontGuide )。
感谢字体作者的无私奉献。
