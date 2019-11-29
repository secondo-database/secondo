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
//characters [1] Type: [] []
//characters [2] Type: [] []
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Implementation of datatypes vector, set and multiset and operators.

[1] Using Storage Manager Berkeley DB

December 2007 Ingmar G[oe]hr, Nicolai Voget, Peter Spindler, Sascha Bergmann

[toc]

1 Collection class implementation

1.1 Overview

1.1.1 Types

This Algebra provides three different types: vector, set, multiset.

All types are implemented in the class Collection, since they are all nearly
the same.

All types can be created from any number of elements as long as the elements
are all of the same type, which has to be from Kind DATA.

Since vector, set and multisets are of Kind DATA themselves, it is possible
to create a vector of vectors of integers for example.

In a vector, the elements are saved as they are added.

In a set, every element is only saved once, so duplicates are being removed.
Also, the elements will be saved ordered, smallest element first, using the
compare function of the elements. This implies that the type of which a vector,
set or multiset should be created has to implement a compare function.

A multiset is nearly the same as a set with the difference, that an element can
be added more than once.


1.1.2 Operators

Following operators are defined:

  * create\_vector

  * create\_set

  * create\_multiset

    (t)[*] [->] "vector(t)"[1]

    (t)[*] [->] "set(t)"[1]

    (t)[*] [->] "multiset(t)"[1]

t has to be of Kind DATA.


  * insert

    set(t) [x] t [->] "set(t)"[1]

    multiset(t) [x] t [->] "multiset(t)"[1]

If set(t) already contains the element to be inserted, the result is the
original set.


  * + (append)

    vector(t) [x] t [->] "vector(t)"[1]


  * delete

    set(t) [x] t [->] "set(t)"[1]

    multiset(t) [x] t [->] "multiset(t)"[1]

Removes the given element from the set/multiset. If the set/multiset does'nt
contain the element, the result is the original set/multiset. If the multiset
contains the element n times, the result contains the element (n-1) times.


  * contains

    vector(t) [x] t [->] "bool"[1]

    set(t) [x] t [->] "bool"[1]

    multiset(t) [x] t [->] CcInt::BasicType()[1]

Returns whether the collection contains the element. If the collection is a
multiset, the count of the element is returned.


  * in

    t [x] vector(t) [->] "bool"[1]

    t [x] set(t) [->] "bool"[1]

    t [x] multiset [->] CcInt::BasicType()[1]

Same as contains.


  * union

  * intersection

  * difference

    set(t) [x] set(t) [->] "set(t)"[1]

    multiset(t) [x] multiset(t) [->] "multiset(t)"[1]


  * concat

    vector(t) [x] vector(t) [->] "vector(t)"[1]

Appends the elements of the second vector to the first vector.


  * \verb+<+ (proper subset)

  * \verb+<=+ (subset)

  * = (equals)

  * \verb+>=+ (superset)

  * \verb+>+ (proper superset)

    set(t) [x] set(t) [->] "bool"[1]

    multiset(t) [x] multiset(t) [->] "bool"[1]


  * get

    vector(t) [x] int [->] "t"[1]

Returns the nth element of vector. If n is less than 0 or greater than size(
vector), an undefined element is returned.


  * components

    vector(t) [->] "stream(t)"[1]

    set(t) [->] "stream(t)"[1]

    multiset(t) [->] "stream(t)"[1]


  * collect\_vector

  * collect\_set

  * collect\_multiset

    stream(t) [->] "vector(t)"[1]

    stream(t) [->] "set(t)"[1]

    stream(t) [->] "multiset(t)"[1]

t has to be of Kind DATA.


  * size

    vector(t) [->] CcInt::BasicType()[1]

    set(t) [->] CcInt::BasicType()[1]

    multiset(t) [->] CcInt::BasicType()[1]

Returns the number of elements contained in the collection.


  * is\_defined

    vector(t) [->] "bool"[1]

    set(t) [->] "bool"[1]

    multiset(t) [->] "bool"[1]

Necessary to difference between an empty and an undefined collection.

2 Includes

*/


//#define DEBUG
//#define DEBUGHEAD
#ifdef DEBUGHEAD
#undef DEBUGHEAD
#endif

#include "CollectionAlgebra.h"
#include "IntSet.h"

#include "ListUtils.h"
#include "Symbols.h"
#include "Stream.h"

using namespace std;

using namespace collection;
/*
3.3 Implementation of Property functions and TypeConstructors

*/
  ListExpr VectorProperty() {
#ifdef DEBUGHEAD
cout << "VectorProperty" << endl;
#endif
    return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("("+Vector::BasicType()+" real)"),
                             nl->StringAtom("(elem1 elem2 .. elem_n)"),
                             nl->StringAtom("(2.839 25.123 3.12 481.2)"),
                             nl->StringAtom("All elements must be of the"
                             "same type."))));
  }

  ListExpr SetProperty() {
#ifdef DEBUGHEAD
cout << "SetProperty" << endl;
#endif
    return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("("+Set::BasicType()+" real)"),
                             nl->StringAtom("(elem1 elem2 .. elem_n)"),
                             nl->StringAtom("(2.839 3.12 25.123 481.2)"),
                             nl->StringAtom("All elements must be of the"
                             "same type."))));
  }

  ListExpr MultisetProperty() {
#ifdef DEBUGHEAD
cout << "MultisetProperty" << endl;
#endif
    return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("("+Multiset::BasicType()+" real)"),
                             nl->StringAtom("((elem1 count1) .. "
                             "(elem_n count_n))"),
                             nl->StringAtom("((2.839 2) (3.12 1) (25.123 1))"),
                             nl->StringAtom("All elements must be of the"
                             "same type."))));
  }


  TypeConstructor vectorTC(
      Vector::BasicType(), VectorProperty,
      Collection::Out, Collection::In,
      0, 0,
      Collection::Create, Collection::Delete,
      Collection::Open, Collection::Save,
      Collection::Close, Collection::Clone,
      Collection::Cast, Collection::SizeOfObj,
      Collection::KindCheck);

  TypeConstructor setTC(
      Set::BasicType(), SetProperty,
      Collection::Out, Collection::In,
      0, 0,
      Collection::Create, Collection::Delete,
      Collection::Open, Collection::Save,
      Collection::Close, Collection::Clone,
      Collection::Cast, Collection::SizeOfObj,
      Collection::KindCheck);

  TypeConstructor multisetTC(
      Multiset::BasicType(), MultisetProperty,
      Collection::Out, Collection::In,
      0, 0,
      Collection::Create, Collection::Delete,
      Collection::Open, Collection::Save,
      Collection::Close, Collection::Clone,
      Collection::Cast, Collection::SizeOfObj,
      Collection::KindCheck);

TypeConstructor IntSetTC(
  IntSet::BasicType(),
  IntSet::Property, IntSet::Out, IntSet::In, 0,0,
  IntSet::Create, IntSet::Delete,
  IntSet::Open, IntSet::Save, IntSet::Close, IntSet::Clone,
  IntSet::Cast, IntSet::Size, IntSet::TypeCheck
);


