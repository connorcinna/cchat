#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "common.h"

const char* log_color[5] = 
{
    "\033[0;37m", //white
    "\033[0;33m", //yellow
    "\033[0;35m", //purple
    "\033[0;31m", //red
    "\033[0m"     //reset to default 
};

const char* log_file = "../log/debug.log";

void debug_log(log_severity_t level, char const* filename, char* msg, ...)
{
    printf("Setting log color\n");
    set_print_color(level);
    printf("Getting va_list\n");
	va_list argp;
    printf("Calling va_start\n");
	va_start(argp, msg);
    va_list copy;
    va_copy(copy, argp);
    printf("Calling vprintf\n");
    vprintf(msg, argp);
    printf("Resetting color\n");
    set_print_color(DEFAULT);
    //open log_file, write the same message
    printf("Opening log file\n");
	FILE* out = fopen(log_file, "a");
    printf("Printing to file\n");
    if (!vfprintf(out, msg, copy))
    {
        printf("Error writing to file: %s", strerror(errno));
    }
    printf("Closing file descriptor\n");
	fclose(out);
    printf("Calling va_end\n");
	va_end(argp);
    va_end(copy);
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
