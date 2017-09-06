/*
----
This file is part of SECONDO.

Copyright (C) 2017, Faculty of Mathematics and Computer Science, Database
Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

SECONDO is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
SECONDO; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [10] title: [{\Large \bf] [}]
//characters [1] tt: [\texttt{] [}]
//[secondo] [{\sc Secondo}]

[10] Operator Implementation of Algebra Distributed4

2017-08-14: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 Preliminary Setup

*/
#include "DTable.h"
#include "DArray.h"
#include "FTextAlgebra.h"
#include "NList.h"
#include "Operator.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "SecondoCatalog.h"
#include <vector>

namespace distributed4 {
  using std::string;
  using distributed2::DArray;
/*
2 Type Mapping Functions

These functions evaluate a ListExpr to see if it matches the expected input
type of the corresponding operator.  They return a ListExpr of the operator's
result data type.

Each Type Mapping Function has the same basic two-step structure: (1) Verify
the number of arguments. (2) Verify the type of arguments.

2.1 addWorker Type Mapping

*/
  ListExpr addWorkerTM(ListExpr args) {
    string err = "dtable x {string,text} x int x {string,text} expected";
    NList l{args};
    if(
        !l.checkLength(4, err) ||
        !l.isList(1) ||
        l.first() != DTable::BasicType() ||
        l.third() != CcInt::BasicType())
      return NList::typeError(err);
    for(NList e: {l.second(), l.fourth()})
      if(e != CcString::BasicType() && e != FText::BasicType())
        return NList::typeError(err);
    return NList(CcBool::BasicType()).listExpr();
  }
/*
2.2 inspectDArray Type Mapping

*/
  ListExpr inspectDArrayTM(ListExpr args) {
    string err = "d[f]array expected";
    NList l{args};
    if(!l.checkLength(1, err) || !l.first().isList())
      return NList::typeError(err);
    NList e{l.first().first()};
    if(e != DArray::BasicType() && e != distributed2::DFArray::BasicType())
      return NList::typeError(err);
    return NList(CcBool::BasicType()).listExpr();
  }
/*
3 Value Mapping Functions

These functions do the productive work of the corresponding operator. They
process input data (args) and post the result (result).

3.1 addWorker Value Mapping

*/
  template<class HostType, class ConfType> int addWorkerVM(Word*
      args, Word& result, int message, Word& local, Supplier s) {
    // Convert passed values.
    DTable* ds{static_cast<DTable*>(args[0].addr)};
    string host{static_cast<HostType*>(args[1].addr)->GetValue()};
    int port{static_cast<CcInt*>(args[2].addr)->GetValue()};
    string conf{static_cast<ConfType*>(args[3].addr)->GetValue()};
    result = qp->ResultStorage(s);
    CcBool* res{static_cast<CcBool*>(result.addr)};
/*
Before modifying the DTable, it is necessary to find out if this operator is
being run on a master or a supervisor. On a master, the command should first be
forwarded to the supervisor, on a supervisor, it should first be forwarded to
all inserters. Only when those operations have indicated success, may the local
DTable be modified.

*/
    auto ds_meta_name{(string)"Ds" + "_adaptive"};
    SecondoCatalog* ctlg{SecondoSystem::GetCatalog()};
    if(auto ds_meta_obj{ctlg->GetObjectValue(ds_meta_name)})
    {
      NList l{ds_meta_obj};
      cout << "debug: " << l << endl;
      cout << "object exists" << endl;
    } else {
      cout << "debug: " << ds_meta_obj << endl;
      cout << "object does not exist" << endl;
    }

/*
On the supervisor, run this command on the inserters first.
TODO: only run on the master having a connection to a supervisor and inserters.
TODO: connect to the worker, to make sure it is available.
TODO: Add the worker on all updaters, then the supervisor, and finally the
      master (this machine).
TODO: distribute the modified DArray to the supervisor and updaters.

*/
    try {
      ds->addWorker(host, port, conf);
      return 0;  //TODO
    } catch(std::runtime_error& e) {
      cmsg.error() << "addWorker failed: " << e.what() << endl;
      cmsg.send();
      res->Set(true, false);
      return 0;
    }
/*
Make the changes permanent and provide a return value.

*/
    qp->SetModified(qp->GetSon(s, 0));
    res->Set(true, true);
    return 0;
  }
/*
3.2 inspectDArray Value Mapping

*/
  int inspectDArrayVM(Word* args, Word& result, int message, Word&
      local, Supplier s) {
    DArray* da{static_cast<DArray*>(args[0].addr)};
    cout << *da << endl;
    result = qp->ResultStorage(s);
    static_cast<CcBool*>(result.addr)->Set(true, da->IsDefined());
    std::map<double,uint32_t> m; m[3] = 47; m[7] = 21;
    DTable test(m, DArray(0));
    cout << "debug: " << test << endl;
    for(int i{0}; i < 9; ++i)
      cout << "debug: test[" << i << "] = " << test.slot(i) << endl;
    test.removeWorker();
    return 0;
  }
/*
4 Selection Functions

For overloaded operators, it is necessary to choose one out of an array of
possible value mapping functions. This is accomplished by listing all possible
value mapping functions in a value mapping array and providing a selection
function that returns an index into that array, thus selecting the appropriate
value mapping function to be used. I chose to use a vector instead of an array
and pass the underlying array to the operator instance below. This way, if the
value mapping functions change, I don't have to worry about updating how many
entries are in the array in all the right places.

4.1 addWorker Selection

*/
  std::vector<ValueMapping> addWorkerArray{
    addWorkerVM<CcString,CcString>,
    addWorkerVM<CcString,FText>,
    addWorkerVM<FText,CcString>,
    addWorkerVM<FText,FText>,
  };

