/*
----  /Tools/TypeMap/OpSigParser.y
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

*/


%{

/*
Includes

*/
#include "OpSigParser.tab.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstring>

#ifdef YYDEBUG
#undef YYDEBUG
#endif
#define YYDEBUG 1

#ifdef YYERROR_VERBOSE
#undef YYERROR_VERBOSE
#endif
#define YYERROR_VERBOSE 1

using namespace std;

/*
Some variables 

*/
extern FILE* opsigin;
//const char* infile = "../Tools/TypeMap/sigs";
//string outfile = "../Tools/TypeMap/OpSigs.tmp";
string outfile2 = "../Tools/TypeMap/OpSigsOpdPar.tmp";
string outfile3 = "../Tools/TypeMap/OpSigsArgs.tmp";
ofstream ofile;
ofstream ofile2;
ofstream ofile3;

extern int opsiglex();
void opsigerror( const char* s ) {
cerr << endl << s << endl << endl;
}

string varToLower(string varNameIn);

vector<string> opsigs;		vector<string> opsigs2;
				vector<string> opsigs3;

/*
Type string is not possible in union

*/
string collectSig     = "";	string collectSig2     = "";
string collectSiglist = "";	string collectSiglist2 = "";
string collectArgs    = "";	string collectArgs2    = "";
string collectRes     = "";
string collectPreds   = "";	string collectSig3     = "";
string collectDecls   = "";	string collectSiglist3 = "";
string collectEnum    = "";	string collectArgs3    = "";

%}

%union {
  const char* tokenchar;
}

/*
 use opsig instead of yy to avoid naming conflicts with other 
 parsers of the system

*/
%name-prefix="opsig"

%token ZZALG ZZWHERE ZZIN ZZPARAM ZZCROSSPRODUCT ZZFOLLOWS ZZSEMICOLON ZZERROR

%token<tokenchar> ZZATTR ZZATTRS ZZCOMBINE ZZCONCAT ZZDISTATTRS
		  ZZMINUS ZZCREATEATTR
		  ZZIDENT ZZSYMBOL ZZINTI

%type<tokenchar> opname type varname attrsindex varindex datatypes datatype

/*
----   The output is specified as follows:
       <algsig>   = <algebraname> <sig>
       <sig>      = (<opname>|<opsymbol>) <siglist>
       <siglist>  = <sigargtypes> <resulttype>
                  | <sigargtypes> <resulttype>; <condpreds>
                  | <sigargtypes> <resulttype> <decls>.
----

*/

%%

start	    : signatures
	    ;

signatures  : algsig
	    | signatures algsig
	    ;

algsig	    : ZZALG ZZIDENT sigs
	      {
		string str2 = $2;
		/* Output in outfile */
		string algSigs = "";
		int size = opsigs.size();
		for(int i=0; i<size; i++) {
		  algSigs += "\n(" + str2 + " " + opsigs[i];
		}
		ofile << algSigs;
		opsigs.clear();
		algSigs	= "";

		/* Output in file OpSigsOpdPar.tmp where
		   operand and parameter are separated */
		string algSigs2 = "";
		int size2 = opsigs2.size();
		for(int i=0; i<size2; i++) {
		  algSigs2 += "\n(" + str2 + " " + opsigs2[i];
		}
		ofile2 << algSigs2;
		opsigs2.clear();
		algSigs2 = "";

		/* Output in file OpSigsArgs.tmp for
		   QueryconstructionViewer          */
		string algSigs3 = "";
		int size3 = opsigs3.size();
		for(int i=0; i<size3; i++) {
		  algSigs3 += "\n(" + str2 + " " + opsigs3[i];
		}
		//ofile3 << algSigs3;
		opsigs3.clear();
		algSigs3 = "";
	      }
	    ;

sigs	    : sig
	      {
		opsigs.push_back(collectSig);
		opsigs2.push_back(collectSig2);
		opsigs3.push_back(collectSig3);
	      }
	    | sigs sig
	      {
		opsigs.push_back(collectSig);
		opsigs2.push_back(collectSig2);
		opsigs3.push_back(collectSig3);
	      }
	    ;

