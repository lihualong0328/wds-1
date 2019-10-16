led_on.bin : led_on.S
	arm-linux-gcc -g -c -o led_on.o led_on.S
	arm-linux-ld -Ttext 0x0000000 -g led_on.o -o led_on_elf	# 先生成elf格式, -Ttext: 指定代码段地址
	arm-linux-objcopy -O binary -S led_on_elf led_on.bin	# 把elf格式转为二进制文件, -O binary: 输出二进制
clean:
	rm -f   led_on.bin led_on_elf *.o
