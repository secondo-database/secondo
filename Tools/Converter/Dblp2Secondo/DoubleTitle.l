
%{

int i;

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

  if(argc > 1) {
    ifile = fopen(argv[1], "r");
    if (ifile == NULL) {
         fprintf(stderr,"ERROR: cannot open file ");
         fprintf(stderr,argv[1]);
         fprintf(stderr,"\n");
         return -1;
    }  
    yyin = ifile;
  }  
  token = yylex();
  while (token != 0)
    token = yylex();
  return 0; 
}
