package viewer.viewer3d.objects;

import viewer.viewer3d.graphic3d.*;
import sj.lang.ListExpr;
import gui.idmanager.*;
import gui.SecondoObject;
import java.awt.*;

public interface Object3D{

public Triangle3DVector getTriangles();
public Line3DVector getLines();
public IDPoint3DVector getPoints();
public boolean readFromSecondoObject(SecondoObject SO);
public ID getID();

public void showSettings(Frame F);
public BoundingBox3D getBoundingBox();

}