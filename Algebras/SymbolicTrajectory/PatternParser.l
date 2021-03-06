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

Started March 2012, Fabio Vald\'{e}s

[TOC]

\section{Overview}
This is the lex file that does the lexical analysis for the Symbolic Trajectory
Algebra.

      TODO: process double quotes separately in conditions.

*/
%{
#include "PatternParser.tab.h"
#include <iostream>
#include <string>
using namespace std;

void deleteCurrentPatternBuffer() {
    yy_delete_buffer(YY_CURRENT_BUFFER);
}

int openParentheses = 0;
int openBrackets = 0;
bool openQuotes = false;

%}
%option prefix="pattern"
%option nounput

%x condition
%x result
%x assignment

si [+-]?[0-9]+
ui [0-9]+
real {si}+(\.{ui}+)?
num {si}|{ui}|{real}
numinterval <{num}{num}(t\ f)?>|<{num}{num}>|{num}
star \*
plus \+
attrname [A-Z][a-zA-Z]*
setrelation disjoint|superset|equal|intersect
wildcard {star}|{plus}
date {si}(-{ui}(-{ui}(-{ui}(\:{ui})(\:{ui}(\.{ui})?)?)?)?)?
time {ui}\:{ui}(\:{ui}(\.{ui})?)?
interval ({date}(\~{date})?)|\~{date}|{date}\~|([a-z_][a-zA-Z0-9_]*)|\~{time}|{time}\~|{time}\~{time}
intervalset {interval}|(\{{interval}(\,[ ]*{interval})*\})
label ([a-zA-Z_\-][a-zA-Z0-9_\-]*)|(\"|\')[a-zA-Z0-9\. \ß\ä\ö\ü\Ä\Ö\Ü\/\(\)\[\]\{\}_\-\*\:\ô\í\ó\à\á\ã\ê\é\ç\ó\õ\ú\´\`]*(\"|\')
value {label}|{const_op}|{numinterval}|{si}|\_
labelset {label}|(({setrelation})?\{{label}(\,[ ]*{label})*\})
valueset {value}|(({setrelation})?\{{value}(\,[ ]*{value})*\})
contents {intervalset}([\ ]*{valueset})+
variable [A-Z][a-zA-Z0-9_\-]*
doubleslash \/{2}
conditiontype label|time|start|end|leftclosed|rightclosed|card|labels|extent|{attrname}
assigntype label|time|start|end|leftclosed|rightclosed|labels|ref|extent
vardottype {variable}\.{conditiontype}
vardottype2 {variable}\.{assigntype}
rightarrow =>
const_op ([^A-Z\(\)\[\]\,\"\t\n ][^\(\)\[\]\,\"\t\n ]+)|{num}|<|>|>=|<=|=|#|\+|\-|\*|TRUE|FALSE|\"[a-zA-Z0-9\. \ß\ä\ö\ü\Ä\Ö\Ü\/\(\)\[\]\{\}_\-]*\"
assign \:=
openregex \[
closeregex (\]\+)|(\]\*)|(\]\?)|\]

%option yylineno
%option noyywrap
%%

<INITIAL>[ \a\b\f\t\v\r]+ {}
<INITIAL>\( {return '(';}
<INITIAL>\) {return ')';}
<INITIAL>{openregex} {patternlval.text = strdup(patterntext); return ZZOPENREGEX;}
<INITIAL>{closeregex} {patternlval.text = strdup(patterntext); return ZZCLOSEREGEX;}
<INITIAL>{variable} {patternlval.text = strdup(patterntext); return ZZVARIABLE;}
<INITIAL>{wildcard} {patternlval.text = strdup(patterntext); return ZZWILDCARD;}
<INITIAL>{contents} {patternlval.text = strdup(patterntext); return ZZCONTENTS;}
<INITIAL>\| {return '|';}
<INITIAL>{rightarrow} {BEGIN(result); return ZZRIGHTARROW;}
<INITIAL>\n {return ZZEND;}
<INITIAL>{doubleslash} {BEGIN(condition); return ZZDOUBLESLASH;}
<condition>{vardottype} {patternlval.text = strdup(patterntext); return ZZVAR_DOT_TYPE;}
<condition>{attrname} {patternlval.text = strdup(patterntext); return ZZATTRNAME;}
<condition>{rightarrow} {BEGIN(result); return ZZRIGHTARROW;}
<condition>{const_op} {patternlval.text = strdup(patterntext); return ZZCONST_OP;}
<condition>\, {if (!openQuotes && openParentheses == 0 && openBrackets == 0) {
                 return ',';
               }
               else {
                 return ZZCOMMA;
               }
              }
<condition>\( {if (!openQuotes) {
                 ++openParentheses;
                 return '(';
               }
              }
<condition>\) {if (!openQuotes) {
                 --openParentheses;
                 return ')';
               }
              }
<condition>\[ {if (!openQuotes) {
                 ++openBrackets;
                 return '[';
               }
              }
<condition>\] {if (!openQuotes) {
                 --openBrackets;
                 return ']';
               }
              }
<condition>\" {openQuotes = !openQuotes;}
<condition>\n {BEGIN(INITIAL); return ZZEND;}
<result>{variable} {patternlval.text = strdup(patterntext); return ZZVARIABLE;}
<result>\( {return '(';}
<result>\) {return ')';}
<result>{doubleslash} {BEGIN(assignment); return ZZDOUBLESLASH;}
<result>\n {BEGIN(INITIAL); return ZZEND;}
<assignment>[ \a\b\f\t\v\r]+ {}
<assignment>{vardottype2} {patternlval.text = strdup(patterntext); return ZZVAR_DOT_TYPE;}
<assignment>{assign} {return ZZASSIGN;}
<assignment>{const_op} {patternlval.text = strdup(patterntext); return ZZCONST_OP;}
<assignment>\, {return ',';}
<assignment>\( {return '(';}
<assignment>\) {return ')';}
<assignment>\[ {return '[';}
<assignment>\] {return ']';}
<assignment>\n {BEGIN(INITIAL); return ZZEND;}
<INITIAL>. {patternlval.text = strdup(patterntext); BEGIN(INITIAL); return ZZERROR;}
%%
namespace stj {
void patternFlushBuffer() {
  BEGIN(INITIAL);
}
}
