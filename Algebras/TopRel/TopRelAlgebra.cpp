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

//[_] [\_]
//[&] [\&]
//[toc] [\tableofcontents]
//[title] [ \title{TopRel Algebra} \author{Thomas Behr} \maketitle]
//[times] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]

[title]
[toc]

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

1. Includes and Definitions

1.1 Includes

*/

#include "TopRel.h"
#include <string>
#include <iostream>
#include <sstream>
#include "NestedList.h"
#include "Algebra.h"
#include "QueryProcessor.h"
#include "SecondoSystem.h"
#include "Attribute.h"
#include "StandardTypes.h"
#include "../../Tools/Flob/DbArray.h"
#include "LogMsg.h"

#include "FTextAlgebra.h"
#include "GenericTC.h"
#include "Symbols.h"




extern "C"{
#include "Tree.h"
}

/*
1.2 Definition of frequently used Constants

*/

// define powers of two
static const unsigned short P0 = 1;
static const unsigned short P1 = 2;
static const unsigned short P2 = 4;
static const unsigned short P3 = 8;
static const unsigned short P4 = 16;
static const unsigned short P5 = 32;
static const unsigned short P6 = 64;
static const unsigned short P7 = 128;
static const unsigned short P8 = 256;
static const unsigned short P9 = 512;



static const int PART_INTER = 1;
static const int NO_EXT_INTER = 2;
static const int O1_EMPTY = 4;
static const int O2_EMPTY = 8;
static const int O1_NON_EMPTY = 16;
static const int O2_NON_EMPTY = 32;
static const int O1_INNER = 64;
static const int O2_INNER = 128;




