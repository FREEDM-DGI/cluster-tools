#pragma once

/*
 * DESCRIPTION:
 *
 * Provides mechanisms to retrieve the current time instant.  This can be
 * done with various time sources, including:
 *
 * 	- CPU Time Stamp Counter
 * 	- system calls
 * 	- ntp servers 
 *
 * Also, contains some useful functions to deal with various time
 * representations (structures).  These include conversions, adding, etc.
 */

/* TODO: where does this time.h come from? */
#include "time.h"
#include "data_types.h"

class time_unit {
	public:
		//static u64 _cpu_ghz;
		static double _cpu_hz;

		u64 init_hz(int seconds);
		bool init_hz_from_file();


		bool _use_cycles;  // use cycles to represent the base time recorded
		int use_cycles(bool choice);

		u64 _cycles;  // cycles that represents the time recorded
		// if cycles are not used, then timespec will be used
		struct timespec _timespec;

		time_unit(void);
		explicit time_unit(bool use_cycles);

		u64 get_nanosecs(void);	
		double get_millisecs(void);
		double get_seconds(void);
		int set_nanosecs(u64 nsecs);
		int set_millisecs(u64 msecs);
		int set_seconds(u64 secs);
		int set_time_unit(time_unit &tu);
		void set_now(void);  // set the recorded time as the current time
		time_unit subtract(const time_unit &rhs);
		time_unit add(time_unit &rhs);
		void add_ns(u64 nsecs);
		void add_sec(u64 secs);
		void sub_sec(u64 secs);
		int set_timespec(const struct timespec &ts);
		struct timespec get_timespec(void);
		struct timeval get_timeval(void);
		bool is_zero_time();

		static u64 nsec2cycles(u64 nsecs);
		static u64 sec2cycles(u64 secs);
		static u64 read_tsc(void);

		static struct timespec read_ntptime(void);

		friend bool operator>(time_unit &t1, time_unit &t2);
		friend bool operator>=(time_unit &t1, time_unit &t2);
		friend bool operator<(time_unit &t1, time_unit &t2);

		friend time_unit operator-(time_unit &t1, time_unit &t2);
		friend time_unit operator+(time_unit &t1, time_unit &t2);

	private:
		
};
