/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 MathUtils class

*/

#include "MathUtils.h"
#include <sstream>

namespace pointcloud2 {

void MathUtils::initializeRnd(unsigned seed) {
    if (seed == 0)
        srand(time(NULL));
    else
        srand(seed);
}

double MathUtils::getRnd() {
    return (double)rand() / RAND_MAX;
}

double MathUtils::getRnd(const double min, const double max) {
    double rnd = (double)rand() / RAND_MAX;
    return min + rnd * (max - min);
}

size_t MathUtils::getRndInt(const size_t size) {
    unsigned _denom((RAND_MAX + 1u) / size);
    size_t result;
    do {
        // rand() % size is biased, therefore:
        result = std::rand() / _denom;
    } while (result >= size);
    return result;
}


std::string formatInt(const long num) {
    // there is no manipulator for grouping of thousands, really?
    std::stringstream result;
    size_t n = num;
    if (n < 0) {
        result << "-";
        n = -n;
    }

    size_t div = 1;
    while (n / div >= 1000)
        div *= 1000;

    bool leadingZeros = false;
    do {
        size_t part = (n / div) % 1000;
        if (leadingZeros) {
            if (part < 10)
                result << "00";
            else if (part < 100)
                result << "0";
        }
        result << part;
        if (div == 1)
            break;
        result << "'";
        div /= 1000;
        leadingZeros = true;
    } while (true);
    return result.str();
}


} // namespace


