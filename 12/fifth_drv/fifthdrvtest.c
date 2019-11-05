
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

int fd;

void my_signal_fun(int signum)
{
	unsigned char key_val;
	read(fd, &key_val, 1);	//app不主动read
	printf("key_val: 0x%x\n", key_val);
}

int main(int argc, char **argv)
{
	unsigned char key_val;
	int ret;
	int Oflags;

	signal(SIGIO, my_signal_fun);	//内核通常发SIGIO表示有数据可读写
	
	fd = open("/dev/buttons", O_RDWR);

	fcntl(fd, F_SETOWN, getpid());	//告诉驱动发给谁, F_SETOWN命令动作由kernel完成

	Oflags = fcntl(fd, F_GETFL); 
	fcntl(fd, F_SETFL, Oflags | FASYNC);//导致调用驱动 fifth_drv_fasync

	while (1) {
		sleep(1000);	// 其他工作
	}
	return 0;
}

