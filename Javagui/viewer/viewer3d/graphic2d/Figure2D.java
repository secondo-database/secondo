package viewer.viewer3d.graphic2d;

import gui.idmanager.*;

/*************************
* Autor : Thomas Behr
* Version 1.1
* Datum : 16.5.2000
**************************/

import java.awt.Color;

/**
 * this class provide 2dim figures (polygons,lines or points)
 */

public class Figure2D {

 /** the id of this figure */
 private   ID              MyID;
 /** the content of this figure */
 private   Point2DVector   Points;
 /** a value for ordering figures */
 private   double          Sort; 
  
 /** creates a new empty figure */
 public Figure2D() {
     Points = new Point2DVector();
     MyID = IDManager.getNextID();
     Sort = 0.0;
  }

  /** check for emptyness */
  public boolean isEmpty() { return Points.isEmpty(); }

  /** set the content  from this to content of Source */
  public void Equalize(Figure2D Source) {
     Points.Equalize(Source.Points);
     Sort = Source.Sort;
     MyID     = Source.MyID.Duplicate();
  }

  /** check for equality */
  public boolean equals(Figure2D Fig2) {
    return Points.equals(Fig2.Points) && 
           (Sort==Fig2.Sort) &&
           MyID.equals(Fig2.MyID);
  }

 
 /** returns a copy from this */
 public Figure2D Duplicate() {
    Figure2D Copy = new Figure2D();
    Copy.Equalize(this);
    return Copy;
  }

 /** set the id of this */
 public void setID(ID newID) { MyID.equalize(newID); }

 /** add a new point */
 public void addPoint(Point2D Pt)
  { Points.append(Pt.duplicate()); }

 /** add a new Point given by (x,y,C) */
 public void addPoint(double x,double y,Color C) {
     Points.append( new Point2D(x,y,C));
 }

 /** return the ID */
 public ID getID() { return MyID; }

 /** returns number of containing points */
 public int getSize() { return Points.getSize(); }

 /** set the sort-value */
 public void setSort(double S) { Sort = S; }

 /** get the sort-value */
 public double getSort() { return Sort; }

 /** returns a readable representation of this */
 public String toString() {
   String S;
   S = "Figure2D : ";
   for (int i=0; i<Points.getSize(); i++ )
      { S += Points.getPoint2DAt(i) + "  "; }
   return S;
 }

 /** check whether this is a point */
 public boolean isPoint() {
    return Points.getSize()==1;
 }

 /** check wether this is a line */
 public boolean isLine(){
    return Points.getSize()==2;
 }
 
 /** check wether this is a triangle */
 public boolean isTriangle() {
    return Points.getSize()>2;
 }

 /**
   * creates a IDPoint from this;
   * isPoint must be true
   */
 public IDPoint2D getPoint() {
 // only invoke, if isPoint==true !!
   Point2D P = Points.getPoint2DAt(0);
   IDPoint2D IDP = new IDPoint2D(P);
   IDP.setID(MyID);
   return IDP;
 }

 /**
  * creates a Line from this;
  * isLine must be true
  */
 public Line2D getLine(){
    Line2D L = new Line2D(Points.getPoint2DAt(0),Points.getPoint2DAt(1));
    L.setID(MyID);
    return L;
 }


 /**
  * returns a set of triangles;
  * isTriangle must be true
  */
 public Triangle2D[]  getTriangles() {

  Triangle2D[] Trs = new Triangle2D[Points.getSize()-2];

  if (Points.getSize()==3) {
      Trs[0] = new Triangle2D( Points.getPoint2DAt(0),
                                    Points.getPoint2DAt(1),
                                    Points.getPoint2DAt(2));
   }
   else {
     int i;
     for( i=0; i< Points.getSize()-2;i++)
        Trs[i] = new Triangle2D( Points.getPoint2DAt(0),
                                      Points.getPoint2DAt(i+1),
                                      Points.getPoint2DAt(i+2));

    
   }

   for(int i=0;i<Trs.length;i++)
     Trs[i].setID(MyID);
   return Trs;
}


} // class 
