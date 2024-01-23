#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include "common.h"

const char* log_color[5] = 
{
    "\033[0;37m", //white
    "\033[0;33m", //yellow
    "\033[0;35m", //purple
    "\033[0;31m", //red
    "\033[0m"     //reset to default 
};

static char cwd[256];

void init_log_path()
{
    //char cwd[256];
    if (!getcwd(cwd, sizeof(cwd)))
    {
        printf("Unable to get current working directory: %s\n", strerror(errno));
    }
    strcat(cwd, "/log/debug.log");
}

void debug_log(log_severity_t level, char const* filename, char* msg, ...)
{
    FILE* out;
    set_print_color(level);
    //initialize the format argument array
	va_list argp;
	va_start(argp, msg);
    //make a copy of the format arguments - used when writing to the file
    va_list copy;
    va_copy(copy, argp);
    //print to console
    vprintf(msg, argp);
    //reset print color back to default
    set_print_color(DEFAULT);
    //open cwd, write the same message
	if (!(out = fopen(cwd, "a"))) 
    {
        printf("Unable to open file %s: %s\n", cwd, strerror(errno));
    }
    //print to file
    if (!vfprintf(out, msg, copy))
    {
        printf("Error writing to file: %s", strerror(errno));
    }
    //free resources
	fclose(out);
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
