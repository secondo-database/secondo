package viewer.viewer3d.graphic3d;

import gui.idmanager.*;
import java.awt.Color;
import viewer.viewer3d.graphic2d.*;
import viewer.viewer3d.objects.BoundingBox3D;

/** this class provides a 3dim point with a own ID */
public class IDPoint3D extends Point3D{

/** creates an new point */
public IDPoint3D(double x,double y,double z, int r, int g, int b){
  super(x,y,z,r,g,b);
  MyID = IDManager.getNextID();
}

/** creates a new point */
public IDPoint3D(double x,double y, double z, Color C){
  super(x,y,z,C);
  MyID = IDManager.getNextID();
}


public BoundingBox3D getBoundingBox(){
  BoundingBox3D BB3 = new BoundingBox3D();
  BB3.set(x,y,z,x,y,z);
  return BB3;
}



/** creates a new point */
public IDPoint3D(double x, double y, double z,int r, int g, int b, ID aID){
  super(x,y,z,r,g,b);
  MyID = aID;
}

/** returns the projection of this point */
public Point2D project(){
   IDPoint2D P= new IDPoint2D(x,y,cr,cg,cb);
   P.setID(MyID);
   return  P;
}


/** creates a figure from this point */
public Figure3D getFigure3D() {
  Figure3D Fig = new Figure3D();
  Fig.addPoint(this);
  return Fig;
}

/** get the ID of this point */
public ID getID() { return MyID;}
/** set the ID of this point */
public void setID(ID newID){ MyID.equalize(newID);}


/** check for equality of Position and Color (not ID) **/
public boolean equalValues(IDPoint3D P){
  return ( (x==P.x) && (y==P.y) && (z==P.z) &&
           (cr==P.cr) && (cg==P.cg) && (cb==P.cb));

}


/** the ID of this point */
private ID MyID;


}



