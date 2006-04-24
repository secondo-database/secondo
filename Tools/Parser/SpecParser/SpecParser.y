%{


/*
1.0 Includes

*/

#include <iostream>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <set>
using namespace std;

/*
1.1 Some external defined variables 

*/
extern int yylineno;
extern int yylex();
extern FILE* yyin;

/*
1.2  Files for writing the rules 

There are four files for writing entries. 

*/
ofstream* lexrules;
ofstream* yaccrules1;
ofstream* yaccrules2;
ofstream* tokens;

/* 
1.3 A set containing tokens for simple postfix operators 

Postfix operators need a special treatment because the different
length of the valuelist. For each different number of arguments before the
operator, a new rule must be inserted and a new token is created. The tokennames
build the content for this set. By looking in the set for already contained
tokens, we can decide whether a rule for this operator already exists.

*/
set<string> simplepostfixtokens;

/*
1.4 Recognition of used Operators

The next two sets collects the names of already used operators as well as
the operatorname together with its pattern. Therewith it is possible 
to ignore operators already transformed as well as operators with the
same name but different patterns.

*/
set<string> operatornames;
set<string> opnamesWithPattern;

/*
1.5 Some variables for statistical informations

*/
int processedSpecifications; // number of processed specifications


/*
1.4 The error function

This function will be called by the parser in case of an error.
The function writes an error message as well as the line number
which has caused the error to the standard error output.

*/
void yyerror( char* s )
{
  cerr << endl << s << endl << endl;
  cerr << "error at line " << yylineno << endl;
}


/*
1.5 The translation

This structur collects all information needed to construct the 
appropriate translation rules.

*/
struct trans{
    ostringstream*  token;
    ostringstream*  opname;
    ostringstream*  pattern;
    string          alias;
    vector<string>* arguments1; // arguments before the operator
    vector<string>* arguments2; // arguments after the operator
    vector<string>* implicitNames; 
    vector<string>* implicitTypes;
    vector<int>*    sublistlengths;
    char            bracket1;   // open bracket after the operator
    char            bracket2;   // close bracket after the operator 
    bool            isSimple;
    bool            hasfunctions;
    bool            isPostfix; // postfix notation needs a special treatment 
} currenttranslation;


/*
1.6 The init function

This function creates all needed variables and sets them to 
defined values.

*/
bool init(){
  // set the translation to be empty
	currenttranslation.token = new ostringstream();
  currenttranslation.opname = new ostringstream();
  currenttranslation.arguments1 = new vector<string>;
  currenttranslation.arguments2 = new vector<string>;
  currenttranslation.implicitNames = new vector<string>;
  currenttranslation.implicitTypes = new vector<string>;
  currenttranslation.sublistlengths = new vector<int>;
  currenttranslation.pattern = new ostringstream();
  currenttranslation.hasfunctions = false;
  currenttranslation.isSimple = true;
  currenttranslation.isPostfix=false;
  // initialize outputfiles
  lexrules = new ofstream("lexrules"); 
  if(lexrules==NULL)
     return false;
  yaccrules1 = new ofstream("yaccrules1"); 
  if(yaccrules1==NULL)
     return false;
  yaccrules2 = new ofstream("yaccrules2"); 
  if(yaccrules2==NULL)
     return false;
  tokens = new ofstream("tokens"); 
  if(tokens==NULL)
     return false;
  processedSpecifications=0;
 
  // we need the "=" operator undepending on any algebras so it is defined fix in 
  // the parser. But we have to ignore it and we have to detect specification 
  // different to a standard infix operator
  operatornames.insert("=");
  opnamesWithPattern.insert("= _infixop_");
 
  return true;
}

/*
1.6 PrintStatistics

This function prints out some statistical information
to the standard output.

*/
void printStatistics(){
  cout << " different operators " << operatornames.size() << endl;
  cout << " processed specification " << processedSpecifications << endl;
}


/*
1.6 Finalize

This function closes all open files.

*/
bool finalize(){
   (*yaccrules1) << "\n\t;\n\n";
   lexrules->close();
   yaccrules1->close();
   yaccrules2->close();
   tokens->close();
   return true;
}


/*
1.8 reset

The reset function sets all value in the current translation
to the initial values.

*/
void reset(){
  currenttranslation.opname->str("");
  currenttranslation.token->str("");
  currenttranslation.pattern->str("");
  currenttranslation.hasfunctions = false;
  currenttranslation.isSimple = true;
  currenttranslation.isPostfix=false;
  currenttranslation.arguments1->clear();
  currenttranslation.arguments2->clear();
  currenttranslation.implicitNames->clear();
  currenttranslation.implicitTypes->clear();
  currenttranslation.sublistlengths->clear();
}


/*
1.9 printtranslation

1.9.1  Print a simple non postfix translation

This function is very easy beacuse all non postfix operators
can't have any function symbols. All infix and prefix 
operator can be handled with standard definitions already
defined in the Parser. The only thing to do is to write a 
appropriate rule in the lex specification.

*/
void print_simple_nonpostfix(){
    const string opname = currenttranslation.opname->str();  
    const string token = currenttranslation.token->str();
    (*lexrules)  << "\"" << opname << "\" ";
    (*lexrules)  << " { yylval=NestedText::Atom(yytext,yyleng); return ";
    (*lexrules)  << token << ";}"  << endl; 
}

/*
1.9.2 print_simple_postfix

This function prints out a postfix function.  
For each different number of arguments (before the operator),
a new rule must be created. A generic rule can't be written 
because occuring conflicts in the resulting parser. 

*/
void print_simple_postfix(){
  const string opname = currenttranslation.opname->str();  
  string token = currenttranslation.token->str();

  int size2=currenttranslation.arguments2->size();
  if(size2>0){
     token = token + "_a";
  }
  
	if(simplepostfixtokens.find(token)==simplepostfixtokens.end()){
		/*
			The rule is not create in this case, we make entries in the
			files rules and yaccrules1.
		*/
		simplepostfixtokens.insert(token);
		// write token
		(*tokens) << token << " ";
		// write rule
		(*yaccrules1) << "\n\t| " ;
		int size = currenttranslation.arguments1->size();
		for(int i=0;i<size;i++){
			 string arg = (*currenttranslation.arguments1)[i];
			 (*yaccrules1) << arg << " ";
		}
    (*yaccrules1) << token ;
    if(size2!=0){
     	(*yaccrules1) << "'[' simplearguments ']'\n" ;
    }
		/* write the translation  */
		string space = "NestedText::AtomC(\" \")";
		(*yaccrules1) << " \t\t{ $$ =";
		(*yaccrules1) << "\t\t NestedText::Concat( ";
		(*yaccrules1) << " \t\t NestedText::AtomC(\"(\"),"; 
		(*yaccrules1) << "\t\t NestedText::Concat( $" << (size+1) << "," << endl;
    // write the arguments before the operator
		for(int i=0; i<size;i++){
				(*yaccrules1) << "\t\t NestedText::Concat(" << space << "," << endl;
				(*yaccrules1) << "\t\t NestedText::Concat( $" << (i+1) << ",";
		}
    if(size2>0){
       (*yaccrules1) << "\t\t NestedText::Concat(NestedText::AtomC(\" \")," << endl;
  	   	(*yaccrules1) << "\t\tNestedText::Concat( $" << (size+3) << "," << endl;
    }
		(*yaccrules1) << " \t\t NestedText::AtomC(\")\")"; 
		for(int i=0;i< size;i++){
				(*yaccrules1) << "))";
		}
    if(size2>0){
        (*yaccrules1) << "))";
    }
		(*yaccrules1) << "));";
		(*yaccrules1) << "\t\t  }\n";
	}
	// write the rule for lex
	(*lexrules) << "\"" << opname << "\" ";
	(*lexrules) << " { yylval=NestedText::Atom(yytext,yyleng); return ";
	(*lexrules) << token << ";}"  << endl; 
}

/* 
1.9.3 print_complex_postfix_without implicit parameters

This function is called when a postfix operator including 'fun' or
'funlist' is be found and the implicit parameter list is empty.

*/

void print_complex_postfix_without_implicit_parameters(){
  const string opname = currenttranslation.opname->str();  
  /* Found an operator with function arguments */
  // define a new token from the alias
  string newtoken = currenttranslation.alias;
	newtoken = "ZZ"+newtoken;
	// write it into the tokens file
	(*tokens) << newtoken << " ";
	// write a lexrule 
	(*lexrules) << "\"" << opname << "\" ";
	(*lexrules) << " { yylval=NestedText::Atom(yytext,yyleng); return ";
	(*lexrules) << newtoken << ";}"  << endl;
	
  // write the pattern for this function
  (*yaccrules1)  << "\n\t| ";
	int size1 = currenttranslation.arguments1->size();
  // write the leading (simple) arguments
	for(int i=0;i<size1;i++)
			(*yaccrules1)  << ((*currenttranslation.arguments1)[i]) << " ";
	(*yaccrules1) <<  newtoken << "'[' ";
	int size2 = currenttranslation.arguments2->size();

  int positions[currenttranslation.arguments2->size()]; // position of this argument in the pattern
  int separators = 0; // number of ';'
  int nextseparator = (*currenttranslation.sublistlengths)[0]; // = length of the first sublist
	for(int i=0;i<size2;i++){
      if(i==nextseparator){ // end of this sublist
        // write separator into the pattern
       (*yaccrules1) << " ';' ";
       separators++;
       nextseparator = i+(*currenttranslation.sublistlengths)[separators];
      }
      positions[i] = size1+3+ i*2;
			string arg =((*currenttranslation.arguments2)[i]);
			if(arg=="funlist")
				arg = "list";
			(*yaccrules1)  << arg;
      if(i!=nextseparator-1) 
        (*yaccrules1) << "',' ";
	}
	(*yaccrules1) << " ']'" << endl;
	// write the translation
	string space = "NestedText::AtomC(\" \")";
	(*yaccrules1) << " \t\t{ $$ =";
	(*yaccrules1) << "\t\t NestedText::Concat( ";
	(*yaccrules1) << " \t\t NestedText::AtomC(\"(\"),"; 
	(*yaccrules1) << "\t\t NestedText::Concat( $" << (size1+1+separators) << "," << endl; // operatortoken
	for(int i=0; i<size1;i++){ // arguments before operator
			(*yaccrules1) << "\t\t NestedText::Concat(" << space << "," << endl;
			(*yaccrules1) << "\t\t NestedText::Concat( $" << (i+1) << ",";
	}
	for(int i=0; i<size2; i++){
			(*yaccrules1) << "\t\t NestedText::Concat(" << space << "," << endl;
			(*yaccrules1) << "\t\t NestedText::Concat( $" << positions[i] << ",";
	}

	(*yaccrules1) << " \t\t NestedText::AtomC(\")\"))"; 
	for(int i=0;i< size1+size2;i++){
			(*yaccrules1) << "))";
	}
	(*yaccrules1) << ");";
	(*yaccrules1) << "\t\t  }\n";
}

/*
1.9.4 print_complex_postfix

This function handles postfix operators including functions 
and implicit parameters. We build a new token as well as one 
or two new yacc rules to handle such operators.

*/
void print_complex_postfix(){
  string token = "ZZ" + currenttranslation.alias;
  string opname = currenttranslation.opname->str();
  // write to the tokens file
  (*tokens) << token << " ";
  // write  rule for lex
	(*lexrules) << "\"" << opname << "\" ";
	(*lexrules) << " { yylval=NestedText::Atom(yytext,yyleng); return ";
	(*lexrules) << token << ";}"  << endl;


    // build the string for functions from the implicit parameters
   int sizei = currenttranslation.implicitNames->size();
   if(sizei>2){
      cerr << " Error: too many implicit parameters at line " << yylineno <<  endl;
      cerr << " allowed numbe is 2, found " << sizei << " ones " << endl;
      cerr << " The operator is " << opname << endl; 
      exit(-1);
   }

   string funname = currenttranslation.alias+"_fun";
   string funlistname = funname+"list";
   // write the rule for yacc
   (*yaccrules1) << "\n\t| ";
   // write leading arguments
   int size1 = currenttranslation.arguments1->size();
   for(int i=0;i<size1;i++){
     (*yaccrules1) << (*currenttranslation.arguments1)[i] << " ";
   }
   // write the operatortoken
   (*yaccrules1) << token << " ";
   // write the argumentlist, count the separators
   bool fun_used = false;
   bool funlist_used = false;

   int separators = 0; 
   int positions[currenttranslation.arguments2->size()];
   int nextseparator = (*currenttranslation.sublistlengths)[separators];
   (*yaccrules1) << "'[' ";  
   int size2 = currenttranslation.arguments2->size();

   for(int i=0;i<size2;i++){
      if(i==nextseparator) { // end of a sublist
         (*yaccrules1) << "';' ";
         separators++;
         nextseparator=i+(*currenttranslation.sublistlengths)[separators];
      }
      positions[i] = size1 + 3 + 2*i;
      string arg = (*currenttranslation.arguments2)[i];
      if(arg == "fun"){
         fun_used = true;
         arg = funname;
      } else if(arg == "funlist"){
         funlist_used=true;
         arg = funlistname;
      }
      (*yaccrules1) << arg ;
      if( (i+1) != nextseparator)
        (*yaccrules1) << "',' ";
   }
   (*yaccrules1) << " ']' \n ";
   // write the translation scheme
	string space = "NestedText::AtomC(\" \")";
	(*yaccrules1) << " \t\t{ $$ =";
	(*yaccrules1) << "\t\t NestedText::Concat( ";
	(*yaccrules1) << " \t\t NestedText::AtomC(\"(\"),"; 
	(*yaccrules1) << "\t\t NestedText::Concat( $" << (size1+1) << "," << endl; // operatortoken
	for(int i=0; i<size1;i++){ // arguments before operator
			(*yaccrules1) << "\t\t NestedText::Concat(" << space << "," << endl;
			(*yaccrules1) << "\t\t NestedText::Concat( $" << (i+1) << ",";
	}
	for(int i=0; i<size2; i++){
			(*yaccrules1) << "\t\t NestedText::Concat(" << space << "," << endl;
			(*yaccrules1) << "\t\t NestedText::Concat( $" << positions[i] << ",";
	}

	(*yaccrules1) << " \t\t NestedText::AtomC(\")\"))"; 
	for(int i=0;i< size1+size2;i++){
			(*yaccrules1) << "))";
	}
	(*yaccrules1) << ");";
	(*yaccrules1) << "\t\t  }\n";

  if(funlist_used) { // without a function list wo don't allow naming of functions 
		// print the rule for the function to yaccrules 2
		(*yaccrules2) << funname << ":   naming "<< funname << "_1" <<  endl;
		(*yaccrules2) << "\t\t  {$$ = NestedText::Concat(NestedText::AtomC(\"(\"), " << endl;
		(*yaccrules2) << "\t\t        NestedText::Concat($1, " << endl;
		(*yaccrules2) << "\t\t        NestedText::Concat(NestedText::AtomC(\" \")," << endl;
		(*yaccrules2) << "\t\t        NestedText::Concat($2, " << endl;
		(*yaccrules2) << "\t\t        NestedText::AtomC(\")\")  ))));} " << endl;
		(*yaccrules2) << "\t\t  | " << funname << "_1   {$$ = $1;} " << endl;
		(*yaccrules2) << "\t\t ; " << endl;
  }

  string extension = funlist_used? "_1":"";

  // write the real function scheme
  (*yaccrules2) << funname << extension << " :  { " << endl;
  for(int i=0;i<sizei;i++){
       (*yaccrules2) << "\t\t paramno++; " << endl;
       (*yaccrules2) << "\t\t strcpy(paramname,\"";
       (*yaccrules2) << (*currenttranslation.implicitNames)[i] << "\");" << endl;
       (*yaccrules2) << "\t\t sprintf(param" << (i+1) << ", \"%s%d\",paramname,paramno);" << endl;
       (*yaccrules2) << "\t\t paramstack.push(param" << (i+1) << ");" << endl;
  }   
  (*yaccrules2) << "\t\t }" << endl;  
  (*yaccrules2) << "\t valueexpr" << endl;
  (*yaccrules2) << "\t\t { " << endl;
  for(int i=sizei;i>0;i--){
     (*yaccrules2) << "\t\t strcpy(param" << (i) << ",paramstack.top().c_str());" << endl;
     (*yaccrules2) << "\t\t paramstack.pop();" << endl;
  }
  
  (*yaccrules2) << "\t\t $$ = NestedText::Concat( NestedText::AtomC(\"(fun \"),"  << endl;
  for(int i=0; i< sizei ; i++){
     (*yaccrules2)  << "\t\t\t  NestedText::Concat( NestedText::AtomC(\"(\"),"  << endl;
     (*yaccrules2)  << "\t\t\t  NestedText::Concat( NestedText::AtomC(param" << (i+1) << ")," << endl;
     (*yaccrules2)  << "\t\t\t NestedText::Concat( NestedText::AtomC(\""; 
     (*yaccrules2)  << " " << ((*currenttranslation.implicitTypes)[i]) << "\")," << endl;
     (*yaccrules2)  << "\t\t\t NestedText::Concat(NestedText::AtomC(\")\")," << endl;
  }
  (*yaccrules2) << "\t\t\t   NestedText::Concat( $2, NestedText::AtomC(\")\")))";
  for(int i=0;i<sizei;i++){
      (*yaccrules2) << " ))))";
  }
  (*yaccrules2) << ";" << endl;
  // delete values of all paramameters
  for(int i=0;i<sizei;i++){
     (*yaccrules2) << "\t\tsprintf(param"<<(i+1)<<",\"%s\",\"\");"<< endl;
  }

  (*yaccrules2) << "\t\t }" << endl;
  (*yaccrules2) << "\t\t | function { $$ = $1; } " << endl;
  (*yaccrules2) << "\t\t ;\n";
  
  if(funlist_used) {
     // rule for making bracket around the funlist
     (*yaccrules2) << funlistname << ": " << funlistname << "_1 " << endl;
     (*yaccrules2) << "\t\t { $$=NestedText::Concat(NestedText::AtomC(\"( \")," << endl;
     (*yaccrules2) << "\t\t           NestedText::Concat($1, " << endl;
     (*yaccrules2) << "\t\t           NestedText::AtomC(\" ) \")));" << endl;
     (*yaccrules2) << " \t\t } " << endl;
     (*yaccrules2) << "\t\t ; " << endl;
     // the funlist itself
     (*yaccrules2) << funlistname << "_1  : "<< funname << "{$$ = $1;}" << endl; // one elem list
     (*yaccrules2) << "\t\t | " << funlistname << "_1 ',' " << funname << endl; // composite list
     (*yaccrules2) << "\t\t  { $$ = NestedText::Concat($1," << endl;
     (*yaccrules2) << "\t\t        NestedText::Concat(NestedText::AtomC(\" \"), $3));" << endl;
     (*yaccrules2) << "\t\t }" << endl;
     (*yaccrules2) << " \t\t ; \n";
   }
 

}


/*

This function creates rules for les and yacc from the information stored in the
current translation. The rules are stored in the appropriate files.

*/

void printtranslation(){
  const string opname = currenttranslation.opname->str();  
  const string token = currenttranslation.token->str();
  
  if(currenttranslation.isSimple && ! currenttranslation.isPostfix){
      print_simple_nonpostfix();
  } else if(currenttranslation.isSimple){
     print_simple_postfix();    
 }
 else{
	int implicitNamesSize = currenttranslation.implicitNames->size();
	int implicitTypesSize = currenttranslation.implicitTypes->size();
	if(implicitNamesSize!=implicitTypesSize){
       cerr << " error in line " << yylineno << endl;
			 cerr << ("Different sizes for implicit names and types ");
			 cerr << " Names : ";
			 for(int i=0;i<implicitNamesSize;i++)
						cerr << ((*currenttranslation.implicitNames)[i]) << endl;
			 cout << " Types : ";
			 for(int i=0;i<implicitTypesSize;i++)
						cerr << ((*currenttranslation.implicitTypes)[i]) << endl;
			finalize();
			 exit(-1);
	}
	if(implicitNamesSize==0){ // without implicite parameters
     print_complex_postfix_without_implicit_parameters();     
  } else{ // operator with implicit parameters
     print_complex_postfix();
  }
 } 
}

%}

