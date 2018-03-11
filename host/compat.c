#include "compat.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef COMPAT_USLEEP
/*
 * http://stackoverflow.com/questions/5801813/c-usleep-is-obsolete-workarounds-for-windows-mingw
 */
#include <windows.h>
#include <stdint.h>

void usleep(int64_t usec)
{
    HANDLE timer;
    LARGE_INTEGER ft;

    /* Convert to 100 nanosecond interval, negative value indicates relative time */
    ft.QuadPart = -(10 * usec);

    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}
#endif /* COMPAT_USLEEP */


#ifdef COMPAT_MINGW_MS_VSNPRINTF
/*
 * This fixes linking with precompiled libusb-1.0.18-win and
 * libusb-1.0.19-rc1-win: "undefined reference to __ms_vsnprintf". See:
 * http://sourceforge.net/p/mingw-w64/mailman/mingw-w64-public/thread/4F8CA26A.70103@users.sourceforge.net/
 */

#include <stdio.h>
#include <stdarg.h>

int __cdecl __ms_vsnprintf(char* __restrict__ d, size_t n, const char* __restrict__ format,
                           va_list arg)
{
    return vsnprintf(d, n, format, arg);
}
#endif /* COMPAT_MINGW_MS_VSNPRINTF */

#ifdef WIN32
int is_file_exist(const char* filename)
{
    return GetFileAttributes(filename) != INVALID_FILE_ATTRIBUTES;
}
#elif UNIX
int is_file_exist(const char* filename)
{
    struct stat st;
    int result = stat(filename, &st);
    return result == 0;
}
#endif

char* get_config_file_path(void)
{
    static char* path;
    char* envvar = NULL;

    if (path == NULL) {
        path = malloc(65535);
        envvar = getenv(ENV_VAR);

        if (envvar == NULL) {
            fprintf(stderr, "Env.variable \"%s\" not found\n", ENV_VAR);
        }

        strcpy(path, envvar);
        strcat(path, DEFAULT_CFG_PATH);
    }

    return path;
}
