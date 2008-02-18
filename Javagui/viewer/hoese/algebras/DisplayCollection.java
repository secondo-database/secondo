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

package  viewer.hoese.algebras;

import  java.awt.geom.*;
import  java.awt.*;
import  viewer.*;
import viewer.hoese.*;
import  sj.lang.ListExpr;
import  javax.swing.border.*;
import  java.util.*;
import  javax.swing.*;
import tools.Reporter;
import java.lang.reflect.*;


/**
 * The base class for Collection Objects
 */
public abstract class DisplayCollection extends DsplGeneric implements Timed,DsplGraph,RenderAttribute,LabelAttribute,DisplayComplex
     {
/** The subtype of the collection */
protected String subtyp;

/** A structur to store the instances of the subtypes display classes */
protected Vector graphVector = new Vector();

/** A structure to store the amount of shapes per object */
protected Vector shapeVector = new Vector();  

/** The layer in which this object is drawn */
protected Layer RefLayer;

/** The category of this object preset by the defaultcategory */
protected Category Cat = Category.getDefaultCat();

   
/** A method to determine wether the given ListExpr is a collection
 * @return The Vector representation of the time intervals this instance is defined at
 * @see <a href="DisplayCollectionsrc.html#isCollection">Source</a>
 */
public boolean isCollection(ListExpr theList){
  if(theList.first().isAtom())
    if(theList.first().symbolValue().equals("vector") || theList.first().symbolValue().equals("set") || theList.first().symbolValue().equals("multiset")) return true;
    else return false;
  return true;
}

/** A method to return the number of stored objects in the collection 
 * @return the number of objects
 * @see <a href="DisplayCollectionsrc.html#numberOfObjects">Source</a>
 */
public int numberOfObjects(ListExpr value){
  return value.listLength();
}

/** A method to determine wether the subtype is an instance of DsplGraph 
 * @return boolean result of the determination
 * @see <a href="DisplayCollectionsrc.html#numberOfObjects">Source</a>
 */
public boolean isGraph(String typename){
  try{
  Class klasse=Class.forName("viewer.hoese.algebras."+"Dspl"+typename);
  Object o = klasse.newInstance();
  if(o instanceof DsplGraph){
   return true;
  }else{
   return false;
  }
  }catch(ClassNotFoundException e){
   Reporter.showError(" Keine Displayklasse gefunden");
   }catch(InstantiationException e){
   Reporter.showError("Displayklasse konnte nicht initialisiert werden");
  }catch(IllegalAccessException e){
   Reporter.showError("Fehler beim Zugriff auf die Displayklasse");
  }
 return false;
}

/** A method to return the typename of the subtype
 * @return the subtype as an ListExpr object
 * @see <a href="DisplayCollectionsrc.html#getTypeName">Source</a>
 */
public ListExpr getTypeName(ListExpr type){
ListExpr typename=null;
 while(!type.isEmpty()){
	if(!isCollection(type) && type.first().isAtom())typename=type.first();
	if(type.first().isAtom()){
	type = type.rest();
	}else{
	type = type.first();
	}
   }
 return typename;
}

/** A method to find a specific shape in the shapeVector
 * @return the position in the shaoeVector
 * @see <a href="DisplayCollectionsrc.html#numberOfObjects">Source</a>
 */
public int findShape(int num){
  for(int i=0;i<shapeVector.size();i++){
    if(((shapeSet)shapeVector.get(i)).hasNumber(num))return i;
  }
  return 0;
}


//timed
/** A method of the Timed-Interface
 * @return the panel with the graphical representation of a temporal datatype		
 * @see <a href="DisplayCollectionsrc.html#getTimeRenderer">Source</a>
 */
public JPanel getTimeRenderer (double PixelTime) {
JPanel pan = new JPanel();
if(isGraph(subtyp)){
 if(graphVector.firstElement() instanceof Timed){
  if(graphVector.size()==1)
    return ((Timed)graphVector.get(0)).getTimeRenderer(PixelTime);
  }
}
return pan;
}




/** A method of the Timed-Interface
 * @return The Vector representation of the time intervals this instance is defined at
 * @see <a href="DisplayCollectionsrc.html#getIntervals">Source</a>
 */
public Vector getIntervals(){
if(isGraph(subtyp)){
  Vector intervalVektor=new Vector();
  if(graphVector.firstElement() instanceof Timed){
    for(int j=0;j<graphVector.size();j++){
      Vector v=((Timed)graphVector.get(j)).getIntervals();
	for(int i=0;i<v.size();i++){
	  intervalVektor.add(v.get(i));
	} 
    }
    return intervalVektor;
  }
}
return null;
}

/** A method of the Timed-Interface
 * @return the global time boundaries [min..max] this instance is defined at
 * @see <a href="DisplayCollectionsrc.html#getBoundingInterval">Source</a>
 */
public Interval getBoundingInterval () {
  if(isGraph(subtyp)){
    Interval intV=null;
    if(graphVector.firstElement() instanceof Timed){
       for(int j=0;j<graphVector.size();j++){
         if(intV==null){
	  intV=((Timed)graphVector.get(j)).getBoundingInterval();
	 }else{
	  intV = intV.union(((Timed)graphVector.get(j)).getBoundingInterval());
	 }
       }
    return intV;
    }
  }
  return null;
}
//ende timed

//begin renderAttribute
/** A method of the RenderAttribute-Interface
 * @return return whether this objects is defined at any time
 * @see <a href="DisplayCollectionsrc.html#mayBeDefined">Source</a>
 */
public boolean mayBeDefined(){
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof RenderAttribute){
      return  (((RenderAttribute)graphVector.get(0)).mayBeDefined());
    }
  }
  return false; 
}

