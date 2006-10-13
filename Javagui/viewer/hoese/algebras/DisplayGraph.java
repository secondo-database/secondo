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
import tools.Reporter;
import java.awt.image.BufferedImage;


/**
 * This class is the superclass of all grapical objects. 
 */

public abstract class DisplayGraph extends DsplGeneric
    implements DsplGraph,DsplSimple {

/** if an error occurs during creation this field will be true */
  protected boolean err = false;

/** The layer in which this object is drawn */
  protected Layer RefLayer;

/** The category of this object preset by the defaultcategory */
  protected Category Cat = Category.getDefaultCat();

/** The Object labeling this one */
  private LabelAttribute labelAttribute;

/** Object changing the color during painting **/
 protected RenderAttribute renderAttribute=null;

/** Position of the label */
  private Point LabPosOffset = new Point(-30, -10);

 /** the typewidth to display in a relation */
 protected int minTypeWidth=0;
 protected int minValueWidth=0;

 /** A point for shippping data between this class and the projectionmanager.
   * this point has only an technical aspect. It avoid to create a lot of
   * points during the projection if this point is reused in the projection during
   * analysing the nested list structure. Using this point instead of using the new 
   * operator can speed up this procedure very much. 
   **/
 static java.awt.geom.Point2D.Double aPoint = new Point2D.Double();

/** A basicstroke for easy creating stroked shapes for line objects.
  * The line painting algorithm seems to be much slower than to paint the
  * border of the same area (which will have the same result. For this reason, it 
  * is recommendet to create an area instead a line and only to paint this area without
  * any fill. For testing the contains predicates, we need this border instead the 
  * complete area. The BasicStroke class provides the needed functionality.
  **/
 static BasicStroke stroke = new BasicStroke(); 


  /** Checks whether the Shape with the given number should be displayed 
    * as a point.
    **/
  public boolean isPointType (int num) {
    return  false;
  }

  /** For LineTypes, no interior will be drawn
    **/
  public boolean isLineType(int num){
    return false;
  }


  /**
   *
   * @return The text of the label.
   */
  public String getLabelText (double time) {
    if(labelAttribute==null)
      return null;
    else
      return labelAttribute.getLabel(time);
  }

  public LabelAttribute getLabelAttribute(){
     return labelAttribute;
  }
   
  /**
   * Sets the label attribute 
   */
  public void setLabelAttribute(LabelAttribute labelAttribute) {
    this.labelAttribute = labelAttribute;
  }

  /** Sets the renderAttribute.
  **/
  public void setRenderAttribute(RenderAttribute renderAttribute){
    this.renderAttribute = renderAttribute;
  }

  /** returns the render attribute **/
  public RenderAttribute getRenderAttribute(){
     return renderAttribute;
  }



  /**
   *
   * @return The position of the label as a Point
   * @see <a href="DisplayGraphsrc.html#getLabPosOffset">Source</a>
   */
  public Point getLabPosOffset () {
    return  LabPosOffset;
  }

  /**
   * Sets the position of the label
   * @param pt The position for the label
   * @see <a href="DisplayGraphsrc.html#setLabPosOffset">Source</a>
   */
  public void setLabPosOffset (Point pt) {
    LabPosOffset = pt;
  }

  /**
   *
   * @return The boundingbox of the drawn Shape
   * @see <a href="DisplayGraphsrc.html#getBounds">Source</a>
   */
  public Rectangle2D.Double getBounds () {
     int num = numberOfShapes();
     Rectangle2D.Double r = null;
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

  /** returns the number of Shapes for this object **/
  public abstract int numberOfShapes();

  public abstract Shape getRenderObject (int num,AffineTransform at);




  /**
   * Sets the category of this object.
   * @param acat A category-object
   * @see <a href="DisplayGraphsrc.html#setCategory">Source</a>
   */
  public void setCategory (Category acat) {
    Cat = acat;
  }

  /**
   *
   * @return The actual category of this instance.
   * @see <a href="DisplayGraphsrc.html#getCategory">Source</a>
   */
  public Category getCategory () {
    return  Cat;
  }

  /**
   * Sets the layer of this instance.
   * @param alayer A layer to which this graphic object belong.
   * @see <a href="DisplayGraphsrc.html#setLayer">Source</a>
   */
  public void setLayer (Layer alayer) {
    RefLayer = alayer;
  }

  /**
   *
   * @return The layer of this object
   * @see <a href="DisplayGraphsrc.html#getLayer">Source</a>
   */
  public Layer getLayer () {
    return  RefLayer;
  }

  /**
   * Tests if a given position is contained in the RenderObject.
   * @param xpos The x-Position to test.
   * @param ypos The y-Position to test.
   * @param scalex The actual x-zoomfactor
   * @param scaley The actual y-zoomfactor
   * @return true if x-, ypos is contained in this points type
   * @see <a href="DisplayGraphsrc.html#contains">Source</a>
   */
  public boolean contains (double xpos, double ypos, double scalex, double scaley) {
    // create an rectangle around the given point
    Rectangle2D.Double r = new Rectangle2D.Double(xpos - 2.0*scalex, ypos - 2.0*scaley, 4.0*scalex, 4.0*scaley);
    int num = numberOfShapes();
    for(int i=0;i<num;i++){
       Shape shp = getRenderObject(i,RefLayer.getProjection());
       if(shp!=null && shp.intersects(r)){
          return true;
       }
    }
    return false;

  }

  /** The text representation of this object
   * @see <a href="DisplayGraphsrc.html#toString">Source</a>
   */
  public String toString () {
    return  extendString(AttrName,minTypeWidth) + " : " + extendString(Cat.getName(),minValueWidth);
  }


  public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth, QueryResult qr)
  { minTypeWidth = typewidth;
    minValueWidth = valuewidth;
    init(type,value,qr);
  }

}





