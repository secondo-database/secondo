/*

6.2 Main Program: Maketex.c

Use the parser to transform from implicitly formatted text to TeX.

*/

main()
{	int error;

	error = yyparse();
	print_tail();
}

print_tail()
{
	printf("\\end{document}\n");
}	

