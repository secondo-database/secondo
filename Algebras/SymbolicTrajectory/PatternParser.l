%{
#include "PatternParser.tab.h"
#include <iostream>
#include <string>
using namespace std;
%}
%option prefix="pattern"

%x condition
%x result
%x assignment

si [+-]?[0-9]+
ui [0-9]+
real {si}+(\.{ui}+)?
star \*
plus \+
wildcard {star}|{plus}
date {si}(-{ui}(-{ui}(-{ui}(\:{ui})(\:{ui}(\.{ui})?)?)?)?)?
interval ({date}(\~{date})?)|\~{date}|{date}\~|([a-z_]+)
label [a-zA-Z_\-][a-zA-Z0-9_\-]*
labelset {label}|(\{{label}(\,[ ]*{label})*\})
contents {interval}[\ ]+{labelset}
contents_result _[ ]+{labelset}
variable [A-Z]{1,2}[0-9]*
doubleslash \/{2}
conditiontype label|time|start|end|card
vardottype {variable}\.{conditiontype}
rightarrow =>
const_op ([^A-Z\(\)\[\]\,\t\n ][^\(\)\[\]\,\t\n ]*)|TRUE|FALSE
vardotlabel {variable}\.label
assign \:=

%option yylineno
%option noyywrap
%%
[ \a\b\f\t\v\r]+ {}
\( {return '(';}
\) {return ')';}
{variable} {patternlval.text = strdup(patterntext); return ZZVARIABLE;}
{wildcard} {patternlval.text = strdup(patterntext); return ZZWILDCARD;}
{contents} {patternlval.text = strdup(patterntext); return ZZCONTENTS;}
{rightarrow} {BEGIN(result); return ZZRIGHTARROW;}
\n {return ZZEND;}
{doubleslash} {BEGIN(condition); return ZZDOUBLESLASH;}
<condition>{vardottype} {patternlval.text = strdup(patterntext); return ZZVAR_DOT_TYPE;}
<condition>{rightarrow} {BEGIN(result); return ZZRIGHTARROW;}
<condition>{const_op} {patternlval.text = strdup(patterntext); return ZZCONST_OP;}
<condition>\, {return ',';}
<condition>\( {return '(';}
<condition>\) {return ')';}
<condition>\[ {return '[';}
<condition>\] {return ']';}
<condition>\n {BEGIN(INITIAL); return ZZEND;}
<result>{variable} {patternlval.text = strdup(patterntext); return ZZVARIABLE;}
<result>\( {return '(';}
<result>\) {return ')';}
<result>{contents_result} {patternlval.text = strdup(patterntext); return ZZCONTENTS_RESULT;}
<result>{doubleslash} {BEGIN(assignment); return ZZDOUBLESLASH;}
<result>\n {BEGIN(INITIAL); return ZZEND;}
<assignment>{vardotlabel} {patternlval.text = strdup(patterntext); return ZZVAR_DOT_LABEL;}
<assignment>{assign} {return ZZASSIGN;}
<assignment>{labelset} {patternlval.text = strdup(patterntext); return ZZLABELSET;}
<assignment>\, {return ',';}
<assignment>\n {BEGIN(INITIAL); return ZZEND;}
. {patternlval.text = strdup(patterntext); return ZZERROR;}
%%
