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

	fds[0].fd     = fd;	//����ѯ���ļ�
	fds[0].events = POLLIN;	//�ڴ������¼���POLLIN ��ʾ�����ݵȴ���ȡ
	while (1)
	{
		//���°�����������,����ʱ����
		ret = poll(fds, 1, 5000);	//1��fd, 5000ms	//��ָ��ʱ���ڲ�ѯ�Ƿ��ж�������,�ǻᵼ��ϵͳ���ߵĲ�ѯ��ʽ
		if (ret == 0) {
			printf("time out\n");
		} else {
			read(fd, &key_val, 1);
			printf("key_val = 0x%x\n", key_val);
		}
	}
	
	return 0;
}

