

package  viewer.hoese;

//import generic.Interval;
import  sj.lang.ListExpr;
import viewer.HoeseViewer;


/**
 * This class is a collection of general used methods for listexpr aand time
 */
public class LEUtils {
  /**
   * Reads an instant out of a listexpr and calculates a time value as double-value
   * @param le A listexpr with sn instant
   * @return A Double object with the time or null if an error occured
   * @see <a href="LEUtilssrc.html#readInstant">Source</a>
   */
  public static Double readInstant (ListExpr le) {
    if ((le.listLength() != 4) && (le.listLength() != 6) && (le.listLength() != 7) && (le.listLength() != 8))
      return  null;
    if ((le.first().atomType() != ListExpr.SYMBOL_ATOM) || (le.second().atomType()
        != ListExpr.INT_ATOM) || (le.third().atomType() != ListExpr.INT_ATOM)
        || (le.fourth().atomType() != ListExpr.INT_ATOM))
      return  null;

    if (le.listLength() >= 6) {
        if(le.first().atomType()!=ListExpr.SYMBOL_ATOM)
	   return null;
	if(!le.first().symbolValue().equals("datetime"))
	   return null;
	le = le.rest();
	int[] Values = new int[7];
	for(int i=0;i<7;i++)
           Values[i]=0;
        int number = le.listLength();
	for(int i=0;i<number;i++){
           if(le.first().atomType()!=ListExpr.INT_ATOM)
	      return null;
	   else
	     Values[i]= le.first().intValue();
	   le=le.rest();
	}
	return new Double(convertDateTime2Double(Values[0],Values[1],Values[2],Values[3],
	                                         Values[4],Values[5],Values[6]));
    }
    else {
      if (!le.first().symbolValue().equals("date"))
        return  null;
      return  new Double(convertDateTime2Double(le.second().intValue(), le.third().intValue(),
          le.fourth().intValue(), 0, 0,0,0));
    }
  }

  /**t
   * Analyses a ListExpr by scanning through the type and value ListExpr
   * @param type The datatype
   * @param value the value of the datatype
   * @param qr Collects the results
   * @see <a href="LEUtilssrc.html#analyse">Source</a>
   */
  public static void analyse (ListExpr type, ListExpr value, QueryResult qr) {
    DsplBase db;
    if (type.isAtom()) {
      db = getClassFromName(type.symbolValue());
      db.init(type, value, qr);
    }
    else {
      db = getClassFromName(type.first().symbolValue());
      db.init(type, value, qr);
    }
  }

  /**
   * Create an instance of a datatype by its name, if there is no class with the name Dspl<name>
   * in the algebras package, and it is not a registered in the configuration-file then the generic class
   * Dsplgeneric is used.
   * @param name The name of the datatype.
   * @return The class is a subtype of DsplBase
   * @see <a href="LEUtilssrc.html#getClassFromName">Source</a>
   */
  public static DsplBase getClassFromName (String name) {
    DsplBase displayObject;
    // displayObject is initialized to the default display class.
    String className = "viewer.hoese.algebras.Dspl" + name;
    try {
      // Tries to set displayObject to an object of the required display class.
      displayObject = (DsplBase)(Class.forName(className).newInstance());
    } catch (Exception except) {
      String App = HoeseViewer.configuration.getProperty(name);
      if (App == null)
        displayObject = new DsplGeneric();
      else
        displayObject = new Dsplexternal(App, name);
    }
    return  displayObject;
  }

  /**
   * Converts a time value from double to String
   * @param t the double value
   * @return The time as String
   * @see <a href="LEUtilssrc.html#convertTimeToString">Source</a>
   */
  public static String convertTimeToString (double t) {
        return DateTime.getString(t);
  }

  /**
   * Convert a time in separated integers into a double-value
   * @param day The day value
   * @param month The month value
   * @param year The year value
   * @param hour The hour value
   * @param minute The minute value
   * @return The converted time as double
   */
  public static double convertDateTime2Double (int day, int month, int year,
      int hour, int minute,int second,int millisecond) {
      double res = DateTime.convertToDouble(year,month,day,hour,minute,second,millisecond);
      return res;
  }

