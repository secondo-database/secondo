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


//[title] [\title{TopRelAlgebra}\author{Thomas Behr}\maketitle]
//[toc]  [\tableofcontents]
//[x] [\ensuremath{\times}]
//[abstract] [\abstract]
//[&] [\&]
//paragraph [21] verb: [\begin{verbatim}] [\end{verbatim}]
//[|] [$\mid$]
//[\f] [\newpage]
//[=>] [\ensuremath{\Rightarrow}]
//[<=>] [\ensuremath{\Leftrightarrow}]

\setcounter{tocdepth}{2}
\setcounter{secnumdepth}{2}

[title]
[toc]
[abstract]


This algebra provides three data types, namely int9m, cluster, and predicategroup.
The int9m type represents the 9 intersection matrix of  Egenhofer's 9 intersection model.
This means an instance of type int9m is a 3 [x] 3 matrix containing only 
boolean values. To learn more about this model, read the original paper 
''A formal definition of Binary Topological Relationships'' of M.J. Egenhofer.
A cluster describes a set of such matrices. Therewith we can describe a predicate 
''inside'' containing a defined set of matrizes. Beside the matrices, each cluster
contains a name for identification.  A predicate cluster is a
set of pairwise disjoint clusters. This can be used for modelling a complete set of 
topological predicates. 

*/

/*
1 The Header File

2.0  Some needed includes


*/


#ifndef TOPREL_H
#define TOPREL_H



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



/*

2.1 Symbolic names for the matrix positions

The following rows define some contants  
used in  this algebra. The meaning of the letters is the following:

  * I  the interior of the object

  * B  the boundary of the object

  * E the exterior of the object

Furthermore, the position of the letter describes the position 
of the object. For example, for two objects A and B respectively,
IE designates the unemptyness of the intersection between the interior of
A and the exterior of B. 

*/


static const unsigned short II = 256;
static const unsigned short IB = 128;
static const unsigned short IE = 64;
static const unsigned short BI = 32;
static const unsigned short BB = 16;
static const unsigned short BE = 8;
static const unsigned short EI = 4;
static const unsigned short EB = 2;
static const unsigned short EE = 1;


const unsigned char emptyBlock[]={ (char)0, (char)0, (char)0,
    (char)0, (char)0, (char)0, (char)0, (char)0, (char)0, (char)0,
    (char)0, (char)0, (char)0, (char)0, (char)0, (char)0, (char)0,
    (char)0, (char)0, (char)0, (char)0, (char)0, (char)0, (char)0,
    (char)0, (char)0, (char)0, (char)0, (char)0, (char)0, (char)0,
    (char)0, (char)0, (char)0, (char)0, (char)0, (char)0, (char)0,
    (char)0, (char)0, (char)0, (char)0, (char)0, (char)0, (char)0,
    (char)0, (char)0, (char)0, (char)0, (char)0, (char)0, (char)0,
    (char)0, (char)0, (char)0, (char)0, (char)0, (char)0, (char)0,
    (char)0, (char)0, (char)0, (char)0, (char)0};



const unsigned char fullBlock[]={ (char)255, (char)255, (char)255,
    (char)255, (char)255, (char)255, (char)255, (char)255, (char)255,
    (char)255, (char)255, (char)255, (char)255, (char)255, (char)255,
    (char)255, (char)255, (char)255, (char)255, (char)255, (char)255,
    (char)255, (char)255, (char)255, (char)255, (char)255, (char)255,
    (char)255, (char)255, (char)255, (char)255, (char)255, (char)255,
    (char)255, (char)255, (char)255, (char)255, (char)255, (char)255,
    (char)255, (char)255, (char)255, (char)255, (char)255, (char)255,
    (char)255, (char)255, (char)255, (char)255, (char)255, (char)255,
    (char)255, (char)255, (char)255, (char)255, (char)255, (char)255,
    (char)255, (char)255, (char)255, (char)255, (char)255, (char)255,
    (char)255};



/*
2.1 The Data Type ~Int9M~

This type is the implementation  of the well know 9-intersections-matrix developed
by M. Egenhofer. The type is just a implementation of a bitvector with 9 elements
representing the 9 entries in the matrix.

*/

class Int9M:  public  StandardAttribute{
   public:

/*

2.1.1 The Standard Constructor

This constructor should never be called directly. It is used in a non-standard-way
in a cast function.

*/

