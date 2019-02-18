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



*/

#include "vlr.h"

#include <cassert>

vlr*  vlr::read(std::ifstream& in, bool extended){
  vlr* r1 = new vlr();
  in.read(reinterpret_cast<char*>(&r1->reserved),2);
  in.read(r1->userId,16);
  in.read(reinterpret_cast<char*>(&r1->recordId),2);
  if(extended){
     in.read(reinterpret_cast<char*>(&r1->recordLength),8);
  } else {
     uint16_t length;
     in.read(reinterpret_cast<char*>(&length),2);
     r1->recordLength = length;
  }
  in.read(r1->description,32);
  vlr* r2=0;
  // check all known VLRs
  if(!r2) r2 = LASF_Projection_2111::create(*r1,in);
  if(!r2) r2 = LASF_Projection_2112::create(*r1,in);
  if(!r2) r2 = LASF_Projection_34735::create(*r1,in);
  if(!r2) r2 = LASF_Projection_34736::create(*r1,in);
  if(!r2) r2 = LASF_Projection_34737::create(*r1,in);
  if(!r2) {
     // jump over data record
     in.seekg(r1->recordLength,std::ios::cur);
     //std::cout << "found unknown vlr"  << std::endl;
     //r1->print(std::cout) << std::endl;
  }
  delete r1;
  if(in.good()){
     return r2;
  } else {
     delete r2;
     return 0;
  }
}


bool LASF_Projection_34735::read(std::ifstream& in){
   size_t r = 0;
   in.read(reinterpret_cast<char*>(&wKeyDirectoryVersion),2);
   r += 2;
   in.read(reinterpret_cast<char*>(&wKeyRevision),2);
   r += 2;
   in.read(reinterpret_cast<char*>(&wMinorRevision),2);
   r += 2;
   in.read(reinterpret_cast<char*>(&wNumberOfKeys),2);
   r += 2;
   for(size_t i=0;i<wNumberOfKeys;i++){
      sKeyEntry entry;
      entry.read(in, r);
      pKeys.push_back(entry);
   } 
   assert(recordLength >= r);
   size_t ignore = recordLength - r;
   if(ignore > 0 ){
      in.seekg(ignore,std::ios::cur);
   }
   return in.good();
}

