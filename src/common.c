#include <errno.h>
#include <stdio.h>
#include "common.h"

void log_error(char* msg)
{
    printf("%s, error code: %s", errno);
}
