
/*
----
This file is part of SECONDO.

Copyright (C) 2017, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

*/


#include "NestedList.h"
#include "AlgebraTypes.h"

namespace general_dijkstra{

   ListExpr GDTM(ListExpr args);

   ListExpr gdijkstraTM(ListExpr args);

   
   template<class T>
   int gdijkstraVMT(Word* args, Word& result, int message, 
                 Word& local, Supplier s);



   template<bool useFun>
   ListExpr minPathCostsTM(ListExpr args);


   template<class T>
   int minPathCost1VMT(Word* args, Word& result, int message, 
                    Word& local, Supplier s);


   template<class T>
   int minPathCost2VMT(Word* args, Word& result, int message, 
                    Word& local, Supplier s);


   ListExpr gbidijkstraTM(ListExpr args);
   
   template<class T>
     int gbidijkstraVMT(Word* args, Word& result, int message, 
                    Word& local, Supplier s);



   template<bool costsAsFun> ListExpr mtMinPathCostsTM(ListExpr args);

   template<class T>
   int mtMinPathCost1VMT(Word* args, Word& result, int message, 
                         Word& local, Supplier s);

   template<class T>
   int mtMinPathCost2VMT(Word* args, Word& result, int message, 
                         Word& local, Supplier s);

}

