%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <set>

#include "OptSecUtils.h"
#include "Types.h"

#define YYDEBUG 1
#define YYERROR_VERBOSE 1


/*#ifdef __cplusplus
extern "C"{
 int optlex(); 
 int opterror (const char *error);
 void opt_scan_string(const char* argument);
 int optparse();
}
#else 
*/
 int optlex(); 
 int opterror (const char *error);
 void opt_scan_string(const char* argument);
 int optparse();
//#endif




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

// define some global variables
std::set<string> usedNames;






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
 char* str;
 M* attrset;
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
        TOKEN_NUMBER TOKEN_CONST TOKEN_AGGR
       TOKEN_INTERSECTION TOKEN_UNION TOKEN_DISTANCE TOKEN_NOT TOKEN_EXISTS TOKEN_INT TOKEN_BOOL TOKEN_STRING TOKEN_REAL TOKEN_IN  
        TOKEN_LINE TOKEN_POINTS TOKEN_MPOINT TOKEN_UREGION TOKEN_RTREE TOKEN_BTREE TOKEN_HASH1 TOKEN_OR TOKEN_AND

%token<strval> TOKEN_ID TOKEN_VARIABLE TOKEN_DIGIT TOKEN_smallLetter TOKEN_LETTER TOKEN_SYMBOL TOKEN_TEXT  TOKEN_DISTINCT TOKEN_AGGROP
 
%type<attrset> attrname newname  query sel_clause rel_clause where_clause groupby_clause orderby_clause first_clause relname value_list
               value transform_clause column_list column index_clause query_list aggr_clause rel_list rel pred_list pred orderattr_list orderattr
               aggr_list aggr groupattr_list ident attr index transform_list transform indextype indexname result_list result attr_bool_expr
               subquerypred attr_expr compop ident2 update_expression datatype table_subquery quant ext_attr_expr attr_type aggrfun const_expr
               const nested_list intlist generic_const intlist2 op attr_expr_list nlist symbol insert_query delete_query update_query create_query
               drop_query  sel_clause2 sql_clause


%%

sql_clause :  TOKEN_LET newname mquery  
                { $$ = $2; }    
           |  TOKEN_LET TOKEN_OPEN_BRACKET newname TOKEN_COMMA mquery TOKEN_COMMA TOKEN_TEXT TOKEN_CLOSE_BRACKET 
               { $$ = $3; }
           |  TOKEN_SQL mquery 
                { $$ = new M(); }
           |  TOKEN_SQL TOKEN_OPEN_BRACKET mquery TOKEN_COMMA TOKEN_TEXT  TOKEN_CLOSE_BRACKET
                { $$ = new M(); } 
;

ident : TOKEN_ID 
        { $$ = new M();
          $$->name.insert($1) ;
        }
      | TOKEN_AND {$$->name.insert("and"); }
      | TOKEN_OR {$$->name.insert("or"); }
      | TOKEN_IN {$$->name.insert("in"); }
      | TOKEN_INTERSECTION {$$->name.insert("intersection"); }
      | TOKEN_AS {$$->name.insert("as"); }
      | TOKEN_UNION {$$->name.insert("union"); }
      | TOKEN_DISTANCE {$$->name.insert("distance"); }
      | TOKEN_NOT {$$->name.insert("not"); }
      | TOKEN_EXISTS {$$->name.insert("exists"); }
      | TOKEN_ANY {$$->name.insert("any"); }
      | TOKEN_SOME {$$->name.insert("some"); }
      | TOKEN_AGGROP {$$->name.insert("aggrop"); }
     
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


