/* 
   1 regioninterpolate.h is the interface include file for the
     region interpolation library

*/

#ifndef REGIONINTERPOLATE_HXX
#define REGIONINTERPOLATE_HXX

#define VERSION "1.3"

#include <istream>
#include <string>
#include <vector>
#include <cassert>

enum {
    NL_LIST = 1,
    NL_STRING,
    NL_SYM,
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
    void appendsym(std::string str);
    void append(bool val);
    void append(RList l);
    void prepend(RList l);
    RList* point(double x, double y);
    void concat(RList l);
    RList obj(std::string name, std::string type);
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
    std::string getSym () {
        assert(type == NL_SYM);
        return str;
    }
    int getType () {
        return type;
    }
    RList* nest();
    bool empty () { return items.empty(); }

    static RList parse(std::istream& f);
    std::string ToString();
};

RList regioninterpolate(RList src, RList dst,
        std::string start, std::string end, std::string args);

void devtest (RList reg);

#endif /* REGIONINTERPOLATE_HXX */
