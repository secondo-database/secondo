/*
----
This file is part of SECONDO.

Copyright (C) 2017,
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
#ifndef ALGEBRAS_DBSERVICE_TRACESETTINGS_HPP_
#define ALGEBRAS_DBSERVICE_TRACESETTINGS_HPP_

namespace DBService {

enum TraceLevel
{
    OFF = 0, // no command line output, no trace files
    FILE = 1, // no command line output, but trace files
    DEBUG = 2 // both command line output and trace files
};

class TraceSettings {
public:
/*
1.1.1 ~getInstance~

Returns the TraceSettings instance (singleton).

*/
    static TraceSettings* getInstance();

/*
1.1.1 ~setTraceLevel~

Sets the trace level as specified.

*/
    void setTraceLevel(TraceLevel level);

/*
1.1.1 ~getTraceLevel~

Retrieves the current trace level.

*/
    TraceLevel getTraceLevel();

/*
1.1.1 ~isDebugTraceOn~

Checks whether the trace level is ~TraceLevel::DEBUG~.

*/
    bool isDebugTraceOn();

/*
1.1.1 ~isFileTraceOn~

Checks whether the trace level is ~TraceLevel::FILE~.

*/
    bool isFileTraceOn();

private:
/*
1.1.1 Constructor

*/
    TraceSettings();

/*
1.1.1 Destructor

*/
    ~TraceSettings(){};
/*

1.1.1 \textit{traceLevel}

Member variable that stores the current trace level.

*/
    TraceLevel traceLevel;

/*

1.1.1 \textit{\_instance}

Pointer to the \textit{TraceSettings} instance (singleton).

*/
    static TraceSettings* _instance;


};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_TRACESETTINGS_HPP_ */
