/*
//[\]	[$\setminus $]


5 The Parser (HTML Version)

(File ~PDParserHTML.y~)

5.1 Introduction

This file contains a ~yacc~ specification of a parser which is transformed by the UNIX tool ~yacc~ into a program file ~y.tab.c~ which in turn is compiled to produce a parser. For an introduction to ~yacc~ specifications see [ASU86, Section 4.9]. Detailed information is given in [SUN88, Section 11].

In fact, the specification contains ``semantic rules'' (or ~actions~); hence, the generated program is a compiler (from PD files into HTML). The parsing technique used is bottom-up LR parsing. Let us briefly consider two example rules:

----	heading		: heading1
			| heading2
			| heading3
			| heading4
			| heading5
			;

	heading1	:  HEAD1 paragraph_rest	{printf("<H1>");
						print($2);
						printf("</H1>\n\n");}
			;
----

The first is a grammar rule stating that a ~heading~ (nonterminal) can be derived into either a ~heading1~ or a ~heading2~ or a ~heading3~ (nonterminal). The second rule says that a ~heading1~ consists of a ~HEAD1~ token followed by a ~paragraph-rest~. This rule has an associated action which is C code enclosed by braces.

Attached to each grammar symbol (nonterminal, or terminal token) is an integer-valued ~attribute~. One can refer to these attributes in a grammar rule by the names ~\$\$~, ~\$1~, ~\$2~, etc. where ~\$\$~ refers to the attribute of the nonterminal on the left hand side and ~\$1~, ~\$2~, etc. to the grammar symbols on the right hand side. Hence, in the second rule,  ~\$\$~ is the attribute attached to ~heading1~, ~\$1~ belongs to ~HEAD1~, and ~\$2~ to ~paragraph-rest~.

For the terminal tokens which are generated in lexical analysis the attribute value is set by an assignment to the variable ~yylval~. The lexical analyser used here assigns always the index of a node of the ~NestedText~ data structure containing the character string matching the token.

For the nonterminals, the attribute value is set by an assignment to ~\$\$~ in the action part of a grammar rule. The bottom-up parser basically works as follows. Terminal tokens and nonterminals recognized earlier are kept on a stack. Whenever the top symbols on the stack correspond to the right hand side of a grammar rule which is applicable with the current derivation, a ~reduction~ is made: The right hand side symbols are removed from the stack and the left hand side nonterminal is put on the stack. In addition, the action associated with the rule (the C code in braces) is executed.

Therefore, in our example rule the ~HEAD1~ and ~paragraph-rest~ symbols are removed from the stack and a ~heading1~ symbol is put on top. The action is executed, namely:

  1 The text ``\verb+<H1>+'' is written to the output file.

  2 The text associated with ~paragraph-rest~ (a ~NestedText~ node whose index is given in ~\$2~) is written (by the ~print~ function from ~NestedText~).

  3 The text ``\verb+</H1>\n\n+'' (a closing brace followed by two end-of-line characters) is written to the output.

Hence for a heading described in the PD file by

----	5 The Parser
----

a piece of HTML code

----	<H1>The Parser</H1>
----

followed by an empty line is written to the output file. To understand the output created in the grammar rules you need to know a little HTML (see, for example, [HTML95]).


5.2 Declaration Section: Definition of Tokens

*/

%{
#include <stdio.h>
#include <stdlib.h>
#include "PDNestedText.h"
#include "PDParserDS.c"
%}

%token OPEN, CLOSE, EPAR, DEFLINE, LETTER, DIGIT, OTHER, TILDE, STAR,
	QUOTE, BLANKTILDE, BLANKSTAR, BLANKQUOTE,
	HEAD1, HEAD2, HEAD3, HEAD4, HEAD5, ENUM1, ENUM2, BULLET1, BULLET2,
	FOLLOW1, FOLLOW2, DISPLAY, FIGURE, STARTREF, REF, VERBATIM,
	PARFORMAT, CHARFORMAT
%%

/*
5.3 Document Structure and Program Sections

*/

document 	: doc
		| doc program_section
		;

doc		: space doc_section 
		| doc program_section doc_section
		;

doc_section 	: OPEN elements CLOSE  
		;

program_section : 			{printf("<PRE>\n");}
		  chars			{printf("</PRE>\n");}
		;