       Int9M(){};

/*
2.1.2 Constructor

This constructor takes the matrix number and create a matrix from it. 
The matrix number is the number resulting from the bitrepresentation of the 
matrix. For this reason, 512 possible matrices exist. A wrong value for the 
matrix number leads not to an error, the number is just corrected to tak the last
9 bits.

*/
       Int9M(const short number){
           value = number & 511;
           defined = true;
       }


/*
2.1.4 Constructor

In this constructor, all matrix entries can be explicitely set. 

*/
       Int9M(const bool II, const bool IB, const bool IE,
             const bool BI, const bool BB, const bool BE,
             const bool EI, const bool EB, const bool EE);


/*
2.1.5 Constructor

The copy constructor

*/
       Int9M(const Int9M& m){
          value = m.value;
          defined = m.defined;
       }



/*
2.1.5 Destructor

*/
       ~Int9M(){};

/*
2.1.6 The ~GetNumber~ function

This function returns the matrix number of this int9m instance.

*/
       int GetNumber()const{ return (value & 511);}

/*
2.1.7 ~Invert~

The invert function changes the value of each entry in the matrix
to its converse. 

*/
       void Invert(){
          value = value ^ 511;
       }

/*
2.1.8 ~Union~

This function realizes an elementwise ''or'' of the matrix entries.

*/

       void Union(Int9M* arg){
          value |= arg->value;
          defined &= arg->defined;
       }
/* 
2.1.9 ~Intersection~

The ~Intersection~ Functions computes a elementwise ''and'' of both
matrices.

*/ 

       void Intersection(Int9M* arg){
          value &= arg->value;
          defined &= arg->defined;
       }

/*
2.1.10 ~SetToNumber~

This function changes the matrix to this one of the given number.

*/
       void SetToNumber(unsigned short number){
           value = number & 511;
           defined = true;
       }

/*
2.1.11 Set Function

This function sets all matrix entries to the given values.

*/
       void Set( const bool II, const bool IB, const bool IE,
                 const bool BI, const bool BB, const bool BE,
                 const bool EI, const bool EB, const bool EE);

/*
2.1.12 Further Set functions

The next nine functions can be used to set a  single entry in the 
matrix to a specified value.

*/
       void SetII(const bool v){ if(v)
                                     value = value | II;
                                 else
                                     value = value & (511 - II);
                               }
       void SetIB(const bool v){ if(v)
                                     value = value | IB;
                                 else
                                     value = value & (511 - IB);
                               }
       void SetIE(const bool v){ if(v)
                                     value = value | IE;
                                 else
                                     value = value & (511 - IE);
                               }
       void SetBI(const bool v){ if(v)
                                     value = value | BI;
                                 else
                                     value = value & (511 - BI);
                               }
       void SetBB(const bool v){ if(v)
                                     value = value | BB;
                                 else
                                     value = value & (511 - BB);
                               }
       void SetBE(const bool v){ if(v)
                                     value = value | BE;
                                 else
                                     value = value & (511 - BE);
                               }
       void SetEI(const bool v){ if(v)
                                     value = value | EI;
                                 else
                                     value = value & (511 - EI);
                               }
       void SetEB(const bool v){ if(v)
                                     value = value | EB;
                                 else
                                     value = value & (511 - EB);
                               }
       void SetEE(const bool v){ if(v)
                                     value = value | EE;
                                 else
                                     value = value & (511 - EE);
                               }
/*
2.1.13 Get Functions

Aided by the next nine functions, the matrix entries of this matrix
can be received.

*/       

