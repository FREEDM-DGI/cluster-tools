#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sched.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h> 
#include <sys/mman.h> // mlockall()
#include <getopt.h>
#include <string.h>
#include <time.h>

#include <fstream>
#include <iostream>
using namespace std;

#include "time_unit.h"
#include "time_period.h"
#include "rt_task.h"

#define OUTPUT_VAR(var_name) cout << #var_name << ": " <<  var_name << endl;

time_unit solo_cycle;
s64 run_time_secs = -1;
bool wait_for_signal = false;

char PROGRAM_NAME[] = "run_cpu";
volatile bool stop_program = false;

void SIG_handler(int)
{
	cout << "stop caught (" << PROGRAM_NAME << ")" << endl;
	stop_program = true;
}

void SIG_start(int)
{
	cout << "Signal start caught (" << PROGRAM_NAME << ")" << endl;
}

/**
 * DESCRIPTION:
 * This is the main loop to take up CPU time.
 *
 * @run_time
 * 		(out) - the total wall clock running time of the code
 * 		(in)  - the maximum amount of time to run
 * 
 * @exec_time
 * 		(out) - the total CPU time this code was able to get
 *
 * @max_preempt
 * 		(out) - maximum amount of time code was preempted
 *
 * NOTE:
 * This function may be interrupted by a signal and may not run
 * all of run_time specified.
 *
 */
void
trial_loop(time_unit& run_time, time_unit& exec_time, time_unit& max_preempt)
{
	time_unit before;
	time_unit curr;
	time_unit diff;
	time_unit max_no_preempt;
	time_unit total;
	time_unit min;
	time_unit begin,end;

	bool first_iteration = true;
	max_preempt.set_nanosecs(0);

	// TODO: solo_cycle should not be greater than max_no_preempt
	// TODO: just an estimate, do automatically
	//       maybe some relation to solo_cycle?
	max_no_preempt.set_nanosecs(250);

	time_unit stop;
	stop.set_now();
	stop = stop + run_time;

	begin.set_now();

	// initialization
	curr.set_now();
	curr.sub_sec(1);
	// start with min being one sec, should be much less than 1 sec.
	min.add_sec(1); 
	for(;;) {
		if (stop_program) break;

		before = curr;
		curr.set_now();

		diff = curr - before;

		// update minimum to get solo_cycle
		if (diff < min) {
			min = diff;
		}

		if (diff > max_no_preempt) {
			// We were preempted, only count one solo_cycle worth
			// of execution.  We have no way to know exactly how
			// much time we actually consumed so just use the
			// minimum amount we could have possibly consumed
			total = total + solo_cycle;
			if (diff > max_preempt && !first_iteration) {
				max_preempt = diff;
			}
		} else {
			// We were NOT preempted.  We know exactly how much
			// time we consumed.  Use this amount of time.
			total = total + diff;
		}

		if (curr > stop) {
			break;
		}
		first_iteration = false;
	}
	end.set_now();
	solo_cycle = min;

	run_time = end - begin;
	exec_time = total;
}

void
trial_loop2(time_unit& run_time, time_unit& exec_time, time_unit& max_preempt, rt_task &tsk)
{
	time_unit before;
	time_unit curr;
	time_unit diff;
	time_unit max_no_preempt;
	time_unit total;
	time_unit min;
	time_unit begin,end;

	bool first_iteration = true;
	max_preempt.set_nanosecs(0);

	// TODO: solo_cycle should not be greater than max_no_preempt
	// TODO: just an estimate, do automatically
	//       maybe some relation to solo_cycle?
	max_no_preempt.set_nanosecs(250);

	time_unit stop;
	stop.set_now();
	stop = stop + run_time;

	begin.set_now();

	// initialization
	curr.set_now();
	tsk.apply_param();
	curr.sub_sec(1);
	// start with min being one sec, should be much less than 1 sec.
	min.add_sec(1); 
	for(;;) {
		if (stop_program) break;

		before = curr;
		curr.set_now();

		diff = curr - before;

		// update minimum to get solo_cycle
		if (diff < min) {
			min = diff;
		}

		if (diff > max_no_preempt) {
			// We were preempted, only count one solo_cycle worth
			// of execution.  We have no way to know exactly how
			// much time we actually consumed so just use the
			// minimum amount we could have possibly consumed
			total = total + solo_cycle;
			if (diff > max_preempt && !first_iteration) {
				max_preempt = diff;
			}
		} else {
			// We were NOT preempted.  We know exactly how much
			// time we consumed.  Use this amount of time.
			total = total + diff;
		}

		if (curr > stop) {
			break;
		}
		first_iteration = false;
	}
	end.set_now();
	solo_cycle = min;

	run_time = end - begin;
	exec_time = total;
}



