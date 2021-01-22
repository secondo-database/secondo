/* 
  This file is part of libpmregion
  
  File:   PMRegion.h
  Author: Florian Heinz <fh@sysv.de>
  
  1 PMRegion.h
    Interface class Definitions for PMregion and RList
 
*/

#ifndef PMREGION_H
#define PMREGION_H

namespace pmr {

class RList;



class MReal;
class MBool;
class ScalarField;

class PMRegion {
    public:
        PMRegion() {}
        ~PMRegion() {}
        PMRegion(Polyhedron p) {
            polyhedron = p;
        }
        PMRegion operator+(PMRegion& pmr);
        PMRegion operator-(PMRegion& pmr);
        PMRegion operator*(PMRegion& pmr);
        RList atinstant(Kernel::FT instant);
        RList atinstant2(Kernel::FT instant);
        MBool mpointinside(RList& mpoint);
        MBool intersects(PMRegion& pmr);
        MReal perimeter();
        MReal area();
        MReal area2();
        RList traversedarea();
        PMRegion createcdpoly();
        PMRegion createcdpoly(RList baseregion);
        PMRegion restrictcdpoly(RList baseregion);
        PMRegion createccdpoly();
        PMRegion createccdpoly(RList baseregion);
        PMRegion createicdpoly(Kernel::FT duration);
        PMRegion createicdpoly(Kernel::FT duration, RList baseregion);
        Kernel::FT coverduration (Kernel::FT x, Kernel::FT y);
        RList coveredlonger (Kernel::FT duration);
        RList coveredshorter (Kernel::FT duration);
        RList intervalcovered (Kernel::FT startduration);
        Kernel::FT avgcover ();
        Kernel::FT avgcover (RList baseregion);
        void openscad(std::string filename);
        ScalarField scalarfield();
        void zthicknessprepare();
        Kernel::FT zthickness(Point2d p2d);
        Plane calculate_plane(Polygon p);
        void translate (Kernel::FT x, Kernel::FT y, Kernel::FT z);
        std::pair<Kernel::FT, Kernel::FT> minmaxz();
        std::pair<Point3d, Point3d> boundingbox();
        void analyze();
        std::vector<Polygon_with_holes_2> projectxy();

        static PMRegion fromRList(RList& rl);
        RList toRList(bool raw);
        RList toRList();
        void toFile(std::string filename);

        static PMRegion fromOFF(std::string off);
        std::string toOFF();

        static PMRegion fromMRegion(RList mr);
        RList toMRegion();
        RList toMRegion2();
        RList toMRegion2(int raw);

        static PMRegion fromRegion(RList reg, Kernel::FT instant1,
                Kernel::FT instant2, Kernel::FT xoff);

        Polyhedron polyhedron;
        Polyhedron zthicknesstmp;
        Tree *zthicknesstree;
        std::map<Point2d, Kernel::FT> ztcache;
        Point_inside *inside_tester;
};

#ifndef RLIST_TYPES
#define RLIST_TYPES
enum {
    NL_LIST = 1,
    NL_STRING,
    NL_SYM,
    NL_DOUBLE,
    NL_FT,
    NL_BOOL
};
#endif

class RList {
    protected:
        int type;
        std::string str;
        double nr;
        Kernel::FT *ft;
        bool boolean;
        std::string ToString(int indent);

    public:
        std::vector<RList> items;

        RList();
        ~RList();
        void append(double nr);
        void append(Kernel::FT nr);
        void append(std::string str);
        void appendsym(std::string str);
        void append(bool val);
        void append(RList l);
        void prepend(RList l);
        RList* point(double x, double y);
        void concat(RList l);
    void toFile(std::string filename);
        double getNr () {
            assert(type == NL_DOUBLE || type == NL_FT);
        if (type == NL_DOUBLE)
            return nr;
        else
            return ::CGAL::to_double(*ft);
        }
        Kernel::FT getFt () {
            assert(type == NL_FT||type == NL_DOUBLE);
            if (type == NL_FT) {
                return *ft;
            } else {
                return Kernel::FT(nr);
            }
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

        RList obj(std::string name, std::string type);
        static RList parse(std::istream& f);
        static RList* parsep(std::istream& f);
        std::string ToString();
};

std::string timestr (Kernel::FT t);
class MReal {
    public:
        RList rl;

        MReal() {
            rl.appendsym("OBJECT");
            rl.appendsym("mreal");
            RList empty;
            rl.append(empty);
            rl.appendsym("mreal");
            rl.append(empty);
        }

        void append (Kernel::FT start, Kernel::FT end,
                Kernel::FT a, Kernel::FT b, Kernel::FT c) {
            RList ureal;
            RList interval;
            interval.append(timestr(start));
            interval.append(timestr(end));
            interval.append((bool)true);
            interval.append((bool)false);

            RList coeffs;
            coeffs.append(a);
            coeffs.append(b);
            coeffs.append(c);
            coeffs.append((bool)false);

            ureal.append(interval);
            ureal.append(coeffs);
            rl.items[4].append(ureal);
        }
};

class MBool {
    public:
        RList rl;

        MBool() {
            rl.appendsym("OBJECT");
            rl.appendsym("mbool");
            RList empty;
            rl.append(empty);
            rl.appendsym("mbool");
            rl.append(empty);
        }

        void append (Kernel::FT start, Kernel::FT end, bool value) {
            RList ubool;
            RList interval;
            interval.append(timestr(start));
            interval.append(timestr(end));
            interval.append((bool)true);
            interval.append((bool)false);

            ubool.append(interval);
            ubool.append(value);
            rl.items[4].append(ubool);
        }
};

class ScalarField {
    public:
        std::vector<Polygon> polygons;
        std::vector<std::vector<Kernel::FT> > coeffs;

        void add (Polygon polygon, Plane plane);
        Kernel::FT value(Point2d p);

        std::string ToString();

        static ScalarField fromRList(RList rl);
        RList toRList();
};

RList decompose_mreg (RList *mreg, int steps, std::vector<RList>& mregs);
std::vector<PMRegion> decompose_pmreg (PMRegion *pmreg, int steps,
        std::vector<RList>& mregs);
}

#endif /* PMREGION_H */
