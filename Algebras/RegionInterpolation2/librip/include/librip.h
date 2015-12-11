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


enum {
    NL_LIST = 1,
    NL_STRING,
    NL_DOUBLE,
    NL_BOOL
};


class RList {
protected:
    int type;
    std::string str;
    double nr;
    bool boolean;
    std::string ToString(int indent);
    
public:
    std::vector<RList> items;
    
    RList();
    void append(double nr);
    void append(std::string str);
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
    std::string getString () {
        assert(type == NL_STRING);
        return str;
    }
    int getType () {
        return type;
    }
    RList* nest();
    std::string ToString();
};

RList regioninterpolate(RList src, RList dst, std::string start, 
                        std::string end,
                        std::string args);

#endif /* REGIONINTERPOLATE_HXX */
