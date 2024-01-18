#include <errno.h>
#include <stdio.h>

enum log_severity
{
    INFO,
    WARN,
    SEVERE,
    FATAL
};
typedef enum log_severity log_severity_t;

const char* log_dir = "../log/";

//Generic log function, writes to log file
void debug_log(log_severity_t level, const char* msg, char const* filename)
{
    printf("%s: %s", filename, msg);
    //open log_file,write the same message
}

//When logging to stdout, set the color ofthe log message depending on the log level
void set_print_color(log_severity_t level) 
{
    printf("");
}
