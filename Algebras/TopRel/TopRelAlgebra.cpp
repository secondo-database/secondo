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
#include "StandardAttribute.h"
#include "StandardTypes.h"
#include "DBArray.h"
#include "LogMsg.h"

#include "FTextAlgebra.h"

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

// The name of the unspecified cluster
static const STRING UNSPECIFIED = "unspecified";

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
             const bool EI, const bool EB, const bool EE){
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
   defined = true;
}

/*
2.1.3 ToListExpr

This function returns the NestedList representation of a
9 intersection matrix.

*/
ListExpr Int9M::ToListExpr() const {
  if(!defined)
    return nl->SymbolAtom("undefined");
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
bool Int9M::ReadFrom(const ListExpr LE){
   // case uf undefined
   if(nl->IsEqual(LE,"undefined")){
       defined = false;
       return true;
   }

   if(nl->AtomType(LE)==IntType){
      int v = nl->IntValue(LE);
      if(v<0) return false;
      if(v>511) return false;
      value = (unsigned short) v;
      defined = true;
      return true;
   }

   if(nl->ListLength(LE)!=9)
      return false;
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
      else // not an allowed type
         return false;
      if(b)
         tmp = tmp | pos;
      pos = pos/2;
      r = nl->Rest(r);
   }
   value = tmp;
   defined = true;
   return true;
}

/*
2.1.5 Equalize functions

By calling the Equalize function, the matrix gets its value from
the argument.

*/
void Int9M::Equalize(const Int9M value){
      Equalize(&value);
}

void Int9M::Equalize(const Int9M* v){
    this->value = v->value;
    this->defined = v->defined;
}

/*
2.1.6 Compare function

This functions compares this matrix with arg. arg must be of type
Int9M.

*/
int Int9M::Compare(const Attribute* arg) const{
    Int9M* v = (Int9M*) arg;
    if(!defined && !v->defined)
        return 0;
    if(!defined)
       return -1;
    if(!v->defined)
       return 1;
    unsigned short pos=1;
    for(int i=0;i<9;i++){
       if(value&pos && !v->value&pos)
          return 1;
       if(!value&pos && v->value&pos)
          return -1;
       pos = pos*2;
    }
    return 0;
}

/*
2.1.9 IsDefined

*/
bool Int9M::IsDefined() const{ return defined; }


/*
2.1.10 SetDefined

*/
void Int9M::SetDefined( bool defined ){
   this->defined=defined;
}

/*
2.1.11 HashValue

*/
size_t Int9M::HashValue() const{
   if(!defined)
      return (size_t) 512;
   else
      return (size_t) value;
}

/*
2.1.12 CopyFrom

*/
void Int9M::CopyFrom(const StandardAttribute* arg){
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
2.1.15 Equal Operator

*/
bool Int9M::operator==(const Int9M I2) const{
   return CompareTo(I2)==0;
}


/*
2.2  Definitions for the Cluster Type

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
void Cluster::SetValueAt(const int pos,const bool value,
                         unsigned char bitvector[])const{
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

void Cluster::SetValueAt(const int pos, const bool value){
    SetValueAt(pos,value,BitVector);
}


/*
2.2.2 The Transpose function

The transpose function modifies this cluster, so that after calling
this function, the cluster contains the transposed matrices. This can be used
to define new cluster from existining ones e.g. a ~contains~ cluster from a
~covered-by~ one.

*/
void Cluster::Transpose(){
   Cluster TMP(0);
   unsigned short number = 0;
   Int9M currentM(number);
   int count = 0;
   for(int i=0;i<512;i++){
      if(ValueAt(i)){
         count++;
         currentM.SetToNumber(i);
         currentM.Transpose();
         TMP.SetValueAt(currentM.GetNumber(),true);
      }
   }
  memcpy(BitVector,TMP.BitVector,64);
}



/*
2.2.2 The ToListExpr Function

This function converts the Cluster into a representation in nested list format.
The ListRepresentation is a list containing the name as string atom at the first
position followed by all matrix numbers of the
contained 9 intersection matrices.

*/
ListExpr Cluster::ToListExpr()const{
    if(!defined)
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
bool Cluster::ReadFrom(const ListExpr LE){

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
            SetValueAt(i,true);
          else
            SetValueAt(i,false);
      }
      strcpy(name,nl->StringValue(nl->First(LE)).c_str());
      destroyTree(T);
      return true;
   }

   /*
   The second case is a list containing valid matrices.
   */
   unsigned char TMP[64];
   STRING TMPname;
   // initialize the temporary bitvector
   //for(int i=0;i<64;i++)
   //   TMP[i]=0;
   memcpy(TMP,emptyBlock,64);

   bool correct = true;
   ListExpr scan = LE;
   ListExpr elem;
   // first, get the name of this cluster
   if(nl->ListLength(scan)<1)
       return false;
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
       if(!current.ReadFrom(elem))
          correct = false;
       else{
          unsigned short v = current.GetNumber();
          SetValueAt(v,true,TMP);
       }
   }

   if(!correct)
      return false;
   memcpy(BitVector,TMP,64);
   strcpy(name,TMPname);
   defined = true;
   return true;
}

/*
2.2.4 Equalize functions

Aided by this functions the value of this cluster can be setted to the
value of another existing cluster.

*/
void Cluster::Equalize(const Cluster value){

     Equalize(&value);
}

void Cluster::Equalize(const Cluster* value){
     strcpy(name, value->name);
     memcpy(BitVector,value->BitVector,64);
     defined = value->defined;
}

