

package  viewer.hoese.algebras;

import  javax.swing.*;
import  sj.lang.ListExpr;
import  viewer.*;
import  viewer.hoese.*;


/**
 * An example how to implement the DsplBase interface for a class displaying images in a
 * different frame.
 */
public class Dsplimage
    implements DsplBase {
  protected String AttrName;
  protected boolean selected;
  private boolean visible = true;
  private static ImageViewer TestFrame = new ImageViewer();
  JLabel lab;

  public String getAttrName () {
    return  AttrName;
  }

  public SecondoFrame getFrame () {
    return  TestFrame;
  }

  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    lab = new JLabel(new ImageIcon(value.stringValue()));
    lab.setOpaque(true);
    qr.addEntry(this);
    return;
  }

  public void setVisible (boolean b) {
    visible = b;
  }

  public boolean getVisible () {
    return  visible;
  }
  public void setSelected (boolean b) {
    selected = b;
  }

  public boolean getSelected () {
    return  selected;
  }

  public String toString () {
    return  AttrName + ":ImageAttr";
  }
}



