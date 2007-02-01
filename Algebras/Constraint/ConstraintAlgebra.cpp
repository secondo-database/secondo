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

[1] Implementation of the Constraint Algebra

January, 2007 Simon Muerner

[TOC]

1 Overview

This implementation file essentially contains the definitions and implementations of the type constructor ~constraint~ with its associated operations.


2 Defines and includes

*/
using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "ConstraintAlgebra.h"
#include "StandardTypes.h"

extern NestedList* nl;
extern QueryProcessor* qp;


namespace Constraint {


/*
3 Implementation of class ~LinearConstraint~

*/
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
}

LinearConstraint* LinearConstraint::Clone()
{
  return new LinearConstraint(*this);
}

LinearConstraint& LinearConstraint::operator=(const LinearConstraint& linC)
{
  this->a1 = linC.Get_a1();
  this->a2 = linC.Get_a2();
  this->b = linC.Get_b();
  strcpy(this->strOp,(linC.Get_Op()).c_str());
  return *this;
}

bool LinearConstraint::IsParallel(const LinearConstraint& linC)
{
  bool isParallel = (AlmostEqual(this->a1,0) &&
                     AlmostEqual(linC.Get_a1(),0)) ||
                    (AlmostEqual(this->a2,0) &&
                     AlmostEqual(linC.Get_a2(),0)) ||
                     AlmostEqual(this->a1/this->a2,
                      linC.Get_a1()/linC.Get_a2());
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
  if((this->Get_Op()==OP_EQ && AlmostEqual(this->a1,0) &&
      AlmostEqual(this->a2,0) && !AlmostEqual(this->b,0)) ||
     (this->Get_Op()==OP_LEQ && AlmostEqual(this->a1,0) &&
      AlmostEqual(this->a2,0) && this->b>0))
  {
    // case (i) or (ii)
    blnFirst = true;
  }
  blnSecond = false;
  if((linC.Get_Op()==OP_EQ && AlmostEqual(linC.Get_a1(),0)
   && AlmostEqual(linC.Get_a2(),0) && !AlmostEqual(linC.Get_b(),0)) ||
     (linC.Get_Op()==OP_LEQ && AlmostEqual(linC.Get_a1(),0) &&
      AlmostEqual(linC.Get_a2(),0) && linC.Get_b()>0))
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
  // (ii) v<=0 for every v in R which is not positive
  // (that means 0 or negative)
  blnFirst = false;
  if((this->Get_Op()==OP_EQ && AlmostEqual(this->a1,0) &&
     AlmostEqual(this->a2,0) && AlmostEqual(this->b,0)) ||
     (this->Get_Op()==OP_LEQ && AlmostEqual(this->a1,0) &&
     AlmostEqual(this->a2,0) && this->b<0))
  {
    // case (i) or (ii)
    blnFirst = true;
  }
  blnSecond = false;
  if((linC.Get_Op()==OP_EQ && AlmostEqual(linC.Get_a1(),0)
   && AlmostEqual(linC.Get_a2(),0) && AlmostEqual(linC.Get_b(),0)) ||
     (linC.Get_Op()==OP_LEQ && AlmostEqual(linC.Get_a1(),0) &&
      AlmostEqual(linC.Get_a2(),0) && linC.Get_b()<=0))
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
  if((!AlmostEqual(this->a1,0) && !AlmostEqual(linC.Get_a1(),0)) ||
        (!AlmostEqual(this->a2,0) && !AlmostEqual(linC.Get_a2(),0)))
  {
    double lamda;
    if(!AlmostEqual(this->a1,0) && !AlmostEqual(linC.Get_a1(),0))
    {
      lamda = this->a1/linC.Get_a1();
    }
    else
    {
      lamda = this->a2/linC.Get_a2();
    }
    if(AlmostEqual(this->a1,(lamda*linC.Get_a1())) &&
       AlmostEqual(this->a2, (lamda*linC.Get_a2())) &&
       AlmostEqual(this->b, (lamda*linC.Get_b())))
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
    // then one constraint represents a set with a horicontal bounding
    // where the ohter has a vertical bounding and therefore
    // the two constraints can't be equal!
    return false;
  }
}

double LinearConstraint::Get_a1() const
{
  return this->a1;
}

double LinearConstraint::Get_a2() const
{
  return this->a2;
}

double LinearConstraint::Get_b() const
{
  return this->b;
}

string LinearConstraint::Get_Op() const
{
  return string(this->strOp);
}

void LinearConstraint::Set_a1(double a1)
{
  this->a1 = a1;
}

void LinearConstraint::Set_a2(double a2)
{
  this->a2 = a2;
}

void LinearConstraint::Set_b(double b)
{
  this->b = b;
}

void LinearConstraint::Set_Op(string strOp)
{
  string strCutOp(strOp,0,4);
  strcpy(this->strOp,strCutOp.c_str());
}

void LinearConstraint::PrintOut() const
{
  cout << "DEBUG: " << a1 << "*x + " << a2
       << "*y + " << b << " " << strOp << endl;
}

/*
4 Implementation of class ~SymbolicRelation~

*/

SymbolicRelation::SymbolicRelation(const int nConstraints,
      const int nTuple) :
      linConstraints(nConstraints),
      symbolicTuples(nTuple),
      mbbox(false)
{
  // nothing to do!
}

SymbolicRelation::SymbolicRelation(const SymbolicRelation& symRel):
  linConstraints(symRel.LinConstraintsSize()),
  symbolicTuples(symRel.SymbolicTuplesSize()),
  mbbox(symRel.BoundingBox())
{
  // copy the DBArray linConstraints:
  int i;
  for(i = 0; i < symRel.LinConstraintsSize(); i++ )
  {
    const LinearConstraint *linC;
    symRel.GetLinConstraints( i, linC );
    this->linConstraints.Put( i, *linC );
  }
  // copy the DBArray symbolicTuples:
  for(i = 0; i < symRel.SymbolicTuplesSize(); i++ )
  {
    const SymbolicTuple *iPair;
    symRel.GetSymbolicTuples( i, iPair );
    this->symbolicTuples.Put( i, *iPair );
  }
}

void SymbolicRelation::Destroy()
{
  this->linConstraints.Destroy();
  this->symbolicTuples.Destroy();
}

SymbolicRelation& SymbolicRelation::operator=(
  const SymbolicRelation& symRel )
{
  int i;
  this->linConstraints.Clear();
  if(symRel.LinConstraintsSize()>0)
  {
    this->linConstraints.Resize(symRel.LinConstraintsSize());
  }
  // copy the DBArray linConstraints:
  for(i = 0; i < symRel.LinConstraintsSize(); i++ )
  {
    const LinearConstraint *linC;
    symRel.GetLinConstraints( i, linC );
    this->linConstraints.Put( i, *linC );
  }
  // copy the DBArray symbolicTuples:
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
  // copy the minimum bounding box:
  this->mbbox = symRel.BoundingBox();
  return *this;
}

void SymbolicRelation::GetSymbolicTuples(const int i,
                          SymbolicTuple const*& symbolicTuple) const
{
  this->symbolicTuples.Get(i, symbolicTuple);
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

void SymbolicRelation::AddSymbolicTuple(
      const vector<LinearConstraint> vLinConstraints)
{
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
  // mbbox will be computed during normalisation (not yet)
  tuplePositionInArray.mbbox = Rectangle<2>(false);
  // append the tuple into the symbolicTuples-DBArray:
  this->symbolicTuples.Append(tuplePositionInArray);
}

void SymbolicRelation::AppendSymbolicRelation(
        const SymbolicRelation& otherSymRel)
{
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
}

void SymbolicRelation::JoinSymbolicRelation(
        const SymbolicRelation& otherSymRel)
{
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
           vSymbolicTuplesOld[i].BoundingBox().Intersects(
              symTuple->BoundingBox()))
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
          AddSymbolicTuple(vlinConstraints2Add);
        }
      }
    }
  }
}

bool SymbolicRelation::OverlapsSymbolicRelation(
                            const SymbolicRelation& otherSymRel) const
{
  int i, j;
  int k1,k2,k3,k4,k;

  if(this->BoundingBox().IsDefined() &&
     otherSymRel.BoundingBox().IsDefined() &&
     this->BoundingBox().Intersects(otherSymRel.BoundingBox()))
  {
	const LinearConstraint *linC;
    for(i = 0; i < this->SymbolicTuplesSize(); i++)
    {
      for(j = 0; j < otherSymRel.SymbolicTuplesSize(); j++)
      {
        const SymbolicTuple *symTuple1, *symTuple2;
        this->GetSymbolicTuples(i, symTuple1);
        otherSymRel.GetSymbolicTuples(j, symTuple2);
        if(symTuple1->BoundingBox().IsDefined() &&
           symTuple2->BoundingBox().IsDefined() &&
           symTuple1->BoundingBox().Intersects(
              symTuple2->BoundingBox()))
        {
		  vector<LinearConstraint> vCEQ;
          vector<LinearConstraint> vNEQ;
		  vector<Point2D> vConvexPolygon;
          k1 = symTuple1->startIndex;
          k2 = symTuple1->endIndex+1;
          k3 = symTuple2->startIndex;
          k4 = symTuple2->endIndex+1;
          for(k=k1; k < k2; k++)
          {
		  	this->GetLinConstraints(k, linC);
            if(linC->Get_Op()==OP_EQ)
            {
              vCEQ.push_back(*linC);
            }
            else // OP_LEQ
            {
              vNEQ.push_back(*linC);
            }
          }
          for(k=k3; k < k4; k++)
          {
            otherSymRel.GetLinConstraints(k, linC);
            if(linC->Get_Op()==OP_EQ)
            {
              vCEQ.push_back(*linC);
            }
            else // OP_LEQ
            {
              vNEQ.push_back(*linC);
            }
          }
          if(vCEQ.size() > 0)
          {
            for(k=0; k < vCEQ.size(); k++)
            {
              // Transformation of equation to equivalent unequations:
              // (a1*x+a2*y+b=0) <=> (a1*x+a2*y+b<=0) AND (-a1*x-a2*y-b<=0)
              LinearConstraint linConstraint1(vCEQ[k].Get_a1(),
                          vCEQ[k].Get_a2(), vCEQ[k].Get_b(), OP_LEQ);
              LinearConstraint linConstraint2(-vCEQ[k].Get_a1(),
                          -vCEQ[k].Get_a2(), -vCEQ[k].Get_b(), OP_LEQ);
              vNEQ.push_back(linConstraint1);
              vNEQ.push_back(linConstraint2);
            }
          }
          HalfPlaneIntersection(vNEQ, vConvexPolygon);
		  if(vConvexPolygon.size() > 0)
		  {
		   	// the conjuction of the two tuples is not empty
			// Early-Stop:
			return true;
		  }
        }
      }
    }
  }
  return false;
}

