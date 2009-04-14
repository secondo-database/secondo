/*

 STPatternAlgebra.cpp

 Created on: Jan 6, 2009
       Author: m.attia


*/

#include "STPatternAlgebra.h"

namespace STP{




/*
1   Operators

*/

/*
1.1 Operator ~pattern~

1.1.1 Type mapping functions of operator ~pattern~

*/

int FindConnector(string sym)
{
  int i=0;
  while(connector[i]!= "0" && connector[i]!= sym) i++;
  if (connector[i] == sym) return i;
  return -1;
}
ListExpr PatternTypeMap2(ListExpr opargs)
{
  bool debugme= false;
  ListExpr args= nl->First(opargs);
  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }


  ListExpr first = nl->First(args), //tuple(x)
  second = nl->Second(args);		  //predicatelist

  nl->WriteToString(argstr, first);
  if(debugme)
  {
    cout<<endl<< argstr<<endl;
    cout<< "nl->ListLength(first)" 
    << nl->ListLength(first)<<endl;
    cout<<"(TypeOfRelAlgSymbol(nl->First(first))" <<
    TypeOfRelAlgSymbol(nl->First(first))<<endl;
    cout.flush();
  }

  //checking for the first parameter tuple(x)
  CHECK_COND( (nl->ListLength(first) == 2) &&
      (TypeOfRelAlgSymbol(nl->First(first)) == tuple),
      "Operator stpattern expects as first argument "
      "a list with structure "
      "(tuple ((a1 t1)...(an tn)))\n"
      "Operator stpattern gets a list with structure '" +
      argstr + "'.");

  //checking ofr the second parameter predicatelist
  nl->WriteToString(argstr, second);
  CHECK_COND( ! nl->IsAtom(second) ,
      "Operator  stpattern expects as second argument a "
      "list of named predicates\n"
      "Operator stpattern gets as second argument '" +
      argstr + "'.\n" );

  ListExpr secondRest = second;
  ListExpr secondFirst;
  secondFirst =  nl->Empty();

  bool isConnector=false;
  while( !nl->IsEmpty(secondRest) )
  {
    secondFirst = nl->First(secondRest);
    secondRest = nl->Rest(secondRest);
    nl->WriteToString(argstr, secondFirst);
    if(!isConnector)
    {
      if(debugme)
      {
        cout<< nl->ToString(secondFirst)<<endl;
        cout.flush();
      }

      CHECK_COND
      ((nl->ListLength(secondFirst) == 1 &&
          nl->IsAtom(nl->First(secondFirst))&&
          nl->SymbolValue(nl->First(secondFirst))=="mbool")||
          (nl->ListLength(secondFirst) == 2 &&
              nl->IsAtom(nl->Second(secondFirst))&&
              nl->SymbolValue(nl->Second(secondFirst))=="mbool" &&
              nl->IsAtom(nl->First(secondFirst))), "Operator "
              "stpattern expects a list of named predicates. "
              "Operator stpattern gets '" + argstr + "'.");
      isConnector= !isConnector;	
    }
    else
    {
      CHECK_COND(nl->IsAtom(secondFirst) &&
          FindConnector(nl->SymbolValue(secondFirst)) != -1  ,
          "Operator stpattern expects a temporal connector"
          " but got." + argstr);
      isConnector=!isConnector;
    }
  }
  ListExpr result = nl->SymbolAtom("bool");
  if(debugme)
  {
    cout<<endl<<endl<<"Operator stpattern accepted the input";
    cout.flush();
  }
  return result;
}

