/*
   1 RList is used to create a RList representation
   of regions and moving regions.
 
*/

#include <string>
#include "interpolate.h"

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
        ret << nr;
    } else if (type == NL_BOOL) {
        ret << (boolean ? "TRUE" : "FALSE");
    } else if (type == NL_STRING) {
        ret << "\"" << str << "\"";
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