sig	    : opname':' sigargtypes ZZFOLLOWS resulttype
	      {/* sig: simple */
		string str1 = $1;
		collectSiglist = ""; collectSig = "";
		collectSiglist += "( " + collectArgs + " )\n ";
		collectSiglist +=  collectRes;
		collectSig  += str1 + " \n ( " + collectSiglist + " )";
		collectSig  += " )\n";
		collectSiglist2 = ""; collectSig2 = "";
		collectSiglist2 += "( " + collectArgs2 + " )\n ";
		collectSiglist2 +=  collectRes;
		collectSig2  += str1 + " \n ( " + collectSiglist2 + " )";
		collectSig2  += " )\n";
		collectSiglist3 = ""; collectSig3 = "";
		collectSiglist3 += collectArgs3;
		collectSig3  += str1 + " " + collectSiglist3;
		collectSig3  += ")\n";
	      }

	    | opname':' sigargtypes ZZFOLLOWS resulttype semicolon condpreds
	      {/* sig: simple + condpreds */
		string str1 = $1;
		collectSiglist = ""; collectSig = "";
		collectSiglist += "( " + collectArgs + " )\n ";
		collectSiglist +=  collectRes  + "\n ";
		collectSiglist += "()\n ";  // without decls
		collectSiglist += "( " + collectPreds + " )";
		collectSig  += str1 + " \n ( " + collectSiglist + " )";
		collectSig  += " )\n";
		collectSiglist2 = ""; collectSig2 = "";
		collectSiglist2 += "( " + collectArgs2 + " )\n ";
		collectSiglist2 +=  collectRes  + "\n ";
		collectSiglist2 += "()\n ";  // without decls
		collectSiglist2 += "( " + collectPreds + " )";
		collectSig2  += str1 + " \n ( " + collectSiglist2 + " )";
		collectSig2  += " )\n";
		collectSiglist3 = ""; collectSig3 = "";
		collectSiglist3 += collectArgs3;
		collectSig3  += str1 + " " + collectSiglist3;
		collectSig3  += ")\n";
	      }

	    | opname':' sigargtypes ZZFOLLOWS resulttype decls '.'
	      {/* sig: simple + decls */
		string str1 = $1;
		collectSiglist = ""; collectSig = "";
		collectSiglist += "( " + collectArgs + " )\n ";
		collectSiglist +=  collectRes  + "\n ";
		collectSiglist += "( " + collectDecls + " )\n ";
		collectSiglist += "()\n ";  // without condpreds
		collectSig  += str1 + " \n ( " + collectSiglist + " )";
		collectSig  += " )\n";
		collectSig2 = collectSig;
		collectSiglist3 = ""; collectSig3 = "";
		collectSiglist3 += collectArgs3;
		collectSig3  += str1 + " " + collectSiglist3;
		collectSig3  += ")\n";
	      }
	    ;

