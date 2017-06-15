obj-m += tmp36.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) tmp36_test.c -o tmp36_test
mod:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules

test: tmp36_test.c
	$(CC) tmp36_test.c -o tmp36_test

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm -f tmp36_test
install:
	sudo cp 99-tmp36sensor.rules /etc/udev/rules.d/
