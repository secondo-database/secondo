/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

 //paragraph [1] title: [{\Large \bf ]	[}]
 //[ae] [\"{a}]
 //[ue] [\"{u}]
 
  1 Overview
  This is the document describes the implementation of the OptParser.
  1.1 OptParser Bison Definition 
  The Bison Parser definition is described in this document.

*/

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <typeinfo>
#include "OptSecUtils.h"
#include "Types.h"


#define YYDEBUG 1
#define YYERROR_VERBOSE 1


int optlex(); 
int opterror (const char *error);
void opt_scan_string(const char* argument);
int optparse();


/*
Variables for return value and error message.

*/

// error message which is passed back to the optimizer
char* err_message;

// if set false, checkOptimizerQuery() Method reports failure of the query
bool success;

/*
some variables corresponding the the options.

*/
bool subqueries = false;
// insert further options here

%}

/*
We need the content of some tokens, e.g. for TOKEN_ID. Hence just the
token is not enough to return. We need some more sophisticated return value.
For further checks, we have to extend this definition.

*/

%verbose
%locations
%union 
{
 char* strval;
 OptParseStruct* attrset;
}


%name_prefix="opt"  // use opt instead of yy to avoid naming conflicts with other 
                    // parsers of the system


/*
Define simple token and token holding a value.

*/
%token TOKEN_SELECT "keyword select" TOKEN_FROM "keyword from" TOKEN_ERROR TOKEN_LET "keyword let" TOKEN_OPEN_BRACKET "symbol (" TOKEN_COMMA "symbol ," TOKEN_SQL "keyword sql" TOKEN_SQOPEN_BRACKET "symbol [" TOKEN_SQCLOSE_BRACKET "symbol ]" TOKEN_CLOSE_BRACKET "symbol )" TOKEN_DOT "symbol ." TOKEN_PUNCT "symbol '"  TOKEN_INDEX_TYPE "keyword indextype" TOKEN_INSERT "keyword insert" TOKEN_INTO "keyword into" TOKEN_VALUES "keyword values" TOKEN_DELETE "keyword delete" TOKEN_UPDATE "keyword update" TOKEN_SET "keyword set" TOKEN_TABLE "keyword table" TOKEN_CREATE "keyword create" TOKEN_COLUMNS "keyword columns" TOKEN_ON "keyword on" TOKEN_DROP "keyword drop" TOKEN_INDEX "keyword index" TOKEN_NULL "keyword null" TOKEN_NON_EMPTY "keyword nonempty" TOKEN_WHERE "keyword where" TOKEN_ORDER_BY "keyword orderby" TOKEN_FIRST "keyword first" TOKEN_LAST "keyword last" TOKEN_GROUP_BY "keyword groupby" TOKEN_AS "keyword as" TOKEN_ASC "keyword asc" TOKEN_DESC "keyword desc" TOKEN_COLON "symbol :"  TOKEN_ANY "keyword any" TOKEN_CUR_OPEN_BRACKET "symbol {"  TOKEN_CUR_CLOSE_BRACKET "symbol }"  TOKEN_FALSE "keyword false" TOKEN_TRUE "keyword true" TOKEN_SOME "keyword some" TOKEN_ROWID "keyword rowid" TOKEN_VALUE "keyword value" TOKEN_INTERSECTION "keyword intersection" TOKEN_UNION "keyword union" TOKEN_DISTANCE "keyword distance" TOKEN_NOT "keyword not" TOKEN_EXISTS "keyword exists"   TOKEN_IN "keyword in" TOKEN_LINE "keyword line" TOKEN_POINTS "keyword points" TOKEN_MPOINT "keyword mpoint" TOKEN_UREGION "keyword uregion" TOKEN_RTREE "keyword rtree" TOKEN_BTREE "keyword btree" TOKEN_HASH1 "keyword hash" TOKEN_OR "keyword or" TOKEN_AND "keyword and" TOKEN_BOOL "boolean value"  TOKEN_ALL "keyword all" TOKEN_DISTINCT "distinct" 

