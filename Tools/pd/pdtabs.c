/*
----
This file is part of the PD system
Copyright (C) 1998 Ralf Hartmut Gueting, Fachbereich Informatik, FernUniversitaet Hagen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
----

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
