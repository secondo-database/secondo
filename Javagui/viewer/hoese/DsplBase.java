
package  viewer.hoese;

import  sj.lang.ListExpr;
import viewer.HoeseViewer;

/** This class need to be implemented by displayclasses of datatypes directly or 
 * indirectly by overwriting a class that implements this one. 
 */
public interface DsplBase {

  /**
   * In relations it is neccessary to get the name of the attribute of this datatype instance in
   * a tuple.
   * @return attribute name
   */
  public String getAttrName ();



  /**
   * If this datatype shouldn't be displayed in the default 2D-geographical viewer this
   * method returns the specialized frame, which can do this.
   * @return An instance of a SecondoFrame, with the ability of displaying this type.
   * @see generic.SecondoFrame
   */
  public SecondoFrame getFrame ();



  /**
   * This method is used to analyse the type and value in NestedList format and build
   * up the intern datastructures for this type. An alphanumeric representation is 
   * neccessary for the displaying this type in the queryresultlist.
   * @param type A ListExpr of the datatype atomic like point or not e.g. rel
   * @param value The value of this object
   * @param qr The queryresultlist to add alphanumeric representation
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr);



  /**
   * Sets the visibility of an object 
   * @param b true=show false=hide
   */
  public void setVisible (boolean b);



  /**
   * Gets the visibility of an object
   * @return true if visible, false if not
   */
  public boolean getVisible ();



  /**
   * Sets the select status of an object, textual or graphical.
   * @param b true if selected, false if not.
   */
  public void setSelected (boolean b);



  /**
   * Gets the select status of an object, textual or graphical
   * @return true if selected, false if not
   */
  public boolean getSelected ();
}