       bool GetII()const {return II & value; }
       bool GetIB()const {return IB & value; }
       bool GetIE()const {return IE & value; }
       bool GetBI()const {return BI & value; }
       bool GetBB()const {return BB & value; }
       bool GetBE()const {return BE & value; }
       bool GetEI()const {return EI & value; }
       bool GetEB()const {return EB & value; }
       bool GetEE()const {return EE & value; }

/*
2.1.14 Transpose 

This function computes the transpose of this matrix. 
This can be used to detect symmetries between matrices.

*/
       void Transpose(){
            Int9M tmp;
            tmp.value = 0;
            tmp.SetII(GetII());
            tmp.SetIB(GetBI());
            tmp.SetIE(GetEI());
            tmp.SetBI(GetIB());
            tmp.SetBB(GetBB());
            tmp.SetBE(GetEB());
            tmp.SetEI(GetIE());
            tmp.SetEB(GetBE());
            tmp.SetEE(GetEE());
            value = tmp.value;
       }

/*
2.1.15 Destroy

This function is needed for using this class as an 
StandardAttribute.

*/
       void Destroy();

/*
2.1.16 ToListExpr

This function computes the ListRepresentation of this matrix.
The ListRepresentation is a list containing 9 boolean elements
describing the value of the matrix entries.

*/
       ListExpr ToListExpr()const  ;

/*
2.1.17 ReadFrom Function

This function reads the value of this matrix from LE. 
If LE don't represent a valid representation of a 
9 intersection matrix, the value of this matrix remains 
unchanged and the result is false. Otherwise, the 
value is changed corresponding to the nested list representation and the
return value is true. Allowed formats are:

  * a single int atom holding a value between 0 and 511 representing a matrix number 

  * a list consisting of nine boolean values representing the matrix entries.

*/
       bool ReadFrom(const ListExpr LE);

/*
2.1.18 Equalize functions

When calling one of the next functions, the values of this matrix are taken
from the argument.

*/
       void Equalize(const Int9M value);
       void Equalize(const Int9M* value);

/*
2.1.19 ToString function

This function returns a string representation of this matrix.

*/

     string ToString();


/*
2.1.19 Further functions

The following functions are needed for using this type as
an attribute type within secondo relations.

*/
       int Compare(const Attribute* arg) const;
       bool Adjacent(const Attribute*) const {return false;}
       bool IsDefined() const;
       void SetDefined( bool defined );
       size_t HashValue() const;
       void CopyFrom(const StandardAttribute* arg);
       Int9M* Clone() const;

/*
2.1.20 Print function

*/

       virtual ostream& Int9M::Print( ostream& os ) const;

/*
2.1.21 CompareTo function

*/
       bool CompareTo(const Int9M M2) const{
          if(!defined && !M2.defined)
            return true;
          if(!defined) return -1;
          if(!M2.defined) return 1;
          if(value>M2.value) return 1;
          if(value<M2.value) return -1;
          return 0;
       }

/*
2.1.22 Equal operator

*/
       bool operator==(const Int9M I2) const;  

/*
2.1.23 Unequal operator

*/
       bool operator!=(const Int9M I2) const{
          return CompareTo(I2)!=0;
       }

/*
2.1.24 Assignment Operator

*/
  inline Int9M& operator=(const Int9M& m){
      value = m.value;
      defined = m.defined;
      return *this;
  }
 

   private:
       // we use the appropriate bits of this value for the different
       // entries in the matrix
       unsigned short value; 
       bool defined;
};



