%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "OptSecUtils.h"

#define YYDEBUG 1
#define YYERROR_VERBOSE 1


#ifdef __cplusplus
extern "C"{
 int optlex();
 int opterror (const char *error);
 void opt_scan_string(const char* argument);
}
#endif

/*
Use the prolog nested list plnl.

*/
extern NestedList* plnl;

/*
Variables for return value and error message.

*/
char* err_message;
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

%union {
 char* strval;
 int numval;
}


%name_prefix="opt"  // use opt instead of yy to avoid naming conflicts with other 
                    // parsers of the system


/*
Define simple token and token holding a value.

*/

%token TOKEN_SELECT TOKEN_FROM TOKEN_STAR TOKEN_ERROR TOKEN_LET TOKEN_OPEN_BRACKET TOKEN_COMMA TOKEN_SQL TOKEN_SQOPEN_BRACKET
       TOKEN_SQCLOSE_BRACKET TOKEN_CLOSE_BRACKET TOKEN_DOT TOKEN_PUNCT TOKEN_SMALL_THAN TOKEN_GREATER_THAN TOKEN_INDEX_TYPE
       TOKEN_INSERT TOKEN_INTO  TOKEN_VALUES TOKEN_DELETE TOKEN_UPDATE TOKEN_SET TOKEN_TABLE TOKEN_CREATE TOKEN_COLUMNS
       TOKEN_ON TOKEN_DROP TOKEN_INDEX  TOKEN_NULL TOKEN_NON_EMPTY TOKEN_WHERE TOKEN_ORDER_BY TOKEN_FIRST
       TOKEN_LAST TOKEN_GROUP_BY  TOKEN_AS TOKEN_COUNT TOKEN_AGGREGATE TOKEN_ASC TOKEN_DESC TOKEN_COLON TOKEN_EQUAL TOKEN_ANY
       TOKEN_CUR_OPEN_BRACKET TOKEN_PLUS TOKEN_CUR_CLOSE_BRACKET  TOKEN_DASH TOKEN_FALSE TOKEN_TRUE TOKEN_SOME TOKEN_ROWID
       TOKEN_VALUE  TOKEN_SMALL_THAN_EQUAL TOKEN_GREATER_THAN_EQUAL  TOKEN_HASH TOKEN_DOUBLE_BRACKET
       TOKEN_MIN TOKEN_MAX TOKEN_SUM TOKEN_AVERAGE TOKEN_EXTRACT TOKEN_NUMBER TOKEN_CONST
       TOKEN_INTERSECTION TOKEN_UNION TOKEN_DISTANCE TOKEN_NOT TOKEN_EXISTS TOKEN_INT TOKEN_BOOL TOKEN_STRING TOKEN_REAL TOKEN_IN  
        TOKEN_LINE TOKEN_POINTS TOKEN_MPOINT TOKEN_UREGION TOKEN_RTREE TOKEN_BTREE TOKEN_HASH1 TOKEN_OR TOKEN_AND

%token<strval> TOKEN_ID TOKEN_VARIABLE TOKEN_DIGIT TOKEN_smallLetter TOKEN_LETTER TOKEN_SYMBOL TOKEN_TEXT  TOKEN_DISTINCT

%type<strval> ident  aggrop

%%

sql_clause :  TOKEN_LET newname mquery  
                    
           |  TOKEN_LET TOKEN_OPEN_BRACKET newname TOKEN_COMMA mquery TOKEN_COMMA TOKEN_TEXT TOKEN_CLOSE_BRACKET 
           |  TOKEN_SQL mquery 
           |  TOKEN_SQL TOKEN_OPEN_BRACKET mquery TOKEN_COMMA TOKEN_TEXT  TOKEN_CLOSE_BRACKET 
;

ident : TOKEN_ID
      | TOKEN_AND {$$ =  (char*)"and"; }
      | TOKEN_OR {$$ =  (char*)"or"; }
      | TOKEN_IN {$$ =  (char*)"in"; }
      | TOKEN_INTERSECTION {$$ = (char*)"intersection"; }
      | TOKEN_AS {$$ = (char*)"as"; }
      | TOKEN_UNION {$$ = (char*)"union"; }
      | TOKEN_DISTANCE {$$ = (char*)"distance"; }
      | TOKEN_NOT {$$ = (char*)"not"; }
      | TOKEN_EXISTS {$$ = (char*)"exists"; }
      | TOKEN_ANY {$$ = (char*)"any"; }
      | TOKEN_SOME {$$ = (char*)"some"; }
      | aggrop 