void SymbolicRelation::ProjectToAxis(const bool blnXAxis,
  const bool blnYAxis)
{
  // Prerequisite: blnXAxis XOR blnYAxis == TRUE
  int i;
  vector<SymbolicTuple> vSymbolicTuplesOld;
  for(i = 0; i < this->SymbolicTuplesSize(); i++)
    {
    const SymbolicTuple *symTuple;
    this->GetSymbolicTuples(i, symTuple);
    vSymbolicTuplesOld.push_back(*symTuple);
  }
  this->linConstraints.Clear();
  this->symbolicTuples.Clear();
  for(i = 0; i < vSymbolicTuplesOld.size(); i++ )
  {
    vector<LinearConstraint> vLinConstraints2Add;
    // each tuple will be treated here:
    if(blnXAxis)
    {
      const double wxLeft = vSymbolicTuplesOld[i].mbbox.MinD(0);
      const double wxRight = vSymbolicTuplesOld[i].mbbox.MaxD(0);
      // add a horizontal segment:
      LinearConstraint linConLine(0.0, 1.0, 0.0, OP_EQ);
      LinearConstraint linConLeft(-1.0, 0.0, wxLeft, OP_LEQ);
      LinearConstraint linConRight(1.0, 0.0, -wxRight, OP_LEQ);
      vLinConstraints2Add.push_back(linConLine);
      vLinConstraints2Add.push_back(linConLeft);
      vLinConstraints2Add.push_back(linConRight);
    }
    else // blnYAxis==true
    {
      const double wyBottom = vSymbolicTuplesOld[i].mbbox.MinD(1);
      const double wyTop = vSymbolicTuplesOld[i].mbbox.MaxD(1);
      // add a vertical segment:
      LinearConstraint linConLine(1.0, 0.0, 0.0, OP_EQ);
      LinearConstraint linConBottom(0.0, -1.0, wyBottom, OP_LEQ);
      LinearConstraint linConTop(0.0, 1.0, -wyTop, OP_LEQ);
      vLinConstraints2Add.push_back(linConLine);
      vLinConstraints2Add.push_back(linConBottom);
      vLinConstraints2Add.push_back(linConTop);
    }
    AddSymbolicTuple(vLinConstraints2Add);
  }
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
      if(!(AlmostEqual(linC->Get_a1(),0) &&
           AlmostEqual(linC->Get_a2(),0) &&
           AlmostEqual(linC->Get_b(),0)) &&
         !(AlmostEqual(linC->Get_a1(),0) &&
           AlmostEqual(linC->Get_a2(),0) &&
           linC->Get_b()<=0 && linC->Get_Op()==OP_LEQ))
      {
        IsRealSubSet = true;
      }
      linConIndex++;
    }
    while(!IsRealSubSet && linConIndex<=iGetSymTuple->endIndex);

    for(linConIndex=iGetSymTuple->startIndex;
        linConIndex<=iGetSymTuple->endIndex; linConIndex++)
    {
      const LinearConstraint *linC;
      this->GetLinConstraints(linConIndex, linC);
      if(!AlmostEqual(linC->Get_a1(),0) ||
         !AlmostEqual(linC->Get_a2(),0) ||
         !AlmostEqual(linC->Get_b(),0))
      {
        // if at minimum one of the coefficients is not equal 0
        // (otherwise we can ignore this linear constraint representing R^2
        // because there is an other constraint which intersects R^2)
        if(linC->Get_Op()==OP_EQ)
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

    if(IsRealSubSet)
    {
      if(vCEQ.size() > 0)
      {
        for(k=0; k < vCEQ.size(); k++)
        {
          // Transformation of equation to equivalent unequations:
          // (a1*x+a2*y+b=0) <=> (a1*x+a2*y+b<=0) AND (-a1*x-a2*y-b<=0)
          LinearConstraint linConstraint1(vCEQ[k].Get_a1(),
                      vCEQ[k].Get_a2(), vCEQ[k].Get_b(), OP_LEQ);
          LinearConstraint linConstraint2(-vCEQ[k].Get_a1(),
                      -vCEQ[k].Get_a2(), -vCEQ[k].Get_b(), OP_LEQ);
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
    vLinConstraints.clear();
    if(vConvexPolygon.size()==0)
    {
      // Spezialfall: unerfuellbares Tupel
      // do nothing (tuple will be eliminated)
    }
    else // vConvexPolygon.size() > 0
    {
      // Case: satis. tupel :
      blnRelIsEmptySet = false;
      ToConstraint(vConvexPolygon, vLinConstraints);
      iSymTuple.isNormal = true;
      iSymTuple.startIndex = lastEndIndex+1;
      iSymTuple.endIndex = lastEndIndex + vLinConstraints.size();
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
  }

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
}

// The following functions are needed for the SymbolicRelation
// class to act as an attribute of a relation:
int SymbolicRelation::NumOfFLOBs() const
{
  return 2;
}

FLOB* SymbolicRelation::GetFLOB(const int i)
{
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
  return 0; // no special order
}

bool SymbolicRelation::Adjacent(const Attribute* arg) const
{
  return false;
}

SymbolicRelation* SymbolicRelation::Clone() const
{
  return new SymbolicRelation(*this);
}

bool SymbolicRelation::IsDefined() const
{
  return true; // even the empty set is "defined"
}

void SymbolicRelation::SetDefined(bool Defined )
{
  // nothing to do
}

size_t SymbolicRelation::HashValue() const
{
  if(LinConstraintsSize()==0)
  {
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
      a1 = linC->Get_a1();
      a2 = linC->Get_a2();
      b = linC->Get_b();
      h=h+(unsigned long)(5*a1+5*a2+b); // Hash-Funktion
    }
    return size_t(h);
  }
}

size_t SymbolicRelation::Sizeof() const
{
  return sizeof( *this );
}

void SymbolicRelation::CopyFrom(const StandardAttribute* right)
{
  *this = *(const SymbolicRelation*)right;
}


const Rectangle<2> SymbolicRelation::BoundingBox() const
{
  if(!this->mbbox.IsDefined())
  {
  	return this->mbbox;
  }
  else
  {
  	double minx = mbbox.MinD(0);
	double maxx = mbbox.MaxD(0);
	double miny = mbbox.MinD(1);
	double maxy = mbbox.MaxD(1);
    return Rectangle<2>( true,
                       minx - FACTOR,
                       maxx + FACTOR,
                       miny - FACTOR,
                       maxy + FACTOR );
  }
}

/*
5 Implementation of auxiliary functions and structures

*/

/*
5.1 ~TriangulateRegion~-function

*/
void TriangulateRegion(const Region* reg,
  vector<vector<double> >& vVertices,
  vector<vector<int> >& vTriangles)
{
  int i;
  int noRetVertices = 0; // number of vertices which will be returned
  vTriangles.clear();
  vVertices.clear();
  if(!reg->IsEmpty())
  {
    // if it is not empty:
    Region* regionOriginal = new Region(*reg, false); // in memory
    Region* region2convert = new Region(*reg, true); // in memory,
                // but only with neccessary halfsegments


    // how many faces?
    regionOriginal->LogicSort();
    region2convert->LogicSort();
    const HalfSegment *hs, *hsnext;
    int currFace, currFace2convert, currCycle, currEdge;
    Point outputP, leftoverP;
    currFace = -1;
    currFace2convert = -1;
    currCycle = -1;
    currEdge = -1;

    vector<vector<Point> > vFace2convert;
    vector<Point> vPointsOfCycle;
    bool bln2Convert = false;

    for(i=0; i < region2convert->Size(); i++ )
    {
      region2convert->Get( i, hs );
      if(i < region2convert->Size()-1)
      {
        region2convert->Get( i+1, hsnext );
      }
      if(currFace != hs->attr.faceno || currCycle != hs->attr.cycleno)
      {
        if(currCycle != hs->attr.cycleno || currFace != hs->attr.faceno)
        {
          // new cycle:
          if(vPointsOfCycle.size() > 0)
          {
            vFace2convert.push_back(vPointsOfCycle);
          }
          vPointsOfCycle.clear();
          currCycle = hs->attr.cycleno;
        }
        if(currFace != hs->attr.faceno)
        {
          // new face:
          if(vFace2convert.size() > 0)
          {
            bln2Convert = true;
            currFace2convert = currFace;
          }
          currFace = hs->attr.faceno;
        }
        if ((hs->GetLeftPoint() == hsnext->GetLeftPoint()) ||
              ((hs->GetLeftPoint() == hsnext->GetRightPoint())))
        {
          outputP = hs->GetRightPoint();
          leftoverP = hs->GetLeftPoint();
        }
        else if ((hs->GetRightPoint() == hsnext->GetLeftPoint()) ||
                   ((hs->GetRightPoint() == hsnext->GetRightPoint())))
        {
          outputP = hs->GetLeftPoint();
          leftoverP = hs->GetRightPoint();
        }
        else
        {
          ErrorReporter::ReportError("INVALID INPUT: wrong data format.");
        }
      }
      else // same face, same cycle:
      {
        outputP=leftoverP;
        if (hs->GetLeftPoint() == leftoverP)
        {
          leftoverP = hs->GetRightPoint();
        }
        else if (hs->GetRightPoint() == leftoverP)
        {
          leftoverP = hs->GetLeftPoint();
        }
        else
        {
          ErrorReporter::ReportError("INVALID INPUT: wrong data format.");
        }
      }
      vPointsOfCycle.push_back(outputP);
      if(i==region2convert->Size()-1 || bln2Convert)
      {
        if(i==region2convert->Size()-1)
        {
          vFace2convert.push_back(vPointsOfCycle);
          currFace2convert = currFace;
        }
        int noPointsOfFace = 0;
        int noCyclesOfFace = vFace2convert.size();
        int cntr[noCyclesOfFace];
        int iCycle;
        for(iCycle=0; iCycle < noCyclesOfFace; iCycle++)
        {
          noPointsOfFace += vFace2convert[iCycle].size();
          cntr[iCycle] = vFace2convert[iCycle].size();
        }
        double vertices[noPointsOfFace+1][2];
        int iVertice = 1;
        for(iCycle=0; iCycle < noCyclesOfFace; iCycle++)
        {
          Region* directionCycle = new Region(0);
          directionCycle->StartBulkLoad();
          const HalfSegment *hsCopy;
          int anzahlHalfsegmente = 0;
          for(int k=0; k<regionOriginal->Size(); k++)
          {
            regionOriginal->Get(k, hsCopy);
            if(hsCopy->attr.faceno==currFace2convert &&
               hsCopy->attr.cycleno==iCycle)
            {
              HalfSegment hs(*hsCopy);
              AttrType attr=hsCopy->GetAttr();
              attr.faceno = 0;
              attr.cycleno = 0;
              hs.SetAttr(attr);
              (*directionCycle) += hs;
              anzahlHalfsegmente++;
            }
          }
          directionCycle->EndBulkLoad(true, false, false, false);
          bool directionCW = directionCycle->GetCycleDirection();
          delete directionCycle;
          // directionCW==true <=> clockwise order

          if((iCycle==0 && directionCW) || (iCycle>0 && !directionCW))
          {
            // then the point-order of the current cycle has
            // to be inverted:
            reverse(vFace2convert[iCycle].begin(),
              vFace2convert[iCycle].end());
          }

          for(int iPoint=0;
            iPoint < vFace2convert[iCycle].size(); iPoint++)
          {
            vertices[iVertice][X] =
              (double)vFace2convert[iCycle][iPoint].GetX();
            vertices[iVertice][Y] =
        (double)vFace2convert[iCycle][iPoint].GetY();
            vector<double> vDoublePair(2);
            vDoublePair[0] = vertices[iVertice][X];
            vDoublePair[1] = vertices[iVertice][Y];
            vVertices.push_back(vDoublePair);
            noRetVertices++;
            iVertice++;
          }
        }

        int noOfTriangles = (noPointsOfFace-2)+2*(noCyclesOfFace-1);
        int triangles[noOfTriangles][3];

        int ret = triangulate_polygon(noCyclesOfFace, cntr, vertices,
                        triangles);
        for (int j = 0; j < noOfTriangles; j++)
        {
          // each triangle will be added to vector triangles:
          vector<int> vTriangleIndices(3);
          vTriangleIndices[0] = triangles[j][0]+
            noRetVertices-noPointsOfFace-1;
          vTriangleIndices[1] = triangles[j][1]+
            noRetVertices-noPointsOfFace-1;
          vTriangleIndices[2] = triangles[j][2]+
            noRetVertices-noPointsOfFace-1;
          vTriangles.push_back(vTriangleIndices);
        }
        vFace2convert.clear();
        bln2Convert = false;
      }
    }
  }
}


/*
5.2 ~GetTheta~-function

*/
double GetTheta(Point2D& p1, Point2D& p2)
{
  double theta, dx, dy, ax, ay, t;
  dx = p2.x - p1.x;
  dy = p2.y - p1.y;
  ax = fabs(dx);
  ay = fabs(dy);
  if(ax==0 && ay==0)
  {
    t = 0;
  }
  else
  {
    t = dy/(ax+ay);
  }
  if(dx<0)
  {
    t = 2-t;
  }
  else if(dy<0)
  {
    t = 4+t;
  }
  theta = 360.0-t*90.0;
  return theta;
}



double GetOrientedArea(double px, double py,
             double vx, double vy,
             double qx, double qy)
{
  return ((qx-px)*(qy+py) +
         (vx-qx)*(vy+qy) +
         (px-vx)*(py+vy))/2;

}

/*
5.3 ~MergeTriangles2ConvexPolygons~-function and structure

Additional datastructure for the ~MergeTriangles2ConvexPolygons~-function

*/
struct EdgeRef
{
  const vector<double>* FromPoint;
  const vector<double>* ToPoint;
  int Edge;
};

class EdgeRefComparator
{
  public:
    bool IsLess(const EdgeRef& A, const EdgeRef& B)
    {
      double x1_A, y1_A, x2_A, y2_A, x1_B, y1_B, x2_B, y2_B;
      if((*A.FromPoint)[X] < (*A.ToPoint)[X] ||
         (AlmostEqual((*A.FromPoint)[X],(*A.ToPoint)[X]) &&
         (*A.FromPoint)[Y] < (*A.ToPoint)[Y]))
      {
        x1_A = (*A.FromPoint)[X];
        y1_A = (*A.FromPoint)[Y];
        x2_A = (*A.ToPoint)[X];
        y2_A = (*A.ToPoint)[Y];
      }
      else
      {
        x1_A = (*A.ToPoint)[X];
        y1_A = (*A.ToPoint)[Y];
        x2_A = (*A.FromPoint)[X];
        y2_A = (*A.FromPoint)[Y];
      }
      if((*B.FromPoint)[X] < (*B.ToPoint)[X] ||
         (AlmostEqual((*B.FromPoint)[X],(*B.ToPoint)[X]) &&
         (*B.FromPoint)[Y] < (*B.ToPoint)[Y]))
        {
        x1_B = (*B.FromPoint)[X];
        y1_B = (*B.FromPoint)[Y];
        x2_B = (*B.ToPoint)[X];
        y2_B = (*B.ToPoint)[Y];
      }
      else
      {
        x1_B = (*B.ToPoint)[X];
        y1_B = (*B.ToPoint)[Y];
        x2_B = (*B.FromPoint)[X];
        y2_B = (*B.FromPoint)[Y];
      }
      if(x1_A < x1_B ||
        (AlmostEqual(x1_A,x1_B) && y1_A < y1_B) ||
        (AlmostEqual(x1_A,x1_B) && AlmostEqual(y1_A,y1_B) &&
        (x2_A < x2_B || AlmostEqual(x2_A,x2_B) && y2_A < y2_B)))
      {
        return true;
      }
      return false;
    }

    bool IsEqual(const EdgeRef& A, const EdgeRef& B)
    {
      return !IsLess(A, B) && !IsLess(B, A);
    }

    bool operator()(const EdgeRef& A, const EdgeRef& B)
    {
      return IsLess(A, B);
    }
};

struct CWPoint
{
  Point2D Point;
  double Theta;
};

class CWPointComparator
{
  public:
    bool IsLess(const CWPoint& A, const CWPoint& B)
    {
      if(A.Theta<B.Theta)
      {
        return true;
      }
    return false;
    }

    bool IsEqual(const CWPoint& A, const CWPoint& B)
    {
      return !IsLess(A, B) && !IsLess(B, A);
    }

    bool operator()(const CWPoint& A, const CWPoint& B)
    {
      return IsLess(A, B);
    }
};

/*
Implementation of the ~MergeTriangles2ConvexPolygons~-function

*/
void MergeTriangles2ConvexPolygons(
  const vector<vector<double> >& vVertices,
  const vector<vector<int> >& vTriangles,
  vector<vector<Point2D> >& vCWPoints)
{
  int i;
  const int FROM = 0;
  const int TO = 1;
  const int NEXT = 2;
  const int PARTNER = 3;
  const int COMPONENT = 4;
  const int DELETED = 5;
  const int NO_EDGES = 0;
  const int FIRST = 1;

  vector<vector<int> > vEdges;
  vector<EdgeRef> vEdgesRefSort;
  vector<vector<int> > vComponents;
  int currComponent = 0;

  // initialize vEdges, vEdgesRefSort, vComponents:

  for(i=0; i<vTriangles.size(); i++)
  {
    // fill vEdges:
    vector<int> vEdgeInfo;
    // insert first edge of current triangle:
    vEdgeInfo = vector<int>(6);
    vEdgeInfo[FROM] = vTriangles[i][0];
    vEdgeInfo[TO] = vTriangles[i][1];
    vEdgeInfo[NEXT] = (3*i+1);
    vEdgeInfo[PARTNER] = -1;
    vEdgeInfo[COMPONENT] = currComponent;
  vEdgeInfo[DELETED] = 0;
    vEdges.push_back(vEdgeInfo);
    // insert second edge of current triangle:
    vEdgeInfo = vector<int>(6);
    vEdgeInfo[FROM] = vTriangles[i][1];
    vEdgeInfo[TO] = vTriangles[i][2];
    vEdgeInfo[NEXT] = (3*i+2);
    vEdgeInfo[PARTNER] = -1;
    vEdgeInfo[COMPONENT] = currComponent;
  vEdgeInfo[DELETED] = 0;
    vEdges.push_back(vEdgeInfo);
    // insert third edge of current triangle:
    vEdgeInfo = vector<int>(6);
    vEdgeInfo[FROM] = vTriangles[i][2];
    vEdgeInfo[TO] = vTriangles[i][0];
    vEdgeInfo[NEXT] = -1;
    vEdgeInfo[PARTNER] = -1;
    vEdgeInfo[COMPONENT] = currComponent;
  vEdgeInfo[DELETED] = 0;
    vEdges.push_back(vEdgeInfo);

    // fill vEdgesRefSort:
    EdgeRef oneEdgeRef;
    // first:
    oneEdgeRef.FromPoint = &vVertices[vTriangles[i][0]];
    oneEdgeRef.ToPoint = &vVertices[vTriangles[i][1]];
    oneEdgeRef.Edge = (3*i);
    vEdgesRefSort.push_back(oneEdgeRef);
    // second:
    oneEdgeRef.FromPoint = &vVertices[vTriangles[i][1]];
    oneEdgeRef.ToPoint = &vVertices[vTriangles[i][2]];
    oneEdgeRef.Edge = (3*i+1);
    vEdgesRefSort.push_back(oneEdgeRef);
    // third:
    oneEdgeRef.FromPoint = &vVertices[vTriangles[i][2]];
    oneEdgeRef.ToPoint = &vVertices[vTriangles[i][0]];
    oneEdgeRef.Edge = (3*i+2);
    vEdgesRefSort.push_back(oneEdgeRef);

    // fill vComponents:
    vector<int> vComponentInfo(2);
    vComponentInfo[NO_EDGES] = 3;
    vComponentInfo[FIRST] = (3*i);
    vComponents.push_back(vComponentInfo);

    currComponent++;
  }
  sort(vEdgesRefSort.begin(), vEdgesRefSort.end(), EdgeRefComparator());

  // set PARTNER of each diagonal in vEdges
  // (only one edge for each diagonal!)
  EdgeRefComparator cmpEdgeRef;
  for(i=0; i<vEdgesRefSort.size()-1; i++)
  {
     if(cmpEdgeRef.IsEqual(vEdgesRefSort[i], vEdgesRefSort[i+1]))
    {
      vEdges[vEdgesRefSort[i].Edge][PARTNER] =
        vEdgesRefSort[i+1].Edge;
      // leave (vEdges[vEdgesRefSort[i+1].Edge][PARTNER] == -1),
      //otherwise the step "remove not neccessary diagonals" is done
      // twice for each diagonal!
    i++;
    }
  }

  // check each edge if it's a diagonal or not
  //   if yes: check if it's not a neccessary diagonal
  //     if yes: delete the current diagonal
  for(i=0; i<vEdges.size(); i++)
  {
     if(vEdges[i][PARTNER]!=-1 &&
        vEdges[i][COMPONENT]!=vEdges[vEdges[i][PARTNER]][COMPONENT])
    {
    int iC1 = vEdges[i][COMPONENT];
    int iC2 = vEdges[vEdges[i][PARTNER]][COMPONENT];
	// only for (not already visited) diagonals:

      int iC1Edge = vComponents[iC1][FIRST];
      int iC1VertexAdjacent2From, iC1VertexAdjacent2To;
    assert(iC1Edge!=-1);
      while(iC1Edge!=-1)
      {
        if(i!=iC1Edge && vEdges[iC1Edge][DELETED]==0)
        {
          if(vEdges[i][FROM]==vEdges[iC1Edge][FROM])
          {
            iC1VertexAdjacent2From = vEdges[iC1Edge][TO];
          }
          else if(vEdges[i][FROM]==vEdges[iC1Edge][TO])
          {
            iC1VertexAdjacent2From = vEdges[iC1Edge][FROM];
          }
          if(vEdges[i][TO]==vEdges[iC1Edge][FROM])
          {
            iC1VertexAdjacent2To = vEdges[iC1Edge][TO];
          }
          else if(vEdges[i][TO]==vEdges[iC1Edge][TO])
          {
            iC1VertexAdjacent2To = vEdges[iC1Edge][FROM];
          }
        }
        iC1Edge = vEdges[iC1Edge][NEXT];
      }
      int iC2Edge = vComponents[iC2][FIRST];
      int iC2VertexAdjacent2From, iC2VertexAdjacent2To;
      while(iC2Edge!=-1)
      {
        if(vEdges[i][PARTNER]!=iC2Edge &&
          vEdges[iC2Edge][DELETED]==0)
        {
          if(vEdges[i][FROM]==vEdges[iC2Edge][FROM])
          {
            iC2VertexAdjacent2From = vEdges[iC2Edge][TO];
          }
          else if(vEdges[i][FROM]==vEdges[iC2Edge][TO])
          {
            iC2VertexAdjacent2From = vEdges[iC2Edge][FROM];
          }
          if(vEdges[i][TO]==vEdges[iC2Edge][FROM])
          {
            iC2VertexAdjacent2To = vEdges[iC2Edge][TO];
          }
          else if(vEdges[i][TO]==vEdges[iC2Edge][TO])
          {
            iC2VertexAdjacent2To = vEdges[iC2Edge][FROM];
          }
        }
        iC2Edge = vEdges[iC2Edge][NEXT];
      }

  bool blnInnenWinkelFromOK, blnInnenWinkelToOK;
  double oriatedAreaBefore;
  double oriatedAreaAfter;

  oriatedAreaBefore = GetOrientedArea(
                    vVertices[vEdges[i][TO]][X],
                    vVertices[vEdges[i][TO]][Y],
                    vVertices[vEdges[i][FROM]][X],
                    vVertices[vEdges[i][FROM]][Y],
                    vVertices[iC1VertexAdjacent2From][X],
                  vVertices[iC1VertexAdjacent2From][Y]);
  oriatedAreaAfter = GetOrientedArea(
                    vVertices[iC2VertexAdjacent2From][X],
                  vVertices[iC2VertexAdjacent2From][Y],
                    vVertices[vEdges[i][FROM]][X],
                    vVertices[vEdges[i][FROM]][Y],
                    vVertices[iC1VertexAdjacent2From][X],
                  vVertices[iC1VertexAdjacent2From][Y]);

  if(AlmostEqual(oriatedAreaBefore, 0))
  {
    blnInnenWinkelFromOK = false;
  }
  else
  {
    blnInnenWinkelFromOK = (oriatedAreaBefore*oriatedAreaAfter>=0);
  }

  oriatedAreaBefore = GetOrientedArea(
                    vVertices[vEdges[i][FROM]][X],
                    vVertices[vEdges[i][FROM]][Y],
                    vVertices[vEdges[i][TO]][X],
                    vVertices[vEdges[i][TO]][Y],
                    vVertices[iC1VertexAdjacent2To][X],
                  vVertices[iC1VertexAdjacent2To][Y]);
  oriatedAreaAfter = GetOrientedArea(
                    vVertices[iC2VertexAdjacent2To][X],
                  vVertices[iC2VertexAdjacent2To][Y],
                    vVertices[vEdges[i][TO]][X],
                    vVertices[vEdges[i][TO]][Y],
                    vVertices[iC1VertexAdjacent2To][X],
                  vVertices[iC1VertexAdjacent2To][Y]);

  if(AlmostEqual(oriatedAreaBefore, 0))
  {
    blnInnenWinkelToOK = false;
  }
  else
  {
    blnInnenWinkelToOK = (oriatedAreaBefore*oriatedAreaAfter>=0);
  }

    if(blnInnenWinkelFromOK && blnInnenWinkelToOK)
      {
        // then the diagonal is not neccessary: do a merge:
        int iSmallerComp, iBiggerComp;
        int iSmallerCompEdge, iSmallerCompLastEdge;
        if(vComponents[iC1][NO_EDGES] <= vComponents[iC2][NO_EDGES])
        {
          iSmallerComp = iC1;
          iBiggerComp = iC2;
        }
        else
        {
          iSmallerComp = iC2;
          iBiggerComp = iC1;
        }

        iSmallerCompEdge = vComponents[iSmallerComp][FIRST];
        while(iSmallerCompEdge!=-1)
        {
          if(vEdges[iSmallerCompEdge][NEXT]==-1)
          {
        assert(vEdges[iSmallerCompEdge][COMPONENT] == iSmallerComp);
            iSmallerCompLastEdge = iSmallerCompEdge;
          }
      vEdges[iSmallerCompEdge][COMPONENT] = iBiggerComp;
          iSmallerCompEdge = vEdges[iSmallerCompEdge][NEXT];
        }
        vEdges[iSmallerCompLastEdge][NEXT] =
          vComponents[iBiggerComp][FIRST];
        vComponents[iBiggerComp][FIRST] =
          vComponents[iSmallerComp][FIRST];
        vComponents[iBiggerComp][NO_EDGES] +=
          vComponents[iSmallerComp][NO_EDGES];
        vComponents[iSmallerComp][NO_EDGES] = 0;
        vComponents[iSmallerComp][FIRST] = -1;
        // Diagonalen streichen: (bzw. als "gestirchen" markieren)
        vEdges[i][DELETED] = 1;
        vEdges[vEdges[i][PARTNER]][DELETED] = 1;
    }

    }
  }

  vCWPoints.clear();

  for(i=0; i<vComponents.size(); i++)
  {
    int iCompEdge = vComponents[i][FIRST];
    if(iCompEdge != -1)
    {
      vector<Point2D> vConvexComponent;
      vector<CWPoint> vCWPointSet;
      CWPoint oneCWPoint;
      oneCWPoint.Theta = -1.0;
      while(iCompEdge!=-1)
      {
        if(vEdges[iCompEdge][DELETED]==0)
        {
          // Add the FROM-Point:
          oneCWPoint.Point = Point2D(
              vVertices[vEdges[iCompEdge][FROM]][X],
              vVertices[vEdges[iCompEdge][FROM]][Y]);
          vCWPointSet.push_back(oneCWPoint);
          // Add the TO-Point:
          oneCWPoint.Point = Point2D(
              vVertices[vEdges[iCompEdge][TO]][X],
              vVertices[vEdges[iCompEdge][TO]][Y]);
          vCWPointSet.push_back(oneCWPoint);
        }
        iCompEdge = vEdges[iCompEdge][NEXT];
      }
      // determine a anker (a point inside the polygon) for cw-comput:
    // let's take the barycentre (Schwerpunkt)
    int k;
    double x_s = 0;
    double y_s = 0;
    for(k=0; k<vCWPointSet.size(); k++)
    {
        x_s +=  vCWPointSet[k].Point.x;
    y_s +=  vCWPointSet[k].Point.y;
      }
    x_s = x_s / vCWPointSet.size();
    y_s = y_s / vCWPointSet.size();

      Point2D ankerPoint(x_s, y_s);

    // compute theta of each point in linear time:
      for(k=0; k<vCWPointSet.size(); k++)
      {
      vCWPointSet[k].Theta = GetTheta(ankerPoint, vCWPointSet[k].Point);
      }
      // sort in cw-order:
      sort(vCWPointSet.begin(), vCWPointSet.end(), CWPointComparator());

      for(int k1=0; k1<vCWPointSet.size(); k1++)
      {
        Point2D nextCWPoint2D = vCWPointSet[k1].Point;
        if(k1==0 ||
           nextCWPoint2D!=vConvexComponent[vConvexComponent.size()-1])
        {
          vConvexComponent.push_back(nextCWPoint2D);
        }
      }
      vCWPoints.push_back(vConvexComponent);
    }
  }
}

/*
5.4 ~GetIntersectionPoint~-function (with linear constraints)

*/
Point2D GetIntersectionPoint(const LinearConstraint& linConFirst,
  const LinearConstraint& linConSecond)
{
  double x = (linConFirst.Get_a2()*linConSecond.Get_b()-
      linConSecond.Get_a2()*linConFirst.Get_b())/
        (linConFirst.Get_a1()*linConSecond.Get_a2()-
        linConSecond.Get_a1()*linConFirst.Get_a2());
  double y = (linConFirst.Get_a1()*linConSecond.Get_b()-
    linConSecond.Get_a1()*linConFirst.Get_b())/
      (linConFirst.Get_a2()*linConSecond.Get_a1()-
      linConSecond.Get_a2()*linConFirst.Get_a1());
  Point2D pIntersection(x, y);
  return pIntersection;
}

/*
5.5 ~MinimumBoundingBox~-function

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
5.6 ~ToConstraint~-function for minimum bounding boxes

*/
void ToConstraint(const Rectangle<2>& mbbox,
      vector<LinearConstraint>& vLinConstraints)
{
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
}

/*
5.7 ~ToConstraint~-function for convex polygon

*/
void ToConstraint(const vector<Point2D>& vConvexPolygon,
        vector<LinearConstraint>& vLinConstraints)
{
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
      if(vConvexPolygon[i].OnSameLineAs(vConvexPolygon[0],
          vConvexPolygon[j]))
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
          linC.Set_a1(leq_factor);
          linC.Set_a2(0.0);
          linC.Set_b(leq_factor*(-pFirst.x));
          linC.Set_Op(OP_LEQ);
        }
        else // pFirst.x!=pSecond.x
        {
          double m,b; // y = mx+b
          //             => (mx -y +b = 0)
          m = (pSecond.y-pFirst.y)/(pSecond.x-pFirst.x);
          // m is defined because of (pFirst.x!=pSecond.x)
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
          linC.Set_a1(leq_factor*m);
          linC.Set_a2(leq_factor*(-1.0));
          linC.Set_b(leq_factor*b);
          linC.Set_Op(OP_LEQ);
        }
        vLinConstraints.push_back(linC);
      }
    }
  }
}

