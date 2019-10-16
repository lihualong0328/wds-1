SECTIONS {
	. = 0x00;	/* 当前地址=0, 从0地址开始排放各个段 */
  	.text          :   { *(.text) }	/* *表示所有源文件, 即crt0.S & leds.c */
	.rodata ALIGN(4) : {*(.rodata)} 
  	.data ALIGN(4) : { *(.data) }
  	.bss ALIGN(4)  : { *(.bss)  *(COMMON) }
}
