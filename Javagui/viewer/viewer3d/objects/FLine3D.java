//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package viewer.viewer3d.objects;

import sj.lang.ListExpr;
import viewer.viewer3d.graphic3d.*;
import java.util.Vector;
import gui.idmanager.*;
import gui.SecondoObject;
import java.awt.*;

public class FLine3D implements Object3D{

public Triangle3DVector getTriangles(){ return null;}
public Line3DVector getLines(){ return Lines;}
public IDPoint3DVector getPoints(){return null;}
/** get red-part by given membership-value */
private int getR(double delta){
  return (int)( (double)(minR) + delta*( (double) (maxR-minR)));
}
/** get green-part by given membership-value */
private int getG(double delta){
  return (int)( (double)(minG) + delta*( (double) (maxG-minG)));
}
/** get blue-part by given membership-value */
private int getB(double delta){
 return (int)( (double)(minB) + delta*( (double) (maxB-minB)));
}

public String toString(){return Name;}


/** check if the type from SO is a FLine
  */
public boolean checkType(SecondoObject SO){
  ListExpr LE = SO.toListExpr();
  if(LE.listLength()!=2) 
     return false;
  else
    return (LE.first().isAtom() && LE.first().atomType()==ListExpr.SYMBOL_ATOM && 
            LE.first().symbolValue().equals("fline"));
}

/** read the value of this FLine from SO */
public boolean readFromSecondoObject(SecondoObject SO){
   ListExpr LE = SO.toListExpr();
   if(LE.listLength()!=2)
      return false;
   if (!checkType(SO))
      return false;
   Name = SO.getName();
   myID = SO.getID();
   return readFromListExpr(LE.second());
}


/** read this FLine from a ListExpr
  * @return true if LE is a valid Representation of a FLine
  * all valid Segments of this List are inserted
  */
private boolean readFromListExpr(ListExpr LE){
  ScaleFactor = 1.0;
  SingleSegments = new Vector();
  Lines = new Line3DVector();

  if(LE==null)
     return false;
  if(LE.listLength()!=2)
     return false;
  ListExpr SFList = LE.first();
  if( !( SFList.isAtom() && (SFList.atomType()==ListExpr.INT_ATOM || SFList.atomType()==ListExpr.REAL_ATOM)))
     return false;
  double z= SFList.atomType()==ListExpr.INT_ATOM ? SFList.intValue() : SFList.realValue();
  if(z<=0)
     return false;
  this.ScaleFactor = z;

  ListExpr Segments = LE.second();
  SingleSegment S;
  boolean ok = true; 
  while( !Segments.isEmpty() & ok) {
    S = new SingleSegment();
    if(S.readFromListExpr(Segments.first())){
       SingleSegments.add(S);
       Segments=Segments.rest();
    }
    else
       ok = false;

  }

  computeLine3Ds();
  return ok;
}

private void computeLine3Ds(){
  Lines = new Line3DVector();
  SingleSegment SS;
  Line3D L3D;
  Point3D P3D1,P3D2;
  BB.set(0,0,0,0,0,0);
  int minx,miny,minz,maxx,maxy,maxz;
  BoundingBox3D BB2 = new BoundingBox3D();
  for (int i=0;i<SingleSegments.size();i++){
    SS = (SingleSegment) SingleSegments.get(i);
  
    minx = Math.min(SS.P1.x,SS.P2.x);
    miny = Math.min(SS.P1.y,SS.P2.y);
    minz = Math.min((int)(ScaleFactor*SS.P1.z),(int)(ScaleFactor*SS.P2.z));
    maxx = Math.max(SS.P1.x,SS.P2.x);
    maxy = Math.max(SS.P1.y,SS.P2.y);
    maxz = Math.max((int)(ScaleFactor*SS.P1.z),(int)(ScaleFactor*SS.P2.z));
    if(i==0)
       BB.set(minx,miny,minz,maxx,maxy,maxz);
    else{
       BB2.set(minx,miny,minz,maxx,maxy,maxz);
       BB.extend(BB2);
    }

    P3D1 = new Point3D(SS.P1.x,SS.P1.y,SS.P1.z*ScaleFactor,getR(SS.P1.z),getG(SS.P1.z),getB(SS.P1.z));
    P3D2 = new Point3D(SS.P2.x,SS.P2.y,SS.P2.z*ScaleFactor,getR(SS.P2.z),getG(SS.P2.z),getB(SS.P2.z));
    L3D = new Line3D(P3D1,P3D2,myID);
    Lines.append(L3D);
  } 
}

public ID getID(){return myID;};


public BoundingBox3D getBoundingBox(){ return BB;}

private Line3DVector Lines = new Line3DVector();
private Vector SingleSegments = new Vector();
private double ScaleFactor;
private int minR=0,minG=0,minB=0,maxR=255,maxG=255,maxB=255;
private ID myID = IDManager.getNextID();
private String Name;
private BoundingBox3D BB= new BoundingBox3D();

public void showSettings(Frame F){
  FuzzySettings FS = new FuzzySettings(F);
  FS.setMinColor(new Color(minR,minG,minB));
  FS.setMaxColor(new Color(maxR,maxG,maxB));
  FS.show();
  if (FS.getReturnValue()==FS.OK){
      Color C1 = FS.getMinColor();
      Color C2 = FS.getMaxColor();
      minR = C1.getRed();
      minG = C1.getGreen();
      minB = C1.getBlue();     
      maxR = C2.getRed();
      maxG = C2.getGreen();
      maxB = C2.getBlue();     
      computeLine3Ds();
  }
}




public boolean nearByXY(double x, double y,double exactness){
  boolean found = false;
  Line3D L;
  double e2 = exactness*exactness;
  for(int i=0;i<Lines.getSize()&&!found;i++){
     L = Lines.get(i);
     if (nearByXY(L.getEP1(),L.getEP2(),x,y,e2))
        found = true;
   }      
   return found;
}


/** returns true if the vertical line in xy is in the near of the
  * segment defined by P1,P2
  */
static boolean nearByXY(Point3D P1, Point3D P2,double x, double y,double square_exactness){

double P1x = P1.getX(),
       P1y = P1.getY(),
       P2x = P2.getX(),
       P2y = P2.getY();
     
double d2 = (P1x-x)*(P1x-x) + (P1y-y)*(P1y-y); 
if(d2<square_exactness)
   return true; // near of P1
d2 = (P2x-x)*(P2x-x) + (P2y-y)*(P2y-y);
if(d2<square_exactness)
   return true;  // near of P2

if( (P1x<x & P2x<x)  || (P1x>x & P2x>x) )  // left or right from (xy)
  return false;

if( (P1y<y & P2y<y)  || (P1y>y & P2y>y) )  // above or under (xy)
   return false;
   
// algorithm from http://astronomy.swin.edu.au/~pbourke/geometry/pointline
d2 = (P2x-P1x)*(P2x-P1x) + (P2y-P1y)*(P2y-P1y); // distance between P1 and P1
double u = ((x-P1x)*(P2x-P1x) + (y-P1y)*(P2y-P1y))/d2;
// compute the point of intersection
double x4 = P1x+u*(P2x-P1x);
double y4 = P1y+u*(P2y-P1y);

double dist2 = (x4-x)*(x4-x) + (y4-y)*(y4-y);
if (dist2<square_exactness)
   return true;
else
   return false;
}


private class SingleSegment{

SingleFPoint P1,P2;

public boolean readFromListExpr(ListExpr LE){
  if(LE.listLength()!=2)
     return false;
  SingleFPoint tmpP1 = new SingleFPoint();
  SingleFPoint tmpP2 = new SingleFPoint();
  if(!( tmpP1.readFromListExpr(LE.first()) && tmpP2.readFromListExpr(LE.second())))
     return false; 
   
  this.P1 = tmpP1;
  this.P2 = tmpP2;
  return true;
}

}
}

