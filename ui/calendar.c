#include "calendar.h"
#include <stdint.h>
// #include <time.h>
// #include <stdio.h>
#include <utils/ustdlib.h>

static custom_tm global_tm;

char *days[7] = {
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Sat",
        "Sun"};

char *months[12] = {
        "JAN",
        "FEB",
        "MAR",
        "APR",
        "MAY",
        "JUN",
        "JUL",
        "AUG",
        "SEP",
        "OCT",
        "NOV",
        "DEC" };

/*
 * Function Prototypes.
 */
// Thu Aug 23 09:12:05 2012
void InitialiseCalendarValues(int sec, int min, int hour, int mday, int month, int year, int wday, int yday);
void GetCalendarTime(char * buffer);
void IncrementCalendarSecond();
static void IncrementCalendarMinute();
static void IncrementCalendarHour();
static void IncrementCalendarDay();
static void IncrementCalendarMonth();
static void IncrementCalendarYear();

void InitialiseCalendarValues(int sec, int min, int hour, int mday, int month, int year, int wday, int yday) {
    global_tm.tm_sec = sec;
    global_tm.tm_min = min;
    global_tm.tm_hour = hour;
    global_tm.tm_mday = mday;
    global_tm.tm_mon = month;
    global_tm.tm_year = year;
    global_tm.tm_wday = wday;
    global_tm.tm_yday = yday;
}

void GetCalendarTime(char * buffer) {
//    char dest[50], sn[2], yr[4];
//
//    strcat(dest, days[global_tm.tm_wday]);
//    strcat(dest, " ");
//    strcat(dest, months[global_tm.tm_mon]);
//    strcat(dest, " ");
//
//    sprintf(sn, "%02d", global_tm.tm_mday);
//    strcat(dest, sn);
//    strcat(dest, " ");
//    sprintf(sn, "%02d", global_tm.tm_hour);
//    strcat(dest, sn);
//    strcat(dest, ":");
//    sprintf(sn, "%02d", global_tm.tm_min);
//    strcat(dest, sn);
//    strcat(dest, ":");
//    sprintf(sn, "%02d", global_tm.tm_sec);
//    strcat(dest, sn);
//    strcat(dest, " ");
//    sprintf(yr, "%04d", global_tm.tm_year);
//    strcat(dest, yr);

    usprintf(buffer, "%02d/%02d/%04d %02d:%02d:%02d", global_tm.tm_mday, global_tm.tm_mon, global_tm.tm_year, global_tm.tm_hour, global_tm.tm_min, global_tm.tm_sec);

    // strcpy(buffer, dest);
}


void IncrementCalendarSecond() {
    global_tm.tm_sec++;

    if (global_tm.tm_sec > 59) {
        global_tm.tm_sec = 0;
        IncrementCalendarMinute();
    }
}

static void IncrementCalendarMinute() {
    global_tm.tm_min++;

    if (global_tm.tm_min > 59) {
        global_tm.tm_min = 0;
        IncrementCalendarHour();
    }
}

static void IncrementCalendarHour() {
    global_tm.tm_hour++;

    if (global_tm.tm_hour > 23) {
        global_tm.tm_hour = 0;
        IncrementCalendarDay();
    }
}

static void IncrementCalendarDay() {
    global_tm.tm_mday++;
    global_tm.tm_wday++;
    global_tm.tm_yday++;

    if (global_tm.tm_wday > 6) {
        global_tm.tm_wday = 0;
    }

    if (global_tm.tm_yday > 365) {
        global_tm.tm_yday = 0;
    }

    if (global_tm.tm_mday > 31) {
        global_tm.tm_mday = 1;
        IncrementCalendarMonth();
    }
}

static void IncrementCalendarMonth() {
    global_tm.tm_mon++;

    if (global_tm.tm_mon > 11) {
        global_tm.tm_mon = 0;
        IncrementCalendarYear();
    }
}

static void IncrementCalendarYear() {
    global_tm.tm_year++;
}
