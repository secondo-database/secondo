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
   return readFromListExpr(LE.second());
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

  ListExpr Triangles = LE.second();
  SingleTriangle T;
  boolean ok = true; 
  while( !Triangles.isEmpty() & ok) {
    T = new SingleTriangle();
    if(T.readFromListExpr(Triangles.first())){
       SingleTriangles.add(T);
       Triangles=Triangles.rest();
    }
    else
       ok = false;

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



private class SingleTriangle{

SingleFPoint P1,P2,P3;

public boolean readFromListExpr(ListExpr LE){
  if(LE.listLength()!=3)
     return false;
  SingleFPoint P1 = new SingleFPoint();
  SingleFPoint P2 = new SingleFPoint();
  SingleFPoint P3 = new SingleFPoint();
  if(!( P1.readFromListExpr(LE.first()) && P2.readFromListExpr(LE.second()) &&
        P3.readFromListExpr(LE.third())))
     return false; 
   
  this.P1 = P1;
  this.P2 = P2;
  this.P3 = P3;
  return true;
}

}
}