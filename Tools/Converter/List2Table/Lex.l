
/*

*/

%{
   int i;
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

[ \t\n(]+       ;

-?[0-9]+       {for (i=0; i<yyleng; i++) putchar(yytext[i]); putchar('\t');}

"TRUE"         {for (i=0; i<yyleng; i++) putchar(yytext[i]); putchar('\t');}

"FALSE"        {for (i=0; i<yyleng; i++) putchar(yytext[i]); putchar('\t');}

{number}       {for (i=0; i<yyleng; i++) putchar(yytext[i]); putchar('\t');}

{symbol}       {for (i=0; i<yyleng; i++) putchar(yytext[i]); putchar('\t');}

{string}       {for (i=1; i<(yyleng-1); i++) putchar(yytext[i]); putchar('\t');}

")"            {putchar('\n');}

%%

main (argc, argv)
int argc;
char** argv;
{
  if (argc == 2)
  {
    FILE *ifile;
    int cmp1, cmp2;

    cmp1 = strcmp(argv[1],"help");
    cmp2 = strcmp(argv[1],"h");
    if ( (cmp1 == 0) || (cmp2 == 0) )
    {
      printf("\nUSAGE:\n");
      printf("\tListToTable inputfilename [> outputfilename]\n\n"); 
      return 0;
    }

    ifile = fopen(argv[1], "r");
    yyin = ifile;
    yylex();
    fclose(ifile);
    return 0;    
  }
}


