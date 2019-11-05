
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
	read(fd, &key_val, 1);	//app������read
	printf("key_val: 0x%x\n", key_val);
}

int main(int argc, char **argv)
{
	unsigned char key_val;
	int ret;
	int Oflags;

	signal(SIGIO, my_signal_fun);	//�ں�ͨ����SIGIO��ʾ�����ݿɶ�д
	
	fd = open("/dev/buttons", O_RDWR);

	fcntl(fd, F_SETOWN, getpid());	//������������˭, F_SETOWN�������kernel���

	Oflags = fcntl(fd, F_GETFL); 
	fcntl(fd, F_SETFL, Oflags | FASYNC);//���µ������� fifth_drv_fasync

	while (1) {
		sleep(1000);	// ��������
	}
	return 0;
}

