obj-m += tmp36.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
install:
	sudo cp 99-tmp36sensor.rules /etc/udev/rules.d/
