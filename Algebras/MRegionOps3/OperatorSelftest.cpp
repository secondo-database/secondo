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

[1] Implementation of the MRegionOps3Algebra

[TOC]

1 Introduction

2 Defines and Includes

*/
#include "OperatorSelftest.h"

using namespace std;

namespace temporalalgebra {
  namespace mregionops3 {
    
    class Selftest{
    public:  
      Selftest(){
        numberOfTestsRun=0;
        numberOfTestsFailed=0;
      }// Konstruktor
    
      ~Selftest(){
      }// Destruktortor
    
      bool run(){
        
        cerr << endl;
        cerr << numberOfTestsRun << " tests run, ";
        cerr << numberOfTestsFailed << " tests failed." << endl <<endl;  
        return (numberOfTestsFailed==0);
      }// run
    private:
      void assert_(string test,  string message, bool success){
        numberOfTestsRun++;
        if(!success){
          numberOfTestsFailed++;
          cerr << "Test failed: "<< test << ": "  << message << endl;
        }// if
      }// assert_     
        
      int numberOfTestsRun;
      int numberOfTestsFailed; 
           
    }; // class Selftest
    
    // TypeMapping für den Operator 'selftest'
    ListExpr selftestTM(ListExpr args){
      string err = " no paramters expected";
      if(!nl->HasLength(args,0)){ 
        return listutils::typeError(err);
      }// if
      return listutils::basicSymbol<CcBool>();
    }// TypeMapping 
    
    // ValueMapping für den Operator 'selftest'
    int selftestVM(Word* args, Word& result, int message, Word& local, 
                  Supplier s){ 
      Selftest test;
      bool res = test.run();
      result = qp->ResultStorage(s);
      CcBool* b = (CcBool*) result.addr;    
      b->Set(true,res);
      return 0;
    }// ValueMapping
    
    // Angaben zum Operator 'selftest'
    OperatorSpec selftestSpec(
      "selftest->bool", 
      "selftest ()", 
      "Selftest for Spatial3d2 ", 
      "query selftest()"
    );

    // Operator zusammensetzen
    Operator* getSelftestPtr(){
      return new Operator(
        "selftest",
        selftestSpec.getStr(),
        selftestVM,
        Operator::SimpleSelect,
        selftestTM
      );
    }// getSelftestPtr  
    
  } // end of namespace mregionops3
} // end of namespace temporalalgebra