sigargtypes : type ZZCROSSPRODUCT type
	      {/* argsOP: e.g. + */
		string str1 = $1; string str3 = $3;
		collectArgs = "";
		collectArgs += str1 + " " + str3;
		collectArgs2 = "";
		collectArgs2 = collectArgs;
		collectArgs3 = "";
		collectArgs3 += "\"" + str1 + ";" + str3 + "\" \"\"";
	      }

	    | type'('varname')'
	      {/* argsOP: e.g. feed/consume */
		string str1 = $1; string str3 = $3;
		string str32 = "";
		str32 = varToLower(str3);
		collectArgs = "";
		collectArgs += "(" + str1 + " ";
		collectArgs += "(var " + str32 + " 1))";
		collectArgs2 = "";
		collectArgs2 = collectArgs;
		collectArgs3 = "";
		collectArgs3 += "\"" + str1 + "\" \"\"";
	      }

	    | type'('varname')' ZZCROSSPRODUCT type
	      {/* argsOP: e.g. atinstant */
		string str1 = $1; string str3 = $3;
		string str6 = $6;
		string str32 = "";
		str32 = varToLower(str3);
		collectArgs = "";
		collectArgs += "(" + str1 + " ";
		collectArgs += "(var " + str32 + " 1)) " + str6;
		collectArgs2 = "";
		collectArgs2 = collectArgs;
		collectArgs3 = "";
		collectArgs3 += "\"" + str1 + "\" \"\"";
	      }
  
	    | type'('varname')' ZZPARAM 
		  '('varname ZZFOLLOWS type')'
	      {/* argsOP: e.g. filter */
		string str1 = $1;  string str3 = $3;
		string str7 = $7;  string str9 = $9;
		string str32 = ""; string str72 = "";
		str32 = varToLower(str3); str72 = varToLower(str7);
		collectArgs = "";
		collectArgs += "(" + str1 + " ";
		collectArgs += "(var " + str32 + " 1)) ";
		collectArgs += "(map ";
		collectArgs += "(var " + str72 + " 1) " + str9 + ")";
		collectArgs2 = "";
		collectArgs2 += "((" + str1 + " ";
		collectArgs2 += "(var " + str32 + " 1))) ";
		collectArgs2 += "((map ";
		collectArgs2 += "(var " + str72 + " 1) " + str9 + "))";
		collectArgs3 = "";
		collectArgs3 += "\"" + str1 + "\" \"" + str9 + "\"";
	      }

	    | type'('varname')' ZZPARAM varname
	      {/* argsOP: e.g. attr */
		string str1 = $1;  string str3 = $3;
		string str6 = $6;
		string str32 = ""; string str62 = "";
		str32 = varToLower(str3); str62 = varToLower(str6);
		collectArgs = "";
		collectArgs += "(" + str1 + " ";
		collectArgs += "(any " + str32 + " 1)) ";
		collectArgs += "(var " + str62 + " 1)";
		collectArgs2 = "";
		collectArgs2 += "((" + str1 + " ";
		collectArgs2 += "(any " + str32 + " 1))) ";
		collectArgs2 += "((var " + str62 + " 1))";
		collectArgs3 = "";
		collectArgs3 += "\"" + str1 + "\" \"\"";
	      }

	    | type'('type'('varname attrsindex')' ')' ZZCROSSPRODUCT
	      type'('type'('varname attrsindex')' ')' 
	      ZZPARAM varname varindex ZZCROSSPRODUCT varname varindex
	      {/* argsOP: e.g. hashjoin */
		string str1 = $1;   string str3 = $3;
		string str5 = $5;   string str6 = $6;
		string str10 = $10; string str12 = $12;
		string str14 = $14; string str15 = $15;
		string str19 = $19; string str20 = $20;
		string str22 = $22; string str23 = $23;
		string str52 = "";  string str142 = "";
		string str192 = ""; string str222 = "";
		str52 = varToLower(str5);   str142 = varToLower(str14);
		str192 = varToLower(str19); str222 = varToLower(str22);
		collectArgs = "";
		collectArgs += "(" + str1 + " (" + str3 + " ";
		collectArgs += "(any " + str52 + " " + str6 + ")))\n     ";
		collectArgs += "(" + str10 + " (" + str12 + " ";
		collectArgs += "(any " + str142 + " " + str15 + ")))\n     ";
		collectArgs += "(var " + str192 + " " + str20 + ")  ";
		collectArgs += "(var " + str222 + " " + str23 + ")";
		collectArgs2 = "";
		collectArgs2 += "((" + str1 + " (" + str3 + " ";
		collectArgs2 += "(any " + str52 + " " + str6 + ")))\n     ";
		collectArgs2 += " (" + str10 + " (" + str12 + " ";
		collectArgs2 += "(any " + str142 + " " + str15 + "))))\n     ";
		collectArgs2 += "((var " + str192 + " " + str20 + ")  ";
		collectArgs2 += "(var " + str222 + " " + str23 + "))";
		collectArgs3 = "";
		collectArgs3 += "\"" + str1 + ";" + str10 + "\" ";
		collectArgs3 += "\"attr;attr\"";
	      }

	    | type'('type'('varname attrsindex')' ')' 
	      ZZPARAM '('varname varindex')' ZZSYMBOL
	      {/* argsOP: e.g. project */
		string str1 = $1;   string str3 = $3;
		string str5 = $5;   string str6 = $6;
		string str11 = $11; string str12 = $12;
		string str14 = $14;
		string str52 = "";  string str112 = "";
		str52 = varToLower(str5); str112 = varToLower(str11);
		collectArgs = "";
		collectArgs += "(" + str1 + " (" + str3 + " ";
		collectArgs += "(any " + str52 + " " + str6 + ")))\n     ";
		collectArgs += "(" + str14 + " ";
		collectArgs += "(lvar " + str112 + " " + str12 + "))";
		collectArgs2 = "";
		collectArgs2 += "((" + str1 + " (" + str3 + " ";
		collectArgs2 += "(any " + str52 + " " + str6 + "))))\n     ";
		collectArgs2 += "((" + str14 + " ";
		collectArgs2 += "(lvar " + str112 + " " + str12 + ")))";
		collectArgs3 = "";
		collectArgs3 += "\"" + str1 + "\" \"attrlist\"";
	      }

	    | type'('type'('varname attrsindex')' ')' 
	      ZZPARAM '('varname varindex ZZCROSSPRODUCT
	          '('type'('varname attrsindex')' ZZFOLLOWS
		 varname varindex')' ')' ZZSYMBOL
	      {/* argsOP: e.g. extend */
		string str1 = $1;   string str3 = $3;
		string str5 = $5;   string str6 = $6;
		string str11 = $11; string str12 = $12;
		string str15 = $15; 
		string str17 = $17; string str18 = $18;
		string str21 = $21; string str22 = $22;
		string str25 = $25; 
		string str52 = "";  string str112 = "";
		string str172 = ""; string str212 = "";
		str52 = varToLower(str5);   str112 = varToLower(str11);
		str172 = varToLower(str17); str212 = varToLower(str21);
		collectArgs = "";
		collectArgs += "(" + str1 + " (" + str3 + " ";
		collectArgs += "(any " + str52 + " " + str6 + ")))\n     ";
		collectArgs += "(" + str25 + " ";
		collectArgs += "(lvar " + str112 + " " + str12 + ") ";
		collectArgs += "(" + str15 + " ";
		collectArgs += "(any " + str172 + " " + str18 + "))\n     ";
		collectArgs += "append ";
		collectArgs += "(lvar " + str212 + " " + str22 + "))";
		collectArgs2 = "";
		collectArgs2 += "((" + str1 + " (" + str3 + " ";
		collectArgs2 += "(any " + str52 + " " + str6 + "))))\n     ";
		collectArgs2 += "((" + str25 + " ";
		collectArgs2 += "(lvar " + str112 + " " + str12 + ") ";
		collectArgs2 += "(" + str15 + " ";
		collectArgs2 += "(any " + str172 + " " + str18 + "))\n     ";
		collectArgs2 += " append ";
		collectArgs2 += "(lvar " + str212 + " " + str22 + ")))";
		collectArgs3 = "";
		collectArgs3 += "\"" + str1 + "\" \"new rel list\"";
	      }

	    | type'('type'('varname attrsindex')' ')' 
	      ZZPARAM '('varname varindex')' ZZSYMBOL ZZCROSSPRODUCT
	      '('varname varindex ZZCROSSPRODUCT
	          '('type'('varname attrsindex')' ZZFOLLOWS
		 varname varindex')' ')' ZZSYMBOL
	      {/* argsOP: e.g. projectextend */
		string str1 = $1;   string str3 = $3;
		string str5 = $5;   string str6 = $6;
		string str11 = $11; string str12 = $12;
		string str14 = $14; 
		string str17 = $17; string str18 = $18;
		string str21 = $21;
		string str23 = $23; string str24 = $24;
		string str27 = $27; string str28 = $28;
		string str31 = $31;
		string str52 = "";  string str112 = "";
		string str172 = ""; string str232 = "";
		string str272 = "";
		str52 = varToLower(str5);   str112 = varToLower(str11);
		str172 = varToLower(str17); str232 = varToLower(str23);
		str272 = varToLower(str27);
		collectArgs = "";
		collectArgs += "(" + str1 + " (" + str3 + " ";
		collectArgs += "(any " + str52 + " " + str6 + ")))\n     ";
		collectArgs += "(" + str14 + " ";
		collectArgs += "(lvar " + str112 + " " + str12 + "))\n     ";
		collectArgs += "(" + str31 + " ";
		collectArgs += "(lvar " + str172 + " " + str18 + ") ";
		collectArgs += "(" + str21 + " ";
		collectArgs += "(any " + str232 + " " + str24 + "))\n     ";
		collectArgs += "append ";
		collectArgs += "(lvar " + str272 + " " + str28 + "))";
		collectArgs2 = "";
		collectArgs2 += "((" + str1 + " (" + str3 + " ";
		collectArgs2 += "(any " + str52 + " " + str6 + "))))\n     ";
		collectArgs2 += "((" + str14 + " ";
		collectArgs2 += "(lvar " + str112 + " " + str12 + "))\n     ";
		collectArgs2 += " (" + str31 + " ";
		collectArgs2 += "(lvar " + str172 + " " + str18 + ") ";
		collectArgs2 += "(" + str21 + " ";
		collectArgs2 += "(any " + str232 + " " + str24 + "))\n     ";
		collectArgs2 += " append ";
		collectArgs2 += "(lvar " + str272 + " " + str28 + ")))";
		collectArgs3 = "";
		collectArgs3 += "\"" + str1 + "\" \"attrlist; new list\"";
	      }

	    | type'('type'('varname attrsindex')' ')' 
	      ZZPARAM '('varname varindex')' ZZSYMBOL ZZCROSSPRODUCT
	      '('varname ZZCROSSPRODUCT
	          '('type'('varname attrsindex')' ZZFOLLOWS
	      	     type'('varname')' ')' ')'
	      {/* argsOP: e.g. projectextendstream */
		string str1 = $1;   string str3 = $3;
		string str5 = $5;   string str6 = $6;
		string str11 = $11; string str12 = $12;
		string str14 = $14;		
		string str17 = $17; string str20 = $20;
		string str22 = $22; string str23 = $23;
		string str26 = $26; string str28 = $28;
		string str52 = "";  string str112 = "";
		string str172 = ""; string str222 = "";
		string str282 = "";
		str52 = varToLower(str5);   str112 = varToLower(str11);
		str172 = varToLower(str17); str222 = varToLower(str22);
		str282 = varToLower(str28);
		collectArgs = "";
		collectArgs += "(" + str1 + " (" + str3 + " ";
		collectArgs += "(any " + str52 + " " + str6 + ")))\n     ";
		collectArgs += "(" + str14 + " ";
		collectArgs += "(lvar " + str112 + " " + str12 + "))\n     ";
		collectArgs += "((var " + str172 + " 1) ";
		collectArgs += "(" + str20 + " ";
		collectArgs += "(any " + str222 + " " + str23 + "))\n     ";
		collectArgs += "append (" + str26 + " ";
		collectArgs += "(var " + str282 + " 1)))";
		collectArgs2 = "";
		collectArgs2 += "((" + str1 + " (" + str3 + " ";
		collectArgs2 += "(any " + str52 + " " + str6 + "))))\n     ";
		collectArgs2 += "((" + str14 + " ";
		collectArgs2 += "(lvar " + str112 + " " + str12 + "))\n     ";
		collectArgs2 += " ((var " + str172 + " 1) ";
		collectArgs2 += "(" + str20 + " ";
		collectArgs2 += "(any " + str222 + " " + str23 + "))\n     ";
		collectArgs2 += " append (" + str26 + " ";
		collectArgs2 += "(var " + str282 + " 1))))";
		collectArgs3 = "";
		collectArgs3 += "\"" + str1 + "\" ";
		collectArgs3 += "\"attrlist; new stream list\"";
	      }

	    | type'('type'('varname attrsindex')' ')' 
	      ZZPARAM '('varname varindex')' ZZSYMBOL ZZCROSSPRODUCT
	      '('varname varindex ZZCROSSPRODUCT
	      '('type'('type'('varname attrsindex')' ')' ZZFOLLOWS
		 varname varindex')' ')' ZZSYMBOL
	      {/* argsOP: e.g. groupby */
		string str1 = $1;   string str3 = $3;
		string str5 = $5;   string str6 = $6;
		string str11 = $11; string str12 = $12;
		string str14 = $14; 
		string str17 = $17; string str18 = $18;
		string str21 = $21; string str23 = $23;
		string str25 = $25; string str26 = $26;
		string str30 = $30; string str31 = $31;
		string str34 = $34;
		string str52 = "";  string str112 = "";
		string str172 = ""; string str252 = "";
		string str302 = "";
		str52 = varToLower(str5);   str112 = varToLower(str11);
		str172 = varToLower(str17); str252 = varToLower(str25);
		str302 = varToLower(str30);
		collectArgs = "";
		collectArgs += "(" + str1 + " (" + str3 + " ";
		collectArgs += "(any " + str52 + " " + str6 + ")))\n     ";
		collectArgs += "(" + str14 + " ";
		collectArgs += "(lvar " + str112 + " " + str12 + "))\n     ";
		collectArgs += "(" + str34 + " ";
		collectArgs += "(lvar " + str172 + " " + str18 + ") ";
		collectArgs += "(" + str21 + " (" + str23 + " ";
		collectArgs += "(any " + str252 + " " + str26 + "))\n     ";
		collectArgs += "append ";
		collectArgs += "(lvar " + str302 + " " + str31 + ")))";
		collectArgs2 = "";
		collectArgs2 += "((" + str1 + " (" + str3 + " ";
		collectArgs2 += "(any " + str52 + " " + str6 + "))))\n     ";
		collectArgs2 += "((" + str14 + " ";
		collectArgs2 += "(lvar " + str112 + " " + str12 + "))\n     ";
		collectArgs2 += " (" + str34 + " ";
		collectArgs2 += "(lvar " + str172 + " " + str18 + ") ";
		collectArgs2 += "(" + str21 + " (" + str23 + " ";
		collectArgs2 += "(any " + str252 + " " + str26 + "))\n     ";
		collectArgs2 += " append ";
		collectArgs2 += "(lvar " + str302 + " " + str31 + "))))";
		collectArgs3 = "";
		collectArgs3 += "\"" + str1 + "\" \"attrlist; new list\"";
	      }

	    | '('type')' '('type')' '('type')' ZZPARAM
	      '('type')' '('type')' '('type')' '('type')'
	      '('type')' '('type')' '('type')'
	      {/* argsOP: testQcV2 */
		vector<string> str(10);
		str[0] = $2;  str[1] = $5;  str[2] = $8;   
		str[3] = $12; str[4] = $15; str[5] = $18; str[6] = $21;
		str[7] = $24; str[8] = $27; str[9] = $30;
		for(int i=0; i<10; i++) {
		  if (str[i] == "Comma" ) {
		    str[i] = ",";
		  }
		  if (str[i] == "Semicolon" ) {
		    str[i] = ";";
		  }
		  if (str[i] == "Space" ) {
		    str[i] = " ";
		  }
		  if (str[i] == "Empty" ) {
		    str[i] = "";
		  }
		}
		collectArgs = "";
		collectArgs2 = "";
		collectArgs3 = "";
		collectArgs3 += "\"" + str[0]  + str[1]  + str[2]  + "\" ";
		collectArgs3 += "\"" + str[3] + str[4] + str[5] + str[6];
		collectArgs3 +=        str[7] + str[8] + str[9] +"\"";
		str.clear();
	      }
	    ;

