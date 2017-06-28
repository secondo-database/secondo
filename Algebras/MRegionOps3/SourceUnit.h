/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
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
//[x] [$\times $]
//[->] [$\rightarrow $]

[1] Implementation of the MRegionOpsAlgebra

April - November 2008, M. H[oe]ger for bachelor thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#include "PFace.h"
#include "PointVectorSegment.h"
#include "NumericUtil.h"


#ifndef SOURCEUNIT_H
#define SOURCEUNIT_H

namespace temporalalgebra {
  namespace mregionops3 {
    
/*
5 class SourceUnit

*/        
    class SourceUnit{
    public:
/*
5.1 Constructor

*/       
      SourceUnit();
/*
5.1 Destructor

*/       
      ~SourceUnit();
/*
5.2 Methods, operators and predicates

5.2.3 addPFace
    
*/        
      void addPFace(const PFace& pf);
      void addPFace(const Point3D& a, const Point3D& b, 
                    const Point3D& c, const Point3D& d);
/*
5.2.4 intersection
    
*/       
      bool intersection(SourceUnit& other, GlobalTimeValues& timeValues);    
/*
5.2.5 Operator <<
    
Print the object values to stream.

*/          
      friend std::ostream& operator <<(std::ostream& os, 
                                       const SourceUnit& unit);
/*
5.2.6 Operator ==  

*/       
      bool operator ==(const SourceUnit& unit)const; 
      
//       void createResultPfaces( ContainerPoint3D& points, 
//                                GlobalTimeValues& timeValues,
//                                std::list<ResultPfaceFactory*>& factorys);
      
    private: 
/*
5.3 Attributes

*/       
      std::vector<PFace*> pFaces;
      mmrtree::RtreeT<2, size_t> pFaceTree;  
    };// class SourceUnit  
  } // end of namespace mregionops3
} // end of namespace temporalalgebra
#endif 
// SOURCEUNIT_H