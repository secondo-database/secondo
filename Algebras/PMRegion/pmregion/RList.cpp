/* 
 * This file is part of libpmregion
 * 
 * File:   RList.cpp
 * Author: Florian Heinz <fh@sysv.de>

 1 RList
   Library internal nested list format
 
*/

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "PMRegion_internal.h"

using namespace pmr;

namespace pmr {

/*
  1.1 Constructor, which creates an empty nested list
 
*/ 
RList::RList() : type(NL_LIST) {
}

/*
   1.2 Appends a real number to this nested list
 
*/ 
void RList::append(double nr) {
    RList nl;
    
    nl.type = NL_DOUBLE;
    nl.nr = nr;
    items.push_back(nl);
}

/*
   1.3 Appends a string to this nested list
 
*/ 
void RList::append(string str) {
    RList nl;
    
    nl.type = NL_STRING;
    nl.str.append(str);
    items.push_back(nl);
}

/*
   1.3a Appends a symbol to this nested list
 
*/ 
void RList::appendsym(string str) {
    RList nl;
    
    nl.type = NL_SYM;
    nl.str.append(str);
    items.push_back(nl);
}

/*
   1.4 Appends a boolean value to this nested list.
 
*/ 
void RList::append(bool val) {
    RList nl;
    
    nl.type = NL_BOOL;
    nl.boolean = val;
    items.push_back(nl);
}

/*
   1.5 Appends another nested list to this nested list.
 
*/ 
void RList::append(RList l) {
    items.push_back(l);
}

void RList::prepend(RList l) {
    items.insert(items.begin(), l);
}

/*
   1.6 ~point~ is a convenience function which adds a nested
   list with the coordinates of a 2d-point to this nested list.

*/
RList* RList::point(double x, double y) {
    RList pt;
    pt.append(x);
    pt.append(y);
    append(pt);

    return this;
}

/*
   1.7 ~nest~ appends a new, empty nested list to this nested
   list and returns a pointer to it.
 
*/
RList* RList::nest() {
    RList nl;
    nl.type = NL_LIST;
    items.push_back(nl);
    
    return &items[items.size()-1];
}

/*
   1.8 ~concat~ appends the items of a given nested list to
   this nested list.

*/
void RList::concat(RList l) {
    items.insert(items.end(), l.items.begin(), l.items.end());
}

// Perform indentation for string output
static string do_indent (int indent) {
    string ret;
    
    ret.append("\n");
    while (indent--)
        ret.append("   ");
    
    return ret;
}

/*
   1.9 ~ToString~ creates a string representation from this
   nested list with a given indentation-level.

*/
string RList::ToString(int indent) {
    std::stringstream ret;
    
    if (type == NL_LIST) {
        ret << do_indent(indent);
        ret << "( ";
        for (unsigned int i = 0; i < items.size(); i++) {
            ret << items[i].ToString(indent+1);
            ret << " ";
        }
        ret << ")";
    } else if (type == NL_DOUBLE) {
        ret << std::setprecision(15) << nr;
    } else if (type == NL_BOOL) {
        ret << (boolean ? "TRUE" : "FALSE");
    } else if (type == NL_STRING) {
        ret << "\"" << str << "\"";
    } else if (type == NL_SYM) {
        ret << str;
    }
    return ret.str();
}

/*
   1.10 ~ToString~ creates a string representation from this
   nested list.

*/
string RList::ToString() {
    return ToString(0);
}

RList RList::obj(string name, string type) {
	RList obj;

	obj.appendsym("OBJECT");
	obj.appendsym(name);
	RList empty;
	obj.append(empty);
	obj.appendsym(type);
	obj.append(*this);

	return obj;
}



#define NUM 257
#define STR 258
#define SYM 259
static double num;
static char *str;
static int state = 0;

// Get the next token from input file (simple lexer)
static int getToken (std::istream& f) {
   static char buf[1024];
   char *ptr = buf;
   
   do {
      int ch = f.get();
      switch (ch) {
       case ')':
       case '(':
	 if (buf != ptr && state == NUM) {
		 f.unget();
		 num = atof(buf);
		 state = 0;
		 return NUM;
	 } else if (buf != ptr && state == SYM) {
		 f.unget();
		 str = buf;
		 state = 0;
		 return SYM;
	 }
	 return ch;

       case '\"':
	 state = STR;
	 if (buf != ptr) {
		 str = buf;
		 state = 0;
		 return STR;
	 }
	 break;
	 
       case '-':
       case '0':
       case '1':
       case '2':
       case '3':
       case '4':
       case '5':
       case '6':
       case '7':
       case '8':
       case '9':
       case '.':
	 if (state == 0)
		 state = NUM;
	 *ptr++ = ch;
	 *ptr = '\0';
	 break;
	 
       case ' ':
       case '\r':
       case '\n':
       case '\t':
	 if (buf != ptr && state == NUM) {
		 num = atof(buf);
		 state = 0;
		 return NUM;
	 } else if (buf != ptr && state == SYM) {
		 str = buf;
		 state = 0;
		 return SYM;
	 }
	 break;
	 
       case EOF:
	 return 0;
	 
       default:
	 if (state == STR || state == SYM) {
		 *ptr++ = ch;
		 *ptr = '\0';
	 } else if (state == 0) {
		 state = SYM;
		 *ptr++ = ch;
		 *ptr = '\0';
	 } else {
		 printf("Parse error: '%c' (state: %d)\n", ch, state);
		 exit(EXIT_FAILURE);
	 }
      }
   } while (!(f.rdstate() & std::istream::eofbit));
   
   return 0;
}

// Parse the textual nested list and create a 
// RList-object from it (a simple parser).
static int _parse (std::istream& f, RList *nl, int depth) {
	int token;
	do {
		token = getToken(f);
		switch (token) {
			case '(':
				_parse(f, nl->nest(), depth+1);
				break;
			case NUM:
				nl->append(num);
				break;
			case STR:
				nl->append((string)str);
				break;
			case SYM:
				if (strcmp(str, "TRUE") == 0)
					nl->append((bool)true);
				else if (strcmp(str, "FALSE") == 0)
					nl->append((bool)false);
				else
					nl->appendsym((string)str);
				break;
			case ')':
				return 0;

		}
	} while (token);

	if (depth == 0 && token == 0)
		return 0;
	else
		return -1;
}

RList RList::parse (std::istream& f) {
	RList nl;

	int st = _parse(f, &nl, 0);
	if (st != 0)
		cerr << "Parse error!" << std::endl;

	return nl.items[0];
}

}