resulttype  : type
	      {/* resOP: e.g. + */
		string str1 = $1;
		collectRes = "";
		collectRes += str1;
	      }

	    | type'('varname')'
	      {/* resOP: e.g. feed/consume/filter */
		string str1 = $1; string str3 = $3;
		string str32 = "";
		str32 = varToLower(str3);
		collectRes = "";
		collectRes += "(" + str1 + " ";
		collectRes += "(var " + str32 + " 1))";
	      }

	    | '('varname',' varname')'
	      {/* resOP: e.g. attr */
		collectRes = "";
		collectRes += "( append ";
		collectRes += "(var attrNo 1) (var attrType 1) )";
	      }

	    | type'('type'('varname attrsindex')' ')'
	      {/* resOP: e.g. hashjoin */
		string str1 = $1; string str3 = $3;
		string str5 = $5; string str6 = $6;
		string str52 = "";  str52 = varToLower(str5);
		collectRes = "";
		collectRes += "(" + str1 + " (" + str3 + " ";
		collectRes += "(var " + str52 + " " + str6 + ")))";	      
	      }

	    | '('varname varindex',' 
		type'('type'('varname attrsindex')' ')' ')'
	      {/* resOP: e.g. project */
		string str2 = $2; string str3 = $3;
		string str5 = $5; string str7 = $7;
		string str9 = $9; string str10 = $10;
		string str22 = "";  string str92 = "";
		str22 = varToLower(str2);  str92 = varToLower(str9);
		collectRes = "";
		collectRes += "( append ";
		collectRes += "(var " + str22 + " " + str3 + ")\n   ";
		collectRes += "(" + str5 + " (" + str7 + " ";
		collectRes += "(var " + str92 + " " + str10 + "))) )";
	      }
	    ;

condpreds   : condpred
	    | condpreds ',' condpred
	    ;

condpred    : ZZATTR'('varname',' varname','
		       varname',' varname')'
	      { /* Ident */
		string str1 = $1;
		string str3 = $3;  string str5 = $5;
		string str32 = ""; string str52 = "";
		str32 = varToLower(str3); str52 = varToLower(str5);
		collectPreds += "(" + str1 + " ";
		collectPreds += "(var " + str32 + " 1) ";
		collectPreds +=	"(var " + str52 + " 1)\n     ";
		collectPreds += "(var attrType 1) (var attrNo 1))\n   ";
	      }

	    | ZZATTR'('varname varindex',' varname attrsindex','
		       varname varindex',' varname varindex')'
	      { /* Ident_i */
		string str1 = $1;
		string str3 = $3;   string str4 = $4;
		string str6 = $6;   string str7 = $7;
				    string str10 = $10;
				    string str13 = $13;
		string str32 = "";  string str62 = "";
		str32 = varToLower(str3); str62 = varToLower(str6);
		collectPreds += "(" + str1 + " ";
		collectPreds += "(var " + str32 +  " " + str4  + ") ";
		collectPreds += "(var " + str62 +  " " + str7  + ")\n     ";
		collectPreds += "(var attrType " + str10 + ") ";
		collectPreds += "(var attrNo "	 + str13 + "))\n   ";
	      }

	    | ZZATTRS'('varname varindex',' varname attrsindex','
			varname varindex',' varname varindex')'
	      { 
		string str1 = $1;
		string str3 = $3;   string str4 = $4;
		string str6 = $6;   string str7 = $7;
		string str9 = $9;   string str10 = $10;
		string str12 = $12; string str13 = $13;
		string str32 = "";  string str62 = "";
		string str92 = "";  string str122 = "";
		str32 = varToLower(str3); str62 = varToLower(str6);
		str92 = varToLower(str9); str122 = varToLower(str12);
		collectPreds += "(" + str1 + " ";
		collectPreds += "(var " + str32  +  " " + str4  + ") ";
		collectPreds += "(var " + str62  +  " " + str7  + ")\n     ";
		collectPreds += "(var " + str92  +  " " + str10 + ") ";
		collectPreds += "(var " + str122 +  " " + str13  + "))\n   ";
	      }

	    | ZZCOMBINE'('varname',' varname','
			  varname attrsindex')'
	      { /* Ident */
		string str1 = $1;
		string str3 = $3;   string str5 = $5;
		string str7 = $7;   string str8 = $8;
		string str32 = "";  string str52 = "";
		string str72 = "";  
		str32 = varToLower(str3); str52 = varToLower(str5);
		str72 = varToLower(str7); 
		collectPreds += "(" + str1 + " ";
		collectPreds += "(var " + str32  +  " 1) ";
		collectPreds += "(var " + str52  +  " 1)\n     ";
		collectPreds += "(var " + str72  +  " " + str8 + "))\n   ";
	      }

	    | ZZCOMBINE'('varname varindex',' varname varindex','
			  varname attrsindex')'
	      { /* Ident_i */
		string str1 = $1;
		string str3 = $3;   string str4 = $4;
		string str6 = $6;   string str7 = $7;
		string str9 = $9;   string str10 = $10;
		string str32 = "";  string str62 = "";
		string str92 = "";  
		str32 = varToLower(str3); str62 = varToLower(str6);
		str92 = varToLower(str9); 
		collectPreds += "(" + str1 + " ";
		collectPreds += "(var " + str32  +  " " + str4  + ") ";
		collectPreds += "(var " + str62  +  " " + str7  + ")\n     ";
		collectPreds += "(var " + str92  +  " " + str10 + "))\n   ";
	      }
			  
	    | ZZCONCAT'('varname attrsindex',' varname attrsindex','
			 varname attrsindex')'
	      { 
		string str1 = $1;
		string str3 = $3;   string str4 = $4;
		string str6 = $6;   string str7 = $7;
		string str9 = $9;   string str10 = $10;
		string str32 = "";  string str62 = "";
		string str92 = "";  
		str32 = varToLower(str3); str62 = varToLower(str6);
		str92 = varToLower(str9); 
		collectPreds += "(" + str1 + " ";
		collectPreds += "(var " + str32  +  " " + str4  + ") ";
		collectPreds += "(var " + str62  +  " " + str7  + ")\n     ";
		collectPreds += "(var " + str92  +  " " + str10 + "))\n   ";
	      }
			 
	    | ZZCREATEATTR'('varname varindex',' 
			     type'('type'('varname attrsindex')' ')' ','
			     varname attrsindex')'
	    | ZZDISTATTRS'('varname attrsindex')'
	      { 
		string str1 = $1;
		string str3 = $3;   string str4 = $4;
		string str32 = "";
		str32 = varToLower(str3);
		collectPreds += "(" + str1 + " ";
		collectPreds += "(var " + str32  +  " " + str4  + "))\n   ";
	      }
	    
	    | ZZMINUS'('varname attrsindex',' varname varindex','
			varname attrsindex')'
	    ;