/*
2.2.5 Compare function

The Compare function compares two clusters. Needed for to be an attribute of a tuple.

*/
int Cluster::CompareTo(const Cluster* C)const{
   if(!defined && !C->defined)
     return 0;
   if(!defined)
      return -1;
   if(!C->defined)
      return 1;
   for(int i=0;i<64;i++){
      if(BitVector[i]<C->BitVector[i])
          return -1;
      if(BitVector[i]>C->BitVector[i])
          return 1;
   }
   if(name<C->name)
      return -1;
   if(name>C->name)
      return 1;
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
2.2.8 Functions manipulation the defined state

With this functions we can query and set the value of the
defined flag of this cluster.

*/
bool Cluster::IsDefined() const{
   return defined;
}

void Cluster::SetDefined( bool defined ){
   this->defined = defined;
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
void Cluster::CopyFrom(const StandardAttribute* arg){
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

bool Cluster::Restrict(string condition){

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
         SetValueAt(i,false);
      }
   }
   destroyTree(T);
   return true;
}

/*
2.2.13 Restrict

This version of ~Restrict~ removes all matrices having a different as the given
value at the specified position. The pos must be from the set [{]II,...,EE[}].
Otherwise the result will not be as expected. 


*/
 void Cluster::Restrict(const int pos, const bool value){
    for(int i=0;i<512;i++){
       if( ((i&pos) !=0) != value){
          SetValueAt(i,false);
       }  
    }
 }


/*
2.2.12 Relax


This function adds all matrices fulfilling the given condition to

*/

bool Cluster::Relax(string condition){

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
         SetValueAt(i,true);
      }
   }
   destroyTree(T);
   return true;
}



/*
2.3.12 Operators for comparisons between Clusters

*/
bool Cluster::operator==(const Cluster C2)const{
      return CompareTo(&C2)==0;
}

bool Cluster::operator<(const Cluster C2)const{
      return CompareTo(&C2)<0;
}

bool Cluster::operator>(const Cluster C2)const{
      return CompareTo(&C2)>0;
}

static int ClusterCompare(const void *a, const void *b){
   Cluster *ca = new ((void*)a) Cluster,
           *cb = new ((void*)b) Cluster;
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
     theClusters( size ),
     canDelete( false ),
     sorted(true),
     unSpecified(0)
     {
   // at this point, we defined a single
   // clusters 'unspecified containing all
   // possible 9 intersection matrices
   unSpecified.Invert();
   unSpecified.SetName(&UNSPECIFIED);
}

/*
2.3.2 The Equalize function

When calling this function, the value of this predicate group
is taken from the argument of this function.

*/
void PredicateGroup::Equalize(const PredicateGroup* PG){
    const Cluster *tmp;
    defined  = PG->defined;
    sorted = PG->sorted;
    canDelete = PG->canDelete;
    int size = PG->theClusters.Size();
    theClusters.Resize(size);
    for(int i=0;i<size;i++){
        PG->theClusters.Get(i,tmp);
        theClusters.Put(i,*tmp);
    }
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
   PredicateGroup* PG = (PredicateGroup*) right;
   if(!defined && !PG->defined)
      return 0;
   if(!defined && PG->defined)
      return -1;
   if(defined && !PG->defined)
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
   const Cluster *tmp1, *tmp2;

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
       cmp = tmp1->Compare(tmp2);
       if(cmp != 0)
          return cmp;
   }
   return 0;
}

