/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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


2013 Hamza Issa


*/

#ifndef DIVISIONSTREAM_H
#define DIVISIONSTREAM_H



#include "TemporalAlgebra.h"
#include <assert.h>

template<class Mapping1,class Unit1, class Mapping2, class Unit2>
class DivisionStream{

public:
 DivisionStream(const Mapping1* m1,const Mapping2* m2,ListExpr list);
 ~DivisionStream(){
  if(this->dividende){

   //this->dividende->DeleteIfAllowed();
   //this->dividende=NULL;
  }
  if(this->diviseur){
   //this->diviseur->DeleteIfAllowed();
   //this->diviseur=NULL;
  }
 
 };

 bool hasMore() const;

 bool getNext(Tuple** result);
   
 bool finished() const{
     return this->done;
  }

private :

 int dividendepos;
 int divideurpos;
    bool done;

 const Mapping1* dividende;
 const Mapping2* diviseur;
 ListExpr typelist;
};


/*Implememntation*/

template<class Mapping1,class Unit1, class Mapping2, class Unit2>
DivisionStream<Mapping1,Unit1,Mapping2,Unit2>::DivisionStream(
         const Mapping1 *m1, const Mapping2 *m2,ListExpr list){

 this->dividende=m1;
 this->diviseur=m2;
 this->dividendepos=0;
 this->divideurpos=0;
 this->done=false;
 this->typelist=nl->Second(list);
}

template<class Mapping1,class Unit1, class Mapping2, class Unit2>
bool DivisionStream<Mapping1,Unit1,Mapping2,Unit2>::hasMore() const{
 return !this->done;
 }
template<class Mapping1,class Unit1, class Mapping2, class Unit2>
bool DivisionStream<Mapping1,Unit1,Mapping2,Unit2>::getNext(Tuple** result){


    /*Here we will put our logique*/
 if(this->dividende==NULL || this->diviseur==NULL){
  this->done=true;
  return false;
 }

 int n2=this->diviseur->GetNoComponents();
 int n1=this->dividende->GetNoComponents();

 if((this->divideurpos>=n2)){
  this->done=true;
  return true;
 }

 Unit2  divideurUnit(true);
 this->diviseur->Get(this->divideurpos,divideurUnit);/*we should icremenet then
                                                      the value of divideurpos*/
    
 Interval<Instant> iv2;
 iv2 = divideurUnit.getTimeInterval();

    /*to check this one*/
 Mapping1* coloumn1=new Mapping1(true);

 Mapping2* coloumn2=new Mapping2(1);
    coloumn2->Add(divideurUnit);



 if(this->dividendepos<n1){

 Unit1 dividendeUnit(true);
 this->dividende->Get(this->dividendepos,dividendeUnit);/*we should icremenent
                                                 then the value of divideurpos*/
    

    Interval<Instant> iv1;
    iv1=dividendeUnit.getTimeInterval();




    
    /*we can use the after of the base class but it is better to implement ouwn
     because there are a lot of assert there*/
 
 bool after=(iv2.start>iv1.end)||(iv2.start==iv1.end && (!iv2.lc ||!iv1.rc));


 while(after){
  this->dividendepos++;
        if(this->dividendepos>=n1){
  break;
  }
        this->dividende->Get(this->dividendepos,dividendeUnit);
        iv1=dividendeUnit.getTimeInterval();
  after=(iv2.start>iv1.end)||(iv2.start==iv1.end && (!iv2.lc ||!iv1.rc));
 }

 if(this->dividendepos>=n1){
  goto finish;
  }

 Interval<Instant> tempresult;

 /*loop until the end of iv1 is outsie iv2 , so we stop beacyse this interval
  can ve used other time*/
 
 bool canbeusedagain=(iv1.end>iv2.end)||(iv1.end==iv2.end &&(iv1.rc &&!iv2.rc));
 while(canbeusedagain==false){
  iv2.Intersection(iv1,tempresult);
  Unit1 unit1(true);
        dividendeUnit.AtInterval(tempresult, unit1);
        coloumn1->Add(unit1);

  this->dividendepos++;
        if(this->dividendepos>=n1){
   break;
  }
        this->dividende->Get(this->dividendepos,dividendeUnit);
        iv1=dividendeUnit.getTimeInterval();
  canbeusedagain=(iv1.end>iv2.end)||(iv1.end==iv2.end && (iv1.rc &&!iv2.rc));

 }
 /*now if end of iv1 is after iv2*/
 after=(iv1.start>iv2.end)||(iv1.start==iv2.end && (!iv1.lc ||!iv2.rc));
 if(!after && canbeusedagain){
    iv1.Intersection(iv2,tempresult);
  Unit1 unit1(true);
        dividendeUnit.AtInterval(tempresult, unit1);
        coloumn1->Add(unit1);
 }
 }
finish:
 if(coloumn1->GetNoComponents()!=0){
  (*result) = new Tuple(this->typelist);
        (*result)->PutAttribute(0,coloumn1);
        (*result)->PutAttribute(1,coloumn2);
 }
 else{
  (*result) = new Tuple(this->typelist);
         //coloumn1->SetDefined(false);
        (*result)->PutAttribute(0,coloumn1);
        (*result)->PutAttribute(1,coloumn2);
 }
 this->divideurpos++;
 
 return true;
}

#endif