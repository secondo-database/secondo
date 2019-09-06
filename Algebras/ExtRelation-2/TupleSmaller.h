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

*/

#pragma once

#include <vector>
#include "Algebras/Relation-C++/RelationAlgebra.h"

class TupleSmaller{
public:
   TupleSmaller(const std::vector<std::pair<int,bool> >& _p): positions(_p){}

   bool operator()(Tuple* t1, Tuple* t2) const{
     for(size_t i=0;i<positions.size();i++){
        int c = t1->GetAttribute(positions[i].first)->
                             Compare(t2->GetAttribute(positions[i].first));
        if(c < 0) return  positions[i].second;
        if(c > 0) return !positions[i].second;
     }
     // tuples are equal
     return false;
   }
   private:
     std::vector<std::pair<int, bool> > positions;
};


class pc{
       public:
         pc(TupleSmaller _comp):comp(_comp){}
         bool operator()(const std::pair<Tuple*,TupleFileIterator*>& a, 
                    const std::pair<Tuple*, TupleFileIterator*>& b){
           return comp(a.first,b.first);
       }
       private:
         TupleSmaller comp;
};

