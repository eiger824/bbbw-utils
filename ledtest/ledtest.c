#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>

#include "logger.h"
#include "strutils.h"

// File to use in sysfs
const char* ain1_path = "/sys/bus/iio/devices/iio:device0/in_voltage1_raw";
const char* export_path = "/sys/class/gpio/export";
const char* direction_path = "/sys/class/gpio/gpio49/direction";
const char* value_path = "/sys/class/gpio/gpio49/value";
const char* unexport_path = "/sys/class/gpio/unexport";

bool listen = false; // if the program will be interactive or not

void sig_handler(int signo)
{
	if (signo == SIGINT)
	{
		if (!listen)
		{
			logger ("\nReceived SIGINT, switching off led and unexporting");
			int res;

			res = write_2_file(value_path,"0");
			(res == -1) ? perror("Error writing value") : 
				logger ("Success! Led is OFF");

			res = write_2_file(unexport_path, "49");
			(res == -1) ? perror("Error unexporting led") :
				logger ("Success! Led 49 is unregistered");
		}
		else
		{
			printf("\n");
		}
		//exit program
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
	printf("-t <m>\tToggle led state every m milliseconds\n");
	printf("-s\tSilent mode (e.g. if running as daemon, logging to file\n");
}

int main (int argc, char *argv[])
{
	//Privileges needed to access GPIO in sysfs
	if (getuid() != 0)
	{
		logger ("Be sudo, my friend\n");
		return 1;
	}

	char *c = malloc(1);
	char *cvalue = NULL;
	bool toggle = false;
	int time = 1;	
	int opt;	

	if (signal(SIGINT, sig_handler) == SIG_ERR)
	{
		printf("Warning, SIGINT won't be catched\n");
	}

	if (argc == 1)
	{
		help();
		return -1;
	}

	while ((opt = getopt(argc, argv, "01hlst:")) != -1)
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
				printf("Will listen to AIN1 Pin (P9_40)\n");                 
				break;
			case 's':
				set_silent(true);
				break;
			case 't':
				memset(c,'1',1); //Start with ON state
				toggle=true;
				time = atoi(optarg);
				logger (msg_app_flt("Will toggle every %f seconds", (float) time / 1000));
				break;
			case '?':
				if (optopt == 'c')
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
				return 1;
			default:
				logger ("Unknown option");
				break;
		}
	}	

	switch (argv[1][0])
	{
		case '1':
			memset(c,'1',1);	
			break;
		case '0':
			memset(c,'0',1);
			break;
		case 't':
			//start with led ON
			memset(c,'1',1);
			toggle=true;
			if (argc == 3) time = atoi(argv[2]);
			printf("Will toggle every %f seconds\n", (float) time / 1000);
			break;
		case 'l':
			listen = true;
			printf("Will listen to AIN1 Pin (P9_40)\n");
			break;
		default:
			printf("Wrong param \"%c\"\n",argv[1][0]);
			help();
			return -1;
	}

	int res;

	if (!listen)
	{
		//setup functions
		res = write_2_file(export_path, "49");
		(res == -1) ? perror("Error exporting") :
			printf("Exported our led: %s\n", export_path);
		res = write_2_file(direction_path, "out");
		(res == -1) ? perror("Error setting direction") :
			printf("Set direction: %s\n", direction_path);

		if (toggle)
		{
			while(1)
			{
				if (!strcmp(c,"1")) memset(c,'0',1);
				else memset(c,'1',1);
				write_2_file(value_path, c);
				(res == -1) ? perror("Error writing value") :
					printf("Success! (%s)\n", (!strcmp(c,"1")) ? "ON" : "OFF");
				usleep(time * 1000);
			}
		}
		else
		{
			write_2_file(value_path, c);
			(res == -1) ? perror("Error writing value") :
				printf("Success! (%s)\n", (!strcmp(c,"1")) ? "ON" : "OFF");
		}	
	}
	else // Read temperature values
	{
		char value[10];
		float temp;
		float raw;
		bool lock = false;
		int res;

		//setup functions
		res = write_2_file(export_path, "49");
		(res == -1) ? perror("Error exporting") :
			printf("Exported our led: %s\n", export_path);
		res = write_2_file(direction_path, "out");
		(res == -1) ? perror("Error setting direction") :
			printf("Set direction: %s\n", direction_path);

		while(1)
		{
			ft = fopen(ain1_path, "r");
			if (ft == NULL)
			{
				perror("Error opening file");
				return -1;
			}

			if ( fgets(value, 10, ft) != NULL )
			{
				raw = atoi(value);
				temp = ((raw * 1800) - 500) / 10;
				printf("\rRaw: %f, processed: %f", raw, temp);
				fflush(stdout);
				//temp fix: if raw > 1780 then switch on led
				if (raw > 1780)
				{
					if (!lock)
					{
						res = write_2_file(value_path,"1");
						if (res == -1) perror("Error");
						lock=!lock;
					}
				}
				else
				{
					if (lock)
					{
						res = write_2_file(value_path,"0");
						if (res == -1) perror("Error");
						lock=!lock;
					}
				}
				fclose(ft);
			}
			usleep(1000000);
		}
	}
	return 0;
}
