#include <time.h>
#include <sys/socket.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
using namespace std;

#include "time_unit.h"

/*
 * x86 assembly(i.e., rdtsc) will fail on arm compile
 */
#if !defined (__i386__) && !defined(__x86_64__)
#warning "INFO: Compiling for ANDROID"
#define ANDROID
#else
#warning "INFO: Compiling for X86"
#endif

#define ISVARSIGNED(V) ({ typeof (V) _V = -1; _V < 0 ? 1 : 0; })
#define OUTPUT_MSG_NUM(msg, var) do { \
	printf(msg); \
	printf(" "); \
	int S = ISVARSIGNED(var); \
	if (S) printf("%lld", (s64)var); \
	else printf("%llu", (u64)var); \
	printf ("\n"); \
} while(0)

#define OUTPUT_NUM(var) do { \
	OUTPUT_MSG_NUM(#var, var); \
} while(0)

#define NSEC_PER_SEC ((s64)1E9)
#define MSEC_PER_SEC ((s64)1E3)
#define POW_2_32 (4294967296LLU)
// offset (in seconds) between January 1st, 1900(NTP)
// and January 1st, 1970(UNIX) (2208988800).
#define UNIX_NTP_DIFF (2208988800LU)


/** 
 * Examples:
 * from_string<int>(my_int, my_string, std::dec);
 *
 * to_string(my_string);
 */
template <class T>
inline std::string to_string(const T& t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}

template <class T>
bool from_string(T& t,
		const std::string& s,
		std::ios_base& (*f)(std::ios_base&))
{
	std::istringstream iss(s);
	return !(iss >> f >> t).fail();
}

static bool
file_exists(string filename) {
	ifstream fin;

	// We must use the string class's c_str() function to convert
	// The C++ string into an old C string (character array).
	// This is because the open() function expects a character array
	fin.open (filename.c_str());
	if (fin.fail()) return false;
	fin.close();

	return true;
}

static bool
str_from_file(string file_name, string &rtn_string)
{
	ifstream file;
	bool rtn_val = false;

	file.open(file_name.c_str());
	if (file.is_open()) {
		getline(file, rtn_string);
		file.close();
		rtn_val = true;
	}

	return rtn_val;
}



// ----- (start) from linux kernel v2.6.29/arch/x86/include/asm/msr.h
/*
 * both i386 and x86_64 returns 64-bit value in edx:eax, but gcc's "A"
 * constraint has different meanings. For i386, "A" means exactly
 * edx:eax, while for x86_64 it doesn't mean rdx:rax or edx:eax. Instead,
 * it means rax *or* rdx.
 */
//  __x86_64__ is gcc/g++ specific
#if __x86_64__
#define DECLARE_ARGS(val, low, high)    unsigned low, high
#define EAX_EDX_VAL(val, low, high)     ((low) | ((u64)(high) << 32))
#define EAX_EDX_ARGS(val, low, high)    "a" (low), "d" (high)
#define EAX_EDX_RET(val, low, high)     "=a" (low), "=d" (high)
#else
#define DECLARE_ARGS(val, low, high)    unsigned long long val
#define EAX_EDX_VAL(val, low, high)     (val)
#define EAX_EDX_ARGS(val, low, high)    "A" (val)
#define EAX_EDX_RET(val, low, high)     "=A" (val)
#endif

u64 time_unit::read_tsc()
{
#ifndef ANDROID
	DECLARE_ARGS(val, low, high);

	asm volatile("rdtsc" : EAX_EDX_RET(val, low, high));

	return EAX_EDX_VAL(val, low, high);
#else
	/* TODO: need alternative for android */
	return 0;
#endif
}
// ----- (end) from linux kernel v2.6.29/arch/x86/include/asm/msr.h

/* TODO: calibrate this
 * maybe provide a calibrate function with some time to wait?
 */