chars 		:	
		| chars text_char	{print($2);}
		| chars TILDE		{print($2);}
		| chars STAR		{print($2);}
		| chars QUOTE		{print($2);}
		| chars '\"'		{print($2);}
		| chars '*'		{print($2);}
		| chars '~'		{print($2);}
		| chars '['		{print($2);}
		| chars ']'		{print($2);}
		| chars follow_elem	{print($2);}
		| chars EPAR		{print($2);}
		| chars DEFLINE		{print($2);}
		;

elements	: 
		| elements definitions
		| elements element	{release_storage();}
		| elements list		/* storage cannot be released after
					lists because of look-ahead */
		;


/*
5.4 Definitions of Special Formats and Characters

*/

definitions	: defs EPAR
		;

defs		: defline	                  
		| defs defline	
		;

defline		: DEFLINE par_format
		| DEFLINE char_format
		| DEFLINE special_char_def
		;

par_format	: PARFORMAT space REF space ident space ':'
			space bracketed2 space bracketed2
				{
				/* test only:
				print($3);
				printf("paragraph definition: %d ", 
					get_ref_index($3));
				print($5);
				print($9);
				print($11);
				*/
				enter_def(get_ref_index($3), $5, $9, $11);}
		;

char_format	: CHARFORMAT space REF space ident space ':'
			space bracketed2 space bracketed2
				{
				/*
				printf("characters definition: %d ", 
					get_ref_index($3));
				print($5);
				print($9);
				print($11);
				*/

				enter_def(get_ref_index($3), $5, $9, $11);}
		;

special_char_def: bracketed2 space bracketed2
					{enter_schar($1, $3);}
		;

space		:
		| space ' '
		| space FOLLOW1
		| space FOLLOW2
		| space DISPLAY
		| space FIGURE
		;

ident 		: LETTER		{$$ = $1;}
		| ident LETTER		{$$ = concat($1, $2);}
		| ident DIGIT		{$$ = concat($1, $2);}
		;

/*
5.5 Text Elements 

5.5.1 Predefined Paragraph Types 

*/

element 	: standard_paragraph
		| heading
		| verb
		| display
		| figure
		| special_paragraph
		;

standard_paragraph : paragraph_rest	{print($1); printf("<P>\n\n");}
		;

heading		: heading1
		| heading2
		| heading3
		| heading4
		| heading5
		;

heading1	: HEAD1 paragraph_rest	{printf("<H1>");
					print($2);
					printf("</H1>\n\n");}
		;

heading2	: HEAD2 paragraph_rest	{printf("<H2>");
					print($2);
					printf("</H2>\n\n");}
		;

heading3	: HEAD3 paragraph_rest	{printf("<H3>");
					print($2);
					printf("</H3>\n\n");}
		;

heading4	: HEAD4 paragraph_rest	{printf("<H4>");
					print($2);
					printf("</H4>\n\n");}
		;

heading5	: HEAD5 paragraph_rest	{printf("<H5>");
					print($2);
					printf("</H5>\n\n");}
		;

verb		: verb_start verb_end
		;

verb_start	: VERBATIM		{printf("<PRE>\n    ");}
		;

verb_end	: chars VERBATIM	{printf("</PRE>\n");}
		;


display		: DISPLAY paragraph_rest {printf("<BLOCKQUOTE>\n");
					printf("        ");
					print($2);
					printf("\n</BLOCKQUOTE>\n\n");}
		;

/*
5.5.2 Figures

*/

figure 		: FIGURE figure_text optional_caption
		  bracketed2 annotations
					{printf("<IMG SRC=\"");
					print($4);
					printf("\" ALT=\"");
					print($2);
					printf("\"> ");
					print($3);
					printf("<P>\n\n");}
		;

/* 
Here a figure description of the form

----	Figure 1: Picture of something [filename]
----

is transformed into a text in HTML:

----	<IMG SRC="filename" ALT="Figure 1"> Picture of something <P>
----

*/

optional_caption:			{$$ = atomc("");}
		| ':' figure_text	{$$ = $2;}
		;

