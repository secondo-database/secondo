package viewer.viewer3d.objects;

import viewer.viewer3d.graphic3d.*;
import sj.lang.ListExpr;
import gui.idmanager.*;
import gui.SecondoObject;
import java.awt.*;

public interface Object3D{

/** returns all containing triangles of this object */
public Triangle3DVector getTriangles();
/** returns all containing lines of this object */
public Line3DVector getLines();
/** returns the containing points of this object */
public IDPoint3DVector getPoints();
/** convert SO to a Object3D */
public boolean readFromSecondoObject(SecondoObject SO);
/** return the ID of this Object */
public ID getID();
/** shows a setting dialog of this object */
public void showSettings(Frame F);
/** returns the bounding box */
public BoundingBox3D getBoundingBox();

/** returns true if a vertical line in (x,y) is in the near of this object
  * the maximal distance is given by exactness
  */
public boolean nearByXY(double x,double y,double exactness);

}


