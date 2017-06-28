obj-m += tl-led.o

all: 
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	gcc test/test-tl-led.c -o test-tl-led

mod:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules

test:
	gcc test/test-tl-led.c -o test-tl-led

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean 
	rm -f test-tl-led
install:
	sudo cp udev/99-tl-led.rules /etc/udev/rules.d/ && chown debian:debian /etc/udev/rules.d/99-tl-led.rules