decls	    : ZZWHERE varname ZZIN '{'datatypes'}'
	      {
		string str2 = $2;   string str5 = $5;
		string str22 = "";
		str22 = varToLower(str2);
		collectDecls = "";
		collectDecls += "((var " + str22  +  " 1) ";
		collectDecls +=	"(" + collectEnum + ")) ";
	      }
	    ;

datatypes   : datatype
	      {
		string str1 = $1;
		collectEnum = "";
		collectEnum += str1;
	      }
	    | datatypes datatype
	      {
		string str2 = $2;
		collectEnum += " " + str2;
	      }
	    ;	    

opname	    : ZZIDENT
	    | ZZIDENT '_' ZZIDENT
	      {	/* for ops with '_' */ 
		string str1 = $1;   string str3 = $3;
		string strOp = str1 + "_" + str3;
		char* buffer = new char[strOp.length()];
		strcpy(buffer, strOp.c_str());
		$$ = buffer;
	      }
	    | ZZSYMBOL 
	    ;

varname	    : ZZIDENT
	    ;

type	    : ZZIDENT
	    ;

attrsindex  : '_'ZZINTI
	      { $$ = $2; }
	    ;

varindex    : '_'ZZINTI
	      { $$ = $2; }
	    | '_'ZZIDENT
	      { $$ = $2; }	    
	    ;

datatype    : type ','
	    | type
	    ;

semicolon   : ZZSEMICOLON { collectPreds = ""; }


%%

/*
Functions

*/

string varToLower(string varNameIn) {
  string varNameOut = "";
  for (int i=0; i<varNameIn.length(); i++) {
  varNameOut += tolower(varNameIn[i]);
  }
  return (varNameOut);
}

bool parseSigs(const char* infile, const string& outfile) {

  FILE* ifile;
  ifile = fopen(infile, "r");
  opsigin = ifile;

  ofile.open(outfile.c_str(), ios_base::out);
  ofile << "(\n";
  ofile2.open(outfile2.c_str(), ios_base::out);
  ofile2 << "(\n";
  ofile3.open(outfile3.c_str(), ios_base::out);
  ofile3 << "(\n";

  opsigparse();	//yyparse()

  ofile << "\n)";
  ofile.close();
  ofile2 << "\n)";
  ofile2.close();
  ofile3 << "\n)";
  ofile3.close();

  return (true);
}

