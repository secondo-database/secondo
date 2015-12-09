/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"u]
//[ae] [\"a]
//[_] [\_]
//[TOC] [\tableofcontents]

[1] ConversionAlgebra

This algebra provides conversion functions for different
data formats.


*/

/*

[TOC]

1 Overview

This file contains the implementation import / export operators.

2 Defines, includes, and constants

*/


#include <cmath>
#include <stack>
#include <limits>
#include <sstream>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>

#include <fcntl.h>
#include "SocketIO.h"

#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "SecondoSystem.h"
#include "FTextAlgebra.h"
#include "SpatialAlgebra.h"
#include "DateTime.h"
#include "TopOpsAlgebra.h"
#include "BinaryFileAlgebra.h"
#include "../../Tools/Flob/DbArray.h"
#include "Symbols.h"
#include "FileSystem.h"
#include "ListUtils.h"

#include "version.h"
#include "DbVersion.h"
#include "RegionTools.h"
#include "NMEAImporter.h"
#include "Stream.h"


extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

#define FILE_BUFFER_SIZE 1048576 

using namespace std;





/*

1  Operator ~rtf2txt~

This operator converts a given Ftext into a text  FileSystem::DeleteFileOrFolder(fileNameS)

1.1 Type Mapping for ~rtf2txt~

*/

ListExpr rtf2txttypemap( ListExpr args )
{
  if(!nl->HasLength(args,1))
   {
    return listutils::typeError("one arguments expected");
   }

  ListExpr arg1 = nl->First(args);
  
  if(!FText::checkType(arg1))
  {
    return listutils::typeError("argument must be a FText value");
  } 
  
  return nl->SymbolAtom(FText::BasicType());
  
   

}






/*

1.2 Value Mapping for ~rtf2txt~

*/
#ifndef SECONDO_WIN32
 

int rtf2txtVM(Word* args, Word& result,
                 int message, Word& local, Supplier s)
{

  result = qp->ResultStorage(s);
  FText* res = static_cast<FText*>(result.addr);  
  FText* objrtf = static_cast<FText*>(args[0].addr);
  
  if(!objrtf->IsDefined())
   {
    res->SetDefined(false);
    return 0;
   } 
   
 string str = objrtf->GetValue();
 ofstream outfile ("tempfile.rtf", ios::out);  
 outfile.write(str.c_str(), str.length());
 outfile.close();
  
 //string call = "unrtf --text --quiet tempfile.rtf > tempfile.txt";     
 string call1 = "unrtf --text --nopict tempfile.rtf > tempfile1.txt";
 string call2_1 = "cat tempfile1.txt | awk 'NR == 1, ";
 string call2_2 = "/-----------------/ { next } { print }' > tempfile2.txt"; 
 string call2 = call2_1 + call2_2;
 
 
 const char * cll1 = call1.c_str();
 const char * cll2 = call2.c_str();
 int back1 = system(cll1);  
 int back2 = system(cll2);  
 
 
 
 if (!(back1 == 0) || !(back2 == 0)) 
 {
   res->SetDefined(false);   
   return 0;
   
 }
 
 ifstream  in ("tempfile2.txt"); 
 in.seekg (0, in.end);
 size_t len = in.tellg();
 in.seekg (0, in.beg); 
  
 char * buffer = new char [len];
 buffer[len-1] = 0; 
 in.read (buffer,len-1);
 in.close(); 
 FileSystem::DeleteFileOrFolder("tempfile.rtf");
 FileSystem::DeleteFileOrFolder("tempfile1.txt");
 FileSystem::DeleteFileOrFolder("tempfile2.txt");
 res->Set(true, buffer);
 delete[] buffer;
 
    
 return 0;  
} 
  
  

 
  
  
//ValueMapping rtf2txtvaluemap[] = {rtf2txtVM<FText>};




/*
1.3 Selection Function for ~rtf2txt~

*/








/*
1.4 Specification  for ~rtf2txt~

*/



const string rtf2txtSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> text -> text </text--->"
    "<text> rtf2txt ( text ) </text--->"
    "<text>Converts a given rtf text into a sinple text" 
    " using the unrtf linux tool. Returns the simple text, if this succeeds "
    "and UNDEFINED if any error occurs.</text--->"
    "<text> query rtf2txt('anyrtftext')  </text--->"
    ") )";


 
 
 

/*
1.5 Operator Instance for operator ~rtf2txt~

*/



Operator rtf2txt ( "rtf2txt",
                   rtf2txtSpec,
                   rtf2txtVM,
                   Operator::SimpleSelect,
                   rtf2txttypemap );
         
                  





#endif







   

/*
25 Creating the Algebra

*/

class ConversionAlgebra : public Algebra
{
public:
  ConversionAlgebra() : Algebra()
  {
    
    #ifndef SECONDO_WIN32
    AddOperator( &rtf2txt);
    #endif
  }
  ~ConversionAlgebra() {};
};

/*
9 Initialization

*/

extern "C"
Algebra*
InitializeConversionAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new ConversionAlgebra());
}


