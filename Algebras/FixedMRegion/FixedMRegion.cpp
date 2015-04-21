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
FixedMRegion::FixedMRegion(Region * _region, const Move & _move, 
  const Point &rot_center, double _starttime){
  r=_region;
  m=_move;
  t=_starttime;
  xm=rot_center.GetX();
  ym=rot_center.GetY();
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


FixedMRegion::FixedMRegion(Region * _region, const Point &_start, 
   double alpha_start, const Point &_speed, double alpha_speed, 
   const Point &rot_center, double _starttime){
  r=_region;
  m = Move(_start.GetX(), _start.GetY(), alpha_start, _speed.GetX(),
    _speed.GetY(), alpha_speed);
  t=_starttime;
  xm=rot_center.GetX();
  ym=rot_center.GetY();
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
      //Region *rfix = atinstant(t);
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
      //Region *rfix = atinstant(t);
      // void AtInstant( const Instant& t, Intime<Point>& result ) const;
      //Point *pointfix = NULL;
      //mp->AtInstant(t, pointfix);
      //FIXME
       //Point *p1 = new Point( true, 0.0, 0.0);
       //  void Intersection(const Point& p, Points& result,
       //             const Geoid* geoid=0) const;
         
       
       //TODO
      return false;
    }
    
    Region * FixedMRegion::traversed2(double ta, double te, double precision){
      Region * res=NULL;
      for(double i=0;i<=(te-ta);i=i+precision){
        Region * tmp = atinstant(ta+i);
        if(res==NULL){
          res=tmp;
        }else{
           Region *tmp2= new Region(*res);
           tmp2->Clear();
           //res->Union(*tmp, *tmp2);
           RobustPlaneSweep::robustUnion(*res,*tmp,*tmp2);
           delete tmp;
           delete res;
           res=tmp2;
        }
      }
      return res;
    }


 Point FixedMRegion::getIntersectionPoint(const Point & p1, const Point & p2, 
  const Point & p3, const Point & p4){
  double x1=p1.GetX();
  double y1=p1.GetY();
  double x2=p2.GetX();
  double y2=p2.GetY();  
  double x3=p3.GetX();
  double y3=p3.GetY();  
  double x4=p4.GetX();
  double y4=p4.GetY();  
  
  double a, b, c, d, e, f, D, Dx;
  double t;

  a=x1-x3;
  b=x1-x2;
  c=x4-x3;

  d=y1-y3;
  e=y1-y2;
  f=y4-y3;

  D=b*f-c*e;

  //if (D==0) {
    //Fallunterscheidung nicht notwendig, weil nur in Spezialfällen aufgerufen
  //}

  Dx=a*f-c*d;

  t=Dx/D;
  
  double x = x1+(x2-x1)*t;
  double y = y1+(y2-y1)*t;
  Point p_res = Point(true, x, y);
  return p_res;
}
    
int FixedMRegion::getTraversedCase(const Point  & p1, const Point & p2,
  const Point &p3, const Point & p4){
  double x1=p1.GetX();
  double y1=p1.GetY();
  double x2=p2.GetX();
  double y2=p2.GetY();  
  double x3=p3.GetX();
  double y3=p3.GetY();  
  double x4=p4.GetX();
  double y4=p4.GetY();  
  
  double a, b, c, d, e, f, D, Dx, Dy;
  double t, s;

  a=x1-x3;
  b=x1-x2;
  c=x4-x3;

  d=y1-y3;
  e=y1-y2;
  f=y4-y3;

  D=b*f-c*e;

  if (D==0) {
    //Fallunterscheidung Geraden parallel oder aufeinander:
    double t=(x3-x1)/(x2-x1); //Problem wenn x1=x2
    if (y3==y1+(y2-y1)*t) {
      //Geraden liegen aufeinander
      return -2;
    } else {
      //Geraden liegen parallel
      return -1;
    }
  }

  Dx=a*f-c*d;
  Dy=b*d-a*e;

  t=Dx/D;
  s=Dy/D;
//Strecke1: (x1, y1), (x2, y2)
//Strecke2: (x3, y3), (x4, y4)
  int cc=0; // Schnitt außerhalb
  //Fallunterscheidung nach (x1, y1), (x2, y2)
  if (t==0) 
    cc=1; //Schnitt auf (x1, y1)
  if (t==1)
    cc=2; //Schnitt auf (x2, y2)
  if ((t>0) && (t<1)) 
    cc=3; //Schnitt zwischen (x1, y1) und (x2, y2)

  //Fallunterscheidung nach (x3, y3), (x4, y4)
  if (s==0) 
    cc+=4; //Schnitt auf (x3, y3)
  if (s==1)
    cc+=8; // Schnitt auf (x4, y4)
  if ((s>0) && (s<1))
    cc+=12; //Schnitt zwischen (x3, y3) und (x4, y4)

  printf("Schnittpunkt: %3.2f, %3.2f\n", x1+(x2-x1)*t, y1+(y2-y1)*t);

  return cc;
}

vector<vector<Point> > FixedMRegion::traversedGetVectorVector(
  vector<Point> v){
    vector<vector<Point> >  vv; //Einen Vektor von Vektoren erzeugen
    vv.push_back(v); //Die einzelnen Polygonzüge hinzufügen (hier nur einer)
    return vv;  
}

vector<vector<Point> > FixedMRegion::traversedCalculateQuadrangle(
  const Point & p1, const Point & p2, const Point & p3, const Point & p4){
    vector<Point> v; //Vektor für den Polygonzug
    v.push_back(p1); 
    //Die einzelnen Punkte hinzufügen
    v.push_back(p2);
    v.push_back(p3);
    v.push_back(p4);
    v.push_back(p1); //Den ersten zum Schluss nochmal
    return traversedGetVectorVector(v);
}

vector<Point> FixedMRegion::traversedCalcTriangle(const Point & p1,
const Point & p2, const Point & p3){
    vector<Point> v; //Vektor für den Polygonzug
    v.push_back(p1); 
    //Die einzelnen Punkte hinzufügen
    v.push_back(p2);
    v.push_back(p3);
    v.push_back(p1); //Den ersten zum Schluss nochmal
    return v;
}

vector<vector<Point> > FixedMRegion::traversedCalculateTriangle(
  const Point & p1, const Point & p2, const Point & p3){
    vector<Point> v; //Vektor für den Polygonzug
    v=traversedCalcTriangle(p1, p2, p3);
    return traversedGetVectorVector(v);
}

vector< vector<Point> > FixedMRegion::traversedCalculateTwoTriangles(
  Point p1, Point p2, Point p3, Point p4){
  Point pi = getIntersectionPoint(p1, p2, p3, p4);
  vector<Point> t1; //Vektor für den Polygonzug
  vector<Point> t2;
  t1=traversedCalcTriangle(pi, p1, p3);
  t2=traversedCalcTriangle(pi, p2, p4);
  vector<vector<Point> > vv; //Einen Vektor von Vektoren erzeugen
  vv.push_back(t1); //Die einzelnen Polygonzüge hinzufügen (hier nur einer)
  vv.push_back(t2);
  return vv; 
}


vector<vector<Point> > FixedMRegion::getTraversedArea(
  const HalfSegment & hsold, const HalfSegment & hsnew){
  vector<vector<Point> > res;
  Point p1=hsold.GetDomPoint();
  Point p2=hsold.GetSecPoint();
  Point p3=hsnew.GetDomPoint();
  Point p4=hsnew.GetSecPoint();
  int casetype = getTraversedCase(p1, p2, p3, p4);
  switch (casetype) {
  case -2://Strecken sind parallel
    //1
  case 0://Schnittpunkt liegt außerhalb beider Strecken
    //2
  case 1://Schnittpunkt liegt auf P1 und außerhalb Strecke2
    //3
  case 2://Schnittpunkt liegt auf P2 und außerhalb Strecke2
    //8
  case 3://Schnittpunkt liegt innerhalb Strecke1 und außerhalb Strecke2
    //16
  case 4://Schnittpunkt liegt außerhalb Strecke1 und auf P3
    //12
  case 8://Schnittpunkt liegt außerhalb Strecke1 und auf P4
    //14
  case 11://Schnittpunkt liegt innerhalb Strecke1 und auf P4
    //17
  case 12://Schnittpunkt liegt außerhalb Strecke1 und innerhalb Strecke2
    //13
  case 13://Schnittpunkt liegt auf P1 und innerhalb Strecke2
    //5
  case 14://Schnittpunkt liegt auf P2 und innerhalb Strecke2
    //11
    res = traversedCalculateQuadrangle(p1, p2, p3, p4); 
    break;
  case 7://Schnittpunkt liegt innerhalb Strecke1 und auf P3
    //15
  case 15://Schnittpunkt liegt innerhalb Strecke1 und innerhalb Strecke2
    //6
    res = traversedCalculateTwoTriangles(p1, p2, p3, p4);
    break;
  case -1://Strecken liegen auf der selben Geraden
    //Entartung
    break;
  case 5://Schnittpunkt liegt auf P1 und auf P3
    //4
    res = traversedCalculateTriangle(p1, p2, p4);
    break;
  case 6://Schnittpunkt liegt auf P2 und auf P3
    //9
    res = traversedCalculateTriangle(p1, p2, p4);
    break;
  case 9://Schnittpunkt liegt auf P1 und auf P4
    //7
    res = traversedCalculateTriangle(p1, p2, p3);
    break;
  case 10://Schnittpunkt liegt auf P2 und auf P4
    //10
    res = traversedCalculateTriangle(p1, p2, p3);
    break;
  default: 
    assert(false);
    break;
  }
  return res;
}

void FixedMRegion::traversedCreateCyclesNotHoles(
  vector< vector<Point> > & v_list){
  for (size_t i=0; i<v_list.size(); i++) {
    if(!getDir(v_list[i])){
      reverseCycle(v_list[i]);
    }
  }
}


Region * FixedMRegion::getDiffRegion(const Region *resultold, 
  const Region * resultnew){
  Region * diffregion = new Region(0);   
  Region *tmp2 = NULL;
  Region *tmp_region = NULL;
  HalfSegment hsold;
  HalfSegment hsnew;
  for( int i = 0; i < resultold->Size(); i++ )
  {
    resultold->Get( i, hsold );
    resultnew->Get( i, hsnew );
    vector<vector<Point> > tmp_polygons=getTraversedArea(hsold, hsnew);
    
    //routine zum orientierung prüfen und korrigieren
    
    tmp_region=buildRegion(tmp_polygons);  //Region erstellen
    
    tmp2= new Region(0);
    //diffregion->Union(*tmp_region, *tmp2);
    RobustPlaneSweep::robustUnion(*diffregion,*tmp_region,*tmp2);    
    delete diffregion;
    delete tmp_region;
    diffregion=tmp2;
  }
  delete tmp2;
  return diffregion;
}



    
Region * FixedMRegion::traversed(double ta, double te, double precision){
   Region * res=atinstant(ta);
   Region * tiold=atinstant(ta);
   for(double i=0;i<=(te-ta);i=i+precision)
   {
     Region * tinew = atinstant(ta+i);
     Region * tmp = getDiffRegion(tiold, tinew);
     Region *tmp2= new Region(*res);
     tmp2->Clear();
     //res->Union(*tmp, *tmp2);
     RobustPlaneSweep::robustUnion(*res,*tmp,*tmp2);
     delete tmp;
     delete res;
     res=tmp2;
     delete tiold;
     tiold=tinew;
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