figure_text 	:			{$$ = atomc("");}
		| figure_text ftext_char {$$ = concat($1, $2);}
		| figure_text TILDE 	{$$ = concat($1, atomc("~"));}
		| figure_text STAR 	{$$ = concat($1, atomc("*"));}
		| figure_text QUOTE 	{$$ = concat($1, atomc("\""));}
		| figure_text emphasized {$$ = concat($1, $2);}
		| figure_text bold_face	{$$ = concat($1, $2);}
		| figure_text special_char_format {$$ = concat($1, $2);}
		| figure_text follow_elem {$$ = concat($1, $2);}
		;

/*
5.5.4 Special Paragraph Formats

*/

special_paragraph: STARTREF paragraph_rest
				
		{int i;

		i = get_startref_index($1);
		if (i > 0)			/* not an empty start ref */
		    pindex = lookup_def(i);
						/* otherwise use previous 
						pindex value */
		if (pindex >= 0)		/* def was found */
		    {printf("%s ", definitions[pindex].open);
		    print($2);
		    printf("%s \n\n", definitions[pindex].close);
		    }
		else print($2);		/* make it a standard paragraph */
		}
		;


/*
5.6 Lists

*/

list 		: itemized1	{printf("<UL>\n");
				print($1);
				printf("\n</UL>\n\n");}
		| enum1		{printf("<OL>\n");
				print($1);
				printf("\n</OL>\n\n");}
		;

itemized1 	: bulletitem1		{$$ = $1;}
		| itemized1 bulletitem1	{$$ = concat($1, $2);}
		;

bulletitem1 	: bulletpar1		{$$ = $1;}
		| bulletitem1 followup1	{$$ = concat($1, $2);}
		| bulletitem1 list2	{$$ = concat($1, $2);}
		;

bulletpar1 	: BULLET1 paragraph_rest
				{$$ = concat(atomc("\n  <LI>"), 
					concat($2, atomc("\n\n")));}
		;

followup1 	: FOLLOW1 paragraph_rest 
					{$$ = concat($1,
						concat($2, atomc("\n\n")));}
		;

enum1 		: enumitem1		{$$ = $1;}
		| enum1 enumitem1	{$$ = concat($1, $2);}
		;

enumitem1 	: enumpar1		{$$ = $1;}
		| enumitem1 followup1	{$$ = concat($1, $2);}
		| enumitem1 list2	{$$ = concat($1, $2);}
		;

enumpar1 	: ENUM1 paragraph_rest
				{$$ = concat(atomc("\n  <LI>"), 
					concat($2, atomc("\n\n")));}
		;

list2 		: itemized2 {$$ = concat(atomc("\n    <UL>\n"),
					concat($1,
					atomc("\n    </UL>\n\n")));}
		| enum2	{$$ = concat(atomc("\n    <OL>\n"),
					concat($1,
					atomc("\n    </OL>\n\n")));}
		;

itemized2 	: bulletitem2		{$$ = $1;}
		| itemized2 bulletitem2	{$$ = concat($1, $2);}
		;

bulletitem2 	: bulletpar2		{$$ = $1;}
		| bulletitem2 followup2	{$$ = concat($1, $2);}
		;

bulletpar2 	: BULLET2 paragraph_rest 
				{$$ = concat(atomc("\n      <LI>"), 
					concat($2, atomc("\n\n")));}
		;

followup2 	: FOLLOW2 paragraph_rest
				{$$ = concat($1, concat($2, atomc("\n\n")));} 
		;

enum2 		: enumitem2		{$$ = $1;}
		| enum2 enumitem2	{$$ = concat($1, $2);}
		;

enumitem2 	: enumpar2		{$$ = $1;}
		| enumitem2 followup2	{$$ = concat($1, $2);}
		;

enumpar2 	: ENUM2 paragraph_rest
				{$$ = concat(atomc("\n      <LI>"), 
					concat($2, atomc("\n\n")));}

		;

/*
5.7 Text Structure


Unfortunately, this gets a bit complex because we must take care of the following:

  * Most tokens that are recognized in lexical analysis may occur in normal text; we must make sure that they can be reduced there. In particular, the tokens defining paragraph formats must be allowed to occur in the middle of a paragraph and not be interpreted there.

  * We must take care of escaping characters with a special meaning. This concerns the ~TILDE~, ~STAR~, and ~QUOTE~ tokens which are formed in LA from
the corresponding characters in square brackets. In normal text, the square brackets must be stripped off. On the other hand, in program text or in definitions (given in square brackets themselves) the character strings should be left untouched.

  * Emphasized text (enclosed by tilde characters) and bold face text (enclosed by stars) may be nested, but we must make sure through the grammar rules that emphasized cannot occur within emphasized etc. Otherwise a second tilde meaning a closing bracket of the emphasized text would be shifted on the stack by the parser rather than reduced as we want.

*/
			