%union{
  char* text;
  int   len;
}

%token 	ZZOPERATOR ZZPATTERN ZZFUN ZZOP ZZINFIXOP  ZZLIST
       	ZZIMPLICIT ZZPARAMETER ZZPARAMETERS ZZTYPE ZZTYPES ZZFUNLIST ZZCOMMENT

%token<text> ZZIDENTIFIER	ZZSYMBOL ZZALIAS

%type<text> name
%type<len> argscomma simpleargscomma simpleargsblank

%%

specfile	:  specs
		      |
		      ;

specs		  : spec
		      | ZZCOMMENT
		      | specs spec
		      | specs ZZCOMMENT
	        ;

spec      :  ZZOPERATOR name ZZALIAS ZZIDENTIFIER ZZPATTERN pattern implicit
            {   processedSpecifications++;
                (*currenttranslation.opname) << $2;
                currenttranslation.alias = $4;
                // check for new operator
                string op = string($2);
                string opWithPattern= op+" "+currenttranslation.pattern->str();
                if(operatornames.find(op)==operatornames.end()){   
                    operatornames.insert(op);
                    opnamesWithPattern.insert(opWithPattern);
                    printtranslation(); // print out the current translation
                } else{
                  // operator already exists check for different pattern
                  if(opnamesWithPattern.find(opWithPattern)==opnamesWithPattern.end()){
                      cerr << " Conficting definition of operator " << op << endl;
                      cerr << " Line in spec file is " << yylineno << endl;
                      exit(-1);
                  }
                }
                reset(); // make clean 
             }
		      ;

