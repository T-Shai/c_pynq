obj-m += axi_dma_dev.o

all:
	sudo ./unload_dev.sh
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo ./load_dev.sh
	make -C test

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	make -C test clean