#ifndef UI_CALENDAR_H_
#define UI_CALENDAR_H_

struct tm a;

typedef struct {
   int tm_sec;         /* seconds,  range 0 to 59          */
   int tm_min;         /* minutes, range 0 to 59           */
   int tm_hour;        /* hours, range 0 to 23             */
   int tm_mday;        /* day of the month, range 1 to 31  */
   int tm_mon;         /* month, range 0 to 11             */
   int tm_year;        /* The number of years since 1900   */
   int tm_wday;        /* day of the week, range 0 to 6    */
   int tm_yday;        /* day in the year, range 0 to 365  */
} custom_tm;

void InitialiseCalendarValues(int sec, int min, int hour, int mday, int month, int year, int wday, int yday);
void GetCalendarTime(char * buffer);
void IncrementCalendarSecond();

#endif /* UI_CALENDAR_H_ */
