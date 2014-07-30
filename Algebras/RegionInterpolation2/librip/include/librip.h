/* 
   1 regioninterpolate.h is the interface include file for the
     region interpolation library

*/

#ifndef REGIONINTERPOLATE_HXX
#define REGIONINTERPOLATE_HXX

#define VERSION "1.3"

#include <string>
#include <vector>
#include <cassert>

using namespace std;

enum {
    NL_LIST = 1,
    NL_STRING,
    NL_DOUBLE,
    NL_BOOL
};


class RList {
protected:
    int type;
    string str;
    double nr;
    bool boolean;
    string ToString(int indent);
    
public:
    vector<RList> items;
    
    RList();
    void append(double nr);
    void append(string str);
    void append(bool val);
    void append(RList l);
    RList* point(double x, double y);
    void concat(RList l);
    double getNr () {
        assert(type == NL_DOUBLE);
        return nr;
    }
    bool getBool () {
        assert(type == NL_BOOL);
        return boolean;
    }
    string getString () {
        assert(type == NL_STRING);
        return str;
    }
    int getType () {
        return type;
    }
    RList* nest();
    string ToString();
};

RList regioninterpolate(RList src, RList dst, string start, string end,
                        string args);

#endif /* REGIONINTERPOLATE_HXX */