/*
2.3.4 ToListExpr

This function computes the external representation  of this
PredicateGroup in nested list format.

*/
ListExpr PredicateGroup::ToListExpr(){
  // we have at least one element in the this predicategroup
  if(!defined)
      return nl->SymbolAtom("undefined");
  const Cluster *C;
  if(theClusters.Size()==0)
     return nl->TheEmptyList();

  theClusters.Get(0,C);
  ListExpr res = nl->OneElemList(C->ToListExpr());
  ListExpr Last = res;
  for(int i=1;i<theClusters.Size();i++){
       theClusters.Get(i,C);
       Last = nl->Append(Last,C->ToListExpr());
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
bool PredicateGroup::ReadFrom(const ListExpr instance){
   int length = nl->ListLength(instance);
   /*
    The maximum count of non-overlapping, non-empty clusters is 512.
   */
   if(length>512){
      return false;
   }
   unSpecified.MakeEmpty();
   unSpecified.Invert();
   Cluster CurrentCluster(false);
   Cluster AllClusters[length];
   int pos =0;
   while(!nl->IsEmpty(instance)){
      // error in cluster representation
      if(!CurrentCluster.ReadFrom(instance))
         return false;
      // empty clusters are not allowed
      if(CurrentCluster.IsEmpty())
         return false;
      //  overlapping cluster found
      if(! unSpecified.Contains(&CurrentCluster))
         return  false;
      // check whether name already used
      const STRING* name = CurrentCluster.GetName();
      for(int i=0;i<pos;i++)
         if(strcmp(*(AllClusters[i].GetName()),*name)==0)
              return false;
      if(strcmp(*name,UNSPECIFIED)==0) // forbidden name
              return false;
      unSpecified.Minus(&CurrentCluster);
      AllClusters[pos].Equalize(CurrentCluster);
      pos++;
   }
   // if this state is reached, the list representation is correct
   defined = true;
   theClusters.Resize(length);
   for(int i=0;i<length;i++)
       theClusters.Put(i,(AllClusters[i]));
   theClusters.Sort(toprel::ClusterCompare);
   sorted=true;
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
   const STRING* name = C->GetName();

   if(strcmp(*name,UNSPECIFIED)==0){
      return false;
   }
   if(!unSpecified.Contains(C)){ // overlap with existing clusters
      return false;
   }
   // check for name conflicts with existing clusters
   const Cluster *tmp;
   for(int i=0;i<theClusters.Size();i++){
        theClusters.Get(i,tmp);
        if(strcmp(*(tmp->GetName()),*name)==0){
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
   if(C->IsEmpty())
       return false;
   const STRING* name = C->GetName();
   if(strcmp(*name,UNSPECIFIED)==0){ // name conflict
      return false;
   }
   // check for name conflicts with existing clusters
   const Cluster *tmp;
   for(int i=0;i<theClusters.Size();i++){
        theClusters.Get(i,tmp);
        if(strcmp(*(tmp->GetName()),*name)==0){
            return false;
        }
   }
   Cluster aux( *tmp );
   aux.Equalize(C);
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
Cluster* PredicateGroup::GetClusterOf(const STRING* name)const{
   if(strcmp((*name),UNSPECIFIED)==0){
     Cluster* res = new Cluster();
     res->Equalize(unSpecified);
     return res;
   }
   int size = theClusters.Size();
   const Cluster* tmp;
   for(int i=0;i<size;i++){
      theClusters.Get(i,tmp);
      const STRING* n;
      n  = tmp->GetName();
      if(strcmp(*n,*name)==0){
         Cluster* res = new Cluster();
         res->Equalize(tmp);
         return res;
      }
   }
   return NULL;
}


/*
4 Functions for the type constructors

4.1 Out Functions

*/
ListExpr OutInt9M(ListExpr TypeInfo, Word value){
   return ((Int9M*)value.addr)->ToListExpr();
}

ListExpr OutCluster(ListExpr TypeInfo, Word value){
   return ((Cluster*)value.addr)->ToListExpr();
}

ListExpr OutPredicateGroup(ListExpr TypeInfo, Word value){
   return ((PredicateGroup*)value.addr)->ToListExpr();
}

/*
4.2 In functions

*/
Word
InInt9M( const ListExpr typeInfo, const ListExpr instance,
        const int errorPos, ListExpr& errorInfo, bool& correct ){
  unsigned short number = 0;
  Int9M* res = new Int9M(number);
  if(res->ReadFrom(instance)){
     correct=true;
     return SetWord(res);
  }
  delete res;
  return SetWord(Address(0));
}

Word
InCluster(const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct){
   Cluster* res = new Cluster(false);
   nl->WriteListExpr(instance);
   if(res->ReadFrom(instance)){
      correct = true;
      return SetWord(res);
   }
   delete res;
   return SetWord(Address(0));
}

Word
InPredicateGroup(const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct){
   PredicateGroup* res = new PredicateGroup(1);
   if(res->ReadFrom(instance)){
      correct = true;
      return SetWord(res);
   }
   delete res;
   return SetWord(Address(0));
}

/*
4.2 Functions for the creation of objects

*/
Word
CreateInt9M( const ListExpr typeInfo )
{
  unsigned short number =0;
  return (SetWord( new Int9M(number)));
}

Word
CreateCluster( const ListExpr typeInfo){
   return (SetWord(new Cluster(false)));
}

Word
CreatePredicateGroup( const ListExpr typeInfo){
   return (SetWord(new PredicateGroup(0)));
}

/*
4.3 Functions for object deletion

*/
void
DeleteInt9M( const ListExpr typeInfo, Word& w )
{
  delete (Int9M*) w.addr;
  w.addr = 0;
}

void
DeleteCluster(const ListExpr typeInfo, Word& w){
   delete (Cluster*) w.addr;
   w.addr = 0;
}

void
DeletePredicateGroup(const ListExpr typeInfo, Word& w){
   delete (PredicateGroup*) w.addr;
   w.addr = 0;
}

/*
4.4 Closing objects

*/
void
CloseInt9M( const ListExpr typeInfo, Word& w )
{
  delete (Int9M*) w.addr;
  w.addr = 0;
}

void CloseCluster(const ListExpr typeInfo, Word& w){
   delete (Cluster*) w.addr;
   w.addr = 0;
}

void
ClosePredicateGroup(const ListExpr typeInfo, Word& w){
   delete (PredicateGroup*) w.addr;
   w.addr = 0;
}

/*
4.5 Copying of objects

*/
Word
CloneInt9M( const ListExpr typeInfo, const Word& w )
{
  return SetWord( ((Int9M *)w.addr)->Clone() );
}

Word CloneCluster(const ListExpr typeInfo, const Word& w){
    return SetWord(((Cluster*)w.addr)->Clone());
}

Word ClonePredicateGroup(const ListExpr typeInfo, const Word& w){
    return SetWord(((PredicateGroup*)w.addr)->Clone());
}

/*
4.6 Size of objects

*/
int
SizeOfInt9M()
{
  return sizeof(Int9M);
}

int SizeOfCluster(){
   return sizeof(Cluster);
}

int SizeOfPredicateGroup(){
   return sizeof(PredicateGroup);
}

/*
4.7 Cast functions

*/
void* CastInt9M( void* addr )
{
  return (new (addr) Int9M);
}

void* CastCluster(void* addr){
   return (new (addr) Cluster);
}

void* CastPredicateGroup(void* addr){
   return (new (addr) PredicateGroup);
}

/*
4.8 Property Functions

*/
ListExpr
Int9MProperty()
{
  return nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("int9m"),
                             nl->StringAtom("(II IB IE BI BB"
                                            " BE EI EB EE)"),
                             nl->StringAtom("TRUE FALSE TRUE ... FALSE")));
}

ListExpr
ClusterProperty()
{
  return nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("cluster"),
                             nl->StringAtom("string int int ... int "),
                             nl->StringAtom("'equals' 5 64 511")));
}

ListExpr
PredicateGroupProperty()
{
  return nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("predicategroup"),
                             nl->StringAtom("<cluster_1>..<cluster_n> "),
                             nl->StringAtom("c1 c2 c3")));
}
/*
4.9 Kind Checking

*/
bool
CheckInt9M( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual(type, "int9m" ));
}

