/*

*/

%{
#include "DBLPParser.y.h"

int i;

//extern YYSTYPE yylval;

//char* yylvaltmp;

/*void getyylval(char text[], int no, char yytmp[MAXYYLVAL]) {
  for ( i=no;i<=(yyleng-(no+2));i++ )
    yytmp[i-no]=yytext[i];
  //yytmp[yyleng-(no+1)]='\0';
    yytmp[i-no]='\0';
  //printf(yytmp);
  //printf("\n");
  }*/

/*void getyylval(char text[], int no, int index) {
  for ( i=no;i<=(yyleng-(no+2));i++ )
    tokenvalues[index][i-no]=yytext[i];
  //yytmp[yyleng-(no+1)]='\0';
    tokenvalues[index][i-no]='\0';
  //printf(yytmp);
  //printf("\n");
}*/

%}



/* regular definitions */

startarticle           <article.*>
endarticle             <\/article>
startinproceedings     <inproceedings.*> 
endinproceedings       <\/inproceedings>
startproceedings       <proceedings.*> 
endproceedings         <\/proceedings>
startbook              <book[ ].*>
endbook                <\/book>
startincollection      <incollection.*>
endincollection        <\/incollection>
startphdthesis         <phdthesis.*>
endphdthesis           <\/phdthesis>
startmasterthesis      <mastersthesis.*>
endmasterthesis        <\/mastersthesis>
startwww               <www.*>
endwww                 <\/www>
author                 <author>.*<\/author>
editor                 <editor>.*<\/editor>
address                <address>.*<\/address>
title                  <title>.*<\/title>
booktitle              <booktitle>.*<\/booktitle>
pages                  <pages>.*<\/pages>
year                   <year>.*<\/year>
journal                <journal>.*<\/journal>
volume                 <volume>.*<\/volume>
number                 <number>.*<\/number>
month                  <month>.*<\/month>
url                    <url>.*<\/url>
cite                   <cite>.*<\/cite>
school                 <school>.*<\/school>
publisher              <publisher>.*<\/publisher>
note                   <note>.*<\/note>
crossref               <crossref>.*<\/crossref>
isbn                   <isbn>.*<\/isbn>
chapter                <chapter>.*<\/chapter>
series                 <series>.*<\/series>

%%

{startarticle}         { return(STARTARTICLE); }
{endarticle}           { return(ENDARTICLE); }
{startinproceedings}   { return(STARTINPROCEEDINGS); }
{endinproceedings}     { return(ENDINPROCEEDINGS); }
{startproceedings}     { return(STARTPROCEEDINGS); }
{endproceedings}       { return(ENDPROCEEDINGS); }
{startbook}            { return(STARTBOOK); }
{endbook}              { return(ENDBOOK); }
{startincollection}    { return(STARTINCOLLECTION); }
{endincollection}      { return(ENDINCOLLECTION); }
{startphdthesis}       { return(STARTPHDTHESIS); }
{endphdthesis}         { return(ENDPHDTHESIS); }
{startmasterthesis}    { return(STARTMASTERTHESIS); }
{endmasterthesis}      { return(ENDMASTERTHESIS); }
{startwww}             { return(STARTWWW); }
{endwww}               { return(ENDWWW); }

