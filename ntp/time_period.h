#ifndef TIME_PERIOD_H
#define TIME_PERIOD_H

#include "data_types.h"
#include "time_unit.h"

class time_period {
	public :
		time_period();
		
		void start();
		void stop();

		int use_cycles(bool choice);

		u64 get_diff_nsec();
		u64 get_diff_cycles();

		time_unit get_diff_tu();

		time_unit _start_time, _stop_time;
	private :
};

#endif // TIME_PERIOD_H

