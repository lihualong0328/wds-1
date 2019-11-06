
#define S3C2440_MPLL_200MHZ     ((0x5c<<12)|(0x01<<4)|(0x02))
#define MEM_CTL_BASE    0x48000000

.text							 	/* 代码段 */
.global _start						/* _start是全局标号 */
_start:
/* 1. 关看门狗；参考 nand 裸板 */
/* 有=的ldr指令是伪汇编指令，编译时编译器会把其转为真汇编指令，见反汇编 */
/* 如果 0x53000000 能表示为立即数，编译时直接转为mov r0, 0x53000000；否则把 0x53000000 存在内存，再读内存(两条指令) */
	ldr r0, =0x53000000
	mov r1, #0
	str r1, [r0]	/* 把r1值存储到r0地址 */

/* 2. 设置时钟，参考 lcd 裸板 */
	ldr r0, =0x4c000014
	mov r1, #0x03;            /* ldr r1, =0x03; 因为0x03简单, 所以可直接用mov */
	str r1, [r0]

	/* 如果HDIVN非0，CPU的总线模式应该从“fast bus mode”变为“asynchronous bus mode” */
	/* 2440手册要求 : 当cpu时钟FCLK!=HCLK(内存时钟)时，就要设为"asynchronous bus mode" */
	mrc	p15, 0, r1, c1, c0, 0		/* 读出控制寄存器 */ 
	orr	r1, r1, #0xc0000000			/* 设置为“asynchronous bus mode” */
	mcr	p15, 0, r1, c1, c0, 0		/* 写入控制寄存器 */

	/* MPLLCON = S3C2440_MPLL_200MHZ */
	ldr r0, =0x4c000004
	ldr r1, =S3C2440_MPLL_200MHZ	/* S3C2440_MPLL_200MHZ 复杂，可能没法用立即数表示，所以用伪汇编 */
	str r1, [r0]

/* 3. 初始化SDRAM */
	ldr r0, =MEM_CTL_BASE	/* 伪汇编 */
	adr r1, sdram_config    /* 伪汇编，sdram_config标号的当前地址（非链接地址） */
	add r3, r0, #(13*4)
1:
	ldr r2, [r1], #4		/* 读取r1地址的值存到r2，再r1+4 */
	str r2, [r0], #4		/* 把r2值存到r0地址，再r0+4 */
	cmp r0, r3
	bne 1b					/* 1b,1back向回跳，可以有多个1标号，跳时可以1f & 1b区别 */

/* 4. 重定位 : 把 bootloader 自身代码从 flash 复制到它的链接地址去 */
	ldr sp, =0x34000000	/* sdram 基地址=0x30000000，64M=4000000；先指向最高内存，栈是向下增长的 */

	bl nand_init

	/* 为 copy_code_to_sdram(r0, r1, r2) 准备形参 */
	//ldr r1, =_start	//取_start的地址，位置相关
	//ldr r1, _start	//取_start处的值，位置无关
	mov r0, #0				/* nor时，0地址就是nor，可直接操作；nand时，0地址是片内ram，但这里用的是nand_read()，0地址是nand的0地址 */
	ldr r1, =_start			/* dest 是链接地址(_start 标号地址==链接地址) */
	ldr r2, =__bss_start	
	sub r2, r2, r1			/* len 是 uboot.bin 大小，见有道 */
	
	bl copy_code_to_sdram	/* 重定位 */
	bl clear_bss
	
/* 5. 执行main */
	/* bl main */	/* bl是相对跳转，如果从0地址开始执行，相对跳转不能跳到 sdram；bl是根据当前指令地址，找到 main 的 offset 值再跳 */
	ldr lr, =halt	/* 设置 main 返回地址 */
	ldr pc, =main	/* 跳转到 sdram */
halt:				/* main 返回在此死循环，避免跑飞 */
	b halt

/* 数据 : start.S里除了指令，还有如下数据部分 */
sdram_config:
	.long 0x22011110	 //BWSCON
	.long 0x00000700	 //BANKCON0
	.long 0x00000700	 //BANKCON1
	.long 0x00000700	 //BANKCON2
	.long 0x00000700	 //BANKCON3  
	.long 0x00000700	 //BANKCON4
	.long 0x00000700	 //BANKCON5
	.long 0x00018005	 //BANKCON6
	.long 0x00018005	 //BANKCON7
	.long 0x008C04F4	 //REFRESH
	.long 0x000000B1	 //BANKSIZE
	.long 0x00000030	 //MRSRB6
	.long 0x00000030	 //MRSRB7

