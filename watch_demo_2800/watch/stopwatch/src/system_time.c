#include "../include/systemtime.h"
#include <string.h>

void system_time_get_string(char *buffer, size_t buffer_size)
{
    time_t now;
    struct tm *tm_info;

    time(&now);
    now += 8 * 3600;//换算为北京时间
    tm_info = localtime(&now);
    
    strftime(buffer, buffer_size, "%H:%M", tm_info);
}