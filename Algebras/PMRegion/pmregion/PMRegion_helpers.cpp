/* 
 * This file is part of libpmregion
 * 
 * File:   PMRegion\_helpers.cpp
 * Author: Florian Heinz <fh@sysv.de>
 
 1 Helper functions
   Date/Time conversion 
   Range processing

*/

#include "PMRegion_internal.h"
#include <sstream>
#include <iomanip>
#include <map>
#include <ctime>

using namespace pmr;

namespace pmr {

/* 
   Helper functions for converting date/time strings into a double
   representing the ms since unix epoch

*/

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

double parsetime (std::string str) {
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

string timestr(double t) {
    struct tm *tm;
    char buf[32], ret[40];
    time_t ti;
    ti = t/1000+3600;

    tm = gmtime(&ti);
    strftime(buf, sizeof(buf), "%F-%T", tm);
    sprintf(ret, "%s.%03d", buf, (int) fmod(t,1000));
    return ret;
}

string timestr(Kernel::FT t) {
    return timestr(::CGAL::to_double(t));
}

template <typename T> void Range<T>::addrange(T a, T b) {
    if (a > b)
        std::swap(a, b);
    typename std::map<T, T>::iterator it1 = range.upper_bound(a);
    if (it1 != range.begin() && (--it1,it1++)->second >= a)
        it1--;
    if (it1 != range.end() && it1->first < a)
        a = it1->first;
    typename std::map<T, T>::iterator it2(it1);
    while (it2 != range.end() && it2->first <= b)
        it2++;
    if (it2 != range.begin() && (--it2,it2++)->second > b)
        b = (--it2,it2++)->second;
    range.erase(it1, it2);
    range[a] = b;
}

template <typename T> void Range<T>::print() {
    typename std::map<T, T>::iterator it = range.begin();
    while (it != range.end()) {
        cerr << it->first << " - " << it->second << endl;
        it++;
    }
}

std::set<Kernel::FT> getZEvents (Polyhedron p) {
    std::set<Kernel::FT> zevents;
    for (Vertex_iterator v = p.vertices_begin(); v != p.vertices_end(); ++v) {
        zevents.insert(v->point().z());
    }

    return zevents;
}


// Keep the linker happy
void rangeinstance () {
    Range<Kernel::FT> dummy;
    dummy.addrange(0, 0);
}

}
