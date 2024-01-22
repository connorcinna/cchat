#ifndef COMMON_H
#define COMMON_H
enum log_severity
{
    INFO,
    WARN,
    SEVERE,
    FATAL,
    DEFAULT
};

typedef enum log_severity log_severity_t;

const char* log_color[5] = 
{
    "\033[0;37m", //white
    "\033[0;33m", //yellow
    "\033[0;35m", //purple
    "\033[0;31m", //red
    "\033[0m"     //reset to default 
};

const char* log_dir = "../log/debug.log";

//Generic log function, writes to log file
void debug_log(log_severity_t level, char const* filename, char* msg, ...);

//When logging to stdout, set the color ofthe log message depending on the log level
void set_print_color(log_severity_t level);

#endif