void
display_usage(int exit_code)
{
	cout << endl;
	cout << "USAGE: " << endl;

	int i=0;
	while (rt_task::long_opts[i].name != NULL) {
		cout << "\t--" << rt_task::long_opts[i].name << endl;
		++i;
	}
	exit(exit_code);
}

void
parse_args(int argc, char *argv[])
{
	int opt;
	int long_index;

	optind = 1;
	opt = getopt_long(argc, argv, rt_task::opt_string, rt_task::long_opts, &long_index);
	while(opt != -1) {
		switch(opt) {
			case 'h':   /* fall-through is intentional */
			case '?':
				display_usage(0);
				break;

			case 0:     /* long option without a short arg */
				if(strcmp("run_time", rt_task::long_opts[long_index].name) == 0) {
					run_time_secs = atoll(optarg);
				}
				else if (strcmp("wait_for_signal", rt_task::long_opts[long_index].name) == 0) {
					wait_for_signal = true;
				}
//				else {
//					display_usage(-1);
//				}
				break;

			default:
				/* You won't actually get here. */
				break;
		}

		opt = getopt_long(argc, argv, rt_task::opt_string, rt_task::long_opts, &long_index);
	}

	if (run_time_secs == -1) {
		cout << "No run_time specified or incorrect run_time given." << endl;
		display_usage(-1);
	} else if (run_time_secs == 0) {
		// TODO: not the best, but set the time for a LONG TIME
		u64 one_year_in_secs = 31536000;
		
		run_time_secs = one_year_in_secs;
	}
}

void set_start_priority() {
	rt_task init_tsk;

	init_tsk._priority=99;
	init_tsk._policy=SCHED_FIFO;
	init_tsk._pid=0;
	init_tsk.apply_param();
}

u64 get_nr_ctxt_switches()
{
	bool found = false;
	ifstream proc;
	u64 total;
	char param[250];

	proc.open("/proc/stat", ios::in);
	
	string line;
	while (!proc.eof()) {
		getline(proc, line);
		// TODO: only grab if line.length() < param length
		// TODO: check return value
		sscanf(line.c_str(), "%s %llu", param, &total);

		if (!strcmp(param, "ctxt")) {
			found = true;
			break;
		}
	}

	proc.close();

	if (!found) {
		total = 0;
	}

	return total;
}

// drand48 - [0.0, 1.0)
//
// multiply by range and add offset
//
// [a,b]
// drand48() * (b - a + 1) + a
int _rand(int a, int b)
{
	double r = drand48();

	return (int)((r * (b - a + 1)) + a);
}

/**
 * Sleep for some random amount of time.
 *
 * @return
 * 	Difference between actual wakeup and expected wakeup.
 *
 * NOTE:
 * Probably best to be absolute time.
 *
 * TODO:
 * Measure record wakeup jitter
 *
 */
time_unit sleep_random()
{
	time_unit time_to_sleep(false);

	time_period time_slept;

	uint64_t nanosecs = _rand(1000, 10000) * 1000;

	clockid_t clock_id = CLOCK_REALTIME;

	time_to_sleep.set_now();
	time_to_sleep.add_ns(nanosecs);

	time_slept.start();
	clock_nanosleep(clock_id, TIMER_ABSTIME, &time_to_sleep._timespec, NULL);
	time_slept.stop();

	time_unit rtn;
	rtn.set_nanosecs(time_slept.get_diff_nsec() - nanosecs);
	
	return rtn;
}

