#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <minos/sysstd.h>
#include <assert.h>

#define SECS_PER_MIN 60
#define SECS_PER_HOUR 3600
#define SECS_PER_DAY 86400
#define DAYS_PER_YEAR 365
#define DAYS_PER_4_YEARS 1461
#define DAYS_PER_400_YEARS 146097

static int days_in_month[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

time_t time(time_t* timer) {
    MinOS_Time time;
    gettime(&time);
    time_t t = time.ms / 1000;
    if(timer) *timer = t;
    return t;
}

double difftime(time_t time1, time_t time0) {
    return (double)time1 - (double)time0;
}

time_t mktime(const struct tm* tm) {
    time_t result = 0;
    int year = tm->tm_year + 1900;
    int mon = tm->tm_mon;
    int day = tm->tm_mday - 1;
    int hour = tm->tm_hour;
    int min = tm->tm_min;
    int sec = tm->tm_sec;

    for(int y = 1970; y < year; y++) {
        result += is_leap_year(y) ? 366 : 365;
    }
    for(int m = 0; m < mon; m++) {
        result += days_in_month[m];
        if(m == 1 && is_leap_year(year)) result++;
    }
    result += day;
    result *= SECS_PER_DAY;
    result += hour * SECS_PER_HOUR + min * SECS_PER_MIN + sec;
    return result;
}

struct tm* localtime(const time_t* timer) {
    static struct tm tm_buf;
    time_t t = *timer;
    int sec = t % SECS_PER_MIN; t /= SECS_PER_MIN;
    int min = t % 60; t /= 60;
    int hour = t % 24; t /= 24;
    int days = t;

    tm_buf.tm_sec = sec;
    tm_buf.tm_min = min;
    tm_buf.tm_hour = hour;
    tm_buf.tm_wday = (days + 4) % 7;

    int year = 1970;
    while(1) {
        int days_in_y = is_leap_year(year) ? 366 : 365;
        if(days < days_in_y) break;
        days -= days_in_y;
        year++;
    }
    tm_buf.tm_year = year - 1900;
    tm_buf.tm_yday = days;

    int mon = 0;
    while(mon < 12) {
        int dim = days_in_month[mon];
        if(mon == 1 && is_leap_year(year)) dim++;
        if(days < dim) break;
        days -= dim;
        mon++;
    }
    tm_buf.tm_mon = mon;
    tm_buf.tm_mday = days + 1;
    tm_buf.tm_isdst = 0;
    return &tm_buf;
}

struct tm* gmtime(const time_t* timer) {
    return localtime(timer);
}

size_t strftime(char *s, size_t max, const char *format, const struct tm *tm) {
    size_t pos = 0;
    while(*format && pos < max - 1) {
        if(*format != '%') {
            s[pos++] = *format++;
            continue;
        }
        format++;
        char buf[16];
        const char* str = buf;
        size_t len = 0;
        switch(*format) {
        case 'Y': len = snprintf(buf, sizeof(buf), "%d", tm->tm_year + 1900); break;
        case 'm': len = snprintf(buf, sizeof(buf), "%02d", tm->tm_mon + 1); break;
        case 'd': len = snprintf(buf, sizeof(buf), "%02d", tm->tm_mday); break;
        case 'H': len = snprintf(buf, sizeof(buf), "%02d", tm->tm_hour); break;
        case 'M': len = snprintf(buf, sizeof(buf), "%02d", tm->tm_min); break;
        case 'S': len = snprintf(buf, sizeof(buf), "%02d", tm->tm_sec); break;
        case '%': s[pos++] = '%'; format++; continue;
        case '\0': goto done;
        default: s[pos++] = *format; format++; continue;
        }
        format++;
        if(pos + len >= max) break;
        memcpy(s + pos, str, len);
        pos += len;
    }
done:
    s[pos] = '\0';
    return pos;
}

clock_t clock(void) {
    MinOS_Time time;
    gettime(&time);
    return time.ms;
}
