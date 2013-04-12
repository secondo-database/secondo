/*

Implementation of DRM class


*/

#include <algorithm>
#include <vector>

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


/*
2 Implementaion of objects interaction matrix

*/

OIM::OIM(bool def) : Attribute(def){
   count = (3 << 4) + 3; // 3 x 3 matrix
   uint8_t tmp[] = {0,0,0,0,0,0,0,0,0};
   memcpy(values,tmp,9*sizeof(uint8_t));
}

OIM::OIM(const OIM& src) : Attribute(src.IsDefined()){
  count = src.count;
  memcpy(values, src.values, 9*sizeof(uint8_t));
}

OIM& OIM::operator=(const OIM& src) {
  SetDefined(src.IsDefined());
  count = src.count;
  memcpy(values, src.values, 9*sizeof(uint8_t));
  return *this;
}

ListExpr OIM::ToListExpr(ListExpr typeInfo) {
  ListExpr valueList = nl->OneElemList(nl->IntAtom(values[0]));
  ListExpr last = valueList;
  int rows = (count & 0xF0) >> 4;
  int cols = count & 0x0F;
  for(int i=1;i<rows*cols;i++){
    last = nl->Append(last, nl->IntAtom(values[i]));
  }
  return nl->ThreeElemList( 
               nl->IntAtom(rows),
               nl->IntAtom(cols),
               valueList);
}

 bool OIM::ReadFrom(ListExpr value, ListExpr typeInfo) {
   if(!nl->HasLength(value,3)){
      return false;
   }
   if( (nl->AtomType(nl->First(value))!= IntType) ||
       (nl->AtomType(nl->Second(value))!=IntType) ||
       (nl->AtomType(nl->Third(value)) != NoAtom) ) {
     return false;
   }
   int rows = nl->IntValue(nl->First(value));
   if(rows<1 || rows > 3){
      return false;
   }
   int cols = nl->IntValue(nl->Second(value));
   if(cols<1 || cols>3){
     return false;
   }
   ListExpr vl = nl->Third(value);
   if(!nl->HasLength(vl,rows*cols)){
      return false;
   }
   uint8_t tmp[9];
   memset(tmp,0,9*sizeof(uint8_t));

   for(int i=0;i<rows*cols;i++){
     ListExpr f = nl->First(vl);
     vl = nl->Rest(vl);
     if(nl->AtomType(f)!=IntType){
        return false;
     }
     int t = nl->IntValue(f);
     if((t<0) || (t>3)){
        return false;
     }
     tmp[i] = (uint8_t) t;
   }

   count = cols;
   count = count |  (((uint8_t)rows) << 4);
   memcpy(values,tmp,9*sizeof(uint8_t));
   return true;
 }


int OIM::Compare(const Attribute* rhs) const {
  int cmp = Attribute::CompareDefs(rhs);
  if(cmp!=0 || !IsDefined()){
      return cmp;
  }
  OIM* oim = (OIM*) rhs;
  if(count < oim->count) return -1;
  if(count > oim->count) return 1;
  int rows = (count & 0xF0 )>> 4;
  int cols = (count & 0x0F);
  for(int i=0;i<rows*cols;i++){
    if(values[i] <oim->values[i]) return -1;
    if(values[i] >oim->values[i]) return 1;
  }
  return 0;
}
    
size_t OIM::HashValue() const {
  int rows = (count & 0xF0 )>> 4;
  int cols = (count & 0x0F);
  size_t res = 0;
  for(int i=0;i<cols*rows;i++){
    res =  (res << 3) | values[i];
  }
  return res;
}

 void OIM::CopyFrom(const Attribute* rhs){
    *this = *( (OIM*)rhs);
 }

 ostream& OIM::Print( ostream& os ) const{
    int rows = (count & 0xF0 )>> 4;
    int cols = (count & 0x0F);
    for(int i=0;i<rows;i++){
      for(int j=0;j<cols;j++){
        os << values[i*cols+j] << " ";
      }
      os << endl;
    }
    return os;
 }

 bool checkProper(double* minD, double* maxD){
    for(int i=0;i<2;i++){
       if(minD[i] > maxD[i]){
         return false;
       }
    }
    return true;
 }


 void OIM::computeFrom(const StandardSpatialAttribute<2>& a, 
                     const StandardSpatialAttribute<2>& b){

    if(!a.IsDefined() || !b.IsDefined()){
      SetDefined(false);
      return;
    }

    Rectangle<2> abox = a.BoundingBox();
    Rectangle<2> bbox = b.BoundingBox();

    if(!abox.IsDefined() || !bbox.IsDefined()){
       // empty objects
       count = (3 << 4 ) | 3;
       memset(values,0,9*sizeof(uint8_t));
       return;
    }

    // compute grid
    // process X
    vector<double> Lx1;
    Lx1.push_back(abox.MinD(0));
    Lx1.push_back(abox.MaxD(0)); 
    Lx1.push_back(bbox.MinD(0));
    Lx1.push_back(bbox.MaxD(0)); 
 
    sort(Lx1.begin(),Lx1.end());

    vector<double> Lx;
    Lx.push_back(Lx1[0]);
    double last = Lx1[0];

    for(int i=1;i<4;i++){
      double next = Lx1[i];
      if(!AlmostEqual(last,next)){
         Lx.push_back(next);
         last = next;
      }
    }


    // process Y
    vector<double> Ly1;
    Ly1.push_back(abox.MinD(1));
    Ly1.push_back(abox.MaxD(1)); 
    Ly1.push_back(bbox.MinD(1));
    Ly1.push_back(bbox.MaxD(1)); 
 
    sort(Ly1.begin(),Ly1.end());

    vector<double> Ly;
    Ly.push_back(Ly1[0]);
    last = Ly1[0];

    for(int i=1;i<4;i++){
      double next = Ly1[i];
      if(!AlmostEqual(last,next)){
         Ly.push_back(next);
         last = next;
      }
    }
    // now the grid is given by Lx and Ly

    uint8_t cols = (uint8_t)Lx.size() -1;
    uint8_t rows = (uint8_t)Ly.size() -1;


    assert(cols>0 && cols <4);
    assert(rows>0 && rows <4);

    count = (rows << 4) | cols;

    double minD[2] = {0,0};
    double maxD[2] = {0,0};

    Rectangle<2> r(true,minD,maxD);

    for(int y = 0; y<rows;y++){
      for(int x = 0;x<cols;x++){
         minD[0] = Lx[x];
         maxD[0] = Lx[x+1];
         minD[1] = Ly[rows-(1+y)];
         maxD[1] = Ly[rows-(0+y)];

         assert(checkProper(minD,maxD));
         r.Set(true,minD,maxD);

         int entry = 0;
         if(a.Intersects(r)){
           entry++;
         } 
         if(b.Intersects(r)){
           entry += 2;
         }
         values[y*cols+x]=entry;
      }
    }
 }