// maybe use number of cycles over ntp retrieved time? 
//double time_unit::_cpu_hz = 3010643978.40235294117647058823;

double time_unit::_cpu_hz = 0;

time_unit::time_unit()
{
	// use _cycles -or- _timespec as base timekeeping
	// in general, try to use _timespec due to much larger time interval coverage
	// TODO: _cycles will hold much less than _timespec

#ifndef ANDROID
	_use_cycles = true;
#else
	_use_cycles = false;
#endif

	_timespec.tv_sec = 0;
	_timespec.tv_nsec = 0;

	_cycles = 0;

	// _cpu_hz only needed if cycles are used for timekeeping
	if (_use_cycles) {
		if (0 == _cpu_hz) {
			if (!init_hz_from_file()) {
				init_hz(4);
			}
		}
	}
}

time_unit::time_unit(bool use_cycles)
	: _use_cycles(use_cycles)
{
#ifdef ANDROID
	if (_use_cycles) {
		cout << "cannot use cycles on ANDROID" << endl;
		exit(EXIT_FAILURE);
	}
#endif

	_timespec.tv_sec = 0;
	_timespec.tv_nsec = 0;

	_cycles = 0;

	// _cpu_hz only needed if cycles are used for timekeeping
	if (_use_cycles) {
		if (0 == _cpu_hz) {
			if (!init_hz_from_file()) {
				init_hz(4);
			}
		}
	}
}

bool
time_unit::init_hz_from_file()
{
	string file_name = "/home/mark/net_sporadic/run_cpu/.cpu_hz";

	if (!file_exists(file_name)) return 0;

	string rtn_string;
	str_from_file(file_name, rtn_string);

	u64 my_int;
	if (from_string<u64>(my_int, rtn_string, std::dec)) {
		// TODO: check proper conversion without loss of precision
		_cpu_hz = (double)my_int;
		printf("_cpu_hz(from file): %10.2f\n", _cpu_hz);
		return _cpu_hz;
	}

	return 0;
}

// TODO: should be able to call this without object
u64
time_unit::init_hz(int seconds)
{
	u64 cyc_start, cyc_stop;
	struct timespec ts_start, ts_stop;

	printf("initializing _cpu_hz for %d seconds\n", seconds);
	cyc_start = read_tsc();
	ts_start = time_unit::read_ntptime();

	usleep(seconds);

	cyc_stop = read_tsc();
	ts_stop = time_unit::read_ntptime();

	u64 elapsed_cycles = cyc_stop - cyc_start;
	// only really need secs for accuracy
	u64 elapsed_sec = ts_stop.tv_sec - ts_start.tv_sec;

	_cpu_hz = (double)elapsed_cycles/(double)elapsed_sec;

	printf("_cpu_hz: %10.2f\n", _cpu_hz);

	// TODO: check proper conversion without loss of precision
	return (u64)_cpu_hz;
}

/**
 * REF:
 * http://www.abnormal.com/~thogard/ntp/
 * http://souptonuts.sourceforge.net/code/queryTimeServer.c.html
 * http://www.ntp.org/
 * http://www.scss.com.au/family/andrew/gps/ntp/ (GPS ntp server)
 *
 * NOTES:
 * To display time associated with the number of seconds use:
 * date -u -d @<seconds>
 */
