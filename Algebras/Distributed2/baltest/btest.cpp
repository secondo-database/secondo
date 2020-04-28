/*

----
This file is part of SECONDO.

Copyright (C) 2020,
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

*/



#include "../BalancedCollect.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <istream>


using namespace std;

void trim(std::string& str, std::string whiteSpaces=" \r\n\t")  {
    std::string::size_type pos = str.find_last_not_of(whiteSpaces);
    if(pos != std::string::npos) {
      str.erase(pos + 1);
      pos = str.find_first_not_of(whiteSpaces);
      if(pos != std::string::npos){
         str.erase(0, pos);
      }    
    } else {
     str.erase(str.begin(), str.end());
    }    
}

int main(int argc, char** argv){

   if(argc!=4){
     cout << "3 arguments required" << endl;
     return 1;
   }

   vector<uint32_t> slotsizes;
   ifstream in(argv[1]);
   string line;
   uint64_t completeSize = 0;
   while(in.good() && !in.eof()){
       getline(in,line);
       trim(line);
       if(line.length()>0){
         int s = atoi(line.c_str());
         slotsizes.push_back(s); 
         completeSize += s; 
       }
   }  
   in.close();
   int noWorkers = atoi(argv[2]);
   string mode(argv[3]);
   if(mode!="simple" && mode!="complete"){
     cout << "third argument mut be one of simple or complete" << endl;
     return 1;
   }

   bool simple = mode=="simple" ;

   
   cout << "slots     : "  << slotsizes.size() << endl;
   cout << "noWorkers : " << noWorkers << endl; 
   cout << " mode     : " << (simple?"simple":"complete") << endl;
   
   vector<uint32_t> res = getMapping(slotsizes,noWorkers,simple);


   if(res.size() != slotsizes.size()){
     cout << "Error, size is different";
     cout << "slotsizes "  << slotsizes.size() << endl;
     cout << "res size " << res.size();
     return 1;
   } 

   vector<uint32_t> workerSizes;
   vector<vector<pair<int,uint32_t> > > worker;
   for(int i=0;i<noWorkers; i++){
      workerSizes.push_back(0);
      vector<pair<int,uint32_t> > w;
      worker.push_back(w);
   }


   size_t maxSize = 0;
   for(size_t i=0;i<res.size();i++){
      uint32_t slotsize = slotsizes[i];
      uint32_t w = res[i];
      workerSizes[w]+=slotsize;
      if(workerSizes[w] > maxSize){
        maxSize = workerSizes[w];
      }
      pair<int, uint32_t> slot(i,slotsize);
      worker[w].push_back(slot);
   }

   double avgSize = (double) completeSize/noWorkers; 

   for(size_t i=0;i<worker.size();i++){
     cout << "worker : " << i << " : " ;
     vector<pair<int,uint32_t> >& v = worker[i];
     for(size_t i = 0; i< v.size();i++ ){
       if(i>0) cout << " + ";
       cout << v[i].second <<"(" << v[i].first <<")";
     }
     cout << " = " << workerSizes[i] << endl; 
   }

   uint32_t minWS = maxSize;
   for(size_t i=0;i<workerSizes.size(); i++){
      if(workerSizes[i] < minWS) minWS = workerSizes[i];
   }

   cout << endl << endl;
   cout << "minimum worker size " << minWS << endl;
   cout << "maximum worker size " << maxSize << endl;
   cout << "average worker size " << avgSize << endl;
   cout << "utilization  : " << ((double)avgSize/(double)maxSize) << endl;

  

}
