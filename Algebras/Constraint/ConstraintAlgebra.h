/*
----
This file is part of SECONDO.

Copyright (C) 2006, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

{\Large \bf Anhang B: Constraint-Template }

[1] Header File of the Constraint Algebra

July, 2006 Simon Muerner

[TOC]

1 Overview

This header file essentially contains the definition of the class Constraint.
This class corresponds to the memory representation for the type constructor
~constraint~ which represents a 2-dimensional (potentially infinite) point set. 


2 Defines and includes

*/
#ifndef __CONSTRAINT_ALGEBRA_H__
#define __CONSTRAINT_ALGEBRA_H__

using namespace std;

#include <cmath>
#include <string>
#include <vector>
#include "SpatialAlgebra.h"


#define MAX( a, b ) ((a) > (b) ? (a) : (b))
#define MIN( a, b ) ((a) < (b) ? (a) : (b))

namespace Constraint {
  
// Konstanten und Enumerationen:
const string OP_EQ = "eq";
const string OP_LEQ = "leq";
// Word-Coordinates (arbitionary big):
const Rectangle<2> WORLD(true, -10000.0, 10000.0, -10000.0, 10000.0); 
enum geotype {UNKNOWN, POINT, SEGMENT, POLYGON};


/*
3 Usefull structs

3.1 Struct ~Point2D~

*/
struct Point2D
{
  Point2D( )
  {
    this->x = 0.0;
    this->y = 0.0;    
  }        
  Point2D( double xCord, double yCord )
  {
    this->x = xCord;
    this->y = yCord;    
  }        
  Point2D( const Point2D& otherPoint )
  {
    this->x = otherPoint.x;
    this->y = otherPoint.y;    
  }        

  Point2D& operator=( const Point2D& otherPoint )
  {
    this->x = otherPoint.x;
    this->y = otherPoint.y;    
    return *this;
  }        

  bool operator==( const Point2D& otherPoint ) const
  {
    return ((AlmostEqual(this->x, otherPoint.x)) && 
            (AlmostEqual(this->y, otherPoint.y)));
  }   

  bool operator!=( const Point2D& otherPoint ) const
  {
    return ((!AlmostEqual(this->x, otherPoint.x)) || 
            (!AlmostEqual(this->y, otherPoint.y)));
  }   
  
  // wichtig fuer sort(...)-Funktion: 
  bool operator<( const Point2D& otherPoint ) const  
  {
    if(AlmostEqual(this->x, otherPoint.x))
    {
      // dann ist y-Koordinaten-Vergleich entschieden:
      if(AlmostEqual(this->y, otherPoint.y))
      {
        return false; 
      }
      else
      {
        return (this->y < otherPoint.y);
      }
    }
    else
    { // dann ist x-Koordinante-Vergleich entscheiden:
      return (this->x < otherPoint.x);   
    }
  }   

  bool OnSegment( const Point2D& p1, const Point2D& p2 ) const
  {
    // Prerequisite: the points in {p1, p2} are not equal 
    // Special case (vertical segment)
    if(p1==(*this) || p2==(*this))
    {
        return true;
    }
    else if(AlmostEqual(p1.x, p2.x) && 
            AlmostEqual(p1.x, (*this).x) && 
            AlmostEqual(p2.x, (*this).x))
    {
      Point2D pSegmentUpp = p1;
      Point2D pSegmentLow = p2;
      if(p1.y < p2.y)
      {
         pSegmentUpp = p2;
         pSegmentLow = p1;
      }
      if((pSegmentLow.y <= (*this).y) && 
        ((*this).y <= pSegmentUpp.y))
      {
        return true;
      }
      else
      {
        return false; 
      }
    }
    else // non-vertical case of segment
    {
      // in order to calculate a line, we have to have two diffrent points 
      // pFirst, pSecond (different x-coordinates!)
      Point2D pSegmentLeft, pSegmentRight;   
      if(p1.x < p2.x)
      {
         pSegmentLeft = p1;
         pSegmentRight = p2;
         
      }
      else
      {
         pSegmentLeft = p2;
         pSegmentRight = p1;      }

      // => (pSegmentLeft.x!=pSegmentRight.x)      
      double m,b; // Geradengleichung y = mx+b
      m = (pSegmentRight.y-pSegmentLeft.y)/(pSegmentRight.x-pSegmentLeft.x); 
      // m is defined because of the fact (pSegmentLeft.x!=pSegmentRight.x)
      b = pSegmentLeft.y - m*pSegmentLeft.x;            
      if((pSegmentLeft.x <= (*this).x) && ((*this).x <= pSegmentRight.x))
      {
        double y = m*(*this).x + b;
        if(AlmostEqual(y, (*this).y))
        {
          return true;
        }
        else
        {
          return false; 
        }
      }
      else
      {
        return false; 
      }
    }
  }
  

  bool OnSameLineAs( const Point2D& p1, const Point2D& p2 ) const
  {
    // Prerequisite: at minimum two of the points in {*this, p1, p2} are 
    // not equal 
    // Special case (vertical line)
    if(AlmostEqual(p1.x, p2.x) && AlmostEqual(p1.x, (*this).x))
    {
      return true; 
    }
    else // if line, not a vertical line, {p1,p2,p3} doesn't have 
         // all the same x-coordinates
    {
      // in order to calculate a line, we have to have two diffrent points 
      // pFirst, pSecond (different x-coordinates!)
      Point2D pFirst, pSecond, pThird;   
      pFirst = p1;
      if(p1!=p2)
      {        
        pSecond = p2;                        
        pThird = (*this);         
      }
      else // p1!=p3
      {
        pSecond = (*this);                        
        pThird = p2;         
      }
      
      // => (pFirst.x!=pSecond.x)      
      double m,b; // Geradengleichung y = mx+b
      m = (pSecond.y-pFirst.y)/(pSecond.x-pFirst.x); 
      // m is defined because of the fact (pFirst.x!=pSecond.x)
      b = pFirst.y - m*pFirst.x;            
      if(AlmostEqual(pThird.y, m*pThird.x + b))
      {
        // if also the third point fits the equation y = mx+b 
        // => they all lie on the same line!
        return true;
      }
      else
      {
        return false;
      }
    }
  }
  
  void printOut()
  {
     cout << " (x,y) == (" << (*this).x << ", " << (*this).y << ") " <<  endl; 
  }
  
  double x;
  double y; 
};

/*
3.2 Struct ~VerticalTrapez~

Trapez with parallel border(s) to y-axis (possibly degenrates to triangle)

*/
struct VerticalTrapez 
{
  VerticalTrapez& operator=( const VerticalTrapez& otherVerticalTrapez )
  {
    this->pUppLeft = otherVerticalTrapez.pUppLeft;
    this->pUppRight = otherVerticalTrapez.pUppRight;    
    this->pLowLeft = otherVerticalTrapez.pLowLeft;
    this->pLowRight = otherVerticalTrapez.pLowRight;        
    this->IsEmpty = otherVerticalTrapez.IsEmpty;        
    return *this;
  }          
  Point2D pUppLeft;
  Point2D pUppRight;
  Point2D pLowLeft;
  Point2D pLowRight;
  bool IsEmpty;
};

/*
3.3 Struct ~SymbolicTuple~

*/
struct SymbolicTuple 
{
  inline SymbolicTuple& operator=( const SymbolicTuple& symTup )
  {
    this->startIndex = symTup.startIndex;
    this->endIndex = symTup.endIndex;
    this->type = symTup.type;
    this->mbbox = symTup.mbbox;
    this->isNormal = symTup.isNormal;
    return *this;
  }
  
  void printOut()
  {
     cout << " (startIndex, endIndex, isNormal) == (" 
          << (*this).startIndex << ", " 
          << (*this).endIndex  << ", " 
          << (*this).isNormal << ") " <<  endl; 
  }        
  
  // indexes to the DBArray of the linear Constraints:
  int startIndex;
  int endIndex;   
  // other related data:
  geotype type;
  Rectangle<2> mbbox;
  bool isNormal;
};

/*
4 Class ~LinearConstraint~ ...

*/
class LinearConstraint
{
  public:
    LinearConstraint();
    LinearConstraint(double, double, double, string);
    ~LinearConstraint();
    LinearConstraint* Clone();
    LinearConstraint& operator=(const LinearConstraint&);    
    bool IsParallel(const LinearConstraint&);    
    bool IsEqual(const LinearConstraint&);    

 
  void printOut()
  {
     cout << "DEBUG: " << a1 << "*x + " << a2 
     << "*y + " << b << " " << strOp << endl;          
  }

    double get_a1() const; 
    double get_a2() const;
    double get_b() const;
    string get_Op() const;
    void set_a1(double); 
    void set_a2(double);
    void set_b(double);
    void set_Op(string);
    
