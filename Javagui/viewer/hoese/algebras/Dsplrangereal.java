

package  viewer.hoese.algebras;

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;
import  java.util.*;


/**
 * A displayclass for the rangereal-type (spatiotemp algebra), alphanumeric only
 */
public class Dsplrangereal extends DsplGeneric {
  Vector ranges = new Vector(10, 5);

  /**
   * Scans the representation of a rangereal datatype, and constructs the ranges Vector 
   * @param v A list of real intervals
   * @see sj.lang.ListExpr
   * @see <a href="Dsplrangerealsrc.html#ScanValue">Source</a>
   */
  private boolean ScanValue (ListExpr v) {
    int i = 0;
    ////System.out.println(v.writeListExprToString());
    while (!v.isEmpty()) {
      ListExpr le = v.first();
      //System.out.println(le.writeListExprToString());
      if (le.listLength() != 4)
        return  false;
      Double start = LEUtils.readNumeric(le.first());
      Double end = LEUtils.readNumeric(le.second());
      if ((start == null) || (end == null) || (le.third().atomType() != ListExpr.BOOL_ATOM)
          || (le.fourth().atomType() != ListExpr.BOOL_ATOM))
        return  false;
      boolean leftcl = le.third().boolValue();
      boolean rightcl = le.fourth().boolValue();
      String left = (leftcl) ? "[" : "]";
      String right = (rightcl) ? "]" : "[";
      //ranges[i++]
      String s = new String(left + start.toString() + "," + end.toString() + 
          right);
      ranges.add(s);
      v = v.rest();
    }
    return  true;
  }

  /**
   * Init. the Dsplrangereal instance.
   * @param type The symbol rangereal
   * @param value A list of real intervals
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplrangerealsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    String e = "";
    if ((!ScanValue(value)) || value.isEmpty())
      qr.addEntry("(" + type.symbolValue() + ":[?,?])"); 
    else {
      //qr.addEntry(type.symbolValue()+": "+ranges); //.elementAt(0));
      for (int i = 0; i < ranges.size(); i += 1)
        e = e + " " + (String)ranges.elementAt(i);
      qr.addEntry(type.symbolValue() + ":" + e);
    }
    return;
  }
}