query : TOKEN_SELECT TOKEN_DISTINCT sel_clause TOKEN_FROM rel_clause where_clause groupby_clause orderby_clause first_clause
       {  
            $$ = $3;
            {
            set<string>  a = $5->name ;
            set<string> :: iterator it ;
            for( it = a.begin() ; it != a.end() ; it ++)
               { 
                 $$->name.insert( *it);
               }
           delete( $5 );
          }

        { 
           set<string>  a = $6->name ;
            set<string>:: iterator it ;
            for( it = a.begin() ; it != a.end() ; it ++)
               { 
                 $$->name.insert( *it);
               }
           delete( $6 );
          }
        {
          set<string>  a = $7->name ;
            set<string> :: iterator it ;
            for( it = a.begin() ; it != a.end() ; it ++)
               { 
                 $$->name.insert( *it);
               }
           delete( $7 );
          }

        { 
           set<string>  a = $8->name ;
            set<string> :: iterator it ;
            for( it = a.begin() ; it != a.end() ; it ++)
               { 
                 $$->name.insert( *it);
               }
           delete( $8 );
          }
        {
          set<string>  a = $9->name ;
            set<string> :: iterator it ;
            for( it = a.begin() ; it != a.end() ; it ++)
               { 
                 $$->name.insert( *it);
               }
           delete( $9 );
          }
           
       cerr << "DEBUG: query 1st rule" << endl; cerr.flush();
    }
       
        | TOKEN_SELECT  sel_clause TOKEN_FROM rel_clause where_clause groupby_clause orderby_clause first_clause
          { $$ = $2 ;
            {
                     set<string> a = $4->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $4 );       
                }
               {
                     set<string> a = $5->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $5 );       
                }
               {
                     set<string> a = $6->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $6 );       
                }
              {
                     set<string>  a = $7->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $7 );       
                }
              {
                     set<string>  a = $8->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $8 );       
                } }
       ;

insert_query : TOKEN_INSERT TOKEN_INTO relname TOKEN_VALUES value_list
               { $$ = $3; 
                  set<string>  a = $5->name ;
                  set<string> :: iterator it ;
                  for( it = a.begin() ; it != a.end() ; it ++)
                    { 
                       $$->name.insert( *it);
                    }
                 delete( $5 );
          }
              | TOKEN_INSERT TOKEN_INTO rel query
                { $$ = $3; }
;

delete_query : TOKEN_DELETE TOKEN_FROM relname where_clause
               { $$ = $3;
                 {
                     set<string>  a = $4->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $4 );       
                } }
;

update_query : TOKEN_UPDATE relname TOKEN_SET transform_clause where_clause
                { $$ = $2;
                 {
                     set<string> a = $4->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $4 );       
                } 
                   {
                     set<string>  a = $5->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $5 );       
                }}
;

create_query : TOKEN_CREATE TOKEN_TABLE newname TOKEN_COLUMNS TOKEN_SQOPEN_BRACKET column_list TOKEN_SQCLOSE_BRACKET
                 { $$ = $3;
                   { 
                     set<string> a = $6->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $6 );
           
           } }
             | TOKEN_CREATE TOKEN_INDEX TOKEN_ON relname TOKEN_COLUMNS index_clause
               { $$ = $4; 

               {
                     set<string>  a = $6->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $6 );       
                }
 }
;

drop_query : TOKEN_DROP TOKEN_TABLE relname
                { $$ = $3; }
           | TOKEN_DROP TOKEN_INDEX indexname
                { $$ = $3;}
           | TOKEN_DROP TOKEN_INDEX TOKEN_ON relname index_clause
              { $$ = $4;}
;

query_list : query
           |query TOKEN_COMMA query_list
;



sel_clause : sel_clause2
            { $$ = $1 ;}
           | TOKEN_NON_EMPTY sel_clause2
            { $$ = $2 ; }
           | aggr_clause
             { $$ = $1 ; }

;

rel_clause : rel
              { $$ = $1; }
           |TOKEN_SQOPEN_BRACKET rel_list TOKEN_SQCLOSE_BRACKET
             { $$ = $2; }
;

where_clause : TOKEN_WHERE TOKEN_SQOPEN_BRACKET pred_list TOKEN_SQCLOSE_BRACKET
               { $$ = $3;
                cerr << "DEBUG: query 4th rule" << endl }
             | TOKEN_WHERE pred
             { $$ = $2; 
               cerr << "DEBUG: query 3rd rule" << endl       }
             |  { $$ = new M(); }
;

orderby_clause : TOKEN_ORDER_BY TOKEN_SQOPEN_BRACKET orderattr_list TOKEN_SQCLOSE_BRACKET
                 { $$ = $3 ; }
               | TOKEN_ORDER_BY orderattr
                 { $$ = $2 ; }
               | { $$ = new M(); }
;

first_clause : TOKEN_FIRST TOKEN_INT
               { $$ = new M() ; }
             | TOKEN_LAST TOKEN_INT
               { $$ = new M() ; } 
             | { $$ = new M(); }