/*
4 Implementation of operators

4.1 Implementation of contains and in

*/
  template<bool contains> ListExpr ContainsInTypeMap(ListExpr args) {

    ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
#ifdef DEBUGHEAD
cout << "ContainsTypeMapping:" << nl->ToString(args) << endl;
#endif
    string opName = (contains?"contains":"in");
    string pos1 = (contains?"first":"second");
    string pos2 = (contains?"second":"first");
    if(nl->ListLength(args)!=2) {
      ErrorReporter::ReportError("Operator " + opName + " expects a list of"
                                  + " length two.");
      return nl->TypeError();
    }
    ListExpr coll = (contains?nl->First(args):nl->Second(args));
    ListExpr elem = (contains?nl->Second(args):nl->First(args));
    if(!Collection::KindCheck(coll, errorInfo)) {
      ErrorReporter::ReportError("Operator " + opName + " expects a vector, "
                                + "set or multiset as " + pos1 + " argument.");
      return nl->TypeError();
    }
    if(!nl->Equal(nl->Second(coll), elem)) {
      ErrorReporter::ReportError("The " + pos2 + " argument of operator "
                                + opName + " has to be of the same type as the"
                                + " elements of the " + pos1 + " argument.");
      return nl->TypeError();
    }
    string type = nl->SymbolValue(nl->First(coll));
    if((type==Vector::BasicType())||(type==Set::BasicType())) {
      return nl->SymbolAtom(CcBool::BasicType());
    } else if(type==Multiset::BasicType()) {
      return nl->SymbolAtom(CcInt::BasicType());
    }
    ErrorReporter::ReportError("Operator " + opName + " expects a vector"
                                + ", set or multiset as " + pos1
                                + " argument.");
    return nl->TypeError();
  }

  template<bool contains> int ContainsInValueMap(Word* args, Word& result,
                                     int message, Word& local, Supplier s) {
#ifdef DEBUGHEAD
cout << "ContainsInValueMap" << endl;
#endif
    result = qp->ResultStorage(s);
    Collection* coll;
    Attribute* elem;
    if(contains) {
      coll = static_cast<Collection*>(args[0].addr);
      elem = static_cast<Attribute*>(args[1].addr);
    } else {
      elem = static_cast<Attribute*>(args[0].addr);
      coll = static_cast<Collection*>(args[1].addr);
    }

    if(!coll->IsDefined()){
      (static_cast<Attribute*>(result.addr))->SetDefined(false);
      return 0;
    }

    int contained = coll->Contains(elem);
    if(coll->GetMyCollType()==collection::multiset) {
      (static_cast<CcInt*>(result.addr))->Set(true, contained);
    } else {
      (static_cast<CcBool*>(result.addr))->Set(true, (contained==1));
    }
    return 0;
  }

  struct containsInfo : OperatorInfo {

    containsInfo()
    {
      name      = "contains";
      signature = Vector::BasicType() + "(t) x t -> " + CcBool::BasicType();
      appendSignature(Set::BasicType() + "(t) x t -> " + CcBool::BasicType());
      appendSignature(Multiset::BasicType() + "(t) x t -> "+CcInt::BasicType());
      syntax    = "_ contains _";
      meaning   = "Contains predicate.";
    }

  };

  struct inInfo : OperatorInfo {

    inInfo()
    {
      name      = "in";
      signature = "t x "+Vector::BasicType() + "(t) -> " + CcBool::BasicType();
      appendSignature("t x "+Set::BasicType() + "(t) -> "+CcBool::BasicType());
      appendSignature("t x "+Multiset::BasicType()+"(t) -> "+
                                                   CcBool::BasicType());
      syntax    = "_ in _";
      meaning   = "Inverted contains predicate.";
    }

  };

/*
4.2 Implementation of add and insert

*/
  template<bool insert> ListExpr InsertTypeMap(ListExpr args) {
    ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
#ifdef DEBUGHEAD
cout << "InsertTypeMapping:" << nl->ToString(args) << endl;
#endif
    string opName = (insert?"insert":"+");
    if(nl->ListLength(args)!=2) {
      ErrorReporter::ReportError("Operator " + opName + " expects a list of"
                                  + " length two.");
      return nl->TypeError();
    }
    ListExpr coll = nl->First(args);
    ListExpr elem = nl->Second(args);
    if(!Collection::KindCheck(coll, errorInfo)) {
      if(insert) {
        ErrorReporter::ReportError("Operator " + opName + " expects a "
                                  + "set or multiset as first argument.");
      } else {
        ErrorReporter::ReportError("Operator " + opName + " expects a "
                                  + "vector as first argument.");
      }
      return nl->TypeError();
    }
    if((nl->ListLength(coll)!=2) || !nl->Equal(nl->Second(coll), elem)) {
      ErrorReporter::ReportError("The second argument of operator "
                                + opName + " has to be of the same type as the"
                                + " elements of the first argument.");
      return nl->TypeError();
    }
    string type = nl->SymbolValue(nl->First(coll));
    if((type==Vector::BasicType())==(!insert)) {
      return coll;
    }
    if(insert) {
      ErrorReporter::ReportError("Operator 'insert' expects a "
                                "set or multiset as first argument.");
    } else {
      ErrorReporter::ReportError("Operator '+' expects a "
                                 "vector as first argument.");
    }
    return nl->TypeError();
  }

  template<bool insert> int InsertValueMap(Word* args, Word& result,
                                      int message, Word& local, Supplier s) {
#ifdef DEBUGHEAD
cout << "InsertValueMap" << endl;
#endif
    Collection* coll = static_cast<Collection*>(args[0].addr);
    Attribute* elem = static_cast<Attribute*>(args[1].addr);

    result = qp->ResultStorage(s);
    Collection* resColl = static_cast<Collection*>(result.addr);

    resColl->CopyFrom(static_cast<Attribute*>(coll));
    resColl->Insert(elem, 1);
    return 0;
  }

  struct insertInfo : OperatorInfo {

    insertInfo()
    {
      name      = "insert";
      signature = Set::BasicType() + "(t) x t -> " + Set::BasicType() + "(t)";
      appendSignature(Multiset::BasicType() + "(t) x t -> " +
                                                Multiset::BasicType() + "(t)");
      syntax    = "_ insert _";
      meaning   = "Inserts the second argument in the first.";
    }

  };

  struct addInfo : OperatorInfo {

    addInfo()
    {
      name      = "+";
      signature = Vector::BasicType() + "(t) x t -> " +
                                                  Vector::BasicType() + "(t)";
      syntax    = "_ + _";
      meaning   = "Adds the element to the vector";
    }

  };

