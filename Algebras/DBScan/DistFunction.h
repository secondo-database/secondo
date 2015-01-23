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

//[_] [\_]
//characters   [1]  verbatim:  [$]  [$]
//characters   [2]  formula:  [$]  [$]
//characters   [3]  capital:  [\textsc{] [}]
//characters   [4]  teletype:  [\texttt{] [}]

1 Header file "DistFunction.h"[4]

March-October 2014, Marius Haug

1.1 Overview

This file contains the implementation of the distance function needed by the
MMMTree.
The classes ~DistCount~, ~IntDist~, ~PointDist~ and ~StringDist~ are orginally
from "MMRTreeAlgebra.cpp".

1.2 Includes

*/
#include <limits>
#include "Point.h"
#include "StandardTypes.h"
#include "StringUtils.h"
#include "TupleIdentifier.h"
#include "DistfunReg.h"
#include "PictureAlgebra.h"

namespace clusterdbscanalg
{
/*
1.3 Declarations and definition of the class ~DistCount~

*/
 class DistCount
 {
  public: 
   DistCount() { cnt = 0; }
      
   void reset() const { cnt =0; }

   size_t getCount() const{ return cnt; }

  protected:
   mutable size_t cnt;
 };
/*
1.4 Declarations and definition of the class ~IntDist~

*/
 class IntDist: public DistCount
 {
  public:

   double operator()(const pair<CcInt*,TupleId>& p1
    ,const pair<CcInt*,TupleId>& p2)
   {
    DistCount::cnt++;
    assert(p1.first);
    assert(p2.first);
  
    if(!p1.first->IsDefined() && !p2.first->IsDefined())
    {
     return 0;
    }
   
    if(!p1.first->IsDefined() || !p2.first->IsDefined())
    {
     return numeric_limits<double>::max();
    }
   
    int i1 = p1.first->GetValue();
    int i2 = p2.first->GetValue();
    int c = i1-i2;
   
    return c<0?-c:c;
   }

   ostream& print(const pair<CcInt*,TupleId>& p, ostream& o)
   {
    o << *(p.first);
    return o;
   }
 };
/*
1.5 Declarations and definition of the class ~RealDist~

*/
 class RealDist: public DistCount
 {
  public:

   double operator()(const pair<CcReal*,TupleId>& p1
    ,const pair<CcReal*,TupleId>& p2)
   {
    DistCount::cnt++;
    assert(p1.first);
    assert(p2.first);
  
    if(!p1.first->IsDefined() && !p2.first->IsDefined())
    {
     return 0;
    }
   
    if(!p1.first->IsDefined() || !p2.first->IsDefined())
    {
     return numeric_limits<double>::max();
    }
   
    int i1 = p1.first->GetValue();
    int i2 = p2.first->GetValue();
    int c = i1-i2;
   
    return c<0?-c:c;
   }

   ostream& print(const pair<CcReal*,TupleId>& p, ostream& o)
   {
    o << *(p.first);
    return o;
   }
 };
/*
1.6 Declarations and definition of the class ~PointDist~

*/
 class PointDist: public DistCount
 {
  public:
  
   double operator()(const pair<Point*,TupleId>& p1
    ,const pair<Point*,TupleId>& p2)
   {
    cnt++;
    assert(p1.first);
    assert(p2.first);
    
    if(!p1.first->IsDefined() && !p2.first->IsDefined())
    {
     return 0;
    }
    
    if(!p1.first->IsDefined() || !p2.first->IsDefined())
    {
     return numeric_limits<double>::max();
    }
    
    return p1.first->Distance(*(p2.first));
   }
     
   ostream& print(const pair<Point*,TupleId>& p, ostream& o)
   {
    o << *(p.first);
    return o;
   }
 };
/*
1.7 Declarations and definition of the class ~StringDist~

*/
 class StringDist: public DistCount
 {
  public:
  
   double operator()(const pair<CcString*,TupleId>& p1
    ,const pair<CcString*,TupleId>& p2)
   {
    cnt++;
    assert(p1.first);
    assert(p2.first);
   
    if(!p1.first->IsDefined() && !p2.first->IsDefined())
    {
     return 0;
    }
    
    if(!p1.first->IsDefined() || !p2.first->IsDefined())
    {
     return numeric_limits<double>::max();
    }
    
    return stringutils::ld(p1.first->GetValue(), p2.first->GetValue());
   }
     
   ostream& print(const pair<CcString*,TupleId>& p, ostream& o)
   {
    o << *(p.first);
    return o;
   }
 };
/*
1.8 Declarations and definition of the class ~CustomDist~

*/ 
 template<class T, class R>
 class CustomDist: public DistCount
 { 
  private:
   QueryProcessor* qp;
   Supplier fun;
 
  public:  
   void initialize(QueryProcessor* queryProcessor, Supplier function)
   {
    qp = queryProcessor;
    fun = function;
   }
   
   double operator()(const pair<T,TupleId>& p1
    ,const pair<T,TupleId>& p2)
   {
    cnt++;
    assert(p1.first);
    assert(p2.first);
   
    if(!p1.first->IsDefined() && !p2.first->IsDefined())
    {
     return 0;
    }
    
    if(!p1.first->IsDefined() || !p2.first->IsDefined())
    {
     return numeric_limits<double>::max();
    }
    
    Word funRes;
    ArgVectorPointer vector;
    vector = qp->Argument(fun);
    ((*vector)[0]).setAddr(p1.first);
    ((*vector)[1]).setAddr(p2.first);
    qp->Request(fun, funRes);
    
    R* result;
    result = (R*) funRes.addr;
        
    double c = result->GetValue();
    return c < 0 ? -c : c;
   }
     
   ostream& print(const pair<T,TupleId>& p, ostream& o)
   {
    o << *(p.first);
    return o;
   }
 };
/*
1.9 Declarations and definition of the class ~PictureDist~

*/
 class PictureDist: public DistCount
 {
  private:
   bool init;
   gta::DistfunInfo df;
     
  public:
   
   PictureDist()
   {
    init = false;
   };
   
   double operator()(const pair<Picture*,TupleId>& p1
    ,const pair<Picture*,TupleId>& p2)
   {
    cnt++;
    assert(p1.first);
    assert(p2.first);
   
    if(!p1.first->IsDefined() && !p2.first->IsDefined())
    {
     return 0;
    }
    
    if(!p1.first->IsDefined() || !p2.first->IsDefined())
    {
     return numeric_limits<double>::max();
    }
    
    if(!gta::DistfunReg::isInitialized())
    {
     gta::DistfunReg::initialize();
    }
    
    if(!init)
    {
     gta::DistDataId id = gta::DistDataReg::getId(Picture::BasicType()
      ,gta::DistDataReg::defaultName(Picture::BasicType()));
     
     df = gta::DistfunReg::getInfo(gta::DFUN_DEFAULT, id);
     
     init = true;
    }
    
    double distance;
    
    df.dist(df.getData(p1.first), df.getData(p2.first), distance);
    
    return distance;
   }
     
   ostream& print(const pair<Picture*,TupleId>& p, ostream& o)
   {
    o << *(p.first);
    return o;
   }
 };
}
