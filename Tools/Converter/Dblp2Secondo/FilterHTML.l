%{

%}

/* regular definitions */
%%

&auml;   { printf("ae");}
&Auml;   { printf("Ae");}
&ouml;   { printf("oe");}
&Ouml;   { printf("Oe");}
&uuml;   { printf("ue");}
&Uuml;   { printf("Ue");}
&amp;    { printf(" + ");}
&lt;     { printf("<"); }
&gt;     { printf(">");}
&Eacute; { printf("E");}
&quot;   { printf("\"");}
&nbsp;   { printf(" ");}
&iexcl;  { printf("!");}
&szlig;  { printf("ss");}
&#19[234567];    { printf("A");}
&#198;   { printf("Ae");}
&#199;   { printf("C");}
&#20[123];    { printf("E");}
&#20[4567];   { printf("I");}
&#208;    { printf("Eth");}
&#209;    { printf("N");}
&#21[0123456];   { printf("O"); }
&#21[789];       { printf("U"); }
&#220;           { printf("U");}
&#221;           { printf("Y");}
&#222;           { printf("?");}
&#223;           { printf("ss");}
&#22[456789];    { printf("a");}
&#230;           { printf("ae");}
&#23[12345];     { printf("e");}
&#23[6789];      { printf("i");}
&#240;           { printf("eth");}
&#241;           { printf("n");}
&#24[234568];    { printf("o");}
&#249;           { printf("u");}
&#25[012];       { printf("u");}
&25[35];         { printf("y");}
&#254;           { printf("?");}
&micro;          { printf("micro");}
&iuml;           { printf("i");}
&times;          { printf("x");}
&.grave;         { printf("%c",yytext[1]);}
&.ring;          { printf("%c",yytext[1]);}
&.circ;          { printf("%c",yytext[1]);}
&.acute;         { printf("%c",yytext[1]);}
&.tilde;         { printf("%c",yytext[1]);}
&reg;            { printf("(R)");}
&.cedil;         { printf("%c",yytext[1]);}
&.slash;         { printf("%c",yytext[1]);}
&..lig;          { printf("%c%c",yytext[1],yytext[2]);}
&.uml;           { printf("%c",yytext[1]);}
&#...;           { printf(" ");}
&#..;            { printf(" ");}
&eth;            { printf("eth");}
&ETH;            { printf("ETH");}
&THORN;          { printf("**");}

%%

int main (argc, argv)
int argc;
char** argv;
{
  FILE *ofile, *ifile;
  char c;
  int token;

  if(argc>1){
    ifile = fopen(argv[1], "r");
    if (ifile == NULL)
    {
      printf("ERROR: cannot open file ");
      printf(argv[1]);
      printf("\n");
      return -1;
    }
    yyin = ifile;
  }
  token = yylex();
  while (token != 0)
    token = yylex();
  return 0;
}

