package SQL2Secondo;

import java_cup10.runtime.*;
import java.io.IOException;
import SecExceptions.*;

import SQL2Secondo.ParsCommandSym;
import static SQL2Secondo.ParsCommandSym.*;

%%

%class ScanCommand

%unicode
%line
%char


%cupsym SQL2Secondo.ParsCommandSym

%implements java_cup10.runtime.Scanner
%function next_token
%type java_cup10.runtime.Symbol
%ignorecase

%init{
	// TODO: code that goes to constructor
%init}

%{
	StringBuffer string = new StringBuffer();
	
	private Symbol sym(int type) {
		return sym(type, yytext());
	}

	private Symbol sym(int type, Object value) {
		return new Symbol(type, yyline, yychar, value);
	}
	
	
	/* Detected SQL keywords which are not supported by Secondo raise this exception*/
	private void notCompError(String message) throws IOException {
		throw new NotSuppException(message);
	}

	private void error()
	throws IOException
	{
		throw new IOException("illegal text at line = "+yyline+", column = "+yychar+", text = '"+yytext()+"'");
	}
%}

LineTerminator		= \r|\n|\r\n
WhiteSpace		= {LineTerminator}|[ \t\f]
letter    		= [a-zA-Z]
digit       		= [0-9]
sign			= "+"|"-"
ident       		= {letter}({letter}|{digit}|_)*
PosNumber		= [1-9][0-9]*
Anzahl			= "("{PosNumber}")"           //to specify datatype eg. char(4)
AnzAnz			= "("{PosNumber}(","{PosNumber})?")"
StringType		= char(acter)?(" varying")?({Anzahl})?
IntType			= dec(imal)?({AnzAnz})?|numeric({AnzAnz})?|(small)?int|integer
RealType		= float({Anzahl})?|real|doubleprecision
BoolType		= "bit(1)"|boolean           //1 = TRUE and 0 = FALSE
DateType		= date
DateValue		= {DateType}" '"{digit}{digit}{digit}{digit}-{digit}{digit}-{digit}{digit}"'"
ExcatNumericLiteral	= ({digit})+("."({digit})*)?|"."({digit})+
ApproxNumericLiteral	= ({ExcatNumericLiteral})E({sign})?({digit})+
ParameterVariableName	= ":"{ident}
JoinType		= (natural{WhiteSpace})?((cross|inner|(left|right|full)({WhiteSpace}outer)?|union){WhiteSpace})?join


UKey			= global|local|temporary|collate|"not null"|"on commit"|nullif|case|when|coalesce|cast
							

ANY			= .

%state STRING

%%

<YYINITIAL> {WhiteSpace}	{ }

<YYINITIAL> {UKey}		{ notCompError(yytext()); }

<YYINITIAL> create	{ return sym(ParsCommandSym.CREATE); }
<YYINITIAL> table	{ return sym(ParsCommandSym.TABLE); }
<YYINITIAL> indextype	{ return sym(ParsCommandSym.INDEXTYPE); }
<YYINITIAL> index	{ return sym(ParsCommandSym.INDEX); }
<YYINITIAL> using	{ return sym(ParsCommandSym.USING); }
<YYINITIAL> btree	{ return sym(ParsCommandSym.BTREE); }
<YYINITIAL> rtree	{ return sym(ParsCommandSym.RTREE); }
<YYINITIAL> hasg	{ return sym(ParsCommandSym.HASH); }
<YYINITIAL> select	{ return sym(ParsCommandSym.SELECT); }
<YYINITIAL> from 	{ return sym(ParsCommandSym.FROM); }
<YYINITIAL> where 	{ return sym(ParsCommandSym.WHERE); }
<YYINITIAL> update	{ return sym(ParsCommandSym.UPDATE); }
<YYINITIAL> set		{ return sym(ParsCommandSym.SET); }
<YYINITIAL> "group by"	{ return sym(ParsCommandSym.GROUP_BY); }
<YYINITIAL> "order by"	{ return sym(ParsCommandSym.ORDER_BY); }
<YYINITIAL> all		{ return sym(ParsCommandSym.ALL); }
<YYINITIAL> some	{ return sym(ParsCommandSym.SOME); }
<YYINITIAL> any		{ return sym(ParsCommandSym.ANY); }
<YYINITIAL> asc		{ return sym(ParsCommandSym.ASC); }
<YYINITIAL> desc	{ return sym(ParsCommandSym.DESC); }
<YYINITIAL> distinct	{ return sym(ParsCommandSym.DISTINCT); }
<YYINITIAL> "insert into"	{ return sym(ParsCommandSym.INSERT_INTO); }
<YYINITIAL> "delete from"	{ return sym(ParsCommandSym.DELETE_FROM); }
<YYINITIAL> "drop table"	{ return sym(ParsCommandSym.DROP_TABLE); }
<YYINITIAL> "drop"	{ return sym(ParsCommandSym.DROP); }



