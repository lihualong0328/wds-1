SECTIONS { 
  firtst  	0x00000000 : { head.o init.o nand.o}
  second 	0x30000000 : AT(4096) { main.o }  /* AT(4096): 为了把 main 放到4k sram 之外的 nand */
} 
 
