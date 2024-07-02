#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "datalink.h"
#include "protocol.h"

#define MAX_SEQ  31
#define NR_BUFS ((MAX_SEQ + 1) / 2)

#define DATA_TIMER 6000
#define ACK_TIMER 800

typedef unsigned char seq_nr;//序列号

typedef struct {
    unsigned char kind; 
    unsigned char seq;
    unsigned char ack;
    unsigned char data[PKT_LEN];
    unsigned int padding;//填充
} FRAME;

static bool Not_NAK = true;
//是否发送了NAK
static int physical_ready = 0;
//物理层是否准备好发送数据

static int is_Between(seq_nr a, seq_nr b, seq_nr c) {
    //判断b是不是在序列号a,c之间
    //如果发生回绕也是正确的
    return ((a <= b && b < c) || (c < a && a <= b) || (b < c && c < a));
}

//对某一帧生成CRC校验,实现成帧
static void put_frame(seq_nr* frame, int len) {
    *(unsigned int*)(frame + len) = crc32(frame, len);
    send_frame(frame, len + 4);
    //这个send_frame是protocol中规定的向物理层发帧的函数
    //+4是因为CRC32产生的校验码是32位四个字节
    physical_ready = 0;
}

static void Send_Frames(FRAME_KIND fk, seq_nr frame_seq, seq_nr ack_seq, unsigned char out_buf[][PKT_LEN]) {
    //Send_Frames是用来封装数据包成帧
    FRAME frame;
    frame.kind = fk;
    frame.seq = frame_seq;
    frame.ack = ack_seq;
    //为了减少往返次数提高带宽利用率，使用了piggybacking
    //如果接收端没有信息要发送，单独发ACK就可以，如果有，顺便携带ACK就行
    if (fk == FRAME_DATA) {
        memcpy(frame.data, out_buf[frame_seq % NR_BUFS], PKT_LEN);
        //填充数据帧
        dbg_frame("Send DATA %d with ACK %d, ID %d""\n", frame.seq, frame.ack, *(short*)frame.data);
        put_frame((seq_nr*)&frame, 3 + PKT_LEN);
        start_timer(frame_seq, DATA_TIMER);
        //启动数据定时器
    }
    else if (fk == FRAME_ACK) {
        dbg_frame(L_BLUE "Send ACK %d" NONE "\n", frame.seq);
        put_frame((seq_nr*)&frame, 2);
        //2是KIND和SEQ，帧格式详情看datalink.h
    }
    else if (fk == FRAME_NAK) {
        Not_NAK = false;
        dbg_frame(L_BLUE "Send NAK %d" NONE "\n", frame.seq);
        put_frame((seq_nr*)&frame, 2);
    }
    stop_ack_timer();
}

static void Move(seq_nr* seq) {
    *seq = (*seq + 1) % (MAX_SEQ + 1); //字符指针后移，移动上界
}

