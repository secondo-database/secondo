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


/**
 * This class is the superclass of all grapical objects. Because it fully implements the DsplGraph
 * interface, the subclasses mustn´t do it.
 */
public class DisplayGraph extends DsplGeneric
    implements DsplGraph,DsplSimple {
/** if an error occurs during creation this field will be true */
  protected boolean err = false;
/** The layer in which this object is drawn */
  protected Layer RefLayer;
/** The category of this object preset by the defaultcategory */
  protected Category Cat = Category.getDefaultCat();
/** The shape that was drawn by this instance */
  protected Shape RenderObject;
/** Point datatayes e.g. point,points need special treatment,therefore this flag need to be set */
  protected boolean ispointType = false;
/** The Object labeling this one */
  private LabelAttribute labelAttribute;
/** Object changing the color during painting **/
 protected RenderAttribute renderAttribute=null;

/** Position of the label */
  private Point LabPosOffset = new Point(-30, -10);
 /** the typewidth to display in a relation */
 protected int minTypeWidth=0;
 protected int minValueWidth=0;
 // a point for shippping data between this class and the projectionmanager
 static java.awt.geom.Point2D.Double aPoint = new Point2D.Double();
 // a basicstroke for easy creating stroked shapes for line objects
 static BasicStroke stroke = new BasicStroke(); 


  /**
   *
   * @return true if it is a pointtype object
   * @see <a href="DisplayGraphsrc.html#isPointType">Source</a>
   */
  public boolean isPointType () {
    return  ispointType;
  }

  /** For LineTypes, no interior will be drawn
    **/
  public boolean isLineType(){
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
    if (RenderObject == null)
      return  null;
    else{
      Rectangle2D r = RenderObject.getBounds2D();
      return new Rectangle2D.Double(r.getX(),r.getY(),r.getWidth(),r.getHeight());
    }
  }

  /**
   * Subclasses overrides this method to design their own Shape-object.
   * @param af The actual transformation of the graphic context, in which the object will be placed
   * @return Shape to render
   * @see <a href="DisplayGraphsrc.html#getRenderObject">Source</a>
   */
  public Shape getRenderObject (AffineTransform af) {
    return  RenderObject;
  }

  /** returns a set of objects to paint.
    * This can be usefull when different kinds of objects
    * should be drawn, e.g. point, line and region within a
    * single object.
    **/
  public Shape[] getRenderObjects(AffineTransform af){
      return null;
  } 



  /** paints this component on g **/
  public void draw(Graphics g, double time){
     if(RefLayer==null)
        return;
     draw(g,RefLayer.getProjection(),time);
  }


  /**
   * This method draws the RenderObject with ist viewattributes collected in the category
   * @param g The graphic context to draw in.
   */
  public void draw (Graphics g,AffineTransform af2,double time) {
   
    Shape sh=null; // transformed renderobject

    Graphics2D g2 = (Graphics2D)g;


    Shape render = getRenderObject(af2);
    Shape[] moreShapes = getRenderObjects(af2);

    // the bounding box of all objects
    Rectangle2D bounds = null;

    if (render == null && moreShapes==null){
      // no object found
      return;
    }

    if(render!=null){ 
        sh = af2.createTransformedShape(render);
        bounds = render.getBounds2D();
    }
    Shape[] shs=null;
    if(moreShapes!=null){
      shs = new Shape[moreShapes.length];
      for(int i=0;i<shs.length;i++){
          shs[i] = af2.createTransformedShape(moreShapes[i]);
          if(bounds==null){
             bounds = moreShapes[i].getBounds2D();
          }else{
             bounds.add(moreShapes[i].getBounds2D());
          }
      }
    }

    Paint fillStyle = Cat.getFillStyle(renderAttribute,time);

    // paint the interior
    if (fillStyle != null && !isLineType()){
      g2.setPaint(fillStyle);
      if(sh!=null){
         g2.fill(sh);
      }
      if(shs!=null){
        for(int i=0;i<shs.length;i++){
           g2.fill(shs[i]);
        }
      }
    }
  
    g2.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER));
    Color aktLineColor = Cat.getLineColor();
    if (selected){
      aktLineColor = new Color(Color.white.getRGB() ^ Cat.getLineColor().getRGB());
    }
    g2.setColor(aktLineColor);

    // paint the border
    if ((Cat.getLineWidth(renderAttribute,time) > 0.0f) || (selected)){
      g2.setStroke(Cat.getLineStroke(renderAttribute,time));
      if(sh!=null){
          g2.draw(sh);
      }
      if(shs!=null){
          for(int i=0;i<shs.length;i++){
             g2.draw(shs[i]); 
          }
      }
    }
    drawLabel(g2, bounds,time);
  }

  /**
   * The draw method for the label.
   * @param g  The graphic context to draw in.
   * @param r  The bounding box of the object to label.
   */
  public void drawLabel (Graphics g, Rectangle2D r, double time) {
    if(r==null){
       System.err.println("drawLabel with null-bounding box called !!");
       return; 
    }
    String LabelText = getLabelText(time);
    if (LabelText == null || LabelText.trim().equals("")){ // no label
      return;
    }

    Graphics2D g2 = (Graphics2D)g;
    AffineTransform af2 = RefLayer.getProjection();
    Point2D.Double p = new Point2D.Double(r.getX() + r.getWidth()/2, r.getY()
        + r.getHeight()/2);
    af2.transform(p, p);
    if (selected) {
      Rectangle2D re = g2.getFont().getStringBounds(LabelText, g2.getFontRenderContext());
      g2.setPaint(new Color(255, 128, 255, 255));
      g2.fill3DRect((int)(p.getX() + LabPosOffset.getX()), (int)(p.getY() +
          LabPosOffset.getY() + re.getY()), (int)re.getWidth(), (int)re.getHeight(),
          true);
    }
    g2.setPaint(Cat.getLineColor());
    float x = (float)(p.getX()+LabPosOffset.getX());
    float y = (float)(p.getY()+LabPosOffset.getX());
    g2.drawString(LabelText, x,y);
  }

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
    //if (! isActual())  RenderObject =createRenderObject();
    if (RenderObject == null)
      return  false;
    Rectangle2D.Double r = new Rectangle2D.Double(xpos - 2.0*scalex, ypos - 2.0*scaley, 4.0*scalex, 4.0*scaley);
    return RenderObject.intersects(r);
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





