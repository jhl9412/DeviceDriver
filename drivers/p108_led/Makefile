MOD	:= led
obj-m	:= $(MOD).o

CROSS = ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
#KDIR	:= /lib/modules/$(shell uname -r)/build
KDIR	:= /home/ubuntu/udoo_linux_bsp/kernel/3.14.1
PWD		:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules $(CROSS)
	scp $(MOD).ko udooer@10.10.141.62:~
clean:
#	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf .*.cmd
	rm -rf *.o
	rm -rf Module.symvers
	rm -rf modules.order
	rm -rf $(MOD).mod