const unsigned char* getEmptyBlock(){

   static const unsigned char emptyBlock[]={ (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0,
    (const unsigned char)0, (const unsigned char)0, (const unsigned char)0};
   return emptyBlock;
}


const unsigned char* getFullBlock(){
  static const unsigned char fullBlock[]={
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255,
    (const unsigned char)255, (const unsigned char)255};
   return fullBlock;
}





// The name of the unspecified cluster
static const STRING_T UNSPECIFIED = "unspecified";

// an array containing the transposedd versions
// for a matrix number
static unsigned short TranspArray[512];


extern NestedList *nl;
extern QueryProcessor* qp;


/*
2 Realization of the classes

*/

namespace toprel{

/*

2.1 The Operations for the Int9M class

2.1.1 A Constructor

This constructor create a new 9 intersection matrix
with predefined content.

*/
Int9M::Int9M(const bool II, const bool IB, const bool IE,
             const bool BI, const bool BB, const bool BE,
             const bool EI, const bool EB, const bool EE):Attribute(true){
    Set(II,IB,IE,BI,BB,BE,EI,EB,EE);
}


/*
2.1.2 The Set function

This function sets all entries in the matrix to the given values.

*/
void Int9M::Set( const bool II, const bool IB, const bool IE,
                 const bool BI, const bool BB, const bool BE,
                 const bool EI, const bool EB, const bool EE){
   SetII(II);
   SetIB(IB);
   SetIE(IE);
   SetBI(BI);
   SetBB(BB);
   SetBE(BE);
   SetEI(EI);
   SetEB(EB);
   SetEE(EE);
   value = value & 511;
   SetDefined(true);
}


void Int9M::Transpose(){
     value = TranspArray[value];
}


/*
2.1.3 ToListExpr

This function returns the NestedList representation of a
9 intersection matrix.

*/
ListExpr Int9M::ToListExpr(ListExpr typeInfo) const {
  if(!IsDefined()){
    return nl->SymbolAtom("undefined");
  }
  ListExpr Last;
  ListExpr res = Last = nl->OneElemList(nl->BoolAtom(II&value));
  Last = nl->Append(Last,nl->BoolAtom(IB&value));
  Last = nl->Append(Last,nl->BoolAtom(IE&value));
  Last = nl->Append(Last,nl->BoolAtom(BI&value));
  Last = nl->Append(Last,nl->BoolAtom(BB&value));
  Last = nl->Append(Last,nl->BoolAtom(BE&value));
  Last = nl->Append(Last,nl->BoolAtom(EI&value));
  Last = nl->Append(Last,nl->BoolAtom(EB&value));
  Last = nl->Append(Last,nl->BoolAtom(EE&value));
  return res;
}

/*
2.1.4 ReadFrom

If this function is called, this matrix gets its value from the
given nested list. If the nested list don't represent a valid
9-intersection matrix, the matrix remains unchanged and the
result of this function will be ~false~.
Allowed representations are a single int atom in range [0, 511],
or a list containing nine boolean (or int) atoms representing the matrix entries.

*/
bool Int9M::ReadFrom(const ListExpr LE,const ListExpr typeInfo){
   // case uf undefined
   if(nl->IsEqual(LE,"undefined")){
       SetDefined(false);
       return true;
   }

   if(nl->AtomType(LE)==IntType){
      int v = nl->IntValue(LE);
      if(v<0){
         ErrorReporter::ReportError("matrixnaumber less than zero");
         return false;
      }
      if(v>511){
         ErrorReporter::ReportError("matrix number more than 511");
         return false;
      }
      value = (unsigned short) v;
      SetDefined(true);
      return true;
   }

   if(nl->ListLength(LE)!=9){
      ErrorReporter::ReportError("invalid listlength");
      return false;
   }
   unsigned short tmp=0;
   unsigned short pos=256;
   ListExpr r = LE;
   ListExpr f;
   bool b;
   for(int i=0;i<9;i++){
      f = nl->First(r);
      if(nl->AtomType(f)==BoolType)
         b = nl->BoolValue(f);
      else if(nl->AtomType(f)==IntType)
         b = nl->IntValue(f)!=0;
      else{ // not an allowed type
         ErrorReporter::ReportError("matrix entry must"
                                    " be an int or a boolean");
         return false;
      }
      if(b)
         tmp = tmp | pos;
      pos = pos/2;
      r = nl->Rest(r);
   }
   value = tmp;
   SetDefined(true);
   return true;
}

/*
2.1.5 Equalize functions

By calling the Equalize function, the matrix gets its value from
the argument.

*/
void Int9M::Equalize(const Int9M& value){
      Equalize(&value);
}

void Int9M::Equalize(const Int9M* v){
    this->value = v->value;
    SetDefined(v->IsDefined());
}

/*
2.1.6 Compare function

This functions compares this matrix with arg. arg must be of type
Int9M.

*/
int Int9M::Compare(const Attribute* arg) const{
    Int9M* v = (Int9M*) arg;
    if(!IsDefined() && !v->IsDefined()){
        return 0;
    }
    if(!IsDefined()){
       return -1;
    }
    if(!v->IsDefined()){
       return 1;
    }

    if(value < v->value){
       return -1;
    } else  if(value >v->value){
       return 1;
    } else {
       return 0;
    }

}


/*
2.1.11 HashValue

*/
size_t Int9M::HashValue() const{
   if(!IsDefined())
      return (size_t) 512;
   else
      return (size_t) value;
}

/*
2.1.12 CopyFrom

*/
void Int9M::CopyFrom(const Attribute* arg){
    Equalize((Int9M*) arg);
}

/*
2.1.13 Clone

*/
Int9M* Int9M::Clone() const
{
   unsigned short number  = 0;
   Int9M* res = new Int9M(number);
   res->Equalize(this);
   return res;
}

/*
2.1.14 Print

*/
 ostream& Int9M::Print( ostream& os ) const{
     os    << value;
     return os;
 }

/*
2.1.15 ToString

This function creates the sting repreentation of a matrix.

*/
  string Int9M::ToString() const{
     int ii = GetII()?1:0;
     int ib = GetIB()?1:0;
     int ie = GetIE()?1:0;
     int bi = GetBI()?1:0;
     int bb = GetBB()?1:0;
     int be = GetBE()?1:0;
     int ei = GetEI()?1:0;
     int eb = GetEB()?1:0;
     int ee = GetEE()?1:0;
     stringstream res;
     res << "\n"<<ii<<" "<<ib<<" "<<ie<<"\n"
         <<       bi<<" "<<bb<<" "<<be<<"\n"
         <<       ei<<" "<<eb<<" "<<ee<<"\n";
     return res.str();
  }


/*
2.1.15 Equal Operator

*/
bool Int9M::operator==(const Int9M& I2) const{
   return CompareTo(I2)==0;
}


/*
2.2  Definitions for the Cluster Type

*/

/*
2.2.1 The ValueAt function

This functions checks whether the matrix with number pos is a
member of this cluster.

*/
bool Cluster::ValueAt(const int pos) const{
   assert(pos >= 0 && pos < 512);
   int bytenum = pos / 8;
   int bytepos = pos % 8;
   unsigned char theByte = BitVector[bytenum];
   switch(bytepos){
       case 0 : return P0 & theByte;
       case 1 : return P1 & theByte;
       case 2 : return P2 & theByte;
       case 3 : return P3 & theByte;
       case 4 : return P4 & theByte;
       case 5 : return P5 & theByte;
       case 6 : return P6 & theByte;
       case 7 : return P7 & theByte;
       default : assert(false);
   }
}



bool Cluster::ValueAt(const int pos,const unsigned char BitVector[64]){
   assert(pos >= 0 && pos < 512);
   int bytenum = pos / 8;
   int bytepos = pos % 8;
   unsigned char theByte = BitVector[bytenum];
   switch(bytepos){
       case 0 : return P0 & theByte;
       case 1 : return P1 & theByte;
       case 2 : return P2 & theByte;
       case 3 : return P3 & theByte;
       case 4 : return P4 & theByte;
       case 5 : return P5 & theByte;
       case 6 : return P6 & theByte;
       case 7 : return P7 & theByte;
       default : assert(false);
   }


}


/*
2.2.2 ToString

*/

string Cluster::ToString() const{
    stringstream o;
    o << name << "(";
    for(int i=0;i<512;i++){
       if(ValueAt(i)){
          o << i << " ";
       }
    }
    o << ")";
    return  o.str();

}


/*
2.2.2 The Contains function

This functions checks whether the given Int9M matrix is part of this cluster

*/

bool Cluster::Contains(const Int9M M) const{
   int number = M.GetNumber();
   return ValueAt(number);
}

/*
2.2.1 The SetValueAt function

This function sets the containedness of the matrix number pos to the
given value.

*/
inline static void SetValueAtSimple(const int pos,const bool value,
                         unsigned char bitvector[]){
    assert(pos>=0 && pos < 512);
    int bytenum = pos / 8;
    int bytepos = pos % 8;
    if(!value){
      switch(bytepos){
          case 0 : bitvector[bytenum] &= (255 - P0);break;
          case 1 : bitvector[bytenum] &= (255 - P1);break;
          case 2 : bitvector[bytenum] &= (255 - P2);break;
          case 3 : bitvector[bytenum] &= (255 - P3);break;
          case 4 : bitvector[bytenum] &= (255 - P4);break;
          case 5 : bitvector[bytenum] &= (255 - P5);break;
          case 6 : bitvector[bytenum] &= (255 - P6);break;
          case 7 : bitvector[bytenum] &= (255 - P7);break;
          default : assert(false);
      }
    } else { // value = true
      switch(bytepos){
           case 0 : bitvector[bytenum] |= P0;break;
           case 1 : bitvector[bytenum] |= P1;break;
           case 2 : bitvector[bytenum] |= P2;break;
           case 3 : bitvector[bytenum] |= P3;break;
           case 4 : bitvector[bytenum] |= P4;break;
           case 5 : bitvector[bytenum] |= P5;break;
           case 6 : bitvector[bytenum] |= P6;break;
           case 7 : bitvector[bytenum] |= P7;break;
           default : assert(false);
      }
    }
}

void Cluster::SetValueAt(const int pos,const bool value,
                         unsigned char bitvector[],
                         unsigned char bitvectorT[])const{
    assert(pos>=0 && pos < 512);
    SetValueAtSimple(pos,value,bitvector);
    int pos2 = TranspArray[pos];
    SetValueAtSimple(pos2,value,bitvectorT);
}


void Cluster::SetValueAt(const int pos, const bool value,
                         const bool updateBox){
    SetValueAt(pos,value,BitVector, BitVectorT);
    if(updateBox){
        updateBoxChecks();
    }
}


/*
2.2.2 The Transpose function

The transpose function modifies this cluster, so that after calling
this function, the cluster contains the transposed matrices. This can be used
to define new cluster from existining ones e.g. a ~contains~ cluster from a
~covered-by~ one.

*/
void Cluster::Transpose(const bool updateBC /*=true*/){
  unsigned char tmp[64];
  memcpy(tmp,BitVector,64);
  memcpy(BitVector,BitVectorT,64);
  memcpy(BitVectorT,tmp,64);
  if(updateBC){
    int btmp = boxchecks;
    boxchecks = boxchecksT;
    boxchecksT = btmp;
  } else {
    boxchecksok = false;
  }
}



/*
2.2.2 The ToListExpr Function

This function converts the Cluster into a representation in nested list format.
The ListRepresentation is a list containing the name as string atom at the first
position followed by all matrix numbers of the
contained 9 intersection matrices.

*/
ListExpr Cluster::ToListExpr(const ListExpr TypeInfo)const{
    if(!IsDefined())
      return nl->SymbolAtom("undefined");
    // check for contained values

    ListExpr res = nl->OneElemList(nl->StringAtom(name));
    ListExpr last = res;
    for(int i=0;i<512;i++)
      if(ValueAt(i)){
          last = nl->Append(last,nl->IntAtom(i));
      }
    return res;
}

/*
2.2.3 The ReadFrom Function

This function reads the value of this cluster from its nested list
representation.  A cluster can be represented in different ways.
In each case, the first element of the list is a string-atom
representing the name of this cluster. It is follows from a
list of representations of 9 intersection matrices or a textual
representation (string, or text) of the conditions which must hold for this cluster.

*/
bool Cluster::ReadFrom(const ListExpr LE,const ListExpr typeInfo){

   // first, we handle the case of definition by condition
   // using the tree parser
   if(nl->ListLength(LE)==2 &&
      nl->AtomType(nl->First(LE))==StringType &&
      ( nl->AtomType(nl->Second(LE))==StringType ||
        nl->AtomType(nl->Second(LE))==TextType )){
      // Definition by condition
      struct tree* T=0;
      string text;
      if(nl->AtomType(nl->Second(LE))==StringType){
          text = (nl->StringValue(nl->Second(LE)));
      }
      else{
         nl->Text2String(nl->Second(LE),text);
      }
      const char* buffer=text.c_str();

      if(!parseString(buffer,&T)){
         char* tmp = GetLastMessage();
         if(tmp){
            string message(tmp);
            ErrorReporter::ReportError(message+"\n");
            free(tmp);
         }else{
           ErrorReporter::ReportError("Unknown error while "
                                      "parsing condition\n");
         }
         return false;
      }
      for(unsigned short i=0;i<512;i++){
          if(evalTree(T,i))
            SetValueAt(i,true,false);
          else
            SetValueAt(i,false,false);
      }
      strcpy(name,nl->StringValue(nl->First(LE)).c_str());
      destroyTree(T);
      updateBoxChecks();
      return true;
   }


   /*
   The second case is a list containing valid matrices.
   */
   unsigned char TMP[64];
   STRING_T TMPname;
   // initialize the temporary bitvector
   //for(int i=0;i<64;i++)
   //   TMP[i]=0;
   memcpy(TMP,getEmptyBlock(),64);

   bool correct = true;
   ListExpr scan = LE;
   ListExpr elem;
   // first, get the name of this cluster
   if(nl->ListLength(scan)<1){
       ErrorReporter::ReportError("Clustername missing");
       return false;
   }
   elem = nl->First(scan);
   if(nl->AtomType(elem)!=StringType){
      ErrorReporter::ReportError("The first elem in the"
                                 " list is not a string");
      return false;
   }

   strcpy(TMPname, nl->StringValue(elem).c_str());
   scan = nl->Rest(scan);
   Int9M current;
   while(!nl->IsEmpty(scan) && correct){
       elem = nl->First(scan);
       scan = nl->Rest(scan);
       if(!current.ReadFrom(elem,nl->TheEmptyList())){
          correct = false;
       }
       else{
          unsigned short v = current.GetNumber();
          SetValueAt(v,true,TMP,BitVectorT);
       }
   }

   if(!correct){
      return false;
   }
   memcpy(BitVector,TMP,64);
   strcpy(name,TMPname);
   Transpose(BitVector,BitVectorT);
   SetDefined(true);
   updateBoxChecks();
   return true;
}

/*
2.2.5 Transpose

*/
void Cluster::Transpose(unsigned char Source[64],
                        unsigned char Target[64]){
   for(int i=0;i<512;i++){
      if(ValueAt(i,Source)){
         SetValueAtSimple(TranspArray[i],true,Target);
      } else {
         SetValueAtSimple(TranspArray[i],false,Target);
      }
   }
}



/*
2.2.4 Equalize functions

Aided by this functions the value of this cluster can be setted to the
value of another existing cluster.

*/
void Cluster::Equalize(const Cluster& value){
     Equalize(&value);
}

void Cluster::Equalize(const Cluster* value){
     memset(name,'\0',MAX_STRINGSIZE);
     strcpy(name, (char*)value->name);
     memcpy(BitVector,value->BitVector,64);
     memcpy(BitVectorT,value->BitVectorT,64);
     SetDefined(value->IsDefined());
     boxchecks = value->boxchecks;
     boxchecksT = value->boxchecksT;
     boxchecksok = value->boxchecksok;
}

/*
2.2.5 Compare function

The Compare function compares two clusters. Needed for to be an attribute of a tuple.

*/
int Cluster::CompareTo(const Cluster* C)const{
   if(!IsDefined() && !C->IsDefined()){
     return 0;
   }
   if(!IsDefined()){
      return -1;
   }
   if(!C->IsDefined()){
      return 1;
   }
   for(int i=0;i<64;i++){
      if(BitVector[i]<C->BitVector[i]){
          return -1;
      }
      if(BitVector[i]>C->BitVector[i]){
          return 1;
      }
   }
   if(string(name)<string(C->name)){
      return -1;
   }
   if(string(name)>string(C->name)){
      return 1;
   }
   return 0;
}

/*
2.2.6 The Adjacent function

We don't want to build ranges over cluster.
For this reason we just return false here.

*/
bool Cluster::Adjacent(const Attribute*) const{
   return false;
}

/*
2.2.7 updateBoxChecks

This function codes some interesting information into the __boxchecks__
variable. The information is coded as


  * boxchecks [&] 1 > 0: each contained matrix has a 1 entry at positions ii | ib | bi | bb,
    if the boxes are disjoint, this cluster cannot contain the toprel of the corresponding
    objects

  * boxchecks [&] 2 > 0: each matrix has a 0 entry at positions ei [&] eb [&] ie [&] be,
    if the boxes are non-equal, this cluster cannot contain the toprel of the corresponding
    objects

  * ...

*/

void Cluster::updateBoxChecks(const unsigned char* bitvector,
                              int& boxchecks){
  boxchecks = 0;
  Int9M m1(1,1,0,1,1,0,0,0,0);
  Int9M m2(0,0,1,0,0,1,1,1,0);

  Int9M m3(1,1,1,1,1,1,0,0,0);
  Int9M m4(1,1,0,1,1,0,1,1,0);

  Int9M m5(0,0,1,0,0,1,0,0,0);
  Int9M m6(0,0,0,0,0,0,1,1,0);


  int n1 = m1.GetNumber();
  int n2 = m2.GetNumber();
  int n3 = m3.GetNumber();
  int n4 = m4.GetNumber();
  int n5 = m5.GetNumber();
  int n6 = m6.GetNumber();

  int part_inter = PART_INTER;
  int no_ext_inter = NO_EXT_INTER;
  int o1_empty = O1_EMPTY;
  int o2_empty = O2_EMPTY;
  int o1_non_empty = O1_NON_EMPTY;
  int o2_non_empty = O2_NON_EMPTY;
  int o1_inner = O1_INNER;
  int o2_inner = O2_INNER;


  for(unsigned short i=0; i<512; i++){
    if(ValueAt(i,bitvector)){
        if(! ( n1 & i ) ){
           part_inter = 0;
        }
        if( (i & n2) ){
           no_ext_inter = 0;
        }
        if( (i & n3)){
           o1_empty = 0;
        }
        if ((i & n4)){
           o2_empty = 0;
        }
        if(!(i & n3)){
           o1_non_empty = 0;
        }
        if(!(i & n4)){
           o2_non_empty = 0;
        }
        if( (i & n5)){
           o1_inner = false;
        }
        if( (i & n6)){
           o2_inner = false;
        }
    }
  }

  boxchecks = part_inter   | no_ext_inter | o1_empty | o2_empty |
              o1_non_empty | o2_non_empty | o1_inner | o2_inner;

}

void Cluster::updateBoxChecks(){
  updateBoxChecks(BitVector,boxchecks);
  updateBoxChecks(BitVectorT,boxchecksT);
  boxchecksok = true;
}

/*
2.2.7 checkBoxes

*/
int Cluster::checkBoxes(const Rectangle<2>& box1, const bool empty1,
               const Rectangle<2>& box2, const bool empty2) const{
  if(!boxchecksok){
      cout << "checkBoxes called but info is not up to date" << endl;
      return 3;
  }

  if(empty1 && empty2){
     Int9M m(0,0,0,0,0,0,0,0,1);
     if(Contains(m)){
        return 1;
     } else {
        return 2;
     }
  }

  if(!empty1 && (boxchecks & O1_EMPTY)){
      return 2;
  }

  if(!empty2 && (boxchecks & O2_EMPTY)){
      return 2;
  }

  if(empty1 && (boxchecks & O1_NON_EMPTY)){
     return 2;
  }

  if(empty2 && (boxchecks & O2_NON_EMPTY)){
     return 2;
  }

  if(boxchecks & PART_INTER){
    if(empty1 || empty2 ){
       return 2;
    }
    if(!box1.Intersects(box2)){
       return 2;
    }
  }

  if(boxchecks & NO_EXT_INTER){
     if(empty1 || empty2){ // boxes are different
        return 2;
     }
     if(box1 != box2){
        return 2;
     }
  }

  if(boxchecks & O1_INNER){
    if(!box2.Contains(box1)){
        return 2;
    }
  }
  if(boxchecks & O2_INNER){
    if(!box1.Contains(box2)){
       return 2;
    }
  }


  return 3; // nothinng known
}



/*
2.2.9 The HashValue function

This function computes a hash value of this cluster.

*/
size_t Cluster::HashValue() const{
   size_t result = 0;
   for(int i=0;i<64;i++)
       result += BitVector[i];
   return result;
}

/*
2.2.10 CopyFrom

Reads the value of this cluster from arg.

*/
void Cluster::CopyFrom(const Attribute* arg){
    Equalize( (Cluster*) arg);
}

/*
2.2.11 Clone

Returns a proper copy of this.

*/
Cluster* Cluster::Clone() const{
   Cluster* res = new Cluster(false);
   res->Equalize(this);
   return res;
}


/*
2.2.12 Restrict


This function restricts the matrices contained within this cluster to such ones
fulfilling the given condition.

*/

bool Cluster::Restrict(string condition, const bool updateBC /*=true*/){

   const char* cond_c = condition.c_str();
   struct tree* T=0;
   if(!parseString(cond_c,&T)){
      char* tmp = GetLastMessage();
      if(tmp){
         cmsg.warning() << "Error in parsing condition"
                        <<" during performing of Restrict" << endl
                        << tmp << endl;
         cmsg.send();
         free(tmp);
         tmp=0;
      } else{
         cmsg.warning() << "Unknown error while parsing argument of"
                        << " restrict" << endl;
         cmsg.send();
      }
      return false; // don't change this cluster
   }
   for(int i=0;i<512;i++){
      if(!evalTree(T,i)){
         SetValueAt(i,false,false);
      }
   }
   destroyTree(T);
   if(updateBC){
      updateBoxChecks();
   } else {
      boxchecksok = false;
   }
   return true;
}

/*
2.2.13 Restrict

This version of ~Restrict~ removes all matrices having a different as the given
value at the specified position. The pos must be from the set [{]II,...,EE[}].
Otherwise the result will not be as expected.


*/
 void Cluster::Restrict(const int pos, const bool value,
                        const bool updateBC/*=true*/){
    for(int i=0;i<512;i++){
       if( ((i&pos) !=0) != value){
          SetValueAt(i,false,false);
       }
    }
    if(updateBC){
       updateBoxChecks();
    } else {
       boxchecksok = false;
    }
 }


void Cluster::Restrict(const Int9M& m, const bool value,
                       const bool updateBC/*=true*/){
   int matrix = m.GetNumber();
   if(value){
      for(int i=0; i<512;i++){
         if( (matrix & i) != matrix ){
            SetValueAt(i,false,false);
         }
      }
   } else { // value==false
      matrix = matrix ^ 511;
      for(int i=0; i<512; i++){
         if(((i ^ 511 ) & matrix)!=matrix){
            SetValueAt(i,false,false);
         }
      }
   }
   if(updateBC){
      updateBoxChecks();
   } else {
      boxchecksok = false;
   }
}


   bool Cluster::isExtension(const Int9M& m) const{
      int matrix = m.GetNumber();
      for(int i=0;i<512;i++){
         if(!(ValueAt(i | matrix ))){
            return false;
         }
      }
      return true;
   }

/*
2.2.12 Relax


This function adds all matrices fulfilling the given condition to

*/

bool Cluster::Relax(string condition, const bool updateBC /*=true*/){

   const char* cond_c = condition.c_str();
   struct tree* T=0;
   if(!parseString(cond_c,&T)){
      char* tmp = GetLastMessage();
      if(tmp){
         cmsg.warning() << "Error in parsing condition during performing"
                        << " of Relax" << endl << tmp << endl;
         cmsg.send();
         free(tmp);
         tmp=0;
      } else{
         cmsg.warning() << "Unknown error while parsing argument of Relax"
                        << endl;
         cmsg.send();
      }
      return false; // don't change this cluster
   }
   for(int i=0;i<512;i++){
      if(evalTree(T,i)){
         SetValueAt(i,true,false);
      }
   }
   destroyTree(T);
   if(updateBC){
      updateBoxChecks();
   } else {
      boxchecksok = false;
   }
   return true;
}



/*
2.3.12 Operators for comparisons between Clusters

*/
bool Cluster::operator==(const Cluster& C2)const{
      return CompareTo(&C2)==0;
}

bool Cluster::operator<(const Cluster C2)const{
      return CompareTo(&C2)<0;
}

bool Cluster::operator>(const Cluster C2)const{
      return CompareTo(&C2)>0;
}

static int ClusterCompare(const void *a, const void *b){
   Cluster *ca = new ((void*)a) Cluster;
   Cluster *cb = new ((void*)b) Cluster;
   return ca->CompareTo(cb);
}

/*
2.3 Implementation of PredicateGroup

2.3.1 Constructor taking the number of clusters

This constructor sets the size of the internal managed
DBArray to the size specified in size. Additionally,
a single cluster 'unspecified' containing all possible
matrices is added.

*/
PredicateGroup::PredicateGroup( const int size ) :
     Attribute(true),
     theClusters( 0 ),
     canDelete( false ),
     sorted(true),
     unSpecified(true)
     {
   // at this point, we defined a single
   // clusters 'unspecified containing all
   // possible 9 intersection matrices
   unSpecified.SetName(&UNSPECIFIED);
}

/*
2.3.2 The Equalize function

When calling this function, the value of this predicate group
is taken from the argument of this function.

*/
void PredicateGroup::Equalize(const PredicateGroup* PG){
    Cluster tmp;
    Attribute::operator=(*PG);
    sorted = PG->sorted;
    canDelete = PG->canDelete;
    theClusters.copyFrom(PG->theClusters);
    unSpecified.Equalize(PG->unSpecified);
}

/*
2.3.3 The Compare function

This function compares two predicategroups. If needed, the
clusters contained in this predicategroup or in the argument
are sorted.

*/
int PredicateGroup::Compare(const Attribute* right) const{
   int cmp;
   const PredicateGroup* PG = static_cast<const PredicateGroup*>(right);

   if(!IsDefined() && !PG->IsDefined())
      return 0;
   if(!IsDefined() && PG->IsDefined())
      return -1;
   if(IsDefined() && !PG->IsDefined())
      return 1;
   // both predicategroups are defined

   if( (cmp = unSpecified.CompareTo(&(PG->unSpecified)))!=0)
       return cmp;

   int size1, size2;
   size1 = theClusters.Size();
   size2 = PG->theClusters.Size();
   if(size1<size2)
      return -1;
   if(size2<size1)
      return 1;
   // size1 == size2
   Cluster tmp1, tmp2;

   if(!sorted){
       theClusters.Sort(toprel::ClusterCompare);
       sorted=true;
   }
   if(!PG->sorted){
       PG->theClusters.Sort(toprel::ClusterCompare);
       PG->sorted=true;
   }

   for(int i=0;i<size1;i++){
       theClusters.Get(i,tmp1);
       PG->theClusters.Get(i,tmp2);
       cmp = tmp1.Compare(&tmp2);
       if(cmp != 0)
          return cmp;
   }
   return 0;
}

/*
2.3.4 ToString

*/
string PredicateGroup::ToString() const{
   Cluster cc;
   int size = theClusters.Size();
   stringstream o;
   o << "group {";
   for(int i=0;i< size;i++){
     theClusters.Get(i,cc);
     if(i>0) {
        o << ", ";
     }
     o << "[" << (cc) << "]";
   }
   o << "}";
   return o.str();

}


/*
2.3.4 ToListExpr

This function computes the external representation  of this
PredicateGroup in nested list format.

*/
ListExpr PredicateGroup::ToListExpr(const ListExpr typeInfo){
  // we have at least one element in the this predicategroup
  if(!IsDefined())
      return nl->SymbolAtom("undefined");
  Cluster C;
  if(theClusters.Size()==0)
     return nl->TheEmptyList();

  theClusters.Get(0,C);
  ListExpr res = nl->OneElemList(C.ToListExpr(nl->TheEmptyList()));
  ListExpr Last = res;
  for(int i=1;i<theClusters.Size();i++){
       theClusters.Get(i,C);
       Last = nl->Append(Last,C.ToListExpr(nl->TheEmptyList()));
  }
  return res;
}

/*
2.3.5 ReadFrom

This function reads the value of this predicategroup from
instance. If the argument is not a valid representation of a
predicate-cluster, this predicate cluster is not changed and
the result will be false.

*/
bool PredicateGroup::ReadFrom(const ListExpr instance, const ListExpr typeInfo){
   int length = nl->ListLength(instance);
   /*
    The maximum count of non-overlapping, non-empty clusters is 512.
   */
   if(length>512){
      ErrorReporter::ReportError("too many clusters");
      return false;
   }
   unSpecified.MakeEmpty();
   unSpecified.Invert();
   Cluster CurrentCluster(false);
   Cluster* AllClusters = new Cluster[length];
   int pos =0;
   ListExpr LE = instance;

   while(!nl->IsEmpty(LE)){
      // error in cluster representation
      ListExpr cl = nl->First(LE);
      if(!CurrentCluster.ReadFrom(cl, nl->TheEmptyList())){
         delete[] AllClusters;
         return false;
      }
      // empty clusters are not allowed
      if(CurrentCluster.IsEmpty()){
         ErrorReporter::ReportError("empty clusters are not allowed");
         delete[] AllClusters;
         return false;
      }
      //  overlapping cluster found
      if(! unSpecified.Contains(&CurrentCluster)){
         ErrorReporter::ReportError("Clusters overlap");
         delete[] AllClusters;
         return  false;
      }
      // check whether name already used
      STRING_T name;
      CurrentCluster.GetName(name);
      for(int i=0;i<pos;i++){
         STRING_T name2;
         AllClusters[i].GetName(name2);
         if(strcmp(name2,name)==0){
              ErrorReporter::ReportError("non disjoint names found");
              delete[] AllClusters;
              return false;
         }
      }

      if(strcmp(name,UNSPECIFIED)==0){ // forbidden name
          ErrorReporter::ReportError(" non valid name for cluster");
          delete[] AllClusters;
          return false;
      }
      unSpecified.Minus(&CurrentCluster);
      AllClusters[pos].Equalize(CurrentCluster);
      pos++;
      LE = nl->Rest(LE);
   }
   // if this state is reached, the list representation is correct
   SetDefined(true);
   theClusters.resize(length);
   for(int i=0;i<length;i++)
       theClusters.Put(i,(AllClusters[i]));
   theClusters.Sort(toprel::ClusterCompare);
   sorted=true;
   delete[] AllClusters;
   return true;
}

/*
2.3.6 The ~Add~ Function

When this function is called, the argument is added as a new cluster to this predicate
group. If this is not possible, e.g. because of overlaps with existing clusters
or name conflicts, the result will be false and this predicate group remains
unchanged. Empty clusters can't be added.

~Note~ : The Cluster is added without correcting the order of clusters in the
DBArray. Remember to reorder the clusters after calling this function.

*/
bool PredicateGroup::Add(Cluster* C){
   if(C->IsEmpty())
     return false;
   STRING_T name;
   C->GetName(name);

   if(strcmp(name,UNSPECIFIED)==0){
      return false;
   }
   if(!unSpecified.Contains(C)){ // overlap with existing clusters
      return false;
   }
   // check for name conflicts with existing clusters
   Cluster tmp;
   for(int i=0;i<theClusters.Size();i++){
        theClusters.Get(i,tmp);
        STRING_T name2;
        tmp.GetName(name2);
        if(strcmp(name2,name)==0){
            return false;
        }
   }
   // all ok, add the cluster
   unSpecified.Minus(C);
   theClusters.Append(*C);
   sorted=false;
   return true;
}

/*
2.3.6 The ~AddWithPriority~ Function

When this function is called, the argument is added as a new cluster to the predicate
group. If the new cluster overlaps with clusters already contained in the
predicate group, the common matrices are removed from the new cluster before
inserting. In case of name conflicts, the result will be false, otherwise the
result is true and the predicate group will contain the new (shortened) cluster.
If the cluster is already empty, or if the cluster is empty after removing existing
matrices, this function will detect an error.

~Note~ : The Cluster is added without correcting the order of clusters in the
DBArray. Remember to reorder the clusters after calling this function.

*/
bool PredicateGroup::AddWithPriority(const Cluster *C){
   if(C->IsEmpty()){
       return false;
   }

   STRING_T name;
   C->GetName(name);
   if(strcmp(name,UNSPECIFIED)==0){ // name conflict
      return false;
   }
   // check for name conflicts with existing clusters
   Cluster tmp;
   for(int i=0;i<theClusters.Size();i++){
        theClusters.Get(i,tmp);
        STRING_T name2;
        tmp.GetName(name2);
        if(strcmp(name2,name)==0){
            return false;
        }
   }



   Cluster aux(*C);
   aux.Intersection(&unSpecified); // remove overlapping clusters
   if(aux.IsEmpty())
     return false;
   // all ok, add the cluster
   unSpecified.Minus(&aux);
   theClusters.Append(aux);
   sorted=false;
   return true;
}

/*
~GetClusterOf~

Returns the clsuetr which has the given name or NULL if
no cluster found with this name. The Caller of this function
has to remove the result.

*/
Cluster* PredicateGroup::GetClusterOf(const STRING_T* name)const{
   if(strcmp((*name),UNSPECIFIED)==0){
     Cluster* res = new Cluster();
     res->Equalize(unSpecified);
     return res;
   }
   int size = theClusters.Size();
   Cluster tmp;
   for(int i=0;i<size;i++){
      theClusters.Get(i,tmp);
      STRING_T n;
      tmp.GetName(n);
      if(strcmp(n,*name)==0){
         Cluster* res = new Cluster();
         res->Equalize(&tmp);
         return res;
      }
   }
   return NULL;
}


Cluster* PredicateGroup::GetClusterOf(const string name)const{
  STRING_T theName;
  strcpy(theName,name.c_str());
  return GetClusterOf(&theName);
}


/*
~SetToDefault~

Sets this predicate group to a default (constant) value.

*/
void PredicateGroup::SetToDefault(){
     MakeEmpty(); // remove all old stuff
     SetDefined(true);
     canDelete=false;
     ListExpr nlCoveredBy = nl->TwoElemList(
                              nl->StringAtom("coveredBy"),
                              nl->TextAtom("ii & !ie & ei & bb")
                            );
     ListExpr nlDisjoint = nl->TwoElemList(
                               nl->StringAtom("disjoint"),
                               nl->TextAtom("!ii & !ib & !bi & !bb")
                            );

     ListExpr nlEqual = nl->TwoElemList(
                         nl->StringAtom("equal"),
                         nl->TextAtom("!ib & !ie & !bi & !be & !ei & !eb")
                      );
     ListExpr nlInside =nl->TwoElemList(
                       nl->StringAtom("inside"),
                       nl->TextAtom("ii & !ie & ei & !bb")
                      );
     ListExpr nlMeet = nl->TwoElemList(
                      nl->StringAtom("meet"),
                      nl->TextAtom("!ii & (ib | bi | bb)")
                    );
     ListExpr nlOverlap = nl->TwoElemList(
                      nl->StringAtom("overlap"),
                      nl->TextAtom("ii & ie & ei")
                    );
     bool ok;
     Cluster clEqual(false);

     ok = clEqual.ReadFrom(nlEqual,nl->TheEmptyList());

     assert(ok);
     Cluster clInside(false);
     ok = clInside.ReadFrom(nlInside,nl->TheEmptyList());
     assert(ok);
     Cluster clMeet(false);
     ok = clMeet.ReadFrom(nlMeet,nl->TheEmptyList());
     assert(ok);
     Cluster clOverlap(false);
     ok = clOverlap.ReadFrom(nlOverlap,nl->TheEmptyList());
     assert(ok);
     Cluster clCoveredBy(false);
     ok = clCoveredBy.ReadFrom(nlCoveredBy,nl->TheEmptyList());
     assert(ok);
     Cluster clDisjoint(false);
     ok = clDisjoint.ReadFrom(nlDisjoint,nl->TheEmptyList());
     assert(ok);
     Cluster clContains(clInside);
     clContains.Transpose();
     clContains.SetName("contains");
     Cluster clCovers(clCoveredBy);
     clCovers.Transpose();
     clCovers.SetName("covers");

     ok = AddWithPriority(&clEqual);
     if(!ok) {cerr  << "Equal not included" << endl; }
     ok = AddWithPriority(&clInside);
     if(!ok) {cerr  << "Inside not included" << endl; }
     ok = AddWithPriority(&clContains);
     if(!ok) {cerr  << "Contains not included" << endl; }
     ok = AddWithPriority(&clMeet);
     if(!ok) {cerr  << "Meet  not included" << endl; }
     ok = AddWithPriority(&clOverlap);
     if(!ok) {cerr  << "Overlap not included" << endl; }
     ok = AddWithPriority(&clDisjoint);
     if(!ok) {cerr  << "Disjoint not included" << endl; }
     ok = AddWithPriority(&clCoveredBy);
     if(!ok) {cerr  << "CoveredBy  not included" << endl; }
     ok = AddWithPriority(&clCovers);
     if(!ok) {cerr  << "Covers not included" << endl; }

     theClusters.Sort(toprel::ClusterCompare);
     sorted = true;
}

/*
3.1.15 Equal Operator

*/
bool PredicateGroup::operator==(const PredicateGroup& I2) const{
   return Compare(&I2)==0;
}


/*
4 Defining Type Constructors

*/

GenTC<Int9M> int9m;
GenTC<Cluster> cluster;
GenTC<PredicateGroup> predicategroup;


/*
7 Definition of Algebra operators

7.1 Type Mapping

*/
ListExpr Int9M_Int9M(ListExpr args){
   if(nl->ListLength(args)!=1){
      ErrorReporter::ReportError("wrong number of arguments expected: 1");
      return nl->TypeError();
   }
   ListExpr arg=nl->First(args);
   if(nl->IsEqual(arg,"int9m")){
      ErrorReporter::ReportError("int9m expected");
      return nl->SymbolAtom("int9m");
   }
   return nl->TypeError();
}

ListExpr Int9M_Int9M_Int9M(ListExpr args){
   if(nl->ListLength(args)!=2){
      ErrorReporter::ReportError("two arguments expected");
      return nl->TypeError();
   }
   if(nl->IsEqual(nl->First(args),"int9m") &&
      nl->IsEqual(nl->Second(args),"int9m")){
      return nl->SymbolAtom("int9m");
   }
   ErrorReporter::ReportError("int9m x int9m expected");
   return nl->TypeError();
}

ListExpr Int9M_Int(ListExpr args){
   if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError("One argument expected");
     return nl->TypeError();
   }
   if(nl->IsEqual(nl->First(args),"int9m")){
     return nl->SymbolAtom(CcInt::BasicType());
   }
   ErrorReporter::ReportError("int9m expected");
   return nl->TypeError();
}

ListExpr Cluster_String(ListExpr args){
   if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError("one argument expected");
     return nl->TypeError();
   }
   if(nl->IsEqual(nl->First(args),"cluster")){
     return nl->SymbolAtom(CcString::BasicType());
   }
   ErrorReporter::ReportError("cluster expected");
   return nl->TypeError();
}

