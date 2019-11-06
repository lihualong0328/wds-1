#include "setup.h"

//����.c�������Ҷ��壬extern
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

//�����boot��������������
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
	params->hdr.size = (sizeof (struct tag_header) + len + 3) >> 2;	//��4ȡ��

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

	/* 0. ���ں����ô���: �ں������Ŀ�ʼ���ֻ�Ӵ��ڴ�ӡһЩ��Ϣ,�����ں�һ��ʼû�г�ʼ������ */
	/* kernelû�����ʱ����һֱѭ�������� */
	uart0_init();
	
	/* 1. ��NAND FLASH����ں˶����ڴ� */
	puts("Copy kernel from nand\n\r");
	//�Լ�д��bootloaderֻ֧�ִ�nand����kernel����������֧����дkernel��nand,��uboot��д
	//OpenJTAG> mtd �鿴����
	//OpenJTAG> boot => ��Load Address��Entry Point��=0x30008000��Data Size=1.8M
	//2440/2410����0x30008000�������޸�kernel����
	nand_read(0x60000+64, (unsigned char *)0x30008000, 0x200000);	//ih_load==Load Address==0x30008000
	puthex(0x1234ABCD);	//����puthex()
	puts("\n\r");
	puthex(*p);
	puts("\n\r");

	//2/3�ο�ubootԴ�룬cmd_bootm.c
	/* 2. ���ò��� */
	puts("Set boot params\n\r");
	setup_start_tag();
	setup_memory_tags();
	setup_commandline_tag("noinitrd root=/dev/mtdblock3 init=/linuxrc console=ttySAC0");
	setup_end_tag();

	/* 3. ��תִ�� */
	puts("Boot kernel\n\r");
	theKernel = (void (*)(int, int, unsigned int))0x30008000;	//0x30008000��ih_ep������ih_ep==ih_load
	theKernel(0, 362, 0x30000100);	//����0x30008000ִ��
	/* 
	 *  mov r0, #0
	 *  ldr r1, =362	//362Ӧ�ò���������
	 *  ldr r2, =0x30000100
	 *  mov pc, #0x30008000 
	 */

	puts("Error!\n\r");
	/* ���һ������, ����ִ�е����� */

	return -1;
}