ListExpr PatternTypeMap3(ListExpr opargs)
{
  bool debugme= false;
  ListExpr args= nl->First(opargs);
  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  ListExpr first = nl->First(args), //tuple(x)
  second = nl->Second(args),		  //predicatelist
  third= nl->Third(args);			  //bool

  nl->WriteToString(argstr, first);
  if(debugme)
  {
    cout<<endl<< argstr<<endl;
    cout<< "nl->ListLength(first)" << nl->ListLength(first)<<endl;
    cout<<"(TypeOfRelAlgSymbol(nl->First(first))" <<
    TypeOfRelAlgSymbol(nl->First(first))<<endl;
    cout.flush();
  }

  //checking for the first parameter tuple(x)
  CHECK_COND( (nl->ListLength(first) == 2) &&
      (TypeOfRelAlgSymbol(nl->First(first)) == tuple),
      "Operator stpattern expects as first argument "
      "a list with structure "
      "(tuple ((a1 t1)...(an tn)))\n"
      "Operator stpattern gets a list with structure '" +
      argstr + "'.");

  //checking ofr the second parameter predicatelist
  nl->WriteToString(argstr, second);
  CHECK_COND( ! nl->IsAtom(second) ,
      "Operator  stpattern expects as second argument a "
      "list of predicates\n"
      "Operator stpattern gets as second argument '" +
      argstr + "'.\n" );


  ListExpr secondRest = second;
  ListExpr secondFirst;
  secondFirst =  nl->Empty();

  bool isConnector=false;
  while( !nl->IsEmpty(secondRest) )
  {
    secondFirst = nl->First(secondRest);
    secondRest = nl->Rest(secondRest);
    nl->WriteToString(argstr, secondFirst);
    if(!isConnector)
    {
      if(debugme)
      {
        cout<< nl->ToString(secondFirst)<<endl;
        cout.flush();
      }

      CHECK_COND
      ((nl->ListLength(secondFirst) == 1 &&
          nl->IsAtom(nl->First(secondFirst))&&
          nl->SymbolValue(nl->First(secondFirst))=="mbool")||
          (nl->ListLength(secondFirst) == 2 &&
              nl->IsAtom(nl->Second(secondFirst))&&
              nl->SymbolValue(nl->Second(secondFirst))=="mbool" &&
              nl->IsAtom(nl->First(secondFirst))),  "Operator "
              "stpattern expects a list of named predicates. "
              "Operator stpattern gets '" + argstr + "'.");
      isConnector= !isConnector;
    }
  }

  //checking for the third parameter bool
  nl->WriteToString(argstr, third);
  CHECK_COND(nl->IsAtom(third) &&
      nl->SymbolValue(third)== "bool",
      "Operator stpattern expects the third parameter "
      "to be a bool expression "
      "but got '" + argstr + "'.");

  ListExpr result = nl->SymbolAtom("bool");
  if(debugme)
  {
    cout<<endl<<endl<<"Operator stpattern accepted the input";
    cout.flush();
  }
  return result;
}

ListExpr PatternTypeMap(ListExpr opargs)
{
  bool debugme= false;
  string argstr;
  ListExpr args= nl->First(opargs);

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }
  nl->WriteToString(argstr, args);
  CHECK_COND(nl->ListLength(args) == 3 ||nl->ListLength(args) == 2 ,
      "Operator stpattern expects a list of length two or three "
      "but got '" + argstr + "'.");
  if(nl->ListLength(args) == 3)
    return PatternTypeMap3(opargs);
  else if(nl->ListLength(args) == 2)
    return PatternTypeMap2(opargs);
}

/*
1.1.2 Value mapping function of operator ~stpattern~

*/


int NextPosition( Periods* deftime, const Instant& t )
{
  assert( deftime->IsOrdered() && t.IsDefined() );

  int last = deftime->GetNoComponents();
  const Interval<Instant> *interval;
  int i=0;

  deftime->Get(i++,interval);
  while(i<last && t > interval->start)
    deftime->Get(i++, interval);
    if(i<= last)
      return i-1;
    else
      return -1;
}

bool GetNextMatchInterval(MBool* predRes,
    Instant stime, Interval<Instant>& nextinterval)
{
  int pos;
  const UBool* cur;
  Periods dtime;
  predRes->DefTime(dtime);
  pos= predRes->Position(stime);

  if(pos== -1 && (pos=NextPosition(&dtime, stime)) == -1)
    return false;

  predRes->Get(pos++,cur );
  while( !cur->constValue.GetBoolval() &&
      pos < predRes->GetNoComponents())
    predRes->Get(pos++, cur);
    if(cur->constValue.GetBoolval())
    {
      nextinterval.end = cur->timeInterval.end;
      nextinterval.rc = cur->timeInterval.rc;
      nextinterval.lc = true;
      nextinterval.start = (cur->timeInterval.start > stime) ?
          cur->timeInterval.start : stime;
          return true;
    }
    return false;
}

bool Match(MBool* predRes, Interval<Instant>& sinterval,
    Supplier connector)
{
  bool debugme=false;
  string con;
  Interval<Instant> nextinterval= sinterval;
  bool match=false;
  if(!predRes->IsDefined())	return false;

  if(connector==0) con="then";
  else	con= nl->ToString(qp->GetSupplierTypeExpr(connector));

  match= GetNextMatchInterval(predRes, sinterval.start, nextinterval);
  //looks for a true unit in predRes starting form sinterval.start
  //and looking only forward. If it is found that predRes was true
  //before, at and maybe after sinterval.star, nextinterval is populated
  //with sinterval.start as the start instant.

  if(!match) return false;

  if(debugme)
  {
    cout<<"Applying connectore: "<< con<<endl; 
    cout<<"Next interval: ";nextinterval.Print(cout);	cout<<endl;
    cout<<"Curr interval: ";sinterval.Print(cout);		cout<<endl;
    cout.flush();
  }
  if(con == "immediately") //now
  {
    if(nextinterval.start == sinterval.start)
    {
      sinterval= nextinterval;
      return true;
    }
    else
      return false;
  }
  else if (con == "follows") //at the end of the current interval
  {
    if(nextinterval.start == sinterval.end)
    {
      sinterval= nextinterval;
      return true;
    }
    else
      return false;
  }
  else if (con == "meanwhile") //during the current interval
  {
    if(nextinterval.start > sinterval.start &&
        nextinterval.start < sinterval.end)
    {
      sinterval= nextinterval;
      return true;
    }
    else
      return false;
  }
  else if (con == "later") //sometime after the current interval
  {

    if(nextinterval.start > sinterval.end)
    {
      sinterval= nextinterval;
      return true;
    }
    else
      return false;
  }
  else if (con == "then") //any time starting from now
  {
    sinterval= nextinterval;
    return true;
  }
  else
    assert(0); //invalid connector symbol
}

