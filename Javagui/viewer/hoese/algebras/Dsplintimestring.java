

package  viewer.hoese.algebras;

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;
import  java.util.*;
import  javax.swing.*;


/**
 * A displayclass for the intimestring-type (spatiotemp algebra), alphanumeric with TimePanel
 */
public class Dsplintimestring extends Dsplinstant {
  String Wert;

  /**
   * A method of the Timed-Interface to render the content of the TimePanel
   * @param PixelTime pixel per hour
   * @return A JPanel component with the renderer
   * @see <a href="Dsplintimestringsrc.html#getTimeRenderer">Source</a>
   */
  public JPanel getTimeRenderer (double PixelTime) {
    JPanel jp = super.getTimeRenderer(PixelTime);
    JLabel jl = (JLabel)jp.getComponent(0);
    jl.setText(jl.getText() + "  " + Wert);
    return  jp;
  }

  /**
   * Scans the representation of a intimestring datatype 
   * @param v An instant and a string value
   * @see sj.lang.ListExpr
   * @see <a href="Dsplintimestringsrc.html#ScanValue">Source</a>
   */
  public void ScanValue (ListExpr v) {
    //System.out.println(v.writeListExprToString());
    if (v.listLength() != 2) {
      System.out.println("Error: No correct intimebool expression: 2 elements needed");
      err = true;
      return;
    }
    super.ScanValue(v.first());
    if (err)
      return; 
    else 
      err = true;
    if (v.second().atomType() != ListExpr.STRING_ATOM)
      return;
    Wert = v.second().stringValue();
    err = false;
  }

  /**
   * Init. the Dsplintimestring instance.
   * @param type The symbol intimestring
   * @param value The value of an instant and an string.
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplintimestringsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    ScanValue(value);
    if (err) {
      System.out.println("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": TA(InstantString))"));
      return;
    } 
    else 
      qr.addEntry(this);
  }

  /** The text representation of this object 
   * @see <a href="Dsplintimestringsrc.html#toString">Source</a>
   */
  public String toString () {
    return  AttrName + ":" + LEUtils.convertTimeToString(TimeBounds.getStart())
        + " " + Wert + ": TA(InstantString) ";
  }
}



