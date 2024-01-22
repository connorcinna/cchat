#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include "common.h"


void debug_log(log_severity_t level, char const* filename, char* msg, ...)
{
    set_print_color(level);
	va_list argp;
	va_start(argp, msg);

    vprintf(msg, argp);

    set_print_color(DEFAULT);

    //open log_file, write the same message
	FILE* out = fopen(log_dir, "w");
	vfprintf(out, msg, argp);
	fclose(out);
	va_end(argp);
}

void set_print_color(log_severity_t level) 
{
    switch (level) 
    {
        case INFO:
            printf(log_color[INFO]);
            break;
        case WARN:
            printf(log_color[WARN]);
            break;
        case SEVERE:
            printf(log_color[SEVERE]);
            break;
        case FATAL:
            printf(log_color[FATAL]);
            break;
        default:
            printf(log_color[DEFAULT]);
            break;
    }
}
