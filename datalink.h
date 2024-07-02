
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


#define NONE          "\x1b[0m"         //清除颜色，即之后的打印为正常输出，之前的不受影响
#define BLACK         "\x1b[0;30m"      //深黑
#define L_BLACK       "\x1b[1;30m"      //亮黑，偏灰褐
#define RED           "\x1b[0;31m"      //深红，暗红
#define L_RED         "\x1b[1;31m"      //鲜红
#define GREEN         "\x1b[0;32m"      //深绿，暗绿
#define L_GREEN       "\x1b[1;32m"      //鲜绿
#define BROWN         "\x1b[0;33m"      //深黄，暗黄
#define YELLOW        "\x1b[1;33m"      //鲜黄
#define BLUE          "\x1b[0;34m"      //深蓝，暗蓝
#define L_BLUE        "\x1b[1;34m"      //亮蓝，偏白灰
#define PURPLE        "\x1b[0;35m"      //深粉，暗粉，偏暗紫
#define L_PURPLE      "\x1b[1;35m"      //亮粉，偏白灰
#define CYAN          "\x1b[0;36m"      //暗青色
#define L_CYAN        "\x1b[1;36m"      //鲜亮青色
#define GRAY          "\x1b[0;37m"      //灰色
#define WHITE         "\x1b[1;37m"      //白色，字体粗一点，比正常大，比bold小
#define BOLD          "\x1b[1m"         //白色，粗体
#define UNDERLINE     "\x1b[4m"         //下划线，白色，正常大小
#define BLINK         "\x1b[5m"         //闪烁，白色，正常大小
#define REVERSE       "\x1b[7m"         //反转，即字体背景为白色，字体为黑色
#define HIDE          "\x1b[8m"         //隐藏
#define CLEAR         "\x1b[2J"         //清除
#define CLRLINE       "\r\x1b[K"        //清除行