/*
4.3 Implementation of operator create

*/
  template<CollectionType collType> ListExpr CreateTypeMap(ListExpr args) {
#ifdef DEBUGHEAD
cout << "CreateTypeMap: " << nl->ToString(args) << endl;
#endif
    string resultType;
    switch(collType) {
    case collection::vector:
        resultType = Vector::BasicType();
        break;
      case collection::set:
        resultType = Set::BasicType();
        break;
      default:
        resultType = Multiset::BasicType();
    }
    string opName = "create_" + resultType;
#ifdef DEBUG
cout << "  Statusbericht CreateTypeMap:" << endl
     << "    resultType: " << resultType << endl;
#endif
    int length = nl->ListLength(args);
    if(length==0) {
      ErrorReporter::ReportError("Operator " + opName + " needs at least one "
          + "element to insert to know the type of the elements.");
      return nl->TypeError();
    }
    ListExpr type = nl->First(args);
#ifdef DEBUG
cout << "  Statusbericht CreateTypeMap:" << endl
     << "    type: " << type << endl;
#endif
    ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
    if(!am->CheckKind(Kind::DATA(), type, errorInfo)) {
      ErrorReporter::ReportError("Operator " + opName + " expects elements of "
          + "Kind DATA.");
      return nl->TypeError();
    }
#ifdef DEBUG
cout << "  Statusbericht CreateTypeMap:" << endl
     << "    Wir sind hinter CheckKind!" << endl;
#endif
    ListExpr rest = nl->Rest(args);
    while(!nl->IsEmpty(rest)) {
      if(!nl->Equal(nl->First(rest), type)) {
        ErrorReporter::ReportError("All arguments of operator " + opName
              + " have to be of the same type.");
        return nl->TypeError();
      }
      rest = nl->Rest(rest);
    }
    return nl->TwoElemList(nl->SymbolAtom(resultType), type);
  }

  int CreateValueMap(Word* args, Word& result,
                                      int message, Word& local, Supplier s) {
#ifdef DEBUGHEAD
cout << "CreateValueMap" << endl;
#endif
    result = qp->ResultStorage(s);
    Collection* resultColl = static_cast<Collection*>(result.addr);
    int sons = qp->GetNoSons(s);
    resultColl->Clear();
    resultColl->SetDefined(true);
    int i = 0;
    while(i < sons) {
      Attribute* elem = static_cast<Attribute*>(args[i].addr);
      resultColl->Insert(elem, 1);
      i++;
    }
    resultColl->Finish();
    return 0;
  }

  struct CreateVectorInfo : OperatorInfo {

    CreateVectorInfo()
    {
      name      = "create_" + Vector::BasicType();
      signature = "t+ -> " + Vector::BasicType() + "(t)";
      syntax    = "create_" + Vector::BasicType() + "(_, _)";
      meaning   = "Creates a " + Vector::BasicType() + " of t.";
    }
  };

  struct CreateSetInfo : OperatorInfo {

    CreateSetInfo()
    {
      name      = "create_" + Set::BasicType();
      signature = "t+ -> " + Set::BasicType() + "(t)";
      syntax    = "create_" + Set::BasicType() + "(_, _)";
      meaning   = "Creates a " + Set::BasicType() + " of t.";
    }
  };

  struct CreateMultisetInfo : OperatorInfo {

    CreateMultisetInfo()
    {
      name      = "create_" + Multiset::BasicType();
      signature = "t+ -> " + Multiset::BasicType() + "(t)";
      syntax    = "create_" + Multiset::BasicType() + "(_, _)";
      meaning   = "Creates a " + Multiset::BasicType() + " of t.";
    }
  };

/*
4.4 Implementation of function collect

*/
  template<CollectionType targetType> ListExpr CollectTypeMap(ListExpr args) {
#ifdef DEBUGHEAD
cout << "CollectTypeMap: " << nl->ToString(args) << endl;
#endif
      string operatorName;
      string resultType;

      switch(targetType){
      case collection::vector:
        operatorName = "collect_vector";
        resultType = Vector::BasicType();
        break;
      case collection::set:
        operatorName = "collect_set";
        resultType = Set::BasicType();
        break;
      case collection::multiset:
        operatorName = "collect_multiset";
        resultType = Multiset::BasicType();
        break;
      default:
        operatorName = "";
      }

      const string errMsg = "Operator " + operatorName
                                        + " expects (stream DATA)";

      ListExpr argStream;
      ListExpr argType;
      ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

      if ( nl->ListLength(args) == 1 )
      {
        argStream = nl->First(args);

        if ( !nl->IsAtom(argStream) && nl->ListLength(argStream) == 2)
        {
            argType = nl->Second(argStream);

            if ( nl->IsEqual(nl->First(argStream), Symbol::STREAM())
               && am->CheckKind(Kind::DATA(), argType, errorInfo) ){
                return nl->TwoElemList(
                                   nl->SymbolAtom(resultType),
                                   argType);
            }
        }
      }
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();

  }

  template<CollectionType targetType> int CollectValueMap(
                                     Word* args, Word& result,
                                     int message, Word& local, Supplier s) {

#ifdef DEBUGHEAD
cout << "CollectValueMap" << endl;
#endif
    result = qp->ResultStorage(s);
    Collection* resColl = static_cast<Collection*>(result.addr);
    resColl->Clear();
    resColl->SetDefined(true);
    Attribute* elemToInsert;
    Word elem = SetWord(Address(0));

    qp->Open(args[0].addr);
    qp->Request(args[0].addr, elem);

    while ( qp->Received(args[0].addr) )
    {
        elemToInsert = static_cast<Attribute*>(elem.addr);
        resColl->Insert(elemToInsert, 1);
        elemToInsert->DeleteIfAllowed();
        qp->Request(args[0].addr, elem);
    }
    resColl->Finish();
    qp->Close(args[0].addr);

    return 0;
  }

  struct collectSetInfo : OperatorInfo {

    collectSetInfo()
    {
      name      = "collect_set";
      signature = Symbol::STREAM() + "(t) -> " + Set::BasicType() + "(t)";
      syntax    = "_ collect_set";
      meaning   = "Collects the stream elements into a new set";
    }

  };

  struct collectMultisetInfo : OperatorInfo {

    collectMultisetInfo()
    {
      name      = "collect_multiset";
      signature = Symbol::STREAM() + "(t) -> " + Multiset::BasicType() + "(t)";
      syntax    = "_ collect_multiset";
      meaning   = "Collects the stream elements into a new multiset";
    }

  };

  struct collectVectorInfo : OperatorInfo {

    collectVectorInfo()
    {
      name      = "collect_vector";
      signature = Symbol::STREAM() + "(t) -> " + Vector::BasicType() + "(t)";
      syntax    = "_ collect_vector";
      meaning   = "Collects the stream elements into a new vector";
    }

  };

/*
4.5 Implementation of operator components

*/
  ListExpr ComponentsTypeMap(ListExpr args) {

#ifdef DEBUGHEAD
cout << "ComponentsTypeMap" << endl;
#endif
      const string errMsg = "Operator components expects (vector DATA)"
                                   " or (set DATA)"
                                   " or (multiset DATA)";

      ListExpr argCollection;
      ListExpr argType;
      ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

      if ( nl->ListLength(args) == 1 )
      {
        argCollection = nl->First(args);

        if ( !nl->IsAtom(argCollection) && nl->ListLength(argCollection) == 2)
        {
            argType = nl->Second(argCollection);

            if (
                (nl->IsEqual(nl->First(argCollection), Vector::BasicType()) ||
                nl->IsEqual(nl->First(argCollection), Set::BasicType()) ||
                nl->IsEqual(nl->First(argCollection), Multiset::BasicType()))
               && am->CheckKind(Kind::DATA(), argType, errorInfo) ){
                return nl->TwoElemList(
                                   nl->SymbolAtom(Symbol::STREAM()),
                                   argType);
            }
        }
      }
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();

  }

  struct ComponentsLocalInfo
  {
    int componentIndex;
    int componentCount;
  };

  int ComponentsValueMap( Word* args, Word& result,
                          int message, Word& local, Supplier s) {

#ifdef DEBUGHEAD
cout << "ComponentsValueMap" << endl;
#endif
      ComponentsLocalInfo *linfo;
      Collection* coll = (Collection*)args[0].addr;

      switch( message )
        {
        case OPEN:
          linfo = new ComponentsLocalInfo;
          linfo->componentIndex = 0;
          if(coll->GetNoUniqueComponents()>0) {
            linfo->componentCount = coll->GetComponentCount(0);
          } else {
            linfo->componentCount = 0;
          }
          local = SetWord(linfo);
          return 0;
        case REQUEST:
          if ( local.addr == 0 ){
            result.addr = 0;
            return CANCEL;
          }

          linfo = (ComponentsLocalInfo*)local.addr;
          if (linfo->componentCount <= 0){
            if(linfo->componentIndex + 1 >= coll->GetNoUniqueComponents()){
              result.addr = 0;
              return CANCEL;
            }else{
              linfo->componentIndex++;
              linfo->componentCount =
                            coll->GetComponentCount(linfo->componentIndex);
            }
          }

          result = SetWord(coll->GetComponent(linfo->componentIndex));
          linfo->componentCount--;
          return YIELD;

        case CLOSE:
          if ( local.addr != 0 )
            {
              linfo = ( ComponentsLocalInfo*) local.addr;
              delete linfo;
              local = SetWord(Address(0));
            }
          return 0;
        }
      return -1; // should not be reached
  }

  struct componentsInfo : OperatorInfo {

    componentsInfo()
    {
      name      = "components";
      signature = Vector::BasicType() + "(t) -> " + Symbol::STREAM() + "(t)";
      appendSignature(Set::BasicType() + "(t) -> " + Symbol::STREAM() + "(t)");
      appendSignature(Multiset::BasicType() + "(t) -> "
                                                    + Symbol::STREAM() + "(t)");
      syntax    = "components(_)";
      meaning   = "Takes the elements from the Collection into a stream";
    }

  };

