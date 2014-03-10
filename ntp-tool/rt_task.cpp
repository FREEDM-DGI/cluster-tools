#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <getopt.h>

#include <iostream>
using namespace std;

#include "time_unit.h"
#include "rt_task.h"

const struct option rt_task::long_opts[] = {
	{"priority", required_argument, NULL, 0},
	{"low_priority", required_argument, NULL, 0},
	{"budget", required_argument, NULL, 0},
	{"period", required_argument, NULL, 0},
//	{"pid", required_argument, NULL, 0},
	{"policy", required_argument, NULL, 0},
	{"max_repl", required_argument, NULL, 0},
	// for other parts of the program
	{"run_time", required_argument, NULL, 0},
	{"wait_for_signal", no_argument, NULL, 0},
	{"chunk_exec_ns", required_argument, NULL, 0},
	{"leave_sem_open", no_argument, NULL, 0},
	{NULL, no_argument, NULL, 0}
};

const char *rt_task::opt_string = "h?";

rt_task::rt_task()
{
	_priority=95;
	_low_priority=5;
	_budget=1000000;
	_period=10000000;
	_pid=-1;
	_policy=SCHED_FIFO;
	_max_repl=10;
}

int
rt_task::setscheduler(int pid, int policy, struct sched_param_sporadic *param) {
	int rtn_val = 0;

	rtn_val = syscall(SYS_sched_setscheduler, pid, policy, param);

	if (rtn_val) {
		perror ("sched_setscheduler");
		printf("You probably should be running as root\n");
		exit(-1);
	}

	return rtn_val;
}

int
rt_task::lock_pages (void)
{
    int res;

    res = mlockall (MCL_CURRENT | MCL_FUTURE);
    if (res != 0) {
        perror ("mlockall");
        printf ("unable to lock pages into memory\n");
    }

	return res;
}

void
rt_task::output_parameters()
{
	time_unit tu_budget, tu_period;

	tu_budget.set_nanosecs(_budget);
	tu_period.set_nanosecs(_period);

	cout << "pid: " << _pid << endl;
	cout << "priority: " << _priority << endl;
	cout << "policy: " << _policy << endl;
	if (_policy == SCHED_SPORADIC) {
		cout << "budget(ms): " << tu_budget.get_millisecs() << endl;
		cout << "period(ms): " << tu_period.get_millisecs() << endl;
		cout << "low_priority: " << _low_priority << endl;
		cout << "max repl: " << _max_repl << endl;
	}
}

int
rt_task::apply_param()
{
	int rtn_val;
	time_unit tu_budget, tu_period;
	struct sched_param_sporadic param;

	param.sched_priority = _priority;
	param.sched_ss_low_priority = _low_priority;

	tu_budget.set_nanosecs(_budget);
	param.sched_ss_init_budget = tu_budget.get_timespec();

	tu_period.set_nanosecs(_period);
	param.sched_ss_repl_period = tu_period.get_timespec();
	
	param.sched_ss_max_repl = _max_repl;

	rtn_val = setscheduler(_pid, _policy, &param);

	return rtn_val;
}

void
rt_task::display_usage(int exit_code)
{
	cout << endl;
	cout << "USAGE: " << endl;

	int i=0;
	while (long_opts[i].name != NULL) {
		cout << "\t--" << long_opts[i].name << endl;
		++i;
	}
	exit(exit_code);
}

int
rt_task::parse_cmd_args(int argc, char *argv[])
{
	int rtn_val=0;
	int opt;
	int long_index;

	opt = getopt_long(argc, argv, opt_string, long_opts, &long_index);
	while(opt != -1) {
		switch(opt) {
			case 'h':   /* fall-through is intentional */
			case '?':
				display_usage(0);
				break;

			case 0:     /* long option without a short arg */
				if(strcmp("priority", long_opts[long_index].name) == 0) {
					_priority = atoi(optarg);
				}
				else if(strcmp("low_priority", long_opts[long_index].name) == 0) {
					_low_priority = atoi(optarg);
				}
				else if(strcmp("budget", long_opts[long_index].name) == 0) {
					_budget = atoll(optarg);
				}
				else if(strcmp("period", long_opts[long_index].name) == 0) {
					_period = atoi(optarg);
				}
				else if(strcmp("pid", long_opts[long_index].name) == 0) {
					_pid = atoi(optarg);
				}
				else if(strcmp("max_repl", long_opts[long_index].name) == 0) {
					_max_repl = atoi(optarg);
				}
				else if(strcmp("policy", long_opts[long_index].name) == 0) {
					if (strcmp("SCHED_SPORADIC", optarg) == 0) {
						_policy = SCHED_SPORADIC;
					} else if (strcmp("SCHED_FIFO", optarg) == 0) {
						_policy = SCHED_FIFO;
					} else {
						cout << "Unknown scheduling policy." << endl;
						display_usage(-1);
					}
				}
				else {
					// display_usage(-1);
				}
				break;

			default:
				/* You won't actually get here. */
				break;
		}

		opt = getopt_long(argc, argv, opt_string, long_opts, &long_index);
	}
	if (_pid == -1) {
		cout << "No pid specified." << endl;
		display_usage(-1);
	}

	return rtn_val;
}