/*
5.8 ~ComputeXOrderedSequenceOfSlabs~-function

*/
void ComputeXOrderedSequenceOfSlabs(const vector<Point2D>& vP,
            vector<Point2D>& vUpperBoundary,
            vector<Point2D>& vLowerBoundary)
{
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
    if(AlmostEqual(vP[i].x,x_min_P) && i_x_minFirst==-1)
    {
      i_x_minFirst = i;
    }
    else if(AlmostEqual(vP[i].x,x_min_P) && i_x_minSecond==-1)
    {
       i_x_minSecond = i;
    }
    else if(AlmostEqual(vP[i].x,x_min_P))
    {
        ErrorReporter::ReportError("ERROR");    }
    if(AlmostEqual(vP[i].x,x_max_P) && i_x_maxFirst==-1)
    {
      i_x_maxFirst = i;
    }
    else if(AlmostEqual(vP[i].x,x_max_P) && i_x_maxSecond==-1)
    {
       i_x_maxSecond = i;
    }
    else if(AlmostEqual(vP[i].x,x_max_P))
    {
        ErrorReporter::ReportError("ERROR");
    }
  }


  if(i_x_minSecond!=-1 && !(i_x_minSecond==i_x_minFirst+1 ||
          (i_x_minSecond==vP.size()-1 && i_x_minFirst==0)))
  {
        ErrorReporter::ReportError("ERROR");
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
       // (only First OR First up, Second down)
      blnScanEnd = ((i % vP.size()) == ((i_x_maxFirst+1) % vP.size()));
    }
    else // (First down, Second up)
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
      // (only First ODER First down, Second up)
      blnScanEnd = ((i % vP.size()) == ((i_x_minFirst+1) % vP.size()));
    }
    else // (First up, Second down)
    {
      blnScanEnd = ((i % vP.size()) == ((i_x_minSecond+1) % vP.size()));
    }
  }
  reverse(vLowerBoundary.begin(), vLowerBoundary.end());
}

/*
The ~PrintSlab~-function is just a tool for debug \& test-activities.

*/

