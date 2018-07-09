
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
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
----

*/

/*
1 Class TupleInfo

This class stores information about a tuple required to performing the DBScan 
Algorithm.

*/

#ifndef TUPLEINFO_H
#define TUPLEINFO_H

namespace dbscan{

  class TupleInfo{
  public:
    TupleInfo():visited(false),clusterNo(-1),isSeed(false),isCore(false){}
    TupleInfo(bool v, int c): visited(v),clusterNo(c), isSeed(false),
                              isCore(false) {}
    TupleInfo(const TupleInfo& src): visited(src.visited), 
                                     clusterNo(src.clusterNo),
                                     isSeed(src.isSeed),
                                     isCore(src.isCore) {}

    TupleInfo& operator=(const TupleInfo& src){
       visited = src.visited;
       clusterNo = src.clusterNo;
       isSeed= src.isSeed; 
       isCore = src.isCore;
       return *this;
    }
 

    bool visited;
    int clusterNo; 
    bool isSeed;
    bool isCore;
  };

}

#endif


