#include <sys/time.h>

long long get_system_current_time()
{
	struct timeval time;
	gettimeofday(&time, nullptr);
	long long curTime = ((long long)(time.tv_sec))*1000+time.tv_usec/1000;
	return curTime;
}