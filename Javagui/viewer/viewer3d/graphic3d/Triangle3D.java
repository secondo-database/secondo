package viewer.viewer3d.graphic3d;

import java.awt.Color;
import gui.idmanager.*;
import viewer.viewer3d.graphic2d.Point2D;
import viewer.viewer3d.graphic2d.Triangle2D;
import viewer.viewer3d.objects.BoundingBox3D;

/***********************************
*
* Autor   : Thomas Behr
* Version : 1.1
* Datum   : 16.5.2000
*
*************************************/



public class Triangle3D {

/** the ID of this triangle */
private   ID         MyID;
/** the cornerpoints */
private   Point3D[]  Points = new Point3D[3];

/** returns a readable representation of this triangle */
public String toString() {
  String S = "Triangle3D ";
  for(int i=0;i<3;i++) { S += Points[i]; }
  return S;
}


/** creates a new Triangle */
public Triangle3D( Point3D  P1, Point3D P2, Point3D P3){
   Points[0] = P1;
   Points[1] = P2;
   Points[2] = P3;
   MyID = IDManager.getNextID();
}

public Triangle3D(Point3D P1,Point3D P2, Point3D P3,ID aID){
   Points[0] = P1;
   Points[1] = P2;
   Points[2] = P3;
   MyID = aID;
}


/** returns a copy of this triangle */
public Triangle3D duplicate() {
  return new Triangle3D(Points[0].duplicate(),Points[1].duplicate(),
                        Points[2].duplicate(),MyID);
}


/** returns the BoundingBox of this Triangle*/
public BoundingBox3D getBoundingBox(){
  BoundingBox3D BB3 = new BoundingBox3D();
  double xmin,xmax,ymin,ymax,zmin,zmax;
  xmin=xmax=Points[0].getX();
  ymin=ymax=Points[0].getY();
  zmin=zmax=Points[0].getZ();
  for(int i=1;i<3;i++){
      if(xmin>Points[i].getX())
         xmin=Points[i].getX();
      if(xmax<Points[i].getX())
         xmax=Points[i].getX();
      if(ymin>Points[i].getY())
         ymin=Points[i].getY();
      if(ymax<Points[i].getY())
         ymax=Points[i].getY();
      if(zmin>Points[i].getZ())
         zmin=Points[i].getZ();
      if(zmax<Points[i].getZ())
         zmax=Points[i].getZ();
  }
  BB3.set(xmin,ymin,zmin,xmax,ymax,zmax);
  return BB3;
}

/** equalize this to Source */
public void equalize(Triangle3D Source) {
  this.MyID.equalize(Source.MyID);
  for(int i=0;i<3;i++)
     this.Points[i].equalize(Source.Points[i]);
}


/** check for equality of the values (not ID) */
public boolean equalValues(Triangle3D D2) {

  boolean result=true;
  
  for(int i=0; i<3 ; i++)      // equal Points
    result = result && Points[i].equals(D2.Points[i]);
 return result;
}


/** check for equality */
public boolean equals(Triangle3D Q2) {
  return MyID.equals(Q2.MyID) && this.equalValues(Q2);
}

/** check for equality of the ID */
public boolean equalID(Triangle3D Q2) {
  return MyID.equals(Q2.MyID);
}

/** set the ID of this triangle */
public void setID(ID QID) { MyID.equalize(QID); }

/** get the ID of this triangle */
public ID getID() { return MyID; }

/** creates a figure3D from this triangle */
public Figure3D getFigure3D() {

  Figure3D Fig = new Figure3D();
  Fig.setID(MyID);
  Fig.addPoint(Points[0]);
  Fig.addPoint(Points[1]);
  Fig.addPoint(Points[2]);
  return Fig;
}

/** returns the projection of this triangle */
public Triangle2D  project() {

  Point2D[] pts2d = new Point2D[3];

  for(int i=0;i<3;i++)
     pts2d[i] = Points[i].project();

  return new Triangle2D(pts2d[0],pts2d[1],pts2d[2]);
}

/** returns a cornerpoint of this triangle */
public Point3D getCP1(){ return Points[0]; }
/** returns a cornerpoint of this triangle */
public Point3D getCP2(){ return Points[1]; }
/** returns a cornerpoint of this triangle */
public Point3D getCP3(){ return Points[2]; }

/** set a cornerpoint of this triangle */
public void setCP1(Point3D P){Points[0] = P; }
/** set a cornerpoint of this triangle */
public void setCP2(Point3D P){Points[1] = P; }
/** set a cornerpoint of this triangle */
public void setCP3(Point3D P){Points[2] = P; }


} // class Triangle3D













