/****************************************************************************

2.2 Implementation Part

(File ~PDNestedText.c~)

*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "PDNestedText.h"

#define AND &&
#define TRUE 1
#define FALSE 0
#undef NULL
#define NULL -1

#define STRINGMAX 300000
/*
Maximal number of characters in buffer ~text~.

*/

#define NODESMAX 30000
/*
Maximal number of nodes available from ~nodespace~.

*/

struct listexpr {
    int left;
    int right;
    int atomstring;		/* index into array text */
    int length;			/* no of chars in atomstring*/
};

/*
If ~left~ is NULL then this represents an atom, otherwise it is a list in which case ~atomstring~ must be NULL.

*/
struct listexpr nodespace[NODESMAX];
int first_free_node = 0;

char text[STRINGMAX];
int first_free_char = 0;

/***************************************

The function ~atom~ creates from a character string ~string~ of length ~length~ a list expression which is an atom containing this string. Possible errors: The text buffer or storage space for nodes may overflow.

***************************************/

int atom(char *string, int length)
{
    int newnode;
    int i;

    /* put string into text buffer: */

    	if (first_free_char + length > STRINGMAX)
	    {fprintf(stderr, "Error: too many characters.\n"); exit(1);}

    	for (i = 0; i< length; i++)
	    text[first_free_char + i] = string[i];
    
    /* create new node */

    	newnode = first_free_node++;
    	if (first_free_node > NODESMAX)
	    {fprintf(stderr, "Error: too many nodes.\n"); exit(1);}

   	nodespace[newnode].left = NULL;
  	nodespace[newnode].right = NULL;
  	nodespace[newnode].atomstring = first_free_char; 
	    first_free_char = first_free_char + length;
	nodespace[newnode].length = length;
    	return(newnode);
}

/****************************************

The function ~atomc~ works like ~atom~ except that the parameter should be a null-terminated string. It determines the length itself. To be used in particular for string constants written directly into the function call.

******************************************/

int atomc(char *string)
{   int length;

    length = strlen(string);
    return(atom(string, length));
}

/**************************************

The function ~concat~ concats two lists; it returns a list expression representing the concatenation. Possible error: the storage space for nodes may be exceeded.

****************************************/

int concat(int list1, int list2)
{
    int newnode;

    newnode = first_free_node++;

    if (first_free_node > NODESMAX)
	{fprintf(stderr, "Error: too many nodes.\n"); exit(1);}

    nodespace[newnode].left = list1;
    nodespace[newnode].right = list2;
    nodespace[newnode].atomstring = NULL;
    nodespace[newnode].length = 0;
    return(newnode);
}

/***************************************

Function ~print~ writes the character strings from all atoms in ~list~ in the right order to standard output. 

*****************************************/

print(int list)
{
    int i;

    if (isatom(list))
	for (i = 0; i < nodespace[list].length; i++)
	    putchar(text[nodespace[list].atomstring + i]);
    else
	{print(nodespace[list].left); print(nodespace[list].right);};
}

/******************************************

Function ~isatom~ obviously checks whether a list expression ~list~ is an atom.

*******************************************/

int isatom(int list)
{
    if (nodespace[list].left == NULL) return(TRUE);
    else return(FALSE);
}

/*******************************************

The function ~copyout~ copies the character strings from all atoms in ~list~ in the right order into a string variable ~target~. Parameter ~lengthlimit~ ensures that the maximal available space in ~target~ is respected; an error occurs if the list expression ~list~ contains too many characters. ~Copyout~ just calls an auxiliary recursive procedure ~copylist~ which does the job.

*******************************************/

copyout(int list, char *target, int lengthlimit)	
{   int i;

    i = copylist(list, target, lengthlimit);
    target[i] = '\0';
}

int copylist(int list, char *target, int lengthlimit)
{   int i, j;

    if (isatom(list))
	if (nodespace[list].length <= lengthlimit - 1)
	    {for (i = 0; i < nodespace[list].length; i++)
		target[i] = text[nodespace[list].atomstring + i];
	    return nodespace[list].length;
	    }
	else
	    {fprintf(stderr, "Error in copylist: too long text.\n"); print(list);
	    exit(1);
	    }
    else
	{i = copylist(nodespace[list].left, target, lengthlimit);
	j = copylist(nodespace[list].right, &target[i], lengthlimit - i);
	return (i+j);
	}
}

/****************************************

Function ~release-storage~ destroys the contents of the text and node buffers. Should be used only when a complete piece of text has been recognized and written to the output. Do not use it for text pieces whose recognition needs look-ahead!

*****************************************/

release_storage()
{   first_free_char = 0;
    first_free_node = 0;
}

/****************************************

Function ~show-storage~ writes the contents of the text and node buffers to standard output; only used for testing.

*****************************************/

show_storage()
{   int i;
    for (i = 0; i < first_free_char; i++) putchar(text[i]);

    for (i = 0; i < first_free_node; i++)
	 printf("node: %d, left: %d, right: %d, atomstring: %d, length: %d\n",
		i, nodespace[i].left, nodespace[i].right, 
		nodespace[i].atomstring, nodespace[i].length);
}
