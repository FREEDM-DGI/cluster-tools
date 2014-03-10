#include <stdlib.h>
#include <sys/time.h>
#include <getopt.h>
#include <stdio.h>

#include <iostream>
using namespace std;

#include "time_unit.h"

bool do_set_time(int argc, char *argv[])
{
	int set_flag = 0;

	while (1)
	{
		static struct option long_options[] =
		{
			{"set", no_argument, &set_flag, 1},
			{0, 0, 0, 0}
		};

		/* getopt_long stores the option index here. */
		int option_index = 0;

		int c = getopt_long (argc, argv, "", long_options, &option_index);
     
		/* Detect the end of the options. */
		if (c == -1)
			break;
     
		switch (c)
		{
			case 0:
				/* If this option set a flag, do nothing else now. */
				if (long_options[option_index].flag != 0)
					break;

				printf("option %s", long_options[option_index].name);
				if (optarg)
					printf (" with arg %s", optarg);
				printf ("\n");

				break;
     
     
			default:
				cout << "unkown option" << endl;
				exit(EXIT_FAILURE);
		}
	}

	return set_flag;
}

int main(int argc, char *argv[])
{
	time_unit time(false);

	struct timespec ts = time.read_ntptime();
	time.set_timespec(ts);
	if (time.is_zero_time()) {
		cout << "unable to get ntp time" << endl;
		exit(EXIT_FAILURE);
	}

	cout << "time (UNIX) received:" << endl;
	cout << "\t" << (u64)time.get_seconds() << endl;
	cout << "\t" << (u64)time.get_nanosecs() << endl;

	cout << endl;
	
	struct timeval tv;
	struct timezone tz;
	int rtn = gettimeofday(&tv, &tz);
	if (rtn) {
		cout << "gettimeofday() failed" << endl;
		exit(EXIT_FAILURE);
	}

	cout << "time gettimeofday():" << endl;
	cout << "\t" << tv.tv_sec << endl;
	cout << "\t" << tv.tv_usec << endl;
	cout << endl;

	//int     tz_minuteswest; /* minutes W of Greenwich */
	// Eastern Time Zone (DST): 240 minutes
	//int     tz_dsttime;     /* type of dst correction */
	cout << "\t" << tz.tz_minuteswest << endl;
	cout << "\t" << tz.tz_dsttime << endl;

	if (do_set_time(argc, argv)) {
		struct timeval set_tv = time.get_timeval();
		struct timezone set_tz;
		set_tz.tz_minuteswest = 240;
		set_tz.tz_dsttime = 0;

		rtn = settimeofday(&set_tv, &set_tz);
		if (rtn) {
			cout << "settimeofday() failed" << endl;
			exit(EXIT_FAILURE);
		} else {
			cout << "settimeofday() successful" << endl;
		}
	}

	return 0;
}
