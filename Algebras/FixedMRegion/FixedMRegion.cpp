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
    {  //printf("huhua\n");
      t=_t;
  xm=_xm;
  ym=_ym;
//  printf("huhua1\n");
  r=_r;
  // printf("huhub\n");
      m = Move(_x0, _y0, _alpha0, _vx, _vy, _valpha);
    //   printf("huhuc\n");
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
      sett(ti);
      Region * result = new Region(*r);
      HalfSegment hs;
      result->StartBulkLoad();
      for( int i = 0; i < result->Size(); i++ )
      {
         result->Get( i, hs );
         
         const Point lp=hs.GetLeftPoint();
         //printf("Pl%d=(%f,%f)\n",i,lp.GetX(),lp.GetY());
         double newx=l.getImgX(lp.GetX(), lp.GetY());
         double newy=l.getImgY(lp.GetX(), lp.GetY());
         Point newlp(true, newx, newy);
         
         const Point rp=hs.GetRightPoint();
         //printf("Pr%d=(%f,%f)\n",i,rp.GetX(),rp.GetY());
         newx=l.getImgX(rp.GetX(), rp.GetY());
         newy=l.getImgY(rp.GetX(), rp.GetY());
         Point newrp(true, newx, newy);
         //printf("Pnl%d=(%f,%f)\n",i,newlp.GetX(),newlp.GetY());
         //printf("Pnr%d=(%f,%f)\n",i,newrp.GetX(),newrp.GetY());
         //HalfSegment tmp= HalfSegment(false, newlp,newrp);
         hs.Set(hs.IsLeftDomPoint(), newlp, newrp);

         result->Put(i, hs);
      }
      result->EndBulkLoad();
      return result;
    }
    
    MBool FixedMRegion::FixedMRegion::inside(MPoint mp, double ta, double te, 
    double precision){
      return MBool();
      //TODO
    }
    
    bool FixedMRegion::inside(MPoint mp, double t){
      Region *rfix = atinstant(t);
      // void AtInstant( const Instant& t, Intime<Point>& result ) const;
//      Point *pointfix = NULL;
      //mp.AtInstant(t, pointfix);
      //FIXME
//       Point *p1 = new Point( true, 0.0, 0.0);
//       bool ret=rfix->Contains(pl);
      //TODO
//      return ret;
    return false;
    }
    
    MBool FixedMRegion::intersection(MPoint mp, double ta, double te, double 
    precision){
      return MBool();
      //TODO
    }
    
    bool FixedMRegion::intersection(MPoint mp, double t){
      Region *rfix = atinstant(t);
      // void AtInstant( const Instant& t, Intime<Point>& result ) const;
      Point *pointfix = NULL;
      //mp->AtInstant(t, pointfix);
      //FIXME
       Point *p1 = new Point( true, 0.0, 0.0);
       //  void Intersection(const Point& p, Points& result,
       //             const Geoid* geoid=0) const;
         
       
       //TODO
      return false;
    }
    
    Region * FixedMRegion::traversed(double ta, double te, double precision){
      Region * res=NULL;
      for(double i=0;i<=(te-ta);i=i+precision){
        Region * tmp = atinstant(ta+i);
        if(res==NULL){
          printf("NULL-Schleife\n");
          res=tmp;
        }else{
          printf("Union-Schleife\n");
          res->Union(* tmp,* res);
        }
        printf("res is %sdefined.\n", (res->IsDefined())?"":"NOT ");
      }
      return res;
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