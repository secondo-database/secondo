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
//[_] [\_]

[1] Header File of the Symbolic Trajectory Algebra

[TOC]

1 Overview

This header file essentially contains the definition of the class ~Pattern~.

2 Defines and includes

*/
// [...]

// --- Including header-files
#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "StandardTypes.h"
#include "TemporalAlgebra.h"
#include "TemporalExtAlgebra.h"
#include "FTextAlgebra.h"
#include <DateTime.h>
#include "CharTransform.h"
#include "SymbolicTrajectoryPattern.h"
#include "SymbolicTrajectoryTools.h"
#include "SymbolicTrajectoryDateTime.h"
#include "Stream.h"
#include <string>

using namespace std;

namespace stj {

class MLabel : public MString {
  public:
    static const string BasicType() { return "mlabel"; }
    static ListExpr MLabelProperty();
    static bool CheckMLabel( ListExpr type, ListExpr& errorInfo );
};

class ULabel : public UString {
  public:
    static const string BasicType() { return "ulabel"; }
    static ListExpr ULabelProperty();
    static bool CheckULabel( ListExpr type, ListExpr& errorInfo );
};

class Pattern {
  public:
    Pattern(string const &text);
    inline Pattern( string const &text , PatParser const &patParser);
    Pattern( const Pattern& rhs );
    ~Pattern();
    inline string GetText() const;
    void SetText( string const &Text );
    inline void SetPatParser(PatParser const &patParser);
    bool Matches(MLabel const &mlabel, size_t startULabel, bool backtrack);
    inline bool isValid();
    inline string getErrMsg();
    Pattern* Clone();

   // algebra support functions
    static Word     In( const ListExpr typeInfo, const ListExpr instance,
                          const int errorPos, ListExpr& errorInfo,
                          bool& correct );
    static ListExpr Out( ListExpr typeInfo, Word value );
    static Word     Create( const ListExpr typeInfo );
    static void     Delete( const ListExpr typeInfo, Word& w );
    static void     Close( const ListExpr typeInfo, Word& w );
    static Word     Clone( const ListExpr typeInfo, const Word& w );
    static bool     KindCheck( ListExpr type, ListExpr& errorInfo );
    static int      SizeOfObj();
    static ListExpr Property();

    // other functions
    static const string BasicType();
    static const bool checkType(const ListExpr type);
    bool checkCurULabel();
    void setStartEnd();
    bool checkLabels();
    bool checkTimeRanges();
    bool checkDateRanges();
    bool checkSemanticRanges();
    bool checkConditions();
    void initializeMatching(size_t startULabel, MLabel const &ml);

  private:
    Pattern() {};
    vector<SinglePattern> getPattern();
    string text;
    PatParser patParser;
    vector<SinglePattern> s_pattern;
    SinglePattern pattern;
    size_t curULabel, maxULabel;
    int64_t firstMatchULabel, lastMatchULabel;
    ULabel ul;
    bool wildcard, result, possibleMatch;
    Interval<Instant> interval;
    string startTimeRange, endTimeRange, timeRange;
    PatEquation patEquation;
    DateTime *dt;
    DateTime startDate, endDate, duration, startTime, endTime, startDate2,
             startPatternDateTime, endPatternDateTime;
};

}