%token<strval> TOKEN_ID "identifier" TOKEN_VARIABLE "variable" TOKEN_DIGIT "digit" TOKEN_smallLetter "small letter" TOKEN_LETTER "letter" TOKEN_SYMBOL "symbol" TOKEN_TEXT "text" TOKEN_CONST "keyword const" TOKEN_STAR "symbol *" TOKEN_PLUS "symbol +" TOKEN_MINUS "symbol -" TOKEN_SMALL_THAN "symbol <" TOKEN_SMALL_THAN_EQUAL "symbol <=" TOKEN_EQUAL "symbol =" TOKEN_GREATER_THAN_EQUAL "symbol >=" TOKEN_GREATER_THAN "symbol >" TOKEN_HASH "symbol #" TOKEN_DOUBLE_BRACKET "symbol <>" TOKEN_INT "integer value" TOKEN_STRING "string value" TOKEN_REAL "real value" TOKEN_COUNT "operator count" TOKEN_AGGREGATE "operator aggregate" TOKEN_EXTRACT "operator extract" TOKEN_AVG "operator avg" TOKEN_SUM "operator sum" TOKEN_MIN "operator min" TOKEN_MAX "operator max" TOKEN_VAR "operator var"  TOKEN_OPERATOR "operator"

%type<attrset> aggregation aggregationoperator asclause attribute attributeboolexpr attributeexpr attributename compareoperator const createquery deletequery dropquery firstclause groupbyclause groupbyattribute groupclauseattribute groupclauseattributelist insertquery integer mquery operatorclause orderbyclause orderbyattribute orderbyattributelist predicate predicatelist quant query querylist relation relclause relationlist result resultlist selclause sqlclause subquerypred transform transformclause transformlist whereclause operator updatequery value valuelist updateexpression      
%type<strval> aggregationoperatornames identifier newname relationname   

%start start

%% 

start : sqlclause
         {
              string errormessages = $1->getErrorMessages();
              if (errormessages.size() > 0)
              {
                   opterror(errormessages.c_str());
              }
              $1->dumpAllInfo();
         }
;
aggregation : aggregationoperator TOKEN_OPEN_BRACKET groupbyattribute TOKEN_CLOSE_BRACKET asclause
              {
               $3->mergeStruct($5);
               $$ = $3;
              }
            | TOKEN_COUNT TOKEN_OPEN_BRACKET distinct groupbyattribute TOKEN_CLOSE_BRACKET asclause
              {
               $4->mergeStruct($6);
               $$ = $4;               
              }
            | TOKEN_COUNT TOKEN_OPEN_BRACKET distinct TOKEN_STAR TOKEN_CLOSE_BRACKET asclause
              {                
                 $$ = $6;
              }
;

aggregationoperator : TOKEN_MAX
                      {
                         $$ = new OptParseStruct();
                         $$->addToOpStack($1);
                      }
                    | TOKEN_MIN
                      {
                         $$ = new OptParseStruct();
                         $$->addToOpStack($1);
                      }
                    | TOKEN_EXTRACT
                      {
                         $$ = new OptParseStruct();
                         $$->addToOpStack($1);
                      }
                    | TOKEN_AVG
                      {
                         $$ = new OptParseStruct();
                         $$->addToOpStack($1);
                      }
                    | TOKEN_SUM
                      {
                         $$ = new OptParseStruct();
                         $$->addToOpStack($1);
                      }
                    | TOKEN_VAR
                      {
                         $$ = new OptParseStruct();
                         $$->addToOpStack($1);
                      }
;

aggregationoperatornames : TOKEN_MAX
                         | TOKEN_MIN
                         | TOKEN_EXTRACT
                         | TOKEN_AVG
                         | TOKEN_SUM
                         | TOKEN_VAR
;

asclause : TOKEN_AS newname
           {
               $$ = new OptParseStruct();
               $$->setAggregationAlias(true);
           }
         | /* epsilon */
           {
               $$ = new OptParseStruct();
           }
;

attribute : attributename
            {
               $$ = $1;
            }
          | TOKEN_ROWID
            {
                string errormessages = "Squared bracket around expression need to be closed.";
                opterror(errormessages.c_str());
                YYABORT;
            }      
;

