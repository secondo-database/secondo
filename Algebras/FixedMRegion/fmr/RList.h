/* 
----
 * This file is part of libfmr
 * 
 * File:   RList.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 6, 2016, 11:32 AM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class RList

[TOC]

1 Overview

Header file with the class definition for the class ~RList~

2 Includes and definitions

*/

#ifndef FMR_RLIST_H
#define FMR_RLIST_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef  __cplusplus
}
#endif
    
#include <string>
#include <vector>

namespace fmr {

// The various list element types
enum {
    NL_LIST = 1,
    NL_STRING,
    NL_SYM,
    NL_DOUBLE,
    NL_BOOL
};

/*
3 Definition of class RList

*/
class RList {
public:
    std::vector<RList> items;
    
    RList();
    void append(double nr);
    void append(std::string str);
    void appendSym(std::string str);
    void append(bool val);
    void append(RList l);
    RList point(double x, double y);
    void concat(RList l);
    double getNr () {
        if (type == NL_DOUBLE)
            return nr;
        else
            return 0;
    }
    bool getBool () {
        if (type == NL_BOOL)
            return boolean;
        else
            return false;
    }
    std::string getString () {
        if (type == NL_STRING)
            return str;
        else
            return "";
    }
    std::string getSym () {
        if (type == NL_SYM)
            return str;
        else
            return "";
    }
    int getType () {
        return type;
    }
    RList& nest();
    std::string ToString();
    RList SecondoObject(std::string name, std::string type);
    RList& operator[](int index) {
        return items[index];
    }
    operator double() {
        return getNr();
    }
    int size() {
        return items.size();
    }
    static RList parseFile (const char *filename);

protected:
    int type;
    std::string str;
    double nr;
    bool boolean;
    std::string ToString(int indent);
    
};

}

#endif  /* RLIST_H */