#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "datalink.h"
#include "protocol.h"

#define MAX_SEQ  31
#define NR_BUFS ((MAX_SEQ + 1) / 2)

#define DATA_TIMER 6000
#define ACK_TIMER 800

typedef unsigned char seq_nr;//���к�

typedef struct {
    unsigned char kind; 
    unsigned char seq;
    unsigned char ack;
    unsigned char data[PKT_LEN];
    unsigned int padding;//���
} FRAME;

static bool Not_NAK = true;
//�Ƿ�����NAK
static int physical_ready = 0;
//������Ƿ�׼���÷�������

static int is_Between(seq_nr a, seq_nr b, seq_nr c) {
    //�ж�b�ǲ��������к�a,c֮��
    //�����������Ҳ����ȷ��
    return ((a <= b && b < c) || (c < a && a <= b) || (b < c && c < a));
}

//��ĳһ֡����CRCУ��,ʵ�ֳ�֡
static void put_frame(seq_nr* frame, int len) {
    *(unsigned int*)(frame + len) = crc32(frame, len);
    send_frame(frame, len + 4);
    //���send_frame��protocol�й涨��������㷢֡�ĺ���
    //+4����ΪCRC32������У������32λ�ĸ��ֽ�
    physical_ready = 0;
}

static void Send_Frames(FRAME_KIND fk, seq_nr frame_seq, seq_nr ack_seq, unsigned char out_buf[][PKT_LEN]) {
    //Send_Frames��������װ���ݰ���֡
    FRAME frame;
    frame.kind = fk;
    frame.seq = frame_seq;
    frame.ack = ack_seq;
    //Ϊ�˼�������������ߴ��������ʣ�ʹ����piggybacking
    //������ն�û����ϢҪ���ͣ�������ACK�Ϳ��ԣ�����У�˳��Я��ACK����
    if (fk == FRAME_DATA) {
        memcpy(frame.data, out_buf[frame_seq % NR_BUFS], PKT_LEN);
        //�������֡
        dbg_frame("Send DATA %d with ACK %d, ID %d""\n", frame.seq, frame.ack, *(short*)frame.data);
        put_frame((seq_nr*)&frame, 3 + PKT_LEN);
        start_timer(frame_seq, DATA_TIMER);
        //�������ݶ�ʱ��
    }
    else if (fk == FRAME_ACK) {
        dbg_frame(L_BLUE "Send ACK %d" NONE "\n", frame.seq);
        put_frame((seq_nr*)&frame, 2);
        //2��KIND��SEQ��֡��ʽ���鿴datalink.h
    }
    else if (fk == FRAME_NAK) {
        Not_NAK = false;
        dbg_frame(L_BLUE "Send NAK %d" NONE "\n", frame.seq);
        put_frame((seq_nr*)&frame, 2);
    }
    stop_ack_timer();
}

static void Move(seq_nr* seq) {
    *seq = (*seq + 1) % (MAX_SEQ + 1); //�ַ�ָ����ƣ��ƶ��Ͻ�
}

