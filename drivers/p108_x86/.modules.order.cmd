cmd_/home/ubuntu/udoo_linux_bsp/drivers/p108/modules.order := {   echo /home/ubuntu/udoo_linux_bsp/drivers/p108/hello.ko; :; } | awk '!x[$$0]++' - > /home/ubuntu/udoo_linux_bsp/drivers/p108/modules.order