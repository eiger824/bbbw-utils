all: test_tl-led_tmp36

test_tl-led_tmp36: main.c logger.h strutils.h
	gcc main.c -o test_tl-led_tmp36

clean:
	rm -rf test_tl-led_tmp36

install:
	sudo cp systemd/ledaemon.service /lib/systemd/system/
	sudo systemctl daemon-reload
	sudo systemctl enable ledaemon
