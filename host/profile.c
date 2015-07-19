#include <sys/time.h>
#include <stdio.h>

struct timeval t0;

void profile_start(void)
{
	if( gettimeofday(&t0,NULL))
	{
		fprintf(stderr,"gettimeofdate failed\n");
	}
}

void profile_check(char *mark)
{
	struct timeval t1;
	if( gettimeofday(&t1,NULL))
	{
		fprintf(stderr,"gettimeofdate failed\n");
	}
	long uelapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec;
	double elapsed = uelapsed/1000000.0;
	fprintf(stderr,"%s : %f\n",mark,elapsed);
}
