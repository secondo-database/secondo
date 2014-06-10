/*
----  /Tools/TypeMap/Mapper.cpp
---- 

----
This file is part of SECONDO.

Copyright (C) 2014,
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


1 Implementation of the mapper class

*/

#include "OpSigParser.h"
#include "Mapper.h"
#include "ListUtils.h"
#include <string>

using namespace std;


ListExpr opsignats;


namespace typemap{

/*
1.1 Constructor

*/
 Mapper::Mapper(NestedList* _pnl, NestedList* _nl) :
     pnl(_pnl), nl(_nl) {
  }


 Mapper::Mapper(){
 }


/*
1.2 Destructor

*/ 
 Mapper::~Mapper(){}


/*
1.3 init

*/
  bool Mapper::init(const string& path){

    char* inpath = new char[path.length()];
    strcpy(inpath, path.c_str());
    if (!parseSigs(inpath, "../Tools/TypeMap/OpSigs.tmp")) {
    
      return false; // if no success
    }
    
    if (!pnl->ReadFromFile("../Tools/TypeMap/OpSigs.tmp", opsignats)) {
      return false; // if no success
    }
    cout << "List of Signatures: " << endl;
    pnl->WriteStringTo(opsignats, cout);
    cout << endl;

    return true;
  }


/*
1.4 getOpSig

*/

  ListExpr Mapper::getOpSig(const string& algebraName,
                            const string& operatorName){

    // sigArgs = (SigArgTypes, Res, (Decls, Preds))
    ListExpr sigArgs = pnl->TheEmptyList();
    ListExpr opsignatsTemp = opsignats;
    string allSigArgs = "";
    string strSigArgs = "";

    //search in list of sigArgs
    while (pnl->HasMinLength(opsignatsTemp, 1)) {
      ListExpr algSymbol = (pnl->First(pnl->First(opsignatsTemp)));
      ListExpr opSymbol  = (pnl->Second(pnl->First(opsignatsTemp)));
      //(condition command is necessary for programm function)
      if ( (pnl->SymbolValue(algSymbol) == algebraName) &&
           (pnl->SymbolValue(opSymbol) == operatorName) ) {
      } //for query tmtypemap(tmgetOpSig("<algebraName>","<operatorName>"))

      if (pnl->SymbolValue(opSymbol) == operatorName) {
	sigArgs = (pnl->Third(pnl->First(opsignatsTemp)));
	pnl->WriteToString(strSigArgs, sigArgs);
	allSigArgs += strSigArgs;
      } //for query tmtypemap(tmgetOpSig("any","<operatorName>"))

      opsignatsTemp = (pnl->Rest(opsignatsTemp));
    }
    // List of all SigArgs of an operator
    allSigArgs = "(" + allSigArgs + ")";
    pnl->ReadFromString(allSigArgs, sigArgs);

    if (!pnl->IsEmpty(sigArgs)) {
      return sigArgs;
    }
    // if sig not found
    if (operatorName != "any") {
      cout << "Operator \'" << operatorName << "\' not found" << endl;
    }
  
    return pnl->TheEmptyList();
  }


/*
1.5 typemap

*/

