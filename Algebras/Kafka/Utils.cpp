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
#include "Utils.h"

#include <zconf.h>
#include <vector>

namespace kafka {

    std::string create_uuid() {
        std::string uuid;
        char buffer[128];

        const char *filename = "/proc/sys/kernel/random/uuid";
        FILE *file = fopen(filename, "r");

        // Does the proc file exists?
        if (access(filename, R_OK) == -1) {
            std::cerr << "Unable to get UUID from kernel" << std::endl;
            exit(-1);
        }

        if (file) {
            while (fscanf(file, "%s", buffer) != EOF) {
                uuid.append(buffer);
            }
        }

        fclose(file);
        return uuid;
    }

#define MILLI_SECOND_MULTIPLIER  1000000

    void sleepMS(int milliseconds) {
        // 1 millisecond = 1,000,000 Nanoseconds
        const long INTERVAL_MS = milliseconds * MILLI_SECOND_MULTIPLIER;
        timespec sleepValue = {0};

        sleepValue.tv_nsec = INTERVAL_MS;
        nanosleep(&sleepValue, NULL);
    }

    std::vector<std::string> split(const std::string &str,
                                   const std::string &delim) {
        std::vector<std::string> tokens;
        size_t prev = 0, pos = 0;
        do {
            pos = str.find(delim, prev);
            if (pos == std::string::npos) pos = str.length();
            std::string token = str.substr(prev, pos - prev);
            if (!token.empty()) tokens.push_back(token);
            prev = pos + delim.length();
        } while (pos < str.length() && prev < str.length());
        return tokens;
    }
}