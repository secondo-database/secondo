

package  viewer.hoese.algebras;

import  javax.swing.*;
import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;


/**
 * A example to implement a separate JFrame as viewer for images. For test reasons only it is placed in algebras.
 */
public class ImageViewer
    implements SecondoFrame {

  public void show (boolean b) {
    f.setVisible(b);
  }

  public void addObject (Object o) {}
  public void removeObject (Object o) {}
  public void select (Object o) {
    if (o != null)
      f.setContentPane(((Dsplimage)o).lab);
    f.pack();
  }
  private static JFrame f = new JFrame();
}