struct timespec
time_unit::read_ntptime()
{
	bool verbose = false;
	struct timespec rtn;
	rtn.tv_sec = 0;
	rtn.tv_nsec = 0;

	// network time server ip address
	// 71.6.202.221
	// 64.73.32.134
	//string ip = "216.184.20.82";
	//string ip = "64.73.32.134";
	string ip = "192.168.200.10";

	// NTP uses port 123
	const uint16_t port = 123;
	int sockfd;
	struct sockaddr_in dst;

	const int maxlen = 512;
	// NTP Data Format
	// ---------------------------------------------
	// LI - alarm condition (clock not synchronized) 
	// version - 4
	// mode - client
	unsigned char msg[48]={0xE3,0,0,0,0,0,0,0,0};
	u32 buf[maxlen]; // returned buffer (each array element should be 32-bits)

	// setup the destination structure
	memset((char*)&dst, 0, sizeof(dst));

	// convert the text address to binary form
	if(!inet_pton(AF_INET, ip.c_str(), &dst.sin_addr)) {
		fprintf(stderr, "error1\n");
		return rtn;
	}
	// AF_INET - IPv4 Internet protocols
	dst.sin_family = AF_INET;
	// HACK: uint16_t cast should not be needed broken for certain libc versions
	dst.sin_port = (uint16_t)htons(port);

	// setup the endpoint
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		fprintf(stderr, "error3\n");
		return rtn;
	}

	// send the request packet
	if (sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&dst, sizeof(dst)) != sizeof(msg)) {
		fprintf(stderr, "error4\n");
		return rtn;
	}

	// get the response
	if (verbose) {
		printf("waiting for packet\n");
	}
	ssize_t bytes_recvd = recv(sockfd, buf, sizeof(buf), 0);
	if (-1 == bytes_recvd) {
		fprintf(stderr, "error5\n");
		return rtn;
	}
	// close the connection
	close(sockfd);

	if (verbose) {
		printf("bytes received: %zd\n", bytes_recvd);
	}
/*
	for(int i=0; i<12; i++) {
		printf("%d\t%-8u\n",i,ntohl(buf[i]));
	}
*/

	// long in ntohl is 32 bits
	// 10 - transmit timestamp (sec)
	// 11 - transmit timestamp (fractional seconds)
	rtn.tv_sec = ntohl(buf[10]) - UNIX_NTP_DIFF;

	// ntp returns 32-bits indicating the fraction of a second
	// since its 32-bits, divide the fractional part by 2^32 to
	// get the fraction
	double frac_sec = ntohl(buf[11]);
	frac_sec = frac_sec / POW_2_32;
	// TODO: check for overflow
	rtn.tv_nsec = (long)(frac_sec * (u64)1E9);

	if (verbose) {
		OUTPUT_NUM(rtn.tv_sec);
		OUTPUT_NUM(rtn.tv_nsec);
	}

	return rtn;
}

int
time_unit::use_cycles(bool choice)
{
	if (choice == _use_cycles) {
		return 0;
	}

	time_unit tmp;
	tmp._use_cycles = choice;		
	tmp.set_nanosecs(this->get_nanosecs());
	*this = tmp;

	return 0;
}

u64
time_unit::get_nanosecs()
{
	if (_use_cycles) {
		// TODO: check for overflow
		return (u64)((double)_cycles / _cpu_hz * (double)1E9);
	} else {
		return (_timespec.tv_sec * NSEC_PER_SEC) + _timespec.tv_nsec;
	}
}

// TODO: doubles are imprecise
// return u64?
double
time_unit::get_millisecs()
{
	return (double)this->get_nanosecs() / (double)1E6;
}

// TODO: doubles are imprecise
// return u64?
double
time_unit::get_seconds()
{
	return (double)this->get_nanosecs() / (double)1E9;
}

static void
set_normalized_timespec(struct timespec *ts, s64 sec, s64 nsec)
{
// NOTE: divides work faster for larger numbers, smaller numbers will
// work faster with while loop
# if 1
	if (nsec < 0) {
		s64 borrow_sec = nsec / (s64)-1E9;
		// we are less than 0, so we have to borrow at least 1 sec
		borrow_sec += 1;

		sec -= borrow_sec;
		nsec += (borrow_sec * (s64)1E9);
	}

	if (sec < 0) {
		// TODO: not sure what to do about negative time?
		cout << "sec < 0" << endl;
		exit(EXIT_FAILURE);
	}

	u64 new_sec = nsec / (u64)1E9;
	u64 new_nsec = nsec - (new_sec * NSEC_PER_SEC);

	// TODO: check for overflow
	ts->tv_sec = (long)(new_sec + sec);
	ts->tv_nsec = (time_t)new_nsec;
#else

/**
 * From linux kernel (include/linux/time.h)
 */
   while (nsec >= NSEC_PER_SEC) {
      nsec -= NSEC_PER_SEC;
      ++sec;
   }
   while (nsec < 0) {
      nsec += NSEC_PER_SEC;
      --sec;
   }
   // TODO: check for overflow
   ts->tv_sec = (long)sec;
   ts->tv_nsec = (time_t)nsec;

#endif
}

