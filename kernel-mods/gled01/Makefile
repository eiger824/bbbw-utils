obj-m += gled01.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) toggle_gled01.c -o toggle_gled01
mod:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
test:
	$(CC) toggle_gled01.c -o toggle_gled01
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean 
	rm -f toggle_gled01
install:
	sudo cp 99-gled01comp.rules /etc/udev/rules.d/ && chown debian:debian /etc/udev/rules.d/99-gled01comp.rules
