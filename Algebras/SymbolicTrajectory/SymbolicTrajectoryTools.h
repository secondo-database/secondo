
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Auxiliary functions of the Symbolic Trajectory Algebra

Started March 2012, Fabio Vald\'{e}s

*/
#include <cstdio>
#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include "TemporalAlgebra.h"
#include "TemporalUnitAlgebra.h"
#include "SecParser.h"

using namespace std; 

string int2String(int i);
int str2Int(string const &text);
set<string> stringToSet(string input);
string setToString(set<string> input);
vector<string> splitPattern(string input);
char* convert(string arg);
string extendDate(string input, const bool start);
bool checkSemanticDate(const string text, const SecInterval uIv,
                       const bool resultNeeded);
bool checkDaytime(const string text, const SecInterval uIv);
bool evaluate(string condStr, const bool eval);