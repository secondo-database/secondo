package viewer.viewer3d.graphic3d;

import java.awt.Color;
import gui.idmanager.*;
import viewer.viewer3d.graphic2d.Point2D;
import viewer.viewer3d.graphic2d.Line2D;
import viewer.viewer3d.objects.BoundingBox3D;

/***********************************
*
* Autor   : Thomas Behr
* Version : 1.0
* Datum   : 3.7.2000
*
*************************************/



public class Line3D {

/** a endpoint */
private   Point3D    EP1,EP2;                           // endpoints
/** a empty-line ? */
private   boolean    empty;
/** the ID */
private   ID         MyID;

/** returns a readable representation of this point */
public String toString() {
 return  "Line3d : " + EP1 + EP2;
 }

/** creates a new line by given endpoints */
public Line3D( Point3D  P1, Point3D P2){
   EP1 = P1.duplicate();
   EP2 = P2.duplicate();
   empty = false;
   MyID = IDManager.getNextID();
  }

public Line3D(Point3D P1, Point3D P2,ID aID){
  EP1 = P1.duplicate();
  EP2 = P2.duplicate();
  empty = false;
  MyID = aID;
}


/** returns a copy of this */
public Line3D duplicate() {
  return new Line3D(EP1,EP2,MyID);
}

/** equalize this to Source */
public void equalize(Line3D Source) {
  this.EP1.equalize(Source.EP1);
  this.EP2.equalize(Source.EP2);
  this.empty = Source.empty;
}

/** check for equal position and color (not needed ID) */
public boolean equalValues(Line3D D2) {
 return EP1.equals(D2.EP1) && EP2.equals(D2.EP2) && empty==D2.empty;
}

/** returns the Bounding Box of this Line */
public BoundingBox3D getBoundingBox(){
  BoundingBox3D BB3 = new BoundingBox3D();
  double x1 = EP1.getX(),
         x2 = EP2.getX(),
         y1 = EP1.getY(),
         y2 = EP2.getY(),
         z1 = EP1.getZ(),
         z2 = EP2.getZ();
  BB3.set(Math.min(x1,x2),Math.min(y1,y2),Math.min(z1,z2),
          Math.max(x1,x2),Math.max(y1,y2),Math.max(z1,z2));
  return BB3;
}


/** check for equality with Q2  */
public boolean equals(Line3D Q2) {
  return EP1.equals(Q2.EP1) && EP2.equals(Q2.EP2) && empty==Q2.empty &&
         MyID.equals(Q2.MyID);
}

/** check for equality of the ID's */
public boolean equalID(Line3D Q2) {
  return  MyID.equals(Q2.MyID);
}

/** set the ID */
public void setID(ID QID)
   { MyID.equalize(QID);
   }

/** get the ID */
public ID   getID()       { return MyID; }


/** get the projection of this line */
public Line2D  project() {
   Line2D L;
   Point2D   Pep1 = EP1.project();
   Point2D   Pep2 = EP2.project();
   L = new Line2D(Pep1,Pep2);
   L.setID(MyID);
   L.setEmpty(empty);
   return L;
}

/** set this line to be empty */
public void setEmpty(boolean e){ empty = e; }
/** is this a empty-line */
public boolean isEmpty() { return empty; }


/** creates a figure3D from this line */
public Figure3D getFigure3D() {
  Figure3D Fig = new Figure3D();
  Fig.addPoint(EP1);
  Fig.addPoint(EP2);
  Fig.setID(MyID);
  return Fig;
}


/** returns a endpoint of this line */
public Point3D getEP1() { return EP1.duplicate(); }
/** returns a endpoint of this line */
public Point3D getEP2() { return EP2.duplicate(); }

/** set a endpoint of this line */
public void setEP1(Point3D P) { EP1.equalize(P); }
/** set a endpoint of this line */
public void setEP2(Point3D P) { EP2.equalize(P); }

} // class Line3D





