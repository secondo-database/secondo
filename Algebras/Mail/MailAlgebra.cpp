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

[1] MailAlgebra

This algebra provides Mail functions for different
purposes.


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

//#include <boost/regex.hpp>. 


extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

#define FILE_BUFFER_SIZE 1048576 

using namespace std;



/*

1 Operator ~sendmail~

1.1 Type Mapping for ~sendmail~

*/


ListExpr sendmailtypemap( ListExpr args )
{
  if(!nl->HasLength(args,4))
   {
    return listutils::typeError("four arguments expected");
   }

  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);
  
  if((!FText::checkType(arg1)) ||  (!FText::checkType(arg2))   
      ||  (!FText::checkType(arg3))  ||  (!FText::checkType(arg4)) )
  {
    return listutils::typeError("all  arguments must have a FText value");
  } 
  
  return nl->SymbolAtom(CcBool::BasicType());
  
   

}


 
  
/*

1.2 Value Mapping for ~sendmail~

*/
 
int sendmailVM(Word* args, Word& result,
                 int message, Word& local, Supplier s)
{
  
  
  FText* firstargsub = static_cast<FText*>(args[0].addr);
  FText* secondargsender = static_cast<FText*>(args[1].addr);  
  FText* thirdargadress = static_cast<FText*>(args[2].addr);  
  FText* fourthargmessage = static_cast<FText*>(args[3].addr);  
  
  
  result = qp->ResultStorage(s);  
  CcBool* b = static_cast<CcBool*>( result.addr );
  bool res = false;
  

  if((!firstargsub->IsDefined()) || (! secondargsender->IsDefined()) 
      || (! thirdargadress->IsDefined()) ||  (! fourthargmessage->IsDefined()) )
   { 
    b->SetDefined(false);
    return 0;
   } 
 
string sub = firstargsub->GetValue();
string sdr = secondargsender->GetValue();
string adr = thirdargadress->GetValue();
string msg = fourthargmessage->GetValue();


string subpart1 = "mail -s '";
string subpart2 = "' -r ";
string subpart3 = " <<< '";
string subpart4 = " ";
string subpart5 = "'";

string finalcall = subpart1 + sub + subpart2 + sdr + 
                   subpart4 + adr + subpart3 + msg + subpart5;

 
const char * cll1 = finalcall.c_str();
int back1 = 0; 
 
size_t at1 = sdr.find('@');
size_t at2 = adr.find('@'); 
size_t dot1 = sdr.find('.', at1 + 1);
size_t dot2 = adr.find('.', at2 + 1);
size_t count1 = std::count(sdr.begin(), sdr.end(), '@');
size_t count2 = std::count(adr.begin(), adr.end(), '@');
size_t count3 = std::count(sdr.begin(), sdr.end(), ':');
size_t count4 = std::count(adr.begin(), adr.end(), ':');
size_t count5 = std::count(sdr.begin(), sdr.end(), '/');
size_t count6 = std::count(adr.begin(), adr.end(), '/');
 
  
    
if ( ((count1 == 1) && (count2 == 1)) && (!((dot1 == string::npos) 
       || (dot2 == string::npos)) &&
      (count3 == 0) && (count4 == 0) && (count5 == 0) && (count6 == 0) ) )
       
    { 
        
     back1 = system(cll1); 
     
     if  (!(back1 == 0)) 
       { 
         b->Set(true, res);   
         return 0;
       }
    
      
      res = true;
      b->Set(true, res);   
      return 0;
    
    
    
    }
    
 else { 
        b->Set(true, res);
        return 0;  
        
      }
    
   

return 0; //never happens
 
}
  
   
    
/*
1.4 Specification  for ~sendmail~

*/

const string sendmailSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> text, text, text, text   -> bool, text </text--->"
    "<text> sendmail (text, text, text, text ) </text--->"
    "<text>Sends a mail specified to the address specified" 
    "with the third argument. The second argument" 
    "is the sender address"
    "The first argument is the subject of the mail" 
    "and the fourth is the message"
    " True is returned if there are no email adress syntax errors," 
    "false in the other case" 
    "and UNDEFINED if any error occurs.</text--->"
    "<text> query sendmail('subject', 'sender@universe2.com', " 
    "'receiver@universe.com', 'message')  </text--->"
    ") )";



 
 

/*
1.5 Operator Instance for operator ~rtf2txt~

*/




                  

Operator sendmail ( "sendmail",
                   sendmailSpec,
                   sendmailVM,
                   Operator::SimpleSelect,
                   sendmailtypemap );
         
                  
  

/*
1.6 Creating the Algebra

*/

class MailAlgebra : public Algebra
{
public:
  MailAlgebra() : Algebra()
  {    
    AddOperator( &sendmail);
  }
  ~MailAlgebra() {};
};

/*
1.7  Initialization

*/

extern "C"
Algebra*
InitializeMailAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new MailAlgebra());
}


