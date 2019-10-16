# 1. 直接编译
#hello : hello.c a.c
#	gcc -o hello hello.c a.c

# 2. 编译 & 链接 分开
hello : hello.o a.o	# make : 想编译出第一个目标; make clean : 指定具体目标
	gcc -o $@ $^	# $^ 所有依赖

%.o : %.c
	gcc -o 	$@ -c $<	# $< 第一个依赖

clean :	# 不存在名为 clean 文件
	rm *.o hello