attributeboolexpr : attribute compareoperator attributeexpr 
                    {
                         $2->mergeStruct($1);
                         $2->mergeStruct($3);
                         $$ = $2;     
                    }
                  | attributeboolexpr TOKEN_AND attributeboolexpr 
                    { 
                         $$ = new OptParseStruct();
                    }
                  | attributeboolexpr TOKEN_OR attributeboolexpr 
                    {
                          $$ = new OptParseStruct();
                    }
                  | TOKEN_NOT attributeboolexpr 
                    {
                          $$ = new OptParseStruct();
                    }
;

attributeexpr : attribute
                {
                    $$ = $1;
                }
              | const
                {
                    $$ = $1;
                }
              | TOKEN_OPEN_BRACKET attributeexpr TOKEN_CLOSE_BRACKET
                {
                   $$ = $2;
                }
              | operatorclause
                {
                   $$ = $1;
                } 
;

attributename : identifier
                {                
                 $$ = new OptParseStruct();         
                 $$->addAttribute($1);
                }
              | newname TOKEN_COLON identifier
                {
                 $$ = new OptParseStruct();
                 $$->addAttribute($3,$1);
                }
;

column : newname TOKEN_COLON datatype
;

columnlist : column
           | columnlist TOKEN_COMMA column
;           

compareoperator : TOKEN_SMALL_THAN
                  {     
                       $$ = new OptParseStruct();
                       $$->addToOpStack($1);
                  }
                | TOKEN_SMALL_THAN_EQUAL
                  { 
                       $$ = new OptParseStruct();
                       $$->addToOpStack($1);
                  }
                | TOKEN_EQUAL
                  { 
                       $$ = new OptParseStruct();
                       $$->addToOpStack($1);
                  }
                | TOKEN_GREATER_THAN_EQUAL           
                  { 
                       $$ = new OptParseStruct();
                       $$->addToOpStack($1);
                  }
                | TOKEN_GREATER_THAN
                  { 
                       $$ = new OptParseStruct();
                       $$->addToOpStack($1);
                  }
                | TOKEN_HASH
                  { 
                       $$ = new OptParseStruct();
                       $$->addToOpStack($1);
                  }
                | TOKEN_DOUBLE_BRACKET
                  { 
                       $$ = new OptParseStruct();
                       $$->addToOpStack($1);
                  }
;

const : TOKEN_BOOL
        { 
          $$ = new OptParseStruct();
          $$->addToOpStack("bool");
        }
      | integer
        { 
          $$ = $1;
        }
      | TOKEN_REAL
        { 
          $$ = new OptParseStruct(); 
          $$->addToOpStack("real");
        }
      | TOKEN_STRING
        { 
          $$ = new OptParseStruct(); 
          $$->addToOpStack("string");
        }
      | TOKEN_TEXT
        {
          $$ = new OptParseStruct();
          $$->addToOpStack("text");
        }
;

createquery : TOKEN_CREATE TOKEN_TABLE newname TOKEN_COLUMNS TOKEN_SQOPEN_BRACKET columnlist TOKEN_SQCLOSE_BRACKET
              {
               $$ = new OptParseStruct();
              }

            | TOKEN_CREATE TOKEN_INDEX TOKEN_ON relationname TOKEN_COLUMNS indexclause
              {
               $$ = new OptParseStruct();
              }
;

datatype : TOKEN_INT
         | TOKEN_REAL
         | TOKEN_BOOL
         | TOKEN_STRING
         | TOKEN_LINE
         | TOKEN_POINTS
         | TOKEN_MPOINT
         | TOKEN_UREGION
;

deletequery : TOKEN_DELETE TOKEN_FROM relationname whereclause
            {
               $$ = new OptParseStruct();
               $$->addRelation($3)
            }
;

distinct : TOKEN_DISTINCT
         | TOKEN_ALL
         | /* epsilon */
;

dropquery : TOKEN_DROP TOKEN_TABLE relationname
            {
               $$ = new OptParseStruct();
            }
          | TOKEN_DROP TOKEN_INDEX indexname
            {
               $$ = new OptParseStruct();
            }
          | TOKEN_DROP TOKEN_INDEX TOKEN_ON relationname indexclause
            {
               $$ = new OptParseStruct();
            }
;

