

package  viewer.hoese;

//import generic.Interval;
import  sj.lang.ListExpr;
import viewer.HoeseViewer;
import gui.Environment;


/**
 * This class is a collection of general used methods for listexpr aand time
 */
public class LEUtils {
  /**
   * This function reads an instant value from the argument list.
   * @result the Double containing the instant value or null if
   * the argument don't represent a valid Instant.
  **/
  public static Double readInstant (ListExpr le) {
      ListExpr value;

      if(le.listLength()==2){ // check for type
         if(le.first().atomType()==le.SYMBOL_ATOM &&
	      ( le.first().symbolValue().equals("instant") ||
	        le.first().symbolValue().equals("datetime"))){
                value = le.second();	  // read over the type

         } else
	     value = le;
      } else
         value = le;


      // string representation
      if(value.atomType()==value.STRING_ATOM){
         long[] daymillis = DateTime.getDayMillis(value.stringValue());
         if(daymillis==null)
	    return null;
	 else
	    return  new Double((double)daymillis[0]+(double)daymillis[1]/86400000.0);
      }

      // real representation
      if(value.atomType()==value.REAL_ATOM){
          return new Double (value.realValue());
      }

      // all other representation are based on proper lists
      // not on atoms
      if(value.atomType()!=value.NO_ATOM)
          return null;


      // Julian representation
      if(value.listLength()==2){
         if(value.first().atomType()==value.INT_ATOM &&
	    value.second().atomType()==value.INT_ATOM){
             double day = (double) value.first().intValue();
	     double ms  = (double) value.second().intValue();
	     return new Double(day + ms /86400000.0);
	 } else
	    return null;
      }

      // Gregorian representation
      if(value.isEmpty())
         return null;

      // check for an deprecated data version
      if(value.first().atomType()==value.SYMBOL_ATOM &&
         value.first().symbolValue().equals("datetime")){
         System.err.println("Deprecated nested list representation of time !!");
	 value = value.rest();
      }

      // at least (day month year) and has to be included
      // at most (day month year hour minute second millisecond) can be included
      if(value.listLength()<3 || value.listLength()>7)
          return null;

      // all contained atoms has to be integers
      ListExpr tmp = value;
      while(!tmp.isEmpty()){
          if(tmp.first().atomType()!=tmp.INT_ATOM)
	     return null;
	  tmp = tmp.rest();
      }
      int year=0,month=0,day=0,hour=0,minute=0,second=0,milli=0;
      // first we read the required parts
      day = value.first().intValue();
      month = value.second().intValue();
      year  = value.third().intValue();
      // read over the date
      value = value.rest().rest().rest();

      int len = value.listLength();
      // the length can be 0,2,3 or 4
      if(len==1 || len > 4)
         return null;
      if(len>0){  // minimum 2
         hour = value.first().intValue();
	 minute = value.second().intValue();
      }
      if(len>2)
         second = value.third().intValue();
      if(len>3)
         milli = value.fourth().intValue();

       // easy check of intervals
       if(year==0) return null; // 1 A.C  is direcly after 1 B.C.
       if(month<0 || month>12) return null;
       if(day<0 || day>31) return null; // a check for 31.2.xxx is omitted here
       if(hour<0 || hour>23) return null;
       if(minute<0 || minute>59) return null;
       if(second<0 || second>59) return null;
       if(milli<0 || milli>999) return null;
       // compute the result
       return new Double(convertDateTime2Double(day,month,year,hour,minute,second,milli));
  }

  /**
   * Analyses a ListExpr by scanning through the type and value ListExpr
   * @param type The datatype
   * @param value the value of the datatype
   * @param qr Collects the results
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
   */
  public static Interval readInterval (ListExpr le) {
    if (le.listLength() != 4){
      if(Environment.DEBUG_MODE){
         System.err.println("wrong listlength for interval (needed is 4) :" + le.listLength());
      }
      return  null;
    }
    Double start = readInstant(le.first());
    if(start==null){
       if(Environment.DEBUG_MODE){
          System.err.println("Error in reading start - instant ");
       }
       return null;
    }
    Double end = readInstant(le.second());
    if(end==null){
       if(Environment.DEBUG_MODE){
          System.err.println("Error in reading end instant ");
       }
       return null;
    }
    //System.out.println("start:"+start+" end:"+end);
    if ((le.third().atomType() != ListExpr.BOOL_ATOM)
        || (le.fourth().atomType() != ListExpr.BOOL_ATOM)){
      if(Environment.DEBUG_MODE){
         System.err.println("not boolean atoms for lefttclosed or rightclosed");
      }
      return  null;
    }
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