int Pattern(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme=false;
  Supplier root, namedpredlist, namedpred,alias, pred,filter,connector=0;
  Word value;
  int noofpred, nosons;
  bool matchPred;
  bool isConnector=false;

  DateTime lastMatchTime(0,0,instanttype);
  Interval<DateTime> 
  lastMatchInterval(lastMatchTime, lastMatchTime,true,true);

  result = qp->ResultStorage( s );
  root = args[0].addr;

  if(debugme)
  {
    cout<<endl<<"Root "; //qp->ListOfTree(root,cout); 
    // nl->ToString(qp->GetSupplierTypeExpr(root));
    //cout<<endl<<"Labellist " <<nl->ToString(label)<<endl;
    cout.flush();
  }

  namedpredlist = qp->GetSupplierSon(root, 1);
  if(qp->GetNoSons(root) == 3)
  {
    filter = qp->GetSupplierSon(root, 2);
    label.clear(); //labels hashtable
  }

  noofpred = qp->GetNoSons(namedpredlist);
  assert(noofpred>0);

  if(debugme)
  {
    cout<<endl<<"Predlist "<< 
    nl->ToString(qp->GetSupplierTypeExpr(namedpredlist));
    cout<<endl<<"Predlist count " << noofpred;
    if(qp->GetNoSons(root) == 3)
      cout<<endl<<"Filter " << 
      nl->ToString(qp->GetSupplierTypeExpr(filter));
      cout.flush();
  }

  for (int i=0; i < noofpred;i++)
  {
    if(!isConnector)
    {
      namedpred= qp->GetSupplierSon(namedpredlist,i);
      nosons = qp->GetNoSons(namedpred);
      assert(nosons==1 || nosons==2);
      alias=0;
      if(nosons == 1)
        pred= qp->GetSupplierSon(namedpred,0);
        else
        {
          alias= qp->GetSupplierSon(namedpred,0);
          pred = qp->GetSupplierSon(namedpred,1);
        }

      qp->Request(pred,value);
      matchPred= Match((MBool*)value.addr,
          lastMatchInterval,connector);

      if(alias !=0)	//populate the labels
        label[nl->ToString(qp->GetSupplierTypeExpr(alias))]
              = lastMatchInterval;

      if(debugme)
      {
        if(matchPred)
        {
          cout<<endl<<"Matched Pred "<< i <<
          " at interval ";
          lastMatchInterval.Print(cout);
        }
        else
          cout<< "didn't match Predicate "
          << i;
        cout.flush();
      }
      if(!matchPred) break;
    }
    else
      connector= qp->GetSupplierSon(namedpredlist,i);
      isConnector= ! isConnector;
  }

  if(matchPred && qp->GetNoSons(root) == 3)  //second part of stpattern
  {
    qp->Request(filter,value);
    label.clear();
    ((CcBool*)result.addr)->CopyFrom((CcBool*)value.addr);
  }
  else
    ((CcBool*)result.addr)->Set(true,matchPred);
    return 0;
}

/*
1.1.3 Specification of operator ~pattern~

*/

OperatorInfo STPatternOperatorInfo( "stpattern",
    "(tuple(x) namedPredicateList bool) -> bool",
    ". stpattern[alias:lifted predicate connector "
    "lifted predicate connector ... ; bool expression ] ",
    "The implementation for spatio temporal pattern queries."
    " Only tuples, fulfilling the specified pattern and the"
    " bool expr are passed on to the output.",
    "query Trains feed filter [. stpattern[a:.Trip inside msnow then"
    " b:distance(.Trip, mehringdamm) < 10.0; intervalstart(\"a\")"
" < intervalstart(\"b\") ] ] count");


/*

1.1.4 Definition of operator ~pattern~

*/

Operator opdefpattern(  STPatternOperatorInfo,
    Pattern,
    PatternTypeMap );