name		  : ZZIDENTIFIER	
            { $$ = $1; }
		      | ZZSYMBOL
            { $$ = $1; }	
		      ;


pattern		: prefix
          | infix    
		      | postfix{
              currenttranslation.isPostfix=true;
            }
          ;

infix     : '_' ZZINFIXOP '_'
            {  currenttranslation.isSimple=true;           
               (*currenttranslation.token) << "ZZINFIXOP"; 
               (*currenttranslation.pattern) << "_infixop_"; 
            }
          ;

prefix		: ZZOP '('simpleargscomma')'		
            {  currenttranslation.isSimple = true;
               (*currenttranslation.token) << "ZZPREFIXOP";
               (*currenttranslation.pattern) << "op(_";
               for(int i=1;i<$3;i++){
                   (*currenttranslation.pattern) << ",_";
               }
               (*currenttranslation.pattern) << ")";
            }
          | ZZOP '(' ')'
            {  currenttranslation.isSimple = true;
               (*currenttranslation.token) << "ZZPREFIXOP";
               (*currenttranslation.pattern) << "op(";
               (*currenttranslation.pattern) << ")";
            }
            
		      ;

postfix		: simpleargsblank ZZOP
          {   currenttranslation.isSimple=true;           
              (*currenttranslation.token) << "ZZPOSTFIXOP" << (currenttranslation.arguments1->size());
              for(int i=0;i<$1;i++){
                 (*currenttranslation.pattern) << "_";
              } 
             (*currenttranslation.pattern) << "op";
          }		
		      | simpleargsblank ZZOP '[' 	arguments ']'
          {    (*currenttranslation.token) << "ZZPOSTFIXOP" << (currenttranslation.arguments1->size());
             for(int i=0;i<$1;i++){
                (*currenttranslation.pattern) << "_";
             } 
             (*currenttranslation.pattern) << "op[";
             int lists = 0;
             int nextlist = (*currenttranslation.sublistlengths)[0];
             int size = currenttranslation.arguments2->size();
             (*currenttranslation.pattern) << (*currenttranslation.arguments2)[0];
             for(int i=1;i< size;i++){
                if(i==nextlist){
                   lists++;
                   nextlist=(*currenttranslation.sublistlengths)[lists];
                   (*currenttranslation.pattern ) << ";";
                }else{
                   (*currenttranslation.pattern) << ",";
                }
                (*currenttranslation.pattern) << (*currenttranslation.arguments2)[i];
             }
             (*currenttranslation.pattern) << "]";
          }		
          ;


