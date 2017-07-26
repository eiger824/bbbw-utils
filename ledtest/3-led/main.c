#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

#include "logger.h"
#include "strutils.h"

// File to use in sysfs
const char* ain1_path = "/sys/bus/iio/devices/iio:device0/in_voltage1_raw";

static unsigned int sample_count = 0; // read count

void sig_handler(int signo)
{
	if (signo == SIGINT)
	{
		logger ("\nReceived SIGINT");
		/* No need to implement anything right now, but 
		 *  but if something was needed, here would come
		 *  the code.
		 */
		exit(0);
	}
}



void help()
{
	printf("USAGE:\nledtest [OPTIONS]\n");
	printf("OPTIONS:\n");

	printf("-h\tShow this help and exit\n");
	printf("-r\tRemove log file and exit\n");
	printf("-s\tSilent mode (e.g. if running as daemon, logging to file)\n");
}

void set_state(unsigned s)
{
	int ret;
	switch(s)
	{
		case 0:
			ret = write_2_led("0","1");
			if (ret == -1) logger (msg_err ("Error", errno));
			ret = write_2_led("1","0");
			if (ret == -1) logger (msg_err ("Error", errno));
			ret = write_2_led("2","0");
			if (ret == -1) logger (msg_err ("Error", errno));
			break;
		case 1:
			ret = write_2_led("0","0");
			if (ret == -1) logger (msg_err ("Error", errno));
			ret = write_2_led("1","1");
			if (ret == -1) logger (msg_err ("Error", errno));
			ret = write_2_led("2","0");
			if (ret == -1) logger (msg_err ("Error", errno));
			break;
		case 2:
			ret = write_2_led("0","0");
			if (ret == -1) logger (msg_err ("Error", errno));
			ret = write_2_led("1","0");
			if (ret == -1) logger (msg_err ("Error", errno));
			ret = write_2_led("2","1");
			if (ret == -1) logger (msg_err ("Error", errno));
			break;
		default:
			fprintf(stderr, "Wrong state\n");
			break;
	}
}

int main (int argc, char *argv[])
{
	char *c = malloc(1);
	char *cvalue = NULL;
	int time = 1;	
	int opt;	
	bool is_silent = false;

	if (signal(SIGINT, sig_handler) == SIG_ERR)
	{
		logger ("Warning, SIGINT won't be catched");
	}

	if (argc == 1)
	{
		help();
		return -1;
	}

	while ((opt = getopt(argc, argv, "hrs")) != -1)
	{
		switch(opt)
		{
			case 'h':
				help();
				return 0;
			case 'r':
				logger ("Will remove log file. Hasta la vista!");
				remove_log_file();
				return 0;
			case 's':
				set_silent(true);
				is_silent = true;
				break;
			case '?':
				if (optopt == 'c')
					logger ( msg_app_int ("Option -%c requires an argument", optopt));
				else if (isprint (optopt))
					logger ( msg_app_int ("Unknown option `-%c'", optopt));
				else
					logger ( msg_app_int ("Unknown option character `\\x%x'", optopt));
				return 1;
			default:
				logger ("Unknown option");
				break;
		}
	}	

	//Privileges needed to access GPIO in sysfs
	if (getuid() != 0)
	{
		logger ("Be sudo, my friend");
		return 1;
	}

	char value[10];
	float raw;
	float temp;

	while(1)
	{
		ft = fopen(ain1_path, "r");
		if (ft == NULL)
		{
			logger ( msg_err("Error opening file", errno));
			return -1;
		}

		if ( fgets(value, 10, ft) != NULL )
		{
			raw = atoi(value);
			temp = (((raw / 4096) * 1800) - 500)/10;
			if (is_silent)
			{
				logger (msg_app_flt3 ("Raw: %f, Temp(C): %f, Sample count: %d", raw, temp, sample_count));
			}
			else
			{
				printf("\rRaw: %f, Temp(C): %f, Sample count: %d", raw, temp, sample_count);
				fflush(stdout);
			}
			/*
			   Temp. ranges: <10: RED
			   10<=t<15: ORANGE
			   15<=t<20: GREEN
			   20<=t<25: ORANGE
			   >25: RED
			 */

			if (temp < 10)
			{
				set_state(0);
			}
			else if (10 <= temp && temp < 15)
			{
				set_state(1);
			}
			else if (15 <= temp && temp < 20)
			{
				set_state(2);
			}
			else if (20 <= temp && temp < 25)
			{
				set_state(1);
			}
			else
			{
				set_state(0);
			}

			++sample_count;
			fclose(ft);
		}
		usleep(10 * 1000000);
	}
	return 0;
}
