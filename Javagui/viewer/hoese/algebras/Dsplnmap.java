
package  viewer.hoese.algebras;

import  java.awt.geom.*;
import  java.awt.*;
import  sj.lang.ListExpr;
import  java.util.*;
import  viewer.*;
import viewer.hoese.*;


/**
 * The displayclass of the NauticalMap algebras nmap datatype.
 */
public class Dsplnmap extends DisplayGraph {
/** The internal datatype representation */


  /**
   * Init. the Dsplnmap instance.
   * @param type The symbol nmap
   * @param value maximal three relations with spatial elements(points, lines, regions).
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplnmapsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.first().symbolValue();
    ListExpr nmap;
    ListExpr nmapRest;
    ListExpr rel1, rel2, rel3;
    nmap= type.first();
    nmapRest= type.rest();
    rel1= nmapRest.third();
    rel2= nmapRest.fourth();
    rel3= nmapRest.fifth();

    String ts;

    rel1.writeListExpr();
    LEUtils.analyse(rel1, value.third(), qr);
    qr.addEntry("---------");

    ts = rel2.writeListExprToString();
    System.out.println("ts: "+ ts);
    LEUtils.analyse(rel2, value.fourth(), qr);
    qr.addEntry("---------");
    ts = rel3.writeListExprToString();
    System.out.println("ts: "+ ts);
    LEUtils.analyse(rel3, value.fifth(), qr);
    qr.addEntry("---------");
  }

}



