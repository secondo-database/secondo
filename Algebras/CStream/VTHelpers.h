/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//characters [1] Type: [] []
//characters [2] Type: [] []
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Helper functions.

[toc]

1 Helper functions

*/
#ifndef __VTHELPERS_H__
#define __VTHELPERS_H__

#include "VTuple.h"

namespace cstream {

class VTHelpers {
public:

    static bool IsVTupleStream(ListExpr s) {
        if (nl->ListLength(s) != 2)
            return false;    
        if (!nl->IsEqual(nl->First(s),Symbol::STREAM()))
            return false;            
        if (!VTuple::CheckType(nl->First(nl->Second(s))))        
            return false;
        return true;
    }

    static void PrintList(std::string name, ListExpr l, int maxdepth) {
        cout << name << 
        "-----------------------------------------------" << endl;
        PrintListRec(l, "", 1, maxdepth);
        cout << endl << endl;
    }

    static void PrintListRec(ListExpr l, std::string prefix,
                            int depth, 
                            int maxdepth) {
        if (depth > maxdepth)
            return;

        int len = nl->ListLength(l);
        ListExpr temp = l;
        for (int i=0; i<len; ++i) {
            std::ostringstream ss;
            ss<<(i+1)<<".";
            std::string s = ss.str();
            cout << prefix << s << nl->ToString(nl->First(temp)) 
            << endl;
            PrintListRec(nl->First(temp), prefix + s, 
            depth+1, maxdepth);
            temp = nl->Rest(temp);
        }
    }
};



//#define DEBUG_OUTPUT

class Log
{
public:

    bool Enabled() {
#ifdef DEBUG_OUTPUT
        return true;
#endif
        return false;
    }

    Log& operator<<(const char* s) {
        if (Enabled()) std::cout << s;
        return *this;
    }
    
    Log& operator<<(char* s) {
        if (Enabled()) std::cout << s;
        return *this;
    }
    
    Log& operator<<(std::string s) {
        if (Enabled()) std::cout << s;
        return *this;
    }

    Log& operator<<(int i) {
        if (Enabled()) std::cout << i;
        return *this;
    }

    Log& operator<<(short n) {
        if (Enabled()) std::cout << n;
        return *this;
    }

    Log& operator<<(unsigned int ui) {
        if (Enabled()) std::cout << ui;
        return *this;
    }

    Log& operator<<(char c) {
        if (Enabled()) std::cout << c;
        return *this;
    }

    Log& operator<<(double d) {
        if (Enabled()) std::cout << d;
        return *this;
    }

    Log& operator<<(float f) {
        if (Enabled()) std::cout << f;
        return *this;
    }

    static Log& Inst()
    {
        static Log inst;
        return inst;
    }

protected:
    Log() {};
    Log(const Log&) {};
    ~Log() {};
};

#define LOG Log::Inst()
#define ENDL "\n"

}

#endif
