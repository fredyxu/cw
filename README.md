# 使用Arduino在ESP8266上实现的CW练习器项目

#cw.ino, cw_mqtt.ino, cw_serial.ino使用如下的接线方式，其他版本请按照各自文件夹下的Readme的说明接线。

### 引脚接线

- LCD显示器

CLOCK(D0) - GPIO 2 (D4)

DATA(D1)  - GPIO 0 (D3)

CS        - GPIO 16 (D0)

DC        - GPIO 5 (D1)

RESET(RES)- GPIO 4 (D2)



- 蜂鸣器

正极    - GPIO 14 (D5)

负极    - GND (G)



- 复位开关

GND(G)

GPIO 12 (D6)


### cw.ino 为带LCD的版本
### cw_serial.ino 为不带LCD的版本，由串口显示信息。默认串口波特率115200
### cw_mqtt.ino 为通过mqtt发送电码的版本，通过串口监视器显示信息。
