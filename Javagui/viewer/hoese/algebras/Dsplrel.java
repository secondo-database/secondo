
package  viewer.hoese.algebras;

import  sj.lang.ListExpr;
import  viewer.*;
import  viewer.hoese.*;


/**
 * A displayclass for the rel-type, the type argument need to be analysed further
 */
public class Dsplrel extends DsplGeneric {

  /**
   * This method is used to analyse the type and value of the relation in NestedList format. * up the intern datastructures for this type. An alphanumeric representation is 
   * neccessary for the displaying this type in the queryresultlist.
   * @param type A relation datatype rel with its type structure 
   * @param value the value of the relation structure
   * @param qr The queryresultlist to add alphanumeric representation
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplrelsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    LEUtils.analyse(type.second(), value, qr);
    return;
  }
}



