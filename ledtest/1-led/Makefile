all: test_gled01_tmp36

test_gled01_tmp36: main.o strutils.o logger.o 
	gcc -Wall -Wextra -std=c11 $^ -o test_gled01_tmp36

main.o: main.c
	gcc -c -Wall -Wextra -std=c11 $< -o $@

%.o: %.c %.h
	gcc -c -Wall -Wextra -std=c11 $< -o $@

clean:
	rm -rf test_gled01_tmp36 *~ *o

install:
	sudo cp systemd/ledaemon.service /lib/systemd/system/
	sudo systemctl daemon-reload
	sudo systemctl enable ledaemon
