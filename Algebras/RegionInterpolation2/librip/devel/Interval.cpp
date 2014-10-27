/*
   1 Interval.cpp
  
   Helper class for defining and handling a time interval.
 
*/

#include "interpolate.h"
#include <ctime>
#include <cstdio>

/*
   1.1 ~timestr~ converts a unix timestamp (ms since 1970-01-01 00:00:00)
       into a string of the form YYYY-mm-dd-HH:MM:ss.SSS
 
*/
static string timestr(double t) {
    struct tm *tm;
    char buf[32], ret[40];
    time_t ti;
    ti = t/1000;
    
    tm = gmtime(&ti);
    strftime(buf, sizeof(buf), "%F-%T", tm);
    sprintf(ret, "%s.%03d", buf, (int) fmod(t,1000));
    return ret;
}

// Helper-function to convert a struct tm interpreted as UTC to unix epoch
static double utctime (struct tm *tm) {
   char *tz;
   double ret;
   
   tz = getenv("TZ");
   setenv("TZ", "UTC", 1);
   tzset();
   ret = mktime(tm);
   if (tz)
     setenv("TZ", tz, 1);
   else
     unsetenv("TZ");
   tzset();
   
   return ret;
}
   

/*
   1.2 ~parsetime~ converts a string of the form YYYY-mm-dd HH:MM:ss.SSS into
       a unix timestamp (ms since 1970-01-01 00:00:00)
 
*/
static double parsetime (string str) {
   struct tm tm;
   unsigned int msec;
   char sep; // Separator, space or -
   
   tm.tm_year = tm.tm_mon = tm.tm_mday = 0;
   tm.tm_sec = tm.tm_min = tm.tm_hour = tm.tm_isdst = msec = 0;
   
   sscanf(str.c_str(), "%u-%u-%u%c%u:%u:%u.%u",
                 &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &sep,
                 &tm.tm_hour, &tm.tm_min, &tm.tm_sec,
                 &msec);
   
   tm.tm_year -= 1900; // struct tm expects years since 1900
   tm.tm_mon--; // struct tm expects months to be numbered from 0 - 11
   
   double ret = utctime(&tm) * 1000 + msec;
   
   return ret;
}

/*
   1.3 ~startstr~ returns a textual representation of the beginning of
       the time interval.
 
*/
string Interval::startstr() {
    return timestr(start);
}

/*
   1.4 ~endstr~ returns a textual representation of the end of
       the time interval.
 
*/
string Interval::endstr() {
    return timestr(end);
}

/*
   1.5 Constructor for a time interval.
       Parameters: 
                  st : Date/Time string in the format of 1.2
                  en   : Date/Time string in the format of 1.2
                  lc     : FALSE, if the interval is left-open
                  rc     : FALSE, if the interval is right-open
 
*/
Interval::Interval(string st, string en, bool lc, bool rc) : lc(lc), rc(rc) {
    start = parsetime(st);
    end = parsetime(en);
}
