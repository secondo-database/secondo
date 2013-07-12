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

Started July 2013, Hamza Issa\'{e}s

[TOC]
\section{Overview}
This algebra includes the operators ~by~ and ~atinstant~.

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
#include <string>
#include <vector>
#include "DivisionStream.h"
extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;


using namespace std;


namespace hyt {



/*start of atInstantImplmentation*/
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
		nl->SymbolAtom("AtInstant"),
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
    			  newattrtype=nl->SymbolAtom( IBool::BasicType() );
    			  inttoappend =1;
    		  }
    		  if( nl->IsEqual( attrtype, MInt::BasicType() ) ){
    			  newattrtype=nl->SymbolAtom( IInt::BasicType() );
    			  inttoappend=2;
    		  }
    		  if( nl->IsEqual( attrtype, MReal::BasicType() ) ){
    			  newattrtype=nl->SymbolAtom( IReal::BasicType() );
    			  inttoappend=3;
    		  }
    		  if( nl->IsEqual( attrtype, MPoint::BasicType() ) ){
    			  newattrtype=nl->SymbolAtom( IPoint::BasicType() );
    			  inttoappend=4;
    		  }
    		  if( nl->IsEqual( attrtype, MString::BasicType() ) ){
newattrtype=nl->SymbolAtom( IString::BasicType() );
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
nl->SymbolAtom(Tuple::BasicType()),
newAttributelist));
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
    	  atuple->PutAttribute(scanIndex+1,pResult);
    }
    else if(type==2){
    	MInt* mint=(MInt*)t->GetAttribute(scanIndex);
    	Intime<CcInt>* pResult = new Intime<CcInt>();
    	mint->AtInstant(* instant, *pResult);
    	atuple->PutAttribute(scanIndex+1,pResult);
    }
    else if(type==1){
    	MBool* mbool=(MBool*)t->GetAttribute(scanIndex);
    	Intime<CcBool>* pResult = new Intime<CcBool>();
    	mbool->AtInstant(* instant, *pResult);
    	atuple->PutAttribute(scanIndex+1,pResult);
    }
    else if(type==3){

    	MReal* mreal=(MReal*)t->GetAttribute(scanIndex);
    	Intime<CcReal>* pResult = new Intime<CcReal>();
    	mreal->AtInstant(* instant, *pResult);
    	atuple->PutAttribute(scanIndex+1,pResult);

    }
    else if(type==5){

     	MString* mstring=(MString*)t->GetAttribute(scanIndex);
     	Intime<CcString>* pResult = new Intime<CcString>();
     	mstring->AtInstant(* instant, *pResult);
     	atuple->PutAttribute(scanIndex+1,pResult);
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

ListExpr ByTypeMapping(ListExpr args){
int noargs = nl->ListLength(args);
string errmsg = "Expected {mT1} x {mT2}, where T in "
                "{point, int, real, bool, string}";
if(noargs!=2){
  return listutils::typeError(errmsg);
}
set<string> supportedArgTypes;
supportedArgTypes.insert(MPoint::BasicType());
supportedArgTypes.insert(MReal::BasicType());
supportedArgTypes.insert(MInt::BasicType());
supportedArgTypes.insert(MBool::BasicType());
supportedArgTypes.insert(MString::BasicType());


ListExpr first = nl->First(args);
ListExpr second= nl->Second(args);

if( !listutils::isASymbolIn(first,supportedArgTypes) ||
    !listutils::isASymbolIn(second,supportedArgTypes)) {
  return listutils::typeError(errmsg);
}

string t1; nl->WriteToString(t1, first);
string t2; nl->WriteToString(t2, second);

NList resTupleType =NList(NList("Raw"),NList(t1)).enclose();
resTupleType.append(NList(NList("Symbolic"),NList(t2)));
NList resType =
NList(NList(Symbol::STREAM()),NList(NList(Tuple::BasicType()),resTupleType));
return resType.listExpr();

}
template<class M1, class U1, class M2, class U2>
int ByValueMappingVM( Word* args, Word& result, int message,
                              Word& local, Supplier s )
{

  DivisionStream<M1,U1,M2,U2>* li;
  switch( message )
  {
    case OPEN:{
      if(local.addr){
        delete static_cast<DivisionStream<M1,U1,M2,U2>*>(local.addr);
        local.setAddr(0);
      }
      M1* mo1;
      mo1 = static_cast<M1*>(args[0].addr);
      M2* mo2;
      mo2 = static_cast<M2*>(args[1].addr);
      li = new DivisionStream<M1,U1,M2,U2>(mo1,mo2,GetTupleResultType(s));
      local.setAddr(li);
      return 0;
    }
    case REQUEST:{
      if(!local.addr){
        return CANCEL;
      }
      li = static_cast<DivisionStream<M1,U1,M2,U2>*>(local.addr);
      Tuple* t = NULL;
      if(li->hasMore()){
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
        delete static_cast<DivisionStream<M1,U1,M2,U2>*>(local.addr);
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

ValueMapping ByValueMapping[] = {

 ByValueMappingVM<MString,UString,MString,UString>,
 ByValueMappingVM<MString,UString,MPoint,UPoint>,
 ByValueMappingVM<MString,UString,MReal,UReal>,
 ByValueMappingVM<MString,UString,MBool,UBool>,
 ByValueMappingVM<MString,UString,MInt,UInt>,

 ByValueMappingVM<MPoint,UPoint,MString,UString>,
 ByValueMappingVM<MPoint,UPoint,MPoint,UPoint>,
 ByValueMappingVM<MPoint,UPoint,MReal,UReal>,
 ByValueMappingVM<MPoint,UPoint,MBool,UBool>,
 ByValueMappingVM<MPoint,UPoint,MInt,UInt>,

 ByValueMappingVM<MReal,UReal,MString,UString>,
 ByValueMappingVM<MReal,UReal,MPoint,UPoint>,
 ByValueMappingVM<MReal,UReal,MReal,UReal>,
 ByValueMappingVM<MReal,UReal,MBool,UBool>,
 ByValueMappingVM<MReal,UReal,MInt,UInt>,

 ByValueMappingVM<MBool,UBool,MString,UString>,
 ByValueMappingVM<MBool,UBool,MPoint,UPoint>,
 ByValueMappingVM<MBool,UBool,MReal,UReal>,
 ByValueMappingVM<MBool,UBool,MBool,UBool>,
 ByValueMappingVM<MBool,UBool,MInt,UInt>,

 ByValueMappingVM<MInt,UInt,MString,UString>,
 ByValueMappingVM<MInt,UInt,MPoint,UPoint>,
 ByValueMappingVM<MInt,UInt,MReal,UReal>,
 ByValueMappingVM<MInt,UInt,MBool,UBool>,
 ByValueMappingVM<MInt,UInt,MInt,UInt>
};


int BySelect( ListExpr args ) {
  int res = 0;
  // first arg type
  if(listutils::isSymbol(nl->First(args),
   MString::BasicType())) { res+=0; }
  else if(listutils::isSymbol(nl->First(args),
   MPoint::BasicType())) { res+=5; }
  else if(listutils::isSymbol(nl->First(args),
   MReal::BasicType()))  { res+=10; }
  else if(listutils::isSymbol(nl->First(args),
    MBool::BasicType())) { res+=15; }
  else if(listutils::isSymbol(nl->First(args),
   MInt::BasicType())) { res+=20; }
  else {res+= -9999999; }
  // second arg type
  if(listutils::isSymbol(nl->Second(args),MString::BasicType())) { res+=0;}
  else if(listutils::isSymbol(nl->Second(args),
    MPoint::BasicType())) { res+=1; }
  else if(listutils::isSymbol(nl->Second(args),
    MReal::BasicType())) { res+=2; }
  else if(listutils::isSymbol(nl->Second(args),
    MBool::BasicType())) { res+=3; }
  else if(listutils::isSymbol(nl->Second(args),
    MInt::BasicType())) { res+=4; }
  else {res+= -9999999; }

  return (((res>=0)&&(res<=24))?res:-1);
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
class HybridTrajectoryAlgebra : public Algebra {
  public:
	HybridTrajectoryAlgebra() : Algebra() {

		AddOperator(&temporalatinstant);
		AddOperator(&temporalatperiods);
		AddOperator(&by);

	}


	~HybridTrajectoryAlgebra() {};
};


}

extern "C"
Algebra* InitializeHybridTrajectoryAlgebra(NestedList *nlRef,
                                             QueryProcessor *qpRef) {
  return new hyt::HybridTrajectoryAlgebra;
}



