#ifndef BBBW_LOGGER_H_
#define BBBW_LOGGER_H_

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "strutils.h"

#define TLLED_DEV "/dev/tl-led"

FILE *ft; // File descriptor used for reading from P9_40
FILE *fd; // File descriptor used for backup 
FILE *fl; // File descriptor used for the log file
int fled; // File descriptor used for interacting with the led

const char* log_path = "/var/log/ledaemon.log";

bool silent = false;

int write_2_led(const char* lednr, const char* value);
void remove_log_file();
void set_silent(bool s);
void logger(const char* msg);
int read_from_file(const char* name, char* buffer);


int write_2_led(const char* lednr, const char* value)
{
	int ret;
	char pair[3];
	pair[0] = lednr;
	pair[1] = value;
	pair[2] = 0;
	fled = open(TLLED_DEV, O_RDWR);
	if (fd < 0) return -1;
	ret = write(fled, pair, 2);
	close(fled);
	if (ret < 0) return errno; 
	return 0;
}

void remove_log_file()
{
	if (!unlink(log_path)) logger ("Success!");
	else logger (msg_err("Error", errno) );
}

void set_silent(bool s)
{
	silent = s;
}

void logger(const char* msg)
{
	char buffer[200];
	strcpy(buffer, "[");
	time_t current_time;
	char *c_time_string;
	current_time = time(NULL);
	c_time_string = ctime(&current_time);
	c_time_string[strlen(c_time_string)-1] = 0;
	strcat(buffer, c_time_string);
	strcat(buffer, "] ");
	strcat(buffer, msg);
	strcat(buffer, "\n");
	
	if (!silent)
	{
		printf(buffer);	
	}
	else
	{
		fl = fopen(log_path, "a");
		if (fl != NULL)
		{
			fputs(buffer, fl);
			fclose(fl);
		}
		else printf("Could not open\n");
	}
}

int read_from_file(const char* name, char* buffer)
{
	char io_state[1];
	buffer = malloc(2);
	fd = fopen(name, "r");
	if (fd == NULL) return -1;
	if (fgets (io_state, 1, fd) != NULL)
	{
		strcpy(buffer,io_state);
		buffer[1] = 0; //null terminated
		fclose(fd);
		return 0;
	}
	else
	{
		return -1;
	}
}

#endif /* BBBW_LOGGER_H_*/