paragraph_rest 	: text annotations EPAR	{$$ = $1;}
		;

text 		: 			{$$ = atomc("");}
		| netext		{$$ = $1;}
		;

netext		: start_elem		{$$ = $1;}
		| netext start_elem	{$$ = concat($1, $2);}
		| netext follow_elem	{$$ = concat($1, $2);}
		;

start_elem	: text_char		{$$ = $1;}
		| TILDE 		{$$ = atomc("~");}
		| STAR 			{$$ = atomc("*");}
		| QUOTE 		{$$ = atomc("\"");}
		| emphasized		{$$ = $1;}
		| bold_face		{$$ = $1;}
		| special_char_format	{$$ = $1;}
		| bracketed		{$$ = $1;}
		;

follow_elem	: HEAD1			{$$ = $1;}
		| HEAD2			{$$ = $1;}
		| HEAD3			{$$ = $1;}
		| HEAD4			{$$ = $1;}
		| HEAD5			{$$ = $1;}
		| ENUM1			{$$ = $1;}
		| ENUM2			{$$ = $1;}
		| FOLLOW1		{$$ = $1;}
		| FOLLOW2		{$$ = $1;}
		| BULLET1		{$$ = $1;}
		| BULLET2		{$$ = $1;}
		| DISPLAY		{$$ = $1;}
		| FIGURE		{$$ = $1;}
		| STARTREF		{$$ = $1;}
		| REF			{$$ = $1;}
		;

text_char	: ftext_char		{$$ = $1;}
		| ':'			{$$ = $1;}
		;

ftext_char 	: LETTER 		{$$ = $1;}
		| DIGIT 		{$$ = $1;}
		| OTHER			{$$ = $1;}
		| BLANKTILDE 		{$$ = $1;}
		| BLANKSTAR 		{$$ = $1;}
		| BLANKQUOTE		{$$ = $1;}
		| '\\'			{$$ = $1;}
		| ' '			{$$ = $1;}
		| '.'			{$$ = $1;}
		| PARFORMAT		{$$ = $1;}
		| CHARFORMAT		{$$ = $1;}
		;

emphasized 	: '~' unemph_list '~'	{$$ = concat(atomc("<EM>"),
						concat($2, 
						atomc("</EM>")));}
		;

bold_face 	: '*' unbold_list '*'	{$$ = concat(atomc("<STRONG>"),
						concat($2, 
						atomc("</STRONG>")));}
		;

unemph_list	:			{$$ = atomc("");}
		| unemph_list unemph	{$$ = concat($1, $2);}
		;

unemph		: text_char		{$$ = $1;}
		| TILDE 		{$$ = atomc("~");}
		| STAR 			{$$ = atomc("*");}
		| QUOTE 		{$$ = atomc("\"");}
		| follow_elem		{$$ = $1;}
		| '*' unboldemph_list '*'	{$$ = concat(atomc("<STRONG>"),
						concat($2, 
						atomc("</STRONG>")));}
		| special_char_format	{$$ = $1;}
		| bracketed		{$$ = $1;}
		;

unbold_list	:			{$$ = atomc("");}
		| unbold_list unbold	{$$ = concat($1, $2);}
		;

unbold		: text_char		{$$ = $1;}
		| TILDE 		{$$ = atomc("~");}
		| STAR 			{$$ = atomc("*");}
		| QUOTE 		{$$ = atomc("\"");}
		| follow_elem		{$$ = $1;}
		| '~' unboldemph_list '~'	{$$ = concat(atomc("<EM>"),
						concat($2, 
						atomc("</EM>")));}
		| special_char_format	{$$ = $1;}
		| bracketed		{$$ = $1;}
		;

unboldemph_list	:			{$$ = atomc("");}
		| unboldemph_list unboldemph	{$$ = concat($1, $2);}
		;

unboldemph	: text_char		{$$ = $1;}
		| TILDE 		{$$ = atomc("~");}
		| STAR 			{$$ = atomc("*");}
		| QUOTE 		{$$ = atomc("\"");}
		| follow_elem		{$$ = $1;}
		| special_char_format	{$$ = $1;}
		| bracketed		{$$ = $1;}
		;

