/*
This class is a FixedMRegion.

*/
using namespace std;
#include "FixedMRegion.h"

/*
This is the default constructor. Do not use.

*/
    FixedMRegion::FixedMRegion(): r(NULL) {}
/*
This is the copy constructor.

*/
    FixedMRegion::FixedMRegion(const FixedMRegion &f): t(f.t), m(f.m),
    r(f.r), l(f.l) {}
/*
This is the constructor. It gets necessary information. 

*/  
FixedMRegion::FixedMRegion(const double _t,
const Move & _m, Region * _r, 
const LATransform & _l): t(_t), m(_m),  r(_r), l(_l) {
  xm=l.getXM();
  ym=l.getYM();
  calculateInternalVars();
}

    FixedMRegion::FixedMRegion(double _t, double _xm, double _ym, 
 Region * _r, double _x0, double _y0, double _alpha0, 
double _vx, double _vy, double _valpha)
    //: 
//t(_t), xm(_xm), ym(_ym), r(_r){
    {  printf("huhua\n");
      t=_t;
  xm=_xm;
  ym=_ym;
  printf("huhua1\n");
  r=_r;
   printf("huhub\n");
      m = Move(_x0, _y0, _alpha0, _vx, _vy, _valpha);
       printf("huhuc\n");
      calculateInternalVars();
}

/*
This is the standard destructor.

*/
    FixedMRegion::~FixedMRegion() {
        if (r!=NULL)
            delete r;
    }
    
    Region FixedMRegion:: interpolate(Region* spots){
      Point *p1 = new Point( true, 0.0, 0.0);
      Point *p2 = new Point( true, 1.0, 0.0);
      Point *p3 = new Point( true, 0.0, 1.0);
      Region result= Region(*p1, *p2, *p3);
      return result;
      //TODO
    }
    
    Region * FixedMRegion::atinstant(double ti){
  //Region *result= new Region(this);
      sett(ti);
  Region * result = new Region(*r);
  HalfSegment hs;
  result->StartBulkLoad();
  for( int i = 0; i < result->Size(); i++ )
  {
    result->Get( i, hs );
    const Point lp=hs.GetLeftPoint();
    printf("Pl%d=(%f,%f)\n",i,lp.GetX(),lp.GetY());
     const Point newlp=Point(l.getImgX(lp.GetX(), lp.GetY()), 
  l.getImgY(lp.GetX(), lp.GetY()));
    const Point rp=hs.GetRightPoint();
    printf("Pr%d=(%f,%f)\n",i,rp.GetX(),rp.GetY());
    const Point newrp=Point(l.getImgX(rp.GetX(), rp.GetY()), 
 l.getImgY(rp.GetX(), rp.GetY()));
    printf("Pnl%d=(%f,%f)\n",i,newlp.GetX(),newlp.GetY());
    printf("Pnr%d=(%f,%f)\n",i,newrp.GetX(),newrp.GetY());
    
    HalfSegment tmp= HalfSegment(false, newlp,newrp);
    result->Put(i, tmp);
    //delete lp;
    //delete rp;
    //delete newlp;
    //delete newrp;
    //delete tmp;
  }
  result->EndBulkLoad();
  return result;
    }
    
    MBool FixedMRegion::inside(MPoint mp, double ta, double te, double
    precision){
      return MBool();
      //TODO
    }
    
    MBool FixedMRegion::intersection(MPoint mp, double ta, double te, double 
    precision){
      return MBool();
      //TODO
    }
    
    Region FixedMRegion::traversed(double ta, double te, double precision){
      Point *p1 = new Point( true, 0.0, 0.0);
      Point *p2 = new Point( true, 1.0, 0.0);
      Point *p3 = new Point( true, 0.0, 1.0);
      Region result= Region(*p1, *p2, *p3);
      return result;
      //TODO
    }
    
    const Region * FixedMRegion::getRegion() const {
      return r;
    }
    void FixedMRegion::setRegion(Region * _r){
      if (r!=NULL)
        delete r;
      r=_r;
    }
   const double FixedMRegion::gett() const {
      return t;
    }
    void FixedMRegion::sett(double _t){
      t=_t;
      calculateInternalVars();
    }
 //TODO   
    void FixedMRegion::calculateInternalVars(){
      double* coord=m.attime(t);
      l = LATransform(coord[0], coord[1], xm, ym, coord[2]);
      delete coord;
    }
    const LATransform& FixedMRegion::getLATransform(){
      return l;
    }
    void FixedMRegion::setLATransform(const LATransform &_l){
      l=_l;
    }
    const Move& FixedMRegion::getMove(){
      return m;
    }
    void FixedMRegion::setMove(const Move &_m){
      m=_m;
    }
 
    ;