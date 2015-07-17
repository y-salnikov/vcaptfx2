#ifdef WIN32
	#include <windows.h>
	#define SLEEP(t) Sleep(t)
	#define COMPAT_MINGW_MS_VSNPRINTF 1
#elif UNIX
	#include <unistd.h>
	#define SLEEP(t) usleep(t*1000)
#endif


