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


/*
This class scans a trace of a gps receiver from a file and evaluates
the GPGGA information. This class only used the height information to write
a moving real to the standard output. 

*/

import java.io.*;
import java.util.*;

public class NMEA2MReal{


private static long days = 0;
private static long hours = 0; // difference between local time and utc in milliseconds
private static double lastTime = -100; // invalid value
private static final long DAYMILLIS=86400000;


/** Return the value of this character as digit.
  * Note that no check is made that c contains
  * actually a digit.
  **/
private static int c2i(char c){
   return (int) (c-'0');
}

/** Converts a String reprenting time in format
  * hhmmss.ss[s] into a Long value represenenting the
  * milliseconds from the beginning of a day up to the
  * given time.
  **/
public static Long getTime(String time){
  // check format
  if(time.length()!=9 && time.length()!=10)
      return null;
  if(time.charAt(6)!='.')
      return null;
  int h = c2i(time.charAt(0))*10 + c2i(time.charAt(1));
  int m = c2i(time.charAt(2))*10 + c2i(time.charAt(3));
  int s = c2i(time.charAt(4))*10 + c2i(time.charAt(5));
  int ms = c2i(time.charAt(7))*100 + c2i(time.charAt(8))*10;
  if(time.length()==10)
     ms += c2i(time.charAt(9));
  long t = ((h*60+m)*60+s)*1000+ms; 
  return new Long(t);
}


/** Processes a single date. **/
public static void processData(String Time, String Height1,String Height2){
   Long T = getTime(Time);
   if(T==null){
       System.err.println("Error in computing time from: "+ Time);
       return;
   } 
   double alt1,alt2,alt;
   try{
      alt1 = Double.parseDouble(Height1);
      alt2 = Double.parseDouble(Height2);
   } catch(Exception e){
     System.err.println("Error in computation of the height from "+Height1+","+Height2);
     return;
   }
   alt = alt1-alt2;


   double t = (T.doubleValue()+hours)/DAYMILLIS+days;
   if(t<lastTime){ // a new day
       days++;
       t += 1;
   }
   lastTime = t;   
   
   unitWriter.appendPoint(alt,t);
}

/** Processes a single line from the input **/
private static void processLine(String line){
  if(!line.startsWith("$GPGGA")){
      return;
  }
  StringTokenizer ST = new StringTokenizer(line,",");
  if(ST.countTokens()!=15){
     System.err.println("Error in processing line "+line);
  }

  String Time,Alt1,Alt2,Qual;
  ST.nextToken(); // GPGGA
  Time = ST.nextToken();
  ST.nextToken(); // Lat
  ST.nextToken(); // north - south
  ST.nextToken(); // lon
  ST.nextToken(); // east - west
  Qual = ST.nextToken(); // quality
  if(Qual.equals("0")){
     System.err.println("Quality invalid ");
     return;
  }

  ST.nextToken(); // number of satellites
  ST.nextToken(); // horizontal dilution
  Alt1 = ST.nextToken(); // antenna altitude
  // the other values are not of interest
  ST.nextToken(); // ignore unit (meters)
  Alt2 = ST.nextToken(); 
  processData(Time,Alt1,Alt2);
}

/** Prints a explanation of the arguments for this programm and exists **/
private static void showUsage(){
   System.err.println(" java NMEA2Secondo [filename [dayoffset  [houroffset [epsilon]]]]  [>outfile]");
   System.err.println(" where");
   System.err.println(" filename: the name of the file to convert or '-' for standardinput ");
   System.err.println(" dayoffset: an integer number describing the distance to the 'NULLDAY' ");
   System.err.println("             have a look to the DateTimeAlgebra for details about this date (see README)");
   System.err.println("houroffset: an integer value describing the differenz between the local time and utc");
   System.err.println(" epsilon: maximum variance for summartizing units ");
   System.exit(0);
}


/** The main function.
  * The arguments are 
  *  0 : the filename or '-' for standard input
  *  1 : offset to the NULLDAY (in days as integer)
  *  2 : EPSILON for summarizing units
  **/

public static void main(String[] args){
   BufferedReader in=null;
   if(args.length>0 && !args[0].equals("-")){
       try{
         in = new BufferedReader(new InputStreamReader(new FileInputStream(args[0])));
       }catch(Exception e){
          System.err.println("Error in open file " + args[0]);
          System.exit(0);
       }

   } else{
     in=new BufferedReader(new InputStreamReader(System.in));
   }
   // evaluate the arguments
   boolean writeOnlyValue=false;
   try{
     int l = args.length;
     if(l>0){
       if(args[0].equals("-?") || args[0].equals("--help") || args[0].equals("-h"))
          showUsage();
     }

     if(l>1){
        if(args[1].toLowerCase().equals("file")){
            long ft = (new File(args[0])).lastModified();
            ft = ft / DAYMILLIS;
            /* 10959 : difference between Nullday used in Java ( 1970-01-01 ) and
                       nullday used in Secondo's DateTimeAlgebra (2000-01-03)
            */
            days = ft - 10959;
        } else
            days = Long.parseLong(args[1]);
     }
     if(l>2){
        if(args[2].toLowerCase().equals("local")){
           long date = (days+10959)*DAYMILLIS+43200000;  // to java nullday + 12 hours
           hours = java.util.TimeZone.getDefault().getOffset(date);
        }else
           hours=Integer.parseInt(args[2])*60*60*1000;
     }
     if(l>3)
        UnitWriter.EPSILON=Double.parseDouble(args[3]);
     if(l>4){
        writeOnlyValue= "-value".equals(args[4]);
     }
   } catch(Exception e){
      showUsage();
   }
   if(!writeOnlyValue){
      System.err.println("convert data with ethe following parameters:");
      System.err.println(" file : "+args[0]);
      System.err.println(" dayoffset :; " + days );
      System.err.println(" houroffset : " + hours );
      System.err.println(" epsilon    : " + UnitWriter.EPSILON);
    }


   String line;
   // print header
   if(!writeOnlyValue){
     System.out.println("( OBJECT pm"+args[0].replace('.','_') + "()");
     System.out.println("   mreal");
   }
   System.out.println("   (");
   try{
      line = in.readLine();
       while(line!=null){
          processLine(line);
          line = in.readLine();
       }
			 in.close();
	 }catch(Exception e){
     System.err.println("Error in processing file");
   }
   unitWriter.write(); // write the last unit
   System.out.println(")");
   if(!writeOnlyValue){
     System.out.println(" () )"); // close value list model mapping and close object
     unitWriter.printStatistic();
   }
   else
     System.out.println();
}


private static UnitWriter unitWriter = new UnitWriter();



/* unit writer for moving reals */

public static class UnitWriter{


/** Appends a new point in time
  **/
boolean appendPoint(double v, double t){
  if(!initialized){ // the first point
    v1 = v;
    t1 = t;
    initialized = true;
    return true; 
  }

  if(t<=t1) // time must be more than before
     return false;

  if(!isComplete){ // only one point is stored
    v2 = v;
    t2 = t;
    double dt = (t2-t1);
    // we compute the change of the coordinates within 1000 milliseconds
    dV = (v2-v1)/dt;
    isComplete=true;
    return true;
  }
  // check whether the new points are a extension
  // of the existing unit
  double dt = (t-t1);
  double ev = v1 + dV*dt;

  if(Math.abs(ev-v)<=EPSILON){ 
     // the expected points a near enough to ignore 
     // intermediate points
     t2 = t;
     v2 = v;
     skippedPoints++;
     return true;
  }
  // the new point is not a extension of the movement
  // write the old unit
  write();
  // and build a new one
  v1 = v2;
  t1 = t2;
  v2 = v;
  t2 = t;
  written = false;
  dt = (t2-t1);
  dV = (v2-v1)/dt;
  isComplete=true;
  return true;
}





/** Writes this unit to the standard output **/
boolean write(){
   double a,b,c;
   if(!isComplete)
      return false;
    System.out.print("    ("); // open unit
    System.out.print("("); // open interval
    System.out.print(t1+" "+t2);
    System.out.print(" TRUE FALSE"); // closeness of interval
    System.out.print(")"); // close interval
    System.out.print("("); // open map
   
    // we assume a linear change of the altitude
    a=0.0;  // no quadratic part
   /* old version */
   //    b=(v1-v2)/(t1-t2);
   // c=v1-t1*b; 
    /* new version */
    c = v1;
    b = (v2-c)/(t2-t1); 
    System.out.print(a+" "+b+" "+c+" ");
    System.out.print(" FALSE"); // no square root

    System.out.print(")"); // close map
    System.out.println(")"); // close unit
    written = true;
    writtenUnits++;
    return true;
}


public static void printStatistic(){
   System.err.println("written Units  : " + writtenUnits );
   System.err.println("skipped Points : " + skippedPoints);
}

// is true if the first point in time is stored
private boolean initialized = false;
private boolean isComplete = false;
private double lastTime=0.0;
private double v1 = 0.0;
private double t1 = 0.0;
private double v2 = 0.0;
private double t2 = 0.0;
private double dV = 0.0;
private boolean written = false;

static double EPSILON=0.00000;

private static long writtenUnits = 0;
private static long skippedPoints = 0;

}




}
