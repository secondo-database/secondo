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

#include <iostream>
#include <thread>


#include "../../SignalingSockets.h"

int main() {

    std::cout << "Starting socket server self stop" << std::endl;
    SignallingSocket simpleSocket1;
    simpleSocket1.open(8080);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    simpleSocket1.close();
    std::cout << "Finished 1" << std::endl;

    std::cout << "Starting socket server for waiting" << std::endl;
    SignallingSocket simpleSocket2;
    simpleSocket2.open(8080);
    while (!simpleSocket2.isSignalReceived()) {
        std::cout << "Waiting" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    simpleSocket2.close();
    std::cout << "Finished 2" << std::endl;
    return 0;
}