int main(int argc, char** argv) {
    FRAME f;
    seq_nr out_buffer[NR_BUFS][PKT_LEN];   //���ʹ��ڻ���
    seq_nr in_buffer[NR_BUFS][PKT_LEN];    //���մ��ڻ���
    seq_nr nbuffered = 0;                //�ѷ��͵��ǻ�ûȷ�ϵ�֡������
    seq_nr ack_expected = 0;               //���ʹ����½�
    seq_nr next_frame_to_send = 0;                 //���ʹ����Ͻ�
    seq_nr frame_expected = 0;             //���մ����½�
    seq_nr too_far = NR_BUFS;         //���մ����Ͻ�
    bool arrived[NR_BUFS];              //��¼����������Щ֡�Ѿ�����
    int event, arg;                     //�¼��ţ�������
    int len = 0;                        //���ݰ�����

    memset(arrived, 0, sizeof(arrived));

    protocol_init(argc, argv);
    lprintf("Designed by Ma Jiashuai, build: " __DATE__ "  "__TIME__"\n");

    enable_network_layer();
    //��ʼ����������Ϊ��

    for (;;) {
        //����ѭ��
        event = wait_for_event(&arg);
        //ʹ���¼����������ȴ��¼��ķ���
        switch (event) {
        case NETWORK_LAYER_READY:
            //�¼��������׼���ã����������һ����
            //Ȼ���������װ��֡������㷢��ȥ
            get_packet(out_buffer[next_frame_to_send % NR_BUFS]);
            ++nbuffered;
            Send_Frames(FRAME_DATA, next_frame_to_send, (frame_expected + MAX_SEQ) % (MAX_SEQ + 1), out_buffer);
            Move(&next_frame_to_send);
            break;

        case PHYSICAL_LAYER_READY:
            physical_ready = 1;
            //��������·��������֮��Ҳ������
            //������8000bps�Ĵ���ֻ��������������50�ֽ��£��Ż�physical_ready
            break;

        case FRAME_RECEIVED:
            //����������һ֡
            len = recv_frame((seq_nr*)&f, sizeof f);
            //CRC У��,len��֡�ĳ���
            if (len < 5 || crc32((seq_nr*)&f, len) != 0) {
                dbg_event(L_RED "**** Receiver Error, Bad CRC Checksum" NONE "\n");
                //У��ʧ�ܣ��ش�NAK,����������SR�����ԣ�ֻ����Խ���֡���кŻش�NAK
                if (f.seq > MAX_SEQ) {
                    Send_Frames(FRAME_NAK, frame_expected, 0, 0);
                }
                else {
                    Send_Frames(FRAME_NAK, f.seq, 0, 0);
                }
                break;
            }

            //�յ����ݰ�
            if (f.kind == FRAME_DATA) {
                dbg_frame(WHITE "Recv DATA %d with " L_GREEN "ACK %d" WHITE", ID %d" NONE "\n", f.seq, f.ack, *(short*)f.data);
                if (f.seq != frame_expected && Not_NAK) {
                    //�����ڴ���֡����û��NAK����NAK
                    Send_Frames(FRAME_NAK, frame_expected, 0, 0);
                }
                else {
                    //����ACK��ʱ��
                    start_ack_timer(ACK_TIMER);
                }
                //֡��ȷ 
                if (is_Between(frame_expected, f.seq, too_far)) {
                    if (!arrived[f.seq % NR_BUFS]) {
                        arrived[f.seq % NR_BUFS] = true;
                        memcpy(in_buffer[f.seq % NR_BUFS], f.data, len - 7);//len-7����ѳ���data�Ĳ����ӵ�
                        //���򽻸������ 
                        while (arrived[frame_expected % NR_BUFS]) {
                            put_packet(in_buffer[frame_expected % NR_BUFS], len - 7);
                            Not_NAK = true;
                            //���������ı����0
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
            //�յ�NAK�����ط�һ�¶�Ӧ��ŵ�֡(f.seq)
            if (f.kind == FRAME_NAK) {
                dbg_frame(L_PURPLE "Recv NAK %d" NONE "\n", f.seq);
                if (is_Between(ack_expected, f.seq, next_frame_to_send)) {
                    Send_Frames(FRAME_DATA, f.seq, (frame_expected + MAX_SEQ) % (MAX_SEQ + 1), out_buffer);
                }
                break;
            }
            //�յ�ACK��
            if (f.kind == FRAME_ACK) {
                dbg_frame(L_GREEN "Recv ACK %d" NONE "\n", f.seq);
                f.ack = f.seq;
            }
            //����ACK�����Ӵ�ACK 
            while (is_Between(ack_expected, f.ack, next_frame_to_send)) {
                --nbuffered;
                //�Ѷ�Ӧ��ŵļ�ʱ������
                stop_timer(ack_expected);
                Move(&ack_expected);
            }
            break;

            //���ݶ�ʱ����ʱ
        case DATA_TIMEOUT:
            dbg_event(YELLOW "---- DATA %d timeout" NONE "\n", arg);
            //ѡ���ش�
            Send_Frames(FRAME_DATA, arg, (frame_expected + MAX_SEQ) % (MAX_SEQ + 1), out_buffer);
            break;

            //ACK��ʱ
        case ACK_TIMEOUT:
            dbg_event(YELLOW "---- ACK timeout" NONE "\n");
            //���͵���ACK��
            Send_Frames(FRAME_ACK, (frame_expected + MAX_SEQ) % (MAX_SEQ + 1), 0, 0);
            break;
        }
        //�������Ľӿ�
        if (nbuffered < NR_BUFS && physical_ready)
            enable_network_layer();
        else
            disable_network_layer();
    }
}