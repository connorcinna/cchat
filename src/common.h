enum log_severity
{
    INFO,
    WARN,
    SEVERE,
    FATAL
};
typedef enum log_severity log_severity_t;

void debug_log(log_severity_t level, char const* filename, char* msg, ...);

void set_print_color(log_severity_t level);

