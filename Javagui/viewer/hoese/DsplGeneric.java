

package  viewer.hoese;

import  sj.lang.ListExpr;
import viewer.HoeseViewer;


/**
 * A generic implementation of the DsplBase Interface. Useful as baseclass to avoid
 * implementation of all the methods. If the datatype is unknown this class will be used.
 * @author  Thomas Höse
 * @version 0.99 1.1.02
 */
public class DsplGeneric
    implements DsplBase {
  protected String AttrName;
  protected boolean selected;
  private boolean visible = true;

  /**
   * In relations it is neccessary to get the name of the attribute of this datatype instance in
   * a tuple.
   * @return attribute name
   * @see <a href="DsplGenericsrc.html#getAttrName">Source</a> 
   */
  public String getAttrName () {
    return  AttrName;
  }
  /**
   * If this datatype shouldn't be displayed in the default 2D-geographical viewer this
   * method returns the specialized frame, which can do this.
   * @return null MainWindow will be used
   * @see generic.SecondoFrame
   * @see generic.MainWindow
   * @see <a href="DsplGenericsrc.html#getFrame">Source</a>
   */

  public SecondoFrame getFrame () {
    return  null;
  }
  /**
   * This method is used to analyse the type and value in NestedList format and build
   * up the intern datastructures for this type. An alphanumeric representation is 
   * neccessary for the displaying this type in the queryresultlist.
   * This class is used if datatype is unknown.
   * @param type A symbolatom with the datatype
   * @param value The value of this object 
   * @param qr The queryresultlist to add alphanumeric representation
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="DsplGenericsrc.html#init">Source</a>
   */

  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    String ts;
    if (type.isAtom())
      ts = type.writeListExprToString(); 
    else 
      ts = type.first().writeListExprToString();
    qr.addEntry(new String("Unknown Type: " + ts));
    HoeseViewer.writeCommand("\nListExpr for unknown type " + ts + value.writeListExprToString());
    return;
  }
  /**
   * Sets the visibility of an object 
   * @param b true=show false=hide
   * @see <a href="DsplGenericsrc.html#setVisible">Source</a>
   */
  public void setVisible (boolean b) {
    visible = b;
  }

  /**
   * Gets the visibility of an object
   * @return true if visible, false if not
   * @see <a href="DsplGenericsrc.html#getVisible">Source</a>
   */
  public boolean getVisible () {
    return  visible;
  }

  /**
   * Sets the select status of an object, textual or graphical.
   * @param b true if selected, false if not.
   * @see <a href="DsplGenericsrc.html#setSelected">Source</a>
   */
  public void setSelected (boolean b) {
    selected = b;
  }

  /**
   * Gets the select status of an object, textual or graphical
   * @return true if selected, false if not
   * @see <a href="DsplGenericsrc.html#getSelected">Source</a>
   */
  public boolean getSelected () {
    return  selected;
  }
}



