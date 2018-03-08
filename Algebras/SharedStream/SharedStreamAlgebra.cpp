
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

 //paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
 //paragraph [10] Footnote: [{\footnote{] [}}]
 //[->] [$\rightarrow$]
 //[TOC] [\tableofcontents]
 //[_] [\_]

*/
#include "Attribute.h"          // implementation of attribute types
#include "Algebra.h"            // definition of the algebra
#include "NestedList.h"         // required at many places
#include "QueryProcessor.h"     // needed for implementing value mappings
#include "AlgebraManager.h"     // e.g., check for a certain kind
#include "Operator.h"           // for operator creation
#include "StandardTypes.h"      // priovides int, real, string, bool type
#include "Algebras/FText/FTextAlgebra.h"
#include "Symbols.h"            // predefined strings
#include "ListUtils.h"          // useful functions for nested lists
#include "Algebras/Stream/Stream.h"             // wrapper for secondo streams
//#include "GenericTC.h"          // use of generic type constructors




#include "LogMsg.h"             // send error messages

#include "Tools/Flob/DbArray.h"  // use of DbArrays


#include "Algebras/Relation-C++/RelationAlgebra.h"           // use of tuples
#include "Algebras/Distributed2/ErrorWriter.h"
#include "Algebras/Distributed2/DArray.h"

#include <math.h>               // required for some operators
#include <stack>
#include <limits>

#include "SocketIO.h"

#include <iostream>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/lexical_cast.hpp>

/*
 0.5 Global Variables

 Secondo uses some variables designed as singleton pattern. For accessing these
 global variables, these variables have to be declared to be extern:

*/

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;

/*
 0.6 Namespace

 Each algebra file defines a lot of functions. Thus, name conflicts may arise
 with function names defined in other algebra modules during compiling/linking
 the system. To avoid these conflicts, the algebra implementation should be
 embedded into a namespace.

*/

namespace sharedstream {




    ListExpr provideTupleTypeTM(ListExpr args) {

          // check number of arguments
        if (!nl->HasLength(args, 2)) {
            return listutils::typeError("wrong number of arguments");
        }
         //first argument must be of type stream
        if (!Stream<Tuple>::checkType(nl->First(args))) {
            return listutils::typeError("stream  x int expected");
        }

         // second argument must be of type integer
        if (!CcInt::checkType(nl->Second(args))) {
            return listutils::typeError("stream x int expected");
        }

         // create the result type (stream)

        return nl->First(args);
    }



    class provideTupleTypeLI {
 //constructor: initializes the class with port and tupleType
    public:
        provideTupleTypeLI(int _port, std::string _tupleType) {
            port = _port;
            tupleType = _tupleType;
            listener = Socket::CreateGlobal("localhost",
                                            boost::lexical_cast<string>
                                                    (port));
            if (listener->IsOk()) {
                running = true;
                t1 = new boost::thread(&provideTupleTypeLI::runInThread, this);
            } else {
                running = false;
                t1 = 0;
                delete listener;
                listener = 0;
            }
        }

         // destructor
        ~provideTupleTypeLI() {
            if (running) {
                running = false;
                listener->CancelAccept();
                t1->join();
                delete t1;
                delete listener;
            }
        }
        void runInThread() {
            while (running) {
                Socket *srv = listener->Accept();
                if (srv) {
                    if (srv->IsOk()) {
                        std::iostream &io = srv->GetSocketStream();
                        io << "TupelType: " << tupleType << endl;
                        srv->Close();
                    }
                    delete srv;
                }
            }
        }


    private:
        Socket *listener;
        bool running;
        string tupleType;
        int port;
        boost::thread *t1;

    }; //end provideTupleTypeLI class




    int provideTupleTypeVM(Word *args, Word &result, int message,
                           Word &local, Supplier s) {
        provideTupleTypeLI *li = (provideTupleTypeLI *) local.addr;
        switch (message) {
            case OPEN : {
                qp->Open(args[0].addr);

                if (li) {
                    delete li;
                    local.addr = 0;
                }

                CcInt *port = (CcInt *) args[1].addr;
                if (!port->IsDefined()) {
                    return 0;
                }
                string tupleType = nl->ToString(nl->Second(qp->GetType(s)));
                local.addr = new provideTupleTypeLI(port->GetValue(),
                                                    tupleType);
                return 0;

            }
            case REQUEST: {

                qp->Request(args[0].addr, result);
                return qp->Received(args[0].addr) ? YIELD : CANCEL;
            }
            case CLOSE: {
                if (li) {
                    delete li;
                    local.addr = 0;
                }
                qp->Close(args[0].addr);
                return 0;
            }
        }
        return 0;
    }



    OperatorSpec provideTupleTypeSpec(
            " stream x int -> stream",
            " _ provideTupleType[_] ",
            "The incoming stream is pass-through. As an side effect "
                    "a server is started on the specified port providing the"
                    "tuple type of the stream.",
            " query plz feed provideTupleType[4211] count"
    );


    Operator provideTupleTypeOp(
            "provideTupleType",
            provideTupleTypeSpec.getStr(),
            provideTupleTypeVM,
            Operator::SimpleSelect,
            provideTupleTypeTM
    );




    class SharedStreamAlgebra : public Algebra {
    public:
        SharedStreamAlgebra() : Algebra() {


            AddOperator(&provideTupleTypeOp);

        }
    };


}//end namespace

/*
9 Initialization of the Algebra

This piece of code returns a new instance of the algebra.


*/
extern "C"
Algebra *
InitializeSharedStreamAlgebra(NestedList *nlRef,
                              QueryProcessor *qpRef) {
    return new sharedstream::SharedStreamAlgebra;
}
