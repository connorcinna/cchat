#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include "common.h"

const char* log_dir = "../log/debug.log";

//Generic log function, writes to log file
void debug_log(log_severity_t level, char const* filename, char* msg, ...)
{
	va_list argp;
	va_start(argp, msg);

    vprintf(msg, argp);

	va_end(argp);
    //open log_file, write the same message
	FILE* out = fopen(log_dir, "w");
	vfprintf(out, msg, argp);
	fclose(out);
}

//When logging to stdout, set the color ofthe log message depending on the log level
void set_print_color(log_severity_t level) 
{
    printf("");
}
