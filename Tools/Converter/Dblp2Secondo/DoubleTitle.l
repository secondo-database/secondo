/*

*/

%{
//#include "DBLPParser.y.h"

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

author                 <author>.*<\/author>
title                  <title>.*<\/title>
newline                [\n]

%%


{title}{newline}*{title}+  { 
                             i=0;
                             while(yytext[i] != '\n') i++;                  
                             yytext[i+1]='\0';     
                             printf(yytext);
                           }



%%

main (argc, argv)
int argc;
char** argv;
{
  FILE *ofile, *ifile;
  char c;
  int token;

    /*ifile = fopen(argv[1], "r");
    if (ifile == NULL)
    {
      printf("ERROR: cannot open file ");
      printf(argv[1]);
      printf("\n");
      return 0;
    }  
    yyin = ifile;*/
    token = yylex();
    while (token != 0)
    token = yylex();
    return 0; 
}
