/*
---- 
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty for Mathematics 
and Computer Science, Database Systems for New Applications.

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

#ifndef TRACE_MACROS_H
#define TRACE_MACROS_H


// some macros which may be useful for tracing the program execution
#ifdef TRACE_ON
#undef ETRACE
#define ETRACE(a) { a }

#undef TRACE_FILE
#define TRACE_FILE(n) {{ \
  ofstream* traceFilePtr = new ofstream(n, ios_base::out | ios_base::app); \
  traceOS = traceFilePtr; }}

#undef TRACE_OS
#define TRACE_OS(os) { traceOS = &os; }  

#undef TRACE
#define TRACE(a) {*traceOS << a << endl;}

#undef NTRACE
#define NTRACE(n,a) { static int ctr=0; ctr++; \
                      if ( (ctr % n)  == 0) \
                       {*traceOS << ctr << " - " << a << endl; }}

#undef SHOW
#define SHOW(a) {*traceOS << "  " << #a << " = " << a << endl;} 

#undef TRACE_ENTER
#define TRACE_ENTER {*traceOS << "* Entering " \
                              << __FUNCTION__ << "@" << __LINE__ << endl;}

#undef TRACE_LEAVE
#define TRACE_LEAVE {*traceOS << "* Leaving  " \
                              << __FUNCTION__ << "@" << __LINE__ << endl;}

#undef DEBUG_MSG
#define DEBUG_MSG(msg) { \
    cerr << __FUNCTION__ << "@" << __LINE__ << ": " \
         << msg << endl; }

#undef DEBUG_VAL
#define DEBUG_VAL(var) { \
    cerr << __FUNCTION__ << "@" << __LINE__ << ": " \
         << #var << " = " << var << endl; }

#undef DEBUG_EXE
#define DEBUG_EXE(expr) { expr }

#else

#define ETRACE(a)
#define TRACE(a)
#define NTRACE(n,a)
#define SHOW(a)
#define TRACE_FILE(n)
#define TRACE_OS(os)
#define TRACE_ENTER
#define TRACE_LEAVE

#define DEBUG_MSG(msg)
#define DEBUG_VAL(var)
#define DEBUG_EXE(expr)
#endif

#endif
