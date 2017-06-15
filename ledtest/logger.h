#ifndef BBBW_LOGGER_H_
#define BBBW_LOGGER_H_

#include "strutils.h"

FILE *ft; // File descriptor used for reading from P9_40
FILE *fd; // File descriptor used for interacting with the led
FILE *fl; // File descriptor used for the log file

const char* log_path = "/var/log/ledaemon.log";

bool silent = false;

int write_2_file(const char* name, const char* value)
{
	fd = fopen(name, "w");
	if (fd == NULL) return -1;
	fputs(value, fd);
	fclose(fd);
	return 0;
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