void PrintSlab(VerticalTrapez currentSlab)
{
   cout << endl << "DEBUG: currentSlab:" << endl;
   cout << "DEBUG: -> currentSlab.IsEmpty == " << currentSlab.IsEmpty
       << endl;
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
5.9 ~GetPointFromSegment~-function

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
5.10 ~GetIntersectionPoint~-function (with Segments)

Input: two Segemnts

Prerequisite: ~pSegment1First~ $\neq$ ~pSegment1Second~ and
              ~pSegment2First~ $\neq$ ~pSegment2Second~

Output:
returns ~IsPoint~==~true~ if there is a single inters point pIntersection
returns ~IsPoint~==~false~ if there is either no intersections point OR
 more than one sinlge intersection point.

*/
void GetIntersectionPoint(
      const Point2D& pSegment1First,
      const Point2D& pSegment1Second,
      const Point2D& pSegment2First,
      const Point2D& pSegment2Second,
      bool& IsPoint, Point2D& pIntersection)
{
  bool blnSegment1Vertical;
  Point2D pSegment1Upp, pSegment1Low;
  Point2D pSegment1Left, pSegment1Right;
  if(AlmostEqual(pSegment1First.x,pSegment1Second.x))
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
  if(AlmostEqual(pSegment2First.x,pSegment2Second.x))
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
    if(AlmostEqual(pSegment1First.x, pSegment2First.x))
    {
      // two vertical segments with same x-coordnates
      // can intersect in one single point
      if(AlmostEqual(pSegment1Upp.y,pSegment2Low.y))
      {
        IsPoint = true;
        pIntersection = pSegment1Upp;
      }
      else if(AlmostEqual(pSegment2Upp.y,pSegment1Low.y))
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
5.11 ~ConvexPolygonIntersection~-function

*/
void ConvexPolygonIntersection(const vector<Point2D>& vP,
        const vector<Point2D>& vQ,
        vector<Point2D>& vPQIntersection)
{
  int i;
  vector<Point2D> vPQIntersectionWD; // with possibly duplicates
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
  else if((vP.size()==1 && vQ.size()==2) ||
          (vP.size()==2 && vQ.size()==1))
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
    if(AlmostEqual((*vTwoElementPointSet)[0].x,
      (*vTwoElementPointSet)[1].x))
    {
      // vertical case
	  double upp_y, low_y;
	  if((*vTwoElementPointSet)[0].y < (*vTwoElementPointSet)[1].y)
      {
	  	upp_y = (*vTwoElementPointSet)[1].y;
		low_y = (*vTwoElementPointSet)[0].y;
	  }
	  else
      {
	  	upp_y = (*vTwoElementPointSet)[0].y;
		low_y = (*vTwoElementPointSet)[1].y;
	  }
      if(pSinglePoint->OnSameLineAs((*vTwoElementPointSet)[0],
               (*vTwoElementPointSet)[1])
        && (low_y < (*pSinglePoint).y ||
          AlmostEqual(low_y, (*pSinglePoint).y))
        && (*pSinglePoint).y < upp_y ||
          AlmostEqual((*pSinglePoint).y, upp_y))
      {
         vPQIntersectionWD.push_back(*pSinglePoint);
      }
    }
    else
    {
      // non-vertical case
	  double left_x, right_x;
	  if((*vTwoElementPointSet)[0].x < (*vTwoElementPointSet)[1].x)
      {
	  	left_x = (*vTwoElementPointSet)[0].x;
		right_x = (*vTwoElementPointSet)[1].x;
	  }
	  else
      {
	  	left_x = (*vTwoElementPointSet)[1].x;
		right_x = (*vTwoElementPointSet)[0].x;
	  }
      if(pSinglePoint->OnSameLineAs((*vTwoElementPointSet)[0],
            (*vTwoElementPointSet)[1])
        && (left_x < (*pSinglePoint).x ||
          AlmostEqual(left_x, (*pSinglePoint).x)) &&
                ((*pSinglePoint).x < right_x ||
          AlmostEqual((*pSinglePoint).x, right_x)))
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
  else if((vP.size()==1 && vQ.size()>=3) ||
          (vQ.size()==1 && vP.size()>=3))
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
    //fuer Vektor vLinConstraints:" << endl;
    for(i=0; i<vLinConstraints.size(); i++)
    {
      double dblSinglePoint =
       vLinConstraints[i].Get_a1()*pSinglePoint.x +
       vLinConstraints[i].Get_a2()*pSinglePoint.y +
         vLinConstraints[i].Get_b();
      if(!(dblSinglePoint < 0 || AlmostEqual(dblSinglePoint, 0)))
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
  else if((vP.size()==2 && vQ.size()>=3) ||
          (vQ.size()==2 && vP.size()>=3))
  {
    // There are three possible cases:
    // (i)   both endpoints of the segment are inside the convex polygon
    // (ii)  one endpoint of the segment is inside the convex polygon
    //       whereas the other one is really outside
    // (iii) both endpoints of the segment are really
    //     outside the convex polygon
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
    for(i=0; i<vLinConstraints.size(); i++)
    {
      double dblSegFirst =  vLinConstraints[i].Get_a1()*pSegmentFirst.x +
        vLinConstraints[i].Get_a2()*pSegmentFirst.y +
        vLinConstraints[i].Get_b();
      double dblSegSecond =
          vLinConstraints[i].Get_a1()*pSegmentSecond.x +
          vLinConstraints[i].Get_a2()*pSegmentSecond.y +
          vLinConstraints[i].Get_b();
	  if(!(dblSegFirst < 0 || AlmostEqual(dblSegFirst, 0)))
      {
        blnSegmentFirstInside = false;
      }
	  if(!(dblSegSecond < 0 || AlmostEqual(dblSegSecond, 0)))
      {
        blnSegmentSecondInside = false;
      }
    }


    if(blnSegmentFirstInside && blnSegmentSecondInside)
    {
      // case (i) both endpoints of the segment are
      // inside the convex polygon:
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
        if(((Point2D)((*vPolygon)[i])).OnSegment(pSegmentFirst,
          pSegmentSecond))
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
              ErrorReporter::ReportError("ERROR: "
              "in ConvexPolygonIntersection: "
        "Case (vP.size()==2 && vQ.size()>=3) || (vQ.size()==2 && "
        "vP.size()>=3), nrOfVertexes==0: more than two intersection "
        "points found.");
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
        ErrorReporter::ReportError("ERROR: wrong nrOfVertexes "
          "in ConvexPolygonIntersection");
      }
    }
  }
  else // (vP.size()>=3 && vQ.size()>=3)
  {
     // Compute x-ordered sequence of slabs of P's upper and lower bound.:
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
      blnElementOfP = (AlmostEqual(next_x,vUppBoundP[iUppBoundP].x)) ||
            (AlmostEqual(next_x,vLowBoundP[iLowBoundP].x));
      blnStartOfP = (AlmostEqual(next_x,vUppBoundP[0].x));
      // x-coord from upp and low at start of P equal!
      blnEndOfP = (AlmostEqual(next_x,vUppBoundP[vUppBoundP.size()-1].x));
      // x-coord from upp and low at end of P equal!
      blnElementOfQ = (AlmostEqual(next_x,vUppBoundQ[iUppBoundQ].x)) ||
          (AlmostEqual(next_x,vLowBoundQ[iLowBoundQ].x));
      blnStartOfQ = (AlmostEqual(next_x,vUppBoundQ[0].x));
      // x-coord from upp and low at start of Q equal!
      blnEndOfQ = (AlmostEqual(next_x,vUppBoundQ[vUppBoundQ.size()-1].x));
      // x-coord from upp and low at end of Q equal!



      if(blnElementOfP)
      {
        // (case 1) there exist pi of P with next_x = x-coord of point pi
        if(blnStartOfP) // (case 1.1) pi = startpoint of P
        {
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
          leftSlabP.IsEmpty = false;
          leftSlabP.pUppRight = vUppBoundP[vUppBoundP.size()-1];
          leftSlabP.pLowRight = vLowBoundP[vLowBoundP.size()-1];
          rightSlabP.pUppLeft = leftSlabP.pUppRight;
          rightSlabP.pLowLeft = leftSlabP.pLowRight;
          rightSlabP.IsEmpty = false;
        }
        else //(case 1.3) pi=element of (P without {startpoint, endpoint})
        {
          // leftSlabP.IsEmpty and rightSlabP.IsEmpty don't change (stay f)
          if(AlmostEqual(next_x,vUppBoundP[iUppBoundP].x))
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
          if(AlmostEqual(next_x,vLowBoundP[iLowBoundP].x))
          {
            leftSlabP.pLowRight = vLowBoundP[iLowBoundP];
            rightSlabP.pLowLeft = leftSlabP.pLowRight;

          }
          else  // then we have to compute the cut at x=next_x :
          {
            // there exists the points vLowBoundP[iLowBoundP-1] and
            // vLowBoundP[iLowBoundP] (!)
            Point2D pLow(GetPointFromSegment(next_x,
                    vLowBoundP[iLowBoundP-1],
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
          // leftSlabP.IsEmpty and rightSlabP.IsEmpty don't change (stay t)
          // do nothing!
        }
        else if(next_x > vUppBoundP[vUppBoundP.size()-1].x)
        {
          // (case 2.2) next_x > x-coord of endpoint of P
          // (x-coord from upp and low at end of P equal!)
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
          // leftSlabP.IsEmpty and rightSlabP.IsEmpty don't change (stay f)
        }
      }

      if(blnElementOfQ)
      {
        // (case 1) there exist pi of Q with next_x = x-coord of point pi
        if(blnStartOfQ)
        {
          // (case 1.1) pi = startpoint of Q
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
          leftSlabQ.IsEmpty = false;
          leftSlabQ.pUppRight = vUppBoundQ[vUppBoundQ.size()-1];
          leftSlabQ.pLowRight = vLowBoundQ[vLowBoundQ.size()-1];
          rightSlabQ.pUppLeft = leftSlabQ.pUppRight;
          rightSlabQ.pLowLeft = leftSlabQ.pLowRight;
          rightSlabQ.IsEmpty = false;
        }
        else //(case 1.3) pi=element of (Q without {startpoint, endpoint})
        {
          // leftSlabQ.IsEmpty and rightSlabQ.IsEmpty don't change (stay f)
          if(AlmostEqual(next_x,vUppBoundQ[iUppBoundQ].x))
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
          if(AlmostEqual(next_x,vLowBoundQ[iLowBoundQ].x))
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
          // leftSlabQ.IsEmpty and rightSlabQ.IsEmpty don't change (stay t)
          // do nothing!
        }
        else if(next_x > vUppBoundQ[vUppBoundQ.size()-1].x)
        {
          // (case 2.2) next_x > x-coord of endpoint of Q
          // (x-coord from upp and low at end of Q equal!)
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
          // leftSlabQ.IsEmpty and rightSlabQ.IsEmpty
          // don't change (stay false)
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

      if(AlmostEqual(vUppBoundP[iUppBoundP].x,next_x) &&
        iUppBoundP < vUppBoundP.size()-1)
      {
        iUppBoundP++;
      }
      if(AlmostEqual(vLowBoundP[iLowBoundP].x,next_x) &&
        iLowBoundP < vLowBoundP.size()-1)
      {
        iLowBoundP++;
      }
      if(AlmostEqual(vUppBoundQ[iUppBoundQ].x,next_x) &&
        iUppBoundQ < vUppBoundQ.size()-1)
      {
        iUppBoundQ++;
      }
      if(AlmostEqual(vLowBoundQ[iLowBoundQ].x,next_x) &&
        iLowBoundQ < vLowBoundQ.size()-1)
      {
        iLowBoundQ++;
      }

      blnStartOfMerge = false;
    }

    // Attetion: it's possible that a Trapez T is degenerated (case *):
    // T.IsEmpty==false && T.pUppLeft==T.pUppRight &&
    // T.pLowLeft==T.pLowRight
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
      if(!vTrapezP[i].IsEmpty && !vTrapezQ[i].IsEmpty)
      {
        // Trapez are not both emtpy!
        blnDegeneratedTrapezP =
          (vTrapezP[i].pUppLeft==vTrapezP[i].pUppRight);
        blnDegeneratedTrapezQ =
          (vTrapezQ[i].pUppLeft==vTrapezQ[i].pUppRight);
        blnDegeneratedCase = (blnDegeneratedTrapezP ||
          blnDegeneratedTrapezQ);
        // Left Border Check:
        if((vTrapezQ[i].pLowLeft.y <= vTrapezP[i].pUppLeft.y) &&
           (vTrapezP[i].pUppLeft.y <= vTrapezQ[i].pUppLeft.y) &&
           (!blnDegeneratedCase || AlmostEqual(vTrapezP[i].pUppLeft.x,
                vTrapezQ[i].pUppLeft.x)))
        {
          vPQIntersectionUppWD.push_back(vTrapezP[i].pUppLeft);
        }
        if((vTrapezP[i].pLowLeft.y <= vTrapezQ[i].pUppLeft.y) &&
           (vTrapezQ[i].pUppLeft.y <= vTrapezP[i].pUppLeft.y) &&
           (!blnDegeneratedCase || AlmostEqual(vTrapezP[i].pUppLeft.x,
                vTrapezQ[i].pUppLeft.x)))
        {
          vPQIntersectionUppWD.push_back(vTrapezQ[i].pUppLeft);
        }

        if((vTrapezQ[i].pLowLeft.y <= vTrapezP[i].pLowLeft.y) &&
           (vTrapezP[i].pLowLeft.y <= vTrapezQ[i].pUppLeft.y) &&
           (!blnDegeneratedCase || AlmostEqual(vTrapezP[i].pUppLeft.x,
                vTrapezQ[i].pUppLeft.x)))
        {
          vPQIntersectionLowWD.push_back(vTrapezP[i].pLowLeft);
        }
        if((vTrapezP[i].pLowLeft.y <= vTrapezQ[i].pLowLeft.y) &&
           (vTrapezQ[i].pLowLeft.y <= vTrapezP[i].pUppLeft.y) &&
           (!blnDegeneratedCase || AlmostEqual(vTrapezP[i].pUppLeft.x,
              vTrapezQ[i].pUppLeft.x)))
        {
          vPQIntersectionLowWD.push_back(vTrapezQ[i].pLowLeft);
        }

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
            nrOfPointsAdded++;
          }
          if(blnS2Exist && vTrapezP[i].pUppLeft.x < pS2.x &&
             pS2.x < vTrapezP[i].pUppRight.x)
          {
            vPQIntersectionUppWD.push_back(pS2);
            nrOfPointsAdded++;
          }
          if(blnS3Exist && vTrapezP[i].pUppLeft.x < pS3.x &&
             pS3.x < vTrapezP[i].pUppRight.x)
          {
            vPQIntersectionUppWD.push_back(pS3);
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
            nrOfPointsAdded++;
          }
          if(blnS3Exist && vTrapezP[i].pUppLeft.x < pS3.x &&
             pS3.x < vTrapezP[i].pUppRight.x)
          {
            vPQIntersectionLowWD.push_back(pS3);
            nrOfPointsAdded++;
          }
          if(blnS4Exist && vTrapezP[i].pUppLeft.x < pS4.x &&
             pS4.x < vTrapezP[i].pUppRight.x)
          {
            vPQIntersectionLowWD.push_back(pS4);
            nrOfPointsAdded++;
          }
          sort(vPQIntersectionLowWD.end()-nrOfPointsAdded,
               vPQIntersectionLowWD.end());
        }
        // Right Border Check:
        if((vTrapezQ[i].pLowRight.y <= vTrapezP[i].pUppRight.y) &&
           (vTrapezP[i].pUppRight.y <= vTrapezQ[i].pUppRight.y) &&
           (!blnDegeneratedCase || AlmostEqual(vTrapezP[i].pUppRight.x,
            vTrapezQ[i].pUppRight.x)))
        {
          vPQIntersectionUppWD.push_back(vTrapezP[i].pUppRight);
        }
        if((vTrapezP[i].pLowRight.y <= vTrapezQ[i].pUppRight.y) &&
           (vTrapezQ[i].pUppRight.y <= vTrapezP[i].pUppRight.y) &&
           (!blnDegeneratedCase || AlmostEqual(vTrapezP[i].pUppRight.x,
            vTrapezQ[i].pUppRight.x)))
        {
          vPQIntersectionUppWD.push_back(vTrapezQ[i].pUppRight);
        }

        if((vTrapezQ[i].pLowRight.y <= vTrapezP[i].pLowRight.y) &&
           (vTrapezP[i].pLowRight.y <= vTrapezQ[i].pUppRight.y) &&
           (!blnDegeneratedCase || AlmostEqual(vTrapezP[i].pUppRight.x,
            vTrapezQ[i].pUppRight.x)))
        {
          vPQIntersectionLowWD.push_back(vTrapezP[i].pLowRight);
        }
        if((vTrapezP[i].pLowRight.y <= vTrapezQ[i].pLowRight.y) &&
           (vTrapezQ[i].pLowRight.y <= vTrapezP[i].pUppRight.y) &&
           (!blnDegeneratedCase || AlmostEqual(vTrapezP[i].pUppRight.x,
           vTrapezQ[i].pUppRight.x)))
        {
          vPQIntersectionLowWD.push_back(vTrapezQ[i].pLowRight);
        }
      }
    }
    vPQIntersectionWD.clear();
    for(i=0; i < vPQIntersectionUppWD.size(); i++)
    {
       vPQIntersectionWD.push_back(vPQIntersectionUppWD[i]);
    }
    for(i=vPQIntersectionLowWD.size()-1; i >= 0; i--)
    {
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
  return;
}

/*
5.12 ~HalfPlaneIntersection~-function

*/
void HalfPlaneIntersection(
        const vector<LinearConstraint>& vLinConstraints,
        vector<Point2D>& vConvexPolygon)
{
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
    blnNoXNoY = (AlmostEqual(linC.Get_a1(),0) &&
                 AlmostEqual(linC.Get_a2(),0));
    blnVerticalBoundary = (!AlmostEqual(linC.Get_a1(),0) &&
                            AlmostEqual(linC.Get_a2(),0));
    blnHorizontalBoundary = (AlmostEqual(linC.Get_a1(),0) &&
                            !AlmostEqual(linC.Get_a2(),0));
    blnLeftSideIn = (linC.Get_a1() > 0);
    blnUpperSideIn = (linC.Get_a2() < 0);
    if(blnNoXNoY)
    {
      // special case 0*x + 0*y + <value> <= 0;
      if(linC.Get_b() <= 0)
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
      else // (linC.Get_b() > 0)
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
        yCut = (-linC.Get_b()/linC.Get_a2());
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
          xCut = (-linC.Get_b()/linC.Get_a1());
        }
        else
        {
          xCut = (-linC.Get_b()-linC.Get_a2()*wyTop)/linC.Get_a1();
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
        xCut = (-linC.Get_b()/linC.Get_a1());
        if(AlmostEqual(wxRight, xCut) ||
          (xCut <= wxRight && !blnLeftSideIn) ||
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
          yCut = (-linC.Get_b()/linC.Get_a2());
        }
        else
        {
          yCut = (-linC.Get_b()-linC.Get_a1()*wxRight)/linC.Get_a2();
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
        yCut = (-linC.Get_b()/linC.Get_a2());
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
          xCut = (-linC.Get_b()/linC.Get_a1());
        }
        else
        {
          xCut = (-linC.Get_b()-linC.Get_a2()*wyBottom)/linC.Get_a1();
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
        xCut = (-linC.Get_b()/linC.Get_a1());
        if(AlmostEqual(wxLeft, xCut) ||
              (xCut <= wxLeft && !blnLeftSideIn) ||
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
          yCut = (-linC.Get_b()/linC.Get_a2());
        }
        else
        {
          yCut = (-linC.Get_b()-linC.Get_a1()*wxLeft)/linC.Get_a2();
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
  }
}

/*
6 Type Constructor ~constraint~ and its support functions

A value of type ~constraint~ represents a 2-dimensional (possibly infinite) pointset.

6.1 ~Out~-function

*/
ListExpr OutConstraint( ListExpr typeInfo, Word value )
{
  SymbolicRelation* symRel = (SymbolicRelation*)value.addr;
  const SymbolicTuple* pSymRelIP;
  const LinearConstraint* pLinConstraint;
  ListExpr result;
  ListExpr tempRes;
  ListExpr lastCon;
  ListExpr lastDis;

  if(symRel->SymbolicTuplesSize()==0)
  {
    result = nl->TheEmptyList();
  }
  else // symRel->SymbolicTuplesSize() > 0 (minimum: 1 Tuple)
  {
    for(unsigned int i = 0; i < symRel->SymbolicTuplesSize(); i++)
    {
      symRel->GetSymbolicTuples(i, pSymRelIP);
      for(int j = pSymRelIP->startIndex; j <= pSymRelIP->endIndex; j++)
      {
        symRel->GetLinConstraints(j, pLinConstraint);
        double a1 = pLinConstraint->Get_a1();
        double a2 = pLinConstraint->Get_a2();
        double b = pLinConstraint->Get_b();
        string Op = pLinConstraint->Get_Op();
        if(i==0)
        {
          if(j==pSymRelIP->startIndex)
          {
            // first conjunction, first disjunction
            tempRes = nl->OneElemList(nl->FourElemList(nl->RealAtom(a1),
              nl->RealAtom(a2), nl->RealAtom(b), nl->SymbolAtom(Op)));
            lastCon = tempRes;
          }
          if(j>pSymRelIP->startIndex)
          {
            // further conjunctions of the first disjunction
            lastCon = nl->Append(lastCon, nl->FourElemList(nl->RealAtom(a1),
              nl->RealAtom(a2), nl->RealAtom(b), nl->SymbolAtom(Op)));
          }
          if(j==pSymRelIP->endIndex)
          {
            // last conjunction of the first disjunction
            result = nl->OneElemList(tempRes);
            lastDis = result;
          }
        }
        else // => i>0
        {
          if(j==pSymRelIP->startIndex)
          {
            // first conjunction
            tempRes = nl->OneElemList(
              nl->FourElemList(nl->RealAtom(a1),
                nl->RealAtom(a2), nl->RealAtom(b), nl->SymbolAtom(Op)));
            lastCon = tempRes;
          }
          if(j>pSymRelIP->startIndex)
          {
            lastCon = nl->Append(lastCon, nl->FourElemList(
              nl->RealAtom(a1), nl->RealAtom(a2),
              nl->RealAtom(b), nl->SymbolAtom(Op)));
          }
          if(j==pSymRelIP->endIndex)
          {
            // last conjunction
            lastDis = nl->Append(lastDis, tempRes);
          }
        }
      }
    }
  }
  return result;
}

/*
6.2 ~In~-function

*/
Word InConstraint( const ListExpr typeInfo, const ListExpr instance,
                   const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = true;
  ListExpr symbolicTuplesNL = instance;
  ListExpr linConstraintsNL;
  ListExpr oneLinConstraintNL;

  if(!nl->IsAtom(instance))
  {
    SymbolicRelation* symbolicRelation = new SymbolicRelation(0, 0);
    while(!nl->IsEmpty(symbolicTuplesNL))
    {
      linConstraintsNL = nl->First(symbolicTuplesNL);
      symbolicTuplesNL = nl->Rest(symbolicTuplesNL);
      if(!nl->IsAtom(linConstraintsNL))
      {
        vector<LinearConstraint> vLinConstraints;
        while(!nl->IsEmpty(linConstraintsNL))
        {
          oneLinConstraintNL = nl->First(linConstraintsNL);
          linConstraintsNL = nl->Rest(linConstraintsNL);
          if(nl->ListLength(oneLinConstraintNL) == 4  &&
              nl->AtomType(nl->Nth(1, oneLinConstraintNL)) == RealType &&
              nl->AtomType(nl->Nth(2, oneLinConstraintNL)) == RealType &&
              nl->AtomType(nl->Nth(3, oneLinConstraintNL)) == RealType &&
              nl->AtomType(nl->Nth(4, oneLinConstraintNL)) == SymbolType)
          {
            LinearConstraint linConstraint(
                nl->RealValue(nl->Nth(1, oneLinConstraintNL)),
                nl->RealValue(nl->Nth(2, oneLinConstraintNL)),
                nl->RealValue(nl->Nth(3, oneLinConstraintNL)),
                nl->SymbolValue(nl->Nth(4, oneLinConstraintNL)));
            vLinConstraints.push_back(linConstraint);
          }
          else
          {
            ErrorReporter::ReportError("There is a non-valid lin."
              " constraint!. ");
            correct = false;
            return SetWord(Address(0));
          }
        }
        if(vLinConstraints.size()>0)
        {
          symbolicRelation->AddSymbolicTuple(vLinConstraints);
        }
        else
        {
            ErrorReporter::ReportError("each tuple must have at "
              "minimum one constraint!");
          correct = false;
          return SetWord(Address(0));
        }
      }
      else
      {
           ErrorReporter::ReportError("linConstraintsNL is atomic  "
              "(shoud be a list!).");
        correct = false;
        return SetWord(Address(0));
      }
    } // while
    // important: each symbolic relation as imput will be
    // directly normalized before saving it:
    symbolicRelation->Normalize();
    return SetWord(symbolicRelation);
  }
  else
  {
    ErrorReporter::ReportError("instance is atomic (shoud be a list!).");
    correct = false;
    return SetWord(Address(0));
  }
}

/*
6.3 Function describing the signature of the type constructor

*/
ListExpr
Constraint2Property()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("constraint"),
  nl->TextAtom("(<tuple>*) where tuple is"
  " (<lin_constraint_1>...<lin_constraint_n> with n>0 "
  "where lin_constraint_i is)"
  " (<a1> <a2> <b> <OP>) where a1, a2, b are"
  "real-values an OP is a symbol in the set {eq, leq}"),
  nl->TextAtom("(((0.0 1.0 2.1 leq)(1.0 1.0 -4.0 eq))"
  "((0.0 1.0 5.0 eq)(1.8 0.0 3.0 leq)))"),
  nl->TextAtom("each linear constraint represents the "
  "(in)equation: <a1>*x + <a2>*y +<b> <OP> 0")
  )));
}

/*
6.4 ~Create~-function

*/

Word CreateConstraint( const ListExpr typeInfo )
{
  SymbolicRelation* symbolicRelation = new SymbolicRelation(0, 0);
  vector<LinearConstraint> vLinConstraints;
  LinearConstraint linConstraint(0.0, 0.0, 0.0, OP_EQ);
  vLinConstraints.push_back(linConstraint);
  symbolicRelation->AddSymbolicTuple(vLinConstraints);
  symbolicRelation->Normalize(); // in order to compute the mmbox
  return (SetWord(symbolicRelation));
}

/*
6.5 ~Delete~-function

*/
void DeleteConstraint( const ListExpr typeInfo, Word& w )
{
  SymbolicRelation *sr = (SymbolicRelation *)w.addr;
  sr->Destroy();
  delete sr;
  w.addr = 0;
}

/*
6.6 ~Close~-function

*/
void CloseConstraint( const ListExpr typeInfo, Word& w )
{
  delete (SymbolicRelation *)w.addr;
  w.addr = 0;
}

/*
6.7 ~Clone~-function

*/
Word CloneConstraint( const ListExpr typeInfo, const Word& w )
{
  return SetWord(((SymbolicRelation *)w.addr)->Clone());
}


/*
6.8 ~SizeOf~-function

*/
int SizeOfConstraint()
{
  return sizeof(SymbolicRelation);
}


/*
6.9 ~CastConstraint~-function

*/
void* CastConstraint(void* addr)
{
  return new (addr) SymbolicRelation;
}

/*
6.10 Kind Checking function

This function checks whether the type constructor is applied correctly. Since
type constructor ~constraint~ does not have arguments, this is trivial.

*/
bool
CheckConstraint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "constraint" ));
}

/*
6.11 Creation of the type constructor instance

*/
TypeConstructor constraint(
        "constraint", //name
        Constraint2Property, //property function describing signature
        OutConstraint,     InConstraint, //Out and In functions
        0,                   0, //SaveToList and RestoreFromList functions
        CreateConstraint,  DeleteConstraint, //object creation and deletion
        0,               0, //open and save functions
        CloseConstraint,   CloneConstraint, //object close, and clone
        CastConstraint, //cast function
        SizeOfConstraint, //sizeof function
        CheckConstraint ); //kind checking function



/*
7 Operators: implementation of type mapping functions

A type mapping function takes a nested list as argument. Its contents are type descriptions of an operator's input parameters. A nested list describing the output type of the operator is returned.

7.1 Type mapping function ~CxC2CTypeMap~

*/
ListExpr CxC2CTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if(nl->IsEqual(arg1, "constraint") &&
      nl->IsEqual(arg2, "constraint"))
    {
      return (nl->SymbolAtom("constraint"));
    }
    else
    {
      ErrorReporter::ReportError("Type mapping function got wrong "
      " types as parameters.");
    }
  }
  else
  {
    ErrorReporter::ReportError("Type mapping function got a parameter"
      " of length != 2.");
  }
  return nl->SymbolAtom( "typeerror" );
}