/*
2.2 The Type Cluster

A cluster is a set of 9-intersection matrices implemented by the Int9M type.
Because only 512 of such matrices exist, we can realize this type by a
bitvector containing 512 elements. A "1" of a specific position means that the
matrix with the appropriate number is contained in this cluster.

*/
class Cluster: public StandardAttribute{
   public:

/*
2.2.1 Constructor

This standard constructor should only be used in the cast function.

*/
      Cluster(){} 
      
/*
2.2.2 Constructor

This constructor creates an empty cluster. The argument is only used for 
making this constructor different to the standard constructor and is 
ignored.

*/
      Cluster(const bool all){
          //for(int i=0;i<64;i++)
          //    BitVector[i]=0;
          if(all){
              memcpy(BitVector,fullBlock,64);
              strcpy(name,"empty");
          } else {
             memcpy(BitVector,emptyBlock,64);
              strcpy(name,"complete");
          }
          defined = true;
      }

/*
2.2.3 Destructor 

*/
      ~Cluster(){}

/*
2.2.4 SetName

This function sets a new name for this cluster.

*/
      void SetName(const STRING* newname){
           strcpy(name,*newname);
      }

/*
2.2.5 Transpose

This function transposes all contained matrices in this cluster. 
This can be used for easy defining of symmetrical clusters, e.g.
   Contains.Transpose().SetName("CoveredBy");

*/
      void Transpose();

/*
2.2.6 Disjoint

This function checks whether this cluster and the argument has no
common matrices.

*/
      bool Disjoint(Cluster* C2){
         for(int i=0;i<64;i++)
            if(BitVector[i] & C2->BitVector[i])
               return false;
         return true;
      }

/*
2.2.7 Contains

This function checks whether all matrices contained in the argument are also
contained in this cluster. Note that for equal clusters the result will be true.

*/      
      bool Contains(const Cluster* C2)const{
         for(int i=0;i<64;i++)
            if((BitVector[i] | C2->BitVector[i]) > BitVector[i])
               return false;
         return true;
      }
      
/*
2.2.8 Contains

This function checks whether the argument is part of this cluster.

*/
      bool Contains(const Int9M M) const;
      
/*
2.2.9 Union
      
This implements the familiar union function for the matrix sets.

*/
    void Union(const Cluster* C2){
          for(int i=0;i<64;i++)
             BitVector[i] |= C2->BitVector[i];
      }

/*
2.2.10 Intersection

This function computes the intersection of this cluster and the argument.

*/
      void Intersection(const Cluster* C2){
          for(int i=0;i<64;i++)
             BitVector[i] &= C2->BitVector[i];
      }


/*
2.2.10 Intersects

A call of this function yields true if this cluster and C2 have common elements.

*/
      bool Intersects(const Cluster* C2){
          for(int i=0;i<64;i++)
            if( BitVector[i] & C2->BitVector[i])
               return true;
          return false;
      }


/*
2.2.11 Minus

~Minus~ removes all matrices contained in C2 from this cluster.

*/ 
      void Minus(const Cluster* C2){
          for(int i=0;i<64;i++)
             BitVector[i] = (BitVector[i] & ( ~(C2->BitVector[i])));

      }

/*
2.2.12 Invert

The ~Invert~ function changes this cluster containing all possible 
matrices minus the matrices conatained originally in it.

*/
      void Invert(){
          for(int i=0;i<64;i++)
              BitVector[i] ^= 255;
      }

/*
2.2.13 MakeEmpty      

This function removes all matrices from this cluster.

*/
      void MakeEmpty(){
          //for(int i=0;i<64;i++)
          //    BitVector[i] = 0;
          memcpy(BitVector,emptyBlock,64);
      }

/*
2.2.14 MakeFull

The MakeFull function changes this cluster to contain all
512 possible matrices.

*/
      void MakeFull(){
         //for(int i=0;i<64;i++)
         //     BitVector[i] = 255;
         memcpy(BitVector,fullBlock,64);
      }
/*
2.2.15 IsEmpty

This function checks whether this cluster is empty.

*/

   bool IsEmpty() const{
           return (memcmp(BitVector,emptyBlock,64)==0);
        }


/*
2.2.16 IsComplete

This function returns true if this cluster contains all 512 
possible matrices.

*/
     bool IsComplete(){
       return (memcmp(BitVector,fullBlock,64)==0);
     }


/*
2.2.15 Size

The Size function counts the matrices contained in this cluster.

*/
      unsigned int Size()const{
          unsigned int c=0;
          for(int i=0; i< 64 ; i++){
              for(unsigned int pos=1; pos<256;pos = 2*pos){
                  if(BitVector[i] & pos & 255)
                     c++;
              }
          }
          return c;

      }

/*
2.5.16 ToListExpr

This function returns the nested list representation of this cluster.
The format of the nested list is a list contaning a string representing the
name of the cluster followed by a set of matrix numbers.

*/      
      ListExpr ToListExpr()const;
      
/*
2.5.17 ReadFrom

This function reads the value of this cluster from the nested list given as
argument. If the nested list don't represent a valid cluster, this cluster
is not changed and the result will be false. Otherwise the cluster takes its
value from the nested list and true is returned. 

Allowed formats are:

1) (Name $n_1$ ... $n_n$)

Where name is a string representing the name of this matrix and $n_1$ .. $n_n$ are
the numbers of the contained  matrices.

2) (Name (...) (...) (...))

Where Name is as before and the next elements are valid representations of the
contained matrices. 

3) (Name Conditions)

This can be used for defining a cluster of a logical level. The name describes the name of
this cluster and Condition is a string (or a text) describing the cluster. In the conditions,
boolean constants (true,false), boolean operators (and, or, not, [=>] [<=>] ) and variables
(ii, ib, ie, bi, bb, be, ei, eb, ee) representing the matrix entries are allowed. All matrices
fulfilling the condition will be element of this cluster. For example, we can define a 
contains cluster by the following command:

[21] let contains = [const cluster value ("contains" <text> ii and  ie and not ei and not bb </text>)]

Instead of writing ''and'', ''or'', ''not'', we can also use the abbreviations ''[&]'', ''[|]'', ''!''.


*/
      bool ReadFrom(const ListExpr LE);

/*

2.2.15 Equalize functions

If one of these functions is called, the value of this cluster is taken from the
argument.

*/