ListExpr Cluster_Cluster(ListExpr args){
   if(nl->ListLength(args)!=1){
      ErrorReporter::ReportError("one argument expected");
      return nl->TypeError();
   }
   if(nl->IsEqual(nl->First(args),"cluster")){
      return nl->SymbolAtom("cluster");
   }
   ErrorReporter::ReportError("cluster expected");
   return nl->TypeError();
}

ListExpr Cluster_Int9M_Cluster(ListExpr args){
   if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("two arguments expected");
     return nl->TypeError();
   }
   if(nl->IsEqual(nl->First(args),"cluster") &&
      nl->IsEqual(nl->Second(args),"int9m")){
      return nl->SymbolAtom("cluster");
   }
   ErrorReporter::ReportError("cluster x int9m expected");
   return nl->TypeError();
}

ListExpr Cluster_Int9M_Bool(ListExpr args){
   if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("cluster x int9m expected");
     return nl->TypeError();
   }
   if(nl->IsEqual(nl->First(args),"cluster") &&
      nl->IsEqual(nl->Second(args),"int9m")){
      return nl->SymbolAtom(CcBool::BasicType());
   }
   ErrorReporter::ReportError("cluster x int9m expected");
   return nl->TypeError();
}

ListExpr Cluster_Cluster_Bool(ListExpr args){
   if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("cluster x cluster expected");
     return nl->TypeError();
   }
   if(nl->IsEqual(nl->First(args),"cluster") &&
      nl->IsEqual(nl->Second(args),"cluster")){
      return nl->SymbolAtom(CcBool::BasicType());
   }
     ErrorReporter::ReportError("cluster x cluster expected");
   return nl->TypeError();
}