firstclause : TOKEN_LAST TOKEN_INT
              {
                    $$ = new OptParseStruct();
              }
            | TOKEN_FIRST TOKEN_INT
              {
                    $$ = new OptParseStruct();
              }
            | /* epsilon */
              {
                    $$ = new OptParseStruct();
              }
;

groupbyattribute : newname
                   {                
                      $$ = new OptParseStruct();
                      $$->addGroupAttribute($1);
                      $$->addAttribute($1);

                   }
                  | newname TOKEN_COLON identifier
                    {
                         $$ = new OptParseStruct();
                         $$->addGroupAttribute($3,$1);
                         $$->addAttribute($3,$1);
                    }
;
                    
groupbyclause : TOKEN_GROUP_BY TOKEN_SQOPEN_BRACKET groupclauseattributelist TOKEN_SQCLOSE_BRACKET
                {     
                    $3->setGroupClause(true);            
                    $$ = $3 ;
                }
              | TOKEN_GROUP_BY groupclauseattribute 
                { 
                    $2->setGroupClause(true);
                    $$ = $2;
                }
              | TOKEN_GROUP_BY TOKEN_SQOPEN_BRACKET TOKEN_SQCLOSE_BRACKET 
                { 
                    $$ = new OptParseStruct();
                    $$->setGroupClause(true);                    
                }
              | TOKEN_GROUP_BY TOKEN_SQOPEN_BRACKET groupclauseattributelist
                {
                 string errormessages = "Squared bracket around expression needs to be closed.";
                 opterror(errormessages.c_str());
                 YYABORT;
                }
              | /* epsilon */
                {
                    $$ = new OptParseStruct();
                }          
;

groupclauseattribute : newname
                       {                
                         $$ = new OptParseStruct();
                         $$->addGroupClauseAttribute($1);
                       }
                     | newname TOKEN_COLON identifier
                       {
                         $$ = new OptParseStruct();
                         $$->addGroupClauseAttribute($3,$1);
                       }
;

groupclauseattributelist: groupclauseattribute
                          {
                              $$=$1;
                          }
                        | groupclauseattributelist TOKEN_COMMA groupclauseattribute
                          {
                              $$=$1;
                          }
;

identifier : TOKEN_ID
             { 
               $$ = $1;
             }
;

indexclause : attributename
            | attributename TOKEN_INDEX_TYPE indextype
;

indextype : TOKEN_BTREE 
          | TOKEN_RTREE
          | TOKEN_HASH1
;

indexname : TOKEN_ID
            {
               //TODO check for index existence
            }
;

insertquery : TOKEN_INSERT TOKEN_INTO relation TOKEN_VALUES valuelist
              {
               $$ = $3;
              }
            | TOKEN_INSERT TOKEN_INTO relation query
              {
               $$ = $3;
              }
;

integer : TOKEN_INT
          {              
              $$ = new OptParseStruct();
              $$->addToOpStack("int");
          }
        | TOKEN_MINUS TOKEN_INT
          {
              $$ = new OptParseStruct();
              $$->addToOpStack("int");
          }
        | TOKEN_PLUS TOKEN_INT
          {
              $$ = new OptParseStruct();
              $$->addToOpStack("int");
          }      
;

mquery : query
         {
          $$ = $1;
         }
       | insertquery
         {
          $$ = $1;
         }
       | updatequery 
         {
          $$ = $1;
         }
       | deletequery
         {
          $$ = $1;
         }
       | createquery
         {
          $$ = $1;
         }
       | dropquery
         {
          $$ = $1;
         }
       | TOKEN_UNION TOKEN_SQOPEN_BRACKET querylist TOKEN_SQCLOSE_BRACKET
         {
          $$ = $3;
         }
       | TOKEN_INTERSECTION TOKEN_SQOPEN_BRACKET querylist TOKEN_SQCLOSE_BRACKET
         {
          $$ = $3;
         }
;

nestedlist : TOKEN_OPEN_BRACKET nestedlistelementlist TOKEN_CLOSE_BRACKET
           | TOKEN_OPEN_BRACKET TOKEN_CLOSE_BRACKET
;

nestedlistelementlist : nestedlistelement                        
                      | nestedlistelementlist nestedlistelement
;

