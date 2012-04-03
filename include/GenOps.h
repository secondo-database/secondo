/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, 
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----


*/

#include "NestedList.h"
extern NestedList* nl;


/*
4.1.0 [->] R

*/
template<class R>
ListExpr TypeMap0(ListExpr args){
  string err ="no arguments expected";
  if(!nl->IsEmpty(args)){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  return nl->SymbolAtom(R::BasicType());
}



/*
4.1.1  A [->] R

This type mapping can be used for non-overloaded
Operators.

*/
template<class A, class R>
ListExpr TypeMap1(ListExpr args){
  string err = A::BasicType()+ " expected";
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  if(!A::checkType(nl->First(args))){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  return nl->SymbolAtom(R::BasicType());
}

/*
4.1.2 A1 [x] A2 [->] R


*/
template<class A1, class A2, class R>
ListExpr TypeMap2(ListExpr args){
  string err = A1::BasicType()+ " x " + A2::BasicType() + " expected";
  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  if(!A1::checkType(nl->First(args)) ||
     !A2::checkType(nl->Second(args))){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  return nl->SymbolAtom(R::BasicType());
}

/*
4.1.2 A1 [x] A2 [x] A3 [->] R


*/
template<class A1, class A2, class A3, class R>
ListExpr TypeMap3(ListExpr args){
  string err = A1::BasicType()+ " x " + A2::BasicType() + "x" + 
               A3::BasicType() + " expected";
  if(nl->ListLength(args)!=3){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  if(!A1::checkType(nl->First(args)) ||
     !A2::checkType(nl->Second(args)) ||
     !A3::checkType(nl->Third(args))){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  return nl->SymbolAtom(R::BasicType());
}

/*
4.1.2 A1 [x] A2 [x] A3 [->] R


*/
template<class A1, class A2, class A3, class A4, class R>
ListExpr TypeMap4(ListExpr args){
  string err = A1::BasicType()+ " x " + A2::BasicType() + " x "+  
               A3::BasicType() + "x" + A4::BasicType() + " expected";
  if(nl->ListLength(args)!=3){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  if(!A1::checkType(nl->First(args)) ||
     !A2::checkType(nl->Second(args)) ||
     !A3::checkType(nl->Third(args)) ||
     !A4::checkType(nl->Fourth(args))){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  return nl->SymbolAtom(R::BasicType());
}



/*
4.1.3 More complex TypeMapping

~simpleTMHelper~

This class provides some functions for easy creating
simple type mappings as templates.

*/

class simpleTMHelper{
  public:
    
/*
~constructor~

*/
    simpleTMHelper():resultType(""),args(0){}

/*
~constructor~

This constructor sets the resulttype.

*/
    simpleTMHelper(const string resultType):args(0){
      this->resultType = resultType;
    }

/*
~operator<<~

Using this operator, we can create the arguments for this type mapping.
For example, to create a simpleTMHelper for a type mapping of the form
a [x] b [->] r, just use the following code:

----

  simpleTMHelper("r") << "a" << "b";

----


*/
    simpleTMHelper& operator<<(const string& arg){
      args.push_back(arg);
      return *this;
    }

/*
~Assignment operator~

*/
    simpleTMHelper& operator=(const simpleTMHelper& src){
       resultType = src.resultType;
       args = src.args;
       return *this;
    }

/*
~Copy constructor~

*/
    simpleTMHelper(const simpleTMHelper& src){
       resultType = src.resultType;
       args = src.args;
    }
/*
~Destructor~

*/
    ~simpleTMHelper(){

     }

/*
~check~

This function checks, whether  the argumentlist matchs this 
simple type mapping.

*/
     bool check(ListExpr args) const{
       if(nl->ListLength(args)!= static_cast<int>(this->args.size())){
          return false;
       } 
       int pos = 0;
       while(!nl->IsEmpty(args)){
          if(!nl->IsEqual(nl->First(args),this->args[pos])){
             return false;
          }
          args = nl->Rest(args);
          pos++;
       }
       return true;
     }

/*
~argString~

Returns the argumentlist formatted as  a x b x ...

*/
     string argString() const{
       if(args.size()==0){
          return "()";
       } else {
          stringstream ss;
          for(unsigned int i=0;i<args.size();i++){
            if(i>0) {
              ss << " x ";
            }
            ss << args[i];
          }
          return ss.str();
       }
     }
/*
~getResultType~

Returns the result type for this value mapping.

*/
     string getResultType()const{
        return resultType;
     }

  private:
    string resultType;
    vector<string> args;
};


/*
1.1 class tm0

This class provides a simple type mapping for operators  
of the format   [->] r.

*/
template<class R>
class tm0 : public simpleTMHelper{
  public:
  tm0():simpleTMHelper(R::BasicType()){
  }
};

/*
1.2 class tm1

Type mapping template for unary operators.

*/
template<class A,class R>
class tm1 : public simpleTMHelper{
 public:
  tm1():simpleTMHelper(R::BasicType()){
    *this << A::BasicType();
  }
};

/*
1.3 class tm2

Type mapping template for binary operators.

*/
template<class A1, class A2,class R>
class tm2 : public simpleTMHelper{
public:
  tm2():simpleTMHelper(R::BasicType()){
    *this << A1::BasicType();
    *this << A2::BasicType();
  }
};


/*
1.3 class tm3

Type mapping template for terner  operators.

*/
template<class A1, class A2, class A3, class R>
class tm3 : public simpleTMHelper{
public:
  tm3():simpleTMHelper(R::BasicType()){
    *this << A1::BasicType();
    *this << A2::BasicType();
    *this << A3::BasicType();
  }
};

/*
1.3 class tm4

Type mapping template for 4 arguments.

*/
template<class A1, class A2, class A3, class A4, class R>
class tm4 : public simpleTMHelper{
public:
  tm4():simpleTMHelper(R::BasicType()){
    *this << A1::BasicType();
    *this << A2::BasicType();
    *this << A3::BasicType();
    *this << A4::BasicType();
  }
};


/*
1.3 class tm5

Type mapping template for terner  operators.

*/
template<class A1, class A2, class A3, class A4, class A5,  class R>
class tm5 : public simpleTMHelper{
public:
  tm5():simpleTMHelper(R::BasicType()){
    *this << A1::BasicType();
    *this << A2::BasicType();
    *this << A3::BasicType();
    *this << A4::BasicType();
    *this << A5::BasicType();
  }
};

/*
1.3 class tm11

Type mapping template for 9 operators.

*/
template<class A1, class A2, class A3, class A4, class A5,
  class A6, class A7, class A8, class R>
class tm9 : public simpleTMHelper{
public:
  tm9():simpleTMHelper(R::BasicType()){
    *this << A1::BasicType();
    *this << A2::BasicType();
    *this << A3::BasicType();
    *this << A4::BasicType();
    *this << A5::BasicType();
    *this << A6::BasicType();
    *this << A7::BasicType();
    *this << A8::BasicType();
  }
};



/*
Enter here classes for further type mappings.

*/


/*
1.5 Complex Type Mapping

This class is a collection of type mappings. It can be used for
simple overloaded operators.

*/

class complexTM{
   public:
/*
~constructor~

*/
     complexTM():mappings(0){}

/*
~add~

This function adds a simple type mapping.

*/
     void add(simpleTMHelper tm){
        mappings.push_back(tm);
     } 

/*
~functor~

This functor realized the type mapping.

*/
     ListExpr operator()(const ListExpr args) const{
       stringstream  err;
       err <<  "allowed = " << endl << getSignatures();
       for(unsigned int i=0;i<mappings.size();i++){
         if(mappings[i].check(args)){
           return nl->SymbolAtom(mappings[i].getResultType());
         }
       } 
       ErrorReporter::ReportError(err.str());
       return nl->TypeError();
     }

/*
~select~

Provides functionality for the slection function.

*/

     int select(const ListExpr args) const{
       for(unsigned int i=0;i<mappings.size();i++){
         if(mappings[i].check(args)){
           return i;
         }
       } 
       return -1;
     }

/*
~getSignatures~

Returns the allowed signatures line by line.

*/
     string getSignatures()const {
       stringstream  sigs;
       for(unsigned int i=0;i<mappings.size();i++){
          sigs << mappings[i].argString() 
               << " -> " << mappings[i].getResultType() 
               << endl; 
       }
       return sigs.str();
     }


/*
~getSpecification~

Returns the specification string.

*/
     const string getSpecification(const string syntax,
                                   const string meaning,
                                   const string example ){
        return string("((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )")
               + "( <text>" + getSignatures() + "</text--->"
               + " <text>" + syntax + "</text--->" 
               + " <text>" + meaning + "</text--->"
               + " <text>" + example + "</text--->))"; 
     }
/*
~getVMCount~

Returns the number of stored value mappings.

*/
   int getVMCount()const{
     return mappings.size();
   }

   private:
      vector<simpleTMHelper> mappings;

};


/*
  ~Generic Value Mapping Function~

  This function realized value mappings in the form
    A1 [x] A2 [->] R
  where Fun is the function applied to a1 and a2.

*/
  template<class A1, class A2, class R, class Fun>
  int GenVM2(Word* args, Word& result, int message,
            Word& local, Supplier s){
    result = ::qp->ResultStorage(s);
    A1* a1 = static_cast<A1*>(args[0].addr);
    A2* a2 = static_cast<A2*>(args[1].addr);
    R* res = static_cast<R*>(result.addr); 
    if(!a1->IsDefined() || !a2->IsDefined()){
      res->SetDefined(false);
    }  else {
      Fun fun;
      fun(a1,a2,res);
  }
  return 0;
}

/*
  ~Generic Value Mapping Function~

  This function realized value mappings in the form
    A1 [x] A2 [x] A3 [->] R
  where Fun is the function applied to a1,a2, and a3.

*/
  template<class A1, class A2, class A3, class R, class Fun>
  int GenVM3(Word* args, Word& result, int message,
            Word& local, Supplier s){
    result = ::qp->ResultStorage(s);
    A1* a1 = static_cast<A1*>(args[0].addr);
    A2* a2 = static_cast<A2*>(args[1].addr);
    A3* a3 = static_cast<A3*>(args[2].addr);
    R* res = static_cast<R*>(result.addr); 
    if(!a1->IsDefined() || !a2->IsDefined() || !a3->IsDefined()){
      res->SetDefined(false);
    }  else {
      Fun fun;
      fun(a1,a2,a3,res);
  }
  return 0;
}

/*
  ~Generic Value Mapping Function~

  This function realized value mappings in the form
    A1 [x] A2 [x] A3 [x] A4 [->] R
  where Fun is the function applied to a1,a2,a3, and a4.

*/
  template<class A1, class A2, class A3,class A4, class R, class Fun>
  int GenVM4(Word* args, Word& result, int message,
            Word& local, Supplier s){
    result = ::qp->ResultStorage(s);
    A1* a1 = static_cast<A1*>(args[0].addr);
    A2* a2 = static_cast<A2*>(args[1].addr);
    A3* a3 = static_cast<A3*>(args[2].addr);
    A4* a4 = static_cast<A4*>(args[3].addr);
    R* res = static_cast<R*>(result.addr); 
    if(!a1->IsDefined() || !a2->IsDefined() || !a3->IsDefined()||
       !a4->IsDefined()){
      res->SetDefined(false);
    }  else {
      Fun fun;
      fun(a1,a2,a3,a4,res);
  }
  return 0;
}

/*
  ~Generic Value Mapping Function~

  This function realized value mappings in the form
    A1 [x] A2 [x] A3 [x] A4 [x] A5 [->] R
  where Fun is the function applied to a1,a2,a3,a4 and a5.

*/
  template<class A1, class A2, class A3,class A4, class A5, 
           class R, class Fun>
  int GenVM5(Word* args, Word& result, int message,
            Word& local, Supplier s){
    result = ::qp->ResultStorage(s);
    A1* a1 = static_cast<A1*>(args[0].addr);
    A2* a2 = static_cast<A2*>(args[1].addr);
    A3* a3 = static_cast<A3*>(args[2].addr);
    A4* a4 = static_cast<A4*>(args[3].addr);
    A5* a5 = static_cast<A5*>(args[4].addr);
    R* res = static_cast<R*>(result.addr); 
    if(!a1->IsDefined() || !a2->IsDefined() || !a3->IsDefined()||
       !a4->IsDefined() || !a5->IsDefined()){
      res->SetDefined(false);
    }  else {
      Fun fun;
      fun(a1,a2,a3,a4,a5,res);
  }
  return 0;
}

/*
  ~Generic Value Mapping Function~

  This function realized value mappings in the form
    A1 [x] A2 [x] A3 [x] A4 [x] A5 [x] A6 [x] A7 [x] A8 [->] R
  where Fun is the function applied to a1,a2,a3,a4,a5,a6,a7, and a8.

*/
  template<class A1, class A2, class A3,
           class A4, class A5, class A6,
           class A7, class A8, class R,
           class Fun>
  int GenVM9(Word* args, Word& result, int message,
            Word& local, Supplier s){
    result = ::qp->ResultStorage(s);
    A1* a1 = static_cast<A1*>(args[0].addr);
    A2* a2 = static_cast<A2*>(args[1].addr);
    A3* a3 = static_cast<A3*>(args[2].addr);
    A4* a4 = static_cast<A4*>(args[3].addr);
    A5* a5 = static_cast<A5*>(args[4].addr);
    A6* a6 = static_cast<A6*>(args[5].addr);
    A7* a7 = static_cast<A7*>(args[6].addr);
    A8* a8 = static_cast<A8*>(args[7].addr);
    R* res = static_cast<R*>(result.addr);
    if(!a1->IsDefined() || !a2->IsDefined() || !a3->IsDefined()||
       !a4->IsDefined() || !a5->IsDefined() || !a6->IsDefined()||
       !a7->IsDefined() || !a8->IsDefined() ){
      res->SetDefined(false);
    }  else {
      Fun fun;
      fun(a1,a2,a3,a4,a5,a6,a7,a8,res);
  }
  return 0;
}

/*
~Generic Value Mapping~

This value mapping realized functions with signatur
  A [->] R

*/
template<class A, class R, class Fun>
int GenVM1(Word* args, Word& result, int message,
          Word& local, Supplier s){
  result = ::qp->ResultStorage(s);
  A* a = static_cast<A*>(args[0].addr);
  R* res = static_cast<R*>(result.addr); 
  if(!a->IsDefined()){
    res->SetDefined(false);
  }  else {
    Fun fun;
    fun(a,res);
  }
  return 0;
}


