/* 
 * This file is part of libfmr
 * 
 * File:   Interval.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 9, 2016, 5:00 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Class Interval

[TOC]

1 Overview

This class represents a time interval consisting of two instants
together with the information, if the interval is leftclosed and/or
rightclosed

*/

#include "fmr_Interval.h"

using namespace fmr;

/*
2 ~timestr~

Converts a unix timestamp (ms since 1970-01-01 00:00:00)
into a string of the form YYYY-mm-dd-HH:MM:ss.SSS
 
*/
std::string Interval::timestr(double t) {
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
3 ~parsetime~

Converts a string of the form YYYY-mm-dd HH:MM:ss.SSS into
a unix timestamp (ms since 1970-01-01 00:00:00)
 
*/
double Interval::parsetime (std::string str) {
   struct tm tm;
   unsigned int msec;
   char sep; // Separator, space or -
   
   tm.tm_year = tm.tm_mon = tm.tm_mday = 0;
   tm.tm_sec = tm.tm_min = tm.tm_hour = tm.tm_isdst = msec = 0;
   
   int st = sscanf(str.c_str(), "%u-%u-%u%c%u:%u:%u.%u",
                   &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &sep,
                   &tm.tm_hour, &tm.tm_min, &tm.tm_sec,
                   &msec);
   if (st < 3)
       return NAN;
    
   tm.tm_year -= 1900; // struct tm expects years since 1900
   tm.tm_mon--; // struct tm expects months to be numbered from 0 - 11
   
   double ret = utctime(&tm) * 1000 + msec;
   
   return ret;
}

/*
4 ~startstr~

Returns a textual representation of the beginning of the time interval. 

*/
std::string Interval::startstr() {
    return timestr(start);
}

/*
5 ~endstr~

Returns a textual representation of the end of the time interval.
 
*/
std::string Interval::endstr() {
    return timestr(end);
}

/*
6 Constructor for a time interval.

Parameters: 
     st : Date/Time string in the format of 1.2
     en : Date/Time string in the format of 1.2
     lc : FALSE, if the interval is left-open
     rc : FALSE, if the interval is right-open
 
*/
Interval::Interval(std::string st, std::string en, bool lc, bool rc) :
                                                                lc(lc), rc(rc) {
    start = parsetime(st);
    end = parsetime(en);
}

/*
7 Constructor for a time interval from a RList representation

*/
Interval::Interval(RList& l) {
    start = parsetime(l[0].getString());
    end = parsetime(l[1].getString());
    lc = l[2].getBool();
    rc = l[3].getBool();
}

/*
8 ~intersection~

Calculates the intersection of two time intervals. Also handles
the closedness correctly

*/
Interval Interval::intersection(Interval& iv) {
    Interval ret;
    
    ret.start = (iv.start > start) ? iv.start : start;
    ret.end   = (iv.end   < end  ) ? iv.end   : end  ;
    
    if (iv.start == start)
        ret.lc = iv.lc && lc;
    else
        ret.lc = (iv.start < start) ? lc : iv.lc;
    
    if (iv.end == end)
        ret.rc = iv.rc && rc;
    else
        ret.rc = (iv.end > end) ? rc : iv.rc;
    
    return ret;
}

/*
9 ~intersects~

Determines if two time intervals intersects. Also handles
the closedness correctly

*/
bool Interval::intersects(Interval& iv) {
    if ( (iv.start > end) ||
        ((iv.start == end) && (!iv.lc || !rc)))
        return false;
    
    if ( (start > iv.end) ||
        ((start == iv.end) && (!lc || !iv.rc)))
        return false;
    
    return true;
}

/*
10 ~getFrac~

Project an instant in the interval into the range [0;1]

*/
double Interval::getFrac(double currentTime) {
    return (currentTime-start)/(end-start);
}

/*
11 ~valid~

Tests, if an interval is valid. This is the case, when start < end or
start == end and lc == true and rc == true

*/
bool Interval::valid() {
    return (start < end) || (start == end && lc && rc);
}

/*
12 ~project~

Reverse operation ~getFrac~ (9), get an instant from a
fraction (i.e. project(0.5) yields the middle of the time interval)

*/
double Interval::project(double t) {
    return end*t+start*(1-t);
}

/*
13 ~ToString~

Return a string representation of the interval

*/
std::string Interval::ToString() {
    return "( "+timestr(start)+" "+timestr(end)+" "+
           (lc?"TRUE":"FALSE")+" "+(rc?"TRUE":"FALSE")+" )";
}

/*
13 ~toRList~

Return an RList representation of the interval

*/
RList Interval::toRList() {
    RList ret;
    
    ret.append(timestr(start));
    ret.append(timestr(end));
    ret.append(lc);
    ret.append(rc);
    
    return ret;
}
