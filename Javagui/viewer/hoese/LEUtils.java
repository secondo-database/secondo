//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


package  viewer.hoese;

//import generic.Interval;
import  sj.lang.ListExpr;
import viewer.HoeseViewer;
import gui.Environment;
import tools.Reporter;
import java.util.StringTokenizer;
import java.math.BigInteger;


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
         if(le.first().atomType()==ListExpr.SYMBOL_ATOM &&
	      ( le.first().symbolValue().equals("instant") ||
	        le.first().symbolValue().equals("datetime"))){
                value = le.second();	  // read over the type

         } else
	     value = le;
      } else
         value = le;


      // string representation
      if(value.atomType()==ListExpr.STRING_ATOM){
         long[] daymillis = DateTime.getDayMillis(value.stringValue());
         if(daymillis==null)
	    return null;
	 else
	    return  Double.valueOf((double)daymillis[0]+(double)daymillis[1]/86400000.0);
      }

      // real representation
      if(value.atomType()==ListExpr.REAL_ATOM){
          return Double.valueOf(value.realValue());
      }

      // integer representation
      if(value.atomType()==ListExpr.INT_ATOM){
          return Double.valueOf(value.intValue());
      }

      // all other representation are based on proper lists
      // not on atoms
      if(value.atomType()!=ListExpr.NO_ATOM)
          return null;

      int length = value.listLength();
      // Julian representation
      if(length==2){
         if(value.first().atomType()==ListExpr.INT_ATOM &&
            value.second().atomType()==ListExpr.INT_ATOM){
             double day = (double) value.first().intValue();
             double ms  = (double) value.second().intValue();
             return Double.valueOf(day + ms /86400000.0);
          } else
             return null;
      }

      // Gregorian representation

      if(value.isEmpty())
         return null;

      //check for date in format (date day month year)
      if(length==4 &&
         value.first().atomType()==ListExpr.SYMBOL_ATOM &&
         value.first().symbolValue().equals("date")){
         Reporter.writeWarning("Deprecated version of instant (date day month year");
         value=value.rest(); //ignore "date"
         if(value.first().atomType()!=ListExpr.INT_ATOM ||
            value.second().atomType()!=ListExpr.INT_ATOM ||
            value.third().atomType()!=ListExpr.INT_ATOM)
           return null;
         return Double.valueOf(convertDateTime2Double(value.first().intValue(),
                                                  value.second().intValue(),
                                                  value.third().intValue(),0,0,0,0)); 
      }


      // check for an deprecated data version
      if(value.first().atomType()==ListExpr.SYMBOL_ATOM &&
         value.first().symbolValue().equals("datetime")){
         Reporter.writeWarning("Deprecated nested list representation of time !!");
       	 value = value.rest();
         length = length-1;
      }

      // at least (day month year) and has to be included
      // at most (day month year hour minute second millisecond) can be included
      if(length<3 || length>7)
          return null;

      // all contained atoms has to be integers
      ListExpr tmp = value;
      while(!tmp.isEmpty()){
          if(tmp.first().atomType()!=ListExpr.INT_ATOM)
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
       return Double.valueOf(convertDateTime2Double(day,month,year,hour,minute,second,milli));
  }

  /**
   * Analyses a ListExpr by scanning through the type and value ListExpr
   * @param type The datatype
   * @param value the value of the datatype
   * @param qr Collects the results
   */
  public static void analyse (String name, int nameWidth, int indent,ListExpr type, ListExpr value, QueryResult qr) {
    DsplBase db;
    if (type.isAtom()) {
      db = getClassFromName(type.symbolValue());
      db.init(name,nameWidth,indent, type,  value, qr);
    }
    else {
      db = getClassFromName(type.first().symbolValue());
      db.init(name, nameWidth, indent, type, value, qr);
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

  /** Converts a String representing an instant into a double value 
    **/
   public static Double convertStringToTime(String s){
      long[] daymillis = DateTime.getDayMillis(s);
      if(daymillis==null){
          return null;
	    } else {
         return  Double.valueOf((double)daymillis[0]+(double)daymillis[1]/86400000.0);
      }
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
     int length = le.listLength();
     if (length != 4){
      Reporter.debug("wrong listlength for interval (needed is 4) :" + length);
      return  null;
    }
    Double start = readInstant(le.first());
    if(start==null){
       Reporter.debug("Error in reading start - instant \n"+
                      le.first().writeListExprToString());
       return null;
    }
    Double end = readInstant(le.second());
    if(end==null){
       Reporter.debug("Error in reading end instant ");
       return null;
    }
    if ((le.third().atomType() != ListExpr.BOOL_ATOM)
        || (le.fourth().atomType() != ListExpr.BOOL_ATOM)){
      Reporter.debug("not boolean atoms for lefttclosed or rightclosed");
      return  null;
    }
    boolean leftcl = le.third().boolValue();
    boolean rightcl = le.fourth().boolValue();
    return  new Interval(start.doubleValue(), end.doubleValue(), leftcl, rightcl);
  }

  /** checks whether the given character is a letter **/
  public static boolean isLetter(char c){
    return (c>='a' && c<='z') ||
           (c>='A' && c<='Z');
  }

  /** checks whether the given character is a digit **/
  public static boolean isDigit(char c){
      return c>='0' && c<='9';
  }  
  


   /** checks whether the given String is a correct symbolvalue **/
  public static boolean isIdent(String s){
     // checks s for the expression  {letter}({letter}|{digit}|{underscore})*
      if(s==null)
          return false;
      int len = s.length();
      if(len==0)
          return false;
      char c = s.charAt(0);
      if(!isLetter(c))
          return false;
      for(int i=len-1;i!=0;i--)
          if(!isLetter(c) && !isDigit(c) && c!='_')
               return false;
      return true;
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
        Reporter.writeError("Error: No correct numeric expression: rat or int or real-type needed");
        return  null;
      }
      if (le.atomType()==ListExpr.INT_ATOM)
         return  Double.valueOf(le.intValue());
      else
         return Double.valueOf(le.realValue());
    }
    else {
      int length = le.listLength();
      if ((length != 5)&& (length != 6)){
        Reporter.writeError("Error: No correct rat expression: 5 or 6 elements needed");
        return  null;
      }
      if (length==5) {
         if ((le.first().atomType() != ListExpr.SYMBOL_ATOM) || (le.second().atomType()
             != ListExpr.INT_ATOM) || (le.third().atomType() != ListExpr.INT_ATOM)
             || (le.fourth().atomType() != ListExpr.SYMBOL_ATOM) || (le.fifth().atomType()
             != ListExpr.INT_ATOM)) {
             Reporter.writeError("Error: No correct rat5 expression: wrong types");
             return  null;
          }
          if ((!le.first().symbolValue().equals("rat")) || (!le.fourth().symbolValue().equals("/"))) {
              Reporter.writeError("Error: No correct rat5 expression: wrong symbols"
                 + le.first().symbolValue() + ":" + le.fourth().symbolValue() +
                  ":");
              return  null;
          }
          double g = (double)le.second().intValue();
          return  Double.valueOf((Math.abs(g) + (double)le.third().intValue()/(double)le.fifth().intValue()));
      }else {
         if ((le.first().atomType() != ListExpr.SYMBOL_ATOM) || (le.second().atomType() != ListExpr.SYMBOL_ATOM)
              ||(le.third().atomType()!= ListExpr.INT_ATOM) || (le.fourth().atomType() != ListExpr.INT_ATOM)
              || (le.fifth().atomType() != ListExpr.SYMBOL_ATOM) || (le.sixth().atomType()
              != ListExpr.INT_ATOM)) {
             Reporter.writeError("Error: No correct rat6 expression: wrong types");
             return  null;
          }
          if ((!le.first().symbolValue().equals("rat")) ||
      	      (!le.fifth().symbolValue().equals("/")  ) ||
      	      !(le.second().symbolValue().equals("-")  ||
      	        le.second().symbolValue().equals("+") )) {
              Reporter.writeError("Error: No correct rat6 expression: wrong symbols"
                  + le.first().symbolValue() + ":" + le.fifth().symbolValue() +
                  ":"+le.writeListExprToString());
              return  null;
          }
          double g = (double)le.third().intValue();
          double v=1;
          if (le.second().symbolValue().equals("-")) v=-1;
          return  Double.valueOf(v*(Math.abs(g) + (double)le.fourth().intValue()/(double)le.sixth().intValue()));
    	}
    }
  }


  public static Double readFrac(ListExpr f){
       if(f.atomType()!=ListExpr.TEXT_ATOM){
          return null;
       }
       String s = f.textValue();
       StringTokenizer st = new StringTokenizer(s,"/");
       if(!st.hasMoreTokens()){
          return null;
       }
       String nomStr = st.nextToken();
       double nom;
       try{
          nom =  (new  BigInteger(nomStr)).doubleValue();       
       } catch(NumberFormatException e){
          return null;
       }
       if(!st.hasMoreTokens()){
          return Double.valueOf(nom);
       } 
       double denom;
       try{
          denom = (new BigInteger(st.nextToken())).doubleValue();  
       } catch(NumberFormatException e){
          return null;
       }
       return Double.valueOf(nom/denom);
  }

}



