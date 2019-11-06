#include "setup.h"

//其他.c中声明且定义，extern
extern void uart0_init(void);
extern void nand_read(unsigned int addr, unsigned char *buf, unsigned int len);
extern void puts(char *str);
extern void puthex(unsigned int val);


static struct tag *params;

void setup_start_tag(void)
{
	params = (struct tag *)0x30000100;

	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size (tag_core);

	params->u.core.flags = 0;
	params->u.core.pagesize = 0;
	params->u.core.rootdev = 0;

	params = tag_next (params);
}

void setup_memory_tags(void)
{
	params->hdr.tag = ATAG_MEM;
	params->hdr.size = tag_size (tag_mem32);
	
	params->u.mem.start = 0x30000000;
	params->u.mem.size  = 64*1024*1024;
	
	params = tag_next (params);
}

//这里的boot不依赖其他代码
int strlen(char *str)
{
	int i = 0;
	while (str[i])
	{
		i++;
	}
	return i;
}

void strcpy(char *dest, char *src)
{
	while ((*dest++ = *src++) != '\0');
}

void setup_commandline_tag(char *cmdline)
{
	int len = strlen(cmdline) + 1;
	
	params->hdr.tag  = ATAG_CMDLINE;
	params->hdr.size = (sizeof (struct tag_header) + len + 3) >> 2;	//向4取整

	strcpy (params->u.cmdline.cmdline, cmdline);

	params = tag_next (params);
}

void setup_end_tag(void)
{
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;
}


int main(void)
{
	void (*theKernel)(int zero, int arch, unsigned int params);
	volatile unsigned int *p = (volatile unsigned int *)0x30008000;

	/* 0. 帮内核设置串口: 内核启动的开始部分会从串口打印一些信息,但是内核一开始没有初始化串口 */
	/* kernel没法输出时，会一直循环在那里 */
	uart0_init();
	
	/* 1. 从NAND FLASH里把内核读入内存 */
	puts("Copy kernel from nand\n\r");
	//自己写的bootloader只支持从nand读出kernel来启动，不支持烧写kernel到nand,用uboot烧写
	//OpenJTAG> mtd 查看分区
	//OpenJTAG> boot => 看Load Address和Entry Point均=0x30008000，Data Size=1.8M
	//2440/2410都是0x30008000，除非修改kernel配置
	nand_read(0x60000+64, (unsigned char *)0x30008000, 0x200000);	//ih_load==Load Address==0x30008000
	puthex(0x1234ABCD);	//测试puthex()
	puts("\n\r");
	puthex(*p);
	puts("\n\r");

	//2/3参考uboot源码，cmd_bootm.c
	/* 2. 设置参数 */
	puts("Set boot params\n\r");
	setup_start_tag();
	setup_memory_tags();
	setup_commandline_tag("noinitrd root=/dev/mtdblock3 init=/linuxrc console=ttySAC0");
	setup_end_tag();

	/* 3. 跳转执行 */
	puts("Boot kernel\n\r");
	theKernel = (void (*)(int, int, unsigned int))0x30008000;	//0x30008000是ih_ep，这里ih_ep==ih_load
	theKernel(0, 362, 0x30000100);	//跳到0x30008000执行
	/* 
	 *  mov r0, #0
	 *  ldr r1, =362	//362应该不是立即数
	 *  ldr r2, =0x30000100
	 *  mov pc, #0x30008000 
	 */

	puts("Error!\n\r");
	/* 如果一切正常, 不会执行到这里 */

	return -1;
}