namespace collection {
/*
4.6 Implementation of operator get

*/
  ListExpr GetTypeMap(ListExpr args) {

#ifdef DEBUGHEAD
cout << "GetTypeMap" << endl;
#endif

      const string errMsg = "Operator get expects (vector DATA) x int";

      ListExpr argCollection;
      ListExpr argIndex;
      ListExpr argType;
      ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

      if ( nl->ListLength(args) == 2 )
      {
        argCollection = nl->First(args);
        argIndex = nl->Second(args);

        if(!(nl->IsAtom(argIndex) && nl->IsEqual(argIndex, CcInt::BasicType())))
        {
            ErrorReporter::ReportError(errMsg);
            return nl->TypeError();
        }


        if ( !nl->IsAtom(argCollection) && nl->ListLength(argCollection) == 2)
        {
            argType = nl->Second(argCollection);

            if (
                (nl->IsEqual(nl->First(argCollection), Vector::BasicType()))
               && am->CheckKind(Kind::DATA(), argType, errorInfo) ){
                return argType;
            }
        }
      }
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();

  }

  int GetValueMap( Word* args, Word& result,
                       int message, Word& local, Supplier s) {

#ifdef DEBUGHEAD
cout << "GetValueMap" << endl;
#endif
    Collection* sourceColl = static_cast<Collection*>(args[0].addr);
    CcInt* index = static_cast<CcInt*>( args[1].addr );
    int indexVal = index->GetIntval();

    result = qp->ResultStorage(s);
    Attribute* resAttribute = static_cast<Attribute*>(result.addr);

    if(sourceColl->GetNoComponents() <= indexVal || indexVal < 0){
        ((Attribute*)result.addr)->SetDefined(false);
    }else{
      Attribute* elem = sourceColl->GetComponent(indexVal);
      resAttribute->CopyFrom(elem);
      elem->DeleteIfAllowed(true);
    }

    return 0;
  }

  struct getInfo : OperatorInfo {

    getInfo()
    {
      name      = "get";
      signature = Vector::BasicType() + "(t) x int -> t";
      syntax    = "get( _, _ )";
      meaning   = "Gets a component from the vector and index or undefined"
                    " if the index is invalid";
    }

  };

/*
4.7 Implementation of operator delete

*/
  ListExpr DeleteTypeMap(ListExpr args) {

#ifdef DEBUGHEAD
cout << "DeleteTypeMap" << endl;
#endif
      const string errMsg = "Operator 'deleteelem' "
                            " expects (set DATA) x DATA"
                            " or (multiset DATA) x DATA";

      ListExpr argCollection;
      ListExpr argCollectionType;
      ListExpr argDeleteType;
      ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

      if ( nl->ListLength(args) == 2 )
      {
        argCollection = nl->First(args);
        argDeleteType = nl->Second(args);

        if (!nl->IsAtom(argCollection) &&
            nl->ListLength(argCollection) == 2)
        {
            argCollectionType = nl->Second(argCollection);

            if (
                (nl->IsEqual(nl->First(argCollection), Set::BasicType()) ||
                nl->IsEqual(nl->First(argCollection), Multiset::BasicType()))
               && am->CheckKind(Kind::DATA(), argCollectionType, errorInfo)
               && nl->Equal(argCollectionType, argDeleteType)){
                return argCollection;
            }
        }
      }
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
  }

  int DeleteValueMap( Word* args, Word& result,
                       int message, Word& local, Supplier s) {

#ifdef DEBUGHEAD
  cout << "DeleteValueMap" << endl;
#endif
    Collection* sourceColl = static_cast<Collection*>(args[0].addr);
    Attribute* elemToDelete = static_cast<Attribute*>( args[1].addr );

    result = qp->ResultStorage( s );
    Collection* resColl = static_cast<Collection*>(result.addr);
    resColl->Clear();

    if(!sourceColl->IsDefined()){
      resColl->SetDefined(false);
    }else{
      for(int i = 0; i < sourceColl->GetNoUniqueComponents(); i++){
        int componentCount = sourceColl->GetComponentCount(i);
        Attribute* elem = sourceColl->GetComponent(i);
        if(elem->Compare(elemToDelete) == 0){
          componentCount--;
        }
        if(componentCount > 0){
          resColl->Insert(elem, componentCount);
        }
        if(elem) {
           elem->DeleteIfAllowed(true);
        }
      }
      resColl->Finish();
    }
    return 0;
  }

  struct deleteInfo : OperatorInfo {

    deleteInfo()
    {
      name      = "deleteelem";
      signature = Set::BasicType() + "(t) x t -> " + Set::BasicType() + "(t)";
      appendSignature(Multiset::BasicType() + "(t) x t -> "
                                            + Multiset::BasicType() + "(t)");
      syntax    = "deleteelem( _, _ )";
      meaning   = "Deletes a component one time from the set or multiset"
                   " if it is present in the collection";
    }

  };

/*
4.8 Implementation of operator concat

*/
  ListExpr ConcatTypeMap(ListExpr args) {

#ifdef DEBUGHEAD
cout << "ConcatTypeMap" << endl;
#endif
      const string errMsg = "Operator concat expects (vector DATA)"
                                   " x (vector DATA)";

      ListExpr argCollection1;
      ListExpr argCollection2;
      ListExpr argCollectionType;

      ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

      if ( nl->ListLength(args) == 2 )
      {
        argCollection1 = nl->First(args);
        argCollection2 = nl->Second(args);

        if (!nl->IsAtom(argCollection1) &&
            nl->ListLength(argCollection1) == 2)
        {
            argCollectionType = nl->Second(argCollection1);

            if (
                nl->IsEqual(nl->First(argCollection1), Vector::BasicType())
               && am->CheckKind(Kind::DATA(), argCollectionType, errorInfo)
               && nl->ToString(argCollection1)
                                == nl->ToString(argCollection2)){
                return argCollection1;
            }
        }
      }
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
  }

  int ConcatValueMap( Word* args, Word& result,
                       int message, Word& local, Supplier s) {

#ifdef DEBUGHEAD
cout << "ConcatValueMap" << endl;
#endif
    Collection* vector1 = static_cast<Collection*>(args[0].addr);
    Collection* vector2 = static_cast<Collection*>(args[1].addr);

    result = qp->ResultStorage( s );
    Collection *resVector = static_cast<Collection*>(result.addr);
    resVector->CopyFrom(static_cast<Attribute*>(vector1));

    for(int eCnt = 0; eCnt < vector2->GetNoUniqueComponents(); eCnt++){
        Attribute* elem = vector2->GetComponent(eCnt);
        resVector->Insert(elem, 1);
        elem->DeleteIfAllowed(true);
    }
    resVector->Finish();
    return 0;
  }