/** A method of the RenderAttribute-Interface
 * @return returns the minimum value of this attribute
 * @see <a href="DisplayCollectionsrc.html#getMinRenderValue">Source</a>
 */  
public double getMinRenderValue(){
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof RenderAttribute){
      return  (((RenderAttribute)graphVector.get(0)).getMinRenderValue());
    }
  }
  return 1.0;
}

/** A method of the RenderAttribute-Interface
 * @return returns the maximum value of this attribute
 * @see <a href="DisplayCollectionsrc.html#getMaxRenderValue">Source</a>
 */   
public double getMaxRenderValue(){
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof RenderAttribute){
      return  (((RenderAttribute)graphVector.get(0)).getMaxRenderValue());
    }
  }
  return 1.0;
}

/** A method of the RenderAttribute-Interface
 * @return returns the defined state at the given time
 * @see <a href="DisplayCollectionsrc.html#isDefined">Source</a>
 */ 
public boolean isDefined(double time){
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof RenderAttribute){
      return  (((RenderAttribute)graphVector.get(0)).isDefined(time));
    }
  }
  return false;
} 

/** A method of the RenderAttribute-Interface
 * @return returns the value of this attribute for the given time
 * @see <a href="DisplayCollectionsrc.html#getRenderValue">Source</a>
 */ 
public double getRenderValue(double time){
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof RenderAttribute){
      return  (((RenderAttribute)graphVector.get(0)).getRenderValue(time));
    }
  }
  return 1.0;
}
//end renderAttribute

//begin dsplGraph
/**
 * Tests if a given position is contained in the RenderObject.
 * @param xpos The x-Position to test.
 * @param ypos The y-Position to test.
 * @param scalex The actual x-zoomfactor
 * @param scaley The actual y-zoomfactor
 * @return true if x-, ypos is contained in this points type
 * @see <a href="DisplayCollection.html#contains">Source</a>
 */
public boolean contains (double xpos, double ypos, double scalex, double scaley) {
  boolean result=false;
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof DsplGraph){
     for(int j=0;j<graphVector.size();j++){
        result=result || (((DsplGraph)graphVector.get(j)).contains(xpos,ypos,scalex,scaley));
     }
    }
  }
  return result;
}

/**
 * A method of the DsplGraph-Interface
 * @return The boundingbox of the drawn Shape
 * @see <a href="DisplayGraphsrc.html#getBounds">Source</a>
 */
