#ifdef WIN32
	#include <windows.h>
	#define SLEEP(t) Sleep(t)
//	#define SLEEP(t) usleep(t*1000)
	#define COMPAT_MINGW_MS_VSNPRINTF 1
	#define DEFAULT_CFG_PATH "\\AppData\\Local\\vcaptfx2\\"
	#define ENV_VAR "USERPROFILE"
//	#define COMPAT_USLEEP 1
#elif UNIX
	#include <unistd.h>
	#include <sys/stat.h>
	#define SLEEP(t) usleep(t*1000)
	#define DEFAULT_CFG_PATH "/.config/vcaptfx2/"
	#define ENV_VAR "HOME"
#endif

int is_file_exist(const char *filename);
char *get_config_file_path(void);
