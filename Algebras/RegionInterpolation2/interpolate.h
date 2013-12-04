/* 
*/

#ifndef INTERPOLATE_HXX
#define	INTERPOLATE_HXX

#include "MovingRegionAlgebra.h"

class MSegs;

class Pt {
public:
    double x, y;
    int valid;
    double angle;

    Pt();
    Pt(double x, double y);
    bool operator<(const Pt& a) const;
    bool operator==(const Pt& a) const;
    Pt   operator-(const Pt& a) const;
    Pt   operator+(const Pt& a) const;
    Pt   operator/(double a) const;
    bool sortAngle(const Pt& a) const;
    void calcAngle(const Pt& pt);
    double distance (Pt p);
    string ToString() const;
};

class Seg {
public:
    Pt s, e;
    int valid;
    
    Seg();
    Seg(Pt s, Pt e);
    double angle() const;
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
    Pt hullPoint, peerPoint;
    Seg hullSeg;
    vector<Seg> convexhull;
    vector<Seg> v;
    vector<Reg> holes;
    int used;
    int isdst;
    
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
    Region MakeRegion(int offx, int offy);
    Pt GetMinXY();
    Pt GetMaxXY();
    Pt GetMiddle();
    MSegs collapse(bool close);
    string ToString() const;
    double distance (Reg r);
    
    static vector<Reg> getRegs(ListExpr le);
    static Pt GetMinXY(vector<Reg> regs);
    static Pt GetMaxXY(vector<Reg> regs);
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
    MSeg divide (double start, double end);
};

class MSegs {
public:
    int ignore;
    vector<MSeg> segs;
    Reg sreg, dreg;

    MSegs();
    void AddMSeg(MSeg m);
    void AddMSegs(vector<MSeg> v);
    vector<MSegmentData> ToMSegmentData(int face, int cycle);
    string ToString() const;
    vector<MSeg> GetMatchingMSegs (MSegs m);
    void MergeConcavity (MSegs c);
    bool intersects(const MSegs& a) const;
    pair<MSegs, MSegs> kill();
    Reg GetSReg();
    Reg GetDReg();
    
    MSegs divide (double start, double end);
};

class MFace {
public:
    MSegs face;
    vector<MSegs> holes, cvs;
    
    MFace();
    MFace(MSegs face);
    void AddConcavity (MSegs c);
    void MergeConcavities ();
    void AddMsegs (MSegs msegs);
    URegion ToURegion(Interval<Instant> iv, int facenr);
    string ToString();
    MFace divide (double start, double end);
};

class MFaces {
public:
    vector<MFace> faces;
    
    MFaces();
    MFaces(MFace face);
    void AddFace (MFace face);
    MRegion ToMRegion(Interval<Instant> iv);
    string ToString();
    MFaces divide (double start, double end);
};

class RotatingPlane {
public:
    MFace face;
    vector<Reg> scvs, dcvs;
    
    RotatingPlane(Reg *src, Reg *dst);
};


static double eps = 0.00001;

static bool nearlyEqual(double a, double b) {
    return abs(a-b) <= eps;
}

#endif	/* INTERPOLATE_HXX */

