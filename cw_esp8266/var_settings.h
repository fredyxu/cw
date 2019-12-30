#ifndef _VAR_SETTINGS_H
#define _VAR_SETTINGS_H

// 设置参数

// 播放电码的时候是否翻译。
bool s_show_code = true;
// 播放自己的代码
bool s_play_my_code = false;
// 滴的最长时长，单位毫秒
int u_time = 150;
// 防止抖动的忽略时间，少于这个时间的按压会忽略。单位：毫秒
int shake_time = 30;
// 播放电码的单位时长
int play_u_time = 120;
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
// 电键类型
// true 手动建
// false 自动键
bool s_key_type = true;
// 按键方向
// true 左嘀，右嗒
bool s_auto_key_type = true;



// 字母，单词间隔时间变量
unsigned long cs_time = 0;

// 单词键入的电码
String key_code = "";
// 一次键入的全部电码
String send_code = "";

#endif