  ListExpr Mapper::typemap(const ListExpr SigArgType,
			   const ListExpr CurrentArgTypes){

    /*
    Input 'SigArgType' is not used.
    SigArgType results from tmtypemap entry and 'getOpSig'.

    cout << "List of SigArgType: " << endl;
    pnl->WriteStringTo(SigArgType, cout);
    cout << endl;

    Function for get the 'CurrentArgTypes' in Mapper is not used.
    CurrentArgTypes come from typemap entry.

    cout << "List of Mapper generated CurrentArgTypes: " << endl;
    nl->WriteStringTo(CurrentArgTypes, cout);
    cout << endl;
    */


  /* 
  Main function for OpSignature Typemapping

  */

    // For testing all functions with Terminal input:

    bool entermode = true;
    while (entermode) {

      string sigInput;  //input string
      cout << endl << "Please enter type mapping function"
		      " delimited by a \'.\':"
		      "  (or \'q.\' to quit)" << endl;
      getline(cin, sigInput, '.'); // for string with blanks and '.' at end
      // check quit
      cin.ignore();
      if (sigInput == "q") {
	entermode = false;
      }
      if (!entermode) {
	return nl->TheEmptyList();
      }

      //typemap getOpSig (only for typemap predicate needed)

      ListExpr inList, SigArgType2;
      string opName =  "";
      signed int typemapF = sigInput.find("typemap(");
      if (typemapF != (-1)) {
	//extract listexpression
	signed int pthStart = sigInput.find("(");
	string inputStr = sigInput.substr(pthStart, sigInput.length());
	nl->ReadFromString(inputStr, inList);
	//ueberpruefe Klammern gehe ins UP
	
	//get opName
	nl->WriteToString(opName, nl->First(inList));
	SigArgType2 = Mapper::getOpSig("any", opName);
      }
      else {
	SigArgType2 = pnl->TheEmptyList();
      }

      //show SigArgs of Operation
      cout << "List of SigArgs (" << opName << ") = "; 
      cout << "( (SigArgTypes, Res, (Decls, Preds)) (...) ): " << endl;
      pnl->WriteStringTo(SigArgType2, cout);
      cout << endl << endl;
    
      //convert SigArgType2 from pnl to nl
      string strSigArgs;
      ListExpr sigArgs;
      pnl->WriteToString(strSigArgs, SigArgType2);
      nl->ReadFromString(strSigArgs, sigArgs);
      // sigArgs = ( (SigArgTypes, Res, (Decls, Preds)) (...) )

      // sigArgs is empty by other predicates as 'typemap'

      //Operators with multiple sigArgs: CurrentArgTypes == SigArgTypes ?
      // e.g. '( ( ( int int ) int ) ( ( int real ) real ) ... )
      //        ((SigArgTypes) Res )
      if ( nl->HasMinLength(sigArgs, 2) &&
	   nl->IsAtom(nl->Second(nl->First(sigArgs))) ) {
	ListExpr sigArgsTemp = sigArgs;
	while (nl->HasMinLength(sigArgsTemp, 1)) {
	  if (nl->Equal(nl->Second(inList),
			nl->First(nl->First(sigArgsTemp)))) {
	    sigArgs = nl->First(sigArgsTemp);
	  }
	  sigArgsTemp = (nl->Rest(sigArgsTemp));
	}
      }
      else {
	// sigArgs in predicate 'typemap' generaly
	if (!nl->IsEmpty(sigArgs)) {
	  sigArgs = nl->First(sigArgs);
	}
      }


      // Input in predicate functions

      if (!tmInput(sigInput, sigArgs)) {
	cout << "typeError" << endl;
      }

    } // end while


  /*
  End of main function for OpSignature Typemapping

  */

    return listutils::typeError();
  }


/* 
1.5.1 Functions for preparing input of OpSignature Typemapping

*/


/* 
1.5.2 Functions for predicates of OpSignature Typemapping

*/