  struct concatInfo : OperatorInfo {

    concatInfo()
    {
      name      = "concat";
      signature = Vector::BasicType() + "(t) x "
                + Vector::BasicType() + "(t) -> " + Vector::BasicType() + "(t)";
      syntax    = "_ _ concat";
      meaning   = "Concatenates two vectors to a new one";
    }

  };

/*
4.9 Implementation of operators union, intersection and difference

*/
  enum MathSetOperationType {unionOp, intersectionOp, differenceOp};

  template<MathSetOperationType opType>
  ListExpr MathSetOperationTypeMap(ListExpr args) {
    string opName;

#ifdef DEBUGHEAD
cout << "MathSetTypeMap" << endl;
#endif
    switch(opType){
    case unionOp:
        opName = "union";
        break;
    case intersectionOp:
        opName = "intersection";
        break;
    case differenceOp:
        opName = "difference";
        break;
    default:
        break;
    }

    const string errMsg = "Operator " + opName + " expects (set DATA)"
                                   " x (set DATA) or"
                                   " (multiset DATA) x (multiset DATA)";


    ListExpr argCollection1;
    ListExpr argCollection2;
    ListExpr argCollectionType;

    ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

    if ( nl->ListLength(args) == 2 )
    {
        argCollection1 = nl->First(args);
        argCollection2 = nl->Second(args);

        if (!nl->IsAtom(argCollection1) &&
            nl->ListLength(argCollection1) == 2)
        {
            argCollectionType = nl->Second(argCollection1);

            if (
                (nl->IsEqual(nl->First(argCollection1), Set::BasicType())
                 || nl->IsEqual(nl->First(argCollection1),
                                Multiset::BasicType()))
               && am->CheckKind(Kind::DATA(), argCollectionType, errorInfo)
               && nl->ToString(argCollection1)
                                == nl->ToString(argCollection2)){
                return argCollection1;
            }
        }
      }
      ErrorReporter::ReportError(errMsg);
      return nl->TypeError();
  }

  template<MathSetOperationType opType>
  int MathSetOperationValueMap( Word* args, Word& result,
                      int message, Word& local, Supplier s) {
#ifdef DEBUGHEAD
    cout << "MathSetValueMap" << endl;
#endif
    Collection* coll1 = static_cast<Collection*>(args[0].addr);
    Collection* coll2 = static_cast<Collection*>(args[1].addr);

    result = qp->ResultStorage( s );
    Collection* resColl = static_cast<Collection*>(result.addr);
    resColl->Clear();

    if( !coll1->IsDefined() || !coll2->IsDefined() ){
      resColl->SetDefined( false );
      return 0;
    }

    int insertCnt;

    int noColl1Components = coll1->GetNoUniqueComponents();
    int noColl2Components = coll2->GetNoUniqueComponents();

    int elementIdx1 = 0;
    int elementIdx2 = 0;

    bool coll1Ended = (noColl1Components <= elementIdx1);
    bool coll2Ended = (noColl2Components <= elementIdx2);

    bool restore1 = !coll1Ended;
    bool restore2 = !coll2Ended;

    Attribute* elem1 = 0;
    Attribute* elem2 = 0;
    while(!coll1Ended && !coll2Ended){
      if(restore1) {
        elem1 = coll1->GetComponent(elementIdx1);
        restore1 = false;
      }
      if(restore2) {
        elem2 = coll2->GetComponent(elementIdx2);
        restore2 = false;
      }

      int compareRes = elem1->Compare(elem2);

      switch(compareRes){
        case -1:
          switch(opType){
            case unionOp:
              resColl->Insert(elem1,coll1->GetComponentCount(elementIdx1));
              elem1->Print(cout) << endl;
              break;
            case differenceOp:
              resColl->Insert(elem1,coll1->GetComponentCount(elementIdx1));
              break;
            default:
              break;
          }
          elementIdx1++; restore1 = true;
          break;
        case 0:
          switch(opType){
            case unionOp:
              resColl->Insert(elem1,  coll1->GetComponentCount(elementIdx1)
                                    + coll2->GetComponentCount(elementIdx2));
              elem1->Print(cout) << endl;
              break;
            case intersectionOp:
              insertCnt =
                    coll1->GetComponentCount(elementIdx1) >
                              coll2->GetComponentCount(elementIdx2) ?
                              coll2->GetComponentCount(elementIdx2) :
                              coll1->GetComponentCount(elementIdx1);
              resColl->Insert(elem1, insertCnt);
              break;
            case differenceOp:
              insertCnt = coll1->GetComponentCount(elementIdx1) -
                        coll2->GetComponentCount(elementIdx2);
              if(insertCnt > 0){
                resColl->Insert(elem1, insertCnt);
              }
              break;
            default:
              break;
          }
          elementIdx1++; restore1 = true;
          elementIdx2++; restore2 = true;
          break;
        case 1:
          switch(opType){
          case unionOp:
            resColl->Insert(elem2, coll2->GetComponentCount(elementIdx2));
            elem2->Print(cout) << endl;
            break;
          default:
            break;
          }
          elementIdx2++;
          restore2 = true;
          break;
        default:
          break;
      }
      coll1Ended = (noColl1Components <= elementIdx1);
      coll2Ended = (noColl2Components <= elementIdx2);
      if(elem1 && restore1) {
        elem1->DeleteIfAllowed(true);
        elem1 = 0;
      }
      if(elem2 && restore2) {
        elem2->DeleteIfAllowed(true);
        elem2 = 0;
      }
    }
    if(elem1) {
       elem1->DeleteIfAllowed(true);
    }
    if(elem2) {
       elem2->DeleteIfAllowed(true);
    }
    for(int iCnt1 = elementIdx1;
                            iCnt1 < coll1->GetNoUniqueComponents(); iCnt1++){
      elem1 = coll1->GetComponent(iCnt1);
      switch(opType){
        case unionOp:
        case differenceOp:
          resColl->Insert(elem1, coll1->GetComponentCount(iCnt1));
          elem1->Print(cout) << endl;
          break;
        default:
          break;
      }
      elem1->DeleteIfAllowed(true);
    }
    for(int iCnt2 = elementIdx2;
                            iCnt2 < coll2->GetNoUniqueComponents(); iCnt2++){
      elem2 = coll2->GetComponent(iCnt2);
      switch(opType){
        case unionOp:
          resColl->Insert(elem2, coll2->GetComponentCount(iCnt2));
          elem2->Print(cout) << endl;
          break;
        default:
          break;
        }
        elem2->DeleteIfAllowed(true);
    }
    resColl->Finish();
    return 0;
  }

  struct unionInfo : OperatorInfo {

    unionInfo()
    {
      name      = "union";
      signature = Set::BasicType() + "(t) x " + Set::BasicType() + "(t) -> "
                                                    + Set::BasicType() + "(t)";
      appendSignature(Multiset::BasicType() + "(t) x "
                                            + Multiset::BasicType() + "(t) -> "
                                            + Multiset::BasicType() + "(t)");
      syntax    = "_ union _";
      meaning   = "assigns the union-operation on two sets or multisets";
    }

  };

  struct intersectionInfo : OperatorInfo {

    intersectionInfo()
    {
      name      = "intersection";
      signature = Set::BasicType() + "(t) x " + Set::BasicType() + "(t) -> "
                                                    + Set::BasicType() + "(t)";
      appendSignature(Multiset::BasicType() + "(t) x " + Multiset::BasicType()
                                            + "(t) -> "
                                            + Multiset::BasicType() + "(t)");
      syntax  = "intersection( _, _)";
      meaning = "assigns the intersection-operation on two sets or multisets";
    }

  };