;

aggr_clause : aggr 
              { $$ = $1 ; }
            | TOKEN_SQOPEN_BRACKET aggr TOKEN_COMMA aggr_list TOKEN_SQCLOSE_BRACKET
              { $$ = $2 ;
                {
                     set<string> a = $4->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $4 );       
                } }
;

groupby_clause : TOKEN_GROUP_BY TOKEN_SQOPEN_BRACKET groupattr_list TOKEN_SQCLOSE_BRACKET
                 { $$ = $3 ; }
               | TOKEN_GROUP_BY attr 
                  { $$ = $2;
                }
               | { $$ = new M(); }
;


attrname : ident {
            $$ = $1;
          }


;

indextype :index
           { $$ = $1 ; }
; 

index: TOKEN_BTREE 
         { $$ = new M() ; }

     | TOKEN_RTREE
         { $$ = new M() ; }
     | TOKEN_HASH1
         { $$ = new M() ; }
;

rel : relname
     {$$ = $1;}
    | relname TOKEN_AS newname
      { $$ = $1;
        {
                     set<string> a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );       
                } }

;

value_list : value
            { $$ = $1 ; }
           | value TOKEN_COMMA value_list
              { $$ = $1 ;
                 {
                     set<string> a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );       
                }}
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
  

      $$ = new M();
      $$->name.insert($1);

    }

; 																			 

transform_clause : transform
                  { $$ = $1 ; }
                 | TOKEN_SQOPEN_BRACKET transform_list TOKEN_SQCLOSE_BRACKET
                   { $$ = $2 ;}
;


column_list : column
              { $$ = $1 ; }
            | column TOKEN_COMMA column_list
              { $$ = $1 ;
               {
                     set<string> a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );       
                } }
;

index_clause : attrname
              { $$ = $1 ;}
             | attrname TOKEN_INDEX_TYPE indextype
               { $$ = $1;
                {
                     set<string>  a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );       
                }
                }
;

indexname : TOKEN_ID
{
    $$ = new M();
    $$->name.insert( $1 );
}
      
;

sel_clause2 : TOKEN_STAR
              { $$ = new M() ; }
            | result
              { $$ = $1 ; }
            | TOKEN_SQOPEN_BRACKET result_list TOKEN_SQCLOSE_BRACKET
             { $$ = $2 ; }
           
;

rel_list : rel
           { $$ = $1; }
         | rel TOKEN_COMMA rel_list
           { $$ = $1; 
             {
                     set<string> a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );       
                }}
;

pred_list : pred {
             $$ = $1;
             cout << "pred_list: rule 1 matched." << endl;}
          | pred TOKEN_COMMA pred_list {
             $$ = $1;
           cout << "pred_list: rule 2 matched." << endl;
              {
                     set<string> a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );       
                }}
;

pred : attr_bool_expr {
          $$ = $1;
       cout << "pred: rule 1 matched." << endl;}
     | subquerypred {
       cout << "pred: rule 2 matched." << endl;
          $$ = $1 ; }

;

attr_bool_expr: TOKEN_BOOL { cout << "attr_bool_expr: rule 1 matched." << endl; }
              | attr_expr compop attr_expr { 
                $$ = $1 ;
                cout << "attr_bool_expr: rule 2 matched." << endl;
                 {
                     set<string>  a = $2->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $2 );       
                }
                   { 
                     set<string> a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );
 } }
              | attr_expr { 
                $$ = $1 ;
                cout << "attr_bool_expr: rule 3 matched." << endl; }
              | attr_bool_expr TOKEN_AND attr_bool_expr { 
                  $$ = $1;
                 cout << "attr_bool_expr: rule 4 matched." << endl;
                  { 
                     set<string>  a = $3->name ;
                     set<string>::iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );}
 }
              | attr_bool_expr TOKEN_OR attr_bool_expr { 
                   $$ = $1;
                       
                  cout << "attr_bool_expr: rule 5 matched." << endl;
                 {
                     set<string>  a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );       
                } }
              | TOKEN_NOT attr_bool_expr { 
                 $$ = $2;
                cout << "attr_bool_expr: rule 6 matched." << endl; }
              ;