int main(int argc, char** argv) {
    FRAME f;
    seq_nr out_buffer[NR_BUFS][PKT_LEN];   //发送窗口缓存
    seq_nr in_buffer[NR_BUFS][PKT_LEN];    //接收窗口缓存
    seq_nr nbuffered = 0;                //已发送但是还没确认的帧的数量
    seq_nr ack_expected = 0;               //发送窗口下界
    seq_nr next_frame_to_send = 0;                 //发送窗口上界
    seq_nr frame_expected = 0;             //接收窗口下界
    seq_nr too_far = NR_BUFS;         //接收窗口上界
    bool arrived[NR_BUFS];              //记录缓冲区中哪些帧已经到达
    int event, arg;                     //事件号，参数号
    int len = 0;                        //数据包长度

    memset(arrived, 0, sizeof(arrived));

    protocol_init(argc, argv);
    lprintf("Designed by Ma Jiashuai, build: " __DATE__ "  "__TIME__"\n");

    enable_network_layer();
    //初始化，缓冲区为空

    for (;;) {
        //无限循环
        event = wait_for_event(&arg);
        //使用事件驱动函数等待事件的发生
        switch (event) {
        case NETWORK_LAYER_READY:
            //事件是网络层准备好，从网络层收一个包
            //然后将这个包封装成帧向物理层发过去
            get_packet(out_buffer[next_frame_to_send % NR_BUFS]);
            ++nbuffered;
            Send_Frames(FRAME_DATA, next_frame_to_send, (frame_expected + MAX_SEQ) % (MAX_SEQ + 1), out_buffer);
            Move(&next_frame_to_send);
            break;

        case PHYSICAL_LAYER_READY:
            physical_ready = 1;
            //在数据链路层和物理层之间也有流控
            //受限于8000bps的带宽，只有在物理层队列在50字节下，才会physical_ready
            break;

        case FRAME_RECEIVED:
            //从物理层接受一帧
            len = recv_frame((seq_nr*)&f, sizeof f);
            //CRC 校验,len是帧的长度
            if (len < 5 || crc32((seq_nr*)&f, len) != 0) {
                dbg_event(L_RED "**** Receiver Error, Bad CRC Checksum" NONE "\n");
                //校验失败，回传NAK,这里体现了SR的特性，只是针对接受帧序列号回传NAK
                if (f.seq > MAX_SEQ) {
                    Send_Frames(FRAME_NAK, frame_expected, 0, 0);
                }
                else {
                    Send_Frames(FRAME_NAK, f.seq, 0, 0);
                }
                break;
            }

            //收到数据包
            if (f.kind == FRAME_DATA) {
                dbg_frame(WHITE "Recv DATA %d with " L_GREEN "ACK %d" WHITE", ID %d" NONE "\n", f.seq, f.ack, *(short*)f.data);
                if (f.seq != frame_expected && Not_NAK) {
                    //不是期待的帧并且没发NAK，发NAK
                    Send_Frames(FRAME_NAK, frame_expected, 0, 0);
                }
                else {
                    //启动ACK计时器
                    start_ack_timer(ACK_TIMER);
                }
                //帧正确 
                if (is_Between(frame_expected, f.seq, too_far)) {
                    if (!arrived[f.seq % NR_BUFS]) {
                        arrived[f.seq % NR_BUFS] = true;
                        memcpy(in_buffer[f.seq % NR_BUFS], f.data, len - 7);//len-7代表把除了data的部分扔掉
                        //按序交付网络层 
                        while (arrived[frame_expected % NR_BUFS]) {
                            put_packet(in_buffer[frame_expected % NR_BUFS], len - 7);
                            Not_NAK = true;
                            //将缓冲区的标记置0
                            arrived[frame_expected % NR_BUFS] = false;
                            Move(&frame_expected);
                            Move(&too_far);
                            start_ack_timer(ACK_TIMER);
                        }
                    }
                    else {
                        dbg_frame(L_CYAN "DATA %d Already Arrived" NONE "\n", f.seq);
                    }
                }
            }
            //收到NAK包，重发一下对应序号的帧(f.seq)
            if (f.kind == FRAME_NAK) {
                dbg_frame(L_PURPLE "Recv NAK %d" NONE "\n", f.seq);
                if (is_Between(ack_expected, f.seq, next_frame_to_send)) {
                    Send_Frames(FRAME_DATA, f.seq, (frame_expected + MAX_SEQ) % (MAX_SEQ + 1), out_buffer);
                }
                break;
            }
            //收到ACK包
            if (f.kind == FRAME_ACK) {
                dbg_frame(L_GREEN "Recv ACK %d" NONE "\n", f.seq);
                f.ack = f.seq;
            }
            //处理ACK包或捎带ACK 
            while (is_Between(ack_expected, f.ack, next_frame_to_send)) {
                --nbuffered;
                //把对应序号的计时器关了
                stop_timer(ack_expected);
                Move(&ack_expected);
            }
            break;

            //数据定时器超时
        case DATA_TIMEOUT:
            dbg_event(YELLOW "---- DATA %d timeout" NONE "\n", arg);
            //选择重传
            Send_Frames(FRAME_DATA, arg, (frame_expected + MAX_SEQ) % (MAX_SEQ + 1), out_buffer);
            break;

            //ACK超时
        case ACK_TIMEOUT:
            dbg_event(YELLOW "---- ACK timeout" NONE "\n");
            //发送单独ACK包
            Send_Frames(FRAME_ACK, (frame_expected + MAX_SEQ) % (MAX_SEQ + 1), 0, 0);
            break;
        }
        //和网络层的接口
        if (nbuffered < NR_BUFS && physical_ready)
            enable_network_layer();
        else
            disable_network_layer();
    }
}