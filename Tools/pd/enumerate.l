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

File renumber.l 

*/

%{
int list = 0;
int sublist = 0;
int section = 0;
int subsection = 0;
int subsubsection = 0;
%}

digit           [0-9]
num             ({digit}{digit}|{digit})

 /* Die Anzahl der führenden Leerzeichen bestimmt die Aufzählungsebene */ 
list1           " "{digit}{digit}" "
list2           "  "{digit}" "
sublist1        "   "{digit}{digit}" "
sublist2        "    "{digit}" "

section         {num}" "
subsection      {num}"."{section}
subsubsection   {num}"."{subsection}

%%
^{list1}        { list++; sublist=0; printf(" %2u ", list); }
^{list2}        { list++; sublist=0; printf(" %2u ", list); }
^{sublist1}     { sublist++; printf("   %2u ", sublist); }
^{sublist2}     { sublist++; printf("   %2u ", sublist); }

^{section}         { section++; subsection=0; subsubsection=0; 
                     printf("%u ", section);
                   }
^{subsection}      { subsection++; subsubsection=0; 
                     printf("%u.%u ", section, subsection);
                   }
^{subsubsection}   { subsubsection++;  
                     printf("%u.%u.%u ", section, subsection, subsubsection);
                   }

%%
/*  Alle Zeichenfolgen, die nicht durch einen 'match' behandelt
 *  werden, werden auf der Standardausgabe ausgegeben
 */
main() { 
   yylex();  
}
