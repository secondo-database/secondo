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

#include "TupleSmaller.h"
#include "mmheap.h"


class OutHeap{
  public:

   OutHeap(std::vector<TupleFile*> _files, 
          mmheap::mmheap<Tuple*, TupleSmaller>* h,
          TupleSmaller _comp):
       files(_files), h2(0), heap(h), comp(_comp), lastFromFile(0), 
       lastFromHeap(0),nextRes(0){
      init();
   }

   ~OutHeap();

   inline Tuple* next(){
     Tuple* res = nextRes;
     retrieveNext();
     return res;
   }


   private:
       std::vector<TupleFile*> files;
       mmheap::mmheap< std::pair<Tuple*, TupleFileIterator*>, pc >* h2;
       mmheap::mmheap<Tuple*, TupleSmaller>* heap;
       TupleSmaller comp;
       Tuple* lastFromFile;
       Tuple* lastFromHeap;
       Tuple* nextRes;

    void init();

    void retrieveNext();

    void retrieveNextFromFile();

    void retrieveNextFromHeap();

};