{author}               { //getyylval(yytext, 8, yylval.string);
                           //printf(yylval.string);
                           //printf("\n");
                         yytext[yyleng-9]='\0';
                         yylval.string = strdup(yytext+8);
                         // remove " by '
                         i=0;
                         while (yylval.string[i] != '\0') {
                           if (yylval.string[i] == '\"') yylval.string[i]='\'';
                           i++;
                         }
                         //yylvaltmp[10] = "\0";
                         //printf("AUTHOR");
                         return(AUTHOR);
                       }
{editor}               { //getyylval(yytext, 8, yylval.string);
                         //printf(yylval);
                         //printf("\n");
                         yytext[yyleng-9]='\0';
                         yylval.string = strdup(yytext+8);
                         return(EDITOR);
                       }
{title}                { /*i=7;
                         while ( (yytext[i]!='.')&&(i < (yyleng-8)) ) {
                           yylval.string[i-7]=yytext[i];
                           i++;
                         }
                         yylval.string[i-7]='\0';*/
                         //printf(yylval.string);
                         //printf("\n");
                         //printf("title");
                         //yylval.string = strdup(yytext);
                         yytext[yyleng-8]='\0';
                         yylval.string = strdup(yytext+7);
                         //printf(yylvaltmp);
                         //printf("\n");
                         return(TITLE);
                       }
{booktitle}            { //getyylval(yytext, 11, yylval.string);
  //printf(yylval);
  //printf("\n");
                         yytext[yyleng-12]='\0';
                         yylval.string = strdup(yytext+11);
                         return(BOOKTITLE);
                       }
{pages}                { //getyylval(yytext, 7, yylval.string);
  //printf(yylval);
  //printf("\n");
                         yytext[yyleng-8]='\0';
                         yylval.string = strdup(yytext+7);
                         return(PAGES);
                       }
{year}                 { //getyylval(yytext, 6, yylval.string);
  //printf(yylval);
  //printf("\n");
                         yytext[yyleng-7]='\0';
                         yylval.string = strdup(yytext+6);
                         return(YEAR);
                       }
{journal}              { //getyylval(yytext, 9, yylval.string);
  //printf(yylval);
  //printf("\n");
                         yytext[yyleng-10]='\0';
                         yylval.string = strdup(yytext+9);
                         return(JOURNAL);
                       }
{volume}               { //getyylval(yytext, 8, yylval.string);
  //printf(yylval);
  //printf("\n");
                         yytext[yyleng-9]='\0';
                         yylval.string = strdup(yytext+8);
                         return(VOLUME);
                       }
{number}                { //getyylval(yytext, 8, yylval.string);
  //printf(yylval);
  //printf("\n");
                         yytext[yyleng-9]='\0';
                         yylval.string = strdup(yytext+8);
                         return(NUMBER);
                       }
{month}                { //getyylval(yytext, 7, yylval.string);
  //printf(yylval);
  //printf("\n");
                         yytext[yyleng-8]='\0';
                         yylval.string = strdup(yytext+7);
                         return(MONTH);
                       }
{url}                  { //getyylval(yytext, 5, yylval.string);
  //printf(yylval);
  //printf("\n");
                         yytext[yyleng-6]='\0';
                         yylval.string = strdup(yytext+5);
                         return(URL);
                       }
{school}               { //getyylval(yytext, 8, yylval.string);
  //printf(yylval);
  //printf("\n");
                         yytext[yyleng-9]='\0';
                         yylval.string = strdup(yytext+8);
                         return(SCHOOL);
                       }
{publisher}            { //getyylval(yytext, 11, yylval.string);
  //printf(yylval);
  //printf("\n");
                         yytext[yyleng-12]='\0';
                         yylval.string = strdup(yytext+11);
                         return(PUBLISHER);
                       }
{isbn}                 { //getyylval(yytext, 6, yylval.string);
  //printf(yylval);
  //printf("\n");
                         yytext[yyleng-7]='\0';
                         yylval.string = strdup(yytext+6);
                         return(ISBN);
                       }

.|\n                   { ; }


%%

 /*main (argc, argv)
int argc;
char** argv;
{
FILE *ofile, *ifile;
char c;
int token;
char* espc = "    ";


    ifile = fopen(argv[1], "r");
    if (ifile == NULL)
    {
      printf("ERROR: cannot open file ");
      printf(argv[1]);
      printf("\n");
      return 0;
    }  
    yyin = ifile;
    token = yylex();
    while (token != 0)
    token = yylex();
    return 0; 
    }*/
