
package  viewer.hoese;

import  java.util.*;
import  javax.swing.*;
import viewer.HoeseViewer;

/** An interface for implementing a new viewer  e.g. Launcher 
 *  @see generic.Launcher
*/
public interface SecondoFrame {

  /**
   * Shows / hides the viewer 
   * @param b If b is true the viewer will be shown
   */
  public void show (boolean b);



  //JFrame getFrameInstance() ;
  /**
   * Adds an objekt to the viewer
   * @param o Instance to add
   */
  public void addObject (Object o);



  /**
   * Removes an objekt from the viewer
   * @param o Instance to remove
   */
  public void removeObject (Object o);



  /**
   * Select an objekt in the viewer
   * @param o Instance to select
   */
  public void select (Object o);
}



