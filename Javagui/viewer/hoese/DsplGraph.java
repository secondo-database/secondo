                
package  viewer.hoese;

import  java.awt.geom.*;
import  java.awt.*;
import  sj.lang.ListExpr;
import viewer.HoeseViewer;

/** To view a datatype in the standard 2d-geographical viewer, this interface must be 
 * implemented.
 */
public interface DsplGraph extends DsplBase
{

  /**
   * to determine, whether the type is displayed as point.
   * @return true if pointtype
   */
  public boolean isPointType ();


  ;


  /**
   * Text of the associated Label.
   * @return Labeltext
   */
  public String getLabelText ();



  /**
   * Sets the labeltext for an object
   * @param label Text of label
   */
  public void setLabelText (String label);



  /**
   * Gets the offset of the labelposition from center of object in pixel.
   * @return relative offset as point.
   */
  public Point getLabPosOffset ();



  /**
   * Sets the offset of the labelposition from center of object in pixel.
   * @param pt relative offset
   */
  public void setLabPosOffset (Point pt);



  /**
   * The boundingbox of the object in Worldcoordinates
   * @return Boundingbox in double precision
   */
  public Rectangle2D.Double getBounds ();



  /**
   * The actual shape of the object. May be null if drawn by itself.
   * @param af The transformation under which this shape will be drawn
   * @return momentary shape object
   */
  public Shape getRenderObject (AffineTransform af);



  /**
   * This method is called to draw this object .
   * @param g The graphic context to draw in.
   */
  public void draw (Graphics g);



  /**
   * Drawing of labeltext.
   * @param g graphics context
   * @param ro the shape of the associated object.
   */
  public void drawLabel (Graphics g, Shape ro);



  /**
   * Sets the category of drawing attributes for this object.
   * @param acat The category to set
   */
  public void setCategory (Category acat);



  /**
   * Gets the category of drawing attributes for this object.
   * @return The category of this object.
   */
  public Category getCategory ();



  /**
   * Specify the layer to which this object belongs.
   * @param alayer A Layer-object
   */
  public void setLayer (Layer alayer);



  /**
   * Gets the layer to which this object belongs.
   * @return Layer-object
   */
  public Layer getLayer ();



  /**
   * Tests if a world position is inside this object, or near by under a certain scale,
   * which is necessary to translate pixel distances to world-distance e.g. line
   * @param xpos x -coordinate of the position
   * @param ypos y -coordinate of the position
   * @param scalex x-scale
   * @param scaley y-scale
   * @return 
   */
  public boolean contains (double xpos, double ypos, double scalex, double scaley);
}



