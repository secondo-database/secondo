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

import java.io.*;
//import java.util.*;

abstract public class Element implements Serializable{
  //Abstract class for all geometric objects, particularly Point, Segment,
  //Triangle, Polygon and all user-defined objects

  //It lists all methods which have to be implemented
  //by all classes which define primitives, it is especially
  //important for the implementation of new primitives
  //since it shows a complete list of essential 
  //methods for that new primitive.


  //members
  //public double area = 0;
  //public double perimeter = 0;
  //public Rect bbox = new Rect(); //bounding box

  //constructors
  //Element(LinkedList vertices);
  //creates a new object with given name and given vertices


  //methods
  abstract Element copy();
  //returns a deep copy of THIS.object as new object

 
  abstract boolean equal(Element inObject) throws WrongTypeException;
  //returns TRUE, if the given object and THIS.object are congruent
  
  abstract byte compX(Element inObject) throws WrongTypeException;
  //compares the x-coordinates of the leftmost points of the
  //given object and THIS.object and returns
  //-1, if THIS.object has a smaller x-coordinate
  //0, if the x-coordinates are equal
  //+1, if THIS.object has a greater x-coordinate

  abstract byte compY(Element inObject) throws WrongTypeException;
  //similar to compx but uses y-coordinates

    abstract byte compare(Element inObject) throws WrongTypeException;
    //uses both, compX and compY

  //void set(String name; LinkedList vertices);
  //sets THIS.name to name and the THIS.object's vertices to
  //this vertices

  //abstract void update();
  //forces a new computation of class members, e.g. area and perimeter

  //double compute_perimeter();
  //returns the perimeter of THIS.object

  //double compute_area();
  //returns the area of THIS.object

  //String isA();
  //returns class name of THIS.object as String

  //boolean isValid();
  //TRUE, if THIS.object is valid due to defined (user-)restriction

  //boolean buildOfTriangles();
  //TRUE, if THIS.object may be parted to a set of triangles

  //LinkedList getTriangles();
  //returns a LinkedList of triangles forming THIS.object, if
  //buildOfTriangles returns TRUE
  //all elements of LinkedList must be of type Triangle

  //abstract void computeBbox();
  //computes the bounding box and sets bbox

  abstract void print();
  //prints the elements data to text window

  abstract boolean intersects(Element inObject) throws WrongTypeException;
  //returns true if this.object an in intersect
  //false else

    abstract Rect rect();
    //returns the bounding box of this.object

    abstract Rational dist(Element inObject) throws WrongTypeException;
    //returns the distance between two Elements of the same type

}//end class Element
