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

7.2 Program ~linebreaks.c~

This program reads a file from standard input and writes it to standard output. Whenever lines longer than LINELENGTH (which is 80 characters) occur, it puts a line break to the position of the last blank read before character 80 and continues in a new line. If there was no blank in such a line, it introduces a line break anyway (possibly in the middle of a word).

*/



#define LINELENGTH 80 
#define EOF -1
#define OR ||

int
main()  
{
	int position, c, lastblank, i;
	int line[LINELENGTH];		

	position = 0;
	lastblank = -1;

	while ((c = getchar()) != EOF) {
		line[position] = c;
		if (c == ' ') lastblank = position;
		position++;

		if ((c == '\n') OR (position == LINELENGTH))
			if (c == '\n') {

				/* output a complete line */

				for (i = 0; i < position; i++)
					putchar(line[i]);
				position = 0;
				lastblank = -1;}

			else if (lastblank > 0) { 	/* a blank exists */

				/* output line up to blank */

				for (i = 0; i < lastblank; i++)
					putchar(line[i]);
				putchar('\n');

				/* move rest of line to the front */

				for (i = lastblank + 1; i < position; i++)
					line[i - (lastblank + 1)] = line[i];
				position = position - (lastblank + 1);
				lastblank = -1;
				}

			     else {			/* no blank exists */

				/* output line anyway */

       				for (i = 0; i < position; i++)
					putchar(line[i]);
				putchar('\n');
				position = 0;
				lastblank = -1;
				}
	}

        return 0;
}