public Rectangle2D.Double getBounds () {
  Rectangle2D.Double r = null;
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof Timed){
      for(int j=0;j<graphVector.size();j++){
        if(r==null){
	  r=((DsplGraph)graphVector.get(j)).getBounds();
        }else{
	  r=(Rectangle2D.Double)r.createUnion(((DsplGraph)graphVector.get(j)).getBounds());
}
      }
      return r;
    }
  }
  int num = numberOfShapes();
   for(int i=0;i<num;i++){
     Shape shp = getRenderObject(i,new AffineTransform());
     if(shp!=null){
       Rectangle2D b = shp.getBounds2D();
       if(r==null){
         r = new Rectangle2D.Double(b.getX(),b.getY(),b.getWidth(),b.getHeight());
       } else {
         r.add(b);
       }
      }
    }
  return r;
}

/**
 * A method of the DsplGraph-Interface
 * @return The position of the label as a Point
 * @see <a href="DisplayGraphsrc.html#getLabPosOffset">Source</a>
 */
public Point getLabPosOffset () {
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof DsplGraph){
      return (((DsplGraph)graphVector.get(0)).getLabPosOffset());
      
    }
  }
  return new Point(0,0);
}

/**
 * A method of the DsplGraph-Interface
 * Sets the position of the label
 * @param pt The position for the label
 * @see <a href="DisplayCollection.html#setLabPosOffset">Source</a>
 */
public void setLabPosOffset (Point pt) {
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof DsplGraph){
      ((DsplGraph)graphVector.get(0)).setLabPosOffset(pt);
    }
  } 
}

/**
 * A method of the DsplGraph-Interface
 * Sets the label Attribute
 * @see <a href="DisplayCollection.html#setLabelAttribute">Source</a>
 */
public void setLabelAttribute(LabelAttribute labelAttribute) {
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof DsplGraph){
      for(int j=0;j<graphVector.size();j++){
        ((DsplGraph)graphVector.get(j)).setLabelAttribute(labelAttribute);
      }
    }
  }
}

/**
 * A method of the DsplGraph-Interface
 * Sets the render Attribute
 * @see <a href="DisplayCollection.html#setRenderAttribute">Source</a>
 */
public void setRenderAttribute(RenderAttribute renderAttribute){
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof DsplGraph){
      for(int j=0;j<graphVector.size();j++){
      ((DsplGraph)graphVector.get(j)).setRenderAttribute(renderAttribute);
      }
    }
  }
}

/**
 * A method of the DsplGraph-Interface
 * @return the render attribute
 * @see <a href="DisplayCollection.html#getRenderAttribute">Source</a>
 */
public RenderAttribute getRenderAttribute(){
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof DsplGraph){
      return (((DsplGraph)graphVector.get(0)).getRenderAttribute());
    }
  }
  return null;
}

/**
 * A method of the DsplGraph-Interface
 * Checks whether the Shape with the given number should be displayed 
 * as a point.
 * @see <a href="DisplayCollection.html#isPointType">Source</a>
 */ 
public boolean isPointType (int num) {
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof DsplGraph){
      return (((DsplGraph)graphVector.get(0)).isPointType(num));
    }
  }
  return false;
}

/**
 * A method of the DsplGraph-Interface
 * Checks whether the Shape with the given number should be displayed 
 * as a line.
 * @see <a href="DisplayCollection.html#isLineType">Source</a>
 */ 
public boolean isLineType(int num){
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof DsplGraph){
      return (((DsplGraph)graphVector.get(0)).isLineType(num));
    }
  }
  return false;
}

/**
 * A method of the DsplGraph-Interface
 * @return The text of the label.
 * @see <a href="DisplayCollection.html#getLabelText">Source</a>
 */
public String getLabelText (double time) {
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof DsplGraph){
      return (((DsplGraph)graphVector.get(0)).getLabelText(time));
    }
  }
  return null;
}

/**
 * A method of the DsplGraph-Interface
 * @return the label attribute.
 * @see <a href="DisplayCollection.html#getLabelAttribute">Source</a>
 */
public LabelAttribute getLabelAttribute(){
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof DsplGraph){
      return (((DsplGraph)graphVector.get(0)).getLabelAttribute());
    }
  }
  return null;
}

