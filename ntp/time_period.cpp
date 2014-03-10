#include "time_period.h"

/*
 * x86 assembly(i.e., rdtsc) will fail on arm compile
 */
#if !defined (__i386__) && !defined(__x86_64__)
#warning "INFO: Compiling for ANDROID"
#define ANDROID
#else
#warning "INFO: Compiling for X86"
#endif

time_period::time_period()
{
#ifdef ANDROID
	_start_time._use_cycles = false;
	_stop_time._use_cycles = false;
#else
	_start_time._use_cycles = true;
	_stop_time._use_cycles = true;
#endif
}

int
time_period::use_cycles(bool choice)
{
	_start_time.use_cycles(choice);
	_stop_time.use_cycles(choice);

	return 0;
}

void
time_period::start()
{
	_start_time.set_now();
}

void
time_period::stop ()
{
	_stop_time.set_now();
}

u64
time_period::get_diff_nsec()
{
	time_unit tu;

	tu = _stop_time - _start_time;

	return tu.get_nanosecs();
}

u64
time_period::get_diff_cycles()
{	
	time_unit tu;

	tu = _stop_time - _start_time;
	
	return tu._cycles;
}


time_unit
time_period::get_diff_tu()
{
	return _stop_time - _stop_time;
}