ListExpr TransposeTM(ListExpr args){
   if(nl->ListLength(args)==1){
      if(nl->IsEqual(nl->First(args),"int9m"))
         return nl->SymbolAtom("int9m");
   }
   else if(nl->ListLength(args)==2){
      if(nl->IsEqual(nl->First(args),"cluster") &&
         nl->IsEqual(nl->Second(args),CcString::BasicType()))
         return nl->SymbolAtom("cluster");
   }
   ErrorReporter::ReportError("int9m  or cluster x string expected");
   return nl->TypeError();
}

ListExpr Cluster_String_Cluster(ListExpr args){
   if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("Two arguments expected");
     return nl->TypeError();
   }
   if(!nl->IsEqual(nl->First(args),"cluster")){
     ErrorReporter::ReportError("First argument must be a cluster");
     return nl->TypeError();
   }
   if(! nl->IsEqual(nl->Second(args),CcString::BasicType())){
     ErrorReporter::ReportError("The second argument must be a string");
     return nl->TypeError();
   }
   return nl->SymbolAtom("cluster");
}

ListExpr Cluster_Cluster_Cluster(ListExpr args){
   if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("cluster x cluster expected");
     return nl->TypeError();
   }
   if(nl->IsEqual(nl->First(args),"cluster") &&
      nl->IsEqual(nl->Second(args),"cluster")){
      return nl->SymbolAtom("cluster");
   }
   ErrorReporter::ReportError("cluster x cluster expected");
   return nl->TypeError();
}