  struct differenceInfo : OperatorInfo {

    differenceInfo()
    {
      name      = "difference";
      signature = Set::BasicType() + "(t) x " + Set::BasicType() + "(t) -> "
                                                   + Set::BasicType() + "(t)";
      appendSignature(Multiset::BasicType() + "(t) x " + Multiset::BasicType() +
                                  "(t) -> "  + Multiset::BasicType() + "(t)");
      syntax    = "difference( _, _)";
      meaning   = "assigns the difference-operation on two sets or multisets";
    }

  };

/*
4.10 Implementation of operation size

*/
int sizeFun(Word* args, Word& result, int message, Word& local, Supplier s){
  Collection* co = (Collection*)args[0].addr;
  result = qp->ResultStorage(s);
  ((CcInt*)result.addr)->Set(co->IsDefined(), co->GetNoComponents());
return 0;
}

ListExpr sizeTypeMap(ListExpr args){
if( nl->ListLength(args) > 0)
  {
    ListExpr arg1 = nl->First(args);

    if (!nl->IsAtom(arg1) && (nl->IsEqual(nl->First(arg1),
       Set::BasicType()) || nl->IsEqual(nl->First(arg1), Multiset::BasicType())
       || nl->IsEqual(nl->First(arg1), Vector::BasicType()))) {
      return nl->SymbolAtom(CcInt::BasicType());
    }
  }

  return nl->SymbolAtom(Symbol::TYPEERROR());

}


struct sizeInfo : OperatorInfo {

  sizeInfo() : OperatorInfo()
  {
    name      = "size";
    signature = Set::BasicType() + " -> " + CcInt::BasicType();
    appendSignature(Multiset::BasicType() + " -> " + CcInt::BasicType());
    appendSignature(Vector::BasicType() + " -> " + CcInt::BasicType());
    syntax    = "size( _ )";
    meaning   = "Number of contained objects";
  }

};

/*
4.11 Implementation of operators <, <=, >, >= and =

*/
int ltFun(Word* args, Word& result, int message, Word& local, Supplier s){
  bool r = true;
  int tmp;
  Collection* co = (Collection*)args[0].addr;
  Collection* cl = (Collection*)args[1].addr;
  result = qp->ResultStorage(s);
  tmp = co->Compare(cl);
  if(tmp == 0){
    r = false;
  }
  else if(tmp == 1)
    r = false;
  else
    r = true;
  ((CcBool*)result.addr)->Set(true, r);
  return 0;
}

ListExpr compareTypeMap(ListExpr args){
  if (nl->IsAtom(args) || nl->ListLength(args) != 2){
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (    !nl->IsAtom(arg1)
       && (    nl->IsEqual(nl->First(arg1), Set::BasicType())
            || nl->IsEqual(nl->First(arg1), Multiset::BasicType())
            || nl->IsEqual(nl->First(arg1), Vector::BasicType())
          )
       && nl->Equal(arg1, arg2)
     ) {
    return nl->SymbolAtom(CcBool::BasicType());
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

struct ltInfo : OperatorInfo {

  ltInfo() : OperatorInfo()
  {
    name      = "<";
    signature = Set::BasicType() + " x " + Set::BasicType() + " -> "
                                                        + CcBool::BasicType();
    appendSignature(Multiset::BasicType() + " x " + Multiset::BasicType()
                                              + " -> " + CcBool::BasicType());
    appendSignature(Vector::BasicType() + " x " + Vector::BasicType() + " -> "
                                                        + CcBool::BasicType());
    syntax    = " _ < _ ";
    meaning   = "Object 1 Smaller Than Object 2";
  }

};

int eqFun(Word* args, Word& result, int message, Word& local, Supplier s){
  bool r = true;
  int tmp;
  Collection* co = (Collection*)args[0].addr;
  Collection* cl = (Collection*)args[1].addr;
  result = qp->ResultStorage(s);
  tmp = co->Compare(cl);
  if(tmp == 0){
    r = true;
  }
  else if(tmp == 1)
    r = false;
  else
    r = false;
  ((CcBool*)result.addr)->Set(true, r);
  return 0;
}

struct eqInfo : OperatorInfo {

  eqInfo() : OperatorInfo()
  {
    name      = "=";
    signature = Set::BasicType() + " x " + Set::BasicType() + " -> "
                                                        + CcBool::BasicType();
    appendSignature(Multiset::BasicType() + " x " + Multiset::BasicType()
                                                + " -> " + CcBool::BasicType());
    appendSignature(Vector::BasicType() + " x " + Vector::BasicType()
                                                + " -> " + CcBool::BasicType());
    syntax    = " _ = _ ";
    meaning   = "Object 1 equals Object 2";
  }

};

int neFun(Word* args, Word& result, int message, Word& local, Supplier s){
  bool r = true;
  int tmp;
  Collection* co = (Collection*)args[0].addr;
  Collection* cl = (Collection*)args[1].addr;
  result = qp->ResultStorage(s);
  tmp = co->Compare(cl);
  if(tmp == 0){
    r = false;
  }
  else
    r = true;
  ((CcBool*)result.addr)->Set(true, r);
  return 0;
}

struct neInfo : OperatorInfo {

  neInfo() : OperatorInfo()
  {
    name      = "#";
    signature = Set::BasicType() + " x " + Set::BasicType() + " -> "
                                                          + CcBool::BasicType();
    appendSignature(Multiset::BasicType() + " x " + Multiset::BasicType()
                                                + " -> " + CcBool::BasicType());
    appendSignature(Vector::BasicType() + " x " + Vector::BasicType() + " -> "
                                                        + CcBool::BasicType());
    syntax    = " _ # _ ";
    meaning   = "Object 1 does not equal Object 2";
  }

};

int gtFun(Word* args, Word& result, int message, Word& local, Supplier s){
  bool r = true;
  int tmp;
  Collection* co = (Collection*)args[0].addr;
  Collection* cl = (Collection*)args[1].addr;
  result = qp->ResultStorage(s);
  tmp = co->Compare(cl);
  if(tmp == 0){
    r = false;
  }
  else if(tmp == 1)
    r = true;
  else
    r = false;
  ((CcBool*)result.addr)->Set(true, r);
  return 0;
}

struct gtInfo : OperatorInfo {

  gtInfo() : OperatorInfo()
  {
    name      = ">";
    signature = Set::BasicType() + " x " + Set::BasicType() +
                                                " -> " + CcBool::BasicType();
    appendSignature(Multiset::BasicType() + " x " + Multiset::BasicType() +
                                                " -> " + CcBool::BasicType());
    appendSignature(Vector::BasicType() + " x " + Vector::BasicType() +
                                                " -> " + CcBool::BasicType());
    syntax    = " _ > _ ";
    meaning   = "Object 1 Greater Than Object 2";
  }

};

int leFun(Word* args, Word& result, int message, Word& local, Supplier s){
  bool r = true;
  int tmp;
  Collection* co = (Collection*)args[0].addr;
  Collection* cl = (Collection*)args[1].addr;
  result = qp->ResultStorage(s);
  tmp = co->Compare(cl);
  if(tmp == 0){
    r = true;
  }
  else if(tmp == 1)
    r = false;
  else
    r = true;
  ((CcBool*)result.addr)->Set(true, r);
  return 0;
}

struct leInfo : OperatorInfo {

  leInfo() : OperatorInfo()
  {
    name      = "<=";
    signature = Set::BasicType() + " x " + Set::BasicType() + " -> "
                                                        + CcBool::BasicType();
    appendSignature(Multiset::BasicType() + " x " + Multiset::BasicType()
                                                + " -> " + CcBool::BasicType());
    appendSignature(Vector::BasicType() + " x " + Vector::BasicType()
                                                + " -> " + CcBool::BasicType());
    syntax    = " _ <= _ ";
    meaning   = "Object 1 Smaller equal Object 2";
  }

};

int geFun(Word* args, Word& result, int message, Word& local, Supplier s){
//cout << "Wir sind in der ge Funktion 0" << endl;
  bool r = true;
  int tmp;
  Collection* co = (Collection*)args[0].addr;
  Collection* cl = (Collection*)args[1].addr;
  result = qp->ResultStorage(s);
  tmp = co->Compare(cl);
  if(tmp == 0){
    r = true;
  }
  else if(tmp == 1)
    r = true;
  else
    r = false;
  ((CcBool*)result.addr)->Set(true, r);
  return 0;
}

struct geInfo : OperatorInfo {

  geInfo() : OperatorInfo()
  {
    name      = ">=";
    signature = Set::BasicType() + " x " + Set::BasicType() + " -> "
                                                          + CcBool::BasicType();
    appendSignature(Multiset::BasicType() + " x " + Multiset::BasicType()
                                                + " -> " + CcBool::BasicType());
    appendSignature(Vector::BasicType() + " x " + Vector::BasicType() + " -> "
                                                        + CcBool::BasicType());
    syntax    = " _ >= _ ";
    meaning   = "Object 1 Greater equal Object 2";
  }

};

/*
Operators over intset

*/
ListExpr collect_intsetTM(ListExpr args){
   if(!nl->HasLength(args,2)){
     return listutils::typeError("2 arguments required");
   }
   if(!Stream<CcInt>::checkType(nl->First(args))){
     return listutils::typeError("first argument is not a stream(int)");
   }
   if(!CcBool::checkType(nl->Second(args))){
     return listutils::typeError("second argument is not a bool");
   }
   return listutils::basicSymbol<IntSet>();
}


int collect_intsetVM(Word* args, Word& result, int message, 
                     Word& local, Supplier s){
   CcBool* IgnoreUndef = (CcBool*) args[1].addr;
   bool ig = IgnoreUndef->IsDefined() && IgnoreUndef->GetValue();
   result = qp->ResultStorage(s);
   IntSet* res = (IntSet*) result.addr;
   Stream<CcInt> stream(args[0]);
   std::set<int> v;
   CcInt* elem;
   stream.open();
   while( (elem = stream.request()) != nullptr ){
       if(!elem->IsDefined()){
          if(!ig){
             res->clear();
             res->SetDefined(false);
          }
          elem->DeleteIfAllowed();
          stream.close();
          return 0;
       } else {
          v.insert(elem->GetValue());
       }
       elem->DeleteIfAllowed();
   } 
   stream.close();
   res->setTo(v);
   return 0;
}

OperatorSpec collect_intsetSpec(
   "stream(int) x bool -> intset",
   " _ collect_intset[_] ",
   "Collects all incoming integers into an intset. "
   "If the second arguemnt is true, undefined values "
   "inside the stream are ignored. If it is false, "
   "the result will be undefined in case of undefined "
   " stream elements",
   "query intstream(1,877) collect_intset[TRUE]"
);

Operator collect_intsetOp(
    "collect_intset",
    collect_intsetSpec.getStr(),
    collect_intsetVM,
    Operator::SimpleSelect,
    collect_intsetTM
);


ListExpr sizeTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("1 arg expected");
  } 
  if(!IntSet::checkType(nl->First(args))){
    return listutils::typeError("intset expected");
  }
  return listutils::basicSymbol<CcInt>();
}

int sizeVM(Word* args, Word& result, int message, 
           Word& local, Supplier s){

   result = qp->ResultStorage(s);
   CcInt* res = (CcInt*) result.addr;
   IntSet* arg = (IntSet*) args[0].addr;
   if(!arg->IsDefined()){
      res->SetDefined(false);
   } else {
      res->Set(true, arg->getSize());
   }
   return 0;
}

OperatorSpec sizeSpec(
  "intset -> int",
  "size(_)",
  "Returns the number of elements of an intset.",
  "query size(is1)"
);

Operator sizeOp(
  "size",
  sizeSpec.getStr(),
  sizeVM,
  Operator::SimpleSelect,
  sizeTM
);

ListExpr containsTM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("2 args expected");
  }
  if(!IntSet::checkType(nl->First(args)) ||
     !CcInt::checkType(nl->Second(args))){
    return listutils::typeError("intset x int expected");
  }
  return listutils::basicSymbol<CcBool>();
}


