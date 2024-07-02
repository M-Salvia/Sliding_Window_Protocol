
/* FRAME kind */
typedef unsigned char FRAME_KIND;
#define FRAME_DATA 0
#define FRAME_ACK 1
#define FRAME_NAK 2
/*
    DATA Frame
    +=========+========+========+===============+========+
    | KIND(1) | SEQ(1) | ACK(1) | DATA(240~256) | CRC(4) |
    +=========+========+========+===============+========+

    ACK Frame
    +=========+========+========+
    | KIND(1) | SEQ(1) | CRC(4) |
    +=========+========+========+

    NAK Frame
    +=========+========+========+
    | KIND(1) | SEQ(1) | CRC(4) |
    +=========+========+========+
*/


#define NONE          "\x1b[0m"         //�����ɫ����֮��Ĵ�ӡΪ���������֮ǰ�Ĳ���Ӱ��
#define BLACK         "\x1b[0;30m"      //���
#define L_BLACK       "\x1b[1;30m"      //���ڣ�ƫ�Һ�
#define RED           "\x1b[0;31m"      //��죬����
#define L_RED         "\x1b[1;31m"      //�ʺ�
#define GREEN         "\x1b[0;32m"      //���̣�����
#define L_GREEN       "\x1b[1;32m"      //����
#define BROWN         "\x1b[0;33m"      //��ƣ�����
#define YELLOW        "\x1b[1;33m"      //�ʻ�
#define BLUE          "\x1b[0;34m"      //����������
#define L_BLUE        "\x1b[1;34m"      //������ƫ�׻�
#define PURPLE        "\x1b[0;35m"      //��ۣ����ۣ�ƫ����
#define L_PURPLE      "\x1b[1;35m"      //���ۣ�ƫ�׻�
#define CYAN          "\x1b[0;36m"      //����ɫ
#define L_CYAN        "\x1b[1;36m"      //������ɫ
#define GRAY          "\x1b[0;37m"      //��ɫ
#define WHITE         "\x1b[1;37m"      //��ɫ�������һ�㣬�������󣬱�boldС
#define BOLD          "\x1b[1m"         //��ɫ������
#define UNDERLINE     "\x1b[4m"         //�»��ߣ���ɫ��������С
#define BLINK         "\x1b[5m"         //��˸����ɫ��������С
#define REVERSE       "\x1b[7m"         //��ת�������屳��Ϊ��ɫ������Ϊ��ɫ
#define HIDE          "\x1b[8m"         //����
#define CLEAR         "\x1b[2J"         //���
#define CLRLINE       "\r\x1b[K"        //�����
