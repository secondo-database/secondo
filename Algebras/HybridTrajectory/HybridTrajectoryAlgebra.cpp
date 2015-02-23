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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Source File of the Hybrid Trajectory Algebra

Started  2013 , Hamza Issa\'{e}s



[TOC]

\section{Overview}
This algebra includes the operators ~by~ and ~atperiods~.

\section{Defines and Includes}

*/

#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "TemporalAlgebra.h"
#include "TemporalExtAlgebra.h"
#include "DateTime.h"
#include "Stream.h"
#include "TemporalUnitAlgebra.h"
#include "../NestedRelation/NestedRelationAlgebra.h"
#include "../SymbolicTrajectory/Algorithms.h"

#include "DivisionStream.h"
#include "HybridTrajectoryAlgebra.h"
#include <string>
#include <vector>


extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;


using namespace std;


namespace hyt {



/*

1.1 start of atInstantImplmentation

*/
ListExpr
atInstantTypeMapping( ListExpr args )
{
    if ( nl->ListLength( args ) == 2 )
    {
        ListExpr arg1 = nl->First( args ),
                 arg2 = nl->Second( args );
        if( nl->IsEqual( arg2, Instant::BasicType() ) )
        {
            if(listutils::isRelDescription2(arg1,
                    AttributeRelation::BasicType()))
            {
  ListExpr attributeList =nl->Second(nl->Second(arg1));
  ListExpr newAttributelist=nl->TheEmptyList();
  ListExpr indexList=nl->TheEmptyList();
  ListExpr lastnewAttributelist=nl->TheEmptyList();
  ListExpr lastindexList=nl->TheEmptyList();
          ListExpr outList;
          bool first=true;
          newAttributelist =nl->OneElemList(
        nl->TwoElemList(
        nl->SymbolAtom("Instant"),
        nl->SymbolAtom( Instant::BasicType())));
          lastnewAttributelist=newAttributelist;
          while (!(nl->IsEmpty(attributeList))){
    ListExpr singleAttribute =nl->First(attributeList);
    attributeList=nl->Rest(attributeList);
    string attrname=nl->SymbolValue(nl->First(singleAttribute));
    ListExpr attrtype=nl->Second(singleAttribute);
              string newattrname=attrname;
              ListExpr newattrtype=attrtype;
              int inttoappend =0;
    if( nl->IsEqual( attrtype, MBool::BasicType() ) ){
      newattrtype=nl->SymbolAtom( CcBool::BasicType() );
                  inttoappend =1;
              }
    if( nl->IsEqual( attrtype, MInt::BasicType() ) ){
      newattrtype=nl->SymbolAtom( CcInt::BasicType() );
                  inttoappend=2;
              }
              if( nl->IsEqual( attrtype, MReal::BasicType() ) ){
      newattrtype=nl->SymbolAtom( CcReal::BasicType() );
                  inttoappend=3;
              }
              if( nl->IsEqual( attrtype, MPoint::BasicType() ) ){
      newattrtype=nl->SymbolAtom( Point::BasicType() );
                  inttoappend=4;
              }
              if( nl->IsEqual( attrtype, MString::BasicType() ) ){
      newattrtype=nl->SymbolAtom( CcString::BasicType() );
                  inttoappend=5;
              }
              lastnewAttributelist =nl->Append(
                      lastnewAttributelist,
                      nl->TwoElemList(
                      nl->SymbolAtom(attrname), newattrtype));

              if(first){
          indexList=nl->OneElemList(nl->IntAtom(inttoappend));
                  lastindexList=indexList;
                  first=false;
              }
              else{
                  lastindexList =nl->Append(
                          lastindexList,
                          nl->IntAtom(inttoappend));
              }
          }
          ListExpr arelstrcut=nl->TwoElemList(
                  nl->SymbolAtom(AttributeRelation::BasicType()),
                  nl->TwoElemList(
      nl->SymbolAtom(Tuple::BasicType()), newAttributelist));
          outList =nl->ThreeElemList(
                  nl->SymbolAtom(Symbol::APPEND()),
                  nl->OneElemList(indexList),
                  arelstrcut);

          return outList;
            }
        }
    }
    return nl->SymbolAtom( Symbol::TYPEERROR() );
}


int
atInstantSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if(listutils::isRelDescription2(arg1, AttributeRelation::BasicType()))
      return 0;

  return -1;
}







int ArelAtInstant( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{



    Word elem(Address(0));

    Supplier son;
    /*Take care about memory leak*/

  AttributeRelation* arel=(AttributeRelation*) args[0].addr;

  Instant* instant=   (Instant*)args[1].addr;
  Relation* r=Relation::GetRelation( arel->getRelId() );

  ListExpr resultType=qp->GetType(s);
  ListExpr resultTupleType =
          SecondoSystem::GetCatalog()->NumericType(resultType);




  TupleType* atupleType =new TupleType(nl->Second(resultTupleType));



    Relation* outrel=new Relation(atupleType);



  AttributeRelation* outarel = (AttributeRelation*)
                              (qp->ResultStorage(s).addr);


  outarel->getTupleIds()->clean();
  outarel->setRel(outrel);
  outarel->setRelId(outrel->GetFileId());


  int tupleIndex=0;



  Tuple* atuple = new Tuple (atupleType);
/*here we can simple copy or transform then copy*/

  while (tupleIndex < arel->getTupleIds()->Size())
  {







       TupleId tid;
    arel->getTupleIds()->Get(tupleIndex, tid);

    Tuple* t = r->GetTuple(tid, false);


    atuple->PutAttribute(0,instant);
    int scanIndex=0;
    while(scanIndex<t->GetNoAttributes()){





        son = qp->GetSupplier(args[2].addr, scanIndex);
        qp->Request(son, elem);
        int type = ((CcInt*)elem.addr)->GetIntval();



        if(type==4){
          MPoint* mp=(MPoint*)t->GetAttribute(scanIndex);
          Intime<Point>* pResult = new Intime<Point>();
          mp->AtInstant(* instant, *pResult);
          atuple->PutAttribute(scanIndex+1,&pResult->value);
    }
    else if(type==2){
        MInt* mint=(MInt*)t->GetAttribute(scanIndex);
        Intime<CcInt>* pResult = new Intime<CcInt>();
        mint->AtInstant(* instant, *pResult);
        atuple->PutAttribute(scanIndex+1,&pResult->value);
    }
    else if(type==1){
        MBool* mbool=(MBool*)t->GetAttribute(scanIndex);
        Intime<CcBool>* pResult = new Intime<CcBool>();
        mbool->AtInstant(* instant, *pResult);
        atuple->PutAttribute(scanIndex+1,&pResult->value);
    }
    else if(type==3){

        MReal* mreal=(MReal*)t->GetAttribute(scanIndex);
        Intime<CcReal>* pResult = new Intime<CcReal>();
        mreal->AtInstant(* instant, *pResult);
        atuple->PutAttribute(scanIndex+1,&pResult->value);

    }
    else if(type==5){

        MString* mstring=(MString*)t->GetAttribute(scanIndex);
        Intime<CcString>* pResult = new Intime<CcString>();
        mstring->AtInstant(* instant, *pResult);
        atuple->PutAttribute(scanIndex+1,&pResult->value);
     }
    else{
        atuple->CopyAttribute(scanIndex, t, scanIndex+1);
    }
    scanIndex ++;

    }


    outrel->AppendTuple(atuple);

    outarel->Append(atuple->GetTupleId());


    t->DeleteIfAllowed();


    tupleIndex ++;
  }


    //atuple->DeleteIfAllowed();
  //atupleType->DeleteIfAllowed();
  result.setAddr(outarel);


  return 0;
}

ValueMapping AtInstantMap[] = {ArelAtInstant };




const string AtInstantSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(arl(X) instant) -> iarl(Y)</text--->"
  "<text>_ atinstant _ </text--->"
  "<text>From a arel contain multiuple moving thing  get the intime value "
  "corresponding to the temporal value at the given instant  for"
  " each attriute inside the arel(X)</text--->"
  "<text>arel1  atinstant instant1</text--->"
  ") )";

Operator temporalatinstant( "atinstant",
                             AtInstantSpec,
                             1,
                             AtInstantMap,
                             atInstantSimpleSelect,
                             atInstantTypeMapping );




/*end of atInstantImplmentation*/


/*start of atPeriodsImplmentation*/
ListExpr
AtPeriodsTypeMapping (ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );
    if( nl->IsEqual( arg2, Periods::BasicType() ) )
    {
        if(listutils::isRelDescription2(arg1,
                AttributeRelation::BasicType())){

            ListExpr attributeList =nl->Second(nl->Second(arg1));
            ListExpr indexList=nl->TheEmptyList();
            ListExpr lastindexList=nl->TheEmptyList();
            ListExpr outList;
            bool first=true;
            while (!(nl->IsEmpty(attributeList))){
                ListExpr singleAttribute =nl->First(attributeList);
                attributeList=nl->Rest(attributeList);
                ListExpr attrtype=nl->Second(singleAttribute);
                int inttoappend =0;
                if( nl->IsEqual( attrtype, MBool::BasicType() ) ){
                    inttoappend =1;
                }
                if( nl->IsEqual( attrtype, MInt::BasicType() ) ){
                    inttoappend=2;
                }
                if( nl->IsEqual( attrtype, MReal::BasicType() ) ){
                    inttoappend=3;
                }
                if( nl->IsEqual( attrtype, MPoint::BasicType() ) ){
                    inttoappend=4;
                }
                if( nl->IsEqual( attrtype, MString::BasicType() ) ){
                          inttoappend=5;
                }




                if(first){
indexList=nl->OneElemList(nl->IntAtom(inttoappend));
lastindexList=indexList;
first=false;

                }

                else{

                    lastindexList =nl->Append(
                            lastindexList,
                            nl->IntAtom(inttoappend));

                }

            }

ListExpr arelstrcut=nl->TwoElemList(
nl->SymbolAtom(AttributeRelation::BasicType()),
nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
nl->Second(nl->Second(arg1))));
outList =nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
nl->OneElemList(indexList),
arelstrcut);
//cout <<"djskdjskdjskdj"<<nl->ToString(outList)<<endl;
return outList;

    }
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}
}


int
atPeriodsSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if(listutils::isRelDescription2(arg1, AttributeRelation::BasicType()))
      return 0;

  return -1; // This point should never be reached
}


int ArelAtPeriod( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
    Word elem(Address(0));
    Supplier son;
    /*Take care about memory leak*/
    AttributeRelation* arel=(AttributeRelation*) args[0].addr;


    Periods* period=   (Periods*)args[1].addr;
    Relation* r=Relation::GetRelation( arel->getRelId() );
    ListExpr resultType=qp->GetType(s);
    ListExpr resultTupleType =
            SecondoSystem::GetCatalog()->NumericType(resultType);
    TupleType* atupleType =new TupleType(nl->Second(resultTupleType));






    Relation* outrel=new Relation(atupleType);



 AttributeRelation* outarel = (AttributeRelation*)
                             (qp->ResultStorage(s).addr);


 outarel->getTupleIds()->clean();
 outarel->setRel(outrel);
 outarel->setRelId(outrel->GetFileId());

 Tuple* atuple = new Tuple (atupleType);
    int tupleIndex=0;

    while (tupleIndex < arel->getTupleIds()->Size())
    {

        TupleId tid;
        arel->getTupleIds()->Get(tupleIndex, tid);
        Tuple* t = r->GetTuple(tid, false);
        int scanIndex=0;
        while(scanIndex<t->GetNoAttributes()){
            son = qp->GetSupplier(args[2].addr, scanIndex);
            qp->Request(son, elem);
            int type = ((CcInt*)elem.addr)->GetIntval();

            if(type==4){
                MPoint* mp=(MPoint*)t->GetAttribute(scanIndex);
                MPoint* pResult = new MPoint(0);
                mp->AtPeriods(* period, *pResult);
                atuple->PutAttribute(scanIndex,pResult);
            }
            else if(type==2){
                MInt* mint=(MInt*)t->GetAttribute(scanIndex);
                MInt* pResult = new MInt(0);
                mint->AtPeriods(* period, *pResult);
                atuple->PutAttribute(scanIndex,pResult);
            }
            else if(type==1){
                MBool* mbool=(MBool*)t->GetAttribute(scanIndex);
                MBool* pResult= new MBool(0);
                mbool->AtPeriods(* period, *pResult);
                atuple->PutAttribute(scanIndex,pResult);
            }
            else if(type==3){
                MReal* mreal=(MReal*)t->GetAttribute(scanIndex);
                MReal* pResult = new MReal(0);
                mreal->AtPeriods(* period, *pResult);
                atuple->PutAttribute(scanIndex,pResult);
            }
            else if(type==5){
MString* mstring=(MString*)t->GetAttribute(scanIndex);
                MString* pResult = new MString(0);
                mstring->AtPeriods(* period, *pResult);
                atuple->PutAttribute(scanIndex,pResult);
            }
            else{
                atuple->CopyAttribute(scanIndex, t, scanIndex);
            }
            scanIndex ++;
        }
        outrel->AppendTuple(atuple);
        outarel->Append(atuple->GetTupleId());

        t->DeleteIfAllowed();
        tupleIndex ++;


    }



    atuple->DeleteIfAllowed();
 atupleType->DeleteIfAllowed();

    result.setAddr(outarel);


    return 0;
}


const string AtPeriodSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(arl(X) atperiods ) -> iarl(X)</text--->"
  "<text>_ atperiods _ </text--->"
  "<text>From a arel contain multiuple moving thing  get the  value "
  "corresponding to the temporal value at the given periods  for"
  " each attriute inside the arel(X)</text--->"
  "<text>arel1  periods periods1</text--->"
  ") )";


ValueMapping AtPeriodsMap[] = {ArelAtPeriod };

Operator temporalatperiods( "atperiods",
                             AtPeriodSpec,
                             1,
                             AtPeriodsMap,
                             atPeriodsSimpleSelect,
                             AtPeriodsTypeMapping );




/*end of atPeriodsImplmentation*/


/*start of implmenation of by operator*/

ListExpr ByTypeMapping(ListExpr args) {
  int noargs = nl->ListLength(args);
  string errmsg = "Expected {mT1} x {mT2}, where T in "
               "{point, int, real, bool, string, label, labels, place, places}";
  if (noargs != 2) {
    return listutils::typeError(errmsg);
  }
  set<string> supportedArgTypes;
  supportedArgTypes.insert(MPoint::BasicType());
  supportedArgTypes.insert(MReal::BasicType());
  supportedArgTypes.insert(MInt::BasicType());
  supportedArgTypes.insert(MBool::BasicType());
  supportedArgTypes.insert(MString::BasicType());
  supportedArgTypes.insert(stj::MLabel::BasicType());
  supportedArgTypes.insert(stj::MLabels::BasicType());
  supportedArgTypes.insert(stj::MPlace::BasicType());
  supportedArgTypes.insert(stj::MPlaces::BasicType());

  ListExpr first = nl->First(args);
  ListExpr second= nl->Second(args);

  if (!listutils::isASymbolIn(first, supportedArgTypes) ||
      !listutils::isASymbolIn(second, supportedArgTypes)) {
    return listutils::typeError(errmsg);
  }

  string t1; nl->WriteToString(t1, first);
  string t2; nl->WriteToString(t2, second);

  NList resTupleType = NList(NList("Raw"), NList(t1)).enclose();
  resTupleType.append(NList(NList("Symbolic"), NList(t2)));
  NList resType = NList(NList(Symbol::STREAM()),
                        NList(NList(Tuple::BasicType()), resTupleType));
  return resType.listExpr();
}

template<class M1, class U1, class M2, class U2>
int ByValueMappingVM(Word* args, Word& result, int message, Word& local,
                     Supplier s) {
  DivisionStream<M1,U1,M2,U2>* li;
  switch (message) {
    case OPEN: {
      if(local.addr) {
        delete static_cast<DivisionStream<M1,U1,M2,U2>*>(local.addr);
        local.setAddr(0);
      }
      M1* mo1 = static_cast<M1*>(args[0].addr);
      M2* mo2 = static_cast<M2*>(args[1].addr);
      li = new DivisionStream<M1, U1, M2, U2>(mo1, mo2, GetTupleResultType(s));
      local.setAddr(li);
      return 0;
    }
    case REQUEST: {
      if(!local.addr) {
        return CANCEL;
      }
      li = static_cast<DivisionStream<M1, U1, M2, U2>*>(local.addr);
      Tuple* t = NULL;
      if (li->hasMore()) {
        li->getNext(&t);
        if (t) {
          result.setAddr(t);
          return YIELD;
        }
      }
      return CANCEL;
    }
    case CLOSE: {
      if (local.addr) {
        delete static_cast<DivisionStream<M1, U1, M2, U2>*>(local.addr);
        local.setAddr(0);
      }
      return 0;
    }
    default: {
      cerr << __PRETTY_FUNCTION__ << "Unknown message = " << message << "."
           << endl;
      return -1;
    }
  } // end switch
  cerr << __PRETTY_FUNCTION__ << "Unknown message = " << message << "."
       << endl;
  return -1;
}

ValueMapping ByValueMapping[] = {
 ByValueMappingVM<stj::MPlaces,stj::UPlaces,stj::MLabel,stj::ULabel>,
 ByValueMappingVM<stj::MPlaces,stj::UPlaces,stj::MLabels,stj::ULabels>,
 ByValueMappingVM<stj::MPlaces,stj::UPlaces,stj::MPlace,stj::UPlace>,
 ByValueMappingVM<stj::MPlaces,stj::UPlaces,stj::MPlaces,stj::UPlaces>,
 ByValueMappingVM<stj::MPlaces,stj::UPlaces,MString,UString>,
 ByValueMappingVM<stj::MPlaces,stj::UPlaces,MPoint,UPoint>,
 ByValueMappingVM<stj::MPlaces,stj::UPlaces,MReal,UReal>,
 ByValueMappingVM<stj::MPlaces,stj::UPlaces,MBool,UBool>,
 ByValueMappingVM<stj::MPlaces,stj::UPlaces,MInt,UInt>,

 ByValueMappingVM<stj::MPlace,stj::UPlace,stj::MLabel,stj::ULabel>,
 ByValueMappingVM<stj::MPlace,stj::UPlace,stj::MLabels,stj::ULabels>,
 ByValueMappingVM<stj::MPlace,stj::UPlace,stj::MPlace,stj::UPlace>,
 ByValueMappingVM<stj::MPlace,stj::UPlace,stj::MPlaces,stj::UPlaces>,
 ByValueMappingVM<stj::MPlace,stj::UPlace,MString,UString>,
 ByValueMappingVM<stj::MPlace,stj::UPlace,MPoint,UPoint>,
 ByValueMappingVM<stj::MPlace,stj::UPlace,MReal,UReal>,
 ByValueMappingVM<stj::MPlace,stj::UPlace,MBool,UBool>,
 ByValueMappingVM<stj::MPlace,stj::UPlace,MInt,UInt>,
 
 ByValueMappingVM<stj::MLabels,stj::ULabels,stj::MLabel,stj::ULabel>,
 ByValueMappingVM<stj::MLabels,stj::ULabels,stj::MLabels,stj::ULabels>,
 ByValueMappingVM<stj::MLabels,stj::ULabels,stj::MPlace,stj::UPlace>,
 ByValueMappingVM<stj::MLabels,stj::ULabels,stj::MPlaces,stj::UPlaces>,
 ByValueMappingVM<stj::MLabels,stj::ULabels,MString,UString>,
 ByValueMappingVM<stj::MLabels,stj::ULabels,MPoint,UPoint>,
 ByValueMappingVM<stj::MLabels,stj::ULabels,MReal,UReal>,
 ByValueMappingVM<stj::MLabels,stj::ULabels,MBool,UBool>,
 ByValueMappingVM<stj::MLabels,stj::ULabels,MInt,UInt>,
 
 ByValueMappingVM<stj::MLabel,stj::ULabel,stj::MLabel,stj::ULabel>,
 ByValueMappingVM<stj::MLabel,stj::ULabel,stj::MLabels,stj::ULabels>,
 ByValueMappingVM<stj::MLabel,stj::ULabel,stj::MPlace,stj::UPlace>,
 ByValueMappingVM<stj::MLabel,stj::ULabel,stj::MPlaces,stj::UPlaces>,
 ByValueMappingVM<stj::MLabel,stj::ULabel,MString,UString>,
 ByValueMappingVM<stj::MLabel,stj::ULabel,MPoint,UPoint>,
 ByValueMappingVM<stj::MLabel,stj::ULabel,MReal,UReal>,
 ByValueMappingVM<stj::MLabel,stj::ULabel,MBool,UBool>,
 ByValueMappingVM<stj::MLabel,stj::ULabel,MInt,UInt>,

 ByValueMappingVM<MString,UString,stj::MLabel,stj::ULabel>,
 ByValueMappingVM<MString,UString,stj::MLabels,stj::ULabels>,
 ByValueMappingVM<MString,UString,stj::MPlace,stj::UPlace>,
 ByValueMappingVM<MString,UString,stj::MPlaces,stj::UPlaces>,
 ByValueMappingVM<MString,UString,MString,UString>,
 ByValueMappingVM<MString,UString,MPoint,UPoint>,
 ByValueMappingVM<MString,UString,MReal,UReal>,
 ByValueMappingVM<MString,UString,MBool,UBool>,
 ByValueMappingVM<MString,UString,MInt,UInt>,

 ByValueMappingVM<MPoint,UPoint,stj::MLabel,stj::ULabel>,
 ByValueMappingVM<MPoint,UPoint,stj::MLabels,stj::ULabels>,
 ByValueMappingVM<MPoint,UPoint,stj::MPlace,stj::UPlace>,
 ByValueMappingVM<MPoint,UPoint,stj::MPlaces,stj::UPlaces>,
 ByValueMappingVM<MPoint,UPoint,MString,UString>,
 ByValueMappingVM<MPoint,UPoint,MPoint,UPoint>,
 ByValueMappingVM<MPoint,UPoint,MReal,UReal>,
 ByValueMappingVM<MPoint,UPoint,MBool,UBool>,
 ByValueMappingVM<MPoint,UPoint,MInt,UInt>,

 ByValueMappingVM<MReal,UReal,stj::MLabel,stj::ULabel>,
 ByValueMappingVM<MReal,UReal,stj::MLabels,stj::ULabels>,
 ByValueMappingVM<MReal,UReal,stj::MPlace,stj::UPlace>,
 ByValueMappingVM<MReal,UReal,stj::MPlaces,stj::UPlaces>,
 ByValueMappingVM<MReal,UReal,MString,UString>,
 ByValueMappingVM<MReal,UReal,MPoint,UPoint>,
 ByValueMappingVM<MReal,UReal,MReal,UReal>,
 ByValueMappingVM<MReal,UReal,MBool,UBool>,
 ByValueMappingVM<MReal,UReal,MInt,UInt>,

 ByValueMappingVM<MBool,UBool,stj::MLabel,stj::ULabel>,
 ByValueMappingVM<MBool,UBool,stj::MLabels,stj::ULabels>,
 ByValueMappingVM<MBool,UBool,stj::MPlace,stj::UPlace>,
 ByValueMappingVM<MBool,UBool,stj::MPlaces,stj::UPlaces>,
 ByValueMappingVM<MBool,UBool,MString,UString>,
 ByValueMappingVM<MBool,UBool,MPoint,UPoint>,
 ByValueMappingVM<MBool,UBool,MReal,UReal>,
 ByValueMappingVM<MBool,UBool,MBool,UBool>,
 ByValueMappingVM<MBool,UBool,MInt,UInt>,

 ByValueMappingVM<MInt,UInt,stj::MLabel,stj::ULabel>,
 ByValueMappingVM<MInt,UInt,stj::MLabels,stj::ULabels>,
 ByValueMappingVM<MInt,UInt,stj::MPlace,stj::UPlace>,
 ByValueMappingVM<MInt,UInt,stj::MPlaces,stj::UPlaces>,
 ByValueMappingVM<MInt,UInt,MString,UString>,
 ByValueMappingVM<MInt,UInt,MPoint,UPoint>,
 ByValueMappingVM<MInt,UInt,MReal,UReal>,
 ByValueMappingVM<MInt,UInt,MBool,UBool>,
 ByValueMappingVM<MInt,UInt,MInt,UInt>
};


int BySelect (ListExpr args) {
  int result = -1;
  if (stj::MPlaces::checkType(nl->First(args))) result = 0;
  if (stj::MPlace::checkType(nl->First(args)))  result = 9;
  if (stj::MLabels::checkType(nl->First(args))) result = 18;
  if (stj::MLabel::checkType(nl->First(args)))  result = 27;
  if (MString::checkType(nl->First(args))) result = 36;
  if (MPoint::checkType(nl->First(args)))  result = 45;
  if (MReal::checkType(nl->First(args)))   result = 54;
  if (MBool::checkType(nl->First(args)))   result = 63;
  if (MInt::checkType(nl->First(args)))    result = 72;
  if (stj::MLabel::checkType(nl->Second(args)))  result += 0;
  if (stj::MLabels::checkType(nl->Second(args))) result += 1;
  if (stj::MPlace::checkType(nl->Second(args)))  result += 2;
  if (stj::MPlaces::checkType(nl->Second(args))) result += 3;
  if (MString::checkType(nl->Second(args))) result += 4;
  if (MPoint::checkType(nl->Second(args)))  result += 5;
  if (MReal::checkType(nl->Second(args)))   result += 6;
  if (MBool::checkType(nl->Second(args)))   result += 7;
  if (MInt::checkType(nl->Second(args)))    result += 8;
  return result;
}



OperatorInfo ByOperatorInfo(
"by",
"{mT1} x {mT2} -> stream(tuple((Raw mt1)(Symbolic mt2))); T1, T2 in "
"{point, real, int, bool, string}",
"by( M1, M2 )",
"let r,s be trajectories.The expression by(r,s) returns set of tuples such "
"that each tuple contains a subtrajectory of r and a subtrajectory "
"of s having "
"the same time interval  and the subtrajectory of s contains "
"only one unit.For two distinct tuples their time intervals are disjoint."
"In other words,s is "
"split into units and each unit is put into a tuple "
"together with the part of "
"r at the same periods ot time.If for an interval , r is not defined "
"the Raw coloumn will be set to undedefined ",
"query by(train5,train8) count"
""
);


Operator by(
  ByOperatorInfo,
  ByValueMapping,
  BySelect,
  ByTypeMapping
);


/*end of implmenetation of by operator*/


/*start implementation of getFullRefinement*/

ListExpr
getFullRefinmentTypeMapping( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );
        if(listutils::isRelDescription(arg1,false)){

        ListExpr attributeList =nl->Second(nl->Second(arg1));
        ListExpr newAttributelist=nl->TheEmptyList();
        
        ListExpr lastnewAttributelist=nl->TheEmptyList();
        
        
        ListExpr index=nl->TheEmptyList();

        ListExpr lastIndex=nl->TheEmptyList();
          
          bool first=true;
          while (!(nl->IsEmpty(attributeList))){

              ListExpr singleAttribute =nl->First(attributeList);
              attributeList=nl->Rest(attributeList);
string attrname=nl->SymbolValue(nl->First(singleAttribute));
              ListExpr attrtype=nl->Second(singleAttribute);
              string newattrname=attrname;
              ListExpr newattrtype=attrtype;
              
              if( nl->IsEqual( attrtype, MBool::BasicType() ) ){
newattrtype=nl->SymbolAtom( UBool::BasicType() );
                  
              }
              else if( nl->IsEqual( attrtype, MInt::BasicType() ) ){
newattrtype=nl->SymbolAtom( UInt::BasicType() );
                  
              }
              else if( nl->IsEqual( attrtype, MReal::BasicType() ) ){
newattrtype=nl->SymbolAtom( UReal::BasicType() );
                  
              }
              else if( nl->IsEqual( attrtype, MPoint::BasicType() ) ){
newattrtype=nl->SymbolAtom( UPoint::BasicType() );
                  
              }
              else if( nl->IsEqual( attrtype, MString::BasicType() ) ){
newattrtype=nl->SymbolAtom( UString::BasicType() );
                  
              }

              else
              return nl->SymbolAtom( Symbol::TYPEERROR() );

              if(first){

                  newAttributelist=nl->OneElemList(
        nl->TwoElemList(nl->SymbolAtom(attrname), newattrtype));
                  lastnewAttributelist=newAttributelist;
                  
index=nl->OneElemList(nl->StringAtom(nl->SymbolValue(attrtype)));
                  lastIndex=index;
                  first=false;


              }
              else{
                              
lastnewAttributelist =nl->Append(lastnewAttributelist,
       nl->TwoElemList(nl->SymbolAtom(attrname), newattrtype));        
       lastIndex =nl->Append(lastIndex,
        nl->StringAtom(nl->SymbolValue(attrtype))); 
              }
          }
          
                  ListExpr arelstrcut=nl->TwoElemList(
                  nl->SymbolAtom(Symbol::STREAM()),
                  nl->TwoElemList(
         nl->SymbolAtom(Tuple::BasicType()),newAttributelist));

                  ListExpr outList =nl->ThreeElemList(
                  nl->SymbolAtom(Symbol::APPEND()),
                  nl->OneElemList(index),
                  arelstrcut);

          //cout <<"djskdjskdjskdj"<<nl->ToString(outList)<<endl;
          return outList;
        
    }
        
    
    }
    
    return nl->SymbolAtom( Symbol::TYPEERROR() );
}


