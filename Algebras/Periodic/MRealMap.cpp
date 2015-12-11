
/*
3.11 ~MRealMap~ 

*/
#include <iostream>
#include <string>

#include "NestedList.h"
#include "PeriodicTypes.h"
#include "PeriodicSupport.h"
#include "StandardTypes.h"

extern NestedList* nl;

using namespace std;
using namespace datetime;

namespace periodic{
/*
~Constructor~

*/
MRealMap::MRealMap(){}

/*
~Constructor~

Creates a realmap for the constant value zero.

*/
MRealMap::MRealMap(int dummy){
   a = 0.0;
   b = 0.0;
   c = 0.0;
   root=false;
}


/*
~constructor~

This constructor sets the value of this moving real map to the
given values. 

*/

MRealMap::MRealMap(double a, double b, double c, bool root){
 Set(a,b,c,root);
}

/*
~Copy constructor~

*/
MRealMap::MRealMap(const MRealMap& source){
   Equalize(&source);
}

/*
~Destructor~

*/
MRealMap::~MRealMap(){}

/*
~Assignment operator~

*/
MRealMap& MRealMap::operator=(const MRealMap& source){
    Equalize(&source);
    return *this;
}

/*
~Set~

This function sets this MRealMap to represent the function:

                Figure 5: Formula of an Moving Real Map  [MRealFormula.eps]

*/
void MRealMap::Set(double a, double b, double c, bool root){
  this->a=a;
  this->b=b;
  this->c=c;
  this->root=root;
  Unify();
}


/*
~ExtremumAt~

Return the point in time where this Map has its extremum.
If no extremum is present, i.e. if a==0, an undefined 
duration value is returned.

*/
DateTime MRealMap::ExtremumAt()const{
   DateTime res(instanttype);
   if(a==0){
      res.SetDefined(false);
      return res;
   }
   res.ReadFrom(-b / (2*a) );
   return res;

}

 
/*
~ReadFrom function~

This function reads the value of this map from a list expression. 
If the list does not represent a valid moving real map,
the value of this map remains unchanged and the result will be __false__.
Otherwise, this value is changed to the value given in the list and 
__true__ is returned.

*/
bool MRealMap::ReadFrom(ListExpr le){
  if(::nl->ListLength(le)!=4)
     return false;
  double tmpa,tmpb,tmpc;
  if(::nl->AtomType(::nl->Fourth(le))!=BoolType)
     return false;
  bool tmproot=::nl->BoolValue(::nl->Fourth(le));
  if(!GetNumeric(::nl->First(le),tmpa))
     return false;
  if(!GetNumeric(::nl->Second(le),tmpb))
     return false;
  if(!GetNumeric(::nl->Third(le),tmpc))
     return false;
  a = tmpa;
  b = tmpb;
  c = tmpc;
  root = tmproot;
  Unify();
  return true;
}

/*
~ToListExpr~

This function returns the nested list representation of this map.

*/
ListExpr MRealMap::ToListExpr()const{
  return ::nl->FourElemList(::nl->RealAtom(a),
                          ::nl->RealAtom(b),
                          ::nl->RealAtom(c),
                          ::nl->BoolAtom(root));
}


/*
~At~

Computes the value of this map for a given instant. Ensure, that the
map is defined at this instant using the ~IsDefinedAt~ function.

*/
double MRealMap::At(const DateTime* duration) const {
  double d = duration->ToDouble();
  return At(d);
}


/*
~At~

This function may be used for a faster computing of a value of this 
map.

*/
double MRealMap::At(const double d) const{
  double r1 = (a*d+b)*d+c;
  if(root) return sqrt(r1);
  return r1;
}

/*
~IsDefinedAt~

This function checks whether this map is defined at the given instant.

*/
bool MRealMap::IsDefinedAt(const DateTime* duration) const {
   if(!root) // defined all the time
       return true;
   double d = duration->ToDouble();
   double r1 = (a*d+b)*d+c;
   return r1>=0; // defined only if expression under the root is positive
}

/*
~IsDefinedAt~

Returns __true__ if this map is defined for the given value.

*/
bool MRealMap::IsDefinedAt(const double d) const{
   if(!root)
       return true;
   double r1 = (a*d+b)*d+c;
   return r1>=0; // defined only if expression under the root is positive
}


/*
~EqualsTo~

This function checks for equality between this map and the argument.

*/
bool MRealMap::EqualsTo(const MRealMap RM2)const{
  return a == RM2.a && 
         b == RM2.b && 
         c == RM2.c &&
         root == RM2.root;
}


/*
~Equalize~

If this function is called, the valu of this map is taken from the argument.

*/
void MRealMap::Equalize(const MRealMap* RM){
   a = RM->a;
   b = RM->b;
   c = RM->c;
   root = RM->root;
}


/*
~EqualsTo~ 

This function checks whether the argument is an extension for this map when the
arguments starts __timediff__ later. This means:

[forall] x [in] [R]: this[->]GetValueAt(x+timediff)==RM[->]GetValueAt(x)


*/
bool MRealMap::EqualsTo(const MRealMap& RM, double timediff) const{
 if(root!=RM.root)
     return false;
 return RM.a == a &&
        RM.b == 2*a*timediff+b &&
        RM.c == timediff*timediff + b*timediff + c; 

}


/*
~Unify~ 

This functions converts this moving real into a unified representation.

*/
void MRealMap::Unify(){
  if(!root) return;
  if(a==0 && b==0){
      c = sqrt(c);
      root = false;
      return;
  }
  if(b==0 && c==0){
      b=sqrt(a);
      a=0;
      root=false;
      return;
  }
}


} // end of namespace periodic