  private:
    bool defined;
    unsigned int dim;
    double a1;
    double a2;
    double b;   
    char strOp[4];     
};

// weitere Hilfsfunktionen

void ToConstraint(const Rectangle<2>&, vector<LinearConstraint>&); 
void ToConstraint(const vector<Point2D>&, vector<LinearConstraint>&);
Rectangle<2> MinimumBoundingBox(const vector<Point2D>&);
void HalfPlaneIntersection(const vector<LinearConstraint>& , vector<Point2D>&);


LinearConstraint::LinearConstraint() {}

LinearConstraint::~LinearConstraint() {}

LinearConstraint::LinearConstraint(
    double a1, 
    double a2, 
    double b, 
    string strOp) 
{
  this->a1 = a1;
  this->a2 = a2;
  this->b = b;
  string strCutOp(strOp,0,4);
  strcpy(this->strOp,strCutOp.c_str());                            
  
  //cout << "DEBUG: strOp.data() == " << strOp.data() << endl; 
}

LinearConstraint* LinearConstraint::Clone() 
{
  return new LinearConstraint(*this);                    
}

LinearConstraint& LinearConstraint::operator=(const LinearConstraint& linC)
{
  this->a1 = linC.get_a1();
  this->a2 = linC.get_a2();
  this->b = linC.get_b();
  strcpy(this->strOp,(linC.get_Op()).c_str());  
  return *this;
}   

bool LinearConstraint::IsParallel(const LinearConstraint& linC)
{
  // TODO: evt. fuer Ungliechung anpassen
  bool isParallel = (this->a1==0 && linC.get_a1()==0) || 
                    (this->a2==0 && linC.get_a2()==0) ||                     
                    AlmostEqual(this->a1/this->a2, linC.get_a1()/linC.get_a2());
  // Semantic in special case one of the linear constraints is 0x+0y+0=0: 
  // will be "parallel" to the other linear constraint.
  return isParallel;
}

bool LinearConstraint::IsEqual(const LinearConstraint& linC)
{
  bool blnFirst, blnSecond;
  
  // if one of the constraints represents the EMPTY SET and 
  // the other one not, then they are not equal:
  // this happens in two cases:
  // (i)  v=0 for every v in R-{0} 
  // (ii) v<=0 for every v in R+-{0}  
  blnFirst = false;
  if((this->get_Op()==OP_EQ && this->a1==0 && this->a2==0 && this->b!=0) ||
     (this->get_Op()==OP_LEQ && this->a1==0 && this->a2==0 && this->b>0))
  {
    // case (i) or (ii)
    blnFirst = true; 
  } 
  blnSecond = false;
  if((linC.get_Op()==OP_EQ && linC.get_a1()==0 
   && linC.get_a2()==0 && linC.get_b()!=0) || 
     (linC.get_Op()==OP_LEQ && linC.get_a1()==0 && 
      linC.get_a2()==0 && linC.get_b()>0))
  {
    // case (i) or (ii)
    blnSecond = true; 
  }
  if(blnFirst!=blnSecond)
  {
    // then only one constraint represents the empty set and 
    // therefore the two constraints can't be equal:
    return false;
  } else if(blnFirst && blnSecond)
  {
    // both constraints represents the empty set and are therefore equal! 
    return true;
  }  
    
  // if one of the constraints represents ALL POINTS in R^2 and 
  // the other one not, then they are not equal:  
  // this happens in two cases:
  // (i)  v=0 for v=0 
  // (ii) v<=0 for every v in R which is not positive (that means 0 or negative)
  blnFirst = false;
  if((this->get_Op()==OP_EQ && this->a1==0 && this->a2==0 && this->b==0) ||
     (this->get_Op()==OP_LEQ && this->a1==0 && this->a2==0 && this->b<0))
  {
    // case (i) or (ii)
    blnFirst = true; 
  } 
  blnSecond = false;
  if((linC.get_Op()==OP_EQ && linC.get_a1()==0 
   && linC.get_a2()==0 && linC.get_b()==0) || 
     (linC.get_Op()==OP_LEQ && linC.get_a1()==0 && 
      linC.get_a2()==0 && linC.get_b()<=0))
  {
    // case (i) or (ii)
    blnSecond = true; 
  }
  if(blnFirst!=blnSecond)
  {
    // then only one constraint represents the set of all points in R^2 and 
    // therefore the two constraints can't be equal!
    return false;
  } else if(blnFirst && blnSecond)
  {
    // both constraints represents the set of 
    // all point in R^2 and are therefore equal! 
    return true;
  }

  // now we know that none of the two constraints is neither 
  // representing the empty set nor the set of all points in R^2
  // this implies that not both a1 and a2 are 0 because 
  // otherwise the constraint would have represented ether the 
  // empty set or the set of all points in R^2:
  if((this->a1!=0 && linC.get_a1()!=0) || (this->a2!=0 && linC.get_a2()!=0))
  {
    double lamda;
    if(this->a1!=0 && linC.get_a1()!=0)
    {
      lamda = this->a1/linC.get_a1();
    }
    else
    {
      lamda = this->a2/linC.get_a2();
    }
    if(AlmostEqual(this->a1,(lamda*linC.get_a1())) &&
       AlmostEqual(this->a2, (lamda*linC.get_a2())) &&
       AlmostEqual(this->b, (lamda*linC.get_b())))
    {
      return true; 
    } 
    else 
    {
      return false;  
    }    
  } 
  else
  {
    // (a1==1 and a2==0 and a1'==0 and a2'==1) or 
    // (a1==0 and a2==1 and a1'==1 and a2'==0)
    // then one constraint represents a set with a horicontal bounding where 
    // the ohter has a vertical bounding and therefore 
    // the two constraints can't be equal!
    return false; 
  }                         
}

double LinearConstraint::get_a1() const  
{
  return this->a1;
}

double LinearConstraint::get_a2() const 
{
  return this->a2;
}

double LinearConstraint::get_b() const 
{
  return this->b;
}

string LinearConstraint::get_Op() const 
{
  return string(this->strOp);
}

void LinearConstraint::set_a1(double a1)
{
  this->a1 = a1;
}

void LinearConstraint::set_a2(double a2)
{
  this->a2 = a2;
}

void LinearConstraint::set_b(double b)
{
  this->b = b;
}

void LinearConstraint::set_Op(string strOp)
{
  string strCutOp(strOp,0,4);
  strcpy(this->strOp,strCutOp.c_str());  
}

/*
5 Class ~SymbolicRelation~ ...

*/
class SymbolicRelation : public StandardSpatialAttribute<2>
{   
  public:
    inline SymbolicRelation() {}
    SymbolicRelation(const int nConstraints, const int nTuple);
    SymbolicRelation(const SymbolicRelation&);
    SymbolicRelation& operator=( const SymbolicRelation&);
    void Destroy();
    void GetSymbolicTuples( const int, SymbolicTuple const*&) const;
    int LinConstraintsSize() const;
    int SymbolicTuplesSize() const;
    void GetLinConstraints( const int, LinearConstraint const*&) const;
    void addSymbolicTuple(const vector<LinearConstraint>);
    void appendSymbolicRelation(const SymbolicRelation&);
    void overlapSymbolicRelation(const SymbolicRelation&);
    void Normalize();
    // The following functions are needed for the SymbolicRelation class 
    // in order to act as an attribute of a relation:
    int NumOfFLOBs() const;
    FLOB* GetFLOB(const int);
    int Compare(const Attribute*) const;
    bool Adjacent(const Attribute*) const;
    SymbolicRelation* Clone() const;
    bool IsDefined() const;
    void SetDefined(bool);    
    size_t HashValue() const;
    size_t Sizeof() const;
    void CopyFrom(const StandardAttribute*);
    const Rectangle<2> BoundingBox() const;     
  private:    
    DBArray<LinearConstraint> linConstraints;
    DBArray<SymbolicTuple> symbolicTuples;    
    Rectangle<2> mbbox;
};

SymbolicRelation::SymbolicRelation(const int nConstraints, const int nTuple) :
      linConstraints(nConstraints), 
      symbolicTuples(nTuple),
      mbbox(false) 
{
  // nichts zu machen!
  cout << "DEBUG: Start & Ende des Konstruktors SymbolicRelation()" << endl; 
}      

SymbolicRelation::SymbolicRelation(const SymbolicRelation& symRel):
  linConstraints(symRel.LinConstraintsSize()),
  symbolicTuples(symRel.SymbolicTuplesSize()), 
  mbbox(symRel.BoundingBox())
{ 
  cout << "DEBUG: Start & Ende des Konstruktors SymbolicRelation(...)" << endl; 
  // Kopieren des linConstraints-DBArrays:
  int i;
  for(i = 0; i < symRel.LinConstraintsSize(); i++ )
  {
    const LinearConstraint *linC;
    symRel.GetLinConstraints( i, linC );
    this->linConstraints.Put( i, *linC );
  }
  // Kopieren des symbolicTuples-DBArrays:
  for(i = 0; i < symRel.SymbolicTuplesSize(); i++ )
  {
    const SymbolicTuple *iPair;
    symRel.GetSymbolicTuples( i, iPair );
    this->symbolicTuples.Put( i, *iPair );
  }        
}      

SymbolicRelation& SymbolicRelation::operator=( const SymbolicRelation& symRel )
{
  int i;
  this->linConstraints.Clear();
  if(symRel.LinConstraintsSize()>0)
  {
    this->linConstraints.Resize(symRel.LinConstraintsSize());
  }  
  // Kopieren des linConstraints-DBArrays:
  for(i = 0; i < symRel.LinConstraintsSize(); i++ )
  {
    const LinearConstraint *linC;
    symRel.GetLinConstraints( i, linC );
    this->linConstraints.Put( i, *linC );
  }
  // Kopieren des symbolicTuples-DBArrays:
  this->symbolicTuples.Clear();
  if(symRel.SymbolicTuplesSize()>0)
  {
    this->symbolicTuples.Resize(symRel.SymbolicTuplesSize());    
  }  
  for(i = 0; i < symRel.SymbolicTuplesSize(); i++ )
  {
    const SymbolicTuple *iPair;
    symRel.GetSymbolicTuples( i, iPair );
    this->symbolicTuples.Put( i, *iPair );
  }     
  // Kopieren der MBBox:
  this->mbbox = symRel.BoundingBox();
  return *this;
}    

void SymbolicRelation::Destroy() 
{
  this->linConstraints.Destroy();                   
  this->symbolicTuples.Destroy();
}

void SymbolicRelation::GetSymbolicTuples(const int i, 
                          SymbolicTuple const*& SymbolicTuple) const
{
  this->symbolicTuples.Get(i, SymbolicTuple);
}

void SymbolicRelation::GetLinConstraints(const int i, 
                    LinearConstraint const*& linearConstraint) const
{
  this->linConstraints.Get(i, linearConstraint);
}

int SymbolicRelation::LinConstraintsSize() const
{
  return this->linConstraints.Size();
}

int SymbolicRelation::SymbolicTuplesSize() const
{
  return this->symbolicTuples.Size();
}

void SymbolicRelation::addSymbolicTuple(
      const vector<LinearConstraint> vLinConstraints)
{
  // Achtung: setzt mbbox vorerst auf <leer>, 
  // weil diese sowieso bei Normalisierung berechnet wird!  
  int startIndex, endIndex;
  SymbolicTuple tuplePositionInArray;
  tuplePositionInArray.isNormal = false;  
  // append the lin. constraints into the linConstraints-DBArray:
  startIndex = linConstraints.Size();
  endIndex = startIndex;          
  for(int i = 0; i < vLinConstraints.size(); i++) 
  {
    this->linConstraints.Append(vLinConstraints[i]);
    endIndex++;
  }
  tuplePositionInArray.startIndex = startIndex;
  tuplePositionInArray.endIndex = --endIndex;
  // mbbox wird erst bei Normalisierung genau berechnet!
  tuplePositionInArray.mbbox = Rectangle<2>(false); 
  // append the tuple into the symbolicTuples-DBArray:  
  this->symbolicTuples.Append(tuplePositionInArray);            
}

void SymbolicRelation::appendSymbolicRelation(
        const SymbolicRelation& otherSymRel)
{
  //cout << "DEBUG: Start der appendSymbolicRelation-Funktion..." << endl; 
  
  // append the lin. constraints into the linConstraints-DBArray:
  int i;
  const int iOffset = this->LinConstraintsSize();
  for(i = 0; i < otherSymRel.LinConstraintsSize(); i++)
  {
    const LinearConstraint *linC;
    otherSymRel.GetLinConstraints(i, linC);
    this->linConstraints.Append(*linC);
  }

  // append and update the tuple into the symbolicTuples-DBArray:    
  for(i = 0; i < otherSymRel.SymbolicTuplesSize(); i++)
  {
    const SymbolicTuple *iPair;
    otherSymRel.GetSymbolicTuples(i, iPair);
    SymbolicTuple tuplePositionInArray; 
    tuplePositionInArray = *iPair;
    tuplePositionInArray.startIndex = iPair->startIndex + iOffset;
    tuplePositionInArray.endIndex = iPair->endIndex + iOffset;    
    this->symbolicTuples.Append(tuplePositionInArray);            
  } 
  // compute the new MBB:
  if(!this->mbbox.IsDefined())
  {
    this->mbbox = otherSymRel.mbbox;
  }
  else if(otherSymRel.mbbox.IsDefined())
  {
    // if both are defined compute Union of the two MBBs:
    this->mbbox = this->mbbox.Union(otherSymRel.mbbox);
  }
  else
  {
    // if  iSymTuple.mbbox is NOT defined: 
    //do nothing (leave this->mbbox as it is)
  }    
  //cout << "DEBUG: Ende der appendSymbolicRelation-Funktion..." << endl; 
}

void SymbolicRelation::overlapSymbolicRelation(
        const SymbolicRelation& otherSymRel)
{
  //cout << "DEBUG: Start der overlapSymbolicRelation-Funktion..." << endl; 
  int i, j;
  int k1,k2,k3,k4,k;
  vector<LinearConstraint> vlinConstraintsOld;
  vector<SymbolicTuple> vSymbolicTuplesOld;

  if(!this->BoundingBox().IsDefined() || 
     !otherSymRel.BoundingBox().IsDefined() || 
     !this->BoundingBox().Intersects(otherSymRel.BoundingBox()))
  {
    this->linConstraints.Clear();
    this->symbolicTuples.Clear();
    this->mbbox = Rectangle<2>(false);    
  }
  else
  {  
    for(i = 0; i < this->LinConstraintsSize(); i++)
    {
      const LinearConstraint *linC;
      this->GetLinConstraints(i, linC);
      vlinConstraintsOld.push_back(*linC);
    }  
  
    for(i = 0; i < this->SymbolicTuplesSize(); i++)
    {
      const SymbolicTuple *symTuple;
      this->GetSymbolicTuples(i, symTuple);
      vSymbolicTuplesOld.push_back(*symTuple);
    }  
    this->linConstraints.Clear();
    this->symbolicTuples.Clear();
    
    for(i = 0; i < vSymbolicTuplesOld.size(); i++)
    {
      for(j = 0; j < otherSymRel.SymbolicTuplesSize(); j++)
      {        
        const SymbolicTuple *symTuple;
        otherSymRel.GetSymbolicTuples(j, symTuple);
        if(vSymbolicTuplesOld[i].mbbox.IsDefined() && 
           symTuple->mbbox.IsDefined() && 
           vSymbolicTuplesOld[i].mbbox.Intersects(symTuple->mbbox))
        {
          k1 = vSymbolicTuplesOld[i].startIndex;
          k2 = vSymbolicTuplesOld[i].endIndex+1;
          k3 = symTuple->startIndex;
          k4 = symTuple->endIndex+1;
          vector<LinearConstraint> vlinConstraints2Add;
          for(k=k1; k < k2; k++)
          {
            vlinConstraints2Add.push_back(vlinConstraintsOld[k]);          
          } 
          for(k=k3; k < k4; k++)
          {
            const LinearConstraint *linC;
            otherSymRel.GetLinConstraints(k, linC);          
            vlinConstraints2Add.push_back(*linC);          
          }        
          addSymbolicTuple(vlinConstraints2Add);
        }
      }
    }           
  }
  //cout << "DEBUG: Ende der overlapSymbolicRelation-Funktion..." << endl; 
}

void SymbolicRelation::Normalize()
{
  // Normalization of all the symbolic tuples in the symbolic relation:

  int i,k;
  int linConIndex, lastEndIndex;
  bool IsRealSubSet, blnRelIsEmptySet;
  vector<LinearConstraint> vLinConstraints2Save;
  vector<SymbolicTuple> vSymbolicTuples2Save;
  this->mbbox = Rectangle<2>(false); 
  blnRelIsEmptySet = true;
  lastEndIndex = -1;  
  for(i = 0; i < this->SymbolicTuplesSize(); i++ )
  {
    vector<LinearConstraint> vCEQ;
    vector<LinearConstraint> vNEQ;    
    vector<LinearConstraint> vLinConstraints;
    vector<Point2D> vConvexPolygon;
    const SymbolicTuple *iGetSymTuple;
    this->GetSymbolicTuples( i, iGetSymTuple );
    SymbolicTuple iSymTuple; 
      
    ///////////////////////////////
    // fill up vCEQ, vNEQ:
    ///////////////////////////////   
    IsRealSubSet = false;
    // to handle special case R^2:
    // 
    // (all constraints have the form 
    // 0x+0y+0 OP 0 OR
    // 0x+0y+<negative value> <= 0)
    //  <=> constraints decribes R^2 
    //  <=> constraints decribes NOT a real subset of R^2   
    //  <=> IsRealSubSet = false;
    // 
    // that is equivalent to:
    //
    // IsRealSubSet = true;
    //  <=> constraints describes a real subset of R^2 
    //  <=> (there exist at least one constraint with the form
    //      NOT 0x+0y+0 OP 0 AND
    //      NOT 0x+0y+<negative value> <= 0)
    linConIndex=iGetSymTuple->startIndex;
    do
    {
      const LinearConstraint *linC;
      this->GetLinConstraints(linConIndex, linC);
      if(!(linC->get_a1()==0 && linC->get_a2()==0 && linC->get_b()==0) &&
         !(linC->get_a1()==0 && linC->get_a2()==0 && 
           linC->get_b()<=0 && linC->get_Op()==OP_LEQ))
      {
        IsRealSubSet = true;
      }        
      linConIndex++;
    } 
    while(!IsRealSubSet && linConIndex<=iGetSymTuple->endIndex);
      
    for(linConIndex=iGetSymTuple->startIndex; 
        linConIndex<=iGetSymTuple->endIndex; linConIndex++)
    {
      //cout << "DEBUG: step 0.1" << endl; 
      const LinearConstraint *linC;
      this->GetLinConstraints(linConIndex, linC);
      if(linC->get_a1()!=0 || linC->get_a2()!=0 || linC->get_b()!=0)
      {
        // if at minimum one of the coefficients is not equal 0
        // (otherwise we can ignore this linear constraint representing R^2
        // because there is an other constraint which intersects R^2)
        if(linC->get_Op()==OP_EQ)
        {
          vCEQ.push_back(*linC);  
        }
        else // OP_LEQ
        {
          vNEQ.push_back(*linC);        
        }
      }        
    }      
    // OK, now vCEQ and cNEQ are correctly filled up!

    // Test about vCEQ, vNEQ:
    //cout << "DEBUG: #Elemente von vCEQ = " << vCEQ.size() << endl;     
    //cout << "DEBUG: #Elemente von vNEQ = " << vNEQ.size() << endl;
           
    if(IsRealSubSet) 
    {
      if(vCEQ.size() > 0)
      {
        for(k=0; k < vCEQ.size(); k++)
        {
          // Transformation of equation to equivalent unequations: 
          // (a1*x+a2*y+b=0) <=> (a1*x+a2*y+b<=0) AND (-a1*x-a2*y-b<=0)
          LinearConstraint linConstraint1(vCEQ[k].get_a1(), 
                      vCEQ[k].get_a2(), vCEQ[k].get_b(), OP_LEQ);
          LinearConstraint linConstraint2(-vCEQ[k].get_a1(), 
                      -vCEQ[k].get_a2(), -vCEQ[k].get_b(), OP_LEQ);
          vNEQ.push_back(linConstraint1);
          vNEQ.push_back(linConstraint2);
        }
      }
      HalfPlaneIntersection(vNEQ, vConvexPolygon);
    }
    else
    {
      // if current symbolic tuple represents R^2 then 
      // we just use the WORLD in order to border it:
      vConvexPolygon.clear();
      const double wxLeft = WORLD.MinD(0);
      const double wxRight = WORLD.MaxD(0);  
      const double wyBottom = WORLD.MinD(1);
      const double wyTop = WORLD.MaxD(1);            
      Point2D pUppLeft(wxLeft, wyTop);
      Point2D pUppRight(wxRight, wyTop);
      Point2D pLowRight(wxRight, wyBottom);
      Point2D pLowLeft(wxLeft, wyBottom);          
      vConvexPolygon.push_back(pUppLeft);
      vConvexPolygon.push_back(pUppRight);
      vConvexPolygon.push_back(pLowRight);
      vConvexPolygon.push_back(pLowLeft);
    }
    //cout << "DEBUG: vNEQ: " << endl;
    //for(j=0; j< vNEQ.size(); j++)
    //{
    //  vNEQ[j].printOut();
    //}                
        
    //cout << "DEBUG: vConvexPolygon: " << endl;
    //for(j=0; j< vConvexPolygon.size(); j++)
    //{
    //  vConvexPolygon[j].printOut();
    //}      
    vLinConstraints.clear();
    if(vConvexPolygon.size()==0)
    {
      // Spezialfall: unerfuellbares Tupel 
      // do nothing (tuple will be eliminated)
    }
    else // vConvexPolygon.size() > 0
    {
      // Fall: erfuellbares Tupel:
      blnRelIsEmptySet = false;  
      ToConstraint(vConvexPolygon, vLinConstraints);         
      iSymTuple.isNormal = true;
      iSymTuple.startIndex = lastEndIndex+1;
      iSymTuple.endIndex = lastEndIndex + vLinConstraints.size();
      iSymTuple.type = UNKNOWN;
      iSymTuple.mbbox = MinimumBoundingBox(vConvexPolygon);
      if(!this->mbbox.IsDefined())
      {
        this->mbbox = iSymTuple.mbbox;
      }
      else if(iSymTuple.mbbox.IsDefined())
      {
         // if both are defined compute Union of the two MBBs:
          this->mbbox = this->mbbox.Union(iSymTuple.mbbox);
      }
      else
      {
        // if  iSymTuple.mbbox is NOT defined: 
        // do nothing (leave this->mbbox as it is)
      }
      
      for(k=0; k < vLinConstraints.size(); k++)
      {
        vLinConstraints2Save.push_back(vLinConstraints[k]);  
      } 
      vSymbolicTuples2Save.push_back(iSymTuple);    
      lastEndIndex += vLinConstraints.size();      
    }                 
  } // end for    
  
  this->linConstraints.Clear();
  this->symbolicTuples.Clear();
  
  if(!blnRelIsEmptySet)
  {
    // Now all lineare Constraints will be saved:
    this->linConstraints.Resize(vLinConstraints2Save.size());
    for(i = 0; i < vLinConstraints2Save.size(); i++)
    {
      this->linConstraints.Append(vLinConstraints2Save[i]);    
    }      
    // Now all tuples will be saved:
    this->symbolicTuples.Resize(vSymbolicTuples2Save.size());
    for(i = 0; i < vSymbolicTuples2Save.size(); i++)
    {
      this->symbolicTuples.Append(vSymbolicTuples2Save[i]);
    }          
  }
     
  //cout << "DEBUG: Ende der Normalize-Funktion..." << endl; 
}

// The following functions are needed for the SymbolicRelation 
// class to act as an attribute of a relation:
int SymbolicRelation::NumOfFLOBs() const
{
  cout << "DEBUG: Start & Ende der Funktion NumOfFLOBs()" << endl; 
  return 2;
}

FLOB* SymbolicRelation::GetFLOB(const int i)
{
  cout << "DEBUG: Start & Ende der Funktion GetFLOB(...)" << endl; 
  assert( i >= 0 && i < NumOfFLOBs() );
  if(i==0) 
  {
    return &linConstraints;  
  }
  // else: 
  return &symbolicTuples;
}

int SymbolicRelation::Compare(const Attribute* arg) const
{
  cout << "DEBUG: Start & Ende der Funktion Compare(...)" << endl; 
  return 0; // no special order 
}

bool SymbolicRelation::Adjacent(const Attribute* arg) const
{
  cout << "DEBUG: Start & Ende der Funktion Adjacent(...)" << endl; 
  return false;
}

SymbolicRelation* SymbolicRelation::Clone() const
{
  cout << "DEBUG: Start & Ende der Funktion Clone()" << endl; 
  return new SymbolicRelation(*this);                    
}

bool SymbolicRelation::IsDefined() const
{
  cout << "DEBUG: Start & Ende der Funktion IsDefined()" << endl; 
  return true; // even the empty set is "defined"
}

void SymbolicRelation::SetDefined(bool Defined )
{
  cout << "DEBUG: Start & Ende der Funktion SetDefined(...)" << endl; 
  // nothing to do  
}   

size_t SymbolicRelation::HashValue() const
{
  cout << "DEBUG: Start der Funktion HashValue()" << endl; 
  if(LinConstraintsSize()==0)
  {
    cout << "DEBUG: Ende der Funktion HashValue() mit return(0)" << endl;   
    return (0);
  }
  else
  {
    unsigned long h=0;
    double a1, a2, b;
    for(int i = 0; ((i < LinConstraintsSize())&&(i<5)); i++ )
    {
      const LinearConstraint *linC;
      GetLinConstraints( i, linC );
      a1 = linC->get_a1();
      a2 = linC->get_a2();
      b = linC->get_b();
      h=h+(unsigned long)(5*a1+5*a2+b); // Hash-Funktion      
    }
    cout << "DEBUG: Ende der Funktion HashValue() mit h=" << h << endl;   
    return size_t(h);
  }
}

size_t SymbolicRelation::Sizeof() const
{
  cout << "DEBUG: Start & Ende der Funktion Sizeof()" << endl; 
  return sizeof( *this );
}

void SymbolicRelation::CopyFrom(const StandardAttribute* right)
{ 
  cout << "DEBUG: Start & Ende der Funktion CopyFrom()" << endl; 
  *this = *(const SymbolicRelation*)right; 
}


const Rectangle<2> SymbolicRelation::BoundingBox() const
{
  cout << "DEBUG: Start & Ende der Funktion BoundingBox()" << endl; 
  return  this->mbbox;
}

/*
6 Further functions

6.1 ~MinimumBoundingBox~-function for a given polygon

*/
Rectangle<2> MinimumBoundingBox(const vector<Point2D>& vConvexPolygon)
{  
  if(vConvexPolygon.size()==0)
  {
    return Rectangle<2>(false); 
  }
  else
  {
    double x_min, x_max, y_min, y_max;   
    x_min = vConvexPolygon[0].x;
    x_max = vConvexPolygon[0].x;
    y_min = vConvexPolygon[0].y;
    y_max = vConvexPolygon[0].y;    
    for(int i=0; i < vConvexPolygon.size(); i++)
    {
      if(vConvexPolygon[i].x < x_min)
      {
        x_min = vConvexPolygon[i].x;
      }
      if(vConvexPolygon[i].x > x_max)
      {
        x_max = vConvexPolygon[i].x;
      }
      if(vConvexPolygon[i].y < y_min)
      {
        y_min = vConvexPolygon[i].y;
      }
      if(vConvexPolygon[i].y > y_max)
      {
        y_max = vConvexPolygon[i].y;
      }       
    }
    return Rectangle<2>(true, x_min, x_max, y_min, y_max); 
  }
}

/*
6.2 ~ToConstraint~-function for a given bounding box (rectangle)

*/
void ToConstraint(const Rectangle<2>& mbbox, 
      vector<LinearConstraint>& vLinConstraints) 
{
  // Eingabe: 2-dimensionale MBB 
  // (degenerierte Faelle moeglich: Segment, Punkt; nicht moeglich: leere Menge)
  //          gegeben als rectangle<2>-Objekt
  // Ausgabe: Vektor mit lienearen Constraints, welche die Eingabe beschreiben
  //cout << "DEBUG: Start der ToConstraint-Funktion (MBB-case)..." << endl; 
  vLinConstraints.clear();
  if(mbbox.IsDefined())
  {
    vector<Point2D> vMainVertexesOfMbb;
    Point2D p1(mbbox.MinD(0), mbbox.MinD(1));
    Point2D p2(mbbox.MinD(0), mbbox.MaxD(1));
    Point2D p3(mbbox.MaxD(0), mbbox.MaxD(1));
    Point2D p4(mbbox.MaxD(0), mbbox.MinD(1));
    vMainVertexesOfMbb.push_back(p1);
    vMainVertexesOfMbb.push_back(p2);
    vMainVertexesOfMbb.push_back(p3);
    vMainVertexesOfMbb.push_back(p4);
    ToConstraint(vMainVertexesOfMbb, vLinConstraints);
  }
  //cout << "DEBUG: Ende der ToConstraint-Funktion (MBB-case)..." << endl;   
}  

/*
6.3 ~ToConstraint~-function for a given polygon

*/
void ToConstraint(const vector<Point2D>& vConvexPolygon, 
        vector<LinearConstraint>& vLinConstraints)
{
  // Eingabe: konvexes Polygon (degenerierte Faelle moeglich: 
  //          Segment, Punkt; nicht moeglich: leere Menge)
  //          gegeben als nicht-leerer Vektor von Point2D-Objekten in cw-order
  // Ausgabe: Vektor mit lienearen Constraints, welche die Eingabe beschreiben
  //          lineare Constraints in cw-order (!) 
  //          d.h. zwei nacheinander folgende Constraints schneiden 
  //          sich immer in einem Punkt!
  //cout << "DEBUG: Start der ToConstraint-Funktion..." << endl; 
  
  int i,j,k;
  bool blnAllEqual;
  bool blnAllOnSameLine;
  
  // Are they (the points defining the convex polygon) all the same?
  blnAllEqual = true;
  for(i=0; i < vConvexPolygon.size(); i++)
  {
    if(vConvexPolygon[i]!=vConvexPolygon[0])
    {
      blnAllEqual = false;
      j = i;
      break; 
    } 
  }  
  // now ether (i)  blnAllEqual==true 
  //     or  blnAllEqual==false AND vConvexPolygon[j]!=vConvexPolygon[0]
  
  if(blnAllEqual)
  {
    // CASE (i) (blnAllEqual==true) behandeln
    LinearConstraint linC1(1.0, 0.0, -vConvexPolygon[0].x, OP_EQ);    
    LinearConstraint linC2(0.0, 1.0, -vConvexPolygon[0].y, OP_EQ);    
    vLinConstraints.push_back(linC1);
    vLinConstraints.push_back(linC2);
  }
  else // blnAllEqual==false AND vConvexPolygon[j]!=vConvexPolygon[0]
  {
    // are they all on the same line?
    // (the line is defined by the non-equal points 
    // vConvexPolygon[0], vConvexPolygon[j])
    double x_min, x_max, y_min, y_max;
    x_min = vConvexPolygon[0].x;
    x_max = vConvexPolygon[0].x;
    y_min = vConvexPolygon[0].y;
    y_max = vConvexPolygon[0].y;    
    blnAllOnSameLine = true;
    for(i=j; i < vConvexPolygon.size(); i++)
    {
      if(vConvexPolygon[i].OnSameLineAs(vConvexPolygon[0], vConvexPolygon[j]))
      {
        //cout << "DEBUG: OnSameLineAs==true " << endl; 
        if(vConvexPolygon[i].x < x_min)
        {
          x_min = vConvexPolygon[i].x;
        }
        if(vConvexPolygon[i].x > x_max)
        {
          x_max = vConvexPolygon[i].x;
        }
        if(vConvexPolygon[i].y < y_min)
        {
          y_min = vConvexPolygon[i].y;
        }
        if(vConvexPolygon[i].y > y_max)
        {
          y_max = vConvexPolygon[i].y;
        }
        j = i; 
        // j will be (after leving the for-construct) the index 
        // of the last point on the same line than 
        // vConvexPolygon[0], vConvexPolygon[j] before there was 
        // a point wich haven't been on the same line in cw-direction
      }
      else
      {
        blnAllOnSameLine = false;
        k = i;
        break; 
      } 
    }

    // now ether (ii)  blnAllOnSameLine==true AND 
    // mbb(x_min, x_max, y_min, y_max) bounds the line with the two 
    // diffrent points vConvexPolygon[0], vConvexPolygon[j]
    // 
    //     or    (iii) blnAllOnSameLine==false AND 
    //                the three points vConvexPolygon[0], 
    //                vConvexPolygon[j], vConvexPolygon[k] are 
    //                all not-equal to each other and are not 
    //                all on the same line!
    if(blnAllOnSameLine)
    {
      // handle CASE (ii): 
      if(AlmostEqual(vConvexPolygon[0].x, vConvexPolygon[j].x)) 
      {                                       
        // special case: vertical line
        LinearConstraint linConLine(1.0, 0.0, -vConvexPolygon[0].x, OP_EQ);
        LinearConstraint linConBottom(0.0, -1.0, y_min, OP_LEQ);        
        LinearConstraint linConTop(0.0, 1.0, -y_max, OP_LEQ);                
        vLinConstraints.push_back(linConLine);
        vLinConstraints.push_back(linConBottom);        
        vLinConstraints.push_back(linConTop);
      }  
      else // vConvexPolygon[0].x!=vConvexPolygon[j].x
      {
        double m,b; // Geradengleichung y = mx+b
        //             fuehrt zu (mx -y +b = 0)
        m = (vConvexPolygon[j].y-vConvexPolygon[0].y)/
            (vConvexPolygon[j].x-vConvexPolygon[0].x); 
        // m definiert wegen (vConvexPolygon[0].x!=vConvexPolygon[j].x)
        b = vConvexPolygon[0].y - m*vConvexPolygon[0].x;
        LinearConstraint linConLine(m, -1.0, b, OP_EQ);     
        LinearConstraint linConLeft(-1.0, 0.0, x_min, OP_LEQ);          
        LinearConstraint linConRight(1.0, 0.0, -x_max, OP_LEQ);
        vLinConstraints.push_back(linConLine); 
        vLinConstraints.push_back(linConLeft);                          
        vLinConstraints.push_back(linConRight);
      }            
                 
    }
    else
    {
      // handle CASE (iii):
      //cout << "DEBUG: step 3" << endl; 
      vector<int> vIndexOfVertexes; // non-redundant vertexes
      vIndexOfVertexes.push_back(0);
      vIndexOfVertexes.push_back(j);
      vIndexOfVertexes.push_back(k);      
      j = 2;
      for(i=k+1; i < vConvexPolygon.size(); i++)
      {
        if(vConvexPolygon[i]!=vConvexPolygon[vIndexOfVertexes[j]])
        {
          if(vConvexPolygon[vIndexOfVertexes[j]].OnSameLineAs(
                vConvexPolygon[vIndexOfVertexes[j-1]],
                vConvexPolygon[i]))  
          {
            vIndexOfVertexes[j] = i;
          }
          else
          {
            vIndexOfVertexes.push_back(i);
            j++;              
          }             
        }
      }
      
      // now we have to post-handle the special case 
      // i=vConvexPolygon.size() (the last vertex)
      if(vConvexPolygon[vIndexOfVertexes[j]].OnSameLineAs(
            vConvexPolygon[vIndexOfVertexes[j-1]], 
            vConvexPolygon[vIndexOfVertexes[0]]))
      {
        // then vIndexOfVertexes[j] (the last vertex) is redundant 
        // and have to be removed:
        vIndexOfVertexes.pop_back();  
        j--;
      }

      if(vConvexPolygon[vIndexOfVertexes[0]].OnSameLineAs(
            vConvexPolygon[vIndexOfVertexes[j]], 
            vConvexPolygon[vIndexOfVertexes[1]]))
      {
        // then vIndexOfVertexes[0] (the first vertex) is 
        // redundant and have to be removed:
        vIndexOfVertexes.erase(vIndexOfVertexes.begin());  
      }
        
      // now vIndexOfVertexes contains the indexes of vConvexPolygon 
      //wich leads to a non-rednundant polygon in cw-order 
      // vConvexPolygon[vIndexOfVertexes[0]],...,
      // vConvexPolygon[vIndexOfVertexes[vIndexOfVertexes.size()]]
      // with vIndexOfVertexes.size() >= 3
      Point2D pFirst, pSecond, pOther; 
      for(i=0; i < vIndexOfVertexes.size(); i++)
      {
        pFirst = vConvexPolygon[vIndexOfVertexes[i]];
        // determine pSecond:
        if(i < (vIndexOfVertexes.size()-1))
        {
          pSecond = vConvexPolygon[vIndexOfVertexes[i+1]];
        }
        else 
        {
            pSecond = vConvexPolygon[vIndexOfVertexes[0]];
        }
        // determine pOther:          
        if(i < (vIndexOfVertexes.size()-2))
        {
          pOther = vConvexPolygon[vIndexOfVertexes[i+2]];
        }
        else 
        {
          if(i == (vIndexOfVertexes.size()-2))
          {
            pOther = vConvexPolygon[vIndexOfVertexes[0]];
          }
          else // i == (vIndexOfVertexes.size()-1)
          {
            pOther = vConvexPolygon[vIndexOfVertexes[1]];
          }
        }
          
        // now we have to calculate the linear constraint (OP_LEQ!)
        // which bounding-line of its half-plane includes the none-equal 
        // points pFirst, pSecond and its half-plane contains the point 
        // pOther (pOther doesn't lie on the bounding-line!)

        LinearConstraint linC;
        double leq_factor;
        if(AlmostEqual(pFirst.x,pSecond.x))
        {                        
          if(pOther.x < pFirst.x)
          {
            leq_factor = 1.0;
          }
          else
          {
            leq_factor = -1.0;
          }
          linC.set_a1(leq_factor);
          linC.set_a2(0.0);
          linC.set_b(leq_factor*(-pFirst.x));            
          linC.set_Op(OP_LEQ);            
        }
        else // pFirst.x!=pSecond.x
        {
          double m,b; // Geradengleichung y = mx+b
          //             fuehrt zu (mx -y +b = 0)
          m = (pSecond.y-pFirst.y)/(pSecond.x-pFirst.x); 
          // m definiert wegen (pFirst.x!=pSecond.x)
          b = pFirst.y - m*pFirst.x;
          if((m*pOther.x - pOther.y + b) < 0)
          {
            leq_factor = 1.0;              
          }
          else 
          {
            // m*pOther.x - pOther.y + b > 0 (">" not ">=" because 
            // pOther is not on the same line!)
            leq_factor = -1.0;
          }
          linC.set_a1(leq_factor*m);
          linC.set_a2(leq_factor*(-1.0));
          linC.set_b(leq_factor*b);            
          linC.set_Op(OP_LEQ);  
        }
        //cout << "DEBUG: step 6" << endl; 
        vLinConstraints.push_back(linC);           
      }        
    }
  }     
  //cout << "DEBUG: Ende der ToConstraint-Funktion..." << endl;   
}  

/*
6.4 ~ComputeXOrderedSequenceOfSlabs~-function 

*/
void ComputeXOrderedSequenceOfSlabs(const vector<Point2D>& vP, 
            vector<Point2D>& vUpperBoundary, vector<Point2D>& vLowerBoundary)
{
  // Eingabe: konvexes Polygon vP 
  //          (degenerierte Faelle nicht moeglich: Segment, Punkt, leere Menge)
  //          gegeben als nicht-leerer (mindestens 3 Elemente) Vektor 
  //          von Point2D-Objekten ohne Dublikate und 
  //          ohne redundante Punkte (cw-order!)
  // Ausgabe: Vektoren von Point2D-Objekten mit jeweils oberen 
  //          und unteren Boundary
  int i;
  
  vUpperBoundary.clear();
  vLowerBoundary.clear();
  
  const Rectangle<2> mbbP = MinimumBoundingBox(vP);
  const double x_min_P = mbbP.MinD(0);
  const double x_max_P = mbbP.MaxD(0);
  
  bool blnScanEnd; 
  int i_x_minFirst = -1;
  int i_x_minSecond = -1;
  int i_x_maxFirst = -1;
  int i_x_maxSecond = -1;
  
  for(i=0; i<vP.size(); i++)
  {
    if(vP[i].x==x_min_P && i_x_minFirst==-1)
    {
      i_x_minFirst = i;
    } 
    else if(vP[i].x==x_min_P && i_x_minSecond==-1)
    {
       i_x_minSecond = i;
    }    
    else if(vP[i].x==x_min_P)
    {
      // darf nicht vorkommen !!!
        cout << "DEBUG: FALSCHE EINGABE: "
        "reduntante Polygon-Punkte auf linker Kante!!!" << endl; 
    }
    if(vP[i].x==x_max_P && i_x_maxFirst==-1)
    {
      i_x_maxFirst = i;
    } 
    else if(vP[i].x==x_max_P && i_x_maxSecond==-1)
    {
       i_x_maxSecond = i;
    }    
    else if(vP[i].x==x_max_P)
    {
      // darf nicht vorkommen !!!
        cout << "DEBUG: FALSCHE EINGABE: "
        "reduntante Polygon-Punkte auf rechter Kante!!!" << endl; 
    }        
  }
  
  
  if(i_x_minSecond!=-1 && !(i_x_minSecond==i_x_minFirst+1 || 
          (i_x_minSecond==vP.size()-1 && i_x_minFirst==0)))
  {
    // darf nicht vorkommen !!!
    cout << "DEBUG: FALSCHE EINGABE: "
    "reduntante Polygon-Punkte auf linker Kante!!!" << endl;     
  }
    
  // Compute vUpperBoundary:
  blnScanEnd = false;
  if(i_x_minSecond==-1 || vP[i_x_minFirst].y > vP[i_x_minSecond].y)
  {
    i = i_x_minFirst;  
  } 
  else
  {
     i = i_x_minSecond;  
  }  
  while(!blnScanEnd)
  {
    vUpperBoundary.push_back(vP[i % vP.size()]);
    i++;
    if(i_x_maxSecond==-1 || (vP[i_x_maxFirst].y > vP[i_x_maxSecond].y))
    {      
       // (d.h. nur First ODER First oben, Second unten)
      blnScanEnd = ((i % vP.size()) == ((i_x_maxFirst+1) % vP.size()));
    }
    else // (d.h. First unten, Second oben)
    {
      blnScanEnd = ((i % vP.size()) == ((i_x_maxSecond+1) % vP.size()));
    }
  }

  // Compute vLowerBoundary:
  blnScanEnd = false;
  if(i_x_maxSecond==-1 || vP[i_x_maxFirst].y < vP[i_x_maxSecond].y)
  {
    i = i_x_maxFirst;  
  } 
  else
  {
     i = i_x_maxSecond;  
  }  
  while(!blnScanEnd)
  {
    vLowerBoundary.push_back(vP[i % vP.size()]);
    i++;
    if(i_x_minSecond==-1 || (vP[i_x_minFirst].y < vP[i_x_minSecond].y)) 
    {      
      // (d.h. nur First ODER First unten, Second oben)
      blnScanEnd = ((i % vP.size()) == ((i_x_minFirst+1) % vP.size()));
    }
    else // (d.h. First oben, Second unten)
    {
      blnScanEnd = ((i % vP.size()) == ((i_x_minSecond+1) % vP.size()));
    }
  }
  reverse(vLowerBoundary.begin(), vLowerBoundary.end());  
}

/*
6.5 ~printSlab~-function  (just for debuging)

*/
void printSlab(VerticalTrapez currentSlab)
{
   cout << endl << "DEBUG: currentSlab:" << endl; 
   cout << "DEBUG: -> currentSlab.IsEmpty == " << currentSlab.IsEmpty << endl; 
   if(!currentSlab.IsEmpty)
   {
    cout << "DEBUG: -> currentSlab.pUppLeft == (" 
         << currentSlab.pUppLeft.x << ", " 
         << currentSlab.pUppLeft.y << ")" << endl; 
    cout << "DEBUG: -> currentSlab.pUppRight == (" 
         << currentSlab.pUppRight.x << ", " 
         << currentSlab.pUppRight.y << ")" << endl; 
    cout << "DEBUG: -> currentSlab.pLowLeft == (" 
         << currentSlab.pLowLeft.x << ", " 
         << currentSlab.pLowLeft.y << ")" << endl; 
    cout << "DEBUG: -> currentSlab.pLowRight == (" 
         << currentSlab.pLowRight.x << ", " 
         << currentSlab.pLowRight.y << ")" << endl;    
  }
  
}

/*
6.6 ~GetPointFromSegment~-function  

*/
Point2D GetPointFromSegment(const double xCut, 
      const Point2D& pLeft, const Point2D& pRight)
{  
  // prerequisite: pLeft.x < xCut < pRight.x 
  double m = (pRight.y-pLeft.y)/(pRight.x-pLeft.x);
  double b = pLeft.y - m*pLeft.x;
  Point2D pointCut(xCut, m*xCut + b);
  return pointCut;
}

/*
6.7 ~GetIntersectionPoint~-function  

*/
void GetIntersectionPoint(
      const Point2D& pSegment1First, 
      const Point2D& pSegment1Second, 
      const Point2D& pSegment2First, 
      const Point2D& pSegment2Second, 
      bool& IsPoint, Point2D& pIntersection)
{
  // Input: two Segemnts
  // prerequisite: pSegment1First != pSegment1Second && 
  //               pSegment2First != pSegment2Second 
  // Output:
  // returns IsPoint==true if there is a single intersection point pIntersection
  // returns IsPoint==false if there is either no intersections point OR 
  //  more than one sinlge intersection point
  
  
  bool blnSegment1Vertical;
  Point2D pSegment1Upp, pSegment1Low;
  Point2D pSegment1Left, pSegment1Right;
  if(pSegment1First.x==pSegment1Second.x)
  {
    blnSegment1Vertical = true;
    if(pSegment1First.y < pSegment1Second.y)
    {
      pSegment1Upp = pSegment1Second;
      pSegment1Low = pSegment1First;
    }
    else
    {
      pSegment1Upp = pSegment1First;
      pSegment1Low = pSegment1Second;        
    }    
  }
  else // Segment1 is not vertical
  {
    blnSegment1Vertical = false;
    if(pSegment1First.x < pSegment1Second.x)
    {
      pSegment1Left = pSegment1First;
      pSegment1Right = pSegment1Second;
    }
    else
    {
      pSegment1Left = pSegment1Second;
      pSegment1Right = pSegment1First;      
    } 
  }

  bool blnSegment2Vertical;
  Point2D pSegment2Upp, pSegment2Low;
  Point2D pSegment2Left, pSegment2Right;
  if(pSegment2First.x==pSegment2Second.x)
  {
    blnSegment2Vertical = true;
    if(pSegment2First.y < pSegment2Second.y)
    {
      pSegment2Upp = pSegment2Second;
      pSegment2Low = pSegment2First;
    }
    else
    {
      pSegment2Upp = pSegment2First;
      pSegment2Low = pSegment2Second;        
    }    
  }
  else // Segment2 is not vertical
  {
    blnSegment2Vertical = false;
    if(pSegment2First.x < pSegment2Second.x)
    {
      pSegment2Left = pSegment2First;
      pSegment2Right = pSegment2Second;
    }
    else
    {
      pSegment2Left = pSegment2Second;
      pSegment2Right = pSegment2First;      
    } 
  }
    
  if(blnSegment1Vertical && blnSegment2Vertical)
  {
    if(pSegment1First.x == pSegment2First.x)
    {
      // two vertical segments with same x-coordnates 
      // can intersect in one single point
      if(pSegment1Upp.y==pSegment2Low.y)
      {
        IsPoint = true;
        pIntersection = pSegment1Upp;
      }         
      else if(pSegment2Upp.y==pSegment1Low.y)
      {
        IsPoint = true;
        pIntersection = pSegment2Upp;
      }
      else
      {
        IsPoint = false; 
      }
    }
    else
    { 
      // two vertical segments with different 
      // x-coordnates can't intersect at all
      IsPoint = false;
    }
  }
  else if((blnSegment1Vertical && !blnSegment2Vertical) || 
         (!blnSegment1Vertical && blnSegment2Vertical))
  {   
    Point2D pSegmentVerticalUpp, pSegmentVerticalLow, 
        pSegmentNonVerticalLeft, pSegmentNonVerticalRight; 
    if(blnSegment1Vertical)
    {
      pSegmentVerticalUpp = pSegment1Upp;
      pSegmentVerticalLow = pSegment1Low;
      pSegmentNonVerticalLeft = pSegment2Left;
      pSegmentNonVerticalRight = pSegment2Right;
    }
    else // blnSegment2Vertical==true
    {
      pSegmentVerticalUpp = pSegment2Upp;
      pSegmentVerticalLow = pSegment2Low;      
      pSegmentNonVerticalLeft = pSegment1Left;
      pSegmentNonVerticalRight = pSegment1Right;
    }
    double m = (pSegmentNonVerticalRight.y-pSegmentNonVerticalLeft.y)/
          (pSegmentNonVerticalRight.x-pSegmentNonVerticalLeft.x);
    double b = pSegmentNonVerticalLeft.y - m*pSegmentNonVerticalLeft.x;
    double y = m*pSegmentVerticalUpp.x+b;
    if((pSegmentNonVerticalLeft.x <= pSegmentVerticalUpp.x) && 
      (pSegmentVerticalUpp.x <= pSegmentNonVerticalRight.x) &&
       (pSegmentVerticalLow.y <= y) && (y <= pSegmentVerticalUpp.y))
    {
      IsPoint = true;
      pIntersection = Point2D(pSegmentVerticalUpp.x, y);      
    }
    else
    {
      IsPoint = false;
    }
  }
  else // (!blnSegment1Vertical && !blnSegment2Vertical)
  {
    double m1 = (pSegment1Right.y-pSegment1Left.y)/
          (pSegment1Right.x-pSegment1Left.x);
    double b1 = pSegment1Left.y - m1*pSegment1Left.x;
    double m2 = (pSegment2Right.y-pSegment2Left.y)/
          (pSegment2Right.x-pSegment2Left.x);
    double b2 = pSegment2Left.y - m2*pSegment2Left.x;     
    if(AlmostEqual(m1, m2) && !AlmostEqual(b1, b2))
    {
      // parallel but not on same line => emtpy set
      IsPoint = false;
    }
    else if(AlmostEqual(m1, m2) && AlmostEqual(b1, b2))
    {
      // on same line 
      if(pSegment1Right==pSegment2Left)
      {
        IsPoint = true;
        pIntersection = pSegment1Right;    
      }
      else if(pSegment2Right==pSegment1Left)
      {
        IsPoint = true;
        pIntersection = pSegment2Right;    
      }
      else
      {
        // intersecting in more than one single point!
        IsPoint = false;        
      }
    }
    else // AlmostEqual(m1, m2)==false
    {
      // let's compute the intersection point of the non-parallel 
      // segments Segment1, Segment2:
      double x = (b2-b1)/(m1-m2);
      double y = m1*x + b1;
      if((pSegment1Left.x <= x) && (x <= pSegment1Right.x) && 
         (pSegment2Left.x <= x) && (x <= pSegment2Right.x))
      {
         // if the intersection point is in both x-intervalls of the 
         // two segements Segment1, Segment2, then it's a 
         // valid intersection point:
        IsPoint = true;
        pIntersection = Point2D(x,y);               
      }                   
      else
      {
         IsPoint = false;
      }      
    }
  }
  return;
}

/*
6.8 ~ConvexPolygonIntersection~-function  

*/
void ConvexPolygonIntersection(const vector<Point2D>& vP, 
        const vector<Point2D>& vQ, 
        vector<Point2D>& vPQIntersection)
{
  // Eingabe: konvexe Polygone vP und vQ (degenerierte Faelle moeglich: 
  //          Segment, Punkt; leere Menge)
  //          gegeben als nicht-leerer Vektor von Point2D-Objekten 
  //          ohne Dublikate und 
  //          ohne redundante Punkte (cw-order!)
  // Ausgabe: konvexes Polygon vPQIntersection 
  //        (ohne Dublikate und ohne redundante Punkte in cw-order), 
  //        welches den Durchschnitt von vP und vQ bildet.
  int i;
  vector<Point2D> vPQIntersectionWD; // with possibly dublicates  
  vPQIntersection.clear();
  if(vP.size()==0 || vQ.size()==0)
  {
    return;
  }
  else if(vP.size()==1 && vQ.size()==1)
  {
    // then the intersection is a point or the empty set
    if(vP[0]==vQ[0])
    {
      vPQIntersectionWD.push_back(vP[0]);          
    }
  }
  else if((vP.size()==1 && vQ.size()==2) || (vP.size()==2 && vQ.size()==1))
  {
    const Point2D* pSinglePoint; 
    const vector<Point2D>* vTwoElementPointSet;
    if(vP.size()==1)
    {
      pSinglePoint = &vP[0];
      vTwoElementPointSet = &vQ;
    }
    else // vQ.size()==1
    {
      pSinglePoint = &vQ[0];
      vTwoElementPointSet = &vP;
    }
    if((*vTwoElementPointSet)[0].x==(*vTwoElementPointSet)[1].x)
    {
      // vertical case
      if(pSinglePoint->OnSameLineAs((*vTwoElementPointSet)[0],
               (*vTwoElementPointSet)[1])
        && (*vTwoElementPointSet)[0].y <= (*pSinglePoint).y &&
               (*pSinglePoint).y <= (*vTwoElementPointSet)[0].y)
      {
         vPQIntersectionWD.push_back(*pSinglePoint);
      }      
    }
    else
    {
      // non-vertical case
      if(pSinglePoint->OnSameLineAs((*vTwoElementPointSet)[0], 
            (*vTwoElementPointSet)[1])
        && (*vTwoElementPointSet)[0].x <= (*pSinglePoint).x && 
                (*pSinglePoint).x <= (*vTwoElementPointSet)[0].x)
      {
         vPQIntersectionWD.push_back(*pSinglePoint);
      }          
    }
  }
  else if(vP.size()==2 && vQ.size()==2)
  {
    if((vP[0]==vQ[0] && vP[1]==vQ[1]) || (vP[0]==vQ[1] && vP[1]==vQ[0]))
    {
      // if P and Q defines the same line
       vPQIntersectionWD.push_back(vP[0]); 
       vPQIntersectionWD.push_back(vP[1]); 
    }
    else
    {    
      bool blnSExist;
      Point2D pS;
      GetIntersectionPoint(vP[0], vP[1], vQ[0], vQ[1], blnSExist, pS);    
      if(blnSExist)
      {
        vPQIntersectionWD.push_back(pS); 
      }
    }
  }  
  else if((vP.size()==1 && vQ.size()>=3) || (vQ.size()==1 && vP.size()>=3))
  { 
    // There are two possible cases:
    // (i)   the point is inside the convex polygon
    // (ii)   the point is really outside the convex polygon
    Point2D pSinglePoint;
    bool blnSinglePointInside;
    const vector<Point2D>* vPolygon;    
    vector<LinearConstraint> vLinConstraints;
    if(vP.size()==1)
    {
      pSinglePoint = vP[0];
      vPolygon = &vQ;
    }
    else
    {
      pSinglePoint = vQ[0]; 
      vPolygon = &vP;      
    }
    ToConstraint(*vPolygon, vLinConstraints);    
    blnSinglePointInside = true;
    //cout << "DEBUG: ToConstraint(*vPolygon, vLinConstraints) ergibt 
    //fuer Vektor vLinConstraints:" << endl;
    for(i=0; i<vLinConstraints.size(); i++)
    {
      double dblSinglePoint =  vLinConstraints[i].get_a1()*pSinglePoint.x + 
       vLinConstraints[i].get_a2()*pSinglePoint.y + vLinConstraints[i].get_b();
      if(dblSinglePoint > 0)
      {
        blnSinglePointInside = false;
        break;
      }
    }  
    if(blnSinglePointInside)
    {
      vPQIntersectionWD.push_back(pSinglePoint);
    }
  }  
  else if((vP.size()==2 && vQ.size()>=3) || (vQ.size()==2 && vP.size()>=3))
  { 
    // There are three possible cases:
    // (i)   both endpoints of the segment are inside the convex polygon
    // (ii)  one endpoint of the segment is inside the convex polygon 
    //       whereas the other one is really outside    
    // (iii) both endpoints of the segment are really outside the convex polygon
    Point2D pSegmentFirst, pSegmentSecond;
    bool blnSegmentFirstInside, blnSegmentSecondInside;
    const vector<Point2D>* vPolygon;    
    vector<LinearConstraint> vLinConstraints;
    if(vP.size()==2)
    {
      pSegmentFirst = vP[0];
      pSegmentSecond = vP[1]; 
      vPolygon = &vQ;
    }
    else
    {
      pSegmentFirst = vQ[0];
      pSegmentSecond = vQ[1]; 
      vPolygon = &vP;      
    }
    ToConstraint(*vPolygon, vLinConstraints);    
    blnSegmentFirstInside = true;
    blnSegmentSecondInside = true;
    //cout << "DEBUG: ToConstraint(*vPolygon, vLinConstraints) 
    //ergibt fuer Vektor vLinConstraints:" << endl;
    for(i=0; i<vLinConstraints.size(); i++)
    {      
      double dblSegFirst =  vLinConstraints[i].get_a1()*pSegmentFirst.x + 
        vLinConstraints[i].get_a2()*pSegmentFirst.y + 
        vLinConstraints[i].get_b();
      double dblSegSecond =  vLinConstraints[i].get_a1()*pSegmentSecond.x + 
          vLinConstraints[i].get_a2()*pSegmentSecond.y + 
          vLinConstraints[i].get_b();
      if(dblSegFirst > 0)
      {
        blnSegmentFirstInside = false;
      }
      if(dblSegSecond > 0)
      {
        blnSegmentSecondInside = false;
      }
      //cout << "DEBUG: (blnSegmentFirstInside, blnSegmentSecondInside) == 
      // (" << blnSegmentFirstInside << ", " << blnSegmentSecondInside 
      //<< ") @ segment nr " << i << endl;       
    }
    
    
    if(blnSegmentFirstInside && blnSegmentSecondInside)
    {
      //cout << "DEBUG: case (i) both endpoints of the segment are 
      // inside the convex polygon" << endl;
      // case (i) both endpoints of the segment are inside the convex polygon:
      vPQIntersectionWD.push_back(pSegmentFirst);
      vPQIntersectionWD.push_back(pSegmentSecond);
    }
    else // at least one end of the segment is outside the convex polygon:
    { 
      // case (ii) one endpoint of the segment is inside the convex polygon 
      //  whereas the other one is really outside:                    
      // case (iii) both endpoints of the segment are really outside 
      //  the convex polygon:
      // both cases can be handled as follows: 
      Point2D pOut1, pOut2, pOut, pIn;      
      if(blnSegmentFirstInside && !blnSegmentSecondInside)
      {
        pIn = pSegmentFirst;
        pOut = pSegmentSecond;
      }
      else if(!blnSegmentFirstInside && blnSegmentSecondInside)
      {
        pIn = pSegmentSecond;
        pOut = pSegmentFirst;        
      }
      else // !blnSegmentFirstInside && !blnSegmentFirstInside
      {
        pOut1 = pSegmentFirst;
        pOut2 = pSegmentSecond;
      }        
        
      Point2D pVertex1, pVertex2;    
      int nrOfVertexes = 0;
      for(i=0; i<(*vPolygon).size(); i++)
      {
        if(((Point2D)((*vPolygon)[i])).OnSegment(pSegmentFirst, pSegmentSecond))
        {
          nrOfVertexes++;
          if(nrOfVertexes==1)
          {
             pVertex1 = (*vPolygon)[i];
          }
          else if(nrOfVertexes==2)
          {
             pVertex2 = (*vPolygon)[i];
             break;
          }   
        }
      }
      //cout << "DEBUG: case nrOfVertexes == " << nrOfVertexes << endl;         
      // there exist 0, 1, or 2 non-equal vertexes (Eckpunkte!) 
      // of the convex polygon that touches the segment:      
      if(nrOfVertexes==2)
      {
        vPQIntersectionWD.push_back(pVertex1); 
        vPQIntersectionWD.push_back(pVertex2); 
      }
      else if(nrOfVertexes==1)
      {
        if(!blnSegmentFirstInside && !blnSegmentSecondInside)
        {
          vPQIntersectionWD.push_back(pVertex1); 
        }
        else 
        {
          // one endpoint of the segment inside, one outside the 
          // boundary of the convex polygon:
          vPQIntersectionWD.push_back(pIn);
          if(pIn!=pVertex1)
          {
            vPQIntersectionWD.push_back(pVertex1);
          }
        }
      }
      else if(nrOfVertexes==0)
      {
        bool blnExistPx1 = false;
        bool blnExistPx2 = false;
        bool blnIntersectionEdgeFound;
        Point2D pIntersection;
        Point2D pPx1, pPx2;
        for(i=0; i<(*vPolygon).size(); i++)
        { 
          Point2D pPolygonVertex1, pPolygonVertex2;
          if(i==0)
          {
            pPolygonVertex1 = (*vPolygon)[(*vPolygon).size()-1];
          }
          else
          {
            pPolygonVertex1 = (*vPolygon)[i-1];
          }
          pPolygonVertex2 = (*vPolygon)[i];
          
          GetIntersectionPoint(pSegmentFirst, pSegmentSecond, 
                pPolygonVertex1, pPolygonVertex2, 
                blnIntersectionEdgeFound, pIntersection);
          if(blnIntersectionEdgeFound)
          {
            if(!blnExistPx1)
            {
              pPx1 = pIntersection;
              blnExistPx1 = true;
            } 
            else if(!blnExistPx2)
            {
              pPx2 = pIntersection;
              blnExistPx2 = true;
            } 
            else
            {
              cout << "ERROR: bei CPI, Fall (vP.size()==2 && "
              "vQ.size()>=3) || (vQ.size()==2 && vP.size()>=3), "
              "nrOfVertexes==0: mehr als zwei Intersecting-Punkte"
              " gefunden!! " << endl;               
            }            
          }
        } // end of for
        
        if(blnExistPx1 && !blnExistPx2)
        {
          vPQIntersectionWD.push_back(pIn); 
          vPQIntersectionWD.push_back(pPx1); 
        }
        else if(!blnExistPx1 && blnExistPx2)
        {
          vPQIntersectionWD.push_back(pIn); 
          vPQIntersectionWD.push_back(pPx2); 
        }
        else if(blnExistPx1 && blnExistPx2)
        {
          vPQIntersectionWD.push_back(pPx1); 
          vPQIntersectionWD.push_back(pPx2); 
        }        
        else // (!blnExistPx1 && !blnExistPx2)
        {
          // nothing 
        }
      }      
      else
      {
        cout  <<  "ERROR: nrOfVertexes = " << nrOfVertexes <<  endl; 
      }
    }
  }  
  else // (vP.size()>=3 && vQ.size()>=3)
  {
     // Compute x-ordered sequence of slabs of P's upper and lower boundary:
    int i;
    vector<Point2D> vUppBoundP, vLowBoundP, vUppBoundQ, vLowBoundQ;  
    
    // 1. Compute an x-ordered sequence of sclabs for 
    // the vertices of P's and Q's upper and lower boundary:
    ComputeXOrderedSequenceOfSlabs(vP, vUppBoundP, vLowBoundP);
    ComputeXOrderedSequenceOfSlabs(vQ, vUppBoundQ, vLowBoundQ);
  
  
    // 2. Merge the four sequences of slabs into one refined sequence: 
    // (sweep-line strategy)
    double next_x;
    int iUppBoundP, iLowBoundP, iUppBoundQ, iLowBoundQ;
    iUppBoundP = 0;
    iLowBoundP = 0;
    iUppBoundQ = 0;
    iLowBoundQ = 0;
    vector<VerticalTrapez> vTrapezP;
    vector<VerticalTrapez> vTrapezQ;
    bool blnStartOfP, blnEndOfP, blnElementOfP;
    bool blnStartOfQ, blnEndOfQ, blnElementOfQ;  
    bool blnEndOfPHandled = false;
    bool blnEndOfQHandled = false;
    bool blnStartOfMerge = true;
    VerticalTrapez leftSlabP, rightSlabP;
    VerticalTrapez leftSlabQ, rightSlabQ;
    leftSlabP.IsEmpty = true;
    rightSlabP.IsEmpty = true;
    leftSlabQ.IsEmpty = true;
    rightSlabQ.IsEmpty = true;  
    while(!(blnEndOfPHandled && blnEndOfQHandled))
    {
      // Compute minimum of current x-coordinates: 
      // (which is actually the new vertical sweep-line):
      if(!blnEndOfPHandled)
      {
        next_x = vUppBoundP[iUppBoundP].x;
      }
      else // blnEndOfQHandled==false
      {
        next_x = vUppBoundQ[iUppBoundQ].x;
      }
      
      if(!blnEndOfPHandled)
      {
        if(vUppBoundP[iUppBoundP].x < next_x) 
        {
          next_x = vUppBoundP[iUppBoundP].x;
        }    
        if(vLowBoundP[iLowBoundP].x < next_x) 
        {
          next_x = vLowBoundP[iLowBoundP].x;
        }
      }
      if(!blnEndOfQHandled)
      {     
        if(vUppBoundQ[iUppBoundQ].x < next_x) 
        {
          next_x = vUppBoundQ[iUppBoundQ].x;
        }
        if(vLowBoundQ[iLowBoundQ].x < next_x) 
        {
          next_x = vLowBoundQ[iLowBoundQ].x;
        }
      }   
      //cout  <<  "DEBUG: sweep-line stops at x = " << next_x <<  endl; 
      // the right slab of the last (old) sweep-line is the 
      // left slab in the current (new) sweep-line:
      leftSlabP = rightSlabP;
      leftSlabQ = rightSlabQ;
  
      // There are only the following possibliy cases for next_x in 
      // the structure of P (same idea for Q!): 
      // (case 1) there exist pi of P with next_x = x-coord of point pi 
      // sub-cases of (case 1):
      // (case 1.1) pi = startpoint of P
      // (case 1.2) pi = endpoint of P
      // (case 1.3) pi = element of (P without {startpoint, endpoint})
      // (case 2) for any point pi of P: next_x != x-coord of point pi 
      // sub-cases of (case 2):
      // (case 2.1) next_x < x-coord of startpoint of P
      // (case 2.2) next_x > x-coord of endpoint of P
      // (case 2.3) x-coord of start point of P < next_x < x-coord 
      // of endpoint of P
      
      // some abbreviations about next_x in order to make the 
      // source code more readable: 
      blnElementOfP = (next_x==vUppBoundP[iUppBoundP].x) || 
            (next_x==vLowBoundP[iLowBoundP].x);
      blnStartOfP = (next_x==vUppBoundP[0].x); 
      // x-coord from upp and low at start of P equal!
      blnEndOfP = (next_x==vUppBoundP[vUppBoundP.size()-1].x); 
      // x-coord from upp and low at end of P equal!
      blnElementOfQ = (next_x==vUppBoundQ[iUppBoundQ].x) || 
          (next_x==vLowBoundQ[iLowBoundQ].x);
      blnStartOfQ = (next_x==vUppBoundQ[0].x); 
      // x-coord from upp and low at start of Q equal!
      blnEndOfQ = (next_x==vUppBoundQ[vUppBoundQ.size()-1].x); 
      // x-coord from upp and low at end of Q equal!
      
    
      
      if(blnElementOfP) 
      {
        // (case 1) there exist pi of P with next_x = x-coord of point pi 
        if(blnStartOfP) // (case 1.1) pi = startpoint of P
        {
          //cout << "DEBUG: P case 1.1" << endl; 
          //cout << "DEBUG: case 1.1" << endl; 
          leftSlabP.IsEmpty = false;
          leftSlabP.pUppLeft = vUppBoundP[0];
          leftSlabP.pLowLeft = vLowBoundP[0];
          leftSlabP.pUppRight = vUppBoundP[0];
          leftSlabP.pLowRight = vLowBoundP[0];                                  
          rightSlabP.IsEmpty = false;
          rightSlabP.pUppLeft = vUppBoundP[0];
          rightSlabP.pLowLeft = vLowBoundP[0];          
        }
        else if(blnEndOfP) // (case 1.2) endpoint of P
        {
          //cout << "DEBUG: P case 1.2" << endl; 
          leftSlabP.IsEmpty = false;
          leftSlabP.pUppRight = vUppBoundP[vUppBoundP.size()-1];
          leftSlabP.pLowRight = vLowBoundP[vLowBoundP.size()-1];
          rightSlabP.pUppLeft = leftSlabP.pUppRight;
          rightSlabP.pLowLeft = leftSlabP.pLowRight;
          rightSlabP.IsEmpty = false;
        }
        else // (case 1.3) pi = element of (P without {startpoint, endpoint})
        {
          //cout << "DEBUG: P case 1.3" << endl; 
          // leftSlabP.IsEmpty and rightSlabP.IsEmpty don't change (stay false)
          if(next_x==vUppBoundP[iUppBoundP].x)
          {
            leftSlabP.pUppRight = vUppBoundP[iUppBoundP];
            rightSlabP.pUppLeft = leftSlabP.pUppRight;          
  
          }
          else  // then we have to compute the cut at x=next_x : 
          {
            // there exists the points vUppBoundP[iUppBoundP-1] and 
            // vUppBoundP[iUppBoundP] (!)        
            Point2D pUpp(GetPointFromSegment(next_x, 
                  vUppBoundP[iUppBoundP-1], 
                  vUppBoundP[iUppBoundP]));
            leftSlabP.pUppRight = pUpp;
            rightSlabP.pUppLeft = pUpp;          
          }        
          if(next_x==vLowBoundP[iLowBoundP].x)
          {
            leftSlabP.pLowRight = vLowBoundP[iLowBoundP];
            rightSlabP.pLowLeft = leftSlabP.pLowRight;          
  
          }
          else  // then we have to compute the cut at x=next_x : 
          {
            // there exists the points vLowBoundP[iLowBoundP-1] and 
            // vLowBoundP[iLowBoundP] (!)        
            Point2D pLow(GetPointFromSegment(next_x, vLowBoundP[iLowBoundP-1], 
                    vLowBoundP[iLowBoundP]));
            leftSlabP.pLowRight = pLow;
            rightSlabP.pLowLeft = pLow;          
          }                
        }
      }
      else // (case 2) for any point pi of P: next_x != x-coord of point pi
      {
        if(next_x < vUppBoundP[0].x) 
        {
          // (case 2.1) next_x < x-coord of startpoint of P 
          // (x-coord from upp and low at start of P equal!)
          //cout << "DEBUG: P case 2.1" << endl; 
          // leftSlabP.IsEmpty and rightSlabP.IsEmpty don't change (stay true)
          // do nothing!
        }
        else if(next_x > vUppBoundP[vUppBoundP.size()-1].x) 
        {
          // (case 2.2) next_x > x-coord of endpoint of P 
          // (x-coord from upp and low at end of P equal!)
          //cout << "DEBUG: P case 2.2" << endl; 
          // don't change leftSlabP.IsEmpty (stay false OR true)
          if(!leftSlabP.IsEmpty)
          {
            leftSlabP.pUppRight = leftSlabP.pUppLeft;
            leftSlabP.pLowRight = leftSlabP.pLowLeft;
          }          
          rightSlabP.IsEmpty = true;
        }
        else 
        {
          // (case 2.3) x-coord of start point of P < next_x < x-coord 
          // of endpoint of P
          //cout << "DEBUG: P case 2.3" << endl; 
          // then we have to compute the cut at x=next_x :
          // there exists the points vUppBoundP[iUppBoundP-1] and 
          // vUppBoundP[iUppBoundP] (!)        
          Point2D pUpp(GetPointFromSegment(next_x, 
              vUppBoundP[iUppBoundP-1], 
              vUppBoundP[iUppBoundP]));
          leftSlabP.pUppRight = pUpp;
          rightSlabP.pUppLeft = pUpp;
          // there exists the points vLowBoundP[iLowBoundP-1] and 
          // vLowBoundP[iLowBoundP] (!)
          Point2D pLow(GetPointFromSegment(next_x, 
                vLowBoundP[iLowBoundP-1], 
                vLowBoundP[iLowBoundP]));
          leftSlabP.pLowRight = pLow;                
          rightSlabP.pLowLeft = pLow;
          // leftSlabP.IsEmpty and rightSlabP.IsEmpty don't change (stay false)
        }      
      } 
  
      if(blnElementOfQ) 
      {
        // (case 1) there exist pi of Q with next_x = x-coord of point pi 
        if(blnStartOfQ) 
        {
          // (case 1.1) pi = startpoint of Q
          //cout << "DEBUG: Q case 1.1" << endl; 
          leftSlabQ.IsEmpty = false;
          leftSlabQ.pUppLeft = vUppBoundQ[0];
          leftSlabQ.pLowLeft = vLowBoundQ[0];
          leftSlabQ.pUppRight = vUppBoundQ[0];
          leftSlabQ.pLowRight = vLowBoundQ[0];                                  
          rightSlabQ.IsEmpty = false;
          rightSlabQ.pUppLeft = vUppBoundQ[0];
          rightSlabQ.pLowLeft = vLowBoundQ[0];                  }
        else if(blnEndOfQ) // (case 1.2) endpoint of Q
        {
          //cout << "DEBUG: Q case 1.2" << endl; 
          leftSlabQ.IsEmpty = false;
          leftSlabQ.pUppRight = vUppBoundQ[vUppBoundQ.size()-1];
          leftSlabQ.pLowRight = vLowBoundQ[vLowBoundQ.size()-1];        
          rightSlabQ.pUppLeft = leftSlabQ.pUppRight;
          rightSlabQ.pLowLeft = leftSlabQ.pLowRight;
          rightSlabQ.IsEmpty = false;
        }
        else // (case 1.3) pi = element of (Q without {startpoint, endpoint})
        {
          //cout << "DEBUG: Q case 1.3" << endl; 
          // leftSlabQ.IsEmpty and rightSlabQ.IsEmpty don't change (stay false)
          if(next_x==vUppBoundQ[iUppBoundQ].x)
          {
            leftSlabQ.pUppRight = vUppBoundQ[iUppBoundQ];
            rightSlabQ.pUppLeft = leftSlabQ.pUppRight;          
  
          }
          else  // then we have to compute the cut at x=next_x : 
          {
            // there exists the points vUppBoundQ[iUppBoundQ-1] and 
            // vUppBoundQ[iUppBoundQ] (!)        
            Point2D pUpp(GetPointFromSegment(next_x, 
                  vUppBoundQ[iUppBoundQ-1], 
                  vUppBoundQ[iUppBoundQ]));
            leftSlabQ.pUppRight = pUpp;
            rightSlabQ.pUppLeft = pUpp;          
          }        
          if(next_x==vLowBoundQ[iLowBoundQ].x)
          {
            leftSlabQ.pLowRight = vLowBoundQ[iLowBoundQ];
            rightSlabQ.pLowLeft = leftSlabQ.pLowRight;          
  
          }
          else  // then we have to compute the cut at x=next_x : 
          {
            // there exists the points vLowBoundQ[iLowBoundQ-1] and 
            // vLowBoundQ[iLowBoundQ] (!)        
            Point2D pLow(GetPointFromSegment(next_x, 
            vLowBoundQ[iLowBoundQ-1], 
            vLowBoundQ[iLowBoundQ]));
            leftSlabQ.pLowRight = pLow;
            rightSlabQ.pLowLeft = pLow;          
          }                
        }
      }
      else // (case 2) for any point pi of Q: next_x != x-coord of point pi
      {
        if(next_x < vUppBoundQ[0].x) 
        {
          // (case 2.1) next_x < x-coord of startpoint of Q 
          // (x-coord from upp and low at start of Q equal!)
          //cout << "DEBUG: Q case 2.1" << endl; 
          // leftSlabQ.IsEmpty and rightSlabQ.IsEmpty don't change (stay true)
          // do nothing!
        }
        else if(next_x > vUppBoundQ[vUppBoundQ.size()-1].x) 
        {
          // (case 2.2) next_x > x-coord of endpoint of Q 
          // (x-coord from upp and low at end of Q equal!)
          //cout << "DEBUG: Q case 2.2" << endl; 
          // don't change leftSlabQ.IsEmpty (stay false OR true)
          if(!leftSlabQ.IsEmpty)
          {
            leftSlabQ.pUppRight = leftSlabQ.pUppLeft;
            leftSlabQ.pLowRight = leftSlabQ.pLowLeft;
          }          
          rightSlabQ.IsEmpty = true;
        }
        else 
        {
          // (case 2.3) x-coord of start point of Q < next_x < x-coord 
          // of endpoint of Q
          //cout << "DEBUG: Q case 2.3" << endl; 
          // then we have to compute the cut at x=next_x :
          // there exists the points vUppBoundQ[iUppBoundQ-1] and 
          // vUppBoundQ[iUppBoundQ] (!)        
          Point2D pUpp(GetPointFromSegment(next_x, 
            vUppBoundQ[iUppBoundQ-1], 
            vUppBoundQ[iUppBoundQ]));
          leftSlabQ.pUppRight = pUpp;
          rightSlabQ.pUppLeft = pUpp;
          // there exists the points vLowBoundQ[iLowBoundQ-1] and 
          // vLowBoundQ[iLowBoundQ] (!)
          Point2D pLow(GetPointFromSegment(next_x, 
            vLowBoundQ[iLowBoundQ-1], 
            vLowBoundQ[iLowBoundQ]));
          leftSlabQ.pLowRight = pLow;                
          rightSlabQ.pLowLeft = pLow;
          // leftSlabQ.IsEmpty and rightSlabQ.IsEmpty don't change (stay false)
        }      
      } 
  
  
      if(!blnStartOfMerge)
      {
        vTrapezP.push_back(leftSlabP);
        vTrapezQ.push_back(leftSlabQ);
      }
      
      
              
      if(blnEndOfP)
      {
        blnEndOfPHandled = true;       
      }
      if(blnEndOfQ)
      {
        blnEndOfQHandled = true;       
      }
              
      if(vUppBoundP[iUppBoundP].x == next_x && iUppBoundP < vUppBoundP.size()-1)
      {
        iUppBoundP++;
        //cout << "DEBUG: iUppBoundP++" << endl;
      }
      if(vLowBoundP[iLowBoundP].x == next_x && iLowBoundP < vLowBoundP.size()-1)
      {
        iLowBoundP++;
        //cout << "DEBUG: iLowBoundP++" << endl;
      }
      if(vUppBoundQ[iUppBoundQ].x == next_x && iUppBoundQ < vUppBoundQ.size()-1)
      {
        iUppBoundQ++;
        //cout << "DEBUG: vUppBoundQ++" << endl;
      }
      if(vLowBoundQ[iLowBoundQ].x == next_x && iLowBoundQ < vLowBoundQ.size()-1)
      {
        iLowBoundQ++;
        //cout << "DEBUG: iLowBoundQ++" << endl;
      }            
         
      blnStartOfMerge = false;
    }
    
    
    //cout << "DEBUG: RESULAT VON MERGE-SCHRITT: vTrapezP:" << endl; 
    //for(i=0; i < vTrapezP.size(); i++)
    //{
    //  printSlab(vTrapezP[i]);
    //}
    //cout << "DEBUG: RESULAT VON MERGE-SCHRITT: vTrapezQ:" << endl; 
    //for(i=0; i < vTrapezQ.size(); i++)
    //{
    //  printSlab(vTrapezQ[i]);
    //}  
    
    
    // Attetion: it's possible that a Trapez T is degenerated (case *): 
    // T.IsEmpty==false && T.pUppLeft==T.pUppRight && T.pLowLeft==T.pLowRight 
    //
    // 3.+4. For each slab, compute the intersection of its trapeziums 
    // (if there is only one, the intersection is empty)
    // and merge them into a result polygon:  
    // Voraussetzung: vTrapezP.size() == vTrapezQ.size()    
    vector<Point2D> vPQIntersectionUppWD, vPQIntersectionLowWD; 
    // remark: "WD" = "with possibly dublicates"  
    bool blnDegeneratedCase;
    bool blnDegeneratedTrapezP;
    bool blnDegeneratedTrapezQ;
    for(i=0; i < vTrapezP.size(); i++)
    {
      //cout << "DEBUG: Trapez Nr: " << i << endl; 
      if(!vTrapezP[i].IsEmpty && !vTrapezQ[i].IsEmpty)
      {
        //cout << "DEBUG: Trapez are not both emtpy! " << i << endl; 
        blnDegeneratedTrapezP = (vTrapezP[i].pUppLeft==vTrapezP[i].pUppRight);
        blnDegeneratedTrapezQ = (vTrapezQ[i].pUppLeft==vTrapezQ[i].pUppRight);
        blnDegeneratedCase = (blnDegeneratedTrapezP || blnDegeneratedTrapezQ);
        // Left Border Check:
        //cout << "DEBUG: Start of Left-Border-Check..." << endl; 
        if((vTrapezQ[i].pLowLeft.y <= vTrapezP[i].pUppLeft.y) && 
           (vTrapezP[i].pUppLeft.y <= vTrapezQ[i].pUppLeft.y) &&
           (!blnDegeneratedCase || AlmostEqual(vTrapezP[i].pUppLeft.x, 
                vTrapezQ[i].pUppLeft.x)))
        {
          //cout << "DEBUG: Left-Border-Check 1 successfull!" << endl; 
          vPQIntersectionUppWD.push_back(vTrapezP[i].pUppLeft); 
          //cout << "DEBUG: Point vTrapezP[" << i 
          //<< "].pUppLeft added (Left Border Check): " << endl;
          //vTrapezP[i].pUppLeft.printOut();
        }        
        if((vTrapezP[i].pLowLeft.y <= vTrapezQ[i].pUppLeft.y) && 
           (vTrapezQ[i].pUppLeft.y <= vTrapezP[i].pUppLeft.y) &&
           (!blnDegeneratedCase || AlmostEqual(vTrapezP[i].pUppLeft.x, 
                vTrapezQ[i].pUppLeft.x)))
        {
          //cout << "DEBUG: Left-Border-Check 2 successfull!" << endl; 
          vPQIntersectionUppWD.push_back(vTrapezQ[i].pUppLeft); 
          //cout << "DEBUG: Point vTrapezQ[" << i 
          //<< "].pUppLeft added (Left Border Check): " << endl;
          //vTrapezQ[i].pUppLeft.printOut();
        }                
    
        if((vTrapezQ[i].pLowLeft.y <= vTrapezP[i].pLowLeft.y) && 
           (vTrapezP[i].pLowLeft.y <= vTrapezQ[i].pUppLeft.y) &&
           (!blnDegeneratedCase || AlmostEqual(vTrapezP[i].pUppLeft.x, 
                vTrapezQ[i].pUppLeft.x)))
        {
          //cout << "DEBUG: Left-Border-Check 3 successfull!" << endl; 
          vPQIntersectionLowWD.push_back(vTrapezP[i].pLowLeft); 
          //cout << "DEBUG: Point vTrapezP[" << i 
          // << "].pLowLeft added (Left Border Check): " << endl;
          //vTrapezP[i].pLowLeft.printOut();
        }        
        if((vTrapezP[i].pLowLeft.y <= vTrapezQ[i].pLowLeft.y) && 
           (vTrapezQ[i].pLowLeft.y <= vTrapezP[i].pUppLeft.y) &&
           (!blnDegeneratedCase || AlmostEqual(vTrapezP[i].pUppLeft.x, 
              vTrapezQ[i].pUppLeft.x)))
        {
          //cout << "DEBUG: Left-Border-Check 4 successfull!" << endl; 
          vPQIntersectionLowWD.push_back(vTrapezQ[i].pLowLeft); 
          //cout << "DEBUG: Point vTrapezQ[" << i 
          // << "].pLowLeft added (Left Border Check): " << endl;
          //vTrapezQ[i].pLowLeft.printOut();          
        }              
        //cout << "DEBUG: End of Left-Border-Check." << endl; 
         
        if(!blnDegeneratedCase)
        {
          // Let's compute possible inner  intersecting points 
          // S1, S2, S3, S4 with 
          //(vTrapezP[i].pUppLeft.x < Si.x < vTrapezP[i].pUppRight.x):
          bool blnS1Exist = false; 
          bool blnS2Exist = false; 
          bool blnS3Exist = false; 
          bool blnS4Exist = false;
          Point2D pS1, pS2, pS3, pS4;
          
                
          GetIntersectionPoint(vTrapezP[i].pUppLeft, 
                               vTrapezP[i].pUppRight, 
                               vTrapezQ[i].pUppLeft, 
                               vTrapezQ[i].pUppRight, 
                               blnS1Exist, pS1);
          GetIntersectionPoint(vTrapezP[i].pUppLeft, 
                               vTrapezP[i].pUppRight, 
                               vTrapezQ[i].pLowLeft, 
                               vTrapezQ[i].pLowRight, 
                               blnS2Exist, 
                               pS2);
          GetIntersectionPoint(vTrapezP[i].pLowLeft, 
                               vTrapezP[i].pLowRight, 
                               vTrapezQ[i].pUppLeft, 
                               vTrapezQ[i].pUppRight, 
                               blnS3Exist, 
                               pS3);
          GetIntersectionPoint(vTrapezP[i].pLowLeft, 
                               vTrapezP[i].pLowRight, 
                               vTrapezQ[i].pLowLeft, 
                               vTrapezQ[i].pLowRight, 
                               blnS4Exist, 
                               pS4);     
          
          // S1, S2 and S3 are on upper bound (if they exist): 
          //add them and sort them in cw-order:
          int nrOfPointsAdded;  
          nrOfPointsAdded = 0;
          if(blnS1Exist && vTrapezP[i].pUppLeft.x < pS1.x && 
             pS1.x < vTrapezP[i].pUppRight.x)
          {
            vPQIntersectionUppWD.push_back(pS1); 
            //cout << "DEBUG: Point S1 added on upper bound: " << endl;
            //pS1.printOut();
            nrOfPointsAdded++;
          }        
          if(blnS2Exist && vTrapezP[i].pUppLeft.x < pS2.x && 
             pS2.x < vTrapezP[i].pUppRight.x)
          {
            vPQIntersectionUppWD.push_back(pS2); 
            //cout << "DEBUG: Point S2 added on upper bound: " << endl;
            //pS2.printOut();
            nrOfPointsAdded++;
          }        
          if(blnS3Exist && vTrapezP[i].pUppLeft.x < pS3.x && 
             pS3.x < vTrapezP[i].pUppRight.x)
          {
            vPQIntersectionUppWD.push_back(pS3); 
            //cout << "DEBUG: Point S3 added on upper bound: " << endl;
            //pS3.printOut();            
            nrOfPointsAdded++;
          }
          sort(vPQIntersectionUppWD.end()-nrOfPointsAdded,
               vPQIntersectionUppWD.end());      
          
          // S2, S3 and S4 are on lower bound (if they exist): 
          // add them and sort them in cw-order:
          nrOfPointsAdded = 0;
          if(blnS2Exist && vTrapezP[i].pUppLeft.x < pS2.x && 
             pS2.x < vTrapezP[i].pUppRight.x)
          {
            vPQIntersectionLowWD.push_back(pS2); 
            //cout << "DEBUG: Point S2 added on lower bound: " << endl;
            //pS2.printOut();            
            nrOfPointsAdded++;
          }        
          if(blnS3Exist && vTrapezP[i].pUppLeft.x < pS3.x && 
             pS3.x < vTrapezP[i].pUppRight.x)
          {
            vPQIntersectionLowWD.push_back(pS3); 
            //cout << "DEBUG: Point S3 added on lower bound: " << endl;
            //pS3.printOut();            
            nrOfPointsAdded++;
          }        
          if(blnS4Exist && vTrapezP[i].pUppLeft.x < pS4.x && 
             pS4.x < vTrapezP[i].pUppRight.x)
          {
            vPQIntersectionLowWD.push_back(pS4); 
            //cout << "DEBUG: Point S4 added on lower bound: " << endl;
            //pS4.printOut();                        
            nrOfPointsAdded++;
          }      
          sort(vPQIntersectionLowWD.end()-nrOfPointsAdded,
               vPQIntersectionLowWD.end());
        }
        // Right Border Check:
        //cout << "DEBUG: Start of Right-Border-Check...." << endl; 
        if((vTrapezQ[i].pLowRight.y <= vTrapezP[i].pUppRight.y) && 
           (vTrapezP[i].pUppRight.y <= vTrapezQ[i].pUppRight.y) && 
           (!blnDegeneratedCase || AlmostEqual(vTrapezP[i].pUppRight.x, 
            vTrapezQ[i].pUppRight.x)))
        {
          vPQIntersectionUppWD.push_back(vTrapezP[i].pUppRight); 
          //cout << "DEBUG: Point vTrapezP[" << i << "].pUppRight added 
          // (Right Border Check): " << endl;
          //vTrapezP[i].pUppRight.printOut();
          //cout << "DEBUG: Right-Border-Check 1 successfull!" << endl; 
        }        
        if((vTrapezP[i].pLowRight.y <= vTrapezQ[i].pUppRight.y) && 
           (vTrapezQ[i].pUppRight.y <= vTrapezP[i].pUppRight.y) &&
           (!blnDegeneratedCase || AlmostEqual(vTrapezP[i].pUppRight.x, 
            vTrapezQ[i].pUppRight.x)))
        {
          vPQIntersectionUppWD.push_back(vTrapezQ[i].pUppRight); 
          //cout << "DEBUG: Point vTrapezQ[" << i << "].pUppRight added 
          // (Right Border Check): " << endl;
          //vTrapezQ[i].pUppRight.printOut();
          //cout << "DEBUG: Right-Border-Check 2 successfull!" << endl; 
        }                
    
        if((vTrapezQ[i].pLowRight.y <= vTrapezP[i].pLowRight.y) && 
           (vTrapezP[i].pLowRight.y <= vTrapezQ[i].pUppRight.y) &&
           (!blnDegeneratedCase || AlmostEqual(vTrapezP[i].pUppRight.x, 
            vTrapezQ[i].pUppRight.x)))
        {
          vPQIntersectionLowWD.push_back(vTrapezP[i].pLowRight); 
          //cout << "DEBUG: Point vTrapezP[" << i << "].pLowRight added 
          // (Right Border Check): " << endl;
          //vTrapezP[i].pLowRight.printOut();
          //cout << "DEBUG: Right-Border-Check 3 successfull!" << endl; 
        }        
        if((vTrapezP[i].pLowRight.y <= vTrapezQ[i].pLowRight.y) && 
           (vTrapezQ[i].pLowRight.y <= vTrapezP[i].pUppRight.y) &&
           (!blnDegeneratedCase || AlmostEqual(vTrapezP[i].pUppRight.x, 
           vTrapezQ[i].pUppRight.x)))
        {
          vPQIntersectionLowWD.push_back(vTrapezQ[i].pLowRight); 
          //cout << "DEBUG: Point vTrapezQ[" << i << "].pLowRight added 
          // (Right Border Check): " << endl;
          //vTrapezQ[i].pLowRight.printOut();
          //cout << "DEBUG: Right-Border-Check 4 successfull!" << endl; 
        }                    
        //cout << "DEBUG: End of Right-Border-Check." << endl; 
      }
    }
    vPQIntersectionWD.clear();
    for(i=0; i < vPQIntersectionUppWD.size(); i++)
    {
       //cout << "DEBUG: vPQIntersectionUppWD[" << i << "] == (" 
       // << vPQIntersectionUppWD[i].x << ", " 
       // << vPQIntersectionUppWD[i].y << ")" << endl;  
       vPQIntersectionWD.push_back(vPQIntersectionUppWD[i]);
    }
    for(i=vPQIntersectionLowWD.size()-1; i >= 0; i--)
    {
       //cout << "DEBUG: vPQIntersectionLowWD[" << i << "] == (" 
       //<< vPQIntersectionLowWD[i].x << ", " 
       //<< vPQIntersectionLowWD[i].y << ")" << endl;  
       vPQIntersectionWD.push_back(vPQIntersectionLowWD[i]);
    }    
  }  

    // Eliminate dublicates in O(n) time 
    // (can only appear sequecially or = first):
  if(vPQIntersectionWD.size()>0)
  {
    vPQIntersection.push_back(vPQIntersectionWD[0]);
    for(i=1; i < vPQIntersectionWD.size(); i++)
    {
      if(vPQIntersectionWD[i]!=vPQIntersectionWD[i-1] && 
        vPQIntersectionWD[i]!=vPQIntersectionWD[0])
      {
          vPQIntersection.push_back(vPQIntersectionWD[i]);
      }
    }  
  }        
  //cout << "DEBUG: Ende der ConvexPolygonIntersection-Funktion..." << endl; 
  return;
}

/*
6.9 ~HalfPlaneIntersection~-function  

*/
void HalfPlaneIntersection(
        const vector<LinearConstraint>& vLinConstraints, 
        vector<Point2D>& vConvexPolygon)
{
  // Eingabe: Vektor mit lienearen Constraints, welche die Eingabe beschreiben
  //          gegeben als nicht-leerer Vektor von linearen Constraints 
  //          welche jeweils Half-Planes oder Mengen der Art 
  //          {1<=0}={}, {-1<=0}=R^2 definieren 
  //          (d.h. mit OP_LEQ-Vergleichsoperator)
  //          (degenerierte Faelle moeglich: unbeschraenkte Objekte, Segment, 
  //           Punkt; nicht moeglich: leere Menge) 
  // Ausgabe: konvexes Polygon (evt. auch nur ein Punkt, 
  //          zwei Punkte oder leer!!)   
  // Durchschnitt berechnen 
  vConvexPolygon.clear();
  if(vLinConstraints.size()==0)
  {
    // nothing to do
  }
  else if(vLinConstraints.size()==1)
  {
    bool blnNoXNoY; // special case 0*x + 0*y + <value> <= 0;
    bool blnVerticalBoundary;
    bool blnHorizontalBoundary ;
    // boolean that indicates whereas the points at left of the 
    // boundary are inside the VERTICAL half-plane :
    bool blnLeftSideIn;  
    // boolean that indicates whereas the points at upper side of the 
    // boundary are inside the NON-VERTICAL half-plane:
    bool blnUpperSideIn; 
    
    const double wxLeft = WORLD.MinD(0);
    const double wxRight = WORLD.MaxD(0);  
    const double wyBottom = WORLD.MinD(1);
    const double wyTop = WORLD.MaxD(1);  
    double xCut, yCut;
    int i;
    
    LinearConstraint linC(vLinConstraints[0]);
    
    vector<Point2D> vConvexPolygonWD; // WD = With Duclicates
    
    blnNoXNoY = (linC.get_a1()==0 && linC.get_a2()==0);
    blnVerticalBoundary = (linC.get_a1()!=0 && linC.get_a2()==0);
    blnHorizontalBoundary = (linC.get_a1()==0 && linC.get_a2()!=0);
    blnLeftSideIn = (linC.get_a1() > 0);
    blnUpperSideIn = (linC.get_a2() < 0);
    if(blnNoXNoY)
    {
      // special case 0*x + 0*y + <value> <= 0;
      if(linC.get_b() <= 0)
      {
        // => R^2
        Point2D p1, p2, p3, p4;  
        p1 = Point2D(wxLeft, wyTop);
        p2 = Point2D(wxRight, wyTop);
        p3 = Point2D(wxRight, wyBottom);
        p4 = Point2D(wxLeft, wyBottom);
        vConvexPolygonWD.push_back(p1);
        vConvexPolygonWD.push_back(p2);        
        vConvexPolygonWD.push_back(p3);        
        vConvexPolygonWD.push_back(p4);        
      }
      else // (linC.get_b() > 0)  
      {
        // => {}
        // do nothing!
      }
    }
    else
    {
      // TOP-bounding of the rectangle WORLD:      
      if(blnHorizontalBoundary)
      {
        yCut = (-linC.get_b()/linC.get_a2());
        if(AlmostEqual(wyTop, yCut) || (yCut <= wyTop && blnUpperSideIn) || 
          (yCut >= wyTop && !blnUpperSideIn))
        {
          Point2D p1, p2;  
          p1 = Point2D(wxLeft, wyTop);
          p2 = Point2D(wxRight, wyTop);
          vConvexPolygonWD.push_back(p1);
          vConvexPolygonWD.push_back(p2);
        }
      }
      else
      {
        if(blnVerticalBoundary)
        {
          xCut = (-linC.get_b()/linC.get_a1());        
        } 
        else
        {
          xCut = (-linC.get_b()-linC.get_a2()*wyTop)/linC.get_a1();
        }
        if(wxLeft <= xCut && xCut <= wxRight)
        {
          // the boundary of the half-plane is clipped 
          // by the Top-Boundary of the WORLD:
          Point2D p1, p2;  
          if(blnLeftSideIn)
          {
            p1 = Point2D(wxLeft, wyTop);
            p2 = Point2D(xCut, wyTop);
          }
          else
          { // right side in
            p1 = Point2D(xCut, wyTop);
            p2 = Point2D(wxRight, wyTop);          
          }        
          vConvexPolygonWD.push_back(p1);
          vConvexPolygonWD.push_back(p2);
        }
        else if((xCut < wxLeft && !blnLeftSideIn) || 
                (xCut > wxRight && blnLeftSideIn))
        {
          Point2D p1(wxLeft, wyTop);
          Point2D p2(wxRight, wyTop);               
          vConvexPolygonWD.push_back(p1);
          vConvexPolygonWD.push_back(p2);          
        }
        
      }
      
      // RIGHT-bounding of the rectangle WORLD:    
      if(blnVerticalBoundary)
      {
        xCut = (-linC.get_b()/linC.get_a1());        
        if(AlmostEqual(wxRight, xCut) || (xCut <= wxRight && !blnLeftSideIn) || 
          (xCut >= wxRight && blnLeftSideIn))
        {
          Point2D p1, p2;  
          p1 = Point2D(wxRight, wyTop);
          p2 = Point2D(wxRight, wyBottom);
          vConvexPolygonWD.push_back(p1);
          vConvexPolygonWD.push_back(p2);
        }
      }
      else
      {
        if(blnHorizontalBoundary)  
        {
          yCut = (-linC.get_b()/linC.get_a2());        
        }
        else
        {
          yCut = (-linC.get_b()-linC.get_a1()*wxRight)/linC.get_a2();
        }
        if(wyBottom <= yCut && yCut <= wyTop)
        {
          // the boundary of the half-plane is clipped by 
          // the Right-Boundary of the WORLD:
          Point2D p1, p2;  
          if(blnUpperSideIn)
          {
            p1 = Point2D(wxRight, wyTop);
            p2 = Point2D(wxRight, yCut);
          }
          else
          { // lower side in
            p1 = Point2D(wxRight, yCut);
            p2 = Point2D(wxRight, wyBottom);
          }        
          vConvexPolygonWD.push_back(p1);
          vConvexPolygonWD.push_back(p2);
        }
        else if((yCut < wyBottom && blnUpperSideIn) || 
                (yCut > wyTop && !blnUpperSideIn))
        {
          Point2D p1(wxRight, wyTop);
          Point2D p2(wxRight, wyBottom);
          vConvexPolygonWD.push_back(p1);
          vConvexPolygonWD.push_back(p2);                   
        }
      }
              
      // BOTTOM-bounding of the rectangle WORLD:        
      if(blnHorizontalBoundary)
      {
        yCut = (-linC.get_b()/linC.get_a2());
        if(AlmostEqual(wyBottom, yCut) || (yCut >= wyBottom && 
          !blnUpperSideIn) || 
          (yCut <= wyBottom && blnUpperSideIn))
        {
          Point2D p1, p2;  
          p1 = Point2D(wxRight, wyBottom);
          p2 = Point2D(wxLeft, wyBottom);
          vConvexPolygonWD.push_back(p1);
          vConvexPolygonWD.push_back(p2);
        }
      }
      else
      {
        if(blnVerticalBoundary)
        {
          xCut = (-linC.get_b()/linC.get_a1());        
        } 
        else
        {
          xCut = (-linC.get_b()-linC.get_a2()*wyBottom)/linC.get_a1();
        }              
        if(wxLeft <= xCut && xCut <= wxRight)
        {
          // the boundary of the half-plane is clipped by 
          // the Bottom-Boundary of the WORLD:
          Point2D p1, p2;
          if(blnLeftSideIn)
          {
            p1 = Point2D(xCut, wyBottom);
            p2 = Point2D(wxLeft, wyBottom);                
          }
          else
          { // right side in
            p1 = Point2D(wxRight, wyBottom);
            p2 = Point2D(xCut, wyBottom);                          
          }        
          vConvexPolygonWD.push_back(p1);
          vConvexPolygonWD.push_back(p2);
        }
        else if((xCut < wxLeft && !blnLeftSideIn) || 
                (xCut > wxRight && blnLeftSideIn))
        {
          Point2D p1(wxRight, wyBottom);
          Point2D p2(wxLeft, wyBottom);                
          vConvexPolygonWD.push_back(p1);
          vConvexPolygonWD.push_back(p2);          
        }
      }
      
      // LEFT-bounding of the rectangle WORLD:  
      if(blnVerticalBoundary)
      {
        xCut = (-linC.get_b()/linC.get_a1());        
        if(AlmostEqual(wxLeft, xCut) || (xCut <= wxLeft && !blnLeftSideIn) || 
              (xCut >= wxLeft && blnLeftSideIn))
        {
          Point2D p1, p2;  
          p1 = Point2D(wxLeft, wyBottom);
          p2 = Point2D(wxLeft, wyTop);
          vConvexPolygonWD.push_back(p1);
          vConvexPolygonWD.push_back(p2);
        }
      }
      else
      {        
        if(blnHorizontalBoundary)  
        {
          yCut = (-linC.get_b()/linC.get_a2());        
        }
        else
        {
          yCut = (-linC.get_b()-linC.get_a1()*wxLeft)/linC.get_a2();
        }
        if(wyBottom <= yCut && yCut <= wyTop)
        {
          // the boundary of the half-plane is clipped by the 
          // Bottom-Boundary of the WORLD:
          Point2D p1, p2;  
          if(blnUpperSideIn)
          {
            p1 = Point2D(wxLeft, yCut);
            p2 = Point2D(wxLeft, wyTop);
          }
          else
          { // lower side in
            p1 = Point2D(wxLeft, wyBottom);
            p2 = Point2D(wxLeft, yCut);
          }        
          vConvexPolygonWD.push_back(p1);
          vConvexPolygonWD.push_back(p2);
        }
        else if((yCut < wyBottom && blnUpperSideIn) || 
                (yCut > wyTop && !blnUpperSideIn))
        {
          Point2D p1(wxLeft, wyBottom);;
          Point2D p2(wxLeft, wyTop);                
          vConvexPolygonWD.push_back(p1);
          vConvexPolygonWD.push_back(p2);          
        }
      }
      
      // Eliminate dublicates (can only appear sequecially or = first):
      if(vConvexPolygonWD.size()>0)
      {
        vConvexPolygon.push_back(vConvexPolygonWD[0]);
        for(i=1; i < vConvexPolygonWD.size(); i++)
        {
          if(vConvexPolygonWD[i]!=vConvexPolygonWD[i-1] && 
             vConvexPolygonWD[i]!=vConvexPolygonWD[0])
          {
            vConvexPolygon.push_back(vConvexPolygonWD[i]);
          }
        }
      }      
    }
  }
  else // vLinConstraints.size() >= 2
  {
    vector<LinearConstraint> vLinConstraintsPart1;
    vector<LinearConstraint> vLinConstraintsPart2;
    const int n = vLinConstraints.size();
    for(int i=0; i < n ; i++)
    {
      if(i < n/2)
      {
        vLinConstraintsPart1.push_back(vLinConstraints[i]);
      }
      else
      {
        vLinConstraintsPart2.push_back(vLinConstraints[i]);
      }
    }
    vector<Point2D> vP;
    vector<Point2D> vQ;
    HalfPlaneIntersection(vLinConstraintsPart1, vP);
    HalfPlaneIntersection(vLinConstraintsPart2, vQ);    
    
    ConvexPolygonIntersection(vP, vQ, vConvexPolygon);     
    
        
    //cout << "DEBUG: ---- Eingabe von folgenden Punkten: 
      //(P fuer CPI-Intersection in HPI-Funktion)"; 
    //int ki;
    //for(ki=0; ki<vP.size(); ki++)
    //{
    //   cout << endl << "vP[" << ki << "]: ";
    //   vP[ki].printOut();
    //}
    //cout << endl << "DEBUG: ---- Eingabe von folgenden Punkten: 
    //(Q fuer CPI-Intersection in HPI-Funktion)"; 
    //for(ki=0; ki<vQ.size(); ki++)
    //{
    //   cout << endl << "vQ[" << ki << "]: ";
    //   vQ[ki].printOut();
    //}    
    //cout << endl << "DEBUG: ---- Ausgabe von folgenden Punkten: 
    //(nach CPI-Intersection in HPI-Funktion)"; 
    //for(ki=0; ki<vConvexPolygon.size(); ki++)
    //{
    //   cout << endl << "vConvexPolygon[" << ki << "]: ";
    //   vConvexPolygon[ki].printOut();
    //}
    //cout << "DEBUG: -------------" << endl;
    
  }
}

} // namespace

#endif