/*
7.2 Type mapping function ~C2CTypeMap~

*/
ListExpr C2CTypeMap( ListExpr args )
{
  ListExpr arg;
  if ( nl->ListLength( args ) == 1 )
  {
    arg = nl->First( args );
    if(nl->IsEqual(arg, "constraint"))
    {
      return (nl->SymbolAtom("constraint"));
    }
    else
    {
      ErrorReporter::ReportError("Type mapping function got wrong "
        " type as parameter.");
    }
  }
  else
  {
    ErrorReporter::ReportError("Type mapping function got a parameter"
      " of length != 1.");
  }
  return nl->SymbolAtom( "typeerror" );
}


/*
7.3 Type mapping function ~projectionTypeMap~

*/
ListExpr projectionTypeMap( ListExpr args )
{
  ListExpr arg1, arg2, retNL;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if(nl->IsEqual(arg1, "constraint") &&
      (nl->IsEqual(arg2, "x") ||
       nl->IsEqual(arg2, "y")))
    {
      retNL = nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->OneElemList(nl->StringAtom(nl->SymbolValue(arg2))),
        nl->SymbolAtom("constraint"));
        return retNL;
    }
    else
    {
      ErrorReporter::ReportError("Type mapping function got wrong "
        " types as parameters. Use x or y as second parameter!");
    }
  }
  else
  {
    ErrorReporter::ReportError("Type mapping function got a parameter"
      " of length != 2.");
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
7.4 Type mapping function ~selectionTypeMap~

*/
ListExpr selectionTypeMap( ListExpr args )
{
  ListExpr argConstraintRel, arg_a1, arg_a2, arg_b, arg_op, retNL;
  if ( nl->ListLength( args ) == 5 )
  {
    argConstraintRel = nl->First( args );
    arg_a1 = nl->Second( args );
    arg_a2 = nl->Third( args );
    arg_b = nl->Fourth( args );
    arg_op = nl->Fifth( args );
    if(nl->IsEqual(argConstraintRel, "constraint") &&
      nl->IsEqual(arg_a1, "real") &&
      nl->IsEqual(arg_a2, "real") &&
      nl->IsEqual(arg_b, "real") &&
      (nl->IsEqual(arg_op, OP_EQ) ||
       nl->IsEqual(arg_op, OP_LEQ)))
    {
      retNL = nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->FourElemList(
          arg_a1,
          arg_a2,
          arg_b,
          nl->StringAtom(nl->SymbolValue(arg_op))),
          nl->SymbolAtom("constraint"));
      return retNL;
    }
    else
    {
      ErrorReporter::ReportError("Type mapping function got wrong "
        " types as parameters.");
    }
  }
  else
  {
    ErrorReporter::ReportError("Type mapping function got a parameter"
      " of length != 5.");
  }
  return nl->SymbolAtom( "typeerror" );
}