;
mquery : query
       | insert_query
       | delete_query
       | update_query 
       | create_query
       | drop_query
       | TOKEN_UNION TOKEN_SQOPEN_BRACKET query_list TOKEN_SQCLOSE_BRACKET
       | TOKEN_INTERSECTION TOKEN_SQOPEN_BRACKET query_list TOKEN_SQCLOSE_BRACKET
          
;


query : TOKEN_SELECT TOKEN_DISTINCT sel_clause TOKEN_FROM rel_clause where_clause orderby_clause first_clause
       {
           cerr << "DEBUG: query 1st rule" << endl; cerr.flush();
    }
        | TOKEN_SELECT  sel_clause TOKEN_FROM rel_clause where_clause orderby_clause first_clause
          
       |TOKEN_SELECT aggr_clause TOKEN_FROM rel_clause where_clause groupby_clause orderby_clause first_clause

     {
           cerr << "DEBUG: query 2nd rule" << endl; cerr.flush();
       }

;

insert_query : TOKEN_INSERT TOKEN_INTO relname TOKEN_VALUES value_list
              | TOKEN_INSERT TOKEN_INTO rel query
;

delete_query : TOKEN_DELETE TOKEN_FROM relname where_clause
;

update_query : TOKEN_UPDATE relname TOKEN_SET transform_clause where_clause
;

create_query : TOKEN_CREATE TOKEN_TABLE newname TOKEN_COLUMNS TOKEN_SQOPEN_BRACKET column_list TOKEN_SQCLOSE_BRACKET
             | TOKEN_CREATE TOKEN_INDEX TOKEN_ON relname TOKEN_COLUMNS index_clause
;

drop_query : TOKEN_DROP TOKEN_TABLE relname
           | TOKEN_DROP TOKEN_INDEX indexname
           | TOKEN_DROP TOKEN_INDEX TOKEN_ON relname index_clause
;

query_list : query
           |query TOKEN_COMMA query_list
;



sel_clause : sel_clause2
           | TOKEN_NON_EMPTY sel_clause2

;

rel_clause : rel
           |TOKEN_SQOPEN_BRACKET rel_list TOKEN_SQCLOSE_BRACKET
;

where_clause : TOKEN_WHERE TOKEN_SQOPEN_BRACKET pred_list TOKEN_SQCLOSE_BRACKET
               {  cerr << "DEBUG: query 4th rule" << endl }
             | TOKEN_WHERE pred
             {  cerr << "DEBUG: query 3rd rule" << endl       }
             | 
;

orderby_clause : TOKEN_ORDER_BY TOKEN_SQOPEN_BRACKET orderattr_list TOKEN_SQCLOSE_BRACKET
               | TOKEN_ORDER_BY orderattr
               | 
;

first_clause : TOKEN_FIRST TOKEN_INT
             | TOKEN_LAST TOKEN_INT 
             | 
;

aggr_clause : aggr 
            | TOKEN_SQOPEN_BRACKET aggr TOKEN_COMMA aggr_list TOKEN_SQCLOSE_BRACKET
;

groupby_clause : TOKEN_GROUP_BY TOKEN_SQOPEN_BRACKET groupattr_list TOKEN_SQCLOSE_BRACKET
               | TOKEN_GROUP_BY groupattr 
               |
;


attrname : ident

;

indextype :index
; 

index: TOKEN_BTREE
     | TOKEN_RTREE
     | TOKEN_HASH1
;

rel : relname
    | relname TOKEN_AS newname

;

value_list : value
           | value TOKEN_COMMA value_list
;

relname : TOKEN_ID
          { string dbname;

  
     string errorMsg ="No database open";
     if(!optutils::isDatabaseOpen(dbname,errorMsg)){
        opterror(errorMsg.c_str());
        return false;
      } 

     


      char* relname = $1; 
      ListExpr type = plnl->TheEmptyList();
      string realname;
      if(!optutils::isObject(relname, realname, type)){
         string err = "Object " + string(relname) + " not known in the database " + dbname;
         opterror(err.c_str());
         return false;
      } 

      if(!optutils::isRelDescription(type)){
         string err = "The object " + realname + " is not a relation.";
         opterror(err.c_str());
         return false;
       }
  

    }

; 																			 

transform_clause : transform
                 | TOKEN_SQOPEN_BRACKET transform_list TOKEN_SQCLOSE_BRACKET
;


column_list : column
            | column TOKEN_COMMA column_list
;

index_clause : attrname
             | attrname TOKEN_INDEX_TYPE indextype
;

indexname : TOKEN_ID
{

}
      
;

