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

public class FRegion3D implements Object3D{

public Triangle3DVector getTriangles(){ return Triangles;}
public Line3DVector getLines(){ return null;}
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


public ID getID(){return myID;};

/** check if the type from SO is a FRegion
  */
public boolean checkType(SecondoObject SO){
  ListExpr LE = SO.toListExpr();
  if(LE.listLength()!=2)
     return false;
  else
    return (LE.first().isAtom() && LE.first().atomType()==ListExpr.SYMBOL_ATOM &&
            LE.first().symbolValue().equals("fregion"));
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
   boolean ok = readFromListExpr(LE.second());
   return ok;
}



/** read this FRegion from a ListExpr
  * @return true if LE is a valid Representaion of a FRegion
  */
public boolean readFromListExpr(ListExpr LE){
  ScaleFactor = 1.0;
  SingleTriangles = new Vector();
  Triangles = new Triangle3DVector();

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

  ListExpr triangle_list = LE.second();
  SingleTriangle T;
  boolean ok = true;
  while( !triangle_list.isEmpty() & ok) {
    T = new SingleTriangle();
    if(T.readFromListExpr(triangle_list.first())){
       SingleTriangles.add(T);
       triangle_list=triangle_list.rest();
    }
    else{
       ok = false;
       //System.out.println("error reading TriangleList :"+Triangles.first().writeListExprToString());
     }

  }

  computeTriangles3D();
  return ok;
}

private void computeTriangles3D(){
  Triangles = new Triangle3DVector();
  SingleTriangle ST;
  Triangle3D T3D;
  Point3D P3D1,P3D2,P3D3;
  BoundingBox3D BB2 = new BoundingBox3D();
  int z1,z2,z3;
  int minx,miny,minz;
  int maxx,maxy,maxz;
  for (int i=0;i<SingleTriangles.size();i++){
    ST = (SingleTriangle) SingleTriangles.get(i);

     z1 =(int)(ScaleFactor*ST.P1.z);
     z2 =(int)(ScaleFactor*ST.P2.z);
     z3 =(int)(ScaleFactor*ST.P3.z);
     minx = Math.min(ST.P1.x,Math.min(ST.P2.x,ST.P3.x));
     miny = Math.min(ST.P1.y,Math.min(ST.P2.y,ST.P3.y));
     minz = Math.min(z1,Math.min(z2,z3));
     maxx = Math.max(ST.P1.x,Math.max(ST.P2.x,ST.P3.x));
     maxy = Math.max(ST.P1.y,Math.max(ST.P2.y,ST.P3.y));
     maxz = Math.max(z1,Math.max(z2,z3));
     if(i==0)
        BB.set(minx,miny,minz,maxx,maxy,maxz);
     else{
        BB2.set(minx,miny,minz,maxx,maxy,maxz);
        BB.extend(BB2);
     }



    P3D1 = new Point3D(ST.P1.x,ST.P1.y,ST.P1.z*ScaleFactor,getR(ST.P1.z),getG(ST.P1.z),getB(ST.P1.z));
    P3D2 = new Point3D(ST.P2.x,ST.P2.y,ST.P2.z*ScaleFactor,getR(ST.P2.z),getG(ST.P2.z),getB(ST.P2.z));
    P3D3 = new Point3D(ST.P3.x,ST.P3.y,ST.P3.z*ScaleFactor,getR(ST.P3.z),getG(ST.P3.z),getB(ST.P3.z));
    T3D = new Triangle3D(P3D1,P3D2,P3D3,myID);
    Triangles.append(T3D);
  }
}

public String toString(){return Name;}

public BoundingBox3D getBoundingBox(){ return BB;}


private Triangle3DVector Triangles = new Triangle3DVector();
private Vector SingleTriangles = new Vector();
private double ScaleFactor;
private int minR=0,minG=0,minB=0,maxR=255,maxG=255,maxB=255;  // later to change
private ID myID = IDManager.getNextID();
private String Name="";
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
      computeTriangles3D();
  }
}


public boolean nearByXY(double x, double y, double exactness){
  double e2 = exactness*exactness;
  boolean found=false;
  Triangle3D T;
  for(int i=0;i<Triangles.getSize()&&!found;i++){
     T=Triangles.get(i);
     if(nearByXY(T,x,y,e2))
        found=true;
  }
  return found;
}


private static boolean nearByXY(Triangle3D T,double x, double y, double square_exactness){
  Point3D P1 = T.getCP1(),
          P2 = T.getCP2(),
          P3 = T.getCP3();
  if( FLine3D.nearByXY(P1,P2,x,y,square_exactness) || FLine3D.nearByXY(P1,P3,x,y,square_exactness) ||
      FLine3D.nearByXY(P2,P3,x,y,square_exactness))
     return true;

  double x1 = P1.getX(), y1=P1.getY(),
         x2 = P2.getX(), y2=P2.getY(),
         x3 = P3.getX(), y3=P3.getY();

  // check if P in T
  // algorithm see: http://mcraefamily.com/MathHelp/GeometryPointAndTriangle3.htm
  double AB = (y-y1)*(x2-x1)-(x-x1)*(y2-y1);
  double CA = (y-y3)*(x1-x3)-(x-x3)*(y1-y3);
  double BC = (y-y2)*(x3-x2)-(x-x2)*(y3-y2);
  return (AB*BC>0) && (BC*CA)>0;
}



private class SingleTriangle{

SingleFPoint P1,P2,P3;

public boolean readFromListExpr(ListExpr LE){
  if(LE.listLength()!=3)
     return false;
  SingleFPoint tmpP1 = new SingleFPoint();
  SingleFPoint tmpP2 = new SingleFPoint();
  SingleFPoint tmpP3 = new SingleFPoint();
  boolean ok =  tmpP1.readFromListExpr(LE.first());
  if(!ok)
     System.out.println("Error reading :"+LE.first().writeListExprToString());
  else{
     ok = tmpP2.readFromListExpr(LE.second());
     if(!ok)
        System.out.println("Error reading :"+LE.second().writeListExprToString());
     else {
        ok = tmpP3.readFromListExpr(LE.third());
        if(!ok)
	  System.out.println("error reading :"+LE.third().writeListExprToString());
      }
  }

  if(ok){
     this.P1 = tmpP1;
     this.P2 = tmpP2;
     this.P3 = tmpP3;
  }
  return ok;
}

}
}

