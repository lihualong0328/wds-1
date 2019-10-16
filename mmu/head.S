@*************************************************************************
@ File：head.S
@ 功能：设置SDRAM，将第二部分代码复制到SDRAM，设置页表，启动MMU，
@       然后跳到SDRAM继续执行
@*************************************************************************       

.text
.global _start
_start:
    ldr sp, =4096                       @ 设置栈指针(sram最顶部)，以下都是C函数，调用前需要设好栈
    bl  disable_watch_dog               @ 关闭WATCHDOG，否则CPU会不断重启
    bl  memsetup                        @ 设置存储控制器以使用SDRAM
    bl  copy_2th_to_sdram               @ 将第二部分代码复制到SDRAM
    bl  create_page_table               @ 设置页表
    bl  mmu_init                        @ 启动MMU
    ldr sp, =0xB4000000                 @ 重设栈指针(以上部分pc指向sram)，指向SDRAM顶端(开始使用va)
    ldr pc, =0xB0004000                 @ 跳到SDRAM中继续执行第二部分代码; 相当于 ldr pc, =main; 因为leds.o中只有main(). 这里是为了方便直观的看到va 在起作用
halt_loop:
    b   halt_loop

