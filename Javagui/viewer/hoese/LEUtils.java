

package  viewer.hoese;

//import generic.Interval;
import  sj.lang.ListExpr;
import viewer.HoeseViewer;


/**
 * This class is a collection of general used methods for listexpr aand time
 */
public class LEUtils {
  public static int NULL_DAY = 730484; //1.1.2000

  /**
   * Reads an instant out of a listexpr and calculates a time value as double-value
   * @param le A listexpr with sn instant
   * @return A Double object with the time or null if an error occured
   * @see <a href="LEUtilssrc.html#readInstant">Source</a> 
   */
  public static Double readInstant (ListExpr le) {
    if ((le.listLength() != 4) && (le.listLength() != 6))
      return  null;
    if ((le.first().atomType() != ListExpr.SYMBOL_ATOM) || (le.second().atomType()
        != ListExpr.INT_ATOM) || (le.third().atomType() != ListExpr.INT_ATOM)
        || (le.fourth().atomType() != ListExpr.INT_ATOM))
      return  null;
    if (le.listLength() == 6) {
      if ((le.fifth().atomType() != ListExpr.INT_ATOM) || (le.sixth().atomType()
          != ListExpr.INT_ATOM) || (!le.first().symbolValue().equals("datetime")))
        return  null;
      return  new Double(convertDateTime2Double(le.second().intValue(), le.third().intValue(), 
          le.fourth().intValue(), le.fifth().intValue(), le.sixth().intValue()));
    } 
    else {
      if (!le.first().symbolValue().equals("date"))
        return  null;
      return  new Double(convertDateTime2Double(le.second().intValue(), le.third().intValue(), 
          le.fourth().intValue(), 0, 0));
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
    double newValue = t + NULL_DAY;
    double days = 0.0;
    double result = 0.0;
    int year, day, month, minute, hour;
    if (newValue > 1) {
      //divide = div(newValue.instant, 365);
      year = (int)newValue/365;
      days = year*365;
      //divide = div(year, 4);
      //test   = divide.quot;
      days = days + year/4;
      //      divide = div(year, 100);
      //    test   = divide.quot;
      days = days - year/100;
      //      divide = div(year, 400);
      //      test   = divide.quot;
      days = days + year/400;
      boolean leapyear = (year != 0) && (((year%4 == 0) && (year%100 != 0))
          || (year%400 == 0));
      if (leapyear) {
        days = days - 1.0;
      }
      //      divide = div(newValue.instant, 1);
      int test = (int)newValue;
      if (days == (double)test) {
        year = year - 1;
        month = 12;
        day = 31;
      } 
      else {
        while (days > test) {
          year = year - 1;
          leapyear = (year != 0) && (((year%4 == 0) && (year%100 != 0)) || 
              (year%400 == 0));
          if (leapyear) {
            days = days - 366.0;
          } 
          else {
            days = days - 365.0;
          }
        }
      }
      int restdays = test;
      // divide   = div(days.instant, 1);
      //test     = divide.quot;
      restdays = restdays - (int)days;
      if (restdays < 32) {
        month = 1;
        day = restdays;
      } 
      else if (restdays < 60) {
        month = 2;
        day = restdays - 31;
      } 
      else if (restdays < 91) {
        month = 3;
        day = restdays - 59;
      } 
      else if (restdays < 121) {
        month = 4;
        day = restdays - 90;
      } 
      else if (restdays < 152) {
        month = 5;
        day = restdays - 120;
      } 
      else if (restdays < 182) {
        month = 6;
        day = restdays - 151;
      } 
      else if (restdays < 213) {
        month = 7;
        day = restdays - 181;
      } 
      else if (restdays < 244) {
        month = 8;
        day = restdays - 212;
      } 
      else if (restdays < 274) {
        month = 9;
        day = restdays - 243;
      } 
      else if (restdays < 305) {
        month = 10;
        day = restdays - 273;
      } 
      else if (restdays < 335) {
        month = 11;
        day = restdays - 304;
      } 
      else {
        month = 12;
        day = restdays - 334;
      }
      leapyear = (year != 0) && (((year%4 == 0) && (year%100 != 0)) || (year%400
          == 0));
      if (leapyear && (restdays > 59)) {
        if (day > 1) {
          day = day - 1;
        } 
        else if (month == 3) {
          month = 2;
          day = 29;
        } 
        else if ((month == 4) || (month == 6) || (month == 8) || (month == 
            9) || (month == 11)) {
          month = month - 1;
          day = 31;
        } 
        else {
          month = month - 1;
          day = 30;
        }
      }
    } 
    else {
      year = 0;
      month = 0;
      day = 0;
    }
    //divide = div(newValue.instant, 1);
    int test = (int)newValue;
    long minutes = Math.round((newValue - (double)test)*1440.0);
    //divide = div(minutes.instant, 60);
    hour = (int)minutes/60;
    minute = (int)minutes%60;
    String m = (day < 10) ? "0" : "";
    m = m + day + ".";
    if (month < 10)
      m = m + "0";
    m = m + month + "." + year + " ";
    if (hour < 10)
      m = m + "0";
    m = m + hour + ":";
    if (minute < 10)
      m = m + "0";
    m = m + minute;
    //	System.out.println(m);
    return  m;
  }

  /**
   * Convert a time in separated integers into a double-value 
   * @param day The day value
   * @param month The month value
   * @param year The year value
   * @param hour The hour value
   * @param minute The minute value
   * @return The converted time as double
   * @see <a href="LEUtilssrc.html#convertDateTime2Double">Source</a> 
   */
  public static double convertDateTime2Double (int day, int month, int year, 
      int hour, int minute) {
    double[] monthoff =  {
      0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
    };
    double days = 0.0;
    double minutes = 0.0;
    double result = 0.0;
    boolean leapyear = (year != 0) && (((year%4 == 0) && (year%100 != 0)) || 
        (year%400 == 0));
    days = year;
    days = days*365;
    //   divide = div(year, 4);
    //   test   = divide.quot;
    days = days + year/4;
    //   divide = div(year, 100);
    //   test   = divide.quot;
    days = days - year/100;
    //   divide = div(year, 400);
    //   test = divide.quot;
    days = days + year/400;
    if (leapyear && ((month == 1) || (month == 2))) {
      days = days - 1.0;
    }
    days += monthoff[month] + day;
    minutes = hour*60.0 + minute;
    result = days + (minutes/1440.0);
    //System.out.println(result);
    result = result - NULL_DAY;
    //System.out.println(result);
    return  result;
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
        System.out.println("Error: No correct numeric expression: rat or int-type needed");
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



