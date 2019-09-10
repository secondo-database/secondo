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

enum typelog {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

struct structlog {
    bool headers = false;
    typelog level = INFO;
};

extern structlog LOGCFG;

class LOG {
public:

    LOG() {}

    LOG(typelog type) {
        msglevel = type;
        if (LOGCFG.headers) {
            operator<<("[" + getLabel(type) + "]");
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

    inline std::string getLabel(typelog type) {
        std::string label;
        switch (type) {
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