/*

Implementation of DRM class


*/

#include "DRM.h"
#include "Attribute.h"
#include "RectangleAlgebra.h"
#include "NestedList.h"
#include "ListUtils.h"

DRM& DRM::operator=(const DRM& src){
   SetDefined(src.IsDefined());
   value = src.value;
   return *this;
}


   /** simple case for two rectangles **/
void DRM::computeFromR(const Rectangle<2>& r1, const Rectangle<2>& r2){
  if(!r2.IsDefined() || !r1.IsDefined()){
      SetDefined(false);
       return;
  }
  if(r2.IsEmpty()){
    value=0;
  } else if(r1.IsEmpty()){
    value = (uint8_t)511u;   
  } else {
    SetDefined(true);
    value = 0; // initialize value

    bool cols[3];
    cols[0] = r1.MinD(0) < r2.MinD(0);
    cols[1] = r1.MinD(0) < r2.MaxD(0) && r1.MaxD(0) > r1.MinD(0);
    cols[2] = r1.MaxD(0) > r2.MaxD(0);
    bool rows[3];
    rows[2] = r1.MinD(1) < r2.MinD(1);
    rows[1] = r1.MinD(1) < r2.MaxD(1) && r1.MaxD(1) > r1.MinD(1);
    rows[0] = r1.MaxD(1) > r2.MaxD(1);



    cout << "cols : " ;
    for( int i=0;i<3;i++){
      cout << cols[i] << " ";
    }
    cout << endl;  
    cout << "rows: " ;
    for( int i=0;i<3;i++){
      cout << rows[i] << " ";
    }
    cout << endl;  

    if(rows[0] && cols[0] ) value |=1;
    if(rows[0] && cols[1] ) value |=2;
    if(rows[0] && cols[2] ) value |=4;
    if(rows[1] && cols[0] ) value |=8;
    if(rows[1] && cols[1] ) value |=16;
    if(rows[1] && cols[2] ) value |=32;
    if(rows[2] && cols[0] ) value |=64;
    if(rows[2] && cols[1] ) value |=128;
    if(rows[2] && cols[2] ) value |=256;

  }
}

void setRectangle(Rectangle<2>& r, double xmin, double ymin, 
                                   double xmax, double ymax){
  double min[2] = {xmin,ymin};
  double max[2] = {xmax,ymax};
  r.Set(true,min,max);
}

void DRM::computeFrom(const StandardSpatialAttribute<2>& a,
                      const StandardSpatialAttribute<2>& b ) {

  if(!a.IsDefined() || !b.IsDefined()) {
    SetDefined(false);
    return;
  }
  SetDefined(true);
  Rectangle<2> abox = a.BoundingBox();
  Rectangle<2> bbox = b.BoundingBox();
  if(!bbox.IsDefined() || bbox.IsEmpty() ||
     !abox.IsDefined() || abox.IsEmpty()){
     value = 0;
     return;
  }
  // otherwise, we can create the grid for computing DRM
  double x0 = min(abox.MinD(0),bbox.MinD(0)) - 5.0;
  double x1 = bbox.MinD(0);
  double x2 = bbox.MaxD(0);
  double x3 = max(abox.MaxD(0),bbox.MaxD(0)) + 5.0; 
    
  double y0 = min(abox.MinD(1),bbox.MinD(1)) - 5.0;
  double y1 = bbox.MinD(1);
  double y2 = bbox.MaxD(1);
  double y3 = max(abox.MaxD(1),bbox.MaxD(1)) + 5.0; 

  double minD[2] = {0,0};
  double maxD[2] = {1,1};

  
  Rectangle<2> r(true,minD,maxD);
  value = 0;

  // row 1
  setRectangle(r, x0,y2,x1,y3);
  if(a.Intersects(r)){
    value |= 1;
  }
  setRectangle(r,x1,y2,x2,y3);
  if(a.Intersects(r)){
    value |= 2;
  }
  setRectangle(r,x2,y2,x3,y3);
  if(a.Intersects(r)){
    value |= 4;
  }


  // row 2
  setRectangle(r,x0,y1,x1,y2);
  if(a.Intersects(r)){
     value |=8;
  }
  setRectangle(r,x1,y1,x2,y2);
  if(a.Intersects(r)){
     value  |=16;
  }
  setRectangle(r,x2,y1,x3,y2);
  if(a.Intersects(r)){
     value  |=32;
  }

  // row3
  setRectangle(r,x0,y0,x1,y1);
  if(a.Intersects(r)){
     value |=64;
  }
  setRectangle(r,x1,y0,x2,y1);
  if(a.Intersects(r)){
     value  |=128;
  }
  setRectangle(r,x2,y0,x3,y1);
  if(a.Intersects(r)){
     value  |=256;
  }

}






ListExpr DRM::ToListExpr(ListExpr typeInfo){
  if(!IsDefined()){
    return listutils::getUndefined();
  } else {
    return nl->IntAtom(value);
  }
}


bool DRM::ReadFrom(ListExpr _value, ListExpr typeInfo) {
  if(listutils::isSymbolUndefined(_value)){
     SetDefined(false);
     return true;
  } 
  if(nl->AtomType(_value)!=IntType){
    return false;
  }
  int v = nl->IntValue(_value);
  if(v<0 || v>511){
    return false;
  }
  SetDefined(true);
  value = v;
  return true;
}



int DRM::Compare(const Attribute* rhs) const{
  int cmp = Attribute::CompareDefs(rhs);
  if(cmp!=0 || !IsDefined()){
     return cmp;
  }
  DRM* d = (DRM*) rhs;
  return value<d->value?-1:(value==d->value?0:1);
}

void DRM::CopyFrom(const Attribute* rhs) {
    *this = *((DRM*)rhs);
}

ostream& DRM::Print( ostream& os ) const {
  if(!IsDefined()){
     os << "undef";
  } else {
     os << value; 
  }
  return os;
}

