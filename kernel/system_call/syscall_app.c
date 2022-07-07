#include <autoconf.h>
#include <stdio.h>
#include <unistd.h>
#include <asm/unistd.h>

int main()
{
	long l;
	printf("input value = ");
	scanf("%ld", &l);
	l = syscall(__NR_mysyscall, l);
	if(l<0)
	{
		perror("syscall");
		return 1;
	}
	printf("mysyscall return value = %ld\n", l);
	return 0;
}
