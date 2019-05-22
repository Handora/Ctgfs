#ifndef gettime_h
#define gettime_h

#ifdef __APPLE__
namespace ctgfs {
namespace rpc {

typedef enum {
	CLOCK_REALTIME,
	CLOCK_MONOTONIC,
	CLOCK_PROCESS_CPUTIME_ID,
	CLOCK_THREAD_CPUTIME_ID
} clockid_t;

int clock_gettime(clockid_t clk_id, struct timespec *tp);

}}// namespace rpc, ctgfs
#endif

#endif