simpleargscomma	: '_'
                 {$$ = 1; }
		            | simpleargscomma ',' '_'
                 {$$ = $1 + 1;}
		            ;

simpleargsblank	: '_'
                {   currenttranslation.arguments1->push_back("valueexpr");
                    $$ = 1;
                }
		            | simpleargsblank '_'
                {   currenttranslation.arguments1->push_back("valueexpr");
                    $$ = $1 + 1;
                }
		            ;

arguments	: sublist
		      | arguments ';' sublist
	       	;

sublist		: argscomma
          { currenttranslation.sublistlengths->push_back($1);
          }
		      | ZZLIST
          { currenttranslation.arguments2->push_back("list");
            currenttranslation.isSimple = false;
            currenttranslation.sublistlengths->push_back(1);
          } 
		      | ZZFUNLIST{
            currenttranslation.arguments2->push_back("funlist");
            currenttranslation.isSimple = false;
            currenttranslation.sublistlengths->push_back(1);
          }
		      ;

argscomma	: arg
          { $$ = 1; }
		      | argscomma ',' arg
          { $$ = $1 + 1; }
		      ;

arg		: '_'
      { currenttranslation.arguments2->push_back("valueexpr");
      }
		  | ZZFUN
      { currenttranslation.arguments2->push_back("fun");
        currenttranslation.isSimple=false;
      }
		  ;