ListExpr SetOpsTM(ListExpr args){
   if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("int9m x int9m or "
                                "cluster x cluster expected");
     return nl->TypeError();
   }
   if(nl->IsEqual(nl->First(args),"cluster") &&
      nl->IsEqual(nl->Second(args),"cluster")){
      return nl->SymbolAtom("cluster");
   }
   if(nl->IsEqual(nl->First(args),"int9m") &&
      nl->IsEqual(nl->Second(args),"int9m")){
      return nl->SymbolAtom("int9m");
   }
   ErrorReporter::ReportError("int9m x int9m or "
                              "cluster x cluster expected");
   return nl->TypeError();
}

ListExpr InvertTM(ListExpr args){
   if(nl->ListLength(args)!=1){
      ErrorReporter::ReportError("int9m or cluster  expected");
      return nl->TypeError();
   }

   if(nl->IsEqual(nl->First(args),"cluster")){
      return nl->SymbolAtom("cluster");
   }
   if(nl->IsEqual(nl->First(args),"int9m") ){
      return nl->SymbolAtom("int9m");
   }
   ErrorReporter::ReportError("int9m or cluster  expected");
   return nl->TypeError();
}

ListExpr CreatePGroupTM(ListExpr args){
   while(!nl->IsEmpty(args)){
      ListExpr current = nl->First(args);
      if(!nl->IsEqual(current,"cluster")){
         ErrorReporter::ReportError("cluster x cluster x ...  expected");
         return nl->TypeError();
      }
      args = nl->Rest(args);
   }
   return nl->SymbolAtom("predicategroup");
}

ListExpr CreateValidPGroupTM(ListExpr args){
  if(nl->ListLength(args)<1){
       ErrorReporter::ReportError("At Least one element required");
       return nl->TypeError();
   }
   while(!nl->IsEmpty(args)){
      ListExpr current = nl->First(args);
      if(!nl->IsEqual(current,"cluster")){
         ErrorReporter::ReportError("only clusters are allowed\n");
         return nl->TypeError();
      }
      args = nl->Rest(args);
   }
   return nl->SymbolAtom("predicategroup");
}

ListExpr ClusterName_OfTM(ListExpr args){
    if(nl->ListLength(args)==2){
        if(nl->IsEqual(nl->First(args),"predicategroup") &&
           nl->IsEqual(nl->Second(args),"int9m")){
            return nl->SymbolAtom(CcString::BasicType());
        }
    }
    ErrorReporter::ReportError("predicategroup x int9m expected");
    return nl->TypeError();
}

ListExpr ClusterOfTM(ListExpr args){
    if(nl->ListLength(args)==2){
        if(nl->IsEqual(nl->First(args),"predicategroup") &&
           nl->IsEqual(nl->Second(args),"int9m")){
            return nl->SymbolAtom("cluster");
        }
    }
    ErrorReporter::ReportError("predicategroup x int9m  expected");
    return nl->TypeError();
}

ListExpr GetClusterTM(ListExpr args){
    if(nl->ListLength(args)==2){
        if(nl->IsEqual(nl->First(args),"predicategroup") &&
           nl->IsEqual(nl->Second(args),CcString::BasicType())){
            return nl->SymbolAtom("cluster");
        }
    }
    ErrorReporter::ReportError("predicategroup x string expected");
    return nl->TypeError();
}

ListExpr SizeOfTM(ListExpr args){
   if(nl->ListLength(args)==1){
      if(nl->IsEqual(nl->First(args),"cluster") |
         nl->IsEqual(nl->First(args),"predicategroup")){
         return nl->SymbolAtom(CcInt::BasicType());
      } else {
        ErrorReporter::ReportError("TopRel:  SizeOf expects a cluster "
                                   "or a predicategroup\n");
        return nl->TypeError();
      }
   }
   ErrorReporter::ReportError("TopRel: Wrong number of"
                              " arguments for sizeof\n");
   return nl->TypeError();
}

ListExpr CreateClusterTM(ListExpr args){
   if(nl->ListLength(args)!=2){
      ErrorReporter::ReportError("CreateCluster expects 2 arguments\n");
      return nl->TypeError();
   }
   if( nl->IsEqual(nl->First(args),CcString::BasicType()) &&
       (nl->IsEqual(nl->Second(args),CcString::BasicType()) ||
        nl->IsEqual(nl->Second(args),FText::BasicType())))
        return nl->SymbolAtom("cluster");
   ErrorReporter::ReportError("CreateCluster requires"
                              " string x {string,text} as arguments\n");
   return nl->TypeError();
}

ListExpr IsCompleteTM(ListExpr args){
  if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError("IsComplete has one element\n");
     return nl->TypeError();
  }
  if(nl->IsEqual(nl->First(args),"predicategroup")){
     return nl->SymbolAtom(CcBool::BasicType());
  }
  ErrorReporter::ReportError("IsComplete"
                             " needs a predicategroup as argument\n");
  return nl->TypeError();
}

ListExpr UnSpecifiedTM(ListExpr args){
  if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError("unspecified needs one element\n");
     return nl->TypeError();
  }
  if(nl->IsEqual(nl->First(args),"predicategroup")){
     return nl->SymbolAtom("cluster");
  }
  ErrorReporter::ReportError("unspecified"
                             " needs a predicategroup as argument\n");
  return nl->TypeError();
}

ListExpr MultiSetOpsTM(ListExpr args){
  if(nl->ListLength(args)<1){
     ErrorReporter::ReportError("At least one argument required\n");
     return nl->TypeError();
  }
  ListExpr rest = nl->Rest(args);
  if(nl->IsEqual(nl->First(args),"int9m")){
      while(!nl->IsEmpty(rest)){
          if(!nl->IsEqual(nl->First(rest),"int9m")){
              ErrorReporter::ReportError("elements must be from"
                                         " the same type");
              return  nl->TypeError();
          }
          rest = nl->Rest(rest);
      }
      return nl->SymbolAtom("int9m");
  }
  if(nl->IsEqual(nl->First(args),"cluster")){
      while(!nl->IsEmpty(rest)){
          if(!nl->IsEqual(nl->First(rest),"cluster")){
              ErrorReporter::ReportError("elements must be from the"
                                         " same type");
              return  nl->TypeError();
          }
          rest = nl->Rest(rest);
      }
      return nl->SymbolAtom("cluster");
  }
  ErrorReporter::ReportError("int9m or cluster required");
  return nl->TypeError();
}

ListExpr PWDisjointTM(ListExpr args){
  ListExpr rest = args;
  while(!nl->IsEmpty(rest)){
    if(!nl->IsEqual(nl->First(rest),"cluster")){
       ErrorReporter::ReportError("elements must be of type cluster");
       return nl->TypeError();
    }
    rest = nl->Rest(rest);
  }
  return nl->SymbolAtom(CcBool::BasicType());
}


ListExpr RestrictRelaxTM(ListExpr args){
  if(nl->ListLength(args)!=2){
      ErrorReporter::ReportError("Restrict and Relax require two arguments\n");
      return nl->TypeError();
  }
  if(!nl->IsEqual(nl->First(args),"cluster")){
      ErrorReporter::ReportError("Restrict and Relax require"
                                 " cluster as first argument \n");
      return nl->TypeError();
  }
  if(!nl->IsEqual(nl->Second(args),CcString::BasicType()) &&
     !nl->IsEqual(nl->Second(args),FText::BasicType())){
      ErrorReporter::ReportError("Restrict and Relax require string or text"
                              " as second argument ");
      return nl->TypeError();
  }
  return nl->SymbolAtom("cluster");
}


ListExpr StdPGroup_TM(ListExpr args){
  if(nl->ListLength(args)==0){
     return nl->SymbolAtom("predicategroup");
  }
  ErrorReporter::ReportError("stdpgroup does not expect any argument.");
  return nl->TypeError();
}


ListExpr TopRelEqualsTM(ListExpr args){
  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("two arguments required");
    return nl->TypeError();
  }
  if(!nl->Equal(nl->First(args),nl->Second(args))){
    ErrorReporter::ReportError("cannot compare different types");
    return nl->TypeError();
  }
  if(nl->IsEqual(nl->First(args),"int9m") ||
     nl->IsEqual(nl->First(args),"cluster") ||
     nl->IsEqual(nl->First(args),"predicategroup")){
     return nl->SymbolAtom(CcBool::BasicType());
  }
  ErrorReporter::ReportError("int9m, cluster, or predicategroup expected");
  return nl->TypeError();
}


/*
7.2 Value Mappings

*/
int Transpose_Int9M_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Int9M* a = (Int9M*) args[0].addr;
  Int9M* res = (Int9M*) result.addr;
  if(res->IsDefined()){
    res->Equalize(a);
    res->Transpose();
  } else {
     res->SetDefined(false);
  }
  return 0;
}

int Transpose_Cluster_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  CcString* b = (CcString*) args[1].addr;
  Cluster* res = (Cluster*) result.addr;
  if(a->IsDefined() && b->IsDefined()){
    res->Equalize(a);
    res->Transpose();
    res->SetName(b->GetStringval());
  } else {
     res->SetDefined(false);
  }
  return 0;
}

int Invert_Int9M_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Int9M* a = (Int9M*) args[0].addr;
  Int9M* res = (Int9M*) result.addr;
  if(a->IsDefined()){
    res->Equalize(a);
    res->Invert();
  } else {
    res->SetDefined(false);
  }
  return 0;
}

int Union_Int9M_Int9M_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Int9M* a = (Int9M*) args[0].addr;
  Int9M* b = (Int9M*) args[1].addr;
  Int9M* res = (Int9M*) result.addr;
  if(a->IsDefined() && b->IsDefined()){
    res->Equalize(a);
    res->Union(b);
  } else {
    res->SetDefined(false);
  }
  return 0;
}

int MultiIntersection_Int9M_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Int9M* a = (Int9M*) args[0].addr;
  Int9M* res = (Int9M*) result.addr;
  if(a->IsDefined()){
    res->Equalize(a);
    int sons = qp->GetNoSons(s);
    for(int i=1;i<sons;i++){
      a = static_cast<Int9M*>(args[i].addr);
      if(!a->IsDefined()){
         res->SetDefined(false);
         return 0;
      }
      res->Intersection(a);
    }
  } else {
     res->SetDefined(false);
  }
  return 0;
}

int Intersection_Int9M_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Int9M* a = (Int9M*) args[0].addr;
  Int9M* b = (Int9M*) args[1].addr;
  Int9M* res = (Int9M*) result.addr;
  if(a->IsDefined() && b->IsDefined()){
    res->Equalize(a);
    res->Intersection(b);
  } else {
    res->SetDefined(false);
  }
  return 0;
}

int Disjoint_Cluster_Cluster_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Cluster* b = (Cluster*) args[1].addr;
  CcBool* res = (CcBool*) result.addr;
  if(a->IsDefined() && a->IsDefined()){
     res->Set(true,a->Disjoint(b));
  } else {
     res->SetDefined(false);
  }
  return 0;
}

int PWDisjoint_Cluster_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
   result = qp->ResultStorage(s);
   int sons = qp->GetNoSons(s);
   Cluster All;
   All.MakeEmpty();
   CcBool* res = static_cast<CcBool*>(result.addr);

   bool resvalue = true;
   for(int i=0;i<sons && resvalue ;i++){
      Cluster* c = static_cast<Cluster*>(args[i].addr);
      if(!c->IsDefined()){
         res->SetDefined(false);
         return 0;
      }
      if(All.Intersects(c)){
         resvalue=false;
      }
      All.Union(c);
   }
   res->Set(true,resvalue);
   return 0;
}


int Add_Fun(Word* args, Word& result, int message,
            Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Int9M* b = (Int9M*) args[1].addr;
  Cluster* res = (Cluster*) result.addr;
  if(a->IsDefined() && b->IsDefined()){
    unsigned short number = b->GetNumber();
    res->Equalize(a);
    res->SetValueAt(number,true);
  } else {
     res->SetDefined(false);
  }
  return 0;
}

int Remove_Fun(Word* args, Word& result, int message,
            Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Int9M* b = (Int9M*) args[1].addr;
  Cluster* res = (Cluster*) result.addr;
  if(a->IsDefined() && b->IsDefined()){
    unsigned short number = b->GetNumber();
    res->Equalize(a);
    res->SetValueAt(number,false);
  } else {
    res->SetDefined(false);
  }
  return 0;
}

int NumberOf_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Int9M* a = (Int9M*) args[0].addr;
  CcInt* res = (CcInt*) result.addr;
  if(a->IsDefined()){
     res->Set(true,a->GetNumber());
  } else {
     res->SetDefined(false);
  }
  return 0;
}

int NameOf_Fun(Word* args, Word& result, int message,
               Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  CcString* res = (CcString*) result.addr;
  if(a->IsDefined()){
     STRING_T name;
     a->GetName(name);
     res->Set(true,name);
  } else {
     res->SetDefined(false);
  }
  return 0;
}

int Rename_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  CcString* b = (CcString*) args[1].addr;
  Cluster* res = (Cluster*) result.addr;
  if(a->IsDefined() && b->IsDefined()){
    res->Equalize(a);
    res->SetName(b->GetStringval());
  } else {
     res->SetDefined(false);
  }
  return 0;
}

int Contains_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Int9M* b = (Int9M*) args[1].addr;
  CcBool* res = (CcBool*) result.addr;
  if(a->IsDefined() && b->IsDefined()){
     res->Set(true,a->ValueAt(b->GetNumber()));
  } else {
     res->SetDefined(false);
  }
  return 0;
}

int Union_Cluster_Cluster_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Cluster* b = (Cluster*) args[1].addr;
  Cluster* res = (Cluster*) result.addr;
  if(a->IsDefined() && b->IsDefined()){
    res->Equalize(a);
    res->Union(b);
  } else {
     res->SetDefined(false);
  }
  return 0;
}

int MultiIntersection_Cluster_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Cluster* res = (Cluster*) result.addr;
  if(!a->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  res->Equalize(a);
  int sons = qp->GetNoSons(s);
  for(int i=1;i<sons;i++){
      a = static_cast<Cluster*>(args[i].addr);
      if(!a->IsDefined()){
        res->SetDefined(false);
        return 0;
      }
      res->Intersection(a);
  }
  return 0;
}

int Intersection_Cluster_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Cluster* b = (Cluster*) args[1].addr;
  Cluster* res = (Cluster*) result.addr;
  if(a->IsDefined() && b->IsDefined()){
     res->Equalize(a);
     res->Intersection(b);
  } else {
     res->SetDefined(false);
  }
  return 0;
}

int Minus_Cluster_Cluster_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Cluster* b = (Cluster*) args[1].addr;
  Cluster* res = (Cluster*) result.addr;
  if(a->IsDefined() && b->IsDefined()){
    res->Equalize(a);
    res->Minus(b);
  } else {
     res->SetDefined(false);
  }
  return 0;
}

int Invert_Cluster_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Cluster* res = (Cluster*) result.addr;
  if(a->IsDefined()){
    res->Equalize(a);
    res->Invert();
  } else {
    res->SetDefined(false);
  }
  return 0;
}

template<class T>
int Restrict_Cluster_T_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
    result = qp->ResultStorage(s);
    Cluster* cluster = (Cluster*) args[0].addr;
    T* cond = (T*) args[1].addr;
    Cluster* res = (Cluster*) result.addr;

    if(cluster->IsDefined() && cond->IsDefined()){
       string cond_s = cond->GetValue();
       res->Equalize(cluster);
       res->Restrict(cond_s);
    }else {
       res->SetDefined(false);
    }
    return 0;
}

template<class T>
int Relax_Cluster_T_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
    result = qp->ResultStorage(s);
    Cluster* cluster = (Cluster*) args[0].addr;
    T* cond = (T*) args[1].addr;
    Cluster* res = (Cluster*) result.addr;

    if(cluster->IsDefined() && cond->IsDefined()){
       string cond_s = cond->GetValue();
       res->Equalize(cluster);
       res->Relax(cond_s);
    } else {
       res->SetDefined(false);
    }
    return 0;
}


int CreatePGroup_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  PredicateGroup* res = (PredicateGroup*) result.addr;
 //get the number of arguments
 int arg_count = qp->GetNoSons(s);
 res->MakeEmpty();
 bool error = false;
 for(int i=0;i<arg_count && ! error; i++){
    Cluster* c = static_cast<Cluster*>(args[i].addr);
    if(!c->IsDefined()){
       res->SetDefined(false);
       return false;
    }
    error = !res->Add(c);
 }
 if(error){
   res->SetDefined(false);
   return 0;
 } else {
   res->SetDefined(true);
 }
 return 0;
}

int CreatePriorityPGroup_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){

   result = qp->ResultStorage(s);
  PredicateGroup* res = (PredicateGroup*) result.addr;
  //get the number of arguments
   int arg_count = qp->GetNoSons(s);
   res->MakeEmpty();
   bool error = false;
   for(int i=0;i<arg_count && ! error; i++){
    Cluster* c = static_cast<Cluster*>(args[i].addr);
    if(!c->IsDefined()){
       res->SetDefined(false);
       return 0;
    }
    error = !res->AddWithPriority(c);
   }
   if(error){
     res->SetDefined(false);
     return 0;
   } else {
     res->SetDefined(true);
   }
 return 0;
}

int CreateValidPGroup_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  PredicateGroup* res = (PredicateGroup*) result.addr;
  Cluster* Valid = (Cluster*)args[0].addr;

  if(!Valid->IsDefined()){
     res->SetDefined(false);
     return 0;
  }

   //get the number of arguments
   int arg_count = qp->GetNoSons(s);
   res->MakeEmpty();
   bool error = false;
   Cluster* Current;
   for(int i=1;i<arg_count && ! error; i++){
      Current = (Cluster*) args[i].addr;
      if(!Current->IsDefined()){
        res->SetDefined(false);
        return 0;
      }
      Current->Intersection(Valid);
      error = !res->Add(Current);
   }
   if(error){
     res->SetDefined(false);
     return 0;
   } else {
     res->SetDefined(true);
   }
   return 0;
}

int ClusterNameOf_Fun(Word* args, Word& result, int message,
                Word& local, Supplier s){
    result = qp->ResultStorage(s);
    PredicateGroup* PG = (PredicateGroup*) args[0].addr;
    Int9M* IM = (Int9M*) args[1].addr;
    CcString* res = (CcString*) result.addr;
    if(PG->IsDefined() && IM->IsDefined()){
      STRING_T name;
      PG->GetNameOf(IM, name);
      res->Set(true, name);
    } else {
      res->SetDefined(false);
    }
    return 0;
}

int ClusterOf_Fun(Word* args, Word& result, int message,
                Word& local, Supplier s){
    result = qp->ResultStorage(s);
    PredicateGroup* PG = (PredicateGroup*) args[0].addr;
    Int9M* IM = (Int9M*) args[1].addr;
    Cluster* res = (Cluster*) result.addr;

    if(!IM->IsDefined() || !PG->IsDefined()){
        res->SetDefined(false);
        return 0;
    }
    Cluster* tmp = PG->GetClusterOf(*IM);
    if(!tmp){
       res->SetDefined(false);
       return 0;
    }
    res->Equalize(tmp);
    delete tmp;
    return 0;
}

int GetCluster_Fun(Word* args, Word& result, int message,
                Word& local, Supplier s){
    result = qp->ResultStorage(s);
    PredicateGroup* PG = static_cast<PredicateGroup*>(args[0].addr);
    CcString* name = static_cast<CcString*>(args[1].addr);
    Cluster* res = static_cast<Cluster*>(result.addr);

    if(!PG->IsDefined() || !name->IsDefined()){
       res->SetDefined(false);
       return 0;
    }

    const STRING_T* str = name->GetStringval();

    Cluster* tmp = PG->GetClusterOf(str);

    if(!tmp){ // name not found
       res->SetDefined(false);
       return 0;
    }
    res->Equalize(tmp);
    delete tmp;
    return 0;
}