      void Equalize(const Cluster value);
      void Equalize(const Cluster* value);
      
/*
2.2.16 Compare Function

This function compares this with the argument.

*/

int CompareTo(const Cluster* C2) const;

/*
2.2.17 Functions supporting the attribute property of this class


The following functions are required to make it possible that a
cluster acts as an attribute type within relations.

*/
      int Compare(const Attribute* arg) const{
         return CompareTo((Cluster*) arg);
      }
      /* returns false in each case */
      bool Adjacent(const Attribute*) const;
      /* returns the defined state of this cluster */
      bool IsDefined() const;
      /* sets the defined state of this cluster */
      void SetDefined( bool defined );
      /* computes a hashvalue for this cluster */
      size_t HashValue() const;
      /* reads the value of this cluster from arg */
      void CopyFrom(const StandardAttribute* arg);
      /* returns a copy of this cluster */
      Cluster* Clone() const;
     

/*
2.2.18 ValueAt

This function returns true iff the matrix with number pos is
part of this cluster.

*/
      bool ValueAt(const int pos) const;
  
/*
2.2.19 SetValueAt

This function sets the containment of the matrix with number pos to
the given value.

*/
      void SetValueAt(const int pos,const bool value);
      
/*
2.2.20 GetName

This function returns the name of this cluster.

*/
      const STRING* GetName() const { return &name; } 


/*
2.2.21 Restrict

This functions changes a cluster in this way that it will contain only such
matrices which are part of the original cluster and fulfill the condition 
given as the argument. If the given string does not represent a valid formula,
the result will be false and the cluster is not changed.

*/

    bool Restrict(string condition);


/*
2.2.22 Relax

This function extends the cluster to all matrices which are already part of the 
cluster or fulfill the given condition. The return value represents the correctness
of the formula describing the condition.

*/

   bool Relax(string condition);

/*
2.2.23 Restrict

This version of restrict, removes all matrixes from the cluster where at the
position given by the first parameter is a different value to the second parameter.
This result of this function is the same as an intersection with a cluster containing all 
matrices with the pecified value at the given position - but this version is faster.

*/
   void Restrict(const int pos, const bool value);



/*
2.2.21 Operators

The following operators can be used for easy comparisons between 
clusters.

*/
      bool operator<(const Cluster C2)const;
      bool operator==(const Cluster C2)const;
      bool operator>(const Cluster C2)const;


   private:
      unsigned char  BitVector[64];
      bool defined;
      STRING name;
      void SetValueAt(const int pos, const bool value, 
                      unsigned char bitvector[]) const;
};




/*
2.3 The Type PredicateGroup 

A PredicateGroup is a set of disjoint clusters.
Aided by this type we can decide, what the cluster for a
given 9Int-Matrix is. A predicategroup contains an additional
cluster with the name 'unspecified' containing all matrices not
included in the other clusters. For this reason, no cluster with the name
'unspecified' can be included manually.

*/

class PredicateGroup: public StandardAttribute{
public:

/*
2.3.1 The Standardconstructor

*/
    PredicateGroup(){}

/*
2.3.2 Constructor

This constructor creates an empty predicate group with the 
given capacity for clusters. In this context,
'empty' means, that this predicate group contains a single 
cluster named ''unspecified'' containing all 512 9-intersection matrices.

*/
    PredicateGroup(int size);
/*
2.3.3. Destructor

This destructor destroys the contained FLOB.

*/
    ~PredicateGroup(){
          theClusters.Destroy();
    }

/*
2.3.4 Equalize functions

When one of the next functions is called, the value of this
predicate cluster will be the same like this one of the argument.

*/

    void Equalize(const PredicateGroup* PC);

    void Equalize(const PredicateGroup PC){
        Equalize(&PC);
    }

/*
2.3.4 Functions for acting as an attribute type 

*/

    bool IsDefined() const{
         return defined;
    }

    void SetDefined( bool Defined){
        this->defined = Defined;
    }

    size_t HashValue() const{
       return unSpecified.HashValue();
    }

    void CopyFrom(const StandardAttribute* right){
       Equalize((PredicateGroup*) right);
    }

    int Compare(const Attribute * arg) const;

    bool Adjacent(const Attribute * arg) const{
        return false;
    }