orderattr : ident
            { $$ = $1 ; }
          | attr TOKEN_ASC {
                $$ = $1;
                }
            
          | attr TOKEN_DESC
             { $$ = $1;
             }
          | TOKEN_DISTANCE TOKEN_OPEN_BRACKET ident2  TOKEN_COMMA ident2 TOKEN_CLOSE_BRACKET
             { $$ = $3 ;
               {
                     set<string>  a = $5->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $5 );       
                }}
         
           
;

ident2: ident
        { $$ = $1 ; }
      | ident TOKEN_COLON ident 
        { $$ = $1;
          {
                     set<string> a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );       
                }}     
;
orderattr_list : orderattr
                  { $$ = $1;}
               | orderattr TOKEN_COMMA orderattr_list
                  { $$ = $1 ; 
                  {
                     set<string> a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );       
                }}
;


aggr_list : aggr
            { $$ = $1 ; }
          | aggr TOKEN_COMMA aggr_list
            { $$ = $1 ;
              {
                     set<string> a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );       
                }}
          | result
            { $$ = $1; }
          | result TOKEN_COMMA aggr_list
           { $$ = $1; 
              {
                     set<string> a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );       
                }}
;

value : TOKEN_INT
          { $$ = new M() ; }
      | TOKEN_BOOL
          { $$ = new M() ; }
      | TOKEN_STRING
          { $$ = new M() ; }
;

groupattr_list : attr {
                $$ = $1;
                }
               | attr TOKEN_COMMA groupattr_list {
                $$ = $3;
               {
                     set<string>  a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );       
                }
                }
               | { $$ = new M(); }
;






transform : TOKEN_ID TOKEN_EQUAL update_expression
             { $$ = $3 ;}
 ;

transform_list : transform 
                 { $$ = $1 ; }
               | transform TOKEN_COMMA transform_list
                 { $$ = $1 ; 
                  {
                     set<string> a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );       
                }
}
;

column : TOKEN_ID TOKEN_COLON datatype
           {  $$ = new M();
              $$->name.insert( $1 );
           }
;

result : attr {
                $$ = $1;
                }
       | attr_expr TOKEN_AS newname{
                $$ = $1;
                }
       | aggr
         { $$ = $1; }
;

result_list :  result
               { $$ = $1; }
             | result TOKEN_COMMA result_list
              { $$ = $1; }
;
ext_attr_expr : TOKEN_DISTINCT attr_expr{
                $$ = $2;
                }
              | TOKEN_DISTINCT TOKEN_STAR
                { $$ = new M() ; }
              | attr_expr{
                $$ = $1;
                }
;



aggrfun : TOKEN_OPEN_BRACKET TOKEN_STAR TOKEN_CLOSE_BRACKET
           { $$ = new M() ; }
        | TOKEN_OPEN_BRACKET TOKEN_PLUS TOKEN_CLOSE_BRACKET
           { $$ = new M() ; }
        | ident {
            string str = *($1->name.begin()); 
            if( (strcmp( str.c_str() ,"union_new" ) !=0 ) ||
             ( strcmp( str.c_str() ,"intersection_new" ) !=0 )) { 
               string err = "The object " +  str  + " is not  valid .";
               opterror(err.c_str());
               return false;
            } 
            $$ = $1;    
        }
    
;




subquerypred : attr_expr TOKEN_IN TOKEN_OPEN_BRACKET table_subquery TOKEN_CLOSE_BRACKET {
                $$ = $1;
                }
             | attr_expr TOKEN_NOT TOKEN_IN TOKEN_OPEN_BRACKET table_subquery TOKEN_CLOSE_BRACKET {
                $$ = $1;
                }
             | TOKEN_EXISTS  TOKEN_OPEN_BRACKET query TOKEN_CLOSE_BRACKET
               { $$ = new M() ; }
             | TOKEN_NOT TOKEN_EXISTS  TOKEN_OPEN_BRACKET query TOKEN_CLOSE_BRACKET
                { $$ = new M() ; }
             | attr_expr compop quant TOKEN_OPEN_BRACKET table_subquery TOKEN_CLOSE_BRACKET {
                $$ = $1;

                  {
                     set<string> a = $2->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $2 );
                  }
               {
                     set<string>  a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );       
                }

                }
             
;