  bool tmInput(string sigInput, ListExpr sigArgs) {
    string inputStr;

    //extract listexpression
    signed int pthStart = sigInput.find("(");
    if (pthStart != (-1)) {
      inputStr = sigInput.substr(pthStart, sigInput.length());
    }

    //build NestedList from input
    ListExpr list;
    nl->ReadFromString(inputStr, list);

    //predicate selection
    string predicate = sigInput.substr(0, pthStart+1); //name of predicate
    bool pred = false;

    //typemap
    signed int typemapF = predicate.find("typemap(");
    if (typemapF != (-1)) {
      cout << "Predicate \'typemap\' found." << endl;
      pred = true;

    /*
    typemap(Op, CurrentArgTypes, ResType) :-	  //PROLOG
      (SigArgTypes, Res),			  //args_res from sigs
      matches(CurrentArgTypes, SigArgTypes, Bindings),	//return bindings
      apply(Res, Bindings, ResType).		  //return resType

    // version for complex signatures:

    typemap(Op, CurrentArgTypes, ResType) :-	  //PROLOG	from sigs
      (SigArgTypes, Res, Decls, Preds),		  //args_res_decls_preds 
      defineTypeSets(Decls),
      matches(CurrentArgTypes, SigArgTypes, Bindings),    //return bindings
      evalPreds(Preds, Bindings, Bindings2),		  //return bindings2
      apply(Res, Bindings2, ResType),			  //return resType
      \+ releaseTypeSets.

    */

      ListExpr args_res, args_res_decls_preds, bindings, bindings2, resType;
      //Simple Signatures
      if (nl->HasLength(sigArgs, 2)) {
	// sig
      	args_res = sigArgs;
	cout << "args_res = ";
        nl->WriteStringTo(args_res, cout);
	cout << endl << endl;
	// matches
	bindings = matches(nl->TwoElemList(nl->Second(list),
					   nl->First(args_res)));
	cout << "bindings = ";
        nl->WriteStringTo(bindings, cout);
	cout << endl << endl;
	// apply
	resType = apply(nl->TwoElemList(nl->Second(args_res),
					bindings));
      } //End of sig
      //Complex Signatures
      if (nl->HasMinLength(sigArgs, 3)) {
	// csig
      	args_res_decls_preds = sigArgs;
	cout << "args_res_decls_preds = ";
        nl->WriteStringTo(args_res_decls_preds, cout);
	cout << endl << endl;
	// matches
	bindings = matches(nl->TwoElemList(nl->Second(list),
					   nl->First(args_res_decls_preds)));
	cout << "bindings = ";
        nl->WriteStringTo(bindings, cout);
	cout << endl << endl;
	// evalPreds
	bindings2 = evalPreds(nl->TwoElemList(nl->Fourth(args_res_decls_preds),
					      bindings));
	cout << "bindings2 = ";
        nl->WriteStringTo(bindings2, cout);
	cout << endl << endl;
	// apply
	resType = apply(nl->TwoElemList(nl->Second(args_res_decls_preds),
					bindings2));
      } //End of csig
      cout << "ResultType = ";
      nl->WriteStringTo(resType, cout);
      cout << endl << endl;
    }

    //matches
    signed int matchesF = predicate.find("matches(");
    if (matchesF != (-1)) {
      cout << "Predicate \'matches\' found." << endl;
      pred = true;
      ListExpr matchesList = matches(list);
      cout << "B = ";
      nl->WriteStringTo(matchesList, cout);
      cout << endl;
    }
  
    //evalPreds
    signed int evalPredsF = predicate.find("evalPreds(");
    if (evalPredsF != (-1)) {
      cout << "Predicate \'evalPreds\' found." << endl;
      pred = true;
      ListExpr evalPredsList = evalPreds(list);
      cout << "Bindings2 = ";
      nl->WriteStringTo(evalPredsList, cout);
      cout << endl;
    }
    //evalPred
    signed int evalPredF = predicate.find("evalPred(");
    if (evalPredF != (-1)) {
      cout << "Predicate \'evalPred\' found." << endl;
      pred = true;
      ListExpr evalPredList = evalPred(list);
      cout << "Bindings2 = ";
      nl->WriteStringTo(evalPredList, cout);
      cout << endl;
    }
  
    //apply
    signed int applyF = predicate.find("apply(");
    if (applyF != (-1)) {
      cout << "Predicate \'apply\' found." << endl;
      pred = true;
      ListExpr applyList = apply(list);
      cout << "ResultType = ";
      nl->WriteStringTo(applyList, cout);
      cout << endl;
    }
  
    //consistent
    signed int consistentF = predicate.find("consistent(");
    if (consistentF != (-1)) {
      cout << "Predicate \'consistent\' found." << endl;
      pred = true;
      ListExpr consistentList = consistent(nl->First(list), nl->Second(list));
      cout << "X = ";
      nl->WriteStringTo(consistentList, cout);
      cout << endl;
    }
    //conflict
    signed int conflictF = predicate.find("conflict(");
    if (conflictF != (-1)) {
      cout << "Predicate \'conflict\' found." << endl;
      pred = true;
      bool conf = conflict(nl->First(list), nl->Second(list));
      cout << conf << endl;
    }
    //isAttr
    signed int isAttrF = predicate.find("isAttr(");
    if (isAttrF != (-1)) {
      cout << "Predicate \'isAttr\' found." << endl;
      pred = true;
      ListExpr isAttrList = isAttr(list);
      if (nl->HasLength(list, 4)) {
        if (nl->SymbolValue(nl->Second(isAttrList)) != "Type") {
	  nl->WriteStringTo(nl->Second(list), cout);
	  cout << " = ";
	  nl->WriteStringTo(nl->Second(isAttrList), cout);
	  cout << endl;
	  nl->WriteStringTo(nl->Third(list), cout);
	  cout << " = ";
	  nl->WriteStringTo(nl->Third(isAttrList), cout);
	  cout << endl;
        }
        else {
	  cout << "Error:  attribute ";
	  nl->WriteStringTo(nl->First(list), cout);
	  cout << " does not occur in attribute list ";
	  nl->WriteStringTo(nl->Fourth(list), cout);
	  cout << endl;
        }
      }
    }
    //attrNames
    signed int attrNamesF = predicate.find("attrNames(");
    if (attrNamesF != (-1)) {
      cout << "Predicate \'attrNames\' found." << endl;
      pred = true;
      ListExpr attrNamesList = attrNames(nl->First(list));
      if (nl->HasLength(list, 2)) {
        nl->WriteStringTo(nl->Second(list), cout);
        cout << " = ";
      }
      else {
        cout << "Names = ";
      }
      nl->WriteStringTo(attrNamesList, cout);
      cout << endl;
    }
    //checkMember
    signed int checkMemberF = predicate.find("checkMember(");
    if (checkMemberF != (-1)) {
      cout << "Predicate \'checkMember\' found." << endl;
      pred = true;
      if (!checkMember(list)) {
        cout << "Not a member of the rest of list" << endl;
      }
    }
    //distinctList
    signed int distinctListF = predicate.find("distinctList(");
    if (distinctListF != (-1)) {
      cout << "Predicate \'distinctList\' found." << endl;
      pred = true;
      if (distinctList(list)) {
        cout << "The list is distincted" << endl;
      }
    }  
    //distinctAttrs
    signed int distinctAttrsF = predicate.find("distinctAttrs(");
    if (distinctAttrsF != (-1)) {
      cout << "Predicate \'distinctAttrs\' found." << endl;
      pred = true;
      if (distinctAttrs(list)) {
        cout << "This are distincted Attributes" << endl;
      }
    }  
    //bound
    signed int boundF = predicate.find("bound(");
    if (boundF != (-1)) {
      cout << "Predicate \'bound\' found." << endl;
      pred = true;
      ListExpr boundList = bound(list);
      cout << "X = ";
      nl->WriteStringTo(boundList, cout);
      cout << endl;
    }
    //addBinding
    signed int addBindingF = predicate.find("addBinding(");
    if (addBindingF != (-1)) {
      cout << "Predicate \'addBinding\' found." << endl;
      pred = true;
      ListExpr addBindingList = addBinding(list);
      cout << "B2 = ";
      nl->WriteStringTo(addBindingList, cout);
      cout << endl;
    }
  
    //
    if (pred == false) {
      cout << "No predicate found!" << endl;
      return false;
    }
  
  
    return true;
  
  } // end tmInput
  
/*
----	matches(ArgTypes, Args, Bindings)
----
  
The list of argument types ~ArgTypes~ matches the list of argument type specifications ~Args~ with the bindings ~Bindings~.
  
*/
  ListExpr matches(ListExpr mList) {
    ListExpr bindings, B, Bindings1, Bindings2, Bindings3;
  
    if ( !nl->IsAtom(nl->First(mList)) ) {
      //eliminate multiple round parantheses: (((1 2 3)))->((1 2 3))
      //(((1 2 3))): the first element of list has not MinLength 2
      while (!nl->HasMinLength(nl->First(mList), 2)) {
        mList = (nl->TwoElemList(nl->First(nl->First(mList)),
				 nl->First(nl->Second(mList))));
      }
    }
  
  //cout << "vor var:";
  //nl->WriteStringTo(mList, cout);
  //cout << endl;
  
    if (!nl->IsAtom(nl->Second(mList))) {
  
      if (nl->IsAtom(nl->First(nl->Second(mList)))) {
  
        if (nl->SymbolValue(nl->First(nl->Second(mList))) == "var") {
      
	  //#identifiers, e.g. plz, ort#
	  //matches(Tc, [var, ident, N], [[ident, N, Tc]]) :-
	  // atom(Tc),!.					      //PROLOG
	  if ( nl->IsAtom(nl->First(mList)) &&
	      (nl->SymbolValue(nl->Second(nl->Second(mList))) == "ident") ) {
	    bindings = (nl->OneElemList(
	    nl->ThreeElemList(nl->Second(nl->Second(mList)),
			      nl->Third(nl->Second(mList)),
			      nl->First(mList))));
	  }
	  if (!nl->IsAtom(nl->First(mList))) {
	    //#type constructor applied to arguments matches variable#
	    // matches([Tc | List], [var, Tc, N], [[Tc, N, [Tc | List]]])
	    //PROLOG
	    if (nl->Equal(nl->First(nl->First(mList)),
			  nl->Second(nl->Second(mList)))) {
	      bindings = (nl->OneElemList(
			    nl->ThreeElemList(nl->Second(nl->Second(mList)),
					      nl->Third(nl->Second(mList)),
					      nl->First(mList))));
	    }
	  }
  
	  return bindings;
  
        } // end if "var"
  
  //cout << "vor any:";
  //nl->WriteStringTo(mList, cout);
  //cout << endl;
  
        //#free variable matching anything#
        //matches(X, [any, Var, N], [[Var, N, X]]).	//PROLOG
        if (nl->SymbolValue(nl->First(nl->Second(mList))) == "any") {  
	  bindings = (nl->OneElemList(
			nl->ThreeElemList(nl->Second(nl->Second(mList)),
			nl->Third(nl->Second(mList)),
			nl->First(mList))));
  //cout << "any-Bindings:";
  //nl->WriteStringTo(bindings, cout);
  //cout << endl;
        }
  
      } // end if (nl->IsAtom(nl->First(nl->Second(mList))))
  
    } // end if (!nl->IsAtom(nl->Second(mList)))
  
  //cout << "vor 7:";
  //nl->WriteStringTo(mList, cout);
  //cout << endl;
    
    if ( !nl->IsAtom(nl->First(mList)) ) {
  
      //matches([Tc | List], [Tc | Rest], Bindings):-   //PROLOG
      //  matches(List, Rest, Bindings),!.
      if (nl->Equal(nl->First(nl->First(mList)),
		    nl->First(nl->Second(mList)))) {
        mList = (nl->TwoElemList(nl->Second(nl->First(mList)),
				 nl->Second(nl->Second(mList))));
        bindings = matches(mList);
      }
    }
    if ( nl->IsAtom(nl->First(mList)) ) {
      if (nl->Equal(nl->First(mList),
		    nl->Second(mList))) {
        mList = (nl->TwoElemList(nl->Empty(),
				 nl->Empty()));	       
      }
      //matches([], [], []).			    //PROLOG
      if (nl->IsEmpty(nl->First(mList)) &&
	    nl->IsEmpty(nl->Second(mList))) {
        bindings =  (nl->Empty());
        return bindings;
      }
  
    }
    
  //cout << "vor 9:";
  //nl->WriteStringTo(mList, cout);
  //cout << endl;
    
    //matches([ArgType | ArgTypes], [Arg | Args], Bindings) :-  //PROLOG
    //  matches(ArgType, Arg, B),
    //  matches(ArgTypes, Args, Bindings1),
    //  consistent(B, Bindings1, Bindings).
    if (nl->HasMinLength(nl->First(nl->First(mList)), 2)) {	       
      if (!nl->IsAtom(nl->Second(nl->First(nl->First(mList)))) ) {
        B = matches(nl->TwoElemList(nl->First(nl->First(mList)),
				    nl->First(nl->Second(mList))));
        Bindings1 = matches(nl->TwoElemList(nl->Second(nl->First(mList)),
					    nl->Second(nl->Second(mList))));
        //bindings = consistent(B, Bindings1);
    
        if (!nl->Equal(B, Bindings1)) {
	  bindings = (nl->TwoElemList(nl->First(B), nl->First(Bindings1)));
        }
        else {
	  bindings = B;
        }
  //cout << "Append-Bindings-2elementig:";
  //nl->WriteStringTo(bindings, cout);
  //cout << endl;      
      }
    }
  
    if ( nl->HasMinLength(nl->First(mList), 3) && 
         nl->IsAtom(nl->Third(nl->First(mList))) )  {
      Bindings2 = matches(nl->TwoElemList(nl->Third(nl->First(mList)),
					  nl->Third(nl->Second(mList))));
      Bindings3 = matches(nl->TwoElemList(nl->Fourth(nl->First(mList)),
					  nl->Fourth(nl->Second(mList))));
      if (!nl->Equal(B, Bindings1)) {					
        bindings = (nl->FourElemList(nl->First(B),
				     nl->First(Bindings1),
				     nl->First(Bindings2),
				     nl->First(Bindings3)));
      }				   
  //cout << "Append-Bindings-4elementig:";
  //nl->WriteStringTo(bindings, cout);  
  //cout << endl;				 
    }
  
  //cout << "nach 9:";
  //nl->WriteStringTo(mList, cout);
  //cout << endl;
    
    return bindings;
  }
  
/*
Two lists of bindings ~B1~ and ~B2~ are consistent, if their sets of variables are disjoint or for equal variables they have the same values. The joint bindings are returned in ~Bindings~.
  
*/
  ListExpr consistent(ListExpr B1, ListExpr B2) {
    ListExpr Bindings;
    
    if ( nl->Equal(B1, B2) ) {
      Bindings = B1;
    }
    else {
      conflict(B1, B2);
      Bindings = nl->Empty();
    }
    return Bindings;
  }
  
/*
Two bindings ~B1~ and ~B2~ are in conflict if they have the same variable but different values.
  
*/
  bool conflict(ListExpr B1, ListExpr B2) {	
    bool conf = false;
  
    //conflict([Tc, N, X], [Tc, N, Y]):-	      //PROLOG
    if ( !nl->Equal(B1, B2) ) {
      conf = true;
      cout << "Conflict between types" << endl;
    }
    return conf;
  }
  
/*
Evaluation of Predicates
  
----	evalPreds(Preds, Bindings, Bindings2)
----
  
Evaluate predicates ~Preds~ based on ~Bindings~, resulting in new ~Bindings2~.
  
*/
  ListExpr evalPreds(ListExpr ePsList) {
    ListExpr bindings3;
    ListExpr Bindings, Bindings2, Bindings3;
    
    //evalPreds([], Bindings, Bindings).
    //evalPreds([Pred | Preds], Bindings, Bindings3) :-
    //  evalPred(Pred, Bindings, Bindings2),
    //  evalPreds(Preds, Bindings2, Bindings3).	  //PROLOG
    if (nl->IsEmpty(nl->First(ePsList))) {
      Bindings = (nl->Second(ePsList));
      bindings3 = Bindings;
    }
    else {
      Bindings2 = evalPred(nl->TwoElemList(nl->First(nl->First(ePsList)),
					   nl->Second(ePsList)));
      Bindings3 = evalPreds(nl->TwoElemList(nl->Rest(nl->First(ePsList)),
					    Bindings2));
      bindings3 = Bindings3;					 
    }
  
    return bindings3;
  }
  
  
  ListExpr evalPred(ListExpr ePList) {
    ListExpr bindings2;
    ListExpr No1, No2, No3, No4, Bindings, Bindings2, Bindings3;
    if (nl->HasMinLength(nl->First(ePList), 2)) {
      No1 = (nl->IntValue(nl->Third(nl->Second(nl->First(ePList)))));
    }
    if (nl->HasMinLength(nl->First(ePList), 3)) {
      No2 = (nl->IntValue(nl->Third(nl->Third(nl->First(ePList)))));
    }
    if (nl->HasMinLength(nl->First(ePList), 4)) {
      No3 = (nl->IntValue(nl->Third(nl->Fourth(nl->First(ePList)))));
    }
    if (nl->HasMinLength(nl->First(ePList), 5)) {
      No4 = (nl->IntValue(nl->Third(nl->Fifth(nl->First(ePList)))));
    }
    Bindings = (nl->Second(ePList));
    
    /*	      PROLOG
    evalPred(
      [attr, [var, ident, No1], [var, attrs, No2], 
        [var, attrType, No3], [var, attrNo, No4]],
      Bindings, Bindings3) :-
      bound(Bindings, [var, ident, No1], Ident),
      bound(Bindings, [var, attrs, No2], Attrs),
      isAttr(Ident, Type, No, Attrs),
      addBinding(Bindings, [var, attrType, No3], Type, Bindings2),
      addBinding(Bindings2, [var, attrNo, No4], No, Bindings3).
  
    */
    if (nl->SymbolValue(nl->First(nl->First(ePList))) == "attr") {
      ListExpr Ident, Attrs, isAttrList, Type, No;
  
      Ident = bound(nl->TwoElemList(Bindings,
				    nl->ThreeElemList(
				      nl->SymbolAtom("var"),
				      nl->SymbolAtom("ident"),
				      nl->IntAtom(No1))));
				     
      Attrs = bound(nl->TwoElemList(Bindings,
				    nl->ThreeElemList(
				      nl->SymbolAtom("var"),
				      nl->SymbolAtom("attrs"),
				      nl->IntAtom(No2))));
      isAttrList = isAttr(nl->FourElemList(Ident,
					   nl->SymbolAtom("Type"),
					   nl->SymbolAtom("No"),
					   Attrs));	 
      Type = (nl->Second(isAttrList));
      No = (nl->Third(isAttrList));
      Bindings2 = addBinding(nl->ThreeElemList(Bindings,
					       nl->ThreeElemList(
						 nl->SymbolAtom("var"),
						 nl->SymbolAtom("attrType"),
						 nl->IntAtom(No3)),
					       Type));
      Bindings3 = addBinding(nl->ThreeElemList(Bindings2,
					       nl->ThreeElemList(
						 nl->SymbolAtom("var"),
						 nl->SymbolAtom("attrNo"),
						 nl->IntAtom(No4)),
					       No));  
      bindings2 = Bindings3;
    }
  
    /*	      PROLOG
    evalPred(
      [concat, [var, attrs, No1], [var, attrs, No2], [var, attrs, No3]],
      Bindings, Bindings2) :-
      bound(Bindings, [var, attrs, No1], List1),
      bound(Bindings, [var, attrs, No2], List2),
      append(List1, List2, List3),
      addBinding(Bindings, [var, attrs, No3], List3, Bindings2).
  
    */
    if (nl->SymbolValue(nl->First(nl->First(ePList))) == "concat") {
      ListExpr List1, List2, List3, list3;
  

      List1 = bound(nl->TwoElemList(Bindings,
				    nl->ThreeElemList(
				      nl->SymbolAtom("var"),
				      nl->SymbolAtom("attrs"),
				      nl->IntAtom(No1))));
      List2 = bound(nl->TwoElemList(Bindings,
				    nl->ThreeElemList(
				      nl->SymbolAtom("var"),
				      nl->SymbolAtom("attrs"),
				      nl->IntAtom(No2))));

      // because the pointer (List3 = List1) changes Bindings
      string strList1;
      nl->WriteToString(strList1, List1);
      nl->ReadFromString(strList1, List3);
      list3 = List3;
      // append
      while (!nl->IsEmpty(List2)) {
        list3 = (nl->Append(nl->End(list3), nl->First(List2)));
        List2 = (nl->Rest(List2));
      }
      Bindings2 = addBinding(nl->ThreeElemList(Bindings,
					       nl->ThreeElemList(
						 nl->SymbolAtom("var"),
						 nl->SymbolAtom("attrs"),
						 nl->IntAtom(No3)),
					       List3));
      bindings2 = Bindings2;
    }
  
    /*	      PROLOG
    evalPred(
      [distinctAttrs, [var, attrs, No]],
      Bindings, Bindings) :-
      bound(Bindings, [var, attrs, No], Attrs),
      distinctAttrs(Attrs).
  
    */
    if (nl->SymbolValue(nl->First(nl->First(ePList))) == "distinctAttrs") {
      ListExpr Attrs;
  
      Attrs = bound(nl->TwoElemList(Bindings,
				    nl->ThreeElemList(
				      nl->SymbolAtom("var"),
				      nl->SymbolAtom("attrs"),
				      nl->IntAtom(No1))));

      if (!distinctAttrs(nl->OneElemList(Attrs))) {
        bindings2 = nl->Empty();
      }
      else {
        bindings2 = Bindings;
      }
    }
  
  
    return bindings2;
  }
  
