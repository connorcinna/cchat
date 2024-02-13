#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <ncurses.h>
#include "common.h"

const char* log_color[5] = 
{
    "\033[0;37m", //white
    "\033[0;33m", //yellow
    "\033[0;35m", //purple
    "\033[0;31m", //red
    "\033[0m"     //reset to default 
};

char cwd[1024];

//TODO: this only works when running it from the main directory - maybe pass absolute path, or check
//if the current directory has a log directory in it, which should be the case if running from root dir 
void init_log_path(void)
{
#ifdef __linux__
    if (!getcwd(cwd, sizeof(cwd)))
    {
        printf("Unable to get current working directory: %s\n", strerror(errno));
		exit(-1);
    }
    strcat(cwd, "/log/debug.log");
#elif _WIN32
	strcat(cwd, "debug.log");
#else
#endif
}

char* log_prefix(char const* filename)
{
	char out[512];
	time_t raw_time = time(NULL);
	struct tm* local_time = localtime(&raw_time);
	char* str_time = asctime(local_time);
	str_time[strcspn(str_time, "\n")] = 0; //strip the newline character
	strcpy(out, str_time);
	strcat(out, " ");
	strcat(out, filename);
	char* ret = malloc(sizeof(out));
	memcpy(ret, out, sizeof(out));
	return ret;
}

void debug_log(log_severity_t level, char const* filename, char* msg, ...)
{
    if (cwd[0] == '\0')
    {
        init_log_path();
    }
    FILE* out;
    set_print_color(level);
    //get current time
	//initialize the format argument array
	va_list argp;
	va_start(argp, msg);
    //make a copy of the format arguments - used when writing to the file
    va_list copy;
    va_copy(copy, argp);
    //print timestamp and file this came from 
	char* pre = log_prefix(filename);
//	printw("[ %s ] ", pre);
	//print formatted string
//    vprintf(msg, argp);
    //reset print color back to default
    set_print_color(DEFAULT);
    //open cwd, write the same message
	if (!(out = fopen(cwd, "a"))) 
    {
        printw("Unable to open file %s: %s\n", cwd, strerror(errno));
    }
    //print to file
	fprintf(out, "[ %s ] ", pre);
    if (!vfprintf(out, msg, copy))
    {
        printw("Error writing to file: %s", strerror(errno));
    }
    //free allocations
	free(pre);
	fclose(out);
	va_end(argp);
    va_end(copy);
}

void chat_print(char* msg, ...)
{
	set_print_color(DEFAULT);
	va_list argp;
	va_start(argp, msg);
    vprintf(msg, argp);
	//printw(msg, argp);
}

void set_print_color(log_severity_t level) 
{
    switch (level) 
    {
        case INFO:
            printf("%s",log_color[INFO]);
            break;
        case WARN:
            printf("%s",log_color[WARN]);
            break;
        case ERROR:
            printf("%s",log_color[ERROR]);
            break;
        case FATAL:
            printf("%s",log_color[FATAL]);
            break;
        default:
            printf("%s",log_color[DEFAULT]);
            break;
    }
}