int containsVM(Word* args, Word& result, int message, 
               Word& local, Supplier s){
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  IntSet* is = (IntSet*) args[0].addr;
  CcInt*  a = (CcInt*) args[1].addr;
  if(!is->IsDefined() || !a->IsDefined()){
    res->SetDefined(false);
  } else {
    res->Set(true, is->contains(a->GetValue()));
  }
  return 0;
}

OperatorSpec containsSpec(
  "intset x int -> bool ",
  " _ contains _ ",
  "Checkes whether an intset contains a given value ",
  "query is1 contains 23"
);

Operator containsOp(
   "contains",
   containsSpec.getStr(),
   containsVM,
   Operator::SimpleSelect,
   containsTM
);


ListExpr feedISTM(ListExpr args){
  if(!nl->HasLength(args,1)){
     return listutils::typeError("1 arg expected");
  }
  if(!IntSet::checkType(nl->First(args))){
     return listutils::typeError("intset expecetd");
  }
  return Stream<CcInt>::wrap(listutils::basicSymbol<CcInt>());
}


class feedISInfo{
  public:
     feedISInfo(IntSet* _is): is(_is), pos(0){
        end = is->IsDefined()?is->getSize():0;
     }

     CcInt* next(){
         if(pos>=end){
             return nullptr;
         }
         return new CcInt(true, is->get(pos++));
     }

  private:
      IntSet* is;
      size_t pos;
      size_t end; 
};

int feedISVM(Word* args, Word& result, int message, 
               Word& local, Supplier s){

  feedISInfo* li = (feedISInfo*) local.addr;
  switch(message) {
     case OPEN: if(li) delete li;
                local.addr = new feedISInfo( (IntSet*) args[0].addr);
                return 0;
     case REQUEST: result.addr = li?li->next():0;
                   return result.addr?YIELD:CANCEL;
     case CLOSE : if(li){
                     delete li;
                     local.addr = 0;
                  }
                  return 0;
  }
  return -1;
}

OperatorSpec feedISSpec(
  "intset -> stream(int)",
  " _ feed ",
  "feedIS the content of an intset into a stream",
  "query is1 feed count"
);

Operator feedISOp(
  "feedIS",
  feedISSpec.getStr(),
  feedISVM,
  Operator::SimpleSelect,
  feedISTM

);


/*
General Type mapping for all set operations.

*/

ListExpr setOpsTM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("2 args expected");
  }
  if(!IntSet::checkType(nl->First(args))
     ||!IntSet::checkType(nl->Second(args))){
    return listutils::typeError("intset x intset expected");
  }
  return listutils::basicSymbol<IntSet>();
}

/*
Classes for different operators

*/
class UnionIS{
  public:
     static void compute(const IntSet& s1, const IntSet& s2,
                     IntSet& result){
          result = s1.add(s2);
     }
};