    PredicateGroup* Clone() const{
       PredicateGroup* res = new PredicateGroup(1);
       res->Equalize(this);
       return res;
    }

    int NumOfFLOBs() const{
       return 1;
    }

    FLOB *GetFLOB(const int i){
       assert(i==0);
       return &theClusters;
    }


/*
2.3.5 The Size function

This function returns the number of contained clusters.
Note that the unspecified cluster is also counted by this function.

*/
    unsigned int Size(){
       return theClusters.Size()+1;
    }

/*
2.3.6 The ~IsComplete~ function

This function returns true if all possible matrices are contained
in named clusters.

*/
   bool IsComplete(){
      return unSpecified.IsEmpty();
   }


/*
2.3.6 The ~IsEmpty~ function

This function returns true if no cluster is contained in this predicategroup.

*/
   bool IsEmpty(){
      return unSpecified.IsComplete();
   }

/*
2.3.7 GetUnspecified

This function returns the cluster representing all non-covered matrices.

*/
   Cluster GetUnspecified(){
      return unSpecified;
   }   


/*
2.3.6 ToListExpr

This function computes the nested list representaton of this predicate cluster.

*/
    ListExpr ToListExpr();

/*
2.3.7 ReadFrom

This function reads the value of this predicate group from it's 
nested list representation. If the list don't represent a valid value,
false is returned. Otherwise, this value is changed and the result
is true. 
A predicate group representation is just a set of clusters.
All contained clusters must be 
disjoint. This means all cluster names must be different and the matrix sets 
can't have any common matrices.

*/
    bool ReadFrom(const ListExpr instance);

/*
2.3.8 MakeEmpty

The ~MakeEmpty~ function removes all clusters from this cluster predicate.
Only the ''unspecified'' cluster will remain in this cluster predicate. This
cluster will contain all 512 possible matrices.


*/
    void MakeEmpty(){ 
        unSpecified.MakeFull();
        theClusters.Clear();
        defined=true;
        sorted=true;
    }

/*
2.3.9 Add

This function add a cluster to this cluster predicate. The new cluster can't
have any common matrix with an existing cluster and the name must be different 
to all names of contained clusters.

*/
    bool Add(Cluster* C);



/*
2.3.9 AddWithPriority

This function add a cluster to this cluster predicate. Existing clusters 
will have a higher priority as the new cluster C. This means, that all 
matrices contained in present clusters are removed from C before inserting it. 

*/
    bool AddWithPriority(const Cluster * C);

/*
2.3.10 GetNameOf

This function searchs for the cluster containing the given matrix 
and returns its name. The search is realized by scanning the whole
FLOB. For this reason, this function  has linear runtime.
This is acceptable because at most 512 non-overlapping clusters
can exist within a single predicategroup. 

Note that this function creates a new STRING object. The caller of this
function has to destroy this object to avoid memory holes.

*/
   const STRING* GetNameOf(Int9M* Matrix) {
       if(unSpecified.Contains(*Matrix)){
          return unSpecified.GetName();
       }
       int s = theClusters.Size();
       const Cluster *C;
       for(int i=0;i<s;i++){
           theClusters.Get(i,C);
           if(C->Contains(*Matrix))
               return C->GetName();
       }
       assert(false); // should never be reached
    }

/*
2.3.10 GetClusterOf

This function searchs for the cluster containing the given matrix 
and returns it. The search is realized by scanning the whole
FLOB. For this reason, this function  has linear runtime.
This is acceptable because at most 512 non-overlapping clusters
can exist within a single predicategroup. 

The caller of this function has to destroy the object produced
by this function.  

*/
   Cluster* GetClusterOf(Int9M Matrix){
       if(unSpecified.Contains(Matrix))
          return &unSpecified;
       int s = theClusters.Size();
       const Cluster *C;
       for(int i=0;i<s;i++){
           theClusters.Get(i,C);
           if(C->Contains(Matrix)){
               return new Cluster(*C);
           }
       }
       assert(false); // should never be reached
    }

/*
2.3.11 GetClusterOf

This function returns a pointer to a cluster with given name.
If no cluster assignet to this name is part of this predicate
group, the result will be __NULL__.  

*/
Cluster* GetClusterOf(const STRING* name) const;
   




private:
   mutable DBArray<Cluster> theClusters;
   bool defined;
   bool canDelete;
   mutable bool sorted;
   Cluster unSpecified; // Cluster containing all non-used matrices
};

#endif