  ListExpr isAttr(ListExpr attrList) {
    ListExpr attrList2;
  
    //attrList:=(Ident, Type, No, List)
    attrList2 = (nl->FourElemList(nl->First(attrList),
				  nl->Second(attrList),
				  nl->IntAtom(1),
				  nl->Fourth(attrList)));
    attrList = isAttr2(attrList2);
  
    return attrList;
  }
  
  ListExpr isAttr2(ListExpr attrList2) {
    int N = nl->IntValue(nl->Third(attrList2));
  
    //attrList2:=(Ident, Type, No, List)
    if (nl->Equal(nl->First(nl->First(nl->Fourth(attrList2))), 
		  nl->First(attrList2))) {
      attrList2 = (nl->FourElemList(nl->First(attrList2),
				nl->Second(nl->First(nl->Fourth(attrList2))),
				nl->Third(attrList2),
				nl->Fourth(attrList2)));
      return attrList2;
    }
    else {
      if (!nl->IsEmpty(nl->Rest(nl->Fourth(attrList2)))) {
        N++;
        attrList2 = (nl->FourElemList(nl->First(attrList2),
				      nl->Second(attrList2),
				      nl->IntAtom(N),
				      nl->Rest(nl->Fourth(attrList2))));
        attrList2 = isAttr2(attrList2);
      }
    }
  
    return attrList2;
  }
  
