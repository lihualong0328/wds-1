#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/* thirddrvtest 
  */
int main(int argc, char **argv)
{
	int fd;
	unsigned char key_val;
	
	fd = open("/dev/buttons", O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
	}

	while (1)
	{
		read(fd, &key_val, 1);	//没数据时,不是阻塞,而是kernel中将此进程挂起休眠
		printf("key_val = 0x%x\n", key_val);
	}
	
	return 0;
}