quant : TOKEN_ANY
         { $$ = new M() ; }
      | TOKEN_SOME
           { $$ = new M() ; }
      | TOKEN_DISTINCT { if (strcmp($1,"all")!=0) {
                            opterror("all, some,or any expected, bzut got distinct");
                            return false;
                         }
        }
;

aggr : TOKEN_COUNT  TOKEN_OPEN_BRACKET TOKEN_DISTINCT TOKEN_STAR TOKEN_CLOSE_BRACKET 
           { $$ = new M (); }
      | TOKEN_COUNT  TOKEN_OPEN_BRACKET TOKEN_STAR TOKEN_CLOSE_BRACKET 
             { $$ = new M (); }
      | TOKEN_COUNT TOKEN_OPEN_BRACKET TOKEN_DISTINCT TOKEN_ID TOKEN_CLOSE_BRACKET 
           { $$ = new M (); }
      | TOKEN_COUNT TOKEN_OPEN_BRACKET TOKEN_ID TOKEN_CLOSE_BRACKET   
           { $$ = new M();
             $$->name.insert( $3 );
             
           }
      | TOKEN_AGGROP  TOKEN_OPEN_BRACKET ext_attr_expr TOKEN_CLOSE_BRACKET 
             { $$ = $3 ;
               }
      | TOKEN_AGGREGATE TOKEN_OPEN_BRACKET ext_attr_expr TOKEN_COMMA aggrfun TOKEN_COMMA attr_type TOKEN_COMMA const TOKEN_CLOSE_BRACKET 
           { $$ = $3 ; 
           {
                     set<string> a = $5->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $5 );       
                }
               {
                     set<string>  a = $7->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $7 );       
                }
           {
                     set<string>  a = $9->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $9 );}
                 }
              ;
                     
                
 newname :  TOKEN_ID
           {  $$ = new M();
              $$->name.insert($1);
       
           
          ListExpr type = plnl->TheEmptyList();
          string realname;
          if(optutils::isObject($1, realname, type)){
	  string err = "Object " + string($1) + " name already exists.Enter a new name.\n";
	  opterror(err.c_str());
	  return false;
	}     

        if(!optutils::isValidID($1)){  
	  string err = "Object " + string($1) + " name already exists.Enter a new name.\n";
	  opterror(err.c_str());
	  return false;
        }
	if(usedNames.find($1) !=usedNames.end()){ 
	  string err = "Object " + string($1) + " name already exists.Enter a new name.\n";
	  opterror(err.c_str());
	  return false;
	}
      
    }
;

         
attr : attrname
   {  cerr << "DEBUG: query 8th rule" << endl;
       $$ = $1;

 }
     | TOKEN_ID TOKEN_COLON attrname 
        { $$ = $3;
          $$->name.insert($1);
         }
     | TOKEN_ROWID
         { $$ = new M() ; }
;



update_expression : const
                   { $$ = $1 ; }
                  |const_expr
                    { $$ = $1 ; }
;



generic_const : TOKEN_SQOPEN_BRACKET TOKEN_CONST TOKEN_COMMA attr_type TOKEN_COMMA TOKEN_VALUE nested_list TOKEN_SQCLOSE_BRACKET 
                { $$ = $4 ;
                  {
                     set<string> a = $7->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $7 );       
                }}
;

table_subquery : query
;

compop: TOKEN_SMALL_THAN
          { $$ = new M() ; }
      | TOKEN_SMALL_THAN_EQUAL
        { $$ = new M() ; }
      |  TOKEN_EQUAL
          { $$ = new M() ; }
      |  TOKEN_GREATER_THAN_EQUAL
          { $$ = new M() ; }
      |  TOKEN_GREATER_THAN
          { $$ = new M() ; }
      |  TOKEN_HASH
          { $$ = new M() ; }
      |  TOKEN_DOUBLE_BRACKET
          { $$ = new M() ; }
;

attr_type : nested_list
          { $$ = $1 ;   }
;

const_expr : nested_list
             { $$ = $1 ; }
;

const : TOKEN_BOOL
         { $$ = new M() ; }
      | TOKEN_INT
        { $$ = new M() ; }
      | TOKEN_REAL
        { $$ = new M() ; }
      | TOKEN_STRING
         { $$ = new M() ; }
      | TOKEN_TEXT
         { $$ = new M() ; }
      | TOKEN_SQOPEN_BRACKET intlist TOKEN_SQCLOSE_BRACKET
          { $$ = $2 ; }
      | generic_const
        { $$ = $1 ; }
      ;

