KBUILD_EXTRA_SYMBOLS := /lib/modules/4.4.50+/source/drivers/usb/gadget/Module.symvers
obj-m += connModule.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
