

package  viewer.hoese.algebras;

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;


/**
 * A displayclass for the string-type, alphanumeric only
 */
public class Dspltext extends DsplGeneric implements DsplSimple{

  /**
   * This method is used to analyse the type and value in NestedList format and build
   * up the intern datastructures for this type. An alphanumeric representation is
   * neccessary for the displaying this type in the queryresultlist.
   * @param type A ListExpr of the datatype string
   * @param value A string in a listexpr
   * @param qr The queryresultlist to add alphanumeric representation
   * @see QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplstringsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
     qr.addEntry(new String(type.symbolValue() + ":" + value.first().textValue()));
     return;
  }

  public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth, QueryResult qr)
  {
     String T = new String(type.symbolValue());
     String V = new String(value.first().textValue());
     T=extendString(T,typewidth);
     V=extendString(V,valuewidth);
     qr.addEntry(T + " : " + V);
     return;

  }

  private String extendString(String S ,int MinWidth){
   String R = new String(S);
   int NoSpaces=MinWidth-R.length();
   for(int i=0;i<NoSpaces;i++)
      R = ' '+R;
   return R;
  }


}