/*
7.5 Type mapping function ~C2BoolTypeMap~

*/
ListExpr C2BoolTypeMap( ListExpr args )
{
  ListExpr arg;
  if ( nl->ListLength( args ) == 1 )
  {
    arg = nl->First( args );
    if(nl->IsEqual(arg, "constraint"))
    {
      return (nl->SymbolAtom("bool"));
    }
    else
    {
      ErrorReporter::ReportError("Type mapping function got wrong "
        " type as parameter.");
    }
  }
  else
  {
    ErrorReporter::ReportError("Type mapping function got a parameter"
      " of length != 1.");
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
7.6 Type mapping function ~CxC2BoolTypeMap~

*/
ListExpr CxC2BoolTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if(nl->IsEqual(arg1, "constraint") &&
      nl->IsEqual(arg2, "constraint"))
    {
      return (nl->SymbolAtom("bool"));
    }
    else
    {
      ErrorReporter::ReportError("Type mapping function got wrong "
      " types as parameters.");
    }
  }
  else
  {
    ErrorReporter::ReportError("Type mapping function got a parameter"
      " of length != 2.");
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
7.7 Type mapping function ~C2IntTypeMap~

*/
ListExpr C2IntTypeMap( ListExpr args )
{
    ListExpr arg;
    if ( nl->ListLength( args ) == 1 )
    {
      arg = nl->First( args );

      if(nl->IsEqual(arg, "constraint"))
      {
        return (nl->SymbolAtom("int"));
      }
    else
    {
    ErrorReporter::ReportError("Type mapping function got wrong "
      " type as parameter.");
    }
    }
  else
  {
    ErrorReporter::ReportError("Type mapping function got a parameter"
      " of length != 1.");
  }
    return nl->SymbolAtom( "typeerror" );
}


/*
7.8 Type mapping function ~Point2CTypeMap~

*/
ListExpr Point2CTypeMap( ListExpr args )
{
  ListExpr arg;
  if ( nl->ListLength( args ) == 1 )
  {
    arg = nl->First( args );

    if(nl->IsEqual(arg, "point"))
    {
      return (nl->SymbolAtom("constraint"));
    }
    else
    {
      ErrorReporter::ReportError("Type mapping function got wrong "
        " type as parameter.");
    }
  }
  else
  {
    ErrorReporter::ReportError("Type mapping function got a parameter"
      " of length != 1.");
  }
  return nl->SymbolAtom( "typeerror" );
}


/*
7.9 Type mapping function ~Points2CTypeMap~

*/
ListExpr Points2CTypeMap( ListExpr args )
{
  ListExpr arg;
  if ( nl->ListLength( args ) == 1 )
  {
    arg = nl->First( args );

    if(nl->IsEqual(arg, "points"))
    {
      return (nl->SymbolAtom("constraint"));
    }
    else
    {
      ErrorReporter::ReportError("Type mapping function got wrong "
        " type as parameter.");
    }
  }
  else
  {
    ErrorReporter::ReportError("Type mapping function got a parameter"
      " of length != 1.");
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
7.10 Type mapping function ~Line2CTypeMap~

*/
ListExpr Line2CTypeMap( ListExpr args )
{
  ListExpr arg;
  if ( nl->ListLength( args ) == 1 )
  {
    arg = nl->First( args );
    if(nl->IsEqual(arg, "line"))
    {
      return (nl->SymbolAtom("constraint"));
    }
    else
    {
    ErrorReporter::ReportError("Type mapping function got wrong "
      " type as parameter.");
    }
  }
  else
  {
    ErrorReporter::ReportError("Type mapping function got a parameter"
      " of length != 1.");
  }
  return nl->SymbolAtom( "typeerror" );
}


/*
7.11 Type mapping function ~Region2CTypeMap~

*/
ListExpr RegionxBool2CTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if(nl->IsEqual(arg1, "region") &&
       nl->IsEqual(arg2, "bool"))
    {
    return (nl->SymbolAtom("constraint"));
    }
    else
    {
      ErrorReporter::ReportError("Type mapping function got wrong "
        " types as parameters. Use TRUE or FALSE as second parameter!");
    }
  }
  else
  {
    ErrorReporter::ReportError("Type mapping function got a parameter"
      " of length != 2.");
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
7.12 Type mapping function ~C2PointTypeMap~

*/
ListExpr C2PointTypeMap( ListExpr args )
{
    ListExpr arg;
    if ( nl->ListLength( args ) == 1 )
    {
      arg = nl->First( args );

      if(nl->IsEqual(arg, "constraint"))
      {
        return (nl->SymbolAtom("point"));
      }
    else
    {
    ErrorReporter::ReportError("Type mapping function got wrong "
      " type as parameter.");
    }
    }
  else
  {
    ErrorReporter::ReportError("Type mapping function got a parameter"
      " of length != 1.");
  }
    return nl->SymbolAtom( "typeerror" );
}

/*
7.13 Type mapping function ~C2PointsTypeMap~

*/
ListExpr C2PointsTypeMap( ListExpr args )
{
    ListExpr arg;
    if ( nl->ListLength( args ) == 1 )
    {
      arg = nl->First( args );

      if(nl->IsEqual(arg, "constraint"))
      {
        return (nl->SymbolAtom("points"));
      }
    else
    {
    ErrorReporter::ReportError("Type mapping function got wrong "
      " type as parameter.");
    }
    }
  else
  {
    ErrorReporter::ReportError("Type mapping function got a parameter"
      " of length != 1.");
  }
    return nl->SymbolAtom( "typeerror" );
}

/*
7.14 Type mapping function ~C2LineTypeMap~

*/
ListExpr C2LineTypeMap( ListExpr args )
{
    ListExpr arg;
    if ( nl->ListLength( args ) == 1 )
    {
      arg = nl->First( args );

      if(nl->IsEqual(arg, "constraint"))
      {
        return (nl->SymbolAtom("line"));
      }
    else
    {
    ErrorReporter::ReportError("Type mapping function got wrong "
      " type as parameter.");
    }
    }
  else
  {
    ErrorReporter::ReportError("Type mapping function got a parameter"
      " of length != 1.");
  }
    return nl->SymbolAtom( "typeerror" );
}

/*
7.15 Type mapping function ~CStream2RegionStreamTypeMap~

*/
ListExpr CStream2RegionStreamTypeMap( ListExpr args )
{
  ListExpr arg11, arg12;
  string out;

  if ( nl->ListLength(args) == 1 )
  {
    if( nl->ListLength(nl->First(args)) == 2 )
  {
      arg11 = nl->First(nl->First(args));
      arg12 = nl->Second(nl->First(args));
      if ( nl->IsEqual(arg11, "stream") &&
          nl->IsEqual(arg12, "constraint") )
      {
        return nl->TwoElemList(nl->SymbolAtom("stream"),
                    nl->SymbolAtom("region"));
      }
      else
      {
      nl->WriteToString(out, nl->First(args));
      ErrorReporter::ReportError("Operator constraint2stream "
      " expects a (stream constraint) as its first argument. "
      "The argument provided "
      "has type '" + out + "' instead.");
      }
   }
   else
   {
    ErrorReporter::ReportError("Type mapping function got a parameter"
    " of wrong length.");
   }
  }
  else
  {
    ErrorReporter::ReportError("Type mapping function got a parameter"
    " of wrong length.");
  }
  return nl->SymbolAtom("typeerror");
}


/*
7.16 Type mapping function ~C2RectTypeMap~

*/
ListExpr C2RectTypeMap( ListExpr args )
{
    ListExpr arg;
    if ( nl->ListLength( args ) == 1 )
    {
      arg = nl->First( args );

      if(nl->IsEqual(arg, "constraint"))
      {
        return (nl->SymbolAtom("rect"));
      }
    else
    {
    ErrorReporter::ReportError("Type mapping function got wrong "
      " type as parameter.");
    }
    }
  else
  {
    ErrorReporter::ReportError("Type mapping function got a parameter"
      " of length != 1.");
  }
    return nl->SymbolAtom( "typeerror" );
}

/*
7.17 Type mapping function ~RegionStream2RegionStreamTypeMap~

*/
ListExpr RegionStream2RegionStreamTypeMap( ListExpr args )
{
  ListExpr arg11, arg12;
  string out;

  if ( nl->ListLength(args) == 1 )
  {
    if( nl->ListLength(nl->First(args)) == 2 )
  {
      arg11 = nl->First(nl->First(args));
      arg12 = nl->Second(nl->First(args));
      if ( nl->IsEqual(arg11, "stream") && nl->IsEqual(arg12, "region") )
      {
        return nl->First(args);
      }
      else
      {
      nl->WriteToString(out, nl->First(args));
        ErrorReporter::ReportError("Operator triangulate expects a "
           "(stream region) as its first argument. "
           "The argument provided "
           "has type '" + out + "' instead.");
      }
   }
   else
   {
    ErrorReporter::ReportError("Type mapping function got a parameter"
    " of wrong length.");
   }
  }
  else
  {
    ErrorReporter::ReportError("Type mapping function got a parameter"
    " of wrong length.");
  }
  return nl->SymbolAtom("typeerror");
}


/*
8 Operators: implementation of value mapping functions

8.1 Value mapping functions of operator ~cunion~

*/
int unionValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  SymbolicRelation* symRelFirst = ((SymbolicRelation*)args[0].addr);
  SymbolicRelation* symRelSecond = ((SymbolicRelation*)args[1].addr);
  SymbolicRelation* symRelResult;

  if(symRelFirst->SymbolicTuplesSize() <
        symRelSecond->SymbolicTuplesSize())
  {
    SymbolicRelation* symRelTemp = symRelFirst;
    symRelFirst = symRelSecond;
    symRelSecond = symRelTemp;
  }

  symRelResult = symRelFirst->Clone();
  symRelResult->AppendSymbolicRelation(*symRelSecond);
  *((SymbolicRelation *)result.addr) = *symRelResult;
  return (0);
}

/*
8.2 Value mapping functions of operator ~cintersection~ alias ~cjoin~

*/
int intersectionValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  SymbolicRelation* symRelFirst = ((SymbolicRelation*)args[0].addr);
  SymbolicRelation* symRelSecond = ((SymbolicRelation*)args[1].addr);
  SymbolicRelation* symRelResult;


  symRelResult = symRelFirst->Clone();
  symRelResult->JoinSymbolicRelation(*symRelSecond);
  symRelResult->Normalize();
  *((SymbolicRelation *)result.addr) = *symRelResult;
  return (0);
}

/*
8.3 Value mapping functions of operator ~cprojection~

*/
int projectionValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  SymbolicRelation* symRel = (SymbolicRelation*)args[0].addr;
  const string strAxis = ((CcString*)args[2].addr)->GetValue();
  // es gilt: strAxis=="x" || strAxis=="y"
  bool blnXAxis = false;
  bool blnYAxis = false;
  if(strAxis=="x")
  {
    blnXAxis = true;
  }
  else // strAxis=="y")
  {
    blnYAxis = true;
  }
  SymbolicRelation* symRelResult;
  symRelResult = symRel->Clone();
  symRelResult->ProjectToAxis(blnXAxis, blnYAxis);
  symRelResult->Normalize();
  *((SymbolicRelation *)result.addr) = *symRelResult;
  return (0);
}

/*
8.4 Value mapping functions of operator ~cselection~

*/
int selectionValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  SymbolicRelation* symRel = (SymbolicRelation*)args[0].addr;
  // TODO: Abklaeren, ob indexe so bleiben??
  const double a1 = (double)((CcReal*)args[1].addr)->GetValue();
  const double a2 = (double)((CcReal*)args[2].addr)->GetValue();
  const double b =  (double)((CcReal*)args[3].addr)->GetValue();
  const string strOp = ((CcString*)args[8].addr)->GetValue();
  // es gilt: strOp==OP_EQ || strOp==OP_LEQ
  // in order to reuse the overlap function, build a symbolic relation
  // with one tuple and one lin. constraint (built out of the parameters)
  SymbolicRelation* symRelResult;
  vector<LinearConstraint> vLinearConstraints;
  LinearConstraint linC(a1, a2, b, strOp);
  vLinearConstraints.push_back(linC);
  SymbolicRelation symRelSecond(0, 0);
  symRelSecond.AddSymbolicTuple(vLinearConstraints);
  symRelSecond.Normalize(); // for mbbox-computation
  // apply the overlap function:
  symRelResult = symRel->Clone();
  symRelResult->JoinSymbolicRelation(symRelSecond);
  symRelResult->Normalize();
  *((SymbolicRelation *)result.addr) = *symRelResult;
  return (0);
}

/*
8.5 Value mapping functions of operator ~csatisfy~

*/
int satisfyValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  SymbolicRelation* symRel = (SymbolicRelation*)args[0].addr;
  ((CcBool *)result.addr)->Set(true, symRel->BoundingBox().IsDefined());
  return (0);
}

/*
8.6 Value mapping functions of operator ~coverlaps~

*/
int overlapsValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  SymbolicRelation* symRelFirst = ((SymbolicRelation*)args[0].addr);
  SymbolicRelation* symRelSecond = ((SymbolicRelation*)args[1].addr);
  ((CcBool *)result.addr)->Set(true,
    symRelFirst->OverlapsSymbolicRelation(*symRelSecond));
  return (0);
}

/*
8.7 Value mapping functions of operator ~no\_tuples~

*/
int no_tuplesValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcInt *)result.addr)->Set( true,
    ((SymbolicRelation*)args[0].addr)->SymbolicTuplesSize() );
  return 0;
}

/*
8.8 Value mapping functions of operator ~no\_constraints~

*/
int no_constraintsValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcInt *)result.addr)->Set( true,
    ((SymbolicRelation*)args[0].addr)->LinConstraintsSize() );
  return 0;
}

/*
8.9 Value mapping functions of operator ~bbox~

*/
int bboxValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  *((Rectangle<2>*)result.addr) =
     ((SymbolicRelation*)args[0].addr)->BoundingBox();
  return 0;
}


/*
8.10 Value mapping functions of operator ~triangulate~

*/
struct TriangulateLocalInfo
{
  vector<Region*> triangles;
  vector<Region*>::iterator iter;
};

int triangulateValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  TriangulateLocalInfo *localInfo;
  if(message==OPEN)
  {
    localInfo = new TriangulateLocalInfo();
    local = SetWord( localInfo );
    qp->Open(args[0].addr);
    return 0;
  }
  else if(message==REQUEST)
  {
    Word elem;
    Region* reg;
    localInfo = (TriangulateLocalInfo*)local.addr;
    if(localInfo->triangles.size()==0 ||
      localInfo->iter == localInfo->triangles.end())
    {
      if(localInfo->iter == localInfo->triangles.end())
      {
        localInfo->triangles.clear();
      }
      bool blnInputRegionIsEmpty = true;
      while(blnInputRegionIsEmpty)
      {
        qp->Request(args[0].addr, elem);
        if(qp->Received(args[0].addr))
        {
          reg = (Region*)elem.addr;
          blnInputRegionIsEmpty = reg->IsEmpty()
            || reg->Size()/2 >= SEGSIZE_TRIANGULATION;
          if(reg->Size()/2 >= SEGSIZE_TRIANGULATION)
          {
            ErrorReporter::ReportError("INVALID INPUT: "
              "too many segments! "
              "Increase SEGSIZE in TRIANGULATION-Lib "
              "in order to process.");
          }
        }
        else
        {
          return CANCEL;
        }
      }
      vector<vector<double> > vVertices;
      vector<vector<int> > vTriangles;
      TriangulateRegion(reg, vVertices, vTriangles);
      for (int j = 0; j < vTriangles.size(); j++)
      {
        ListExpr resultNL =
      nl->ThreeElemList(
            nl->TwoElemList(
              nl->RealAtom((Coord)vVertices[vTriangles[j][0]][X]),
              nl->RealAtom((Coord)vVertices[vTriangles[j][0]][Y])),
            nl->TwoElemList(
              nl->RealAtom((Coord)vVertices[vTriangles[j][1]][X]),
              nl->RealAtom((Coord)vVertices[vTriangles[j][1]][Y])),
            nl->TwoElemList(
              nl->RealAtom((Coord)vVertices[vTriangles[j][2]][X]),
              nl->RealAtom((Coord)vVertices[vTriangles[j][2]][Y])));
        ListExpr regionNL = nl->OneElemList(nl->OneElemList(resultNL));
        ListExpr errorInfoInRegion;
        bool correctInRegion;
        Region* r = (Region*)InRegion(nl->TheEmptyList(),
          regionNL, 0, errorInfoInRegion, correctInRegion ).addr;
    r->EndBulkLoad( false, false, false, true);
        localInfo->triangles.push_back( r );
      }
      // und Regions in localInfo->triangles speichern
      localInfo->iter = localInfo->triangles.begin();
    }
    result = SetWord( *localInfo->iter++ );
    return YIELD;
  }
  if(message==CLOSE)
  {
    qp->Close(args[0].addr);
    localInfo = (TriangulateLocalInfo*)local.addr;
    delete localInfo;
    return 0;
  }
  return 0;
}