sel_clause2 : TOKEN_STAR
            | result
            | TOKEN_SQOPEN_BRACKET result_list TOKEN_SQCLOSE_BRACKET
            | TOKEN_COUNT TOKEN_OPEN_BRACKET TOKEN_DISTINCT TOKEN_STAR TOKEN_CLOSE_BRACKET
            | TOKEN_COUNT TOKEN_OPEN_BRACKET  TOKEN_STAR TOKEN_CLOSE_BRACKET
            | TOKEN_COUNT TOKEN_OPEN_BRACKET TOKEN_DISTINCT TOKEN_ID TOKEN_CLOSE_BRACKET
            | TOKEN_COUNT TOKEN_OPEN_BRACKET TOKEN_ID TOKEN_CLOSE_BRACKET
            | aggrop TOKEN_OPEN_BRACKET ext_attr_expr TOKEN_CLOSE_BRACKET
            | TOKEN_AGGREGATE TOKEN_OPEN_BRACKET ext_attr_expr TOKEN_COMMA aggrfun TOKEN_COMMA datatype TOKEN_COMMA const TOKEN_CLOSE_BRACKET
;

rel_list : rel
         | rel TOKEN_COMMA rel_list
;

pred_list : pred 
          | pred TOKEN_COMMA pred_list
;

pred : attr_bool_expr
     | subquerypred

;

attr_bool_expr: TOKEN_BOOL
              | attr_expr compop attr_expr
              | attr_expr
              | attr_bool_expr TOKEN_AND attr_bool_expr
              | attr_bool_expr TOKEN_OR attr_bool_expr
              | TOKEN_NOT attr_bool_expr
              ;

orderattr : ident
          | attr TOKEN_ASC
          | attr TOKEN_DESC
          | TOKEN_DISTANCE TOKEN_OPEN_BRACKET ident TOKEN_COMMA ident TOKEN_CLOSE_BRACKET
           
;

orderattr_list : orderattr
               | orderattr TOKEN_COMMA orderattr_list
;


aggr : groupattr
     | groupattr TOKEN_AS newname
     | aggr2
;

aggr_list : aggr
          | aggr TOKEN_COMMA aggr_list
;

value : TOKEN_INT
      | TOKEN_BOOL
      | TOKEN_STRING
;

groupattr : attr
;

groupattr_list : groupattr
               | groupattr TOKEN_COMMA groupattr_list 
               | 
;

var : ident

;




transform : TOKEN_ID TOKEN_EQUAL update_expression
 ;

transform_list : transform 
               | transform TOKEN_COMMA transform_list
;

column : TOKEN_ID TOKEN_COLON datatype

;

result : attr
       | attr_expr TOKEN_AS TOKEN_ID
;

result_list :  result
             | result TOKEN_COMMA result_list

ext_attr_expr : TOKEN_DISTINCT attr_expr
              | attr_expr
;



aggrfun : TOKEN_OPEN_BRACKET TOKEN_STAR TOKEN_CLOSE_BRACKET
        | TOKEN_OPEN_BRACKET TOKEN_PLUS TOKEN_CLOSE_BRACKET
        | ident { 
            if( (strcmp( $1 ,"union_new" ) !=0 ) ||
             ( strcmp( $1 ,"intersection_new" ) !=0 )) { 
               string err = "The object " + string($1) + " is not  valid .";
               opterror(err.c_str());
               return false;
            }     
        }
    
;




subquerypred : attr_expr TOKEN_IN TOKEN_OPEN_BRACKET table_subquery TOKEN_CLOSE_BRACKET
             | attr_expr TOKEN_NOT TOKEN_IN TOKEN_OPEN_BRACKET table_subquery TOKEN_CLOSE_BRACKET
             | TOKEN_EXISTS  TOKEN_OPEN_BRACKET query TOKEN_CLOSE_BRACKET
             | TOKEN_NOT TOKEN_EXISTS  TOKEN_OPEN_BRACKET query TOKEN_CLOSE_BRACKET
             | attr_expr compop quant TOKEN_OPEN_BRACKET table_subquery TOKEN_CLOSE_BRACKET
             
;

quant : TOKEN_ANY
      | TOKEN_SOME
      | TOKEN_DISTINCT { if (strcmp($1,"all")!=0) {
                            opterror("all, some,or any expected, bzut got distinct");
                            return false;
                         }
        }
;

