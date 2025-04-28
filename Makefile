obj-m += minibinder.o

KERNELDIR ?= ../linux
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -f *.ko *.mod.c *.mod.o *.o modules.order Module.symvers

install:
	sudo insmod minibinder.ko

uninstall:
	sudo rmmod minibinder || true

.PHONY: all clean install uninstall