  /**
   * Reads an interval out of a listexpr and calculates a time interval as Interval-object
   * @param le A listexpr with an interval
   * @return A Interval object with the sart- and endtime  or null if an error occured
   * @see generic.Interval
   * @see <a href="LEUtilssrc.html#readInterval">Source</a>
   */
  public static Interval readInterval (ListExpr le) {
    if (le.listLength() != 4)
      return  null;
    Double start = readInstant(le.first());
    Double end = readInstant(le.second());
    //System.out.println("start:"+start+" end:"+end);
    if ((start == null) || (end == null) || (le.third().atomType() != ListExpr.BOOL_ATOM)
        || (le.fourth().atomType() != ListExpr.BOOL_ATOM))
      return  null;
    boolean leftcl = le.third().boolValue();
    boolean rightcl = le.fourth().boolValue();
    return  new Interval(start.doubleValue(), end.doubleValue(), leftcl, rightcl);
  }

  /**
   * Reads an numeric (a rational or integer number) out of a listexpr and calculates a numeric value as double-value
   * @param le A ListExpr with a numeric expression
   * @return A Double object with the numeri value or null if an error occured
   * @see <a href="LEUtilssrc.html#readNumeric">Source</a>
   */
  public static Double readNumeric (ListExpr le) {
    if (le.isAtom()) {
      if (! (le.atomType()==ListExpr.INT_ATOM || le.atomType()==ListExpr.REAL_ATOM)) {
        System.out.println("Error: No correct numeric expression: rat or int or real-type needed");
        return  null;
      }
      if (le.atomType()==ListExpr.INT_ATOM)
         return  new Double(le.intValue());
      else
         return new Double(le.realValue());
    }
    else {
      if ((le.listLength() != 5)&& (le.listLength() != 6)){
        System.out.println("Error: No correct rat expression: 5 elements needed");
        return  null;
      }
      if (le.listLength()==5) {
      if ((le.first().atomType() != ListExpr.SYMBOL_ATOM) || (le.second().atomType()
          != ListExpr.INT_ATOM) || (le.third().atomType() != ListExpr.INT_ATOM)
          || (le.fourth().atomType() != ListExpr.SYMBOL_ATOM) || (le.fifth().atomType()
          != ListExpr.INT_ATOM)) {
        System.out.println("Error: No correct rat5 expression: wrong types");
        return  null;
      }
      if ((!le.first().symbolValue().equals("rat")) || (!le.fourth().symbolValue().equals("/"))) {
        System.out.println("Error: No correct rat5 expression: wrong symbols"
            + le.first().symbolValue() + ":" + le.fourth().symbolValue() +
            ":");
        return  null;
      }
      double g = (double)le.second().intValue();
      return  new Double((Math.abs(g) + (double)le.third().intValue()/(double)le.fifth().intValue()));
    }else {
      if ((le.first().atomType() != ListExpr.SYMBOL_ATOM) || (le.second().atomType() != ListExpr.SYMBOL_ATOM)
      ||(le.third().atomType()!= ListExpr.INT_ATOM) || (le.fourth().atomType() != ListExpr.INT_ATOM)
          || (le.fifth().atomType() != ListExpr.SYMBOL_ATOM) || (le.sixth().atomType()
          != ListExpr.INT_ATOM)) {
        System.out.println("Error: No correct rat6 expression: wrong types");
        return  null;
      }
      if ((!le.first().symbolValue().equals("rat")) ||
      	  (!le.fifth().symbolValue().equals("/")  ) ||
      	  !(le.second().symbolValue().equals("-")  ||
      	    le.second().symbolValue().equals("+") )) {
        System.out.println("Error: No correct rat6 expression: wrong symbols"
            + le.first().symbolValue() + ":" + le.fifth().symbolValue() +
            ":"+le.writeListExprToString());
        return  null;
      }
      double g = (double)le.third().intValue();
      double v=1;
      if (le.second().symbolValue().equals("-")) v=-1;
      return  new Double(v*(Math.abs(g) + (double)le.fourth().intValue()/(double)le.sixth().intValue()));
    	}
    }
  }

}



