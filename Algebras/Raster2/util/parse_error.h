/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

*/

#ifndef RASTER2_UTIL_PARSE_ERROR_H
#define RASTER2_UTIL_PARSE_ERROR_H

#include <exception>

namespace raster2
{
    namespace util
    {
/*
1 Class ~parse\_error~

A ~parse\_error~ should be thrown and caught inside an In function to centralize
error handling when parsing list expressions.

*/
        class parse_error : public std::exception {
          public:
            parse_error(const std::string& err) : std::exception(), msg(err) {}
            ~parse_error() throw() {}
            const char* what() const throw() { return msg.c_str(); }
          private:
            std::string msg;
        };
    }
}

#endif /* #ifndef RASTER2_UTIL_PARSE_ERROR_H */
