obj-m += tsulab.o
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
all: run test clean
	dmesg | tail -n 8
run:
	make -C $(KERNELDIR) M=$(PWD) modules
	insmod tsulab.ko
clean:
	rmmod tsulab.ko
	make -C $(KERNELDIR) M=$(PWD) clean
test:
	cat /proc/tsu
	cat /proc/tsu
	cat /proc/tsu
	cat /proc/tsu