<YYINITIAL> user	{ return sym(ParsCommandSym.USER); }
<YYINITIAL> current_user	{ return sym(ParsCommandSym.CURRENT_USER); }
<YYINITIAL> session_user	{ return sym(ParsCommandSym.SESSION_USER); }
<YYINITIAL> system_user	{ return sym(ParsCommandSym.SYSTEM_USER); }
<YYINITIAL> value	{ return sym(ParsCommandSym.VALUE); }
<YYINITIAL> union 	{ return sym(ParsCommandSym.UNION); }
<YYINITIAL> except 	{ return sym(ParsCommandSym.EXCEPT); }
<YYINITIAL> values 	{ return sym(ParsCommandSym.VALUES); }
<YYINITIAL> null 	{ return sym(ParsCommandSym.NULL); }
<YYINITIAL> default	{ return sym(ParsCommandSym.DEFAULT); }
<YYINITIAL> intersect	{ return sym(ParsCommandSym.INTERSECT); }
<YYINITIAL> overlaps	{ return sym(ParsCommandSym.OVERLAPS); }
<YYINITIAL> match	{ return sym(ParsCommandSym.MATCH); }
<YYINITIAL> unique	{ return sym(ParsCommandSym.UNIQUE); }
<YYINITIAL> fulltext	{ return sym(ParsCommandSym.FULLTEXT); }
<YYINITIAL> spatial	{ return sym(ParsCommandSym.SPATIAL); }
<YYINITIAL> partial	{ return sym(ParsCommandSym.PARTIAL); }
<YYINITIAL> full	{ return sym(ParsCommandSym.FULL); }
<YYINITIAL> exists	{ return sym(ParsCommandSym.EXISTS); }
<YYINITIAL> is		{ return sym(ParsCommandSym.IS); }
<YYINITIAL> between	{ return sym(ParsCommandSym.BETWEEN); }
<YYINITIAL> true	{ return sym(ParsCommandSym.TRUE); }
<YYINITIAL> false	{ return sym(ParsCommandSym.FALSE); }
<YYINITIAL> unknown	{ return sym(ParsCommandSym.UNKNOWN); }
<YYINITIAL> like	{ return sym(ParsCommandSym.LIKE); }
<YYINITIAL> having	{ return sym(ParsCommandSym.HAVING); }
<YYINITIAL> "where current of"	{ return sym(ParsCommandSym.WHERE_CURRENT_OF); }
<YYINITIAL> corresponding		{ return sym(ParsCommandSym.CORRESPONDING); }
<YYINITIAL> cascade	{ return sym(ParsCommandSym.CASCADE); }
<YYINITIAL> restrict	{ return sym(ParsCommandSym.RESTRICT); }
<YYINITIAL> constraint	{ return sym(ParsCommandSym.CONSTRAINT); }
<YYINITIAL> "foreign key"	{ return sym(ParsCommandSym.FOREIGN_KEY); }
<YYINITIAL> "primary key"	{ return sym(ParsCommandSym.PRIM_KEY); }
<YYINITIAL> references	{ return sym(ParsCommandSym.REFERENCES); }
<YYINITIAL> check	{ return sym(ParsCommandSym.CHECK); }
<YYINITIAL> alter	{ return sym(ParsCommandSym.ALTER); }
<YYINITIAL> add		{ return sym(ParsCommandSym.ADD); }
<YYINITIAL> column		{ return sym(ParsCommandSym.COLUMN); }
<YYINITIAL> {DateValue}	{ return sym(ParsCommandSym.DATE_VALUE); }


