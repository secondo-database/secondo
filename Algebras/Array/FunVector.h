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

March 2006, M. Spiekermann. The classes ~FunInfo~ and ~FunVector~ are offered
as Interface since they may be useful for other operator implementations outside
the ArrayAlgebra.

*/

#ifndef SEC_FunVector_H
#define SEC_FunVector_H

#include <vector>
#include "Algebra.h"

/*
1 Data Structures for Parameter Functions

The datastructures below may be useful for value mappings of operators
which work with more than one parameter function.

1.1 Class ~FunInfo~

Each object of this class contains a function (given by a Supplier object)
together with some additional information, e.g. an assigned number
(["]function-id["]) and an assigned name.

A function can be requested with given parameters. The system measures, sums up
and prints out the used CPU time of the function. The total number of function
requests is also available.

*/

class FunInfo {
  public :
    FunInfo();
    FunInfo(int, string, Supplier);
    double getTime();
    Supplier getSupplier() { return supplier; }
    const string& getName() { return name; }
    void request(Word*, int, Word&, string);
    void request(Word, Word&, string);
    void request(Word, Word, Word&, string);
    void open();
    void close();
  private :
    int no;
    string name;
    Supplier supplier;
    int timesUsed;
    double consumedTime;
    
  friend bool ::operator<(const FunInfo&, const FunInfo&);
  friend std::ostream& ::operator<<(std::ostream&, const FunInfo&);
};

/*
2.2 Class ~FunVector~

This class uses the class template ["]vector["]. Each object of the class
~FunVector~ contains a vector of ~FunInfo~ objects. The vector is initialized
with a set of functions (given by a Supplier object) and an array of function
names. After initializing the vector, a single function or all functions stored
in the vector may be requested.

The class also provides some useful methods for the implementation of the
switch- and the select algorithm.

*/
class FunVector {
 
  public:
    void load(Word, Word*, const bool doRequest=false);
    
    void requestFun(int, Word, Word&, string);
    void requestFun(int, Word, Word, Word&, string);
    void requestAll(Word, Word&, string);
    void requestAll(Word, Word, Word&, string);
    
    void open(const size_t pos)  { funInfos[pos].open(); }
    void close(const size_t pos) { funInfos[pos].close(); }
    
    void openAll()  { sendMsgForAll(OPEN); }
    void closeAll() { sendMsgForAll(CLOSE); }
    
    int getMin();
    inline FunInfo& get(const size_t pos) { return funInfos[pos]; }
    inline size_t size()                  { return funInfos.size(); }

    void writeSummary();
    void reorder();
    
  private:
    void sendMsgForAll(const int msg); 
    std::vector<FunInfo> funInfos;
    void addFunction(string, Supplier);
};

bool ::operator<( const FunInfo& f1, const FunInfo& f2 );
std::ostream& ::operator<<( std::ostream& os, const FunInfo& f );




#endif
