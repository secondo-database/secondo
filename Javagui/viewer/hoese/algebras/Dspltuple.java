

package  viewer.hoese.algebras;

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;


/**
 * A displayclass for the tuple-type. For each attribute in this tuple its displayclass is called
 */
public class Dspltuple extends DsplGeneric {

  /**
   * This method is used to analyse the type and value of this tuple type.
   * For each attribute the helper method displayTuple is called
   * neccessary for the displaying this type in the queryresultlist.
   * @param type datatype tuple with its attribute-types 
   * @param value A listexpr of the attribute-values
   * @param qr The queryresultlist to add alphanumeric representation
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dspltuplesrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    int maxAttribNameLen = maxAttributLength(type.second());
    while (!value.isEmpty()) {
      displayTuple(type.second(), value.first(), maxAttribNameLen, qr);
      value = value.rest();
      if (!value.isEmpty())
        qr.addEntry("---------");
    }
    return;
  }

  /**
   * A method which create an instance of each displayclass that appears as attribute-type, 
   * and calls its init method.
   * @see <a href="Dspltuplesrc.html#displayTuple">Source</a>
   */
  private void displayTuple (ListExpr type, ListExpr value, int maxNameLen, 
      QueryResult qr) {
    int i;
    String s;
    DsplBase dg;
    while (!value.isEmpty()) {
      s = type.first().first().symbolValue();
      dg = LEUtils.getClassFromName(type.first().second().symbolValue());
      if (dg instanceof DsplSimple){
         ((DsplSimple)dg).init(type.first().first(),maxNameLen, value.first(),0, qr);
      }
      else{
         dg.init(type.first().first(), value.first(), qr);
      }
      type = type.rest();
      value = value.rest();
    }
    return;
  }

  /**
   * Calculate the length of the longest attribute name.
   * @param type A ListExpr of the attribute types
   * @return maximal length of attributenames
   * @see <a href="Dspltuplesrc.html#maxAttributLength">Source</a>
   */
  private static final int maxAttributLength (ListExpr type) {
    int max, len;
    String s;
    max = 0;
    while (!type.isEmpty()) {
      s = type.first().first().symbolValue();
      len = s.length();
      if (len > max)
        max = len;
      type = type.rest();
    }
    return  max;
  }
}



