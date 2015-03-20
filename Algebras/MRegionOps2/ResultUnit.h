/*
----
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, 
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]

[1] Implementation with exakt dataype

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

[TOC]

1 Helper classes and methods

1.1 Some forward declaration of helper methods

1 class declarations

2 Defines and Includes

*/
#include "MSegment.h"
namespace mregionops2 {

class ResultUnit {

public:

/*

1.1 Constructor

*/
    ResultUnit();

/*

1.1 Methods

1.1.1 StartBulkLoad

*/
    inline void StartBulkLoad() {

        index = 0;
    }
/*

1.1.2 AddSegment

*/
    inline void AddSegment(MSegment ms) {

    }
/*

1.1.3 EndBulkLoad

*/
    void EndBulkLoad(bool merge);

/*

1.1.4 ConvertToURegionEmb

*/
URegionEmb2* ConvertToURegionEmb(DbArray<MSegmentData>* segments,
Interval<Instant> interval) const;
/*

1.1.5 IsEmpty

*/
    inline bool IsEmpty() const {

        return msegments.size() == 0;
    }

private:
/*

2.1 Private methods

*/
    void AddMSegmentData(URegionEmb* uregion,
                         DbArray<MSegmentData>* segments,
                         MSegmentData& dms) const;

    static bool Less(const MSegment& ms1, const MSegment& ms2) {
        return ms1.LessByMedianHS(ms2);
    }

    static bool LogicLess(const MSegment& ms1, const MSegment& ms2) {
        return ms1.LogicLess(ms2);
    }
    const Interval<Instant> interval;
    vector<MSegment> msegments;
    unsigned int index;
};
}
