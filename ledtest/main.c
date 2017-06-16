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

bool listen = false; // if the program will be interactive or not
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
	
	printf("-1\tToggle led ON and exit\n");
	printf("-0\tToggle led OFF and exit\n");
	printf("-h\tShow this help and exit\n");
	printf("-l\t\"Listen\" mode, non-interactive\n");
	printf("-r\tRemove log file and exit\n");
	printf("-t <m>\tToggle led state every m milliseconds\n");
	printf("-s\tSilent mode (e.g. if running as daemon, logging to file)\n");
}

int main (int argc, char *argv[])
{
	char *c = malloc(1);
	char *cvalue = NULL;
	bool toggle = false;
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

	while ((opt = getopt(argc, argv, "01hlrst:")) != -1)
	{
		switch(opt)
		{
			case '1':
				memset(c,'1',1);
				break;
			case '0':
				memset(c,'0',1);
				break;
			case 'h':
				help();
				return 0;
			case 'l':
				listen=true;
				logger ("Will listen to AIN1 Pin (P9_40)"); 
				break;
			case 'r':
				logger ("Will remove log file. Hasta la vista!");
				remove_log_file();
				return 0;
			case 's':
				set_silent(true);
				is_silent = true;
				break;
			case 't':
				memset(c,'1',1); //Start with ON state
				toggle=true;
				time = atoi(optarg);
				logger (msg_app_flt("Will toggle every %f seconds", (float) time / 1000));
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

	int res;

	if (!listen)
	{
		if (toggle)
		{
			while(1)
			{
				if (!strcmp(c,"1")) memset(c,'0',1);
				else memset(c,'1',1);
				res = write_2_led(c);
				if (res == -1) logger ( msg_err ("Error writing value", errno));
				usleep(time * 1000);
			}
		}
		else
		{
			res = write_2_led(c);
			(res == -1) ? logger ( msg_err ("Error writing value", errno)) :
				logger ( msg_app_str("Success! (%s)", (!strcmp(c,"1")) ? "ON" : "OFF"));
		}	
	}
	else // Read temperature values
	{
		char value[10];
		float raw;
		float temp;
		bool lock = false;
		int res;
		
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
				if (temp > 25)
				{
					if (!lock)
					{
						res = write_2_led("1");
						if (res == -1) logger (msg_err ("Error", errno));
						lock=!lock;
					}
				}
				else
				{
					if (lock)
					{
						res = write_2_led("0");
						if (res == -1) logger (msg_err ("Error", errno));
						lock=!lock;
					}
				}
				++sample_count;
				fclose(ft);
			}
			usleep(10 * 1000000);
		}
	}
	return 0;
}