intlist: intlist2
         { $$ = $1 ; }
       | { $$ = new M(); }
       ;

intlist2 : TOKEN_INT
           { $$ = new M() ; }
         | TOKEN_INT TOKEN_COMMA intlist2
           { $$ = $3 ; }
         ;

attr_expr : attr { 
            $$ = $1 ; 
            cout << "attr_expr: rule 1 matched: attr." << endl; }
          | const{ cout << "attr_expr: rule 2 matched: const" << endl;
             $$ = $1 ; }
          | TOKEN_OPEN_BRACKET attr_expr TOKEN_CLOSE_BRACKET { 
            $$ = $2;
            cout << "attr_expr: rule 3 matched." << endl; }
          | attr_expr op attr_expr {
             $$ = $1;
             cout << "attr_expr: rule 4 matched." << endl;
               {
                     set<string> a = $2->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $2 );       
                }
                {
                     set<string>  a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );       
                }}
          | attr_expr op 
           { $$ = $1;
            cout << "attr_expr: rule 6 matched." << endl;
              {
                     set<string> a = $2->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $2 );       
                } }
          | op TOKEN_OPEN_BRACKET attr_expr_list TOKEN_CLOSE_BRACKET { 
             $$ = $1;
             cout << "attr_expr: rule 7 matched." << endl;

                  {
                     set<string>  a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );       
                } }
          | op TOKEN_OPEN_BRACKET TOKEN_CLOSE_BRACKET { cout << "attr_expr: rule 8 matched." << endl; }
             { $$ = $1 ; }
          ;




op          : TOKEN_SYMBOL
             {  cerr << "DEBUG: query 5th rule" << endl }
            | TOKEN_ID
             {  cerr << "DEBUG: query 9th rule" << endl }
            | compop
               { $$ = $1 ; }
            | TOKEN_PLUS
              { $$ = new M() ; }
            | TOKEN_STAR
               { $$ = new M() ; }
            | TOKEN_INT
            {  cerr << "DEBUG: query 7th rule" << endl }
            
            ;

attr_expr_list : attr_expr
                { $$ = $1 ; }
               | attr_expr TOKEN_COMMA attr_expr_list
                 { $$ = $1 ;
                  {
                     set<string> a = $3->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $3 );       
                }}
              ;
          
nested_list : TOKEN_OPEN_BRACKET nlist TOKEN_CLOSE_BRACKET
               { $$ = $2 ; }
            | TOKEN_OPEN_BRACKET TOKEN_CLOSE_BRACKET
              { $$ = new M() ; }
            | TOKEN_BOOL
              { $$ = new M() ; }
            | TOKEN_INT
               { $$ = new M() ; }
            | TOKEN_STRING
               { $$ = new M() ; }
            | TOKEN_TEXT
              { $$ = new M() ; }
            | TOKEN_REAL
                { $$ = new M() ; }
            | symbol
               { $$ = $1 ; }
            ;
symbol      : TOKEN_SYMBOL
               { $$ = new M() ; }
            | TOKEN_STAR
              { $$ = new M() ; }
            ;


nlist : nested_list 
        { $$ = $1 ; }
     | nested_list nlist
         { $$ = $1 ; 
     {
                     set<string> a = $2->name ;
                     set<string> :: iterator it ;
                     for( it = a.begin() ; it != a.end() ; it ++)
                        { 
                         $$->name.insert( *it);
                         }
                    delete( $2 );       
                }
}
          
     
;

datatype : TOKEN_INT
             { $$ = new M() ; }
         | TOKEN_REAL
            { $$ = new M() ; }
         | TOKEN_BOOL
             { $$ = new M() ; }
         | TOKEN_STRING
            { $$ = new M() ; }
         | TOKEN_LINE
             { $$ = new M() ; }
         | TOKEN_POINTS
             { $$ = new M() ; }
         | TOKEN_MPOINT
            { $$ = new M() ; }
         | TOKEN_UREGION
           { $$ = new M() ; }
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

   usedNames.clear();

   optdebug=0;

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