implicit	: ZZIMPLICIT rest
           { (*currenttranslation.pattern) << "implicit ";
             int size = currenttranslation.implicitNames->size();
             for(int i=0;i<size;i++){
               (*currenttranslation.pattern) << (*currenttranslation.implicitNames)[i] << " ";
             }
             (*currenttranslation.pattern) << "types ";
             size = currenttranslation.implicitTypes->size();
             for(int i=0;i<size;i++){
               (*currenttranslation.pattern) << (*currenttranslation.implicitTypes)[i] << " ";
             }
           }
		      |
		      ;

rest		: ZZPARAMETER parameterlist ZZTYPE typelist
        |  ZZPARAMETERS parameterlist ZZTYPES typelist
        ;
parameterlist : ZZIDENTIFIER
                { currenttranslation.implicitNames->push_back(string($1)); 
                }
              | parameterlist ',' ZZIDENTIFIER
                { currenttranslation.implicitNames->push_back(string($3));
                }
              ;
typelist : ZZIDENTIFIER
                { currenttranslation.implicitTypes->push_back(string($1));
                }
        | typelist ',' ZZIDENTIFIER
                { currenttranslation.implicitTypes->push_back(string($3));
                }
        ;
%%


int main(int argc, char** argv) {
    FILE* ifile;
    if(argc > 1){
       ifile = fopen(argv[1], "r");
       if (ifile == NULL) {
          fprintf(stderr,"ERROR: cannot open file %s\n",argv[1]);
          fprintf(stderr,argv[1]);
          fprintf(stderr,"\n");
          return -1;
        }
        yyin = ifile;
    }
    if(!init()){
      cerr << "Error in initialization " << endl;
      return -1;
    }
    if(yyparse()!=0){
       cerr << " Error in parsing specification" << endl;
       return -1;
    }
    printStatistics();
    if(!finalize()){
       cerr << "Error in finalization " <<  endl;
       return -1;
    }
    return 0;
}
