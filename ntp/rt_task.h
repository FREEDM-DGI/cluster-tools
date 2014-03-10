#pragma once

#include "data_types.h"

// REF: include/linux/sched.h
#define SCHED_SPORADIC		6

struct sched_param_sporadic {
	int sched_priority;
	int sched_ss_low_priority;
	struct timespec sched_ss_repl_period;
	struct timespec sched_ss_init_budget;
	int sched_ss_max_repl;
};

class rt_task {
	public:
		rt_task();

		static int setscheduler(int pid, int policy,
			struct sched_param_sporadic *param);
		static int lock_pages(void);

		int parse_cmd_args(int argc, char *argv[]);
		void display_usage(int exit_code);

		int apply_param();
		void output_parameters();

		int _priority;
		int _low_priority;
		u64 _budget;
		u64 _period;
		int _pid;
		int _policy;
		int _max_repl;
		static const struct option long_opts[];
		static const char *opt_string;

	private:
};
