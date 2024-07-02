#ifndef __PROTOCOL_fr12hn_H__

#ifdef  __cplusplus
extern "C" {
#endif

#define VERSION "4.0"

#ifndef	_CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>

#include "lprintf.h"

/* Initalization */ 
extern void protocol_init(int argc, char **argv);

/* Event Driver */
extern int wait_for_event(int *arg);
//导致进程等待
#define NETWORK_LAYER_READY  0//网络层有待发送的分组
#define PHYSICAL_LAYER_READY 1//物理层发送队列的长度低于50字节
#define FRAME_RECEIVED       2//物理层收到了一整帧
#define DATA_TIMEOUT         3//定时器超时，arg返回超时的定时器编号
#define ACK_TIMEOUT          4//ACK定时器超时

/* Network Layer functions */
#define PKT_LEN 256
//packet长度规定
extern void enable_network_layer(void);
//数据链路层缓冲区未满，可以承接新的任务，允许网络层发送数据分组
extern void disable_network_layer(void);
//数据链路层无法发送分组
extern int  get_packet(unsigned char *packet);
//将网络层传来的分拷贝到指针p指向的缓冲区中
//函数返回值是分组长度
extern void put_packet(unsigned char *packet, int len);
//给出一个报告，包括坐标时间、分组数目、有效数据传输率bps，实际线路利用率，帧校验和错误个数
/* Physical Layer functions */
extern int  recv_frame(unsigned char *buf, int size);
//size为存放帧的缓冲区大小，返回值时是帧的长度
extern void send_frame(unsigned char *frame, int len);
//将内存frame处长度为len的缓冲区向物理层发送一帧，每字节1ms，帧与帧之间的边界保留1ms


extern int  phl_sq_len(void);

/* CRC-32 polynomium coding function */
extern unsigned int crc32(unsigned char *buf, int len);
//返回一个32比特整数，生成校验和方法是crc32(p,len)，验证校验和的方法是crc32(p,len+4)
/* Timer Management functions */
extern unsigned int get_ms(void);
extern void start_timer(unsigned int nr, unsigned int ms);
//nr是计时器编号，在0~63之间，ms是超时间值，ms从数据发送完毕后开始计时
extern void stop_timer(unsigned int nr);
//在超时之前重新启动同一编号的定时器，按照新的时间设置超时事件
extern void start_ack_timer(unsigned int ms);
//启动时刻为当前时刻...因为ack的发送时间可以忽略不计
extern void stop_ack_timer(void);
//依然按照先前的时间设置超时事件
/* Protocol Debugger */
//用于协议动作的调试，不是语言级调试
extern char *station_name(void);

extern void dbg_event(char *fmt, ...);//发生不正常事件时候才发
extern void dbg_frame(char *fmt, ...);//正常的帧的信息
extern void dbg_warning(char *fmt, ...);//警告

#define MARK lprintf("File \"%s\" (%d)\n", __FILE__, __LINE__)

#ifdef  __cplusplus
}
#endif

#endif
