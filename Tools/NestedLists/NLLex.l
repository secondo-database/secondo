/*

//paragraph [1] Title: [{\Large \bf ] [}]

[1] Scanner of Stable Nested Lists

Copyright (C) 1996 Gral Support Team

Februar 1996 Holger Schenk

Textatoms are divided into parts of size 60 and appended to the previously 
created text atom. If the final pattern of a text atom \verb+</text--->+ 
can't be matched the text atom is destroyed. A separate list is only 
creating a text atom and appending the parts to the atom if lexing is O.K.
There is certainly a better solution, but there is no C-Interface to the 
list module at the moment ..

*/
                        

%{
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "NestedList.h"
#include "NLParser.h"
#include "NLParser.tab.hpp"

#define yylval NLParser::yylval

static char *scanErrMsg = "Error: Unexpected end of file while scanning text atom!\n";
static char *cmpstr = "</text--->";
static int zeichen;
static int pos = 0;
static int tmppos = 0;
static int textAtomPosition = 0;

char buffer[50];
char *s = buffer;

double f;

// the yylex() function that's actually used
#define YY_DECL int NLScanner::yylex()

#include "NLScanner.h"        // The interface of the derived class

/*
The following yylex function is never called, but overruled by
NLScanner::yylex()

*/
int yyFlexLexer::yylex()
{
    return 0;
}
  
%}
otherChar      [^\(\)\"A-Za-z0-9_ \t\n]
letter         [a-zA-Z]
digit          [0-9]
ident          {letter}({letter}|{digit}|_)*  
math           {otherChar}{otherChar}*
symbol         {math}|{ident}
num1	       [-+]?{digit}+\.?([eE][-+]?{digit}+)?
num2	       [-+]?{digit}*\.{digit}+([eE][-+]?{digit}+)?
number	       {num1}|{num2}

string         \"[^\"]*\"         

%%

[ \t\n]+       ;

-?[0-9]+       { 
                 /* ECHO; printf(" : is an integer\n"); */
                 yylval = nl->IntAtom( atoi( yytext ) );
                 return (INTEGER);
               }

{number}       {
                 /* ECHO; printf(" : is a real\n"); */
                 f = atof( yytext );
                 yylval = nl->RealAtom( f );
                 /* printf("\n Parser reals: %10.3f \n", f); */
                 return (REAL);
               }

"TRUE"         { 
                 /* ECHO; printf(" : is a boolean\n"); */
                 yylval = nl->BoolAtom( true );
                 return (BOOLEAN);
               }

"FALSE"        { 
                 /* ECHO; printf(" : is a boolean\n"); */
                 yylval = nl->BoolAtom( false );
                 return (BOOLEAN);
               }

"<text>"       { 
                 /* Initialization */
                 string textAtomBuffer = "";
                 textAtomPosition = 0;
                 pos = 0;
                 yylval = nl->TextAtom();

                 /* Read text until you reach </text---> */
                 do
                 { 
                   zeichen = yyinput();
                   /* Check if one or more of three special cases fit  */
                   /* Case 1: textAtomBuffer full?                     */
                   if ( textAtomPosition == 60 )
                   {
                     nl->AppendText( yylval, textAtomBuffer );
                     /* New empty textAtomBuffer: */
                     textAtomBuffer = "";
                     textAtomPosition = 0;
                   }
                   /* Case 2: End of file? */
                   if ( zeichen == 0 )
                   {
                     nl->Destroy( yylval );
                     cerr << scanErrMsg;
                     exit( 1 );
                     /* break; */
                   }
                   /* Case 3: zeichen = next chararacter in </text--->? */
                   if ( zeichen == cmpstr[pos] )
                   {
                     /* Increase pos, and do not insert zeichen in      */
                     /* textAtomBuffer                                  */
                     pos++;
                   }
                   /* No special case. In particular, not part of </text--->.*/
                   else
                   {
                     /* Step 1: Special treatment if the last characters 
                                in front of the current zeichen were the
                                beginning of </text--->. 
                        Step 2: Insert zeichen in textAtomBuffer
                     */
                     if ( pos != 0 )
                     {
                       /* (Step 1:)                                          */
                       /* Insert in textAtomBuffer the characters which were */
                       /* beginning of </text---> in the last pos runs of    */
                       /* the 'while (pos != 10)'-loop.                      */
                       /* If textAtomBuffer is full, append textAtomBuffer   */
                       /* to yylval before filling textAtomBuffer with the   */
                       /* remaining characters (of the beginning of          */
                       /* </text--->).                                       */
                       tmppos = pos;
                       do
                       {
                         if ( textAtomPosition < 60 )
                         {
                           textAtomBuffer += cmpstr[tmppos - pos];
                           textAtomPosition++;
                           pos--;
                         }
                         else
                         {
                           nl->AppendText(yylval, textAtomBuffer );
                           /* New empty textAtomBuffer: */
                           textAtomBuffer = "";
                           textAtomPosition = 0;
                         }
                       }
                       while (pos != 0);
                     }  
                     /* (Step 2:) standard case */
                     textAtomBuffer += zeichen;
                     textAtomPosition++;
                   } /* No special case, not part of </text--->. */
                 }
                 while (pos != 10);

                 /* At this point </text---> was reached (and not inserted   */
                 /* in textAtomBuffer).                                      */
                 if ( textAtomPosition < 60 )
                 {
                   nl->AppendText( yylval, textAtomBuffer );
                 }
                 return (TEXT);
               }

{string}       {
                 /* ECHO; printf(" : is a string\n"); */
                 if ( yyleng - 2 > 48 )
                 {
                   printf("Error: too long string\n");
                 }
                 else 
                 {
                   s = strncpy( s, &yytext[1], yyleng-2 );
                   s[yyleng-2] = '\0';
                   yylval = nl->StringAtom( s );
                   return (STRING);
                 }
               }

{symbol}       { 
                 /* ECHO; printf(" : is a symbol\n"); */
                 yylval = nl->SymbolAtom( yytext );
                 return (SYMBOL);
               }

"("            { 
                 /* ECHO; printf("\n"); */ 
                 return (OPEN);
               }

")"            {
                 /* ECHO; printf("\n"); */
                 return (CLOSE);
               }

.              { 
                 /* ECHO; printf("Fail in scanner\n"); */ 
               }
%%
int yywrap() { return 1; }