// not used yet, maybe needed sometime
/*
static void
normalize_timespec(struct timespec *ts)
{
	set_normalized_timespec(ts, ts->tv_sec, ts->tv_nsec);
}
*/

int
time_unit::set_nanosecs(u64 nsecs)
{
	if (_use_cycles) {
/*
		// _cycles can potentially be HUGE!, so check for overflow
		if (0 != nsecs) {
			if ( _cpu_hz > ULLONG_MAX / nsecs ) { // a * b would overflow
				printf("Can't handle overflow, exiting");
				exit(1);
			}
		}
*/
		// TODO: check for overflow
		_cycles = (u64)((double)nsecs / (double)1E9 * _cpu_hz);
	} else {
		// TODO: check for overflow of nsecs (u64 -> s64)
		set_normalized_timespec(&_timespec, 0, nsecs);
	}

	return 0;
}

int
time_unit::set_millisecs(u64 msecs)
{
	return set_nanosecs(msecs*(u64)1E6);
}

int
time_unit::set_seconds(u64 secs)
{
	return set_nanosecs(secs*(u64)1E9);
}

int
time_unit::set_time_unit(time_unit &tu)
{
	// TODO: check if both this and tu both _use_cycles
	if (_use_cycles) {
		_cycles = tu._cycles;
	} else {
		_timespec = tu._timespec;
	}

	return 0;
}

void
time_unit::set_now()
{	
	if (_use_cycles) {
		_cycles = read_tsc();
	} else {
		// TODO: check return value?
		clock_gettime(CLOCK_REALTIME, &_timespec);
	}
}

/**
 * @rhs: time_unit to subtract
 * @return: this - rhs
 *
 * DESCRIPTION:
 * Take a time unit and subtract it from this one.
 *
 * TODO: if rhs is > this, we will wrap
 */
time_unit
time_unit::subtract(const time_unit &rhs)
{	
	time_unit lhs = *this;
	time_unit rtn_val = *this;  // make sure we have setings (e.g., _use_cycles) correct

	if (_use_cycles) {
		// TODO: make sure both are using _use_cycles
		rtn_val._cycles = lhs._cycles - rhs._cycles;
	} else {
		set_normalized_timespec(&rtn_val._timespec,
				lhs._timespec.tv_sec - rhs._timespec.tv_sec,
				lhs._timespec.tv_nsec - rhs._timespec.tv_nsec
		);
	}

	return rtn_val;
}

/**
 * timespec_add_ns - Adds nanoseconds to a timespec
 * @a:      pointer to timespec to be incremented
 * @ns:     unsigned nanoseconds value to be added
 *
 * from Linux kernel include/linux/time.h
 */
static inline void
timespec_add_ns(struct timespec *a, u64 ns)
{
	u64 secs = ns / (u64)1E9;
	u64 nsecs = ns - (secs * (u64)1E9);

	secs += a->tv_sec;
	nsecs += a->tv_nsec;

	set_normalized_timespec(a, secs, nsecs);
}

static inline void
timespec_add_sec(struct timespec *a, time_t sec)
{
	a->tv_sec += sec;
}

/**
 *
 * returns a time_unit with the added value
 */
