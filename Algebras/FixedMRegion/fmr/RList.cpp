/* 
----
 * This file is part of libfmr
 * 
 * File:   RList.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 6, 2016, 11:48 AM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the class ~RList~

[TOC]

1 Overview

RLists are a form of nested lists used to represent the various objecttypes
in this library. It is used to convert from/to a textual representation or
from/to foreign objects.

*/

#include "fmr_RList.h"
#include <sstream>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace fmr;

/*
2 Constructor
 
Creates an empty nested list.

*/
RList::RList() : type(NL_LIST) {
}

/*
3 ~append~ double

Appends a real number to this list.
 
*/
void RList::append(double nr) {
    RList nl;

    nl.type = NL_DOUBLE;
    nl.nr = nr;
    items.push_back(nl);
}

/*
4 ~append~ string

Appends a string to this list.
 
*/
void RList::append(std::string str) {
    RList nl;

    nl.type = NL_STRING;
    nl.str.append(str);
    items.push_back(nl);
}

/*
5 ~appendSym~ string

Appends a symbol to this list.
 
*/
void RList::appendSym(std::string str) {
    RList nl;

    nl.type = NL_SYM;
    nl.str.append(str);
    items.push_back(nl);
}

/*
6 ~append~ bool

Appends a boolean value to this list.
 
*/
void RList::append(bool val) {
    RList nl;

    nl.type = NL_BOOL;
    nl.boolean = val;
    items.push_back(nl);
}

/*
7 ~append~ RList

Appends a sublist to this list.
 
*/
void RList::append(RList l) {
    items.push_back(l);
}

/*
8 ~nest~

Appends a new, empty nested list to this nested
list and returns a reference to it.
 
*/
RList& RList::nest() {
    RList nl;
    nl.type = NL_LIST;
    items.push_back(nl);

    return items[items.size() - 1];
}

/*
9 ~concat~

Appends the items of a given list to this list.

*/
void RList::concat(RList l) {
    items.insert(items.end(), l.items.begin(), l.items.end());
}

// Perform indentation for string output
static std::string do_indent(int indent) {
    std::string ret;

    ret.append("\n");
    while (indent--)
        ret.append("   ");

    return ret;
}

/*
10 ~ToString~

Creates a string representation from this
list with a given indentation level.

*/
std::string RList::ToString(int indent) {
    std::stringstream ret;

    if (type == NL_LIST) {
        ret << do_indent(indent);
        ret << "( ";
        for (unsigned int i = 0; i < items.size(); i++) {
            ret << items[i].ToString(indent + 1);
            ret << " ";
        }
        ret << ")";
    } else if (type == NL_DOUBLE) {
        ret << nr;
    } else if (type == NL_BOOL) {
        ret << (boolean ? "TRUE" : "FALSE");
    } else if (type == NL_STRING) {
        ret << "\"" << str << "\"";
    } else if (type == NL_SYM) {
        ret << "" << str << "";
    }
    return ret.str();
}

/*
11 ~ToString~

Creates a string representation from this nested list.

*/
std::string RList::ToString() {
    return ToString(0);
}

/*
12 ~SecondoObject~

Constructs a Secondo-compatible object with name ~name~ and type
~type~ from this nested list. Useful for saving object files,
which can be read in by secondo.

*/
RList RList::SecondoObject(std::string name, std::string type) {
    RList ret;
    
    ret.appendSym("OBJECT");
    ret.appendSym(name);
    ret.nest();
    ret.appendSym(type);
    ret.append(*this);
    
    return ret;
}

/*
13 Parser

Parses an RList from a file.

*/
#define NUM 257
#define STRING 258
#define SYM 259
#define BOOL 260
static double num;
static std::string str;
static bool b;

static int getToken(FILE *f) {
    char buf[1024], *ptr = buf;
    int curtype = 0;

    do {
        int ch = fgetc(f);
        switch (ch) {
            case ')':
            case '(':
                if (buf == ptr)
                    return ch;

            case ' ':
            case '\r':
            case '\n':
            case '\t':
                if (buf != ptr) {
                    ungetc(ch, f);
                    int _curtype = curtype;
                    curtype = 0;
                    switch (_curtype) {
                        case NUM:
                            num = atof(buf);
                            return NUM;
                        case SYM:
                            if (!strcasecmp(buf, "TRUE")) {
                                b = true;
                                return BOOL;
                            } else if (!strcasecmp(buf, "FALSE")) {
                                b = false;
                                return BOOL;
                            } else {
                                str = buf;
                                return SYM;
                            }
                        case STRING:
                            str = buf;
                            return STRING;
                    }
                    return curtype;
                }
                break;

            case EOF:
                return 0;

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
                if (curtype == 0)
                    curtype = NUM;
            case '"':
                if (curtype == 0) {
                    curtype = STRING;
                    break;
                } else if (curtype == STRING && ch == '"')
                    break;
            default:
                if (curtype == 0)
                    curtype = SYM;
                *ptr++ = ch;
                *ptr = '\0';
                break;
        }
    } while (!feof(f));

    return 0;
}

// Parse the textual nested list and create a 
// RList-object from it (a simple parser).
static void parse(FILE *f, RList& nl, int depth) {
    do {
        int token = getToken(f);
        switch (token) {
            case '(':
                parse(f, nl.nest(), depth+1);
                break;
            case NUM:
                nl.append(num);
                break;
            case STRING:
                nl.append(str);
                break;
            case BOOL:
                nl.append(b);
                break;
            case SYM:
                nl.appendSym(str);
                break;
            case ')':
                return;
            default:
                return;

        }
    } while (1);
}

/*
14 ~parseFile~

Parse the given file with the textual representation of a list
and return a corresponding RList object.

*/
RList RList::parseFile(std::string filename) {
    FILE *f;

    RList ret;

    f = fopen(filename.c_str(), "r");
    if (!f)
        return ret;
    if (getToken(f) != '(') {
        printf("Parse error! (Beginning)\n");
        return ret;
    } else {
        parse(f, ret, 0);
        if (getToken(f) != 0) {
            printf("Parse error! (End)\n");
            return ret;
        }
    }

    fclose(f);

    return ret;
}
