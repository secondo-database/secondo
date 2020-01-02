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
#include "log.hpp"

#include <zconf.h>
#include <vector>
#include <websocketpp/utilities.hpp>

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
        if (milliseconds <= 0)
            LOG(ERROR) << "sleepMS: Invalid argument " << milliseconds;

        int sec = milliseconds / 1000;
        milliseconds = milliseconds % 1000;

        // 1 millisecond = 1,000,000 Nanoseconds
        const long INTERVAL_NS = milliseconds * MILLI_SECOND_MULTIPLIER;
        if (INTERVAL_NS > 999999999)
            LOG(WARN) << "INTERVAL_NS > 999999999 warn ";

        timespec sleepValue = {0};
        sleepValue.tv_sec = sec;
        sleepValue.tv_nsec = INTERVAL_NS;

        int rval = nanosleep(&sleepValue, NULL);
        if (rval == 0)
            LOG(TRACE) << "Sleep ok ";
        else if (errno == EINTR)
            LOG(ERROR) << "sleepMS: EINTR 4 Interrupted system call";
        else if (errno == EINVAL)
            LOG(ERROR) << "sleepMS: EINVAL 22 Invalid argument ";
        else
            LOG(ERROR) << "sleepMS: Sleep error " << errno;

//    nanosleep(&sleepValue, &sleepValue);
    }

//    void sleepMS(int milliseconds) {
//        // 1 millisecond = 1,000,000 Nanoseconds
//        const long INTERVAL_MS = milliseconds * MILLI_SECOND_MULTIPLIER;
//        timespec sleepValue = {0};
//
//        sleepValue.tv_nsec = INTERVAL_MS;
//        nanosleep(&sleepValue, NULL);
//    }

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


    void removeMultipleSpaces(std::string &line) {
        size_t pos;
        while( ( pos = line.find( "  " ) )!=std::string::npos )
            line = line.replace( pos, 2, " " );
    }

    // Deprecated
// Function to in-place trim all spaces in the
// string such that all words should contain only
// a single space between them.
    void removeSpaces1(std::string &str) {
        std::cout << "removeSpaces from " << str;
        // n is length of the original string
        int n = str.length();

        // i points to next position to be filled in
        // output string/ j points to next character
        // in the original string
        int i = 0, j = -1;

        // flag that sets to true is space is found
        bool spaceFound = false;

        // Handles leading spaces
        while (++j < n && str[j] == ' ');

        // read all characters of original string
        while (j < n) {
            // if current characters is non-space
            if (str[j] != ' ') {
                // copy current character at index i
                // and increment both i and j
                str[i++] = str[j++];

                // set space flag to false when any
                // non-space character is found
                spaceFound = false;
            }
                // if current character is a space
            else if (str[j++] == ' ') {
                // If space is encountered for the first
                // time after a word, put one space in the
                // output and set space flag to true
                if (!spaceFound) {
                    str[i++] = ' ';
                    spaceFound = true;
                }
            }
        }

        // Remove trailing spaces
        if (i <= 1)
            str.erase(str.begin() + i, str.end());
        else
            str.erase(str.begin() + i - 1, str.end());
        std::cout << " res " << str << std::endl;
    }

    int parseInt(const std::string &value) {
        try {
            return stoi(value);
        } catch (const std::exception &e) {
            LOG(ERROR)
                    << "parseInt: Error parsing  " << value;
            return 0;
        }
    }

    double parseDouble(const std::string &value) {
        try {
            return stod(value);
        } catch (const std::exception &e) {
            LOG(ERROR)
                    << "parseDouble: Error parsing  " << value;
            return 0;
        }
    }

    bool parseBoolean(std::string basicString) {
        std::string str = websocketpp::utility::to_lower(basicString);
        if (str == "true" || str == "t")
            return true;
        if (str == "false" || str == "f")
            return false;

        return std::stoi(str);
    }


}