template<class T>
int SizeOf_T_Fun(Word* args, Word& result, int message,
                Word& local, Supplier s){
    result = qp->ResultStorage(s);
    T* arg = (T*) args[0].addr;
    CcInt* res = static_cast<CcInt*>(result.addr);
    if(arg->IsDefined()){
       res->Set(true,arg->Size());
    } else {
       res->SetDefined(false);
    }
    return 0;
}

template<class T>
int CreateCluster_T_Fun(Word* args, Word& result, int message,
                Word& local, Supplier s){
   result = qp->ResultStorage(s);
   Cluster* res = (Cluster*) result.addr;
   CcString* arg1 = (CcString*) args[0].addr; // name
   T*  arg2 = (T*) args[1].addr;              // description

   if(!arg1->IsDefined() || !arg2->IsDefined()){
      res->SetDefined(false);
      return 0;
   }

   string cond = arg2->GetValue();
   struct tree* t=0;
   if(!parseString(cond.c_str(),&t)){
      char* Emsg = GetLastMessage();
      ErrorReporter::ReportError(string(Emsg)+"\n");
      free(Emsg);
      ((Cluster*)result.addr)->SetDefined(false);
      return 0;
   }
   // parsing successful
   res->SetName(arg1->GetStringval());
   res->MakeEmpty();
   for(unsigned short i=0;i<512;i++){
      if(evalTree(t,i)){
          res->SetValueAt(i,true);
       }
   }
   destroyTree(t);
   return 0;
}

int IsComplete_Fun(Word* args, Word& result, int message,
                Word& local, Supplier s){
   result = qp->ResultStorage(s);
   PredicateGroup* arg = (PredicateGroup*) args[0].addr;
   CcBool* res = static_cast<CcBool*>(result.addr);
   if(arg->IsDefined()){
       res->Set(true,arg->IsComplete());
   } else {
       res->SetDefined(false);
   }
   return 0;
}


int UnSpecified_Fun(Word* args, Word& result, int message,
                Word& local, Supplier s){
   result = qp->ResultStorage(s);
   PredicateGroup* PG = (PredicateGroup*) args[0].addr;
   Cluster* res = static_cast<Cluster*>(result.addr);
   if(PG->IsDefined()){
        res->Equalize(PG->GetUnspecified());
   } else {
        res->SetDefined(false);
   }
   return 0;
}


int StdPGroup_Fun(Word* args, Word& result, int message,
                Word& local, Supplier s){
     result= qp->ResultStorage(s);
     ((PredicateGroup*) result.addr)->SetToDefault();
     return 0;
}


template <class A>
int TopRelEqualsFun(Word* args, Word& result, int message,
                    Word& local, Supplier s){
   result = qp->ResultStorage(s);
   A* a1 = (A*) args[0].addr;
   A* a2 = (A*) args[1].addr;
   CcBool* res = (CcBool*) result.addr;
   if(!a1->IsDefined() || !a2->IsDefined()){
      res->Set(false,false);
   } else {
      res->Set(true, (*a1)==(*a2));
   }
   return 0;
}



/*
7.3 Specification of operators

*/
const string TransposeSpec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example 1\""
      " \"Example 2\" )"
      " ( <text> int9m -> int9m  | cluster x string -> cluster </text--->"
      " \" transpose(_ [,_] ) \" "
      " <text>returns the transposed matrix, or "
      "the renamed cluster where all contained matrices"
      " was transposed</text---> "
      " \"query transpose(IM) ;\""
      " <text>let contains=transpose(inside,\"contains\");</text---> ))";

const string InvertSpec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \" t -> t , t in {int9m,cluster}\""
      " \" invert(_) \" "
      "  \"returns the inverted element \" "
      "  \" query invert(IM)\" ))";

