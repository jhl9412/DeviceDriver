#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "ioctl_test.h"
#define DEVICE_FILENAME  "/dev/ledkey_block"
int main()
{
    ioctl_test_info info={0,{0}};
	int dev;
    char buff = 15;
    int ret;
    int key_old = 0;
	int cnt=0;
	double intval=0.0;
//	dev = open( DEVICE_FILENAME, O_RDWR|O_NONBLOCK);
    dev = open( DEVICE_FILENAME, O_RDWR);//block mode
	if(dev<0)
	{
		perror("open()");
		return 1;
	}
    ret = write(dev,&buff,sizeof(buff));
	if(ret < 0)
		perror("write()");
	buff = 0;
	do {
	    info.size = 0;
        ret=ioctl(dev, IOCTLTEST_READ, &info );
        intval = info.size/100.0;
		buff = info.buff[0];
		printf("intval : %.2lf\n",intval);

		printf("ret : %d, cnt : %d\n",ret,cnt++);
		if(ret < 0){
  			perror("read()");
			return 1;
		}
  		if(info.size == 0) printf("timeout\n");
		if(buff ==0) continue;
		if(buff != key_old)
		{
			if(buff)
			{
				printf("key_no : %d\n",buff);
				write(dev,&buff,sizeof(buff));
			}
			if(buff == 8)
				break;
			key_old = buff;
		}
	} while(1);
    close(dev);
    return 0;
}
