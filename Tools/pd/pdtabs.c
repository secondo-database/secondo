/*

7.1 Program pdtabs.c

This program converts tab symbols into corresponding sequences of blanks. With the standard text editor, each tab corresponds to 8 blanks.

*/

#define TABLENGTH 8
#define EOF -1

int
main()  
{
        int c, position, nblanks, i;

        position = 0;
        while ((c = getchar()) != EOF) 
                if (c == '\n')
                        {position = 0; putchar(c);}
                else if (c == '\t') {
                        nblanks = TABLENGTH - (position % TABLENGTH);
                        for (i = 0; i < nblanks; i++)
                                {position++; putchar(' ');}
                        }
                else {position++; putchar(c);}
       return 0;
}
