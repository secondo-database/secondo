/* 
*/

#ifndef INTERPOLATE_HXX
#define	INTERPOLATE_HXX

#include "MovingRegionAlgebra.h"

class Pt {
private:

public:
    int x, y;
    double angle;

    Pt();
    Pt(int x, int y);
    bool operator<(const Pt& a) const;
    bool sortAngle(const Pt& a) const;
    void calcAngle(const Pt& pt);
    bool operator==(const Pt& a) const;
    string ToString();
};


class Seg {
private:

public:
    int x1, y1, x2, y2;
    Seg();
    Seg(int x1, int y1, int x2, int y2);

    double angle() const;
    bool operator<(const Seg& a) const;
    bool operator==(const Seg& a) const;
    string ToString();
    void ChangeDir();
};

vector<Seg> sortSegs(vector<Seg> v);

class MSeg {
public:
    int sx1, sy1, sx2, sy2, fx1, fy1, fx2, fy2;
    
    MSeg();
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
private:
    MSegs face;
    vector<MSegs> holes;
    vector<MSegs> cvs;
    
public:
    MFace();
    MFace(MSegs face);
    void AddHole (MSegs msegs);
    URegion ToURegion(Interval<Instant> iv);
};

class MFaces {
private:
    vector<MFace> faces;
    
public:
    MFaces();
    MFaces(MFace face);
    void AddFace (MFace face);
    MRegion ToMRegion(Interval<Instant> iv);
};

class Reg {
private:
    int cur;
    Pt hullPoint;

public:
    vector<Seg> convexhull;
    vector<Seg> v;
    vector<Reg> cvs;
    Reg *parent;
    int parentseg;
    Reg();
    Reg(ListExpr le);
    Reg(vector<Seg> v);
    Reg(Reg *parent, int parentseg);
    void AddSeg(Seg& a);
    void Print();
    void Close();
    void ConvexHull();
    void Translate(int offx, int offy);
    Seg Next();
    Seg Prev();
    Seg Cur();
    int End();
    void Sort();
    vector<Pt> getPoints();
    vector<Reg> Concavities();
    Region MakeRegion();
    Region MakeRegion(int offx, int offy);
    Pt GetMinXY();
    MSegs collapse();
    
    static vector<Reg> getRegs(ListExpr le);
};

#endif	/* INTERPOLATE_HXX */

