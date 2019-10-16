SECTIONS {  /* 把程序分为 first & second 两段 */
  firtst    0x00000000 : { head.o init.o }  /* head.S & init.c 编译出的内容都放在 firtst 段; firtst链接地址是 0x00000000 */
  second    0xB0004000 : AT(2048) { leds.o }  /* leds.c 内容放 second 段; second 链接地址是 0xB0004000 */
} 
 