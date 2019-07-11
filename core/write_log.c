#include "write_log.h"

int write_log (FILE* pFile, const char *format, ...) {
    va_list arg;
    int done;

    va_start (arg, format);

    time_t time_log = time(NULL);
    struct tm* tm_log = localtime(&time_log);
    fprintf(pFile, "%04d-%02d-%02d %02d:%02d:%02d ", tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday, tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec);

    done = vfprintf (pFile, format, arg);
    va_end (arg);

    fflush(pFile);
    return done;
}