const string UnionSpec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( <text>type_i x type_i -> type_i ,"
      " where type_i in {int9m,cluster} </text---> "
      " \" _ union _ \" "
      "  \" union function \" "
      "  \" query i1 union i2 \" ))";

const string IntersectionSpec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( <text>type_i x type_i -> type_i ,"
      " where type_i in {int9m,cluster} </text---> "
      " \" _ intersection  _ \" "
      "  \" intersection function \" "
      "  \" query i1 intersection i2 \" ))";

const string MultiIntersectionSpec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( <text>t x t x ... xt -> t,"
      " where t in {int9m, cluster} </text---> "
      " \" multiintersection (_,_,...) \" "
      "  \" computes the subset included in all arguments \" "
      "  \" query multiintersection(c1,c2,c3) \" ))";

const string AddSpec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \"cluster x int9m -> cluster\""
      " \" _ + _ \" "
      "  \"inserts a new int9m to the cluster \" "
      "  \" query c + i \" ))";

const string RemoveSpec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \"cluster x int9m -> cluster\""
      " \" _ - _ \" "
      "  \"removes  a int9m from the cluster \" "
      "  \" query c - i \" ))";

const string MinusSpec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \"cluster x cluster -> cluster\""
      " \" _ - _ \" "
      "  <text>removes  the elements of o cluster from other one </text---> "
      "  \" query c1 - c2 \" ))";

const string NumberOfSpec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \"int9m -> int\""
      " \" number_of(_) \" "
      "  \"returns the matrix number \" "
      "  \" query number_of(i) \" ))";

const string NameOfSpec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \"cluster -> string\""
      " \" name_of(_) \" "
      "  \"returns the name of this cluster \" "
      "  \" query name_of(c) \" ))";

const string RenameSpec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \"cluster x string -> cluster\""
      " \" _ renamecluster[ _ ] \" "
      "  \"changes the name of a cluster \" "
      "  \" query c renamecluster[\"inside\"] \" ))";

const string ContainsSpec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \"cluster x int9m -> bool\""
      " \" _ contains _ \" "
      "  <text>checks whether the second element"
      " is member of the first one </text---> "
      "  \" query c contains i \" ))";

const string DisjointSpec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \"cluster x cluster -> bool\""
      " \" _ disjoint _ \" "
      "  \"checks whether the clusters are disjoint\" "
      "  \" query c1 disjoint c2 \" ))";

const string CreatePGroupSpec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \"cluster x cluster x ... -> predicategroup\""
      " \" createpgroup( _, _, ... ) \" "
      "  \"creates a predicate group from existing clusters\" "
      "  \" let pc1 = createpgroup(c1, c2, c3) \" ))";

const string CreatePriorityPGroupSpec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \"cluster x cluster x ... -> predicategroup\""
      " \" createprioritypgroup( _, _, ... ) \" "
      "  <text>creates a predicate group from existing clusters "
      " If Clusters are overlapping, the common matrixes are "
      " assigned to the  attribute located prior in the "
      "arglist </text--->"
      "  \" let pc1 = createprioritypgroup(c1, c2, c3) \" ))";

const string CreateValidPGroupSpec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \"cluster x cluster x ... -> predicategroup\""
      " \" createvalidpgroup( _ _ ... ) \" "
      " <text>creates a predicate group from existing clusters  "
      " The first cluster defines all valid matrices </text--->"
      "  \" let pc1 = createvalidpgroup(valid, c1, c2, c3) \" ))";

const string ClusterNameOf_pc_m_Spec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \"predicategroup x int9m -> string\""
      " \"  clustername_of(_ , _) \" "
      "  <text>returns the name of the matrix in the given "
      " predicategroup </text---> "
      "  <text>query clustername_of(StdPG,m1)="
      "\"inside\" </text---> ))";

const string ClusterOf_pc_m_Spec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \"predicategroup x int9m -> cluster\""
      " \"  clusterof(_ , _) \" "
      "  <text>returns the predicate group containing the matrix</text---> "
      "  <text>query clusterof(StdPG,m1) </text---> ))";

const string SizeOf_Spec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \" {cluster, predicategroup} -> int \""
      " \"  sizeof(_)\" "
      "  \"returns the number of elements in the argument\" "
      "  <text>query sizeof(p) </text---> ))";

const string CreateCluster_Spec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \" string x {text, string} -> cluster  \""
      " \"  createcluster(_ , _)\" "
      "  <text>creates an cluster with given name and condition</text---> "
      "  <text>query createcluster(\"test\",\"ii & !ee\") </text---> ))";

const string IsComplete_Spec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \" predicategroup -> bool  \""
      " \"  isComplete(_) \" "
      " <text>checks whether the clusters contained in the argument   "
      " cover all 512 possible matrices </text--->"
      " \" query isComplete(stdpg) \"))";

const string UnSpecified_Spec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \" predicategroup -> cluster  \""
      " \"  unspecified(_) \" "
      " <text>returns the cluster containing all"
      "  non-used matrices  </text---> "
      " \" query unspecified(stdpg) \"))";


const string PWDisjoint_Spec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \" cluster x cluster x ... -> bool  \""
      " \" pwdisjoint(_,_,...)  \" "
      " <text>checks whether the arguments are pairwise"
      " disjoint  </text---> "
      " \" query pwdisjoint(c1,c2,c3) \"))";


const string Restrict_Spec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \" cluster x {string, text} -> cluster  \""
      " \" restrict(_, _)  \" "
      " <text>restricts the arguments cluster to such matrices fulfilling"
      " the condition of the second argument  </text---> "
      " <text> query restrict(c1, \"ee & ii\" )</text--->))";


const string Relax_Spec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \" cluster x {string, text} -> cluster  \""
      " \" relax(_, _)  \" "
      " <text>add all matrices fulfilling "
      " the condition of the second argument to the cluster </text---> "
      " <text> query relax(c1,\" ee & ii\") </text--->))";

const string StdPGroup_Spec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \"  -> predicategroup  \""
      " \" predicategroup()  \" "
      " <text>Computes a default predicategroup.</text---> "
      " <text> query stdpgroup()  </text--->))";


const string TopRelEqualsSpec =
    "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    " ( <text>t x t -> bool , t in {int9m, cluster, predicategroup}</text--->"
    " \" _ = _  \" "
    " <text>Check for equality</text---> "
    " <text> query [const int9m value 3] = [const int9m value 4] "
    "</text--->))";


const string GetClusterSpec =
    "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    " ( <text>predicategroup x string -> cluster</text--->"
    " \" clusterof(group,name)  \" "
    " <text>returns the cluster in group specified by name</text---> "
    " <text> query clusterof(std,\"equals\") "
    "</text--->))";

/*

This specification is for debugging only.

*/
const string STD_Spec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \" ---signature---  \""
      " \"  ---syntax---  \" "
      " \" ---meaning---\""
      " \" ---example--- \"))";

/*
7.4 Value Mapping Array for overloaded operators

*/
ValueMapping TransposeMap[] = { Transpose_Int9M_Fun,
                                Transpose_Cluster_Fun };
ValueMapping UnionMap[] = { Union_Int9M_Int9M_Fun,
                            Union_Cluster_Cluster_Fun };
ValueMapping InvertMap[] = { Invert_Int9M_Fun, Invert_Cluster_Fun };
ValueMapping MultiIntersectionMap[] = { MultiIntersection_Int9M_Fun,
                                   MultiIntersection_Cluster_Fun };

ValueMapping IntersectionMap[] = { Intersection_Int9M_Fun,
                                   Intersection_Cluster_Fun };

ValueMapping SizeOfMap[] = { SizeOf_T_Fun<Cluster>,
                             SizeOf_T_Fun<PredicateGroup>};

ValueMapping CreateClusterMap[] = { CreateCluster_T_Fun<CcString>,
                                    CreateCluster_T_Fun<FText>};

ValueMapping RestrictMap[] = { Restrict_Cluster_T_Fun<CcString>,
                               Restrict_Cluster_T_Fun<FText>};

ValueMapping RelaxMap[] = { Relax_Cluster_T_Fun<CcString>,
                            Relax_Cluster_T_Fun<FText>};

ValueMapping TopRelEqualsMap[] = { TopRelEqualsFun<Int9M>,
                                   TopRelEqualsFun<Cluster>,
                                   TopRelEqualsFun<PredicateGroup>};
/*
7.6 Selection Functions

*/
static int TransposeSelect(ListExpr args){
   if(nl->IsEqual(nl->First(args),"int9m"))
      return 0;
   if(nl->IsEqual(nl->First(args),"cluster") &&
      nl->IsEqual(nl->Second(args),CcString::BasicType()))
      return 1;
   return -1; // should never be reached
}

static int SetOpsSelect(ListExpr args){
   if(nl->IsEqual(nl->First(args),"int9m"))
      return 0;
   if(nl->IsEqual(nl->First(args),"cluster"))
      return 1;
   return -1; // should never be reached
}

static int InvertSelect(ListExpr args){
   if(nl->IsEqual(nl->First(args),"int9m"))
      return 0;
   if(nl->IsEqual(nl->First(args),"cluster"))
      return 1;
   return -1; // should never be reached
}

static int SizeOfSelect(ListExpr args){
   if(nl->IsEqual(nl->First(args),"cluster"))
      return 0;
   if(nl->IsEqual(nl->First(args),"predicategroup"))
      return 1;
   return -1; // should never be reached
}

static int CreateClusterSelect(ListExpr args){
   if(nl->IsEqual(nl->Second(args),CcString::BasicType()))
      return 0;
   else
      return 1;
}

static int RestrictSelect(ListExpr args){
   if(nl->IsEqual(nl->Second(args),CcString::BasicType()))
      return 0;
   else
      return 1;
}

static int RelaxSelect(ListExpr args){
   if(nl->IsEqual(nl->Second(args),CcString::BasicType()))
      return 0;
   else
      return 1;
}

static int TopRelEqualsSelect(ListExpr args){
  string a1 = nl->SymbolValue(nl->First(args));
  if(a1=="int9m") return 0;
  if(a1=="cluster") return 1;
  if(a1=="predicategroup") return 2;
  return -1;
}

/*
7.7 Definition of operators

*/
Operator toprel_equals(
         "=",     // name
         TopRelEqualsSpec,   // specification
         3,               // number of functions
         TopRelEqualsMap,    // array of value mappings
         TopRelEqualsSelect,
         TopRelEqualsTM);

Operator transpose(
         "transpose",     // name
         TransposeSpec,   // specification
         2,               // number of functions
         TransposeMap,    // array of value mappings
         TransposeSelect,
         TransposeTM);

Operator union_op(
         "union",     // name
         UnionSpec,   // specification
         2,               // number of functions
         UnionMap,    // array of value mappings
         SetOpsSelect,
         SetOpsTM);

Operator multiintersection(
         "multiintersection",     // name
         MultiIntersectionSpec,   // specification
         2,               // number of functions
         MultiIntersectionMap,    // array of value mappings
         SetOpsSelect,
         MultiSetOpsTM);

Operator intersection(
         "intersection",     // name
         IntersectionSpec,   // specification
         2,               // number of functions
         IntersectionMap,    // array of value mappings
         SetOpsSelect,
         SetOpsTM);

Operator invert(
         "invert",     // name
        InvertSpec,   // specification
         2,               // number of functions
         InvertMap,    // array of value mappings
         InvertSelect,
         InvertTM);

Operator sizeof_op(
         "size",     // name
         SizeOf_Spec,   // specification
         2,               // number of functions
         SizeOfMap,    // array of value mappings
         SizeOfSelect,
         SizeOfTM);

Operator restrict_op(
         "restrict",     // name
         Restrict_Spec,   // specification
         2,               // number of functions
         RestrictMap,    // array of value mappings
         RestrictSelect,
         RestrictRelaxTM);

Operator relax_op(
         "relax",     // name
         Relax_Spec,   // specification
         2,               // number of functions
         RelaxMap,    // array of value mappings
         RelaxSelect,
         RestrictRelaxTM);

Operator createcluster(
         "createcluster",     // name
         CreateCluster_Spec,   // specification
         2,               // number of functions
         CreateClusterMap,    // array of value mappings
         CreateClusterSelect,
         CreateClusterTM);

Operator add(
         "+", // name
         AddSpec, // specification
         Add_Fun,
         Operator::SimpleSelect,
         Cluster_Int9M_Cluster);

Operator pwdisjoint(
         "pwdisjoint", // name
         PWDisjoint_Spec, // specification
         PWDisjoint_Cluster_Fun,
         Operator::SimpleSelect,
         PWDisjointTM);

Operator getcluster(
         "getcluster", // name
         GetClusterSpec, // specification
         GetCluster_Fun,
         Operator::SimpleSelect,
         GetClusterTM);

Operator unspecified(
         "unspecified", // name
         UnSpecified_Spec, // specification
         UnSpecified_Fun,
         Operator::SimpleSelect,
         UnSpecifiedTM);

Operator iscomplete(
         "isComplete", // name
         IsComplete_Spec, // specification
         IsComplete_Fun,
         Operator::SimpleSelect,
         IsCompleteTM);

Operator minus(
         "-", // name
         MinusSpec, // specification
         Minus_Cluster_Cluster_Fun,
         Operator::SimpleSelect,
         Cluster_Cluster_Cluster);

Operator remove(
         "-", // name
         RemoveSpec, // specification
         Remove_Fun,
         Operator::SimpleSelect,
         Cluster_Int9M_Cluster);

Operator number_of(
         "number_of", // name
         NumberOfSpec, // specification
         NumberOf_Fun,
         Operator::SimpleSelect,
         Int9M_Int);

Operator name_of(
         "name_of", // name
         NameOfSpec, // specification
         NameOf_Fun,
         Operator::SimpleSelect,
         Cluster_String);

Operator rename(
         "renamecluster", // name
         RenameSpec, // specification
         Rename_Fun,
         Operator::SimpleSelect,
         Cluster_String_Cluster);

Operator contains(
         "contains", // name
         ContainsSpec, // specification
         Contains_Fun,
         Operator::SimpleSelect,
         Cluster_Int9M_Bool);

Operator disjoint(
         "disjoint", // name
         DisjointSpec, // specification
         Disjoint_Cluster_Cluster_Fun,
         Operator::SimpleSelect,
         Cluster_Cluster_Bool);

Operator createpgroup(
         "createpgroup", // name
         CreatePGroupSpec, // specification
         CreatePGroup_Fun,
         Operator::SimpleSelect,
         CreatePGroupTM);

Operator createprioritypgroup(
         "createprioritypgroup", // name
         CreatePriorityPGroupSpec, // specification
         CreatePriorityPGroup_Fun,
         Operator::SimpleSelect,
         CreatePGroupTM);

Operator createvalidpgroup(
         "createvalidpgroup", // name
         CreateValidPGroupSpec, // specification
         CreateValidPGroup_Fun,
         Operator::SimpleSelect,
         CreateValidPGroupTM);

Operator clustername_of(
         "clustername_of", // name
         ClusterNameOf_pc_m_Spec, // specification
         ClusterNameOf_Fun,
         Operator::SimpleSelect,
         ClusterName_OfTM);

Operator clusterof(
         "clusterof", // name
         ClusterOf_pc_m_Spec, // specification
         ClusterOf_Fun,
         Operator::SimpleSelect,
         ClusterOfTM);

Operator stdpgroup(
         "stdpgroup", // name
         StdPGroup_Spec, // specification
         StdPGroup_Fun,
         Operator::SimpleSelect,
         StdPGroup_TM);

} // namespace toprel

/*
STream operators

*/

ostream& operator<<(ostream& o, const toprel::Int9M& p){
    o << p.ToString();
    return o;
}


ostream& operator<<(ostream& o, const toprel::Cluster& c){
   o << c.ToString();
   return o;
}

ostream& operator<<(ostream& o, const toprel::PredicateGroup& p){
   o << p.ToString();
   return o;
}

/*
7 Creating the Algebra

*/
class TopRelAlgebra : public Algebra
{
 public:
  TopRelAlgebra() : Algebra()
  {
    AddTypeConstructor( &toprel::int9m );
    AddTypeConstructor( &toprel::cluster);
    AddTypeConstructor( &toprel::predicategroup);

    toprel::int9m.AssociateKind(Kind::DATA());
    toprel::cluster.AssociateKind(Kind::DATA());
    toprel::predicategroup.AssociateKind(Kind::DATA());

    AddOperator(&toprel::invert);
    AddOperator(&toprel::union_op);
    AddOperator(&toprel::intersection);
    AddOperator(&toprel::multiintersection);
    AddOperator(&toprel::add);
    AddOperator(&toprel::number_of);
    AddOperator(&toprel::remove);
    AddOperator(&toprel::rename);
    AddOperator(&toprel::name_of);
    AddOperator(&toprel::contains);
    AddOperator(&toprel::disjoint);
    AddOperator(&toprel::minus);
    AddOperator(&toprel::createpgroup);
    AddOperator(&toprel::createvalidpgroup);
    AddOperator(&toprel::createprioritypgroup);
    AddOperator(&toprel::clustername_of);
    AddOperator(&toprel::clusterof);
    AddOperator(&toprel::transpose);
    AddOperator(&toprel::sizeof_op);
    AddOperator(&toprel::createcluster);
    AddOperator(&toprel::iscomplete);
    AddOperator(&toprel::unspecified);
    AddOperator(&toprel::pwdisjoint);
    AddOperator(&toprel::restrict_op);
    AddOperator(&toprel::relax_op);
    AddOperator(&toprel::stdpgroup);
    AddOperator(&toprel::toprel_equals);
    AddOperator(&toprel::getcluster);
  }
  ~TopRelAlgebra() {};
};


void InitializeTranspArray(){
   toprel::Int9M m(0);
   for(int i=0;i<512;i++){
      m.SetValue(i);
      m.TransposeSlow();
      TranspArray[i] = m.GetNumber();
   }

}


/*
8 Initialization of the Algebra


*/
extern "C"
Algebra*
InitializeTopRelAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  InitializeTranspArray();
  return (new TopRelAlgebra());
}

