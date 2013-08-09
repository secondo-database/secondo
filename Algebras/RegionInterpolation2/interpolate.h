/* 
*/

#ifndef INTERPOLATE_HXX
#define	INTERPOLATE_HXX

#include "MovingRegionAlgebra.h"

class MSegs;

class Pt {
public:
    int x, y;
    int valid;
    double angle;

    Pt();
    Pt(int x, int y);
    bool operator<(const Pt& a) const;
    bool operator==(const Pt& a) const;
    Pt   operator-(const Pt& a) const;
    Pt   operator+(const Pt& a) const;
    Pt   operator/(int a) const;
    bool sortAngle(const Pt& a) const;
    void calcAngle(const Pt& pt);
    int distance (Pt p);
    string ToString();
};

class Seg {
public:
    int x1, y1, x2, y2;
    int valid;
    
    Seg();
    Seg(int x1, int y1, int x2, int y2);
    double angle() const;
    bool operator<(const Seg& a) const;
    bool operator==(const Seg& a) const;
    string ToString();
    void ChangeDir();
    
    static vector<Seg> sortSegs(vector<Seg> v);
};

class Reg {
private:
    int cur;

public:
    Pt hullPoint, peerPoint;
    vector<Seg> convexhull;
    vector<Seg> v;
    vector<Reg> holes;
    int used;
    
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
    string ToString();
    int distance (Reg r);
    
    static vector<Reg> getRegs(ListExpr le);
    static Pt GetMinXY(vector<Reg> regs);
    static Pt GetMaxXY(vector<Reg> regs);
};

class MSeg {
public:
    int sx1, sy1, sx2, sy2, fx1, fy1, fx2, fy2;
    
//    MSeg();
    MSeg(int sx1, int sy1, int sx2, int sy2,
         int fx1, int fy1, int fx2, int fy2);
    MSegmentData ToMSegmentData(int face, int cycle, int segno);
    string ToString() const;
    bool operator<(const MSeg& a) const;
    bool operator==(const MSeg& a) const;
    bool intersects(const MSeg& a) const;
};

class MSegs {
public:
    vector<MSeg> segs;

    MSegs();
    void AddMSeg(int sx1, int sy1, int sx2, int sy2,
                 int fx1, int fy1, int fx2, int fy2);
    void AddMSegs(vector<MSeg> v);
    vector<MSegmentData> ToMSegmentData(int face, int cycle);
    string ToString() const;
    vector<MSeg> GetMatchingMSegs (MSegs m);
    void MergeConcavity (MSegs c);
    bool intersects(const MSegs& a) const;
};

class MFace {
public:
    MSegs face;
    vector<MSegs> holes;
    
    MFace();
    MFace(MSegs face);
    void AddMsegs (MSegs msegs);
    URegion ToURegion(Interval<Instant> iv, int facenr);
    string ToString();
};

class MFaces {
public:
    vector<MFace> faces;
    
    MFaces();
    MFaces(MFace face);
    void AddFace (MFace face);
    MRegion ToMRegion(Interval<Instant> iv);
    string ToString();
};

class RotatingPlane {
public:
    MFace face;
    vector<Reg> scvs, dcvs;
    
    RotatingPlane(Reg *src, Reg *dst);
};


#endif	/* INTERPOLATE_HXX */

