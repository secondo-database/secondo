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

public class FPoint3D implements Object3D{

public Triangle3DVector getTriangles(){ return null;}
public Line3DVector getLines(){ return null;}

public IDPoint3DVector getPoints(){return Points;}


/** check if the type from SO is a FPoint
  */
public boolean checkType(SecondoObject SO){
  ListExpr LE = SO.toListExpr();
  if(LE.listLength()!=2) 
     return false;
  else
    return (LE.first().isAtom() && LE.first().atomType()==ListExpr.SYMBOL_ATOM && 
            LE.first().symbolValue().equals("fpoint"));
}

/** read the value of this Fpoint from SO */
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

public String toString(){return Name;}


/** read this FPoint from a non typed ListExpr */
private boolean readFromListExpr(ListExpr LE){

  double SF = 1.0;
  Points = new IDPoint3DVector();
  SingleFPoints = new Vector();
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
  
 // read the single Points
  ListExpr points_le = LE.second();
  SingleFPoint P;
  boolean ok = true; 
  while( !points_le.isEmpty() & ok) {
    P = new SingleFPoint();
    if(P.readFromListExpr(points_le.first())){
       SingleFPoints.add(P);
       points_le=points_le.rest();
    }
    else
       ok = false;
  }

  computePoints3D();
  return ok;
}



private void computePoints3D(){
  // compute the 3DPointVector
  Points = new IDPoint3DVector(); 
  IDPoint3D P3D;
  SingleFPoint P;
  BB.set(0,0,0,0,0,0);
  BoundingBox3D BB2= new BoundingBox3D();
  for(int i=0;i<SingleFPoints.size();i++){
      P = (SingleFPoint) SingleFPoints.get(i);
      if(i==0) 
         BB.set(P.x,P.y,(int)(P.z*ScaleFactor),P.x,P.y,(int)(P.z*ScaleFactor));
      else{
         BB2.set(P.x,P.y,(int)(P.z*ScaleFactor),P.x,P.y,(int)(P.z*ScaleFactor));
         BB.extend(BB2);
      } 

      P3D = new IDPoint3D(P.x,P.y,
                          P.z*ScaleFactor,
                          getRed(P.z),getGreen(P.z),getBlue(P.z));
      P3D.setID(myID); // each member of this FPoint has the same ID
      Points.append(P3D);
  }
}


/** get red-part by given membership-value */
private int getRed(double delta){
  return (int)( (double)(minR) + delta*( (double) (maxR-minR)));
}

/** get green-part by given membership-value */
private int getGreen(double delta){
  return (int)( (double)(minG) + delta*( (double) (maxG-minG)));
}

/** get blue-part by given membership-value */
private int getBlue(double delta){
 return (int)( (double)(minB) + delta*( (double) (maxB-minB)));
}


public void showSettings(Frame F){
  FuzzySettings FS = new FuzzySettings(F);
  FS.setMinColor(new Color(minR,minG,minB));
  FS.setMaxColor(new Color(maxR,maxG,maxB));
  FS.show();
  if (FS.getReturnValue()==FuzzySettings.OK){
      Color C1 = FS.getMinColor();
      Color C2 = FS.getMaxColor();
      minR = C1.getRed();
      minG = C1.getGreen();
      minB = C1.getBlue();     
      maxR = C2.getRed();
      maxG = C2.getGreen();
      maxB = C2.getBlue();     
      computePoints3D();
  }
}



public ID getID(){return myID;};

public BoundingBox3D getBoundingBox(){ return BB;};


public boolean nearByXY(double x, double y,double exactness){
  Point3D P;
  boolean found = false;
  double d2;
  double e2 = exactness*exactness; // the square of exactness
  for(int i=0;i<Points.getSize()&&!found;i++){
     P = Points.get(i);
     d2=(P.getX()-x)*(P.getX()-x) + (P.getY()-y)*(P.getY()-y); // the square of distance
     if(d2<e2)
       found=true;
  }
  return found;
}




private IDPoint3DVector Points = new IDPoint3DVector();
private Vector SingleFPoints = new Vector();
private double ScaleFactor;
private int minR=0,minG=0,minB=0,maxR=255,maxG=255,maxB=255;
private ID myID= IDManager.getNextID();
private String Name;
private BoundingBox3D BB= new BoundingBox3D();;
}




