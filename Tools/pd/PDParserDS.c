/*

4 Data Structures for the Parser

(File ~PDParserDS.c~)

This file contains data structures for special paragraph and character formats (Section 4.2) and special characters (Section 4.3), also some auxiliary functions for the parser (Section 4.4).

*/
#include "PDNestedText.h"
#include <string.h>

#define AND &&


/**************************************************************************
4.1 Global Constants and Variables

**************************************************************************/

#define BRACKETLENGTH 500

/*****************************
Length of text that may occur in square brackets within normal text.

******************************/


int pindex = -1;
		
/*****************************
Index (in array ~definitions~, see below) of the most recently used special paragraph format.

*****************************/


int cindex = -1;
		
/*****************************
Index (in array ~definitions~, see below) of the most recently used special character format.

*****************************/



/**************************************************************************
4.2 Data Structure for Definitions of Special Paragraph or Character Formats

***************************************************************************/

#define DEFMAX 100
#define NAMELENGTH 30
#define COMLENGTH 100

struct def {
	int index;
	char name[NAMELENGTH];
	char open[COMLENGTH];
	char close[COMLENGTH];
} definitions[DEFMAX];

int first_free_def = 0;
int last_global_def = -1;

/******************************************
Contains definitions of special paragraph or character formats, such as

----	paragraph [1] Title: [{\bf \Large \begin{center}] [\end{center} }]
----

Here 1 would become the ~index~, ~Title~ would be the ~name~, the material enclosed in the first pair of square brackets would be the ~open~, and that in the second pair of brackets the ~close~ component.

Into this array are put first definitions from the header documentation section. When these are complete, the variable ~last-global-def~ is set to the array index of the last entry. Then, for each paragraph which has annotation lines further definitions may be appended. The lookup procedure ~lookup-def~ (see below) searches from the end so that if finds first paragraph annotations. (However, paragraph annotations are not yet implemented.)

The following function ~enter-def~ puts a quadruple into this array. The last three parameters are (indices of) list expressions.

****************************************/

enter_def(int index, int name, int open, int close)
{   definitions[first_free_def].index = index;
    copyout(name, definitions[first_free_def].name, NAMELENGTH);

	/* This function copies the first parameter list expression into a
        string given as a second parameter. Part of NestedText.*/

    copyout(open, definitions[first_free_def].open, COMLENGTH);
    copyout(close, definitions[first_free_def].close, COMLENGTH);

    first_free_def++;

    if (first_free_def >= DEFMAX)
	{fprintf(stderr, "Error in enter_def: table full.\n"); 
	exit(1);
	}
}

/*****************************************

Function ~lookup-def~ finds an array index in array ~definitions~ such that its ~index~ component has value ~i~. Starts at the end and searches backwards in order to find paragraph annotations first. Returns either the array index, or -1 if entry was not found.

*******************************************/

int lookup_def(int i)
{   int j;
    j = first_free_def;
    do j--;
    while ((j>=0) AND (definitions[j].index != i));
    return j;
}

/**************************************************************************
4.3 Data Structure for Definitions of Special Characters

***************************************************************************/

#define CODELENGTH 31		/* 30 usable characters + 0C */
#define SCMAX 100

struct schar {
	char code[CODELENGTH];
	char command[COMLENGTH];
} schars[SCMAX];

int first_free_schar = 0;

/*********************************************
Contains definitions of special characters such as:

----	[ue]	[\"{u}]
----

The function ~enter-schar~ puts such a pair into the data structure. Parameters are again indices of list expressions in array ~nodespace~. 

********************************************/

enter_schar(int code, int command)
{   copyout(code, schars[first_free_schar].code, CODELENGTH);
    copyout(command, schars[first_free_schar].command, COMLENGTH);

    first_free_schar++;

    if (first_free_schar >= SCMAX)
	{printf("Error in enter_schar: table full.\n"); 
	exit(1);
	}
}


/**************************************
The function ~lookup-schar~ tries to find a parameter ~string~ as a ~code~ component under some index ~j~ in the array ~schars~ with special character definitions. It returns this index. If such an entry was not found, it returns a negative index value.

**************************************/

int lookup_schar(char *string)
{   int j;
    j = first_free_schar;
    do j--;
    while ((j>=0) AND (strcmp(string, schars[j].code) != 0));
    return j;
}


/**************************************************************************
4.4 Auxiliary Functions

The function ~get-startref-index~ gets as a parameter a list expression ~listexpr~ containing a special paragraph format number in square brackets followed by a blank (the number can have one or two digits), or an empty pair of square brackets (as a reference to a special format used previously). Hence examples are:

----	[15] [7] or []
----

The function returns either the numeric value (the format number) or 0 for an empty pair of brackets.

**************************************/


int get_startref_index(int listexpr)
{   char ref[6];
    int length;

    copyout(listexpr, ref, 6);
    length = strlen(ref);

    switch(length) {
	case 3:					/* empty brackets */
		return 0;
	case 4: 				/* one digit */
		return (ref[1] - '0');
	case 5: 				/* two digits */
		return (10 * (ref[1] - '0') + (ref[2] - '0'));
	default:
		{fprintf(stderr, "Error in get_startref_index: length is %d.\n", 
			length);
		exit(1);
		}
    }
}

/*************************************

Function ~get-ref-index~ does the same for a reference without the trailing blank.

***************************************/

int get_ref_index(int listexpr)
{   return(get_startref_index( concat(listexpr, atom(" ", 1)) ));
}
