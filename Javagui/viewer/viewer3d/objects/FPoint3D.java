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
  ListExpr Points = LE.second();
  SingleFPoint P;
  boolean ok = true; 
  while( !Points.isEmpty() & ok) {
    P = new SingleFPoint();
    if(P.readFromListExpr(Points.first())){
       SingleFPoints.add(P);
       Points=Points.rest();
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

      P3D = new IDPoint3D(P.x*ScaleFactor,P.y*ScaleFactor,
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
  if (FS.getReturnValue()==FS.OK){
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

private IDPoint3DVector Points = new IDPoint3DVector();
private Vector SingleFPoints = new Vector();
private double ScaleFactor;
private int minR=0,minG=0,minB=0,maxR=255,maxG=255,maxB=255;
private ID myID= IDManager.getNextID();
private String Name;
private BoundingBox3D BB= new BoundingBox3D();;
}


