
/*

*/

%{
int i;
%}

blank		[ ]
tab		[\t]
newline		[\n]
fieldsep	({blank}|{tab})*{tab}({blank}|{tab})*
rowsep		({blank}|{tab})*({newline}({blank}|{tab})*)+
char		[^\t\n]
nonblankchar	[^\ \t\n]
string		{nonblankchar}({char}*{nonblankchar})*
digit		[0-9]
nonzero		[1-9]
zero		[0]
int		-?{nonzero}{digit}*|{zero}
bool		TRUE|FALSE
real		-?{digit}+\.({digit}+)?(E-?{digit}+)?
start		=+({blank}|{tab})*({newline}({blank}|{tab})*)+
end		({blank}|{tab})*({newline}({blank}|{tab})*)+=+{newline}?

%%
{int}		{for (i=0; i<yyleng; i++) putchar(yytext[i]);}
{bool}		{for (i=0; i<yyleng; i++) putchar(yytext[i]);}
{real}		{for (i=0; i<yyleng; i++) putchar(yytext[i]);}
{string}      	{printf("\""); for (i=0; i<yyleng; i++) 
			putchar(yytext[i]); printf("\"");}

{fieldsep}	printf(" ");
{rowsep}	printf(")\n\t(");
{start}		printf("\t(");
{end}		printf(")\n");

%%

main (argc, argv)
int argc;
char** argv;
{
  if (argc == 2)
  {
    FILE *ofile, *ifile;
    char c;
    int cmp1, cmp2;

    cmp1 = strcmp(argv[1],"help");
    cmp2 = strcmp(argv[1],"h");
    if ( (cmp1 == 0) || (cmp2 == 0) )
    {
      printf("\nUSAGE:\n");
      printf("\tTableToList inputfilename [> outputfilename]\n\n"); 
      return 0;
    }

    ofile = fopen("TTL", "w");
    putc('=', ofile);
    putc('\n', ofile);
    fclose(ofile);
    ofile = fopen("TTL", "a");
    ifile = fopen(argv[1], "r");
    if (ifile == NULL)
    {
      printf("ERROR: cannot open file ");
      printf(argv[1]);
      printf("\n");
      return 0;
    } 
    while ((c = getc(ifile)) != EOF)
      putc(c, ofile);
    putc('\n', ofile);
    putc('=', ofile);
    fclose(ifile);
    fclose(ofile);
    ofile = fopen("TTL", "r");
    yyin = ofile;
    printf("(\n");
    yylex();
    printf(")\n");
    fclose(ofile);
    remove("TTL");
    return 0;    
  }
  else
  {
    printf("ERROR: wrong number of parameters\n");
    printf("Try TableToList h or TableToList help for more information\n");
    return 0;
  }
}

