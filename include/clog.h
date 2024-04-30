#ifndef CLOG_H
#define CLOG_H

#ifdef __cplusplus
extern "C"{
#endif

#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array)[0])) //i dont actually use this anywhere right now
#define ctrl(x) ((x) & 0x1f) //don't use this either, but maybe useful at some point

enum log_severity
{
    INFO,
    WARN,
   	ERROR, 
    FATAL,
    DEFAULT
};


typedef enum log_severity log_severity_t;

#define clog(log_severity, ...) debug_log(log_severity, __FILE__, __VA_ARGS__)

//initializes the log path for this module
void init_log_path(void);

//return the current local time for printing to the console
char* log_prefix(char const* filename);

//Generic log function, writes to log file
void debug_log(log_severity_t level, char const* filename, const char* msg, ...);

//When logging to stdout, set the color ofthe log message depending on the log level
void set_print_color(log_severity_t level);
#ifdef __cplusplus
}
#endif

#endif
