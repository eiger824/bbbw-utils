obj-m += tl-led.o

all: mod

mod:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean 
install:
	sudo cp udev/99-tl-led.rules /etc/udev/rules.d/ && chown debian:debian /etc/udev/rules.d/99-tl-led.rules
