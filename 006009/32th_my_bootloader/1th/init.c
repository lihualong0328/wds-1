
/* NAND FLASH控制器 */
#define NFCONF (*((volatile unsigned long *)0x4E000000))
#define NFCONT (*((volatile unsigned long *)0x4E000004))
#define NFCMMD (*((volatile unsigned char *)0x4E000008))	//发出命令，eg>读写命令，把命令值写入reg即可；NFCMMD 是 8bit, 所以unsigned char *
#define NFADDR (*((volatile unsigned char *)0x4E00000C))
#define NFDATA (*((volatile unsigned char *)0x4E000010))	//读写数据
//NFMECCD0/1	//nand缺陷:读时可能位反转，即大部分对，可能某一位出错，需要校验码
#define NFSTAT (*((volatile unsigned char *)0x4E000020))

/* GPIO */
#define GPHCON              (*(volatile unsigned long *)0x56000070)
#define GPHUP               (*(volatile unsigned long *)0x56000078)

/* UART registers */
#define ULCON0              (*(volatile unsigned long *)0x50000000)
#define UCON0               (*(volatile unsigned long *)0x50000004)
#define UFCON0              (*(volatile unsigned long *)0x50000008)
#define UMCON0              (*(volatile unsigned long *)0x5000000c)
#define UTRSTAT0            (*(volatile unsigned long *)0x50000010)
#define UTXH0               (*(volatile unsigned char *)0x50000020)
#define URXH0               (*(volatile unsigned char *)0x50000024)
#define UBRDIV0             (*(volatile unsigned long *)0x50000028)

#define TXD0READY   (1<<2)


void nand_read(unsigned int addr, unsigned char *buf, unsigned int len);


int isBootFromNorFlash(void)
{
	volatile int *p = (volatile int *)0;
	int val;

	val = *p;
	*p = 0x12345678;
	if (*p == 0x12345678)
	{
		/* 写成功, 是nand启动 */
		*p = val;
		return 0;
	}
	else
	{
		/* NOR不能像内存一样写 */
		return 1;
	}
}

void copy_code_to_sdram(unsigned char *src, unsigned char *dest, unsigned int len)
{	
	int i = 0;
	
	/* 如果是NOR启动 */
	//利用nor特点，读时同内存，写时不同内存
	//nand启动，0地址是片内4k ram，可写可读
	if (isBootFromNorFlash())
	{
		while (i < len)
		{
			dest[i] = src[i];
			i++;
		}
	}
	else
	{
		//nand_init();	//即使nor启动，也要去nand读取kernel
		nand_read((unsigned int)src, dest, len);
	}
}

void clear_bss(void)
{
	//汇编中可直接引入__bss_start变量，c中extern
	extern int __bss_start, __bss_end;
	int *p = &__bss_start;
	
	for (; p < &__bss_end; p++)
		*p = 0;
}

//hardware/lcd/nand.c是兼容2410/2440，复杂，这里手写
void nand_init(void)
{
#define TACLS   0
#define TWRPH0  1
#define TWRPH1  0
	/* 设置时序 */
	NFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);
	/* 使能NAND Flash控制器, 初始化ECC, 禁止片选 */
	NFCONT = (1<<4)|(1<<1)|(1<<0);		/* nand总线宽度=8bit, BusWidth=0 */
}

void nand_select(void)
{
	NFCONT &= ~(1<<1);	
}

void nand_deselect(void)
{
	NFCONT |= (1<<1);	
}

void nand_cmd(unsigned char cmd)
{
	volatile int i;
	NFCMMD = cmd;
	for (i = 0; i < 10; i++);	//调试发现：发出之后需等待一会再发地址 nand_addr
}

void nand_addr(unsigned int addr)
{
	unsigned int col  = addr % 2048;	//"2048"编译出错时，可采取移位	//重定位时，从0地址开始拷贝，且boot.bin<2K，检查不出col=addr/2048的错误
	unsigned int page = addr / 2048;
	volatile int i;

	NFADDR = col & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR = (col >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	
	NFADDR  = page & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 16) & 0xff;
	for (i = 0; i < 10; i++);
}

void nand_wait_ready(void)
{
	while (!(NFSTAT & 1));	//原理图状态RnB引脚，1 is Ready, 0 is Busy
}

unsigned char nand_data(void)
{
	return NFDATA;
}

//nand 8个数据线，所以ale=1,发地址; cle=1,发命令
void nand_read(unsigned int addr, unsigned char *buf, unsigned int len)
{
	int col = addr % 2048;		//从一页的某个位置读
	int i = 0;
		
	/* 1. 选中(片选) */			//操作nand，需要先发出片选信号nFCE，即允许/禁止nand控制器发出片选信号(读写命令/地址时，自动发出片选)
	nand_select();

	while (i < len)
	{
		/* 2. 发出读命令00h */
		nand_cmd(0x00);

		/* 3. 发出地址(分5步发出) */
		nand_addr(addr);

		/* 4. 发出读命令30h */	//确定去读，其他nand，书中nand比较小，读写命令不一样，eg>只需发00h即可
		nand_cmd(0x30);

		/* 5. 判断状态 */		//nand智能根据读地址找到此页数据，并把页数据读到Page Register中，需要时间
		nand_wait_ready();

		/* 6. 读数据 */			//暂不关心oob，读页数据即可，读下一页，接着2
		for (; (col < 2048) && (i < len); col++)	//读到页结尾且<len
		{
			buf[i] = nand_data();
			i++;
			addr++;
		}
		
		col = 0;				//第二次读时，是连续的，col肯定=0
	}

	/* 7. 取消选中 */			//读完		
	nand_deselect();
}


#define PCLK            50000000	// init.c中的clock_init函数设置PCLK为50MHz
#define UART_CLK        PCLK		// UART0的时钟源设为PCLK
#define UART_BAUD_RATE  115200		// 波特率
#define UART_BRD        ((UART_CLK  / (UART_BAUD_RATE * 16)) - 1)

/*
 * 初始化UART0
 * 115200,8N1,无流控
 */
void uart0_init(void)
{
    GPHCON  |= 0xa0;    // GPH2,GPH3用作TXD0,RXD0
    GPHUP   = 0x0c;     // GPH2,GPH3内部上拉

    ULCON0  = 0x03;     // 8N1(8个数据位，无较验，1个停止位)
    UCON0   = 0x05;     // 查询方式，UART时钟源为PCLK
    UFCON0  = 0x00;     // 不使用FIFO
    UMCON0  = 0x00;     // 不使用流控
    UBRDIV0 = UART_BRD; // 波特率为115200
}

/*
 * 发送一个字符
 */
void putc(unsigned char c)
{
    /* 等待，直到发送缓冲区中的数据已经全部发送出去 */
    while (!(UTRSTAT0 & TXD0READY));
    
    /* 向UTXH0寄存器中写入数据，UART即自动将它发送出去 */
    UTXH0 = c;
}

void puts(char *str)
{
	int i = 0;
	while (str[i])
	{
		putc(str[i]);
		i++;
	}
}

// TEST
void puthex(unsigned int val)
{
	/* 0x1234abcd */
	int i;
	int j;
	
	puts("0x");

	for (i = 0; i < 8; i++)
	{
		j = (val >> ((7-i)*4)) & 0xf;
		if ((j >= 0) && (j <= 9))
			putc('0' + j);
		else
			putc('A' + j - 0xa);
		
	}
	
}