int main(int argc, char *argv[])
{
	u64 start_ctxt_switches;
	u64 end_ctxt_switches;
	time_unit tu;
	rt_task tsk;

	set_start_priority();
	rt_task::lock_pages();

	tsk._pid=0;
	tsk.parse_cmd_args(argc, argv);
	parse_args(argc, argv);

	struct sigaction sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	// register the handler
	sa.sa_handler = SIG_handler;
	sigaction(SIGTERM, &sa, 0);
	sigaction(SIGINT, &sa, 0); // CTRL-C

	sa.sa_handler = SIG_start;
	sigaction(SIGUSR1, &sa, 0);

	srand48(time (NULL));

	time_unit init_run_time;
	init_run_time.add_sec(1);

	time_unit run_time;
	run_time.add_sec(run_time_secs);

	time_unit exec_time;
	time_unit max_preempt;
	trial_loop(init_run_time, exec_time, max_preempt); // first time to initialize solo_cycle

	cout << "solo_cycle(ns): " << solo_cycle.get_nanosecs() << endl;
	cout << "solo_cycle(cycles): " << solo_cycle._cycles << endl;

	tsk.output_parameters();
	if (wait_for_signal) {
		// TODO/BUG: signal may be sent, but we may not have a high enough priority
		// to start running immediately.  If this happens we will not get the
		// correct start time.  We will get a start time when we actually get
		// to run.  Ideally, we want the time at which we get signalled.  Maybe
		// use message passing or something.  Also, we set ourselves at a high
		// priority and lower to native priority once we get our start time
		// stamp.
		cout << "waiting to start!" << endl;
		pause();
	}

	bool test_utilization = true;
	bool test_exec_window = true;
	if (test_utilization) {

		start_ctxt_switches = get_nr_ctxt_switches();

		trial_loop2(run_time, exec_time, max_preempt, tsk);

		end_ctxt_switches = get_nr_ctxt_switches();

		// TODO: may want to lower priority to prevent interference for other tasks
		// that are running since our measurement has completed
		if (stop_program) {
			cout << "program stopped early due to signal" << endl;
		}

		cout << "context switches: " << end_ctxt_switches - start_ctxt_switches << endl;

		cout << "maximum time preempted(ns): " << max_preempt.get_nanosecs() << endl;

		cout << "thread 0 recorded CPU percent: ("
			<< (double)exec_time.get_nanosecs()/(double)run_time.get_nanosecs()
			<< " %)"
			<< endl;

	}

	if (test_exec_window) {
		sync(); sync();

		// TODO: should be local
		// max_preempt
		// exec_time
		// run_time

		time_unit max_wakeup_jitter;
		// vary sliding window time
		// start with max_preempt and continue for some large enough where average utilization does not change
		time_unit slide_window_size = max_preempt;

		const uint64_t window_incr = (uint64_t)1E8;
		const int num_steps = 3;
		const int num_trials = 10;

		time_unit min_exec_time[num_steps];
		for (int j=0; j<num_steps; ++j) {
			cout << "j: " << j << endl;

			slide_window_size.add_ns(window_incr);

			min_exec_time[j] = slide_window_size;
			time_unit wakeup_jitter;

			for (int i=0; i<num_trials; ++i) {
				wakeup_jitter = sleep_random();
				if (wakeup_jitter >=  slide_window_size) {
					min_exec_time[j].set_nanosecs(0);

					if (wakeup_jitter > max_wakeup_jitter)
						max_wakeup_jitter = wakeup_jitter;

					break;
				}
				run_time = slide_window_size - wakeup_jitter;
				trial_loop2(run_time, exec_time, max_preempt, tsk);

				if (exec_time < min_exec_time[j])
					min_exec_time[j] = exec_time;

				if (wakeup_jitter > max_wakeup_jitter)
					max_wakeup_jitter = wakeup_jitter;
			}
		}

		cout << "variable sliding window sizes: " << endl;
		slide_window_size = max_preempt;
		for (int i=0; i<num_steps; ++i) {
			slide_window_size.add_ns(window_incr);
				cout << slide_window_size.get_nanosecs()
				<< " "
				<< (double)min_exec_time[i].get_nanosecs()/(double)slide_window_size.get_nanosecs()
				<< endl;
		}
	}

	return 0;
}
