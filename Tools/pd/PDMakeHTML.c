/*

3 Main Program: MakeHTML.c

Use the parser to transform from implicitly formatted text to HTML.

*/

int main() {	

    int error=0;
    error = yyparse();
    return error;
}