/**
 * A method of the DsplGraph-Interface
 * Sets the category of this object.
 * @param acat A category-object
 * @see <a href="DisplayCollection.html#setCategory">Source</a>
 */
public void setCategory (Category acat) {
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof DsplGraph){
	for(int j=0;j<graphVector.size();j++){
          ((DsplGraph)graphVector.get(j)).setCategory(acat);
        }
    }
  }
  Cat = acat;
}

/**
 * A method of the DsplGraph-Interface
 * @return The actual category of this instance.
 * @see <a href="DisplayCollection.html#getCategory">Source</a>
 */
public Category getCategory () {
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof DsplGraph){
      return (((DsplGraph)graphVector.get(0)).getCategory());
    }
  }
  return  Cat;
}

/**
 * A method of the DsplGraph-Interface
 * Sets the layer of this instance.
 * @param alayer A layer to which this graphic object belong.
 * @see <a href="DisplayCollection.html#setLayer">Source</a>
 */
public void setLayer (Layer alayer) {
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof DsplGraph){
     for(int j=0;j<graphVector.size();j++){
       ((DsplGraph)graphVector.get(j)).setLayer(alayer);
     }
    }
  }    
  RefLayer = alayer;
}

/**
 * A method of the DsplGraph-Interface
 * @return The layer of this object
 * @see <a href="DisplayCollection.html#getLayer">Source</a>
 */
public Layer getLayer () {
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof DsplGraph){
      return (((DsplGraph)graphVector.get(0)).getLayer());
    }
  }
  return  RefLayer;
}

/**
 * A method of the DsplBase-Interface
 * @return The name of the Attribute
 * @see <a href="DisplayCollection.html#getAttrName">Source</a>
 */
public String getAttrName(){
 if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof DsplGraph){
	System.out.println("attrName: "+(((DsplGraph)graphVector.get(0)).getAttrName()));
      return (((DsplGraph)graphVector.get(0)).getAttrName());
    }
  }
  return subtyp;
}
//end dsplGraph

//begin labelAttribute
/**
 * A method of the LabelAttribute-Interface
 * @return The label of the Attribute
 * @see <a href="DisplayCollection.html#getLabel">Source</a>
 */
public String getLabel(double time){
if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof LabelAttribute){
      return (((LabelAttribute)graphVector.get(0)).getLabel(time));
    }
  }
  return null;
}

//end Labelattribute

//begin displayComplex
/** Draw this object to g.
 * The time parameter can be used to create a special representation of 
 * a time dependent object or for asking the renderattribute for some
 * values. The currently used transformation matrix can be used for example
 * if an object should have the same size in each zoom level. 
 * @param g:    the used graphics context
 * @param time: the current time in the animation. 
 * @param at:   the currently used transformation matrix.
 * @see <a href="DisplayCollection.html#draw">Source</a>
**/
public void draw(Graphics g, double time, AffineTransform at){
  if(isGraph(subtyp)){
    if(graphVector.firstElement() instanceof DisplayComplex){
      ((DisplayComplex)graphVector.get(0)).draw(g,time,at);
    }
  }
  
}
//end displayComplex

/**
 *A class for storing the shapes of the graphical objects in an collection
 **/
protected class shapeSet{
 private int[] set;
 private int lastSearch;
 private int lastPosition;
  public shapeSet(int position,int count){
    set=new int[count];  
    for(int i=0;i<count;i++){
      set[i]=position+i;
    }
  }

/**
 * a method to determine wether the shape is from this instance of the object
**/
  public boolean hasNumber(int num){
   lastSearch=num;		
   for(int i=0;i<set.length;i++){
   if(set[i]==num){
	lastPosition=i;
	return true;
   }
   }
   return false;
  }

/**
 * a method to get the position of the given shape in the shapeVector 
 * @param num the number of the shape
 * @return the postion in the shapeVector
**/
  public int getPosition(int num){
    if(num==lastSearch) return lastPosition;
    else{
    for(int i=0;i<set.length;i++){
      if(set[i]==num){
	return i;
      }
    }
  }
  return 0;
}

}

}



