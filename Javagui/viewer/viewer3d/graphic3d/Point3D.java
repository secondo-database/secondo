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

package viewer.viewer3d.graphic3d;

import gui.idmanager.*;
import java.awt.Color;
import viewer.viewer3d.graphic2d.*;

/** this class provides a 3dim point with a own ID */
public class Point3D extends Figure3D{
  
  Point3DSequence s;



public Point3D(double x,double y,double z, int r, int g, int b,ID aID){
  s = new Point3DSequence();
  s.addPoint(new Point3DSimple(x,y,z,r,g,b)); 
  myID = IDManager.getNextID(); 
  myID.equalize(aID);
}


/** creates an new point */
public Point3D(double x,double y,double z, int r, int g, int b){
   this(x,y,z,r,g,b,IDManager.getNextID());
}

public Figure3D copy(){
   return  new Point3D(getX(),getY(),getZ(),getColor(),myID);
}


public Point3DSimple getLocation(){
   return s.getPoint3DAt(0).duplicate();
}


public Point3D(double x, double y, double z, Color c, ID aID){
   this(x,y,z,c.getRed(),c.getGreen(),c.getBlue());
   myID.equalize(aID);
}

public double getX(){ return s.getPoint3DAt(0).getX(); }

public double getY(){ return s.getPoint3DAt(0).getY(); }

public double getZ(){ return s.getPoint3DAt(0).getZ(); }

public Color getColor(){ return s.getPoint3DAt(0).getColor(); }

/** creates a new point */
public Point3D(double x,double y, double z, Color C){
   this(x,y,z,C.getRed(),C.getGreen(),C.getBlue());
}


public BoundingBox3D getBoundingBox(){
  BoundingBox3D BB3 = new BoundingBox3D();
  Point3DSimple p = s.getPoint3DAt(0);
  BB3.set(p.getX(),p.getY(),p.getZ(),p.getX(),p.getY(),p.getZ());
  return BB3;
}

/** returns the projection of this point */
public Figure2D project(FM3DGraphic fm){
   Point2DSequence s2 = fm.figureTransformation(s);
   if(s2.isEmpty()){ // a victim of clipping
      return null;
   }

   Point2D p2 = s2.getPoint2DAt(0);
   IDPoint2D ip2 = new IDPoint2D(p2,myID);
   ip2.setSort(s2.getSort()); 
   return ip2;
}


public Point3D duplicate(){
  return new Point3D(getX(),getY(),getZ(),getColor(),myID);
}


public void moveTo(double x, double y,double z){
    Point3DSimple p = s.getPoint3DAt(0);
    p.moveTo(x,y,z);
    s.setPoint3DAt(p,0);
}

}



