/* 
*/

#ifndef INTERPOLATE_HXX
#define	INTERPOLATE_HXX

#define URMODE2

#include "MovingRegionAlgebra.h"

class MSegs;
class MFace;

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

    static vector<Seg> sortSegs(vector<Seg> v);
};

class Reg {
private:
    int cur;

public:
    Reg *parent;

    Pt hullPoint, peerPoint;
    Seg hullSeg;
    vector<Seg> convexhull;
    vector<Seg> v;
    vector<Reg> holes;
    bool ishole;
    int used;
    int isdst;
    pair<Pt, Pt> bbox;

    Reg();
    Reg(ListExpr le);
    Reg(vector<Seg> v);
    void AddSeg(Seg& a);
    void Print();
    void Close();
    void ConvexHull();
    void Translate(int offx, int offy);
    void Begin();
    Seg Next();
    Seg Prev();
    Seg Cur();
    int End();
    void Sort();
    vector<Pt> getPoints();
    vector<Reg> Concavities();
    vector<Reg> Concavities2(Reg *r2);
    Region MakeRegion();
    Region MakeRegion(double offx, double offy, double scalex, double scaley);
    pair<Pt, Pt> GetBoundingBox();
    Pt GetMiddle();
    Pt GetCentroid();
    MSegs collapse(bool close);
    MSegs collapse(bool close, Pt dst);
    MFace collapseWithHoles(bool close);
    Pt collapsePoint();
    MSegs GetMSegs();
    string ToString() const;
    double distance(Reg r);
    Reg Merge(Reg r);
    Reg ClipEar();
    vector<MSegs> Evaporate(bool close);
    void AddHole (vector<Seg> segs);

    static vector<Reg> getRegs(ListExpr le);
    static pair<Pt, Pt> GetBoundingBox(vector<Reg> regs);
    static pair<Pt, Pt> GetBoundingBox(set<Reg*> regs);
};

class MSeg {
public:
    Pt is, ie, fs, fe;

    MSeg();
    MSeg(Pt is, Pt ie, Pt fs, Pt fe);
    MSegmentData ToMSegmentData(int face, int cycle, int segno);
    string ToString() const;
    bool operator<(const MSeg& a) const;
    bool operator==(const MSeg& a) const;
    bool intersects(const MSeg& a) const;
    void ChangeDirection();
    MSeg divide(double start, double end);
};

class MSegs {
public:
    int ignore, iscollapsed, id;
    vector<MSeg> segs;
    Reg sreg, dreg;
    pair<Pt,Pt> bbox;

    MSegs();
    void AddMSeg(MSeg m);
    vector<MSegmentData> ToMSegmentData(int face, int cycle, int segno);
    string ToString() const;
    vector<MSeg> GetMatchingMSegs(MSegs m);
    void MergeConcavity(MSegs c);
    bool intersects(const MSegs& a) const;
    void updateBBox(MSeg& seg);
    pair<Pt, Pt> calculateBBox();
    
    pair<MSegs, MSegs> kill();
    Reg GetSReg();
    Reg GetDReg();

    int getLowerLeft();
    int findNext(int index);
    MSegs divide(double start, double end);
};

class MFace {
public:
    bool needStartRegion, needEndRegion;
    MSegs face;
    vector<MSegs> holes, cvs;
    vector<Reg> sevap, devap;
    pair<Pt, Pt> bbox;

    MFace();
    MFace(MSegs face);
    void AddConcavity(MSegs c);
    void MergeConcavities();
    void AddMsegs(MSegs msegs);
    URegion ToURegion(Interval<Instant> iv, int facenr);
    ListExpr ToListExpr();
    string ToString();
    MFace divide(double start, double end);
    Reg CreateBorderRegion(bool src);
};

class MFaces {
public:
    vector<MFace> faces;
    vector<Reg> *sregs, *dregs;
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
    vector<Reg> CreateBorderRegions(bool src);
    MFaces CreateBorderMFaces(bool src);
    
    static ListExpr fallback(vector<Reg> *s, vector<Reg> *d, 
                             Interval<Instant> iv);
};

class RotatingPlane {
public:
    MFace face;
    vector<Reg> scvs, dcvs;

    RotatingPlane(Reg *src, Reg *dst, int depth);
};

MFaces interpolate(vector<Reg> *sregs, vector<Reg> *dregs, int depth,
        bool evap, string args);

#endif	/* INTERPOLATE_HXX */