  ListExpr attrNames(ListExpr attrNList) { 
    ListExpr names, Names;
  
    //attrNames([ [Ident, _] | Rest], [Ident | Names]) :-
    //  attrNames(Rest, Names).				    //PROLOG
    if (!nl->IsEmpty(attrNList)) {
      names = (nl->OneElemList(nl->First(nl->First(attrNList))));
      Names = names;
      attrNList = (nl->Rest(attrNList));
    }
    while (!nl->IsEmpty(attrNList)) {
      Names = (nl->Append(Names, nl->First(nl->First(attrNList))));
      attrNList = (nl->Rest(attrNList));
    }
  
    return names;
  }
  
  bool checkMember(ListExpr cMList) {
    ListExpr cMList2 = (nl->Second(cMList));
  
    //checkMember(Name, Names) :-
    //  member(Name, Names).		      		    //PROLOG
    while (!nl->IsEmpty(cMList2)) {
      if (nl->IsEqual(nl->First(cMList), nl->ToString(nl->First(cMList2)))) {
        cout << "Error:  attribute \'";
        nl->WriteStringTo(nl->First(cMList), cout);
        cout << "\' occurs among attributes ";
        nl->WriteStringTo(cMList2, cout);
        cout << endl;
        return true;
      }
      cMList2 = (nl->Rest(cMList2));
    }
    return false;
  }
  
