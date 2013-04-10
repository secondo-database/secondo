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

void DRM::computeFrom(const Rectangle<2>& r1, const Rectangle<2>& r2){
  if(!r1.IsDefined() || !r2.IsDefined()){
      SetDefined(false);
       return;
  }
  if(r1.IsEmpty()){
    value=0;
  } else if(r2.IsEmpty()){
    value = (uint8_t)511u;   
  } else {
    SetDefined(true);
    value = 0; // initialize value

    bool cols[3];
    cols[0] = r1.MinD(0) <= r2.MinD(0);
    cols[1] = (r1.MinD(0) <= r2.MaxD(0)) && 
              (r1.MaxD(0) >= r2.MinD(0));
    cols[2] = r1.MaxD(0) >= r2.MaxD(0);
    bool rows[3];
    rows[0] = r1.MinD(1) <= r2.MinD(1);
    rows[1] = (r1.MinD(1) <= r2.MaxD(1)) && 
              (r1.MaxD(1) >= r2.MinD(1));
    rows[2] = r1.MaxD(1) >= r2.MaxD(1);

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

    cout << "value bfore setting" << value << endl;

    if(rows[0] && cols[0] ) value |=1;
    if(rows[0] && cols[1] ) value |=2;
    if(rows[0] && cols[2] ) value |=4;
    if(rows[1] && cols[0] ) value |=8;
    if(rows[1] && cols[1] ) value |=16;
    if(rows[1] && cols[2] ) value |=32;
    if(rows[2] && cols[0] ) value |=64;
    if(rows[2] && cols[1] ) value |=128u;
    if(rows[2] && cols[2] ) value |=256u;

    cout << "value after setting" << value << endl;

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