nestedlistelement: nestedlist             
                 | TOKEN_BOOL              
                 | TOKEN_INT
                 | TOKEN_STRING
                 | TOKEN_TEXT
                 | TOKEN_REAL
;

newname : TOKEN_ID
        | aggregationoperatornames
        | TOKEN_OPERATOR
;

operator : TOKEN_OPERATOR
           {
               $$ = new OptParseStruct();               
               $$->addToOpStack($1);
           }
         | TOKEN_SYMBOL // to suffice math symbols like = or +
           {
               $$ = new OptParseStruct();
               $$->addToOpStack($1);
           }           
         | TOKEN_STAR /* exeception for Operator symbol, which is already used as special character */
           {
               $$ = new OptParseStruct();
               $$->addToOpStack($1);
           }          
         | TOKEN_PLUS /* exeception for Operator symbol, which is already used as special character sign integer*/
           {
               $$ = new OptParseStruct();
               $$->addToOpStack($1);
           }
         | TOKEN_MINUS /* exeception for Operator symbol, which is already used as special character */
           {
               $$ = new OptParseStruct();
               $$->addToOpStack($1);
           }
;

operatorclause : operator
                {
                    $$ = $1;
                }
              // infix operator
               | attributeexpr operator attributeexpr
                 {   
                    // pass Attributes to operator
                    $2->mergeStruct($1);
                    $2->mergeStruct($3);
                    $$ = $2;
                 }
               // prefix operator
               | attributeexpr operator 
                 {                    
                     $2->mergeStruct($1);
                     $$ = $2;
                 }
              // postfixbrackets without parameter      
               | operator TOKEN_OPEN_BRACKET TOKEN_CLOSE_BRACKET
                 {
                     $$ = $1;                    
                 }
               // postfixbrackets with one or more parameters      
               | operator TOKEN_OPEN_BRACKET attributeexpr TOKEN_CLOSE_BRACKET
                 {
                     $1->mergeStruct($3);
                     $$ = $1;           
                 }  
               // unary operator
               | attributeexpr compareoperator attributeexpr
                 {
                  
                    $2->mergeStruct($1);
                    $2->mergeStruct($3);
                    $$ = $2;
                 }         
;

orderbyattribute: attribute
                  {
                    $$=$1;
                  }
;

orderbyattributelist : orderbyattribute
                       {
                          $$=$1;
                       }
                     | orderbyattributelist TOKEN_COMMA orderbyattribute
                       {
                          $$=$1;
                       }
;

orderbyclause : TOKEN_ORDER_BY TOKEN_SQOPEN_BRACKET orderbyattributelist sortorder TOKEN_SQCLOSE_BRACKET
                { 
                    $$ = $3 ;
                }
              | TOKEN_ORDER_BY TOKEN_SQOPEN_BRACKET orderbyattributelist
                 {
                  string errormessages = "Squared bracket around expression needs to be closed.";
                  opterror(errormessages.c_str());
                  YYABORT;
                 }  
              | TOKEN_ORDER_BY orderbyattribute sortorder
                {     
                    $$ = $2 ;
                }
              | /* epislon */
                { 
                    $$ = new OptParseStruct(); 
                }
;

predicate : attributeboolexpr 
            {
               $$ = $1;
            }
          | subquerypred 
            {
               $$ = $1; 
               $$->subqueriesAllowed();               
            }
;

predicatelist : predicate
                {
                    $$ = $1;
                }
              | predicatelist TOKEN_COMMA predicate
                {
                    $$ = $1
                }
;

query : TOKEN_SELECT distinct selclause TOKEN_FROM relclause whereclause groupbyclause orderbyclause firstclause
        { 
          // merge all OptParseStructs
          $7->dumpAllInfo();
          $7->mergeStruct($8);          
          $6->mergeStruct($7);
          $3->mergeStruct($6);                    
          $5->mergeStruct($3);
          $5->checkAttributes();
          $5->checkOperators();
          $5->checkAggregation();
          $5->dumpAllInfo();
          $$ = $5;
        }
;

querylist : query
            {
               $$ = $1;
            }
          | querylist TOKEN_COMMA query
            {  
               $1->mergeStruct($3);
               $$ = $1;
            }
;          