int
getFullRefinmentSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if(listutils::isRelDescription(arg1, false))
      return 0;

  return -1;
}


OperatorInfo getFullRefinmentOperatorInfo(
"getHybridPartition",
"(rel(T)) -> stream(uT); T is a tuple containing moving"
"stuff only, uT is tuple containning unit of moving thing"
"at given periods",
"rel getHybridPartition",
"divide first tuple of a relation containg a stuff of moving point"
"to stream of tuple where each one contains the moving"
"units to a restricted periods",
""
);




 


BaseClass::BaseClass(){

}

void BaseClass::incrementIndex(){

    this->scaningIndex++;
}

void BaseClass::resetIndex(){

    this->scaningIndex=0;
}



void FullRefinment::fillmaps(){

         int index=0;
         std::vector<string>::iterator iter;
for (iter = this->inType.begin(); iter != this->inType.end(); ++iter)
         {
             string type=*iter;

             //cout <<"Type:"<<type<<endl;
             if(type.compare(MPoint::BasicType())==0){

                 GeneralMapping<MPoint,UPoint>* m=
new GeneralMapping<MPoint,UPoint>((MPoint*)this->tuple->GetAttribute(index));
                 this->totals.insert(std::make_pair(index,m));
             }
             else
                 if(type.compare(MString::BasicType())==0){

GeneralMapping<MString,UString>* m=new 
GeneralMapping<MString,UString>((MString*)this->tuple->GetAttribute(index));
this->totals.insert(std::make_pair(index,m));
                 }
             else 
                 if(type.compare(MBool::BasicType())==0){
                     GeneralMapping<MBool,UBool>* m=new 
GeneralMapping<MBool,UBool>((MBool*)this->tuple->GetAttribute(index));
this->totals.insert(std::make_pair(index,m));
                 }
             else 
                 if(type.compare(MInt::BasicType())==0){
                     GeneralMapping<MInt,UInt>* m=new 
GeneralMapping<MInt,UInt>((MInt*)this->tuple->GetAttribute(index));
this->totals.insert(std::make_pair(index,m));
                 }
             else 
                 if(type.compare(MReal::BasicType())==0){
                     GeneralMapping<MReal,UReal>* m=new 
GeneralMapping<MReal,UReal>((MReal*)this->tuple->GetAttribute(index));
this->totals.insert(std::make_pair(index,m));
                 }

             index ++;
         }   
}

FullRefinment::FullRefinment(Tuple* args,vector<string> inputType,
 ListExpr outputTupleExpression){

        this->tuple=args;
        this->ote=outputTupleExpression;
        this->numberofAttribute=nl->ListLength(ote);
        this->inType=inputType;
        this->fillmaps();
        this->firsttime=true;
        this->fillmaps();

}


