package de.fernunihagen.dna.hoese;

import javamini.awt.Shape;
import javamini.awt.geom.Rectangle2D;
import android.graphics.Point;
import javamini.awt.geom.AffineTransform;



/** To view a datatype in the standard 2d-geographical viewer, this interface must be 
 * implemented.
 */
public interface DsplGraph extends DsplBase
{

  /** Returns the number of contained shapes
    */
   public int numberOfShapes();

  /**
   * to determine, whether the type is displayed as point.
   * @return true if pointtype
   */
  public boolean isPointType (int num);

  /**
    *   determines whether the type is a line (no interior)
    **/
  public boolean isLineType(int num);   


  /**
   * Text of the associated Label.
   * @return Labeltext
   */
  public String getLabelText ( double time);



  /** Returns the attribute controlling the 
    * creation of the label.
    **/
  public LabelAttribute getLabelAttribute(); 

  /**
   * Sets the labeltext for an object
   * @param label Text of label
   */
  public void setLabelAttribute(LabelAttribute labelAttribute);

  /**
    * sets the renderattribute of this object 
    **/
   public void setRenderAttribute(RenderAttribute renderAttribute);  
     

   /** returns the render attribute assigned to this object **/
   public RenderAttribute getRenderAttribute();


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
   *  Returns one of the current Shapes of this object. 
   * @param num: the number of the requested shape
   */
  public Shape getRenderObject (int num, AffineTransform af);


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

