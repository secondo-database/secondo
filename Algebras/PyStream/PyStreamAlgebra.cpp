/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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

#include "StandardTypes.h"
#include "QueryProcessor.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "Symbols.h"
#include "Base64.h"

#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/TupleIdentifier/TupleIdentifier.h"
#include "Stream.h"

#include "StringUtils.h"

#include "ClientSocket.h"
#include "SocketException.h"
#include <string>


extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

using namespace std;
 
namespace pyStream{

 /*
  1 Operators
  1.2 pySend Operator

  1.2.2 Type mapping function for send operators

 */


 ListExpr pySend_TM( ListExpr args )
 {

        
   if(nl->ListLength(args)!=2){
        return listutils::typeError("two arguments expected");
   }

   string err = " stream(tuple(...) x int expected";

   ListExpr stream = nl->First(args);
   ListExpr port = nl->Second(args);

   if(( !Stream<Tuple>::checkType(nl->First(stream)) )
                    || !CcInt::checkType(nl->First(port)) ){
    return listutils::typeError(err);
   }

   if(!listutils::isNumeric(nl->Second(port))) {
    return listutils::typeError(err);
   }
   if(listutils::getNumValue(nl->Second(port)) < 0
        || listutils::getNumValue(nl->Second(port)) > 65535 ) {
    return listutils::typeError("A portnumber between 0 and 65535 expected");
   }
    
        
   return nl->First(stream);
 }


 /*
  1.2.3 LocalInfo Class for Value mapping

 */


 class pySendLocalInfo {
  public:
    
   pySendLocalInfo(Word& streamArg, CcInt* arg, ListExpr type):
    stream(streamArg), type_(type)
        
   {
    try
     {
      client_socket = new ClientSocket( "localhost", arg->GetValue() );
      std::string tupleType = nl->ToString(type_)  + "\n";
      *client_socket << tupleType;
    }
    catch ( SocketException& e) {
     std::cout << "Exception was caught:" << e.description() << "\n";
    }

    stream.open();
            
  }
     
  ~ pySendLocalInfo() {
   stream.close();
  }

  Tuple* sendNext() {
            
   Tuple* t;
            
   while( ( t = stream.request() ) != 0 ){
                
            
    ListExpr l = t->Out( nl->TwoElemList( nl->Second(type_),
                                         nl->TheEmptyList() ) );
    std::string data;
    nl->WriteToString(data, l);
    data = data + "\n";
                    
    try
     {
                        
      *client_socket << data;
                                  
     }
     catch ( SocketException& e) {
      std::cout << "Exception was caught:" << e.description() << "\n";
     }
                    
    return t;
   }
   std::string fin = "quit\n";
   *client_socket << fin;
   return 0;
            
  }
            
   private:
    ClientSocket* client_socket;
    Stream<Tuple> stream;
    ListExpr type_;
 };

 /*
  1.2.4 Value mapping function for operator pySend

 */

 int pySend_VM(Word* args, Word& result, int message, Word& local, Supplier s)

 {
        
   pySendLocalInfo *li = (pySendLocalInfo *) local.addr;
   Word tuple;
   ListExpr type;
   //Tuple* tuple;
    
   switch(message)
    {
      case OPEN:
                
        qp->Open(args[0].addr);
        type = qp->GetType(qp->GetSon(s,0));
        if (li) delete li;
          
        local.addr = new pySendLocalInfo(args[0], (CcInt *) args[1].addr, type);
        return 0;

    case REQUEST:
          
        result.addr = li?li->sendNext():0;
        return result.addr?YIELD:CANCEL;

                
    case CLOSE:
                
        qp->Close(args[0].addr);
        if(li)
            {
                delete li;
                local.addr = 0;
            }
                
            return 0;
    }
        
    return 0;
  }

 /*
  1.2.5 pySend operator specification

 */

 const std::string pySend_Spec =
 "( ( \"Signature\" \"Syntax\" \"Meaning\" "
 "\"Example\" ) "
 "( "
 "<text>((stream (tuple([a1:d1, ... ,an:dn]"
 "))) x int) -> (stream (tuple([a1:d1, ... ,"
 "an:dn]))) or \n"
 "((stream T) x int) -> (stream T), "
 "for T in kind DATA.</text--->"
 "<text>_ pysend [ _ ]</text--->"
 "<text>Distributes stream of Tuples"
 "to a specific port.</text--->"
 "<text>query plz feed head[7] pysend[30000] count"
 "</text--->"
 ") )";


 /*
  1.2.6 Operator pySend

 */

 Operator pySend_Op(
    "pysend",
    pySend_Spec,
    pySend_VM,
    Operator::SimpleSelect,
    pySend_TM
 );



 /*
  1.3 Operator pyReceive
  1.3.2 TypeMapping of operator ~pyreceive~
  Expects a relation structure and a port number of int.
 
 */

