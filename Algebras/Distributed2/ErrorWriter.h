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


//[$][\$]
//[_][\_]

*/

#ifndef ALGEBRAS_DISTRIBUTED2_ERRORWRITER_H_
#define ALGEBRAS_DISTRIBUTED2_ERRORWRITER_H_

#include <fstream>
#include <string>

#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include "StringUtils.h"

namespace distributed2 {

/*
1 Class ~ErrorWriter~

This class will help to write error log files for the Distributed2Algebra.

*/
class ErrorWriter{
 public:
    ErrorWriter();

    ~ErrorWriter();

    void writeLog(const std::string& host, const int port,
              const int server_pid, const std::string& message);

 private:
     std::string dbname;
     std::string pid;
     boost::mutex mtx;
     std::ofstream out;

};
} /* namespace distributed2 */

#endif /* ALGEBRAS_DISTRIBUTED2_ERRORWRITER_H_ */
