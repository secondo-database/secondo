

package  viewer.hoese;

import  java.util.*;
import  javax.swing.*;
import viewer.HoeseViewer;

  /** If datatypes have a temporal component this interface must be implemented.
  */

public interface Timed {

  /**
   * Gets the over all time boundarys
   * @return Interval
   * @see generic.Interval
   */
  public Interval getTimeBounds ();


  /**
   * Gets the list of intervals this object is defined at
   * @return Vector of intervals
   * @see generic.Interval
   */

  public Vector getIntervals();
  /**
   * In the TimePanel component a temporal datatype can be represented individually.
   * This method defines a specific output as JPanel
   * @param PixelTime How much timeunits have a pixel.
   */  
  public JPanel getTimeRenderer (double PixelTime);
}