/*
8.11 Value mapping functions of import operator ~point2constraint~

*/
int point2constraintValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  SymbolicRelation* symRelResult = new SymbolicRelation(0, 0);
  Point* point2convert = (Point*)args[0].addr;
  if( point2convert->IsDefined() )
  {
    if(point2convert->Inside(WORLD))
    {
      // if point is defined and inside the WORLD: convert it
      Point2D p2Convert = Point2D(
        (double)point2convert->GetX(),
        (double)point2convert->GetY());
      vector<Point2D> vPoints;
      vPoints.push_back(p2Convert);
      vector<LinearConstraint> vLinConstraints;
      ToConstraint(vPoints, vLinConstraints);
      symRelResult->AddSymbolicTuple(vLinConstraints);
    }
    else
    {
      // the point is outside the WORLD: not possible to convert!
    ErrorReporter::ReportError("WARNING: The point is outside"
      " the WORLD: not possible to convert.");
    }
  }
  symRelResult->Normalize();
  *((SymbolicRelation *)result.addr) = *symRelResult;
  return (0);
}

/*
8.12 Value mapping functions of import operator ~points2constraint~

*/
int points2constraintValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  SymbolicRelation* symRelResult = new SymbolicRelation(0, 0);
  Points* points2convert = (Points*)args[0].addr;
  if( points2convert->IsValid() )
  {
    //if it contains only defined points and no duplicates.
    bool blnExistPointsOutWORLD = false;
    for(int i=0; i < points2convert->Size(); i++)
    {
      const Point *point2convert;
      points2convert->Get(i, point2convert);
      if(point2convert->Inside(WORLD))
      {
        // if point is defined and inside the WORLD: convert it
        Point2D p2Convert = Point2D(
          (double)point2convert->GetX(),
          (double)point2convert->GetY());
        vector<Point2D> vPoints;
        vPoints.push_back(p2Convert);
        vector<LinearConstraint> vLinConstraints;
        ToConstraint(vPoints, vLinConstraints);
        symRelResult->AddSymbolicTuple(vLinConstraints);
      }
      else
      {
        blnExistPointsOutWORLD = true;
      }
    }

    if(blnExistPointsOutWORLD)
    {
    ErrorReporter::ReportError("WARNING: The set of points does have "
      " points which are outsidethe WORLD: not possible to"
      " convert them.");
    }
  }
  else
  {
    ErrorReporter::ReportError("WARNING: The set of points is not "
      " valid: not possible to convert.");  }
  symRelResult->Normalize();
  *((SymbolicRelation *)result.addr) = *symRelResult;
  return (0);
}


/*
8.13 Value mapping functions of import operator ~line2constraint~

*/
int line2constraintValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  int i;
  result = qp->ResultStorage( s );
  SymbolicRelation* symRelResult = new SymbolicRelation(0, 0);
  Line* line2convert = (Line*)args[0].addr;
  if(!line2convert->IsEmpty())
  {
    // if it is not empty:
    Line* clippedLine = new Line(0);
    bool blnLineTotalInWORLD = WORLD.Contains(line2convert->BoundingBox());
    if(!blnLineTotalInWORLD)
    {
      // then the line has to be clipped with the WORLD:
      bool inside;
      line2convert->WindowClippingIn(WORLD, *clippedLine, inside);
      line2convert = clippedLine;
    }

    // now the (if it is neccessary: clipped) line will be converted:
    for(i=0; i < line2convert->Size(); i++)
    {
      const HalfSegment *hs;
      line2convert->Get(i, hs);
      if(hs->IsLeftDomPoint())
      {
        Point lp = hs->GetLeftPoint();
        Point rp = hs->GetRightPoint();
        Point2D lp2Convert = Point2D(
          (double)lp.GetX(),
          (double)lp.GetY());
        Point2D rp2Convert = Point2D(
          (double)rp.GetX(),
          (double)rp.GetY());
        vector<Point2D> vPoints;
        vPoints.push_back(lp2Convert);
        vPoints.push_back(rp2Convert);
        vector<LinearConstraint> vLinConstraints;
        ToConstraint(vPoints, vLinConstraints);
        symRelResult->AddSymbolicTuple(vLinConstraints);
      }
    }
    delete clippedLine;

    if(!blnLineTotalInWORLD)
    {
      // the line is partially outside the WORLD: not possible to convert!
    ErrorReporter::ReportError("WARNING: The line does contain "
      "segments which are (partially) outside the WORLD: "
      " clipped with the WORLD during convert process.");
    }
  }
  symRelResult->Normalize();
  *((SymbolicRelation *)result.addr) = *symRelResult;
  return (0);
}

/*
8.14 Value mapping functions of import operator ~region2constraint~

*/
int region2constraintValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  SymbolicRelation* symRelResult = new SymbolicRelation(0, 0);
  Region* reg = (Region*)args[0].addr;

  bool blnMergeTriangles2ConvPoly = false;
  if (((CcBool*)args[1].addr)->IsDefined() &&
        ((CcBool*)args[1].addr)->GetBoolval() )
  {
    blnMergeTriangles2ConvPoly = true;
  }

  if(!reg->IsEmpty() && reg->Size()/2 < SEGSIZE_TRIANGULATION)
  {
    // if it is not empty:
    Region *region2convert = new Region(*reg, false); // in memory
    bool blnRegionTotalInWORLD = WORLD.Contains(region2convert->BoundingBox());
    Region* clippedRegion = new Region(0);
    if(!blnRegionTotalInWORLD)
    {
      region2convert->WindowClippingIn(WORLD, *clippedRegion);
      delete region2convert;
      region2convert = clippedRegion;
    }
    else
    {
      delete clippedRegion;
    }
    vector<vector<double> > vVertices;
    vector<vector<int> > vTriangles;
    TriangulateRegion(reg, vVertices, vTriangles);
    vector<vector<Point2D> > vCWPoints2convert;
    if(blnMergeTriangles2ConvPoly)
    {
      MergeTriangles2ConvexPolygons(vVertices,
                                    vTriangles,
                                    vCWPoints2convert);
      for (int j = 0; j < vCWPoints2convert.size(); j++)
      {
        // each convex polygon will be converted individually:

        vector<LinearConstraint> vLinConstraints;
        ToConstraint(vCWPoints2convert[j], vLinConstraints);
        symRelResult->AddSymbolicTuple(vLinConstraints);
      }
    }
    else
    {
      // only triangles
      for (int j = 0; j < vTriangles.size(); j++)
      {
        // each triangle will be converted individually:
        vector<Point2D> v3Points2convert;
        Point2D p0(vVertices[vTriangles[j][0]][X],
                   vVertices[vTriangles[j][0]][Y]);
        Point2D p1(vVertices[vTriangles[j][1]][X],
                   vVertices[vTriangles[j][1]][Y]);
        Point2D p2(vVertices[vTriangles[j][2]][X],
                   vVertices[vTriangles[j][2]][Y]);

     double orientedArea =  GetOrientedArea(p0.x, p0.y,
                                            p1.x, p1.y,
                                            p2.x, p2.y);
    if(orientedArea<0)
    {
      // then p0,p1,p2 is cw-order
          v3Points2convert.push_back(p0);
          v3Points2convert.push_back(p1);
          v3Points2convert.push_back(p2);
    }
        else if(orientedArea>0)
    {
      // then p0,p1,p2 is ccw-order
      // => p0,p2,p1 is cw-order
          v3Points2convert.push_back(p0);
          v3Points2convert.push_back(p2);
          v3Points2convert.push_back(p1);
    }
    else // orientedArea==0
    {
        assert(false); // triangle on line not possible
    }
        vector<LinearConstraint> vLinConstraints;
        ToConstraint(v3Points2convert, vLinConstraints);
        symRelResult->AddSymbolicTuple(vLinConstraints);
      }
    }
    if(!blnRegionTotalInWORLD)
    {
      // the line is partially outside the WORLD: not possible to convert!
      ErrorReporter::ReportError("WARNING: The region does contain "
        "faces which are (partially) outside the WORLD: "
        " clipped with the WORLD during convert process.");
    }
  }
  else
  {
    if(reg->Size()/2 >= SEGSIZE_TRIANGULATION)
    {
       ErrorReporter::ReportError("INVALID INPUT: too many segments! "
      "Increase SEGSIZE in TRIANGULATION-Lib in order to process.");
    }
  }
  symRelResult->Normalize();
  *((SymbolicRelation *)result.addr) = *symRelResult;
  return (0);
}

/*
8.15 Value mapping functions of export operator ~constraint2point~

*/
int constraint2pointValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  int i;
  result = qp->ResultStorage( s );
  SymbolicRelation* symRel = (SymbolicRelation*)args[0].addr;
  Point* pointResult;
  // if symRel is emtpy then output: invalid input
  // else:
  //   if nrT = #(symbolic tuples which represents a single point) = 1 then:
  //    convert this tuple to a single point
  //  else (nrT=0 or nrT > 1):
  //    output: invalid input
  //

  bool blnInvalidInput = false;

  if(symRel->SymbolicTuplesSize()==0)
  {
    blnInvalidInput = true;
  }
  else
  {
    const LinearConstraint *linC1, *linC2;
    int noPoints = 0;
    for(i=0; i < symRel->SymbolicTuplesSize(); i++)
    {
      const SymbolicTuple *symTuple;
      symRel->GetSymbolicTuples(i, symTuple);
      if(symTuple->endIndex-symTuple->startIndex==1)
      {
      // symTuple contains 2 lin. Constraints
        symRel->GetLinConstraints(symTuple->startIndex, linC1);
        symRel->GetLinConstraints(symTuple->startIndex+1, linC2);
        if(linC1->Get_Op()==OP_EQ && linC2->Get_Op()==OP_EQ)
        {
          noPoints++;
        }
      }
    }
    if(noPoints == 1)
    {
      // convert:
      pointResult = new Point( true, (Coord)(-linC1->Get_b()),
        (Coord)(-linC2->Get_b()));
    }
    else
    {
       blnInvalidInput = true;
    }
  }

  if(!blnInvalidInput)
  {
    *((Point *)result.addr) = *pointResult;
    delete pointResult;
  }
  else
  {
    ErrorReporter::ReportError("INVALID INPUT: "
      "There is none or more than one point contained in "
      "the constraint.");
    ((Point *)result.addr)->SetDefined( false );
  }
  return (0);
}

/*
8.16 Value mapping functions of export operator ~constraint2points~

*/
int constraint2pointsValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  int i;
  result = qp->ResultStorage( s );
  SymbolicRelation* symRel = (SymbolicRelation*)args[0].addr;
  Points* pointsResult = new Points(0);
  const LinearConstraint *linC1, *linC2;

  pointsResult->StartBulkLoad();
  for(i=0; i < symRel->SymbolicTuplesSize(); i++)
  {
    const SymbolicTuple *symTuple;
    symRel->GetSymbolicTuples(i, symTuple);
    if(symTuple->endIndex-symTuple->startIndex==1)
    {
    // symTuple contains 2 lin. Constraints
      symRel->GetLinConstraints(symTuple->startIndex, linC1);
      symRel->GetLinConstraints(symTuple->startIndex+1, linC2);
      if(linC1->Get_Op()==OP_EQ && linC2->Get_Op()==OP_EQ)
      {
        Point* p = new Point( true, (Coord)(-linC1->Get_b()),
            (Coord)(-linC2->Get_b()));
        (*pointsResult) += (*p);
        delete p;
      }
    }
  }
  pointsResult->EndBulkLoad();
  *((Points *)result.addr) = *pointsResult;
  delete pointsResult;
  return (0);
}

/*
8.17 Value mapping functions of export operator ~constraint2line~

*/
int constraint2lineValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  int i;
  result = qp->ResultStorage( s );
  SymbolicRelation* symRel = (SymbolicRelation*)args[0].addr;
  Line* lineResult = new Line(0);
  const LinearConstraint *linC1, *linC2, *linC3;

  lineResult->StartBulkLoad();
  int edgeno = 0;
  for(i=0; i < symRel->SymbolicTuplesSize(); i++)
  {
    const SymbolicTuple *symTuple;
    symRel->GetSymbolicTuples(i, symTuple);
    if(symTuple->endIndex-symTuple->startIndex==2)
    {
    // symTuple contains 3 lin. Constraints
      symRel->GetLinConstraints(symTuple->startIndex, linC1);
      symRel->GetLinConstraints(symTuple->startIndex+1, linC2);
      symRel->GetLinConstraints(symTuple->startIndex+2, linC3);
      if(linC1->Get_Op()==OP_EQ &&
         linC2->Get_Op()==OP_LEQ &&
         linC3->Get_Op()==OP_LEQ)
      {
        Point2D pFrom2D = GetIntersectionPoint(*linC1, *linC2);
        Point2D pTo2D = GetIntersectionPoint(*linC1, *linC3);
        Point pFrom(true, (Coord)pFrom2D.x, (Coord)pFrom2D.y);
        Point pTo(true, (Coord)pTo2D.x, (Coord)pTo2D.y);
        HalfSegment hs(true, pFrom, pTo);
        hs.attr.edgeno = edgeno;
        *lineResult += hs;
        hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
        *lineResult += hs;
        edgeno++;
      }
    }
  }
  lineResult->EndBulkLoad();
  *((Line *)result.addr) = *lineResult;
  delete lineResult;
  return (0);
}

/*
8.18 Value mapping functions of export operator ~constraint2region~

*/
struct constraint2regionLocalInfo
{
  vector<Region*> convexPolygons;
  vector<Region*>::iterator iter;
};