bool
CheckCluster( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual(type, "cluster" ));
}

bool
CheckPredicateGroup( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual(type, "predicategroup" ));
}

/*
4.10 Open Functions

*/
bool
OpenInt9M( SmiRecord& valueRecord,
           size_t& offset,
           const ListExpr typeInfo,
           Word& value )
{
  Int9M *i9m = (Int9M*)Attribute::Open( valueRecord, offset, typeInfo );
  value = SetWord( i9m );
  return true;
}

bool
OpenCluster( SmiRecord& valueRecord,
           size_t& offset,
           const ListExpr typeInfo,
           Word& value )
{
  Cluster *cluster;
  cluster = (Cluster*)Attribute::Open( valueRecord, offset, typeInfo );
  value = SetWord( cluster );
  return true;
}

bool
OpenPredicateGroup( SmiRecord& valueRecord,
           size_t& offset,
           const ListExpr typeInfo,
           Word& value )
{
  PredicateGroup *pgroup;
  pgroup = (PredicateGroup*)Attribute::Open( valueRecord,
                                                offset, typeInfo );
  value = SetWord( pgroup );
  return true;
}

/*
4.11 Save Functions

*/
bool
SaveInt9M( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  Int9M *int9m = (Int9M *)value.addr;
  Attribute::Save( valueRecord, offset, typeInfo, int9m );
  return true;
}

bool
SaveCluster( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  Cluster *cluster = (Cluster *)value.addr;
  Attribute::Save( valueRecord, offset, typeInfo, cluster );
  return true;
}

bool
SavePredicateGroup( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  PredicateGroup *pgroup = (PredicateGroup *)value.addr;
  Attribute::Save( valueRecord, offset, typeInfo, pgroup );
  return true;
}

/*
5 Type Constructor

*/
TypeConstructor int9m(
        "int9m",
        Int9MProperty,
        OutInt9M,  InInt9M,
        0,        0,
        CreateInt9M, DeleteInt9M,
        OpenInt9M, SaveInt9M, CloseInt9M, CloneInt9M,
        CastInt9M,
        SizeOfInt9M,
        CheckInt9M );

TypeConstructor cluster(
        "cluster",
        ClusterProperty,
        OutCluster,  InCluster,
        0,        0,
        CreateCluster, DeleteCluster,
        OpenCluster, SaveCluster, CloseCluster, CloneCluster,
        CastCluster,
        SizeOfCluster,
        CheckCluster );

TypeConstructor predicategroup(
        "predicategroup",
        PredicateGroupProperty,

        OutPredicateGroup,  InPredicateGroup,
        0,        0,
        CreatePredicateGroup, DeletePredicateGroup,
        OpenPredicateGroup, SavePredicateGroup,
        ClosePredicateGroup, ClonePredicateGroup,
        CastPredicateGroup,
        SizeOfPredicateGroup,
        CheckPredicateGroup );

/*
7 Definition of Algebra operators

7.1 Type Mapping

*/
ListExpr Int9M_Int9M(ListExpr args){
   if(nl->ListLength(args)!=1)
      return nl->SymbolAtom("typeerror");
   ListExpr arg=nl->First(args);
   if(nl->AtomType(arg)!=SymbolType)
      return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(arg,"int9m"))
      return nl->SymbolAtom("int9m");
   return nl->SymbolAtom("typeerror");
}

ListExpr Int9M_Int9M_Int9M(ListExpr args){
   if(nl->ListLength(args)!=2)
      return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(nl->First(args),"int9m") &&
      nl->IsEqual(nl->Second(args),"int9m"))
      return nl->SymbolAtom("int9m");
   return nl->SymbolAtom("typeerror");
}

ListExpr Int9M_Int(ListExpr args){
   if(nl->ListLength(args)!=1)
     return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(nl->First(args),"int9m"))
     return nl->SymbolAtom("int");
   return nl->SymbolAtom("typeerror");
}

ListExpr Cluster_String(ListExpr args){
   if(nl->ListLength(args)!=1)
     return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(nl->First(args),"cluster"))
     return nl->SymbolAtom("string");
   return nl->SymbolAtom("typeerror");
}

ListExpr Cluster_Cluster(ListExpr args){
   if(nl->ListLength(args)!=1)
      return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(nl->First(args),"cluster"))
      return nl->SymbolAtom("cluster");
   return nl->SymbolAtom("typeerror");
}

ListExpr Cluster_Int9M_Cluster(ListExpr args){
   if(nl->ListLength(args)!=2)
     return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(nl->First(args),"cluster") &&
      nl->IsEqual(nl->Second(args),"int9m"))
      return nl->SymbolAtom("cluster");
   return nl->SymbolAtom("typeerror");
}

ListExpr Cluster_Int9M_Bool(ListExpr args){
   if(nl->ListLength(args)!=2)
     return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(nl->First(args),"cluster") &&
      nl->IsEqual(nl->Second(args),"int9m"))
      return nl->SymbolAtom("bool");
   return nl->SymbolAtom("typeerror");
}

ListExpr Cluster_Cluster_Bool(ListExpr args){
   if(nl->ListLength(args)!=2)
     return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(nl->First(args),"cluster") &&
      nl->IsEqual(nl->Second(args),"cluster"))
      return nl->SymbolAtom("bool");
   return nl->SymbolAtom("typeerror");
}

ListExpr TransposeTM(ListExpr args){
   if(nl->ListLength(args)==1){
      if(nl->IsEqual(nl->First(args),"int9m"))
         return nl->SymbolAtom("int9m");
   }
   else if(nl->ListLength(args)==2){
      if(nl->IsEqual(nl->First(args),"cluster") &&
         nl->IsEqual(nl->Second(args),"string"))
         return nl->SymbolAtom("cluster");
   }
   return nl->SymbolAtom("typeerror");
}

ListExpr Cluster_String_Cluster(ListExpr args){
   if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("Two arguments expected");
     return nl->SymbolAtom("typeerror");
   }
   if(!nl->IsEqual(nl->First(args),"cluster")){
     ErrorReporter::ReportError("First argument must be a cluster");
     return nl->SymbolAtom("typeerror");
   }
   if(! nl->IsEqual(nl->Second(args),"string")){
     ErrorReporter::ReportError("The second argument must be a string");
     return nl->SymbolAtom("typeerror");
   }
   return nl->SymbolAtom("cluster");
}

ListExpr Cluster_Cluster_Cluster(ListExpr args){
   if(nl->ListLength(args)!=2)
     return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(nl->First(args),"cluster") &&
      nl->IsEqual(nl->Second(args),"cluster"))
      return nl->SymbolAtom("cluster");
   return nl->SymbolAtom("typeerror");
}

ListExpr SetOpsTM(ListExpr args){
   if(nl->ListLength(args)!=2)
     return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(nl->First(args),"cluster") &&
      nl->IsEqual(nl->Second(args),"cluster"))
      return nl->SymbolAtom("cluster");
   if(nl->IsEqual(nl->First(args),"int9m") &&
      nl->IsEqual(nl->Second(args),"int9m"))
      return nl->SymbolAtom("int9m");
   return nl->SymbolAtom("typeerror");
}

ListExpr InvertTM(ListExpr args){
   if(nl->ListLength(args)!=1)
     return nl->SymbolAtom("typeerror");
   if(nl->IsEqual(nl->First(args),"cluster"))
      return nl->SymbolAtom("cluster");
   if(nl->IsEqual(nl->First(args),"int9m") )
      return nl->SymbolAtom("int9m");
   return nl->SymbolAtom("typeerror");
}

ListExpr CreatePGroupTM(ListExpr args){
   while(!nl->IsEmpty(args)){
      ListExpr current = nl->First(args);
      if(!nl->IsEqual(current,"cluster"))
         return nl->SymbolAtom("typeerror");
      args = nl->Rest(args);
   }
   return nl->SymbolAtom("predicategroup");
}

ListExpr CreateValidPGroupTM(ListExpr args){
  if(nl->ListLength(args)<1){
       ErrorReporter::ReportError("At Least one element required");
       return nl->SymbolAtom("typeerror");
   }
   nl->WriteListExpr(args);
   while(!nl->IsEmpty(args)){
      ListExpr current = nl->First(args);
      if(!nl->IsEqual(current,"cluster")){
         ErrorReporter::ReportError("only clusters are allowed\n");
         nl->WriteListExpr(current);
         return nl->SymbolAtom("typeerror");
      }
      args = nl->Rest(args);
   }
   return nl->SymbolAtom("predicategroup");
}

ListExpr ClusterName_OfTM(ListExpr args){
    if(nl->ListLength(args)==2){
        if(nl->IsEqual(nl->First(args),"predicategroup") &&
           nl->IsEqual(nl->Second(args),"int9m"))
            return nl->SymbolAtom("string");
    }
    return nl->SymbolAtom("typeerror");
}

ListExpr ClusterOfTM(ListExpr args){
    if(nl->ListLength(args)==2){
        if(nl->IsEqual(nl->First(args),"predicategroup") &&
           nl->IsEqual(nl->Second(args),"int9m"))
            return nl->SymbolAtom("cluster");
    }
    return nl->SymbolAtom("typeerror");
}

ListExpr SizeOfTM(ListExpr args){
   if(nl->ListLength(args)==1){
      if(nl->IsEqual(nl->First(args),"cluster") |
         nl->IsEqual(nl->First(args),"predicategroup"))
         return nl->SymbolAtom("int");
      else {
        ErrorReporter::ReportError("TopRel:  SizeOf expects a cluster "
                                   "or a predicategroup\n");
        return nl->SymbolAtom("typeerror");
      }
   }
   ErrorReporter::ReportError("TopRel: Wrong number of"
                              " arguments for sizeof\n");
   return nl->SymbolAtom("typerror");
}

ListExpr CreateClusterTM(ListExpr args){
   if(nl->ListLength(args)!=2){
      ErrorReporter::ReportError("CreateCluster expects 2 arguments\n");
      return nl->SymbolAtom("typeerror");
   }
   if( nl->IsEqual(nl->First(args),"string") &&
       (nl->IsEqual(nl->Second(args),"string") ||
        nl->IsEqual(nl->Second(args),"text")))
        return nl->SymbolAtom("cluster");
   ErrorReporter::ReportError("CreateCluster requires"
                              " string x {string,text} as arguments\n");
   return nl->SymbolAtom("typeerror");
}

ListExpr IsCompleteTM(ListExpr args){
  if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError("IsComplete has one element\n");
     return nl->SymbolAtom("typeerror");
  }
  if(nl->IsEqual(nl->First(args),"predicategroup"))
     return nl->SymbolAtom("bool");
  ErrorReporter::ReportError("IsComplete"
                             " needs a predicategroup as argument\n");
  return nl->SymbolAtom("typeerror");
}

ListExpr UnSpecifiedTM(ListExpr args){
  if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError("unspecified needs one element\n");
     return nl->SymbolAtom("typeerror");
  }
  if(nl->IsEqual(nl->First(args),"predicategroup"))
     return nl->SymbolAtom("cluster");
  ErrorReporter::ReportError("unspecified"
                             " needs a predicategroup as argument\n");
  return nl->SymbolAtom("typeerror");
}

ListExpr MultiSetOpsTM(ListExpr args){
  if(nl->ListLength(args)<1){
     ErrorReporter::ReportError("At least one argument required\n");
     return nl->SymbolAtom("typeerror");
  }
  ListExpr rest = nl->Rest(args);
  if(nl->IsEqual(nl->First(args),"int9m")){
      while(!nl->IsEmpty(rest)){
          if(!nl->IsEqual(nl->First(rest),"int9m")){
              ErrorReporter::ReportError("elements must be from"
                                         " the same type");
              return  nl->SymbolAtom("typerror");
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
              return  nl->SymbolAtom("typerror");
          }
          rest = nl->Rest(rest);
      }
      return nl->SymbolAtom("cluster");
  }
  ErrorReporter::ReportError("int9m or cluster required");
  return nl->SymbolAtom("typeerror");
}

ListExpr PWDisjointTM(ListExpr args){
  ListExpr rest = args;
  while(!nl->IsEmpty(rest)){
    if(!nl->IsEqual(nl->First(rest),"cluster")){
       ErrorReporter::ReportError("elements must be from the same type");
       return  nl->SymbolAtom("typeerror");
    }
    rest = nl->Rest(rest);
  }
  return nl->SymbolAtom("bool");
  ErrorReporter::ReportError("cluster required");
  return nl->SymbolAtom("typeerror");
}


ListExpr RestrictRelaxTM(ListExpr args){
  if(nl->ListLength(args)!=2){
      ErrorReporter::ReportError("Restrict and Relax require two arguments\n");
      return nl->SymbolAtom("typeerror");
  }
  if(!nl->IsEqual(nl->First(args),"cluster")){
      ErrorReporter::ReportError("Restrict and Relax require"
                                 " cluster as first argument \n");
      string type;
      nl->WriteToString(type,nl->First(args));
      ErrorReporter::ReportError("but get "+type+ "\n");
      return nl->SymbolAtom("typeerror");
  }
  if(!nl->IsEqual(nl->Second(args),"string") &&
     !nl->IsEqual(nl->Second(args),"text")){
      string type;
      nl->WriteToString(type,nl->Second(args));
      ErrorReporter::ReportError("Restrict and Relax require string or text"
                             " as second argument but receive "+type+"\n");
      return nl->SymbolAtom("typeerror");
  }
  return nl->SymbolAtom("cluster");
}


/*
7.2 Value Mappings

*/
int Transpose_Int9M_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Int9M* a = (Int9M*) args[0].addr;
  Int9M* res = (Int9M*) result.addr;
  res->Equalize(a);
  res->Transpose();
  return 0;
}

int Transpose_Cluster_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  CcString* b = (CcString*) args[1].addr;
  Cluster* res = (Cluster*) result.addr;
  res->Equalize(a);
  res->Transpose();
  res->SetName(b->GetStringval());
  return 0;
}

int Invert_Int9M_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Int9M* a = (Int9M*) args[0].addr;
  Int9M* res = (Int9M*) result.addr;
  res->Equalize(a);
  res->Invert();
  return 0;
}

int Union_Int9M_Int9M_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Int9M* a = (Int9M*) args[0].addr;
  Int9M* b = (Int9M*) args[1].addr;
  Int9M* res = (Int9M*) result.addr;
  res->Equalize(a);
  res->Union(b);
  return 0;
}

int MultiIntersection_Int9M_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Int9M* a = (Int9M*) args[0].addr;
  Int9M* res = (Int9M*) result.addr;
  res->Equalize(a);
  int sons = qp->GetNoSons(s);
  for(int i=1;i<sons;i++)
      res->Intersection((Int9M*)args[i].addr);
  return 0;
}

int Intersection_Int9M_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Int9M* a = (Int9M*) args[0].addr;
  Int9M* res = (Int9M*) result.addr;
  res->Equalize(a);
  res->Intersection((Int9M*)args[1].addr);
  return 0;
}

int Disjoint_Cluster_Cluster_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Cluster* b = (Cluster*) args[1].addr;
  CcBool* res = (CcBool*) result.addr;
  res->Set(true,a->Disjoint(b));
  return 0;
}

int PWDisjoint_Cluster_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
   result = qp->ResultStorage(s);
   int sons = qp->GetNoSons(s);
   Cluster All;
   All.MakeEmpty();
   int res = true;
   for(int i=0;i<sons && res ;i++){
      if(All.Intersects((Cluster*)args[i].addr))
         res=false;
      All.Union((Cluster*)args[i].addr);
   }
   ((CcBool*)result.addr)->Set(true,res);
   return 0;
}


int Add_Fun(Word* args, Word& result, int message,
            Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Int9M* b = (Int9M*) args[1].addr;
  Cluster* res = (Cluster*) result.addr;
  unsigned short number = b->GetNumber();
  res->Equalize(a);
  res->SetValueAt(number,true);
  return 0;
}

int Remove_Fun(Word* args, Word& result, int message,
            Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Int9M* b = (Int9M*) args[1].addr;
  Cluster* res = (Cluster*) result.addr;
  unsigned short number = b->GetNumber();
  res->Equalize(a);
  res->SetValueAt(number,false);
  return 0;
}

int NumberOf_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Int9M* a = (Int9M*) args[0].addr;
  CcInt* res = (CcInt*) result.addr;
  res->Set(true,a->GetNumber());
  return 0;
}

int NameOf_Fun(Word* args, Word& result, int message,
               Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  CcString* res = (CcString*) result.addr;
  res->Set(true,a->GetName());
  return 0;
}

int Rename_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  CcString* b = (CcString*) args[1].addr;
  Cluster* res = (Cluster*) result.addr;
  res->Equalize(a);
  res->SetName(b->GetStringval());
  return 0;
}

int Contains_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Int9M* b = (Int9M*) args[1].addr;
  CcBool* res = (CcBool*) result.addr;
  res->Set(true,a->ValueAt(b->GetNumber()));
  return 0;
}

int Union_Cluster_Cluster_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Cluster* b = (Cluster*) args[1].addr;
  Cluster* res = (Cluster*) result.addr;
  res->Equalize(a);
  res->Union(b);
  return 0;
}

int MultiIntersection_Cluster_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Cluster* res = (Cluster*) result.addr;
  res->Equalize(a);
  int sons = qp->GetNoSons(s);
  for(int i=1;i<sons;i++)
      res->Intersection((Cluster*)args[i].addr);
  return 0;
}

int Intersection_Cluster_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Cluster* res = (Cluster*) result.addr;
  res->Equalize(a);
  res->Intersection((Cluster*)args[1].addr);
  return 0;
}

int Minus_Cluster_Cluster_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Cluster* b = (Cluster*) args[1].addr;
  Cluster* res = (Cluster*) result.addr;
  res->Equalize(a);
  res->Minus(b);
  return 0;
}

int Invert_Cluster_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Cluster* a = (Cluster*) args[0].addr;
  Cluster* res = (Cluster*) result.addr;
  res->Equalize(a);
  res->Invert();
  return 0;
}

int Restrict_Cluster_String_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
    result = qp->ResultStorage(s);
    Cluster* cluster = (Cluster*) args[0].addr;
    CcString* cond = (CcString*) args[1].addr;
    const STRING* cond_c = cond->GetStringval();
    string cond_s(*cond_c);
    Cluster* res = (Cluster*) result.addr;
    res->Equalize(cluster);
    res->Restrict(cond_s);
    return 0;
}

int Restrict_Cluster_Text_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
    result = qp->ResultStorage(s);
    Cluster* cluster = (Cluster*) args[0].addr;
    FText* cond = (FText*) args[1].addr;
    string cond_s( (cond->Get()));
    Cluster* res = (Cluster*) result.addr;
    res->Equalize(cluster);
    res->Restrict(cond_s);
    return 0;
}

int Relax_Cluster_String_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
    result = qp->ResultStorage(s);
    Cluster* cluster = (Cluster*) args[0].addr;
    CcString* cond = (CcString*) args[1].addr;
    string cond_s(*(cond->GetStringval()));
    Cluster* res = (Cluster*) result.addr;
    res->Equalize(cluster);
    res->Relax(cond_s);
    return 0;
}

int Relax_Cluster_Text_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
    result = qp->ResultStorage(s);
    Cluster* cluster = (Cluster*) args[0].addr;
    FText* cond = (FText*) args[1].addr;
    string cond_s(cond->Get());
    Cluster* res = (Cluster*) result.addr;
    res->Equalize(cluster);
    res->Relax(cond_s);
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
    error = !res->Add((Cluster*)(args[i].addr));
 }
 if(error){
   res->SetDefined(false);
   ErrorReporter::ReportError("The given clusters overlaps ");
   return 0;
 }
 else
   res->SetDefined(true);
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
    error = !res->AddWithPriority((Cluster*)(args[i].addr));
 }
 if(error){
   res->SetDefined(false);
   ErrorReporter::ReportError("The given clusters overlaps ");
   return 0;
 }
 else
   res->SetDefined(true);
 return 0;
}

int CreateValidPGroup_Fun(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  result = qp->ResultStorage(s);
  PredicateGroup* res = (PredicateGroup*) result.addr;
  Cluster* Valid = (Cluster*)args[0].addr;
 //get the number of arguments
 int arg_count = qp->GetNoSons(s);
 res->MakeEmpty();
 bool error = false;
 Cluster* Current;
 for(int i=1;i<arg_count && ! error; i++){
    Current = (Cluster*) args[i].addr;
    Current->Intersection(Valid);
    error = !res->Add(Current);
 }
 if(error){
   res->SetDefined(false);
   ErrorReporter::ReportError("The given clusters overlaps ");
   return 0;
 }
 else
   res->SetDefined(true);
 return 0;
}

int ClusterNameOf_Fun(Word* args, Word& result, int message,
                Word& local, Supplier s){
    result = qp->ResultStorage(s);
    PredicateGroup* PG = (PredicateGroup*) args[0].addr;
    Int9M* IM = (Int9M*) args[1].addr;
    CcString* res = (CcString*) result.addr;
    res->Set(true,PG->GetNameOf(IM));
    return 0;
}

int ClusterOf_Fun(Word* args, Word& result, int message,
                Word& local, Supplier s){
    result = qp->ResultStorage(s);
    PredicateGroup* PG = (PredicateGroup*) args[0].addr;
    Int9M* IM = (Int9M*) args[1].addr;
    Cluster* res = (Cluster*) result.addr;
    Cluster* tmp = PG->GetClusterOf(*IM);
    res->Equalize(tmp);
    delete tmp;
    return 0;
}

int SizeOfCluster_Fun(Word* args, Word& result, int message,
                Word& local, Supplier s){
    result = qp->ResultStorage(s);
    Cluster* arg = (Cluster*) args[0].addr;
    ((CcInt*)result.addr)->Set(true,arg->Size());
    return 0;
}

