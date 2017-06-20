obj-m += tmp36.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) test/test_tmp36.c -o test_tmp36
mod:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules

test: test/test_tmp36.c
	$(CC) test/test_tmp36.c -o test_tmp36

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm -f test_tmp36
install:
	sudo cp udev/99-tmp36sensor.rules /etc/udev/rules.d/
