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
//���½��̵ȴ�
#define NETWORK_LAYER_READY  0//������д����͵ķ���
#define PHYSICAL_LAYER_READY 1//����㷢�Ͷ��еĳ��ȵ���50�ֽ�
#define FRAME_RECEIVED       2//������յ���һ��֡
#define DATA_TIMEOUT         3//��ʱ����ʱ��arg���س�ʱ�Ķ�ʱ�����
#define ACK_TIMEOUT          4//ACK��ʱ����ʱ

/* Network Layer functions */
#define PKT_LEN 256
//packet���ȹ涨
extern void enable_network_layer(void);
//������·�㻺����δ�������Գн��µ�������������㷢�����ݷ���
extern void disable_network_layer(void);
//������·���޷����ͷ���
extern int  get_packet(unsigned char *packet);
//������㴫���ķֿ�����ָ��pָ��Ļ�������
//��������ֵ�Ƿ��鳤��
extern void put_packet(unsigned char *packet, int len);
//����һ�����棬��������ʱ�䡢������Ŀ����Ч���ݴ�����bps��ʵ����·�����ʣ�֡У��ʹ������
/* Physical Layer functions */
extern int  recv_frame(unsigned char *buf, int size);
//sizeΪ���֡�Ļ�������С������ֵʱ��֡�ĳ���
extern void send_frame(unsigned char *frame, int len);
//���ڴ�frame������Ϊlen�Ļ�����������㷢��һ֡��ÿ�ֽ�1ms��֡��֮֡��ı߽籣��1ms


extern int  phl_sq_len(void);

/* CRC-32 polynomium coding function */
extern unsigned int crc32(unsigned char *buf, int len);
//����һ��32��������������У��ͷ�����crc32(p,len)����֤У��͵ķ�����crc32(p,len+4)
/* Timer Management functions */
extern unsigned int get_ms(void);
extern void start_timer(unsigned int nr, unsigned int ms);
//nr�Ǽ�ʱ����ţ���0~63֮�䣬ms�ǳ�ʱ��ֵ��ms�����ݷ�����Ϻ�ʼ��ʱ
extern void stop_timer(unsigned int nr);
//�ڳ�ʱ֮ǰ��������ͬһ��ŵĶ�ʱ���������µ�ʱ�����ó�ʱ�¼�
extern void start_ack_timer(unsigned int ms);
//����ʱ��Ϊ��ǰʱ��...��Ϊack�ķ���ʱ����Ժ��Բ���
extern void stop_ack_timer(void);
//��Ȼ������ǰ��ʱ�����ó�ʱ�¼�
/* Protocol Debugger */
//����Э�鶯���ĵ��ԣ��������Լ�����
extern char *station_name(void);

extern void dbg_event(char *fmt, ...);//�����������¼�ʱ��ŷ�
extern void dbg_frame(char *fmt, ...);//������֡����Ϣ
extern void dbg_warning(char *fmt, ...);//����

#define MARK lprintf("File \"%s\" (%d)\n", __FILE__, __LINE__)

#ifdef  __cplusplus
}
#endif

#endif
