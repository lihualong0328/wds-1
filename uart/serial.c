#include "s3c24xx.h"
#include "serial.h"

#define TXD0READY   (1<<2)
#define RXD0READY   (1)

#define PCLK            50000000    // init.c 中的 clock_init 函数设置 PCLK 为50MHz
#define UART_CLK        PCLK        //  UART0 的时钟源设为 PCLK
#define UART_BAUD_RATE  115200      // 波特率
#define UART_BRD        ((UART_CLK  / (UART_BAUD_RATE * 16)) - 1)

/*
 * 初始化UART0
 * 115200,8N1,无流控
 */
void uart0_init(void)
{
    GPHCON  |= 0xa0;    // GPH2,GPH3用作串口的 TXD0,RXD0
    GPHUP   = 0x0c;     // GPH2,GPH3内部上拉，可忽略

    ULCON0  = 0x03;     // 8N1(8个数据位，无较验，1个停止位)，要和串口工具保持一致
    UCON0   = 0x05;     // 查询方式 & UART时钟源为PCLK，对应[11:10]；串口通过查询 / 中断知道数据发送 / 接收完
    UFCON0  = 0x00;     // 不使用FIFO
    UMCON0  = 0x00;     // 不使用流控
    UBRDIV0 = UART_BRD; // 波特率为115200
}

/*
 * 发送一个字符
 */
void putc(unsigned char c)
{
    /* 等待，直到发送缓冲区中的上次数据已全部发送出去，否则会覆盖 */
    while (!(UTRSTAT0 & TXD0READY));
    
    /* 向UTXH0寄存器中写入数据，UART自动发送出去 */
    UTXH0 = c;
}

/*
 * 接收字符
 */
unsigned char getc(void)
{
    /* 等待，直到接收缓冲区中的有数据 */
    while (!(UTRSTAT0 & RXD0READY));    // 状态寄存器 UTRSTAT0 的bit0
    
    /* 直接读取URXH0寄存器，即可获得接收到的数据 */
    return URXH0;
}

/*
 * 判断一个字符是否数字
 */
int isDigit(unsigned char c)
{
    if (c >= '0' && c <= '9')
        return 1;
    else
        return 0;       
}

/*
 * 判断一个字符是否英文字母
 */
int isLetter(unsigned char c)
{
    if (c >= 'a' && c <= 'z')
        return 1;
    else if (c >= 'A' && c <= 'Z')
        return 1;       
    else
        return 0;
}