  bool distinctList(ListExpr distLList) {
    ListExpr distLList2;
  
    //distinctList([Elem | Rest]):-
    //  \+ checkMember(Elem, Rest),
    //  distinctList(Rest).		      		    //PROLOG
    while (!nl->IsEmpty(nl->First(distLList))) {
      distLList2 = (nl->TwoElemList(nl->First(nl->First(distLList)),
				    nl->Rest(nl->First(distLList))));
      if (checkMember(distLList2)) {
        return false;
      }
      // if no more occurance of this attribut
      distLList = (nl->OneElemList(nl->Rest(nl->First(distLList))));
    }
    return true;
  }
  
  bool distinctAttrs(ListExpr distAList) {
    ListExpr Names;
  
    //distinctAttrs(Attrs):-
    //  attrNames(Attrs, Names),
    //  distinctList(Names).				    //PROLOG
    Names = attrNames(nl->First(distAList));
    if (distinctList(nl->OneElemList(Names))) { 
      return true;
    }
    return false;
  }
  
/*
----	bound(Bindings, [var, Tc, No], Bound)
        bound(Bindings, [lvar, Tc, No], Bound)
----
  
  The first version finds a binding for a given variable if it exists. The second version is used for list variables. For them, all bindings of the form [Tc, [No, i], X\_i] will be collected into a list [X\_1, ..., X\_n] and be returned in ~Bound~. 
  
*/
  ListExpr bound(ListExpr boList) {
  
    //bound([ [Tc, No, X] | _], [var, Tc, No], X) :- !.	    //PROLOG
    if (nl->Equal(nl->First(nl->First(nl->First(boList))),
		  nl->Second(nl->Second(boList))) &&
        nl->Equal(nl->Second(nl->First(nl->First(boList))),
		  nl->Third(nl->Second(boList)))) {
      boList = (nl->Third(nl->First(nl->First(boList))));

      return boList;
    }
    if (!nl->IsEmpty(nl->Rest(nl->First(boList)))) {
      boList = bound(nl->TwoElemList(nl->Rest(nl->First(boList)),
				     nl->Second(boList)));
    }
    else {
      cout << "Error: no binding found for variable ";
      nl->WriteStringTo(nl->Second(nl->Second(boList)), cout);
      cout << "_";
      nl->WriteStringTo(nl->Third(nl->Second(boList)), cout);
      cout << endl;
    }
  
    return boList;
  }
  
/*
----	addBinding(Bindings, [var, Tc, N], Type, Bindings2)
----
  
*/
  ListExpr addBinding(ListExpr aBList) {
    ListExpr first, bindings2, Bindings, Bindings2;
  
    //addBinding(Bindings, [var, Tc, N], Type, Bindings2) :-
    //  consistent([[Tc, N, Type]], Bindings, Bindings2).	    //PROLOG
    first = (nl->ThreeElemList(nl->Second(nl->Second(aBList)),
			       nl->Third(nl->Second(aBList)),
			       nl->Third(aBList)));
    Bindings = (nl->First(aBList));
    string strFirst, strSecond, strBindings, strBindings2;
    nl->WriteToString(strFirst, first);  
    while (!nl->IsEmpty(Bindings)) {
      nl->WriteToString(strBindings, (nl->First(Bindings)));
      strSecond = strSecond + " " + strBindings;
      Bindings = (nl->Rest(Bindings));
    }
    strBindings2 = "(" + strFirst + " " + strSecond + ")";
    nl->ReadFromString(strBindings2, Bindings2);
    bindings2 = Bindings2;
  
    return bindings2;
  }
  
/*
Applying the ~bindings~ to the result type specification ~res~ yields
 the result type ~resType~.
  
*/
  ListExpr apply(ListExpr aList) {
    ListExpr resType, Type, ArgTypes;
  
  //cout << "vor var:";
  //nl->WriteStringTo(aList, cout);
  //cout << endl;
  
    if ( !nl->IsAtom(nl->First(aList)) ) {
  
      if (nl->SymbolValue(nl->First(nl->First(aList))) == "var") {
  
        //apply([var, Tc, N], [], typeerror) :-  !,	//PROLOG
        if (nl->IsEmpty(nl->Second(aList))) {
	  cout << "Error: no binding for variable ";
	  nl->WriteStringTo(nl->Second(nl->First(aList)), cout);
	  cout << "_";
	  nl->WriteStringTo(nl->Third(nl->First(aList)), cout);
	  cout << " found." << endl;
	  resType = listutils::typeError();
	  return resType;
        }	
        //apply([var, Tc, N], [ [Tc, N, Type] | _], Type) //PROLOG
        if (nl->SymbolValue(nl->Second(nl->First(aList))) == 
	      nl->SymbolValue(nl->First(nl->First(nl->Second(aList)))) &&
	    nl->IntValue(nl->Third(nl->First(aList))) == 
	      nl->IntValue(nl->Second(nl->First(nl->Second(aList))))) {
	  Type = nl->Third(nl->First(nl->Second(aList)));
	  resType = Type;
	  return resType;
        }
        //apply([var, Tc, N], [ _ | Rest], Type) :-
        // !, apply([var, Tc, N], Rest, Type).		//PROLOG
        if (!nl->IsEmpty(nl->Rest(nl->Second(aList)))) {
	  Type = apply(nl->TwoElemList(nl->First(aList),
				       nl->Rest(nl->Second(aList))));
	  resType = Type;
        }
        else {
	  resType = aList;
	  return resType;
        }
      } // end if "var"
  
  //cout << "vor 4:";
  //nl->WriteStringTo(aList, cout);
  //cout << endl;
  
      //apply([Tc , List], Bindings, [Tc , Type]) :-
      //  apply(List, Bindings, Type).		  //PROLOG
      if (nl->HasLength(nl->First(aList), 2)) {
        ListExpr ListBindings = (nl->TwoElemList(nl->Second(nl->First(aList)),
						 nl->Second(aList)));
        Type = apply(ListBindings);
        if (nl->IsEqual(Type, "typeerror")) {
	  return resType = Type;
        }
        else {
	  resType = (nl->TwoElemList(nl->First(nl->First(aList)), Type));
        }
      }
  
  //cout << "vor 7:";
  //nl->WriteStringTo(aList, cout);
  //cout << endl;
  
      //apply([append, Extra, Res], B, [append, ExtraArgs, ResultType]) :-
      //  apply(Extra, B, ExtraArgs),
      //  apply(Res, B, ResultType).		  //PROLOG
      if (nl->SymbolValue(nl->First(nl->First(aList))) == "append") {
        ListExpr ExtraArgs = apply(nl->TwoElemList(nl->Second(nl->First(aList)),
						   nl->Second(aList)));
        ListExpr ResultType = apply(nl->TwoElemList(nl->Third(nl->First(aList)),
						    nl->Second(aList)));
        resType = (nl->ThreeElemList(nl->SymbolAtom("append"),
				     ExtraArgs,
				     ResultType));
      }
  
    } // end if ( !nl->IsAtom(nl->First(aList)) )
  
  //cout << "vor 5:";
  //nl->WriteStringTo(aList, cout);
  //cout << endl;
  
    //apply(ArgTypes, [], ArgTypes).		  //PROLOG
    if (nl->IsEmpty(nl->Second(aList))) {
      ArgTypes = (nl->First(aList));
      resType = ArgTypes;
    }
  
  
    return resType;
  }
  
  
  
  /* 
    End of functions for predicates of OpSignature Typemapping
  
  */
  


} // end of namespace typemape







