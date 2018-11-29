#ifndef BBBW_LOGGER_H_
#define BBBW_LOGGER_H_

#include <stdbool.h>

#define GLED01_DEV "/dev/gled01"
#define LOG_PATH  "/var/log/ledaemon.log"

FILE *ft; // File descriptor used for reading from P9_40
FILE *fd; // File descriptor used for backup 
FILE *fl; // File descriptor used for the log file
int fled; // File descriptor used for interacting with the led

int write_2_led(const char* value);
void remove_log_file();
void set_silent(bool s);
void logger(const char* msg);
int read_from_file(const char* name, char* buffer);

#endif /* BBBW_LOGGER_H_*/
