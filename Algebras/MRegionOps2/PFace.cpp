/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
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
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[pow] [\verb+^+]

[1] Headerfile of the Point and Vector classes

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype, 

April - November 2014, S. Schroer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#include "PFace.h"
#include "Point3DExt.h"
#include "PointExtSet.h"
#include "IntersectionSegment.h"


namespace mregionops2 {

PFace::PFace(SourceUnit2* _unit, Point2D _is, 
             Point2D _ie, Point2D _fs, Point2D _fe,
             bool _insideAbove, unsigned int _cycNo, 
             unsigned int _faceNo) :

    unit(_unit), 
    is(_is),
    ie(_ie),
    fs(_fs),
    fe(_fe),
    insideAbove(_insideAbove),
    cycleNo(_cycNo),
    faceNo(_faceNo)
{

    SetInitialStartPointIsA();
//    ComputeBoundingRect();
    ComputeNormalVector();
    ComputeVerticalVector();
    ComputeWVector();
}

// case 1: the segment lies on eges from p-face
PFace::TouchMode PFace::LiesOnBorder(const Segment3D& seg)
    {
      if (seg.GetStart().LiesBetween(GetA_XYT(), GetC_XYT()) &&
          seg.GetEnd().LiesBetween(GetA_XYT(), GetC_XYT()))
        return LEFT;

      if (seg.GetStart().LiesBetween(GetB_XYT(), GetD_XYT()) &&
          seg.GetEnd().LiesBetween(GetB_XYT(), GetD_XYT()))
        return RIGHT;

      return NONE;
    }


PFace::TouchMode PFace::BorderLiesOn(PFace& pf)
    {
      TouchMode touchMode = NONE;

// left eges
      if (GetA_XYT().LiesBetween(pf.GetA_XYT(), pf.GetB_XYT()) &&
          GetC_XYT().LiesBetween(pf.GetC_XYT(), pf.GetD_XYT()) &&
	  !(((GetA_XYT() == pf.GetA_XYT()) && (GetC_XYT() == pf.GetC_XYT())) ||
	    ((GetA_XYT() == pf.GetB_XYT()) && (GetC_XYT() == pf.GetD_XYT()))))
      {
        touchMode = LEFT;
      }

// right eges
      if (GetB_XYT().LiesBetween(pf.GetA_XYT(), pf.GetB_XYT()) &&
          GetD_XYT().LiesBetween(pf.GetC_XYT(), pf.GetD_XYT()) &&
	  !(((GetB_XYT() == pf.GetA_XYT()) && (GetD_XYT() == pf.GetC_XYT())) ||
	    ((GetB_XYT() == pf.GetB_XYT()) && (GetD_XYT() == pf.GetD_XYT()))))
      {

// which eges left or right on the pface
        if (touchMode == LEFT)
	  touchMode = BOTH;
	else
	  touchMode = RIGHT;
      }
      return touchMode;
    }

// p-faces are parallel
bool PFace::IsParallel(const PFace& pf)
{

// if the normalvector are a duplicated from the other normalvector
// the p-faces are "linear abhängig" and parallel
Vector3D nv = pf.GetNormalVector();
return ((normalVector.GetX() * nv.GetY() == normalVector.GetY() * nv.GetX()) &&
(normalVector.GetY() * nv.GetZ() == normalVector.GetZ() * nv.GetY()) &&
(normalVector.GetX() * nv.GetZ() == normalVector.GetZ() * nv.GetX()));
}

bool PFace::IntersectionPlaneSegment(const Segment3D& seg, Point3D& result) {

// Precondition: 
// We compute the intersection point
// of the plane - defined by the PFace - and the segment.

    Vector3D u(seg.GetStart(), seg.GetEnd());
    Vector3D w(GetA_XYT(), seg.GetStart());

    mpq_class d = GetNormalVector() * u;
    mpq_class n = -(GetNormalVector() * w);

    if (d == 0) // Segment is parallel to plane.
        return false;

    mpq_class s = n / d;

    // No intersection point, if s < or s > 1
    if ((s < 0) || (s > 1)) 
        return false;

    // Compute segment intersection point:
    result = seg.GetStart() + s * u;

    return true;
}

bool PFace::Isintersectedby(const Segment3D seg, Segment3D& resultseg)
{
//  precondition: seg liegt in der gleichen Ebene wie das pface
//  Anfang und Endpunkte liegen auf der selben Zeitachse wie das pface

//  Variablen für die umgerechneten w-Koordinaten
    mpq_class w_A;
    mpq_class w_B;
    mpq_class w_C;
    mpq_class w_D;
    mpq_class w_x1;
    mpq_class w_x2;

//  calculate the w-coordinate for all points
    w_A =  TransformToW(GetA_XYT());
    w_B =  TransformToW(GetB_XYT());
    w_C =  TransformToW(GetC_XYT());
    w_D =  TransformToW(GetD_XYT());
    w_x1 = TransformToW(seg.GetStart());
    w_x2 = TransformToW(seg.GetEnd());

//  if operation = MINUS and UnitB, then the normalenvector turn over
    SetOp op = GetOperation();
    if ((op == MINUS) && IsPartOfUnitB()) 
	  {
	  w_A = -w_A,
	  w_B = -w_B;
	  w_C = -w_C;
	  w_D = -w_D;
	  w_x1 = -w_x1;
	  w_x2 = -w_x2;
	  }

// case one: seg is complete in pface
     if ((w_A <= w_x1) && (w_x1 <= w_B) &&
         (w_C <= w_x2) && (w_x2 <= w_D))
     {
        resultseg = seg;
        return true;
     }

   bool isegfound = false;
   Point3D P_left;

// case two: schneidet das seg die linke Kante
if (((w_C - w_x2) * (w_A - w_x1)) < 0)
{
	isegfound = true;
	mpq_class ratio;
	ratio = abs(w_x1 - w_A) / (abs(w_C - w_x2) + abs(w_x1 - w_A));
	P_left = Point3D(GetA_XYT(), GetC_XYT(), ratio);
        if (w_x1 < w_A)
	{
         resultseg = Segment3D(P_left, seg.GetEnd() );
        }
	else
	{
	 resultseg = Segment3D(seg.GetStart(), P_left );
	}
}

// case three : schneidet das seg die rechte Kante
if (((w_D - w_x2) * (w_B - w_x1)) < 0)
{
	mpq_class ratio;
	ratio = abs(w_x1 - w_B) / (abs(w_D - w_x2) + abs(w_x1 - w_B));
	Point3D P_right = Point3D(GetB_XYT(), GetD_XYT(), ratio);

	if (isegfound)
	{
	    resultseg = Segment3D( P_left, P_right); 
	    return true;
	}

	if (w_x1 < w_B)
	    resultseg = Segment3D(seg.GetStart(), P_right );
	else
	    resultseg = Segment3D(P_right, seg.GetEnd());
            isegfound = true;
}

return isegfound;

}

void PFace::Intersection(PFace& pf)
{
// 1. parallel, sind linear abhängig
   if (this->IsParallel(pf) )
   {
	Segment3D iseg;
	Segment3D resultseg;

//	1a. koplanar, liegen in einer Ebene
	Vector3D vec (GetA_XYT(), pf.GetA_XYT());
	if (vec * GetNormalVector() != 0)
	{
	  return;
	}

	if (Isintersectedby(Segment3D(pf.GetA_XYT(), pf.GetC_XYT()), resultseg))
	{

           AddIntSeg(resultseg, pf.GetAngle(), POSITIVE);
        }

	if (Isintersectedby(Segment3D(pf.GetB_XYT(), GetD_XYT()), resultseg))
	{ 
            AddIntSeg(resultseg, pf.GetAngle().GetOpposite(), NEGATIVE);
        }
 
        if (pf.Isintersectedby(Segment3D(GetA_XYT(), GetC_XYT()), resultseg))
        {
            pf.AddIntSeg(resultseg, GetAngle(), POSITIVE);
        }
        if (pf.Isintersectedby(Segment3D(GetB_XYT(), GetD_XYT()), resultseg))
        {
	    pf.AddIntSeg(resultseg, GetAngle().GetOpposite(), NEGATIVE);
	} 
  
        return;
    }
        
// 2. pfaces are not parallel 
// We store all edges of this PFaceA as 3DSegments in the vector edgesPFaceA:
   vector<Segment3D> edgesPFaceA;

// da stehen drei / vier Kanten = EdgesPFaceA / 
// B = Menge der Kanten beider Pfaces
   edgesPFaceA.push_back(Segment3D(this->GetA_XYT(), this->GetC_XYT()));
   edgesPFaceA.push_back(Segment3D(this->GetB_XYT(), this->GetD_XYT()));

// kein Dreieck was auf der Spitze steht
   if (!this->GetPointInitial())
      edgesPFaceA.push_back(Segment3D(this->GetA_XYT(), this->GetB_XYT()));

// Endpunkt auch keine Spitze vom Dreieck
   if (!this->GetPointFinal())
      edgesPFaceA.push_back(Segment3D(this->GetC_XYT(), this->GetD_XYT()));

// We store all edges of the other PFaceB as 3DSegments
// in the vector edgesPFaceB:
   vector<Segment3D> edgesPFaceB;

   edgesPFaceB.push_back(Segment3D(pf.GetA_XYT(), pf.GetC_XYT()));
   edgesPFaceB.push_back(Segment3D(pf.GetB_XYT(), pf.GetD_XYT()));

// kein Dreieck was auf der Spitze steht
   if (!pf.GetPointInitial())
      edgesPFaceB.push_back(Segment3D(pf.GetA_XYT(), pf.GetB_XYT()));

// Endpunkt auch keine Spitze vom Dreieck
   if (!pf.GetPointFinal())
      edgesPFaceB.push_back(Segment3D(pf.GetC_XYT(), pf.GetD_XYT()));

   PointExtSet intPointSet;
   Point3DExt intPoint;

// Intersect the plane - defined by the other PFace - 
// with all edges of this PFace, if two intersections found
// not parallel, only two intersections are possible
   unsigned int i = 0;
   while (intPointSet.Size() < 2 && i < edgesPFaceA.size()) {

      if (pf.IntersectionPlaneSegment(edgesPFaceA[i], intPoint)) {
          intPoint.sourceFlag = PFACE_A;
          intPointSet.Insert(intPoint);
      }
      i++;
  }

// We need exactly two intersection points.
   if (intPointSet.Size() != 2) 
      return;

// Intersect the plane - defined by this PFace - 
// with all edges of the other PFace: Max zwei aus A und weitere zwei aus B = 4
   unsigned int j = 0;
   while (intPointSet.Size() < 4 && j < edgesPFaceB.size()) {

      if (this->IntersectionPlaneSegment(edgesPFaceB[j], intPoint)) {
          intPoint.sourceFlag = PFACE_B;
          intPointSet.Insert(intPoint);
      }
      j++;
  }

  Segment3D intSeg;
    
// There is no intersection
   if (!intPointSet.GetIntersectionSegment(intSeg))
      return; 
  
// second case is orthogonal to t-Achse (waagerecht - wichtig für Rand)
   if (intSeg.IsOrthogonalToTAxis())
   {
    Direction dir;
    Direction oppDir;

//  VerticalVektor steht senkrecht auf der unteren Kante und 
//  liegt in der Ebene drin
//  Vektor der von A --> B geht
//  Normalenvektor der senkrecht auf der Fläche steht und nach außen zeigt
//  Schnittsegement das Paralle zur XY Achse liegt, 
//  zeigt in die selbe Richtung A-B
//  Senkrecht auf Normalenvektor + senkrecht auf der Grundlinie, 
//  um Winkel zu berechnen
//  wir brauchen nur einen relativen Winkel
//  X- / Y-Zeile ... Vektor der nach innen zeigt, deshalb negativ
//  2-dim Koordinatensystem (VerticalVektor - NormalenVektor)

Angle ang = Angle(Vector2D(pf.GetVerticalVector() * GetVerticalVector(),
                         -(pf.GetVerticalVector() * GetNormalVector())));
Angle pfAng = Angle(Vector2D(GetVerticalVector() * pf.GetVerticalVector(),
                         -(GetVerticalVector() * pf.GetNormalVector())));

//  zeigen in die selbe Richtung - Fläche in die selbe Richtung
//  zeigen in die selbe Richtung - Winkel < 90 Grad, das Skalarprodukt positiv
    if ((GetNormalVector() * pf.GetNormalVector()) > 0)
   	 {
     	  dir = POSITIVE;
    	  oppDir = NEGATIVE;
  	 }
    else
	 {
	      dir = NEGATIVE;
	      oppDir = POSITIVE;
	 }
	
    AddHorizontalIntSeg(intSeg, ang.GetOpposite(), oppDir);
    pf.AddHorizontalIntSeg(intSeg, pfAng.GetOpposite(), oppDir);

    AddHorizontalIntSeg(intSeg, ang, dir);
    pf.AddHorizontalIntSeg(intSeg, pfAng, dir);
    return;

    }

// p-faces not parallel
// the intersection segment is not orthogonal to t-Achse
   switch(this->LiesOnBorder(intSeg))
   {
   case LEFT:
     pf.AddIntSeg(intSeg, GetAngle(), POSITIVE);
     break;
   case RIGHT:
     pf.AddIntSeg(intSeg, GetAngle().GetOpposite(), NEGATIVE);
     break;
   case NONE:
     pf.AddIntSeg(intSeg, GetAngle(), POSITIVE);
     pf.AddIntSeg(intSeg, GetAngle().GetOpposite(), NEGATIVE);
     break;
   case BOTH:
     break;
   }

  switch(pf.LiesOnBorder(intSeg))
  {
   case LEFT:
     AddIntSeg(intSeg, pf.GetAngle(), POSITIVE);
     break;
   case RIGHT:
     AddIntSeg(intSeg, pf.GetAngle().GetOpposite(), NEGATIVE);
     break;
   case NONE:
     AddIntSeg(intSeg, pf.GetAngle(), POSITIVE);
     AddIntSeg(intSeg, pf.GetAngle().GetOpposite(), NEGATIVE);
     break;
   case BOTH:
     break;
  }
  }

void PFace::AddIntSeg(const Segment3D& seg, Angle angle, Direction dir)
{
    IntersectionSegment* intSeg;

    SetOp op = GetOperation();
    Angle pAng = GetAngle();
    Angle pAngOpp = pAng.GetOpposite();

    if ((op == UNION) || ((op == MINUS) && IsPartOfUnitB()))
    {
intSeg = new IntersectionSegment(seg, this, angle - pAngOpp, pAng - angle, dir);
    }
    else
    {
intSeg = new IntersectionSegment(seg, this, pAngOpp - angle, angle - pAng, dir);
    }
        
    intSegs.AddIntSeg(intSeg);
    
//  Plumbline sparen mit dem Wissen der Nr der Zyklen und pfaces
//  GetT - neue Punkte der resultierenden U-Region
    unit->SetCycleStatus(faceNo, cycleNo, HASINTSEGS);
    unit->AddTimestamp(seg.GetStart().GetT());
    unit->AddTimestamp(seg.GetEnd().GetT());
}

void PFace::AddHorizontalIntSeg(const Segment3D& seg, 
                                Angle angle, Direction dir)
{
    IntersectionSegment* intSeg;

    SetOp op = GetOperation();
    Angle pAng = Angle(0);
    Angle pAngOpp = pAng.GetOpposite();

    if ((op == UNION) || ((op == MINUS) && IsPartOfUnitB()))
    {
intSeg = new IntersectionSegment(seg, this, angle - pAngOpp, pAng - angle, dir);
    }
    else
    {
intSeg = new IntersectionSegment(seg, this, pAngOpp - angle, angle - pAng, dir);
    }
        
    horizontalIntSegs.AddIntSeg(intSeg);
    
    unit->SetCycleStatus(faceNo, cycleNo, HASINTSEGS);
    unit->AddTimestamp(seg.GetStart().GetT());
    unit->AddTimestamp(seg.GetEnd().GetT());
}
    
void PFace::AddToIntSegsTable(unsigned int ind, IntersectionSegment* iseg) 
{
  intSegsToInterval.insert(pair<unsigned int, IntersectionSegment*>(ind,iseg));
}

Angle PFace::GetAngle()
{
  if (angle.IsInfinite()) ComputeAngle();
  
  return angle;
}

void PFace::ComputeAngle()
{
  Vector2D v;

// if point A equals point B
  if (AEqualsB())
  {
    v = Vector2D(GetC_XY(), GetD_XY());
  }
  else
  {
    v = Vector2D(GetA_XY(), GetB_XY());
  }

  angle = Angle(v);
}

void PFace::ComputeNormalVector()
{
  Vector3D v1, v2;

  if (!AEqualsB())
  {
    v1 = Vector3D(GetA_XYT(), GetB_XYT());
    v2 = Vector3D(GetA_XYT(), GetC_XYT());
  }
  else
  {
    v1 = Vector3D(GetD_XYT(), GetC_XYT());
    v2 = Vector3D(GetD_XYT(), GetB_XYT());
  }

// Skalarprodukt = 0 -> normalVector
// sqr isn't exakt, needs rational datatype
   normalVector = v1.CrossProduct(v2);
}

void PFace::ComputeVerticalVector()
{
  Vector3D v;

  if (!AEqualsB())
  {
    v = Vector3D(GetA_XYT(), GetB_XYT());
  }
  else
  {
    v = Vector3D(GetC_XYT(), GetD_XYT());
  }

  verticalVector = normalVector.CrossProduct(v);
}

void PFace::ComputeWVector() {

    Vector3D tVector(0, 0, 1);
    // The vector w is either the normalized cross product 
    // of the normal vector and the t-unit-vector, or it's opposite.
    // This depends on the kind of set-operation, we want to perform.
    wVector = tVector.CrossProduct(normalVector);
    
    // Usually, we look at the PFace from the inside.   
    if (GetOperation() == MINUS && IsPartOfUnitB()) {
        
    // Only in this case, we want to look from the outside.
    wVector = (-1) * wVector;
    }
}

void PFace::FinalizeIntSegs()
{

// IntersectionSegment::Finalize()
   intSegs.FinalizeIntSegs();

   if (GetOperation() == MINUS && IsPartOfUnitB())
   {
// dreht in IntersectionSegment::InvertAreaDirection die Richtung rum
// der Normalenvektor ändert sich, deshalb Richtungswechsel
    intSegs.InvertAreaDirections();
   }

// IntersectionSegment::Finalize()
   horizontalIntSegs.FinalizeIntSegs();
   intSegs.FillIntSegsTable(unit->GetTimeIntervalCount());
}


// füll die SET-Liste für das Zeitintervall n 
void PFace::FillIntSegIntervallList(unsigned int intervalNo)
{

   intSegIntervalList.clear();
   mpq_class startTime = GetTimeIntervalStart(intervalNo);
   mpq_class endTime = GetTimeIntervalEnd(intervalNo);
   
   pair<multimap<unsigned int, IntersectionSegment*>::iterator,
        multimap<unsigned int, IntersectionSegment*>::iterator> eqr;
   eqr = intSegsToInterval.equal_range(intervalNo);
   for (multimap<unsigned int, IntersectionSegment*>::iterator iter = eqr.first;
        iter != eqr.second; iter++)
   {
     IntersectionSegment* intSeg = iter->second;
     intSeg->ComputeIntervalW(startTime, endTime);
     intSegIntervalList.insert(intSeg);
   }
}

void PFace::SetInitialStartPointIsA() {
    
   if (is != ie) {
        
    initialStartPointIsA = (is < ie) == insideAbove;
        
    } else {
        
    initialStartPointIsA = (fs < fe) == insideAbove;
    }
    
}

void PFace::Print() {

    cout << "PFace:" << endl;
    cout << endl;
    
    cout << "Starttime: " << unit->GetStartTime() << endl;
    cout << "Endtime: " << unit->GetEndTime() << endl;
    
    Point3D p_tmp = GetA_XYT();
    cout << "A_XYT: " << p_tmp << endl;
    p_tmp = GetB_XYT();
    cout << "B_XYT: " << p_tmp << endl;
    p_tmp = GetC_XYT();
    cout << "C_XYT: " << p_tmp << endl;
    p_tmp = GetD_XYT();
    cout << "D_XYT: " << p_tmp << endl;
    cout << endl;

//  cout << "A_WT: " << GetA_WT() << endl;
//  cout << "B_WT: " << GetB_WT() << endl;
//  cout << "C_WT: " << GetC_WT() << endl;
//  cout << "D_WT: " << GetD_WT() << endl;

    cout << endl;
    Vector3D vec = normalVector;
    cout << "Normal vector: " << vec << endl;
    vec = verticalVector;
    cout << "Vertical vector: " << vec << endl;
    vec = wVector;
    cout << "W vector: " << vec << endl;
    cout << endl;
//  cout << "State: " << GetStateAsString() << endl;
    cout << "initialStartPointIsA: " << initialStartPointIsA << endl;
    cout << "insideAbove: " << insideAbove << endl;
    cout << "Cycle number: " << cycleNo << endl;
    cout << "face number: " << faceNo << endl;
    Angle a = angle;
    cout << "angle: " << a << endl;
    cout << endl;
    cout << "IntersectionSegments: " << endl;
    intSegs.Print();
    cout << "Horizontal intersectionSegments: " << endl;
    horizontalIntSegs.Print();
    cout << endl;
    cout << "IntSegsTable: " << endl;
    if (intSegsToInterval.size() == 0)
    {
      cout << "   Empty" << endl;
    }
    else
    {
multimap<unsigned int, IntersectionSegment*>::iterator iter;
for (iter = intSegsToInterval.begin(); iter != intSegsToInterval.end(); iter++)
    {
     cout << (*iter).first <<  "=>" << endl;
     cout << (*iter).second << endl;
    }
    }

    mpq_class wt;
Point3D tmp(GetA_XYT().GetX() + ((GetB_XYT().GetX() - GetA_XYT().GetX()) / 3),
            GetA_XYT().GetY() + ((GetB_XYT().GetY() - GetA_XYT().GetY()) / 3),
            GetA_XYT().GetT());
    wt = TransformToW(tmp);
    cout << " w 1/3: " << wt << endl;
tmp = Point3D(GetA_XYT().GetX()+2*((GetB_XYT().GetX()-GetA_XYT().GetX()) / 3),
              GetA_XYT().GetY()+2*((GetB_XYT().GetY()-GetA_XYT().GetY()) / 3),
              GetA_XYT().GetT());
    wt = TransformToW(tmp);
    cout << " w 2/3: " << wt << endl;

    cout << endl;
    cout << "*********************************************" << endl;
    cout << endl;
}

}