  int addWorkerSelect(ListExpr args) {
    NList l{args};
    int i{0};
    if(l.second() == FText::BasicType()) i += 2;
    if(l.fourth() == FText::BasicType()) i += 1;
    return i;
  }
/*
5 Operator Specifications

The "OperatorSpec"[1] class formats human-readable specification for the
operators in a consistent way. The constructor expects four or five strings in
the following order:

  * signature

  * syntax

  * meaning

  * example

  * remark (optional)

5.1 addWorker Specification

*/
  OperatorSpec addWorkerSpec{
    DTable::BasicType() + " x {" + CcString::BasicType() + "," +
      FText::BasicType() + "} x " + CcInt::BasicType() + " x {" +
      CcString::BasicType() + "," + FText::BasicType() + "} -> " +
      CcBool::BasicType(),
    "addWorker(ds, host, port, conffile)",
    "Adds a worker given by host, port, and conffile to ds. No slots are "
      "mapped to the new worker at this time, though.",
    "query addWorker([const " + DTable::BasicType() + " value (ds 4 ((\"s1\" "
      "1234 \"cfg.ini\") (\"s2\" 1234 \"cfg.ini\")))], \"snew\", 1234, "
      "\"cfg.ini\")",
  };
/*
5.2 inspectDArray Specification

*/
  OperatorSpec inspectDArraySpec{
    "d[f]array -> bool",
    "inspectDArray(arrayname)",
    "Outputs the internal structure of a DArray.",
    "query inspectDArray([const " + DArray::BasicType() + "(" +
      CcInt::BasicType() + ") value (da1 4 ((\"onehost\" 1234 \"cfg.ini\") "
      "(\"otherhost\" 1234 \"cfg.ini\")))])",
  };
/*
6 Operator Instances

The "Operator"[1] class takes care of putting together operator instances given
data for a simple or an overloaded operator. The constructor expects the
following elements:

  * name of operator ("string"[1])

  * specification of operator ("string"[1], returned by the "getStr"[1] method
    of "OperatorSpec"[1])

  * value mapping function ("ValueMapping"[1], simple operators only)

  * number of value mapping functions ("int"[1], overloaded operators only)

  * array of value mapping functions ("ValueMapping[]"[1], overloaded operators
    only)

  * selection function ("SelectFunction"[1], simple operators use
    "Operator::SimpleSelect"[1])

  * type mapping function ("TypeMapping"[1])

6.1 addWorker Instance

*/
  Operator addWorkerOp{
    "addWorker",
    addWorkerSpec.getStr(),
    static_cast<int>(addWorkerArray.size()),
    addWorkerArray.data(),
    addWorkerSelect,
    addWorkerTM,
  };
/*
6.2 inspectDArray Instance

*/
  Operator inspectDArrayOp{
    "inspectDArray",
    addWorkerSpec.getStr(),
    inspectDArrayVM,
    Operator::SimpleSelect,
    inspectDArrayTM,
  };
}
