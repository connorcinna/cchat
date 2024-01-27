#ifndef COMMON_H
#define COMMON_H

#define MAXCONN 10
#define BUF_SZ 256

enum log_severity
{
    INFO,
    WARN,
    SEVERE,
    FATAL,
    DEFAULT
};

typedef enum log_severity log_severity_t;

//initializes the log path for this module
void init_log_path(void);

//return the current local time for printing to the console
char* log_prefix(char const* filename);

//Generic log function, writes to log file
void debug_log(log_severity_t level, char const* filename, char* msg, ...);

//When logging to stdout, set the color ofthe log message depending on the log level
void set_print_color(log_severity_t level);


#endif
