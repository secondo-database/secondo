/*

 STPatternAlgebra.cpp

 Created on: Jan 6, 2009
       Author: m.attia


*/

#include "STPatternAlgebra.h"

namespace STP{




/*
 4	Operators

*/

/*
4.1 Operator ~pattern~

4.1.1 Type mapping function of operator ~pattern~

*/

#if defined(design_stream)

ListExpr PatternTypeMap(ListExpr args)
{
	bool debugme= false;
	ListExpr errorInfo;
	errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
	string argstr, argstr2;

	if(debugme)
	{
		cout<<endl<< nl->ToString(args)<<endl;
		cout.flush();
	}

	CHECK_COND(nl->ListLength(args) == 2,
			"Operator stpattern expects a list of length two.");

	ListExpr first = nl->First(args),
	second = nl->Second(args);

	nl->WriteToString(argstr, first);
	if(debugme)
	{
		cout<<endl<< argstr<<endl;
		cout<< "nl->ListLength(first)" << nl->ListLength(first)<<endl;
		cout<<"(TypeOfRelAlgSymbol(nl->First(first))" <<
		TypeOfRelAlgSymbol(nl->First(first))<<endl;
		cout<<"nl->ListLength(nl->Second(first)) "<<
		nl->ListLength(nl->Second(first))<<endl;
		cout<<"TypeOfRelAlgSymbol(nl->First(nl->Second(first))" <<
		TypeOfRelAlgSymbol(nl->First(nl->Second(first)))<<endl;
		cout<<"nl->ListLength(nl->Second(first))" <<
		nl->ListLength(nl->Second(first))<<endl;
		cout<<"IsTupleDescription(nl->Second(nl->Second(first))" <<
		IsTupleDescription(nl->Second(nl->Second(first)))<<endl;
		cout.flush();
	}
	CHECK_COND(nl->ListLength(first) == 2  &&
		(TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
		(nl->ListLength(nl->Second(first)) == 2) &&
		(TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
		(nl->ListLength(nl->Second(first)) == 2) &&
		(IsTupleDescription(nl->Second(nl->Second(first)))),
		"Operator stpattern expects as first argument a "
		"list with structure (stream (tuple ((a1 t1)...(an tn))))\n"
		"Operator pattern gets as first argument '" +
		argstr + "'." );

	nl->WriteToString(argstr, second);
	CHECK_COND( ! nl->IsAtom(second) ,
			"Operator  stpattern expects as second argument a "
			"list of functions\n"
			"Operator stpattern gets as second argument '" +
			argstr + "'.\n" );




	ListExpr secondRest = second;
	ListExpr secondFirst, attrType, newAttrList, numberList;
	secondFirst = attrType = newAttrList = numberList = nl->Empty();
	ListExpr lastNewAttrList, lastNumberList;
	lastNewAttrList = lastNumberList = nl->Empty();

	string attrName = "";
	while( !nl->IsEmpty(secondRest) )
	{
		secondFirst = nl->First(secondRest);
		secondRest = nl->Rest(secondRest);

		nl->WriteToString(argstr, secondFirst);
		CHECK_COND(nl->ListLength(secondFirst) == 2 &&
			nl->IsAtom(nl->First(secondFirst)),
			"Operator stpattern expects that every function "
			"in the function list has a valid identifier "
			"Operator stpattern gets instead of this identifier "
			"name a list with structure '" + argstr + "'.");

		secondFirst= nl->Second(secondFirst);
		nl->WriteToString(argstr, secondFirst);
		if(debugme)
		{
			cout<< endl<<argstr<<endl;
			cout<< "nl->ListLength(secondFirst)" <<
			nl->ListLength(secondFirst)<<endl;
			cout<< "TypeOfRelAlgSymbol(nl->First(secondFirst)) "<<
			TypeOfRelAlgSymbol(nl->First(secondFirst))<<endl;
			cout<< "nl->IsAtom(nl->Third(secondFirst)) " <<
			nl->IsAtom(nl->Third(secondFirst))<<endl;
			cout<< "nl->SymbolValue(nl->Third(secondFirst)) "<<
			nl->SymbolValue(nl->Third(secondFirst))<<endl;
			cout.flush();
		}
		CHECK_COND(nl->ListLength(secondFirst) == 3 &&
			TypeOfRelAlgSymbol(nl->First(secondFirst)) == ccmap &&
			nl->IsAtom(nl->Third(secondFirst)) &&
			nl->SymbolValue(nl->Third(secondFirst)) == "mbool",
			"Operator stpattern expects a list with structure "
			"(map (tuple ((a1 t1)...(an tn))) mbool)\n"
			"Operator stpattern gets a list with "
			"structure '" + argstr + "'.");

		CHECK_COND((nl->Equal(nl->Second(first),
					nl->Second(secondFirst))),
			"Operator stpattern: Input tuple for mapping "
			"(third argument) and the first argument\n"
			"tuple must have the same description." );
	}

	if(debugme)
	{
		cout<<endl<<endl<<"Operator stpattern accepted the input";
		cout.flush();
	}
	return nl->First(args);
}

#endif


#if defined(design_tuple)
ListExpr PatternTypeMap(ListExpr args)
{
	bool debugme= false;
	ListExpr errorInfo;
	errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
	string argstr, argstr2;

	if(debugme)
	{
		cout<<endl<< nl->ToString(args)<<endl;
		cout.flush();
	}

	CHECK_COND(nl->ListLength(args) == 2,
			"Operator stpattern expects a list of length two.");

	ListExpr first = nl->First(args),
	second = nl->Second(args);

	nl->WriteToString(argstr, first);
	if(debugme)
	{
		cout<<endl<< argstr<<endl;
		cout<< "nl->ListLength(first)" << nl->ListLength(first)<<endl;
		cout<<"(TypeOfRelAlgSymbol(nl->First(first))" <<
		TypeOfRelAlgSymbol(nl->First(first))<<endl;
		cout.flush();
	}

	CHECK_COND( (nl->ListLength(first) == 2) &&
		(TypeOfRelAlgSymbol(nl->First(first)) == tuple),
		"Operator stpattern expects as first argument "
		"a list with structure "
		"(tuple ((a1 t1)...(an tn)))\n"
		"Operator stpattern gets a list with structure '" +
		argstr + "'.");

	nl->WriteToString(argstr, second);
	CHECK_COND( ! nl->IsAtom(second) ,
			"Operator  stpattern expects as second argument a "
			"list of functions\n"
			"Operator stpattern gets as second argument '" +
			argstr + "'.\n" );

	ListExpr secondRest = second;
	ListExpr secondFirst, attrType, newAttrList, numberList;
	secondFirst = attrType = newAttrList = numberList = nl->Empty();
	ListExpr lastNewAttrList, lastNumberList;
	lastNewAttrList = lastNumberList = nl->Empty();

	string attrName = "";
	while( !nl->IsEmpty(secondRest) )
	{
		secondFirst = nl->First(secondRest);
		secondRest = nl->Rest(secondRest);

		if(debugme)
		{
			cout<< nl->ToString(secondFirst)<<endl;
			cout.flush();
		}
		nl->WriteToString(argstr, secondFirst);
		CHECK_COND(	nl->IsAtom(secondFirst) &&
				nl->SymbolValue(secondFirst)=="mbool",
			"Operator stpattern expects a list of mbool. "
			"Operator stpattern gets '" + argstr + "'.");
	}

	if(debugme)
	{
		cout<<endl<<endl<<"Operator stpattern accepted the input";
		cout.flush();
	}

	return nl->SymbolAtom("bool");
}
#endif

/*
4.1.2 Value mapping function of operator ~project~

*/

bool Match(MBool* predRes, Instant& stime)
{
	int pos;
	const UBool* cur;
	Intime<CcBool> tinit;
	if(!predRes->IsDefined())	return false;
	pos= predRes->Position(stime);
	predRes->Initial(tinit);
	if(stime < tinit.instant)
		pos=0;
	if(pos== -1)
		return false;
	predRes->Get(pos++,cur );
	while( !cur->constValue.GetBoolval() &&
				pos < predRes->GetNoComponents())
		predRes->Get(pos++, cur);
	if(cur->constValue.GetBoolval())
	{
		stime = (cur->timeInterval.start > stime) ?
					cur->timeInterval.start : stime;
		return true;
	}
	return false;
}

#if defined(design_stream)
int Pattern(Word* args, Word& result, int message, Word& local, Supplier s)
{
	bool debugme=false;
	Word t, value;
	Tuple* tup;
	Supplier supplier,*supplier2,*supplier3;
	int nooffun;
	ArgVectorPointer*funargs;
	DateTime lastMatchTime(instanttype);
	bool matchPred;
	switch (message)
	{
	case OPEN :

		qp->Open(args[0].addr);
		return 0;

	case REQUEST :
		supplier = args[1].addr;
		nooffun = qp->GetNoSons(supplier);
		assert(nooffun>0);
		supplier2= new Supplier[nooffun];
		supplier3= new Supplier[nooffun];
		funargs=new ArgVectorPointer[nooffun];

		for (int i=0; i < nooffun;i++)
		{
			supplier2[i] = qp->GetSupplier(supplier, i);
			supplier3[i] = qp->GetSupplier(supplier2[i], 1);
			funargs[i] = qp->Argument(supplier3[i]);
		}

		qp->Request(args[0].addr,t);
		while (qp->Received(args[0].addr))
		{
			tup = (Tuple*)t.addr;
			lastMatchTime.ToMinimum();
			if(debugme)
				cout<<endl<< "Matching Tuple TID: "<<
					(int)tup->GetTupleId();
			for (int i=0; i < nooffun;i++)
			{
				((*funargs[i])[0]).setAddr(tup);
				qp->Request(supplier3[i],value);
				matchPred= Match((MBool*)value.addr,
								lastMatchTime);
				if(debugme)
				{
					if(matchPred)
						cout<< " matched Pred "<< i <<
						" at time "<<
						lastMatchTime.ToDouble();
					else
						cout<< "didn't match Predicate "
							<< i;
					cout.flush();
				}
				if(!matchPred) break;
			}
			if(!matchPred)
			{
				tup->DeleteIfAllowed();
				qp->Request(args[0].addr,t);
			}
			else
			{
				result.setAddr(tup);
				delete[] supplier2;
				delete[] supplier3;
				delete[] funargs;
				return YIELD;
			}
		}
		return CANCEL;

	case CLOSE :
		qp->Close(args[0].addr);
		return 0;
	}
	return 0;
}
#endif

#if defined(design_tuple)
int Pattern(Word* args, Word& result, int message, Word& local, Supplier s)
{
	bool debugme=false;
	Supplier predlist, pred;
	Word value;
	int noofpred;
	bool matchPred;

	DateTime lastMatchTime(instanttype);

	result = qp->ResultStorage( s );
	predlist = args[1].addr;
	noofpred = qp->GetNoSons(predlist);
	assert(noofpred>0);

	for (int i=0; i < noofpred;i++)
	{
		pred= qp->GetSon(predlist,i);
		qp->Request(pred,value);
		matchPred= Match((MBool*)value.addr,
				lastMatchTime);
		if(debugme)
		{
			if(matchPred)
				cout<< " matched Pred "<< i <<
				" at time "<<
				lastMatchTime.ToDouble();
			else
				cout<< "didn't match Predicate "
				<< i;
			cout.flush();
		}
		if(!matchPred) break;
	}
	((CcBool*)result.addr)->Set(true,matchPred);
	return 0;
}
#endif
/*
4.1.3 Specification of operator ~pattern~

*/

#if defined(design_stream)
OperatorInfo STPatternOperatorInfo( "stpattern",
	"((stream x) ((map1 x mbool1)...(mapn x mbooln))) -> (stream x)",
	"stpattern[p1: lifted predicate , f2: lifted predicate, ... ] ",
	"Naive implementation for spatio temporal pattern queries."
	" It works like filter. Only tuples, fulfilling the "
	"specified pattern are passed on to the output.",
	"query Trains feed stpattern[f1: .Trip inside "
	"msnow, f2: distance(.Trip, mehringdamm) < 10.0 ]   count");
#endif

#if defined(design_tuple)
OperatorInfo STPatternOperatorInfo( "stpattern",
	"((stream x) ((map1 x mbool1)...(mapn x mbooln))) -> (stream x)",
	"stpattern[p1: lifted predicate , f2: lifted predicate, ... ] ",
	"Naive implementation for spatio temporal pattern queries."
	" It works like filter. Only tuples, fulfilling the "
	"specified pattern are passed on to the output.",
	"query Trains feed stpattern[f1: .Trip inside "
	"msnow, f2: distance(.Trip, mehringdamm) < 10.0 ]   count");
#endif
/*

4.1.4 Definition of operator ~pattern~

*/

Operator opdefpattern(  STPatternOperatorInfo,
		Pattern,
		PatternTypeMap );



/*
5 Implementation of the Algebra Class

*/

class STPatternAlgebra : public Algebra
{
public:
	STPatternAlgebra() : Algebra()
	{

		/*

5.2 Registration of Types


		*/


		/*
5.3 Registration of Operators

		*/
		opdefpattern.SetRequestsArguments();
		AddOperator(&opdefpattern);
	}
	~STPatternAlgebra() {};
};


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
7 Examples and Tests

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