int constraint2regionValueMap( Word* args, Word& result, int message,
           Word& local, Supplier s )
{
  constraint2regionLocalInfo *localInfo;

  if(message==OPEN)
  {
      localInfo = new constraint2regionLocalInfo();

      local = SetWord( localInfo );
      qp->Open(args[0].addr);
      return 0;
  }
  else if(message==REQUEST)
  {
    Word elem;
    SymbolicRelation* sr;
    localInfo = (constraint2regionLocalInfo*)local.addr;
    if(localInfo->convexPolygons.size()==0 ||
      localInfo->iter == localInfo->convexPolygons.end())
    {
      if(localInfo->iter == localInfo->convexPolygons.end())
      {
        localInfo->convexPolygons.clear();
      }
      // new request:
      bool blnInputConstraintStreamIsEmpty = true;
      while(blnInputConstraintStreamIsEmpty)
      {
        qp->Request(args[0].addr, elem);
        if(qp->Received(args[0].addr))
        {
          sr = (SymbolicRelation*)elem.addr;
          blnInputConstraintStreamIsEmpty =
             (sr->SymbolicTuplesSize()==0);
        }
        else
        {
          return CANCEL;
        }
      }
      for(unsigned int iSymTuple = 0;
          iSymTuple < sr->SymbolicTuplesSize(); iSymTuple++)
      {
        const SymbolicTuple* symTuple;
        sr->GetSymbolicTuples(iSymTuple, symTuple);
        if(symTuple->endIndex-symTuple->startIndex >= 2)
        {
          // if more than 2
          const LinearConstraint* linConstraint1;
          const LinearConstraint* linConstraint2;
          const LinearConstraint* linConstraint3;
          sr->GetLinConstraints(symTuple->startIndex,
                        linConstraint1);
          sr->GetLinConstraints(symTuple->startIndex+1,
                        linConstraint2);
          sr->GetLinConstraints(symTuple->startIndex+2,
                        linConstraint3);
          if(linConstraint1->Get_Op()==OP_LEQ &&
               linConstraint2->Get_Op()==OP_LEQ &&
               linConstraint3->Get_Op()==OP_LEQ)
          {
            ListExpr regionNL, resultNL, lastNL;
            Point2D startPoint2D, fromPoint2D, toPoint2D;

            for(int iLinC = symTuple->startIndex;
                iLinC <= symTuple->endIndex; iLinC++)
            {
              if(iLinC==symTuple->startIndex)
              {
                const LinearConstraint* firstLinConstraint;
                const LinearConstraint* secondLinConstraint;
                const LinearConstraint* lastLinConstraint;
                sr->GetLinConstraints(symTuple->startIndex,
                  firstLinConstraint);
                sr->GetLinConstraints(symTuple->startIndex+1,
                  secondLinConstraint);
                sr->GetLinConstraints(symTuple->endIndex,
                  lastLinConstraint);
                fromPoint2D = GetIntersectionPoint(
                  *lastLinConstraint,
                  *firstLinConstraint);
                startPoint2D = fromPoint2D;
                toPoint2D = GetIntersectionPoint(
                  *firstLinConstraint, *secondLinConstraint);
                resultNL = nl->OneElemList(
                  nl->TwoElemList(
                  nl->RealAtom(fromPoint2D.x),
                  nl->RealAtom(fromPoint2D.y)));
                lastNL = resultNL;
              }
              else //if(iLinC <= symTuple->endIndex)
              {
                const LinearConstraint* currLinConstraint;
                const LinearConstraint* nextLinConstraint;
                sr->GetLinConstraints(iLinC,
                currLinConstraint);
                sr->GetLinConstraints(iLinC+1,
                nextLinConstraint);
                toPoint2D = GetIntersectionPoint(
                *currLinConstraint,
                *nextLinConstraint);
                lastNL = nl->Append(lastNL,
                 nl->TwoElemList(
                  nl->RealAtom(fromPoint2D.x),
                  nl->RealAtom(fromPoint2D.y)));
              }
              fromPoint2D = toPoint2D;
            }
            regionNL = nl->OneElemList(nl->OneElemList(resultNL));
            ListExpr errorInfoInRegion;
            bool correctInRegion;
            Region* r = (Region*)InRegion(nl->TheEmptyList(),
              regionNL, 0, errorInfoInRegion,
                correctInRegion ).addr;
      // force ComputeRegion():
      r->EndBulkLoad( false, false, false, true);
            // then the current symbolic tuple is a convex
            // polygon (not a point, not a line)!
            localInfo->convexPolygons.push_back( r );
          }
        }
      }
      localInfo->iter = localInfo->convexPolygons.begin();
    }
    result = SetWord( *localInfo->iter++ );
    return YIELD;
  }
  else if(message==CLOSE)
  {
      qp->Close(args[0].addr);
      localInfo = (constraint2regionLocalInfo*)local.addr;
      delete localInfo;
      return 0;
  }
  return 0;
}


/*
9 Operators: specifications strings and final definition of operator

Definition of operators is done in a way similar to definition of type constructors: an instance of class ~Operator~ is defined.

9.1 Definition of specification strings

*/
const string ConstraintSpecUnion  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>constraint x constraint -> constraint</text--->"
  "<text>_ cunion _</text--->"
  "<text>union of two constraints.</text--->"
  "<text>query objConstraintPolygon1 cunion objConstraintPolygon2"
  "</text--->) )";

const string ConstraintSpecJoin  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>constraint x constraint -> constraint</text--->"
  "<text>_ cjoin _</text--->"
  "<text>join of two constraints (equal semantics like the "
  "coverlap-operator).</text--->"
  "<text>query objConstraintPolygon1 cjoin objConstraintPolygon2</text--->"
  ") )";

const string ConstraintSpecIntersection  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>constraint x constraint -> constraint</text--->"
  "<text>_ cintersection _</text--->"
  "<text>intersection of two constraints (equal semantics like the "
  "coverlap-operator).</text--->"
  "<text>query objConstraintPolygon1 cintersection objConstraintLine1"
  "</text--->) )";

const string ConstraintSpecProjection  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>constraint x axis -> constraint</text--->"
  "<text>cprojection( _ , _ )</text--->"
  "<text>projection of a constraint to an certain axis (x or y).</text--->"
  "<text>query cprojection(objConstraintPolygon1, x)</text--->"
  ") )";

const string ConstraintSpecSelection  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>constraint x real x real x real x {eq, leq} ->"
  " constraint</text--->"
  "<text>cselection( _ , _ , _ , _ , _ )</text--->"
  "<text>select (restrict) a constraint relation to a "
  "certain linear constraint.</text--->"
  "<text>query cselection(objConstraintPolygon1, 1.0, 0.0, -7.0, leq)"
  "</text--->) )";

const string ConstraintSpecSatisfy  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>constraint -> bool</text--->"
  "<text>csatisfy( _ )</text--->"
  "<text>checks if the represented pointset is empty or not.</text--->"
  "<text>query csatisfy(objConstraintPolygon1)</text--->"
  ") )";

const string ConstraintSpecOverlaps  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>constraint x constraint -> bool</text--->"
  "<text>_ coverlaps _</text--->"
  "<text>return TRUE if there exist points which are in "
  "both symbolic relations.</text--->"
  "<text>query objConstraintPolygon1 coverlaps "
  "objConstraintPolygon2</text--->"
  ") )";

const string ConstraintSpecNo_Tuples  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>constraint -> int</text--->"
  "<text>no_tuples( _ )</text--->"
  "<text>return the number of symbolic tuples of a constraint value "
  "(symbolic relation) </text--->"
  "<text>query no_tuples(objConstraintPolygon1)</text--->"
  ") )";

const string ConstraintSpecNo_Constraints  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>constraint -> int</text--->"
  "<text>no_constraints( _ )</text--->"
  "<text>return the total number of atomar linear constraints of a "
  "constraint value (symbolic relation) </text--->"
  "<text>query no_constraints(objConstraintPolygon1)</text--->"
  ") )";

const string ConstraintSpecBBox  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>constraint -> rect</text--->"
  "<text>bbox( _ )</text--->"
  "<text>return the bounding box of a symbolic relation"
  "region values (triangles)</text--->"
  "<text>query bbox(objConstraintPolygon1)</text--->"
  ") )";

const string ConstraintSpecTriangulate  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>stream(region) -> stream(region)</text--->"
  "<text>_ triangulate </text--->"
  "<text>triangulate a stream of region values into a stream of "
  "region values (triangles)</text--->"
  "<text>query Flaechen feed project[geoData] transformstream "
  "triangulate transformstream count</text--->"
  ") )";

const string ConstraintSpecPoint2Constraint  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>point -> constraint</text--->"
  "<text>point2constraint( _ )</text--->"
  "<text>converts a point value from the spatial-algebra "
  "to a constraint value.</text--->"
  "<text>query UBahnhof feed extend[p: point2constraint(.geoData "
  "scale[0.1])] filter[csatisfy(.p)=TRUE] project[p] count</text--->"
  ") )";

const string ConstraintSpecPoints2Constraint  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>points -> constraint</text--->"
  "<text>points2constraint( _ )</text--->"
  "<text>converts a points value (set of points) from the spatial-algebra "
  "to a constraint value.</text--->"
  "<text>query points2constraint(train7stations scale[0.1])</text--->"
  ") )";

const string ConstraintSpecLine2Constraint  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>line -> constraint</text--->"
  "<text>line2constraint( _ )</text--->"
  "<text>converts a line value (set of segments) from the spatial-algebra "
  "to a constraint value.</text--->"
  "<text>query line2constraint(PotsdamLine scale[0.1])</text--->"
  ") )";

const string ConstraintSpecRegion2Constraint  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>region x bool -> constraint</text--->"
  "<text>region2constraint( _ , _ )</text--->"
  "<text>converts a region value (set of faces = polygons with "
  "inner holes) "
  " from the spatial-algebra to a constraint value. "
  "The first argument is the region, the second argument is"
  " a boolean flag which indicates if the function should also "
  " merge the triangules to bigger convex polygons if possible.</text--->"
  "<text>query region2constraint(thecenter scale[0.1], TRUE)</text--->"
  ") )";

const string ConstraintSpecConstraint2Point  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>constraint -> point</text--->"
  "<text>constrain2point( _ )</text--->"
  "<text>converts a constraint value (provided it does contain"
  " one single point) "
  " to a point value of the spatial-algebra.</text--->"
  "<text>query constraint2point(objConstraintPoint1)</text--->"
  ") )";

const string ConstraintSpecConstraint2Points  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>constraint -> points</text--->"
  "<text>constrain2points( _ )</text--->"
  "<text>converts a constraint value (provided it does contain points) "
  " to a points value of the spatial-algebra.</text--->"
  "<text>query constraint2points(objConstraintPoints1)</text--->"
  ") )";

const string ConstraintSpecConstraint2Line  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>constraint -> line</text--->"
  "<text>constrain2line( _ )</text--->"
  "<text>converts a constraint value (provided it does contain "
  "line-segments) "
  " to a line value of the spatial-algebra.</text--->"
  "<text>query constraint2line(objConstraintLine1)</text--->"
  ") )";

const string ConstraintSpecConstraint2Region  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>stream(constraint) -> stream(region)</text--->"
  "<text>_ constrain2region _</text--->"
  "<text>converts a stream of constraint values (provided it does "
  "contain not only points and lines) "
  " to a stream of region values of the spatial-algebra.</text--->"
  "<text>query objConstraintPolygon1 feed constraint2region "
  "count</text--->"
  ") )";


/*
9.2 Definition of the operators

*/
Operator constraintunion( "cunion",
                          ConstraintSpecUnion,
                          unionValueMap,
                          Operator::SimpleSelect,
                          CxC2CTypeMap);

Operator constraintjoin( "cjoin",
                          ConstraintSpecJoin,
                          intersectionValueMap,
                          Operator::SimpleSelect,
                          CxC2CTypeMap);

Operator constraintintersection( "cintersection",
                          ConstraintSpecIntersection,
                          intersectionValueMap,
                          Operator::SimpleSelect,
                          CxC2CTypeMap);

Operator constraintprojection( "cprojection",
                          ConstraintSpecProjection,
                          projectionValueMap,
                          Operator::SimpleSelect,
                          projectionTypeMap);

Operator constraintselection( "cselection",
                          ConstraintSpecSelection,
                          selectionValueMap,
                          Operator::SimpleSelect,
                          selectionTypeMap);

Operator constraintsatisfy( "csatisfy",
                          ConstraintSpecSatisfy,
                          satisfyValueMap,
                          Operator::SimpleSelect,
                          C2BoolTypeMap);

Operator constraintoverlaps( "coverlaps",
                          ConstraintSpecOverlaps,
                          overlapsValueMap,
                          Operator::SimpleSelect,
                          CxC2BoolTypeMap);

Operator no_tuples( "no_tuples",
                          ConstraintSpecNo_Tuples,
                          no_tuplesValueMap,
                          Operator::SimpleSelect,
                          C2IntTypeMap);

Operator no_constraints( "no_constraints",
                          ConstraintSpecNo_Constraints,
                          no_constraintsValueMap,
                          Operator::SimpleSelect,
                          C2IntTypeMap);

Operator bbox( "bbox",
                          ConstraintSpecBBox,
                          bboxValueMap,
                          Operator::SimpleSelect,
                          C2RectTypeMap);

Operator triangulate( "triangulate",
                          ConstraintSpecTriangulate,
                          triangulateValueMap,
                          Operator::SimpleSelect,
                          RegionStream2RegionStreamTypeMap);

Operator point2constraint( "point2constraint",
                          ConstraintSpecPoint2Constraint,
                          point2constraintValueMap,
                          Operator::SimpleSelect,
                          Point2CTypeMap);

Operator points2constraint( "points2constraint",
                          ConstraintSpecPoints2Constraint,
                          points2constraintValueMap,
                          Operator::SimpleSelect,
                          Points2CTypeMap);

Operator line2constraint( "line2constraint",
                          ConstraintSpecLine2Constraint,
                          line2constraintValueMap,
                          Operator::SimpleSelect,
                          Line2CTypeMap);

Operator region2constraint( "region2constraint",
                          ConstraintSpecRegion2Constraint,
                          region2constraintValueMap,
                          Operator::SimpleSelect,
                          RegionxBool2CTypeMap);

Operator constraint2point( "constraint2point",
                          ConstraintSpecConstraint2Point,
                          constraint2pointValueMap,
                          Operator::SimpleSelect,
                          C2PointTypeMap);

Operator constraint2points( "constraint2points",
                          ConstraintSpecConstraint2Points,
                          constraint2pointsValueMap,
                          Operator::SimpleSelect,
                          C2PointsTypeMap);

Operator constraint2line( "constraint2line",
                          ConstraintSpecConstraint2Line,
                          constraint2lineValueMap,
                          Operator::SimpleSelect,
                          C2LineTypeMap);

Operator constraint2region( "constraint2region",
                          ConstraintSpecConstraint2Region,
                          constraint2regionValueMap,
                          Operator::SimpleSelect,
                          CStream2RegionStreamTypeMap);


/*
10 Creating the Algebra

*/

class ConstraintAlgebra : public Algebra
{
 public:
  ConstraintAlgebra() : Algebra()
  {
    AddTypeConstructor( &constraint );

    constraint.AssociateKind("DATA");
	constraint.AssociateKind("SPATIAL2D");

    AddOperator( &constraintunion );
    AddOperator( &constraintjoin );
    AddOperator( &constraintintersection );
    AddOperator( &constraintprojection );
    AddOperator( &constraintselection );
    AddOperator( &constraintsatisfy );
    AddOperator( &constraintoverlaps );
    AddOperator( &no_tuples );
    AddOperator( &no_constraints );
    AddOperator( &bbox );
    AddOperator( &triangulate );
    AddOperator( &point2constraint );
    AddOperator( &points2constraint );
    AddOperator( &line2constraint );
    AddOperator( &region2constraint );
    AddOperator( &constraint2point );
    AddOperator( &constraint2points );
    AddOperator( &constraint2line );
    AddOperator( &constraint2region );
  }
  ~ConstraintAlgebra() {};
};

ConstraintAlgebra constraintAlgebra;

/*
11 Initialization

["]Each algebra module needs an initialization function. The algebra manager has a reference to this function if this algebra is included in the list of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the inst. of the algebra class and to provide references to the global nested list container (used to store constructor, type, operator and object infor.) and to the query processor.

The function has a C interface to make it possible to load the algebra dynamically at runtime.["] [Point02]

*/
extern "C"
Algebra*
InitializeConstraintAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&constraintAlgebra);
}

} // namespace

/*
12 References

[Point02] Algebra Module PointRectangleAlgebra. FernUniversit[ae]t Hagen, Praktische Informatik IV, Secondo System, Directory ["]Algebras/PointRectangle["], file ["]PointRectangleAlgebra.cpp["], since July 2002

*/
