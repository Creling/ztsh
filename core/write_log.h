#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#ifndef WRITE_LOG_H
#define WRITE_LOG_H

int write_log (FILE* pFile, const char *format, ...);

#endif