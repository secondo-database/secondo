/*

6.2 Main Program: Maketex.c

Use the parser to transform from implicitly formatted text to TeX.

*/

int main()
{	int error=0;

	error = yyparse();
	print_tail();
        return error;
}

print_tail()
{
	printf("\\end{document}\n");
}	