quant : TOKEN_ANY
        {
            $$ = new OptParseStruct();
        }
      | TOKEN_SOME
        {
          $$ = new OptParseStruct();
        }
      | TOKEN_DISTINCT 
        {
          $$ = new OptParseStruct();
        }
;    

relation : relationname
           { 
               $$ = new OptParseStruct();
               $$->addRelation($1);
           }
         | relationname TOKEN_AS newname 
           { 
               $$ = new OptParseStruct();
               $$->addRelation($1,$3);
               $$->addUsedAlias($3);
           }
;

relclause : relation
            {             
               $$ = $1;
            }
           | TOKEN_SQOPEN_BRACKET relationlist TOKEN_SQCLOSE_BRACKET
             {                      
               $$ = $2;
             }
           | TOKEN_SQOPEN_BRACKET relationlist
             {
              string errormessages = "Squared bracket around relationlist needs to be closed.";
              opterror(errormessages.c_str());
              YYABORT;
             }
;

relationlist : relation
               { 
                    $$ = $1;
               }
             | relationlist TOKEN_COMMA relation
               {
                    $1->mergeStruct($3);
                    $$ = $1;
               }
;

relationname : TOKEN_ID
          { 
               $$=$1;
          }          
;

result : attribute 
         {
           $$ = $1;
         }
             
       | attributeexpr TOKEN_AS newname
         {
           $$ = $1;
           $$->addUsedAlias($3);           
           $1->dumpAllInfo();
         }
       | aggregation
         { 
           $1->setAggregationoperatorUsed(true);
           $$ = $1;
         }
;

resultlist : result
             {
               $$ = $1;
             }
           | resultlist TOKEN_COMMA result
             {
               $1->addOperatorSeparator();
               $1->mergeStruct($3);
               $$ = $1;
               
             }
;

selclause : TOKEN_STAR 
            {
               // debug
               cout << "selclause: TOKEN_STAR " << endl;
               $$ = new OptParseStruct();
            }
            | result
              {
               // debug
               cout << "selclause: result" << endl;
               $$ = $1;
              }             
            | TOKEN_SQOPEN_BRACKET resultlist TOKEN_SQCLOSE_BRACKET
             {
               // debug
               cout << "selclause: TOKEN_SQOPEN_BRACKET resultlist TOKEN_SQCLOSE_BRACKET" << endl;
               $$ = $2;
             }
            | TOKEN_SQOPEN_BRACKET resultlist
             {
               string errormessages = "Squared brackets need to be closed.";
               opterror(errormessages.c_str());
               YYABORT;
             }  
            | resultlist TOKEN_COMMA result
             {
               string errormessages = "If there is more than one expression in the selection clause it needs to be enclosed in squared brackets.";
               opterror(errormessages.c_str());
               YYABORT;
             }
;

sortorder : TOKEN_ASC
          | TOKEN_DESC
          | /* Epsilon */
;

sqlclause :  TOKEN_LET newname mquery
             {
               $$ = $3;
             }
          |  TOKEN_LET TOKEN_OPEN_BRACKET newname TOKEN_COMMA mquery TOKEN_COMMA TOKEN_TEXT TOKEN_CLOSE_BRACKET
             {
               $5->addNewName($3);
               $$ = $5;
             }
          
          | TOKEN_SQL mquery
             {
               $$ = $2;
             }
          | mquery
             {
               $$ = $1;
             }             
          | TOKEN_SQL TOKEN_OPEN_BRACKET mquery TOKEN_COMMA TOKEN_TEXT  TOKEN_CLOSE_BRACKET
            {
               $$ = $3;
            }
;

subquerypred : attributeexpr TOKEN_IN TOKEN_OPEN_BRACKET query TOKEN_CLOSE_BRACKET
               {
                    $$=$4
               }
             | attributeexpr TOKEN_NOT TOKEN_IN TOKEN_OPEN_BRACKET query TOKEN_CLOSE_BRACKET
               {
                   $$=$1
               }             
             | TOKEN_EXISTS TOKEN_OPEN_BRACKET query TOKEN_CLOSE_BRACKET
               {
                    $$=$3;
               }
             | TOKEN_NOT TOKEN_EXISTS TOKEN_OPEN_BRACKET query TOKEN_CLOSE_BRACKET
               {
                    $$=$4;
               }
             | attributeexpr compareoperator quant TOKEN_OPEN_BRACKET query TOKEN_CLOSE_BRACKET 
               {
                    $$=$1;
               }