int SizeOfPredicateGroup_Fun(Word* args, Word& result, int message,
                Word& local, Supplier s){
    result = qp->ResultStorage(s);
    PredicateGroup* arg = (PredicateGroup*) args[0].addr;
    ((CcInt*)result.addr)->Set(true,arg->Size());
    return 0;
}

int CreateCluster_string_Fun(Word* args, Word& result, int message,
                Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcString* arg1 = (CcString*) args[0].addr;
   CcString*  arg2 = (CcString*) args[1].addr;
   Cluster* res = (Cluster*) result.addr;
   const STRING* cond;
   cond  = arg2->GetStringval();
   struct tree* t=0;
   if(!parseString((char*)cond,&t)){
      ErrorReporter::ReportError("Error in parsing condition\n");
      char* Emsg = GetLastMessage();
      ErrorReporter::ReportError(string(Emsg)+"\n");
      free(Emsg);
      res->SetDefined(false);
      return 0;
   }
   // parsing successful
   res->SetName(arg1->GetStringval());
   res->MakeEmpty();
   for(unsigned short i=0;i<512;i++)
      if(evalTree(t,i))
          res->SetValueAt(i,true);
   destroyTree(t);
   return 0;
}

int CreateCluster_text_Fun(Word* args, Word& result, int message,
                Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcString* arg1 = (CcString*) args[0].addr;
   FText*  arg2 = (FText*) args[1].addr;
   const char* cond = arg2->Get();
   struct tree* t=0;
   if(!parseString(cond,&t)){
      ErrorReporter::ReportError("Error in parsing condition\n");
      char* Emsg = GetLastMessage();
      ErrorReporter::ReportError(string(Emsg)+"\n");
      free(Emsg);
      ((Cluster*)result.addr)->SetDefined(false);
      return 0;
   }
   // parsing successful
   Cluster* res = (Cluster*) result.addr;
   res->SetName(arg1->GetStringval());
   res->MakeEmpty();
   for(unsigned short i=0;i<512;i++)
      if(evalTree(t,i))
          res->SetValueAt(i,true);
   destroyTree(t);
   return 0;
}

int IsComplete_Fun(Word* args, Word& result, int message,
                Word& local, Supplier s){
   result = qp->ResultStorage(s);
   PredicateGroup* arg = (PredicateGroup*) args[0].addr;
   ((CcBool*)result.addr)->Set(true,arg->IsComplete());
   return 0;
}


int UnSpecified_Fun(Word* args, Word& result, int message,
                Word& local, Supplier s){
   result = qp->ResultStorage(s);
   PredicateGroup* PG = (PredicateGroup*) args[0].addr;
   ((Cluster*)result.addr)->Equalize(PG->GetUnspecified());
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
      " <text>returns the matrix (renamed cluster)  "
      "symmetrical to the original one</text---> "
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
      " ( <text>type_i x type_i x ... -> type_i ,"
      " where type_i in {int9m,cluster} </text---> "
      " \" trintersection (_,_,...) \" "
      "  \" computes the intersection of all arguments \" "
      "  \" query trintersection(c1,c2,c3) \" ))";

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
      "  \"removes  the elements of o cluster from other one \" "
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
      " ( \"cluster x string -> int\""
      " \" _ renamecluster[ _ ] \" "
      "  \"changes the name of a cluster \" "
      "  \" query c renamecluster[''inside''] \" ))";

const string ContainsSpec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \"cluster x int9m -> bool\""
      " \" _ contains _ \" "
      "  \"checks whether the second element is member of the first one\" "
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
      "  \"returns the name of the matrix in the given "
      " predicategroup\" "
      "  <text>query clustername_of(StdPG,m1)="
      "\"inside\" </text---> ))";

const string ClusterOf_pc_m_Spec =
      "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      " ( \"predicategroup x int9m -> cluster\""
      " \"  clusterof(_ , _) \" "
      "  \"returns the predicate group containing the matrix\" "
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
      "  \"creates an cluster with given name and condition\" "
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

ValueMapping SizeOfMap[] = { SizeOfCluster_Fun,
                             SizeOfPredicateGroup_Fun};

ValueMapping CreateClusterMap[] = { CreateCluster_string_Fun,
                                    CreateCluster_text_Fun};

ValueMapping RestrictMap[] = { Restrict_Cluster_String_Fun,
                               Restrict_Cluster_Text_Fun};

ValueMapping RelaxMap[] = { Relax_Cluster_String_Fun,
                            Relax_Cluster_Text_Fun};
/*
7.6 Selection Functions

*/
static int TransposeSelect(ListExpr args){
   if(nl->IsEqual(nl->First(args),"int9m"))
      return 0;
   if(nl->IsEqual(nl->First(args),"cluster") &&
      nl->IsEqual(nl->Second(args),"string"))
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
   if(nl->IsEqual(nl->Second(args),"string"))
      return 0;
   else
      return 1;
}

static int RestrictSelect(ListExpr args){
   if(nl->IsEqual(nl->Second(args),"string"))
      return 0;
   else
      return 1;
}

static int RelaxSelect(ListExpr args){
   if(nl->IsEqual(nl->Second(args),"string"))
      return 0;
   else
      return 1;
}

/*
7.7 Definition of operators

*/
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
         "sizeof",     // name
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

} // namespace toprel

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

    toprel::int9m.AssociateKind("DATA");
    toprel::cluster.AssociateKind("DATA");
    toprel::predicategroup.AssociateKind("DATA");

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
  }
  ~TopRelAlgebra() {};
} toprelAlgebra;

/*
8 Initialization of the Algebra


*/
extern "C"
Algebra*
InitializeTopRelAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&toprelAlgebra);
}

