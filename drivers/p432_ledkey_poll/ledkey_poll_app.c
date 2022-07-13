#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>

#define DEVICE_FILENAME "/dev/ledkey_poll"

int main(int argc, char *argv[])
{
	int dev;
	char buff;
	int ret;
	int num = 1;
	struct pollfd Events[2];
	char keyStr[80];

    if(argc != 2)
    {
        printf("Usage : %s [led_data(0x0~0xf)]\n",argv[0]);
        return 1;
    }
    buff = (char)strtoul(argv[1],NULL,16);
    if(!((0 <= buff) && (buff <= 15)))
    {
        printf("Usage : %s [led_data(0x0~0xf)]\n",argv[0]);
        return 2;
    }

//  dev = open(DEVICE_FILENAME, O_RDWR | O_NONBLOCK);
  	dev = open(DEVICE_FILENAME, O_RDWR );
	if(dev < 0)
	{
		perror("open");
		return 2;
	}
	write(dev,&buff,sizeof(buff));

	fflush(stdin);//stdin 입력 버퍼를 깨끗하게 비움.
	memset( Events, 0, sizeof(Events));
  	Events[0].fd = fileno(stdin);//file pointer를 file descriptor로 바꿈
  	Events[0].events = POLLIN;
	Events[1].fd = dev;//디바이스 드라이버 장치 파일 등록.
	Events[1].events = POLLIN;//읽기
	while(1)
	{
		ret = poll(Events, 1, 2000);
		if(ret<0)
		{
			perror("poll");
			exit(1);
		}
		else if(ret==0)
		{
  			printf("poll time out : %d Sec\n",2*num++);
			continue;
		}
		if(Events[0].revents & POLLIN) //keyled
		{
			fgets(keyStr,sizeof(keyStr),stdin);
			if(keyStr[0] == 'q')
				break;
			keyStr[strlen(keyStr)-1] = '\0';
			printf("STDIN : %s\n",keyStr);
			buff = (char)atoi(keyStr);
			write(dev,&buff,sizeof(buff));
		}
		else if(Events[1].revents & POLLIN)  //stdin
		{
			ret = read(dev,&buff,sizeof(buff));
			printf("key_no : %d\n",buff);
			write(dev,&buff,sizeof(buff));
			if(buff == 8)
				break;
		}
	}
	close(dev);
	return 0;
}