plain_list	:			{$$ = atomc("");}
		| plain_list plain	{$$ = concat($1, $2);}
		;

plain		: text_char		{$$ = $1;}
		| TILDE 		{$$ = atomc("~");}
		| STAR 			{$$ = atomc("*");}
		| QUOTE 		{$$ = atomc("\"");}
		| bracketed		{$$ = $1;}
		;

/*
5.8 Special Character Formats

*/

special_char_format : 	'\"' plain_list '\"' REF
	
			{int i;

			i = get_ref_index($4);
			cindex = lookup_def(i);
			if (cindex >= 0)	/* def was found */
			    {$$ = concat(
				   atomc(definitions[cindex].open),
				   concat($2,
				     atomc(definitions[cindex].close)  ));
			    }
			else			/* ignore special format */
			    $$ = $2;
			}

		| '\"' plain_list '\"'			

			{if (cindex >= 0)	/* def exists */
			    {$$ = concat(
				   atomc(definitions[cindex].open),
				   concat($2,
				     atomc(definitions[cindex].close)  ));
			    }
			else			/* ignore special format */
			    $$ = $2;
			}

		;

/*
5.9 Text in Square Brackets: Checking for Special Characters

*/

bracketed 	: '[' btext ']'		

		{char bracketstring[BRACKETLENGTH];
		int i;
		int length;

		copyout($2, bracketstring, BRACKETLENGTH);
		length = strlen(bracketstring);

		if (length <= CODELENGTH - 1)
		    {i = lookup_schar(bracketstring);
		    if (i >= 0)			/* found */
			$$ = atomc(schars[i].command);
		    else
			$$ = concat($1,
				concat($2, $3));
		    }
		else
		    $$ = concat($1, concat($2, $3));
		}
		;

btext 		:			{$$ = atomc("");}
		| btext text_char	{$$ = concat($1, $2);}
		| btext TILDE 		{$$ = concat($1, atomc("~"));}
		| btext STAR 		{$$ = concat($1, atomc("*"));}
		| btext QUOTE 		{$$ = concat($1, atomc("\""));}
		| btext follow_elem	{$$ = concat($1, $2);}
		| btext '\"'		{$$ = concat($1, $2);}
		| btext '*'		{$$ = concat($1, $2);}
		| btext '~'		{$$ = concat($1, $2);}
		| btext '[' btext ']'	

		{char bracketstring[BRACKETLENGTH];
		int i;
		int length;

		copyout($3, bracketstring, BRACKETLENGTH);
		length = strlen(bracketstring);

		if (length <= CODELENGTH - 1)
		    {i = lookup_schar(bracketstring);
		    if (i >= 0)			/* found */
			$$ = concat($1, atomc(schars[i].command));
		    else
			{$$ = concat($1, 
				concat($2,
					concat($3, $4)));}
		    }
		else
		    {$$ = concat($1, 
				concat($2,
					concat($3, $4)));}
		}
		;

/*
5.10 Uninterpreted Square Brackets (Used in Definitions)

*/

bracketed2 	: '[' btext2 ']'	{$$ = $2;}
		;

btext2 		:			{$$ = atomc("");}
		| btext2 text_char	{$$ = concat($1, $2);}
		| btext2 TILDE		{$$ = concat($1, $2);}
		| btext2 STAR		{$$ = concat($1, $2);}
		| btext2 QUOTE		{$$ = concat($1, $2);}
		| btext2 follow_elem	{$$ = concat($1, $2);}
		| btext2 '\"'		{$$ = concat($1, $2);}
		| btext2 '*'		{$$ = concat($1, $2);}
		| btext2 '~'		{$$ = concat($1, $2);}
		| btext2 '[' btext2 ']'	{$$ = concat($1, 
						concat($2,
						concat($3, $4)));}
		;

annotations 	: 
		;
	

%%


#include "lex.yy.c"

int yyerror(const char *msg)
{
  if (yytext[0] == '\n' && yytext[1] == '\n')
    fprintf(stderr, "%s in paragraph before line %d.\n", msg, yylineno);
  else
    fprintf(stderr,
            "%s at line %d reading symbol '%s'.\n", msg, yylineno, yytext);
 
}