;

transformclause : transform
                  { $$ = $1 ; }
                | TOKEN_SQOPEN_BRACKET transformlist TOKEN_SQCLOSE_BRACKET
                  { $$ = $2 ; }
;

transformlist : transform
                {
                 $$ = $1;
                }
              | transformlist TOKEN_COMMA transform
                { 
                 $$ = $3;
                }
;

transform : TOKEN_ID TOKEN_EQUAL updateexpression
            { 
             $$ = $3 ;
            }
;

valuelist : value
          | valuelist TOKEN_COMMA value
;

value : TOKEN_INT
        {
          $$ = new OptParseStruct();
        }
      | TOKEN_BOOL
        {
          $$ = new OptParseStruct();
        }
      | TOKEN_STRING
        {
          $$ = new OptParseStruct();
        }
;

whereclause : TOKEN_WHERE predicate
             { 
               // debug
               cout << "whereclause : TOKEN_WHERE predicate" << endl;
               $$ = $2; 
             }
             | TOKEN_WHERE TOKEN_SQOPEN_BRACKET predicatelist TOKEN_SQCLOSE_BRACKET
               { 
                    cout << "whereclause : TOKEN_WHERE TOKEN_SQOPEN_BRACKET predicatelist TOKEN_SQCLOSE_BRACKET" << endl;               
                    $$ = $3;
               }
             | TOKEN_WHERE TOKEN_SQOPEN_BRACKET predicatelist
               {
                string errormessages = "Squared bracket around expression need to be closed.";
                opterror(errormessages.c_str());
                YYABORT;
               }  
             | /* epsilon */
              { 
               $$ = new OptParseStruct(); 
              }
;

updateexpression : const
                   {
                    $$ = new OptParseStruct();
                   }
                 | nestedlist
                   {
                    $$ = new OptParseStruct();
                   }
;

updatequery : TOKEN_UPDATE relation TOKEN_SET transformclause whereclause
              {
               $$ = $4;
              }
;
%%

/*
~opterror~

Bison error handling method which does set the global variable err_message in case of an error while parsing.

*/
int opterror (const char *error)
{
  success=false;
  err_message = (char*)malloc(strlen(error)+1);
  strcpy(err_message, error);
  return 0;
}


/*
declaration of a function work around memory leaks.


*/
extern "C"{void optlexDestroy();}


/*
~checkOptimizerQuery~

Main function of the OptParser. Checks a sql query against the requirements of the 
secondo's optimizer. Does return false if the query was unsuccessful and fills errmsg with error. The variable
success is being set false within the ~opterror~ method to indicate if an error has occured.

*/

bool checkOptimizerQuery(const char* argument, char*& errmsg) {
	optdebug = 1;
     try 
     {
          string dbname;
          string errorMsg ="No database open";
          // only continue if database is open
          if(!optutils::isDatabaseOpen(dbname,errorMsg))
          {	
               success = false;
               err_message = (char*)malloc(strlen(errorMsg.c_str())+1);
               strcpy(err_message, errorMsg.c_str());
          }
          else
          {
               success = true;
		     optlexDestroy();
		     opt_scan_string(argument);
		     optparse();
		}
		if (success) 
		{
			     errmsg = 0;
			     if (err_message) 
			     {
				     free( err_message);
				     err_message = 0;
			     }
			cout << "Query was accepted" << endl;
			return true;
		}
		if (err_message == 0) 
		{
			cerr << "There is an error, but no message" << endl;
		}
		errmsg = err_message;
		err_message = 0;
		cout << "Query was not accepted" << endl;
		return false;	} 
	catch (...) 
	{
		opterror("internal error during parsing");
		errmsg = strdup("internal error");
		return false;
	}

}
/*
Sets an compilerOption. If the option is not found, the result will be false. 

*/
bool setSqlParserOption(const string& optionName, const bool enable) {
	if (optionName == "subqueries") {
		subqueries = enable;
		return true;
	}
	// insert further options here
	return false;
}




