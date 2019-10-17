/*
----
This file is part of SECONDO.

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <sys/time.h>

enum typelog {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR
};

struct structlog {
    bool headers = true;
    typelog level = DEBUG;
};

extern structlog LOGCFG;
extern timeval oldTime;


class LOG {
public:

    LOG() {}

    LOG(typelog type) {
        msglevel = type;
        if (LOGCFG.headers && (msglevel >= LOGCFG.level)) {
            operator<<("[" + getLabel(type) + "]") << getDiff() << " "
                                                   << getFormattedTime() << " ";
        }
    }

    ~LOG() {
        if (opened) {
            std::cout << std::endl;
        }
        opened = false;
    }

    template<class T>
    LOG &operator<<(const T &msg) {
        if (msglevel >= LOGCFG.level) {
            std::cout << msg;
            opened = true;
        }
        return *this;
    }

private:
    bool opened = false;
    typelog msglevel = DEBUG;

    static std::string getFormattedTime() {
        timeval curTime{};
        gettimeofday(&curTime, NULL);
        int milli = curTime.tv_usec / 1000;

        char buffer[80];
        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));

        char currentTime[84] = "";
        sprintf(currentTime, "%s:%d", buffer, milli);
        return std::string(currentTime);
    }

    static std::string formatDiff(const timeval &curTime) {
        char buffer[80];
        strftime(buffer, 80, "%H:%M:%S", gmtime(&curTime.tv_sec));

        char currentTime[100] = "";
        sprintf(currentTime, "[Δ %s:%06ld]", buffer, curTime.tv_usec);
        return std::string(currentTime);
    }

    std::string getDiff() {
        if (oldTime.tv_sec == 0 && oldTime.tv_usec == 0)
            gettimeofday(&oldTime, NULL);

        timeval curTime{};
        gettimeofday(&curTime, NULL);

        timeval diff{};
        timeval_subtract(&diff, &curTime, &oldTime);
        oldTime = curTime;

        return formatDiff(diff);
    }

    static void timeval_subtract(struct timeval *result, struct timeval *x,
                                 struct timeval *y) {
        /* Perform the carry for the later subtraction by updating y. */
        if (x->tv_usec < y->tv_usec) {
            int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
            y->tv_usec -= 1000000 * nsec;
            y->tv_sec += nsec;
        }
        if (x->tv_usec - y->tv_usec > 1000000) {
            int nsec = (x->tv_usec - y->tv_usec) / 1000000;
            y->tv_usec += 1000000 * nsec;
            y->tv_sec -= nsec;
        }

        /* Compute the time remaining to wait.
           tv_usec is certainly positive. */
        result->tv_sec = x->tv_sec - y->tv_sec;
        result->tv_usec = x->tv_usec - y->tv_usec;
    }


    inline std::string getLabel(typelog type) {
        std::string label;
        switch (type) {
            case TRACE:
                label = "TRACE";
                break;
            case DEBUG:
                label = "DEBUG";
                break;
            case INFO:
                label = "INFO ";
                break;
            case WARN:
                label = "WARN ";
                break;
            case ERROR:
                label = "ERROR";
                break;
        }
        return label;
    }
};

#endif //LOG_H