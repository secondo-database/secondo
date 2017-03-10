/*

----
This file is part of SECONDO.

Copyright (C) 2017, 
University in Hagen, 
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


//[_] [\_]

*/

#ifndef MEDGE_H
#define MEDGE_H

namespace mm2algebra{

class MEdge{

  public:
    MEdge(const int _source, const int _target, 
          const double _costs, Tuple* _info):
         source(_source), target(_target), costs(_costs),
         info(_info) {
      info->IncReference();
   }

   MEdge(const MEdge& e): source(e.source), target(e.target),costs(e.costs),
                          info(e.info){
        info->IncReference();
   }

   ~MEdge(){
      info->DeleteIfAllowed();
   }

   MEdge& operator=(const MEdge& e){
     source = e.source;
     target = e.target;
     costs = e.costs;
     if(info){
       info->DeleteIfAllowed();
     }
     info = e.info;
     info->IncReference(); 
     return *this;
   }

  int source;
  int target;
  double costs;
  Tuple* info;
};

} // end of namespace

#endif 