 ListExpr pyReceive_TM(ListExpr args) {
   string errLen = "Two arguments expected!";

   int len = nl->ListLength(args);
   if(len!=2){
     return listutils::typeError(errLen);
   }
    
   ListExpr first = nl->First(args);
    
   std::string data1;
   nl->WriteToString(data1, first);
    
   std::string data2;
   nl->WriteToString(data2, nl->Second(args));
    
   if(!listutils::isRelDescription(nl->First(first),true) &&
        !listutils::isRelDescription(nl->First(first),false)){
     return listutils::typeError("Argument of type rel(tuple(...)) expected");
   }
    
    
    
   string err2 = "int argument representing port expected!";
   if (!CcInt::checkType(nl->First(nl->Second(args)))) {
     return listutils::typeError(err2);
   }


   if(!listutils::isNumeric(nl->Second(nl->Second(args)))) {
     return listutils::typeError(err2);
   }
    
   if(listutils::getNumValue(nl->Second(nl->Second(args))) < 0 ||
        listutils::getNumValue(nl->Second(nl->Second(args))) > 65535 ) {
     return listutils::typeError("portnumber between 0 and 65535 expected");
   }

      
   return nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()),
                                nl->Second(nl->First(first)));
   

  }


 /*
 
  1.3.3 LocalInfo fuction for Value mapping
 */


 class pyReceiveLocalInfo {
   public:
        
     pyReceiveLocalInfo(ListExpr type, CcInt* arg):
       type_(type)
       //, tupleType(0), BasicTuple(0)
     {
                
       try
        {
           client_socket = new ClientSocket( "127.0.0.1", arg->GetValue() );
                    
        }
      catch ( SocketException& e) {
        std::cout << "Exception was caught:" << e.description() << "\n";
       }
            
            
      std::string tt;
      nl->WriteToString(tt, type_);
            
      numTupleType = SecondoSystem::GetCatalog()->NumericType(type_);
      numTupleType = nl->TwoElemList( numTupleType,
                            nl->ListLength(nl->Second(numTupleType)));
            
      std::string numtt;
      nl->WriteToString(numtt, numTupleType);
                
   }
   ~pyReceiveLocalInfo() {
    }

   Tuple* getNext() {
        
      std::string msg;
      try
        {
           *client_socket >> msg;
                    
        }
      catch ( SocketException& e) {
         std::cout << "Exception was caught:" << e.description() << "\n";
      }
            
      if (!msg.empty()){
         ListExpr value;
         if (nl->ReadFromString(msg, value)){
            ListExpr errInfo;
            bool correct = false;
            Tuple* res = Tuple::In(numTupleType, value, 0, errInfo, correct);
            if (correct){
                            
               return res;
            }
                        
            res->DeleteIfAllowed();
        }
                
    }
            
            
    return 0;
 }

 private:
    
   ClientSocket* client_socket;
   ListExpr numTupleType;
   ListExpr type_;
    };




 /*
  1.3.4  ValueMapping of operator ~pyreceive~
  All real work is done by the getNext()
  function of the corresponding LocalInfo<> Class.

 */

 int pyReceive_VM(Word* args, Word& result, int message,
                     Word& local, Supplier s) {
    ListExpr type, t;
    pyReceiveLocalInfo* li = static_cast<pyReceiveLocalInfo*> (local.addr);
        
    switch(message) {

        case OPEN: {
                
            qp->Open(args[0].addr);
            type = nl->Second(GetTupleResultType(s));
            t = nl->Second(GetTupleResultType(s));
            std::string data1;
            nl->WriteToString(data1, type);
            
            std::string data2;
            nl->WriteToString(data2, t);
            
            CcInt* port = (CcInt*) args[1].addr;

            if (li) {
               delete li;
            }
            local.addr = new pyReceiveLocalInfo(type, port );
                
            return 0;
        }

        case REQUEST: {
                
            result.addr = li?li->getNext():0;
            return result.addr?YIELD:CANCEL;
            
        }

        case CLOSE: {
                
            qp->Close(args[0].addr);
            if (li) {
                delete li;
                local.addr = 0;
            }
            return 0;
        }

    }
    return 0;
  }

 /*
    1.3.5 Specification of operator ~pyReceive~

 */

  const std::string pyReceive_Spec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> Rel(tuple(x)) X (int)"
    " -> stream (tuple(x)))"
    "</text--->"
    "<text>_ pyreceive [ _ ]</text--->"
    "<text>Receives a stream of Tuples from a given port number.</text--->"
    "<text>let X = [const rel(tuple([plz: int, Ort: string])) value ()]"
    "pyreceive[30000] consume</text--->"
    ") )";

 /*
    1.3.6 Definition of operator ~pyreceive~

 */

 Operator pyReceive_Op(
    "pyreceive",
    pyReceive_Spec,
    pyReceive_VM,
    Operator::SimpleSelect,
    pyReceive_TM
 );

 /*
    2 Corresponding class for Algebra

 */

 
 class PyStreamAlgebra : public Algebra
  {
    public:
        PyStreamAlgebra() : Algebra()
        {
            AddOperator(&pySend_Op);
            pySend_Op.SetUsesArgsInTypeMapping();
       
            AddOperator(&pyReceive_Op);
            pyReceive_Op.SetUsesArgsInTypeMapping();
        }
        ~PyStreamAlgebra() {};
  };




 /*
    3 Initialization
 */

 extern "C"
    Algebra*
    InitializePyStreamAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
    {
        nl = nlRef;
        qp = qpRef;
        return (new pyStream::PyStreamAlgebra());
    }


} //end of namespace