aggr2 : TOKEN_COUNT  TOKEN_OPEN_BRACKET TOKEN_DISTINCT TOKEN_STAR TOKEN_CLOSE_BRACKET TOKEN_AS newname
      | TOKEN_COUNT  TOKEN_OPEN_BRACKET TOKEN_STAR TOKEN_CLOSE_BRACKET TOKEN_AS newname
      | aggrop  TOKEN_OPEN_BRACKET ext_attr_expr TOKEN_CLOSE_BRACKET renaming
      | TOKEN_AGGREGATE TOKEN_OPEN_BRACKET ext_attr_expr TOKEN_COMMA aggrfun TOKEN_COMMA attr_type TOKEN_COMMA const TOKEN_CLOSE_BRACKET TOKEN_AS newname
;

renaming: TOKEN_AS newname
        | 
        ;


newname : TOKEN_ID
     {}
;

attr : attrname
     | var TOKEN_COLON attrname 
     | TOKEN_ROWID
;



update_expression : const
                  |const_expr
;



generic_const : TOKEN_SQOPEN_BRACKET TOKEN_CONST TOKEN_COMMA attr_type TOKEN_COMMA TOKEN_VALUE nested_list TOKEN_SQCLOSE_BRACKET 

;

table_subquery : query
;

compop: TOKEN_SMALL_THAN
      | TOKEN_SMALL_THAN_EQUAL
      |  TOKEN_EQUAL
      |  TOKEN_GREATER_THAN_EQUAL
      |  TOKEN_GREATER_THAN
      |  TOKEN_HASH
      |  TOKEN_DOUBLE_BRACKET
;

aggrop : TOKEN_MIN {$$ =  (char*)"min"; }
       | TOKEN_MAX {$$ =  (char*)"max"; }
       | TOKEN_SUM {$$ =  (char*)"sum"; }
       | TOKEN_AVERAGE {$$ =  (char*)"avg"; }
       | TOKEN_EXTRACT {$$ =  (char*)"extract"; }
       | TOKEN_COUNT {$$ =  (char*)"count"; }
;
 
attr_type : nested_list
          {}
;

const_expr : nested_list
;

const : TOKEN_BOOL
      | TOKEN_INT
      | TOKEN_REAL
      | TOKEN_STRING
      | TOKEN_TEXT
      | generic_const
             
;

attr_expr : attr
          | const
          | TOKEN_OPEN_BRACKET attr_expr TOKEN_CLOSE_BRACKET
          | attr_expr op attr_expr
          | attr_expr op
          | attr_expr op TOKEN_SQOPEN_BRACKET attr_expr_list TOKEN_SQCLOSE_BRACKET
 //         | attr_expr attr_expr op TOKEN_SQOPEN_BRACKET attr_expr_list TOKEN_SQCLOSE_BRACKET
//          | attr_expr attr_expr attr_expr op TOKEN_SQOPEN_BRACKET attr_expr_list TOKEN_SQCLOSE_BRACKET
          | op TOKEN_OPEN_BRACKET attr_expr_list TOKEN_CLOSE_BRACKET
          | op TOKEN_OPEN_BRACKET TOKEN_CLOSE_BRACKET
          ;

op          : TOKEN_SYMBOL
             {  cerr << "DEBUG: query 5th rule" << endl }
            | TOKEN_ID
            | compop
            | TOKEN_PLUS
            | TOKEN_STAR
            
            ;

attr_expr_list : attr_expr
               | attr_expr TOKEN_COMMA attr_expr_list
              ;
          
nested_list : TOKEN_OPEN_BRACKET nlist TOKEN_CLOSE_BRACKET
            | TOKEN_OPEN_BRACKET TOKEN_CLOSE_BRACKET
            | TOKEN_BOOL
            | TOKEN_INT
            | TOKEN_STRING
            | TOKEN_TEXT
            | TOKEN_REAL
            | symbol
            ;
symbol      : TOKEN_SYMBOL
            | TOKEN_STAR
            ;


nlist : nested_list 
     | nested_list nlist
     
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







%%

/*
error handling

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

Main function. Checks a sql query against the requierements of the 
secondo's optimizer.

*/

bool checkOptimizerQuery(const char* argument, char*& errmsg){

   try{

    success = true;
    optlexDestroy();
 
    opt_scan_string(argument);

    optparse();
    if(success){
       errmsg=0;
       if(err_message){
          free(err_message);
          err_message = 0;
       }
       return true;
    }
    if(err_message == 0){
       cerr << "There is an error, but no message" << endl;
    }
    errmsg = err_message;
    err_message= 0;
    return false; 
  } catch(...){
      opterror("internal error during parsing");
      errmsg = strdup("internal error");
      return false;
  }

}

/*
Sets an compilerOption. If the option is not found, the ersult will be false. 

*/
bool setSqlParserOption(const string& optionName, const bool enable){
   if(optionName == "subqueries") {
       subqueries = enable;
       return true;
   }
   // insert further options here
   return false;
}






