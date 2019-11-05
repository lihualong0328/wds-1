#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>

int main(int argc, char **argv)
{
	int fd;
	unsigned char key_val;
	int ret;
	struct pollfd fds[1];
	
	fd = open("/dev/buttons", O_RDWR);

	fds[0].fd     = fd;	//待查询的文件
	fds[0].events = POLLIN;	//期待返回事件，POLLIN 表示有数据等待读取
	while (1)
	{
		//按下按键立即返回,否则超时返回
		ret = poll(fds, 1, 5000);	//1个fd, 5000ms	//在指定时间内查询是否有动作发生,是会导致系统休眠的查询方式
		if (ret == 0) {
			printf("time out\n");
		} else {
			read(fd, &key_val, 1);
			printf("key_val = 0x%x\n", key_val);
		}
	}
	
	return 0;
}