/*

1.2 Definition of other operator used as helpers
	 with stpattern operator

*/

/*

1.2.1 Definition of operator ~intervalstart~

*/
ListExpr IntervalStartEndTypeMap(ListExpr args)
{
  bool debugme=false;
  string argstr;
  if(debugme)
  {
    cout<<endl<< nl->ToString(args) <<endl;
    cout<< nl->ListLength(args)  << ".."<< nl->IsAtom(nl->First(args))<<
    ".."<< nl->SymbolValue(nl->First(args));
    cout.flush();
  }
  nl->WriteToString(argstr, args);
  CHECK_COND(nl->ListLength(args) == 1 &&
      nl->IsAtom(nl->First(args)) &&
      nl->SymbolValue(nl->First(args))== "string",
      "Operator intervalstart expects a string symbol "
      "but got." + argstr);
  return nl->SymbolAtom("instant");
}

template <bool leftbound>
int IntervalBound
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme= false;
  Interval<Instant> interval;
  Word input;
  string lbl;

  //qp->Request(args[0].addr, input);
  lbl= ((CcString*)args[0].addr)->GetValue();
  if(debugme)
  {
    cout<<endl<<"Accessing the value of label "<<lbl;
    cout.flush();
  }

  map<string, Interval<Instant> >::iterator iter = 
    label.find(lbl);
  if( iter != label.end() )
    interval = iter->second;
    else
      return 1;

  if(debugme)
  {
    cout<<endl<<"Value is ";
    interval.Print(cout);
    cout.flush();
  }
  result = qp->ResultStorage( s );
  if(leftbound)
    ((Instant*)result.addr)->CopyFrom(&interval.start);
    else
      ((Instant*)result.addr)->CopyFrom(&interval.start);
      return 0;
}

OperatorInfo IntervalSOperatorInfo( "intervalstart",
    "string -> instant",
    "intervalstart(\"label\")",
    "Used within stpattern operator to read the start value"
    "of a label.",
    "query Trains feed filter[. stpattern[a: .Trip inside msnow,  "
    "then b: distance(.Trip, mehringdamm) < 10.0 ; "
"intervalstart(\"b\") > intervalstart(\"a\") ) ] count");

Operator opdefintervals(  IntervalSOperatorInfo,
    IntervalBound<true>,
    IntervalStartEndTypeMap );


/*

1.2.2 Definition of operator ~intervalend~

*/

OperatorInfo IntervalEOperatorInfo( "intervalend",
    "string -> instant",
    "intervalend(\"label\")",
    "Used within stpattern operator to read the end value"
    "of a label.",
    "query Trains feed filter[. stpattern[a: .Trip inside msnow,  "
    "then b: distance(.Trip, mehringdamm) < 10.0 ; "
"intervalend(\"b\") > intervalend(\"a\") ) ] count");


Operator opdefintervale(  IntervalEOperatorInfo,
    IntervalBound<false>,
    IntervalStartEndTypeMap );

/*
2 Implementation of the Algebra Class

*/

class STPatternAlgebra : public Algebra
{
public:
  STPatternAlgebra() : Algebra()
  {

/*

2.1 Registration of Types


*/


/*
2.2 Registration of Operators

*/
    opdefpattern.SetRequestsArguments();
    AddOperator(&opdefpattern);
    AddOperator(&opdefintervals);
    AddOperator(&opdefintervale);
  }
  ~STPatternAlgebra() {};
};


/*
3 Initialization

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

To link the algebra together with the system you must create an
entry in the file "makefile.algebra" and to define an algebra ID in the
file "Algebras/Management/AlgebraList.i.cfg".

*/



extern "C"
Algebra*
InitializeSTPatternAlgebra( NestedList* nlRef,
    QueryProcessor* qpRef )
    {
  // The C++ scope-operator :: must be used to qualify the full name
  return new STP::STPatternAlgebra;
    }

/*
4 Examples and Tests

The file "PointRectangle.examples" contains for every operator one example.
This allows one to verify that the examples are running and to provide a coarse
regression test for all algebra modules. The command "Selftest <file>" will
execute the examples. Without any arguments, the examples for all active
algebras are executed. This helps to detect side effects, if you have touched
central parts of Secondo or existing types and operators.

In order to setup more comprehensive automated test procedures one can write a
test specification for the ~TestRunner~ application. You will find the file
"example.test" in directory "bin" and others in the directory "Tests/Testspecs".
There is also one for this algebra.

Accurate testing is often treated as an unpopular daunting task. But it is
absolutely inevitable if you want to provide a reliable algebra module.

Try to write tests covering every signature of your operators and consider
special cases, as undefined arguments, illegal argument values and critical
argument value combinations, etc.


*/

}
