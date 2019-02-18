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



0 Header for the Streaming Operators "feed" and "collect"

*/

#pragma once
#include "Operator.h"

#include "../utility/PcPoint.h"
#include "../tcPointcloud2.h"

namespace pointcloud2{

/*
1 Feed Operator

*/
class op_feed {

    static ListExpr feedTM(ListExpr args);

    static int feedVM(Word* args, Word& result, int message,
            Word& local, Supplier s);

    std::string getOperatorSpec();

public:
    explicit op_feed() = default;
    ~op_feed() = default;

    std::shared_ptr<Operator> getOperator();


};

/*
1.1 LocalInfo class for the feed operator

*/
class FeedLocalInfo{
    size_t _pos = 0;
    size_t _countOfPoints = 0;

    pointcloud2::Pointcloud2* _pc2;
    ListExpr _resultType;
    std::shared_ptr<PcPoint> _currentPoint;
    
public:
    FeedLocalInfo(pointcloud2::Pointcloud2* pc2, ListExpr resultType) 
    : _pc2(pc2), _resultType(resultType){
        //DEBUG
        //cout << "_resultType: " << nl->ToString(_resultType) << endl;

        _countOfPoints = _pc2->getPointCount();
        _currentPoint = std::make_shared<PcPoint>();
    }
    ~FeedLocalInfo() = default;

    Tuple* getNext();
};

/*
2 Collect Operator
collectPc2: stream(tuple(...) xP xA xREF xZ1...Zn
-> pointcloud2(REF, (tuple(|Z1:t1,...,Zn:tn|)))

*/

class OPCollectPc2{

    static ListExpr collectPc2TM(ListExpr args);

    //template<class T>
    static int collectPc2VMT( Word* args, Word& result, int message,
            Word& local, Supplier s );

    //static ValueMapping importPc2FromLasVM[];

    //static int importPc2FromLasSelect(ListExpr args);

    std::string getOperatorSpec();

public:
    explicit OPCollectPc2() = default;
    ~OPCollectPc2() = default;

    std::shared_ptr<Operator> getOperator();
};


} // end of namespace pointcloud2