<YYINITIAL> {ParameterVariableName}	{ return sym(ParsCommandSym.PAR_VAR_NAME, yytext()); }	


<YYINITIAL> "("		{ return sym(ParsCommandSym.LPARENT); }
<YYINITIAL> ")"		{ return sym(ParsCommandSym.RPARENT); }
<YYINITIAL> ","		{ return sym(ParsCommandSym.COMMA); }
<YYINITIAL> "."		{ return sym(ParsCommandSym.DOT); }

<YYINITIAL> "+"		{ return sym(ParsCommandSym.PLUS); }
<YYINITIAL> "-"		{ return sym(ParsCommandSym.MINUS); }
<YYINITIAL> "*"		{ return sym(ParsCommandSym.ASTERISK); }
<YYINITIAL> "/"		{ return sym(ParsCommandSym.SOLIDUS); }
<YYINITIAL> "<>"	{ return sym(ParsCommandSym.NOT_EQUAL); }
<YYINITIAL> "<="	{ return sym(ParsCommandSym.LESS_EQUAL); }
<YYINITIAL> ">="	{ return sym(ParsCommandSym.GREATER_EQUAL); }
<YYINITIAL> "="		{ return sym(ParsCommandSym.EQUAL); }
<YYINITIAL> "<"		{ return sym(ParsCommandSym.LESS); }
<YYINITIAL> ">"		{ return sym(ParsCommandSym.GREATER); }

<YYINITIAL> {StringType}	{ return sym(ParsCommandSym.STRING); }
<YYINITIAL> {IntType}	{ return sym(ParsCommandSym.INT); }
<YYINITIAL> {RealType}	{ return sym(ParsCommandSym.REAL); }
<YYINITIAL> {BoolType}	{ return sym(ParsCommandSym.BOOL); }
<YYINITIAL> {DateType}	{ return sym(ParsCommandSym.DATE); }

<YYINITIAL> {JoinType}	{ return sym(ParsCommandSym.JOIN_TYPE, yytext().toLowerCase()); }




<YYINITIAL> as 		{ return sym(ParsCommandSym.AS); }
<YYINITIAL> min 	{ return sym(ParsCommandSym.MIN); }
<YYINITIAL> max 	{ return sym(ParsCommandSym.MAX); }
<YYINITIAL> avg 	{ return sym(ParsCommandSym.AVG); }
<YYINITIAL> sum 	{ return sym(ParsCommandSym.SUM); }
<YYINITIAL> count 	{ return sym(ParsCommandSym.COUNT); }
<YYINITIAL> by		{ return sym(ParsCommandSym.BY); }
<YYINITIAL> on		{ return sym(ParsCommandSym.ON); }
<YYINITIAL> in		{ return sym(ParsCommandSym.IN); }
<YYINITIAL> not		{ return sym(ParsCommandSym.NOT); }
<YYINITIAL> and		{ return sym(ParsCommandSym.AND); }
<YYINITIAL> or		{ return sym(ParsCommandSym.OR); }


<YYINITIAL> {ident}	{ return sym(ParsCommandSym.ID, yytext().toLowerCase()); }

<YYINITIAL>	{ExcatNumericLiteral}	{ return sym(ParsCommandSym.ENL, yytext()); }
<YYINITIAL>	{ApproxNumericLiteral}	{ return sym(ParsCommandSym.ANL, yytext()); }

<YYINITIAL>	'	{ string.setLength(0); yybegin(STRING); }


<STRING>	'	{ yybegin(YYINITIAL); return sym(ParsCommandSym.A_STRING, string.toString()); }

<STRING>	[^\n\r'\"\\]+	{ string.append(yytext()); }
<STRING>	\\t	{ string.append("\t"); }
<STRING>	\\n	{ string.append("\n"); }
<STRING>	\\r	{ string.append("\r"); }
<STRING>	\"	{ string.append("'"); }
<STRING>	''	{ string.append("'"); }
<STRING>	\\	{ string.append("\\"); }


//{ANY}			{ return sym(ANY); }

