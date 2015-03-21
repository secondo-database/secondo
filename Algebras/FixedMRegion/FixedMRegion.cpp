/*
This class is a FixedMRegion.

*/
using namespace std;
#include "FixedMRegion.h"; 

/*
This is the constructor. It gets necessary information. 

*/  
    FixedMRegion::FixedMRegion(const Region &_r, const LATransform &_l,
const Move &_m, double t): r(_r), l(_l), m(_m), t(_t) {
  calculateInternalVars();
}
/*
This is the default constructor. Do not use.

*/
    FixedMRegion::FixedMRegion(){}
/*
This is the copy constructor.

*/
    FixedMRegion::FixedMRegion(const FixedMRegion &f): t(f.t), m(f.m), 
    l(f.l), r(f.r) {}
/*
This is the standard destructor.

*/
    FixedMRegion::~FixedMRegion() {}
    
    
    const Region& FixedMRegion::getRegion(){
      return r;
    }
    void FixedMRegion::setRegion(const Region &_r){
      r=_r;
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
    double FixedMRegion::gett(){
      return t;
    }
    void FixedMRegion::sett(double _t){
      t=_t;
      calculateInternalVars();
    }
    
    //FixedMRegion::calculateInternalVars(){
      //double* tmp=m.attime(t);
      
    //}
    ;