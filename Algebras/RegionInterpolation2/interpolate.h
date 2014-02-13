/* 
*/

#ifndef INTERPOLATE_HXX
#define INTERPOLATE_HXX

#define LUA5_1
//#define LUA5_2

#if defined(LUA5_1) || defined(LUA5_2)
#define USE_LUA
#define LUA_ADD_FUNCTION(FNAME) lua_pushcfunction(L, lua_ ## FNAME); \
                                lua_setglobal(L, #FNAME)
#endif

#define URMODE2

#include "MovingRegionAlgebra.h"

class MSegs;
class MFace;
class MFaces;

class Pt {
public:
    double x, y;
    int valid;
    long double angle, dist;

    Pt();
    Pt(double x, double y);
    bool operator<(const Pt& a) const;
    bool operator==(const Pt& a) const;
    Pt operator-(const Pt& a) const;
    Pt operator+(const Pt& a) const;
    Pt operator/(double a) const;
    Pt operator*(const Pt& a) const;
    bool sortAngle(const Pt& a) const;
    void calcAngle(const Pt& pt);
    double distance(Pt p);
    string ToString() const;
    static double sign(const Pt& a, const Pt& b, const Pt& c);
    static bool insideTriangle(const Pt& a, const Pt& b, const Pt& c,
            const Pt& x);
};

class Seg {
public:
    Pt s, e;
    int valid;

    Seg();
    Seg(Pt s, Pt e);
    long double angle() const;
    bool operator<(const Seg& a) const;
    bool operator==(const Seg& a) const;
    string ToString() const;
    void ChangeDir();
    bool intersects(const Seg& a) const;

};

class Face {
private:
    int cur;

public:
    Face *parent;

    Pt peerPoint;
    Seg hullSeg;
    vector<Seg> convexhull;
    vector<Seg> v;
    vector<Face> holes;
    bool ishole;
    int used;
    int isdst;
    pair<Pt, Pt> bbox;

    Face();
    Face(ListExpr le);
    Face(vector<Seg> v);
    void AddSeg(Seg a);
    void Print();
    void Close();
    Face ConvexHull();
    void Translate(int offx, int offy);
    Seg Begin();
    Seg Next();
    Seg Prev();
    Seg Cur();
    bool End();
    void Sort();
    vector<Pt> getPoints();
    vector<Face> Concavities();
    vector<Face> Concavities2(Face *r2);
    Region MakeRegion(bool withholes);
    Region MakeRegion(double offx, double offy, double scalex, double scaley,
                      bool withholes);
    pair<Pt, Pt> GetBoundingBox();
    Pt GetMiddle();
    Pt GetCentroid();
    MSegs collapse(bool close);
    MSegs collapse(bool close, Pt dst);
    MFace collapseWithHoles(bool close);
    Pt collapsePoint();
    MSegs GetMSegs(bool triangles);
    string ToString() const;
    double distance(Face r);
    double GetArea();
    Face ClipEar();
    vector<MSegs> Evaporate(bool close);
    void AddHole (Face hole);
    bool Check();
    void IntegrateHoles();

    static vector<Face> getFaces(ListExpr le);
    static pair<Pt, Pt> GetBoundingBox(vector<Face> faces);
    static pair<Pt, Pt> GetBoundingBox(set<Face*> faces);
    static vector<Seg> sortSegs(vector<Seg> v);
    static MFaces CreateMFaces(vector<Face> *faces);
};

class MSeg {
public:
    Pt is, ie, fs, fe;
//    bool valid;
    vector<Pt> ip, fp;

    MSeg();
    MSeg(Pt is, Pt ie, Pt fs, Pt fe);
    MSegmentData ToMSegmentData(int face, int cycle, int segno);
    string ToString() const;
    bool operator<(const MSeg& a) const;
    bool operator==(const MSeg& a) const;
    bool intersects(const MSeg& a, bool checkSegs) const;
    bool Merge(const MSeg& a);
    bool Split(MSeg& n, MSeg& m1, MSeg& m2);
    void ChangeDirection();
    MSeg divide(double start, double end);
};

class MSegs {
public:
    int iscollapsed, id;
    vector<MSeg> msegs;
    Face sreg, dreg;
    pair<Pt,Pt> bbox;

    MSegs();
    Face CreateBorderFace(bool initial);
    void AddMSeg(MSeg m);
    vector<MSegmentData> ToMSegmentData(int face, int cycle, int segno);
    string ToString() const;
    vector<MSeg> GetMatchingMSegs(MSegs m);
    bool MergeConcavity(MSegs c);
    bool intersects(const MSegs& a, bool matchIdent, bool matchSegs) const;
    void updateBBox(MSeg& seg);
    pair<Pt, Pt> calculateBBox();
    
    pair<MSegs, MSegs> kill();
    Face GetSReg();
    Face GetDReg();

    int getLowerLeft();
    int findNext(int index);
    MSegs divide(double start, double end);
    
};

class MFace {
public:
    bool needStartRegion, needEndRegion;
    MSegs face;
    vector<MSegs> holes, cvs;
    vector<Face> sevap, devap;
    pair<Pt, Pt> bbox;

    MFace();
    MFace(MSegs face);
    bool Check();
    bool SortCycle();
    void AddConcavity(MSegs c);
    void MergeConcavities();
    URegion ToURegion(Interval<Instant> iv, int facenr);
    ListExpr ToListExpr();
    void PrintMRegionListExpr();
    string ToString();
    MFace divide(double start, double end);
    Face CreateBorderFace(bool src);
};

class MFaces {
public:
    vector<MFace> faces;
    vector<Face> *sregs, *dregs;
    bool needSEvap, needDEvap;

    MFaces();
    MFaces(MFace face);
    void AddFace(MFace face);
    MRegion ToMRegion(Interval<Instant> iv);
    URegion ToURegion(Interval<Instant> iv, double start, double end);
    ListExpr ToListExpr(Interval<Instant> iv, double start, double end);
    ListExpr ToMListExpr(Interval<Instant> iv);
    string ToString();
    MFaces divide(double start, double end);
    vector<Face> CreateBorderFaces(bool src);
    MFaces CreateBorderMFaces(bool src);
    
    static ListExpr fallback(vector<Face> *s, vector<Face> *d, 
                             Interval<Instant> iv);
};

class RotatingPlane {
public:
    MFace mface;
    vector<Face> scvs, dcvs;

    RotatingPlane(Face *src, Face *dst, int depth, bool evap);
};

MFaces interpolate(vector<Face> *sregs, vector<Face> *dregs, int depth,
        bool evap, string args);

Word InMRegion(const ListExpr typeInfo,
        const ListExpr instance,
        const int errorPos,
        ListExpr& errorInfo,
        bool& correct);

vector<pair<Face *, Face *> > matchFacesSimple(vector<Face> *src,
        vector<Face> *dst, int depth, string args);
vector<pair<Face *, Face *> > matchFacesNull(vector<Face> *src,
        vector<Face> *dst, int depth, string args);
vector<pair<Face *, Face *> > matchFacesLowerLeft(vector<Face> *src,
        vector<Face> *dst, int depth, string args);
vector<pair<Face *, Face *> > matchFacesDistance(vector<Face> *src,
        vector<Face> *dst, int depth, string args);
vector<pair<Face *, Face *> > matchFacesLua(vector<Face> *src,
        vector<Face> *dst, int depth, string args);


#endif /* INTERPOLATE_HXX */
