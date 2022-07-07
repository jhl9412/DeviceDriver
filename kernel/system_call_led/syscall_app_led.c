#include <autoconf.h>
#include <stdio.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <stdlib.h>
void print_led(unsigned long l)
{
	int i=0, j=1;
	while(i!=4)
	{
	    if((l>>i)&1) putchar('O');
	    else putchar('X');
	    if(i!=3) putchar(':');
	    else putchar('\n');
	    i++;
	    j<<=1;
	}
}

int main(int argc, char **argv)
{
	unsigned long l=0;
	int flag=0;
	if(argc==2)
	{
//		l=strtoul(argv[1],NULL,32);
		l=atol(argv[1]);
		l=syscall(__NR_mysyscall, l);
		if(l<0)
		{
			perror("syscall");
			exit(3);
		}
		puts("1:2:3:4");
		print_led(l);
		return 0;
	}
	else if(argc==1)
	{
		puts("1:2:3:4");
		while(1)
		{	
			if(flag){
				l = syscall(__NR_mysyscall, l);
				print_led(l);
				l<<=1;
				if(l==0) l=1;
				if(l>8) flag =0;
			}
			else
			{
				l = syscall(__NR_mysyscall, l);
				print_led(l);
				l>>=1;
				if(l<1) flag =1;
			}
			if(l<0)
			{
				perror("syscall");
				exit(3);
			}
			usleep(200000);
		}
	}
	return 0;
}
