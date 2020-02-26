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

Feb. 2006 M. Spiekermann. The TestRunner is now a command option for SecondoTTY and
not an stand alone program. This saves time for linking and saves diskspace, since both
applications hav minimal differences.

*/

/*
Feb 2006, M. Spiekermann
   
1 The Main Routine

Main routine for ~SecondoBDB~ and ~SecondoCS~. This source file will be compiled twice. The
resulting object files will be used to link together ~SecondoBDB~ and ~SecondoCS~. There will
be some shell scripts which invoke the above programs with different options, e.g.

SecondoTTYBDB
SecondoPL
TestRunner

SecondoTTYCS
SecondoPLCS
TestRunnerCS
  
*/   

#include <stdexcept>

#include "License.h"
#include "TTYParameter.h"
#include "LogMsg.h"
#include "Messages.h"

using namespace std;

// forward declarations
extern int SecondoTTYMode(const TTYParameter&);
extern int SecondoTestRunner(const TTYParameter&);

#ifndef NO_OPTIMIZER
extern int SecondoPLMode(TTYParameter&);
extern int SecondoPLTTYMode(TTYParameter&);
#endif

#ifndef SEC_TTYCS
extern int SecondoServerMode( const int, const char**);
#endif


class Simple2Handler : public MessageHandler {

  public:
  virtual bool handleMsg(NestedList* nl, ListExpr list, 
                         int source __attribute__((unused))){
     #ifdef THREAD_SAFE
     boost::lock_guard<boost::mutex> guard(mtx);
     #endif

     if(!nl->HasMinLength(list,2)){
       return false;
     }
     if(!nl->IsEqual(nl->First(list),"simple2")){
        return false;
     }
     list = nl->Rest(list);
     if(nl->HasLength(list,1)){
	 list = nl->First(list);
         switch(nl->AtomType(list)){
            case SymbolType : 	 std::cout << nl->SymbolValue(list);
			         break;
            case StringType : 	 std::cout << nl->SymbolValue(list);
			         break;

            case TextType : 	 std::cout << nl->TextValue(list);
			         break;
            case IntType : 	 std::cout << nl->IntValue(list);
			         break;
            case RealType : 	 std::cout << nl->RealValue(list);
			         break;
            default : std::cout << nl->ToString(list);
         }			      
     } else {
         std::cout << nl->ToString(list);
     }
     std::cout << endl;
     return true;
  }
 
  Simple2Handler() {};
  ~Simple2Handler() {};
 
}; 



int
main( const int argc, char* argv[] )
{
  try {  

  TTYParameter tp(argc,argv);

#ifndef SEC_TTYCS
  if ( tp.isServerMode() )
    return SecondoServerMode(tp.numArgs, (const char**)tp.argValues);
#endif

  // Add message handlers
  MessageCenter* msg = MessageCenter::GetInstance();
    
  // uncomment the lines below in order to 
  // activate the example handler. The operator
  // count2 demonstrates how to send messages.
  SimpleHandler* sh = new SimpleHandler();
  msg->AddHandler(sh);
  
  Simple2Handler* s2h = new Simple2Handler();
  msg->AddHandler(s2h);

  ProgMesHandler* pmh = new ProgMesHandler();
  msg->AddHandler(pmh);

#ifdef SECONDO_PL 
  if ( tp.isPLMode() )
    return SecondoPLMode(tp);
#endif


#ifdef SECONDO_PL 
  if ( tp.isPLTTYMode() )
    return SecondoPLTTYMode(tp);
#endif

  cout << License::getStr() << endl;
  
  // Testrunner or TTY or TTYCS
  if ( !tp.CheckConfiguration() )
    return 1;
  
  if ( tp.isTestRunnerMode() )
    return SecondoTestRunner(tp);
  
  return SecondoTTYMode(tp);

  } catch (exception& e) {
    cerr << e.what() << endl;
  }
}