class MinusIS{
  public:
     static void compute(const IntSet& s1, const IntSet& s2,
                     IntSet& result){
          result = s1.minus(s2);
     }
};

class IntersectionIS{
  public:
     static void compute(const IntSet& s1, const IntSet& s2,
                     IntSet& result){
          result = s1.intersection(s2);
     }
};

class SDiffIS{
  public:
     static void compute(const IntSet& s1, const IntSet& s2,
                     IntSet& result){
          result = s1.sdiff(s2);
     }
};

/*
General value mapping for set operations.

*/
template<class OP>
int setOpsVM(Word* args, Word& result, int message, 
               Word& local, Supplier s){

   result = qp->ResultStorage(s);
   IntSet* a1 = (IntSet*) args[0].addr;
   IntSet* a2 = (IntSet*) args[1].addr;
   IntSet* res = (IntSet*) result.addr;
   OP::compute(*a1,*a2,*res);
   return 0;
}


OperatorSpec getSetOpSpec(const std::string& opname, bool infix){
   std::string syntax = infix ? "_ " + opname + " _" : opname + "(_ , _)";
   std::string ex = infix ? "query is1 " + opname + " is2"
                          : "query " + opname +"(is1, is2)";
   OperatorSpec res( "intset x intset -> intset",
                     syntax,
                     "computes the " + opname + " of two intsets ",
                     ex);
   return res;
}

Operator unionOp2(
   "union",
   getSetOpSpec("union",true).getStr(),
   setOpsVM<UnionIS>,
   Operator::SimpleSelect,
   setOpsTM
);

Operator minusOp(
   "minus",
   getSetOpSpec("minus",true).getStr(),
   setOpsVM<MinusIS>,
   Operator::SimpleSelect,
   setOpsTM
);

Operator intersectionOp2(
   "intersection",
   getSetOpSpec("intersection", false).getStr(),
   setOpsVM<IntersectionIS>,
   Operator::SimpleSelect,
   setOpsTM
);


Operator sdiffOp(
   "sdiff",
   getSetOpSpec("sdiff", true).getStr(),
   setOpsVM<SDiffIS>,
   Operator::SimpleSelect,
   setOpsTM
);


/*
~minCommon~

*/
ListExpr minCommonTM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("2 arguments expected");
  }
  if(   !IntSet::checkType(nl->First(args)) 
     || !IntSet::checkType(nl->Second(args))) {
    return listutils::typeError("intset x intset expected");
  }
  return listutils::basicSymbol<CcInt>();
}

int minCommonVM(Word* args, Word& result, int message, 
                Word& local, Supplier s){
   result = qp->ResultStorage(s);
   IntSet* a1 = (IntSet*) args[0].addr;
   IntSet* a2 = (IntSet*) args[1].addr;
   CcInt* res = (CcInt*) result.addr;
   int value;
   bool intersects = a1->minCommon(*a2,value);
   res->Set(intersects,value);
   return 0;
}

OperatorSpec minCommonSpec(
  "intset x intset -> int",
  "minCommon(_,_)",
  "Computes the minimum common element of both sets. "
  "If such an element does not exist, an undefined value "
  "is returned.",
  "query minCommon(is1,is2)"
);

Operator minCommonOp(
  "minCommon",
  minCommonSpec.getStr(),
  minCommonVM,
  Operator::SimpleSelect,
  minCommonTM
);




/*
5 Implementation of class CollectionAlgebra, registration of TypeConstructors
and operators

*/
class CollectionAlgebra : public Algebra {
    public:
    CollectionAlgebra() : Algebra() {
      AddTypeConstructor(&vectorTC);
      AddTypeConstructor(&setTC);
      AddTypeConstructor(&multisetTC);
      AddTypeConstructor(&IntSetTC);

      vectorTC.AssociateKind(Kind::DATA());
      setTC.AssociateKind(Kind::DATA());
      multisetTC.AssociateKind(Kind::DATA());
      IntSetTC.AssociateKind(Kind::DATA());

      AddOperator(containsInfo(), ContainsInValueMap<true>,
                  ContainsInTypeMap<true>);
      AddOperator(inInfo(), ContainsInValueMap<false>,
                  ContainsInTypeMap<false>);
      AddOperator(insertInfo(), InsertValueMap<true>, InsertTypeMap<true>);
      AddOperator(addInfo(), InsertValueMap<false>, InsertTypeMap<false>);
      AddOperator(CreateVectorInfo(), CreateValueMap,
                  CreateTypeMap<collection::vector>);
      AddOperator(CreateSetInfo(), CreateValueMap,
                  CreateTypeMap<collection::set>);
      AddOperator(CreateMultisetInfo(), CreateValueMap,
                  CreateTypeMap<collection::multiset>);
      AddOperator(collectSetInfo(), CollectValueMap<collection::set>,
                  CollectTypeMap<collection::set>);
      AddOperator(collectMultisetInfo(), CollectValueMap<collection::multiset>,
                  CollectTypeMap<collection::multiset>);
      AddOperator(collectVectorInfo(), CollectValueMap<collection::vector>,
                  CollectTypeMap<collection::vector>);
      AddOperator(componentsInfo(), ComponentsValueMap,
                  ComponentsTypeMap);
      AddOperator(getInfo(), GetValueMap,
                  GetTypeMap);
      AddOperator(deleteInfo(), DeleteValueMap,
                  DeleteTypeMap);
      AddOperator(concatInfo(), ConcatValueMap,
                  ConcatTypeMap);
      AddOperator(unionInfo(), MathSetOperationValueMap<unionOp>,
                  MathSetOperationTypeMap<unionOp>);
      AddOperator(intersectionInfo(), MathSetOperationValueMap<intersectionOp>,
                  MathSetOperationTypeMap<intersectionOp>);
      AddOperator(differenceInfo(), MathSetOperationValueMap<differenceOp>,
                  MathSetOperationTypeMap<differenceOp>);
      AddOperator( sizeInfo(), sizeFun, sizeTypeMap );
      AddOperator( eqInfo(), eqFun, compareTypeMap );
      AddOperator( gtInfo(), gtFun, compareTypeMap );
      AddOperator( ltInfo(), ltFun, compareTypeMap );
      AddOperator( geInfo(), geFun, compareTypeMap );
      AddOperator( leInfo(), leFun, compareTypeMap );
      AddOperator( neInfo(), neFun, compareTypeMap );

      AddOperator(&collect_intsetOp);
      AddOperator(&sizeOp);
      AddOperator(&containsOp);
      AddOperator(&feedISOp);
      AddOperator(&unionOp2);
      AddOperator(&minusOp);
      AddOperator(&intersectionOp2);
      AddOperator(&sdiffOp);
      AddOperator(&minCommonOp);

    }
    ~CollectionAlgebra() {};
  };
} // end namespace collection
/*
6 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime (if it is built as a dynamic link library). The name
of the initialization function defines the name of the algebra module. By
convention it must start with "Initialize<AlgebraName>".

*/
extern "C"
Algebra* InitializeCollectionAlgebra(NestedList* nlRef, QueryProcessor* qpRef) {
  return new (collection::CollectionAlgebra);
}


namespace Vector{
  const string BasicType() {return "vector"; };
  const bool checkType(ListExpr list){
     return collection::Collection::checkType(list, BasicType());
  }
}

namespace Set{
  const string BasicType() {return "set"; };
  const bool checkType(ListExpr list){
     return collection::Collection::checkType(list, BasicType());
  }
}

namespace Multiset{
  const string BasicType() {return "multiset"; };
  const bool checkType(ListExpr list){
     return collection::Collection::checkType(list, BasicType());
  }
}