time_unit
time_unit::add(time_unit &rhs)
{
	time_unit rtn_val = *this;
	
	if (_use_cycles) {
		rtn_val._cycles = this->_cycles + rhs._cycles;
		if (rtn_val._cycles < rhs._cycles) {
			printf("Can't handle overflow, exiting");
			exit(1);
		}
	} else {
		timespec_add_ns(&rtn_val._timespec, rhs.get_nanosecs());
	}

	return rtn_val;
}

u64
time_unit::nsec2cycles(u64 nsecs)
{
	u64 rtn_val;
/*
	// _cycles can potentially be HUGE!, so check for overflow
	if (0 != nsecs) {
		if ( _cpu_hz > ULLONG_MAX / nsecs ) { // a * b would overflow
			printf("Can't handle overflow, exiting");
			exit(1);
		}
	}
*/

	// TODO: check for overflow
	rtn_val = (u64)((double)nsecs / (double)1E9 * _cpu_hz);

	return rtn_val;
}

u64
time_unit::sec2cycles(u64 secs)
{
	u64 rtn_val;

	// TODO: use get_seconds()?
	rtn_val = nsec2cycles(secs * (u64)1E9);

	return rtn_val;
}

int
time_unit::set_timespec(const struct timespec &ts)
{
	this->set_nanosecs(ts.tv_nsec);
	this->add_sec(ts.tv_sec);

	return 0;
}

struct timespec
time_unit::get_timespec()
{
	struct timespec rtn_val;
	
	u64 nsecs = this->get_nanosecs();
	set_normalized_timespec(&rtn_val, 0, nsecs);

	return rtn_val;
}

struct timeval
time_unit::get_timeval()
{
	struct timespec ts;
	
	u64 nsecs = this->get_nanosecs();
	set_normalized_timespec(&ts, 0, nsecs);

	struct timeval rtn_val;
	rtn_val.tv_sec = ts.tv_sec;
	rtn_val.tv_usec = ts.tv_nsec / 1000;

	return rtn_val;
}

void
time_unit::add_ns(u64 nsecs)
{
	if (_use_cycles) {
		_cycles += nsec2cycles(nsecs);
	} else {
		timespec_add_ns(&_timespec, nsecs);
	}
}

void
time_unit::add_sec(u64 secs)
{
	this->add_ns(secs * (u64)1E9);
}

void
time_unit::sub_sec(u64 secs)
{
	// TODO: optimize by not creating a temp instance
	time_unit tu_secs;
	time_unit rtn;

	tu_secs.set_seconds(secs);
	
	rtn = this->subtract(tu_secs);

	this->set_time_unit(rtn);
}


bool
time_unit::is_zero_time()
{
	if (_use_cycles) {
		if (_cycles == 0) {
			return true;
		}
	} else {
		if (_timespec.tv_sec == 0 && _timespec.tv_nsec == 0) {
			return true;
		}
	}
	
	return false;
}

bool operator> (time_unit &t1, time_unit &t2)
{
	if (t1._use_cycles && t2._use_cycles) {
		return t1._cycles > t2._cycles;
	} else {
		return t1.get_nanosecs() > t2.get_nanosecs();
	}
}

bool operator>=(time_unit &t1, time_unit &t2)
{
	// TODO: check if t1 and t2 have differing _use_cycles?
	if (t1._use_cycles && t2._use_cycles) {
		return t1._cycles >= t2._cycles;
	} else {
		return t1.get_nanosecs() >= t2.get_nanosecs();
	}
}

bool operator< (time_unit &t1, time_unit &t2)
{
	if (t1._use_cycles && t2._use_cycles) {
		return t1._cycles < t2._cycles;
	} else {
		return t1.get_nanosecs() < t2.get_nanosecs();
	}
}

time_unit operator- (time_unit &t1, time_unit &t2)
{
	return t1.subtract(t2);
}

time_unit operator+ (time_unit &t1, time_unit &t2)
{
	return t1.add(t2);
}
