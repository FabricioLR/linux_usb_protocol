obj-m := mouse.o

default:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
	
logs:
	dmesg --follow-new

load:
	insmod mouse.ko

remove:
	rmmod mouse