void FullRefinment::getNext(Tuple** result){


        
             Instant mininst;
             bool minisclosed;

            Instant endinst;
            bool endisclosed;


            std::map<int, BaseClass*>::iterator iter;
std::pair<int, BaseClass*> firstEntry=*this->totals.begin() ;
            BaseClass* first=firstEntry.second;

            
            if(this->firsttime==true){
                first->resetIndex();
             first->getNextStart(mininst,minisclosed,true);
            iter = this->totals.begin()++;
            for (iter; iter != this->totals.end(); ++iter)  
            {

            iter->second->resetIndex();
             Instant inst;
             bool isclosed;
             iter->second->getNextStart(inst,isclosed,true);
             if(inst<mininst){
                 mininst=inst;
                 minisclosed=isclosed;
             }
             else if(inst==mininst)
                 minisclosed=minisclosed||isclosed;
            }

            this->firsttime=false;

        }
        else
        {
        
         
         bool firstmindone=false;
         iter=this->totals.begin();
         for ( iter;iter != this->totals.end(); ++iter) 
         {
            if(iter->second->isFinished())
                 continue;


             
             int res;
             if(firstmindone==false){

res=iter->second->getNextStart(mininst,minisclosed,false);
                firstmindone=true;
             }
             else{
             
            Instant inst;
            bool isclosed;
            res=iter->second->getNextStart(inst,isclosed,false);
                 if(inst<mininst){
                 mininst=inst;
                 minisclosed=isclosed;
             
                 }
             
                 else if(inst==mininst)
                 
                     minisclosed=minisclosed||isclosed;
             }
             
             if(res==1)
                 break;

         }

        }


            bool finishfirst=false;
            iter=this->totals.begin();
            for (iter; iter != this->totals.end(); ++iter)  
            {
iter->second->setLastStartInstant(mininst,minisclosed);

            
                if(iter->second->isFinished())
                 
                    continue;




                
                        
                if(finishfirst==false){
                
iter->second->getNearestEnd(endinst,endisclosed);
                    finishfirst=true;
                }
                else{
                Instant inst;
                bool isclosed;

                    iter->second->getNearestEnd(inst,isclosed);
                    if(inst<endinst){
                 endinst=inst;
                 endisclosed=isclosed;
            
                    }
             
                    else if(inst==endinst){
                 endisclosed=endisclosed&&isclosed;
             
                    }
                }


            }





*result=new Tuple(this->ote);
for (iter = this->totals.begin(); 
iter != this->totals.end(); ++iter)
{
iter->second->
 setLastfinishInstant(endinst,endisclosed);
                Attribute* tem;
                iter->second->Get(&tem);
                (*result)->PutAttribute(iter->first,tem);
                iter->second->makeIncrementifOK();
            }

        
             
        
}

bool FullRefinment::haveMoreUnits(){
         

std::map<int, BaseClass*>::iterator iter;
for (iter = this->totals.begin()
 ; iter != this->totals.end(); ++iter)
         {
             if(iter->second->isFinished()==false)
                 return true;
         }
         return false;   
}

FullRefinment::~FullRefinment(){
        
std::map<int, BaseClass*>::iterator iter;
for (iter = this->totals.begin()
; iter != this->totals.end(); ++iter)
         {
             delete iter->second;
         }
         
     }
/*take the rel and produce the stream of tuple*/
int getHybridRefinmentVM( Word* args, Word& result,
int message,Word& local, Supplier s )
{


  GenericRelation* r;
  GenericRelationIterator* rit;
  FullRefinment* li;
  switch( message )
  {
    case OPEN:{
      if(local.addr){
        delete static_cast<FullRefinment*>(local.addr);
        local.setAddr(0);
      }
      r = static_cast<GenericRelation*>(args[0].addr);
      rit = r->MakeScan();
      Tuple *t = rit->GetNextTuple();
      vector<string> vec;
      Word elem(Address(0));
       Supplier son ;
      for(int scanIndex=0;scanIndex<t->GetNoAttributes() ;scanIndex++){
          
        son = qp->GetSupplier(args[1].addr, scanIndex);
        qp->Request(son, elem);
        vec.push_back (((CcString*)elem.addr)->GetValue());
      }
      li = new FullRefinment(t,vec,nl->Second(GetTupleResultType(s)));
      local.setAddr(li);
      return 0;
    }
    case REQUEST:{
      if(!local.addr){
        return CANCEL;
      }
      li = static_cast<FullRefinment*>(local.addr);
      Tuple* t = NULL;
      if(li->haveMoreUnits()){
        li->getNext(&t);
        if(t){
          result.setAddr(t);
          return YIELD;
        }
      }
      return CANCEL;
    }
    case CLOSE:{
      if(local.addr){
        delete static_cast<FullRefinment*>(local.addr);
        local.setAddr(0);
      }
      return 0;
    }
    default:{
      cerr << __PRETTY_FUNCTION__ << "Unknown message = " << message << "."
           << endl;
      return -1;
    }
  } // end switch
  cerr << __PRETTY_FUNCTION__ << "Unknown message = " << message << "."
       << endl;
  return -1;

}

ValueMapping getFullRefinementValueMapping[] = {getHybridRefinmentVM};
Operator getFullRefinement(
  getFullRefinmentOperatorInfo,
  getFullRefinementValueMapping,
  getFullRefinmentSelect,
  getFullRefinmentTypeMapping
);

class HybridTrajectoryAlgebra : public Algebra {
  public:
    HybridTrajectoryAlgebra() : Algebra() {

        AddOperator(&temporalatinstant);
        AddOperator(&temporalatperiods);
        AddOperator(&by);
        AddOperator(&getFullRefinement);

    }


    ~HybridTrajectoryAlgebra() {};
};


}

extern "C"
Algebra* InitializeHybridTrajectoryAlgebra(NestedList *nlRef,
                                             QueryProcessor *qpRef) {
  return new hyt::HybridTrajectoryAlgebra;
}
