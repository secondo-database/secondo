/* 
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

   file : viewer/hoese/DateTime.java
   last change: 8-2003 , Thomas Behr */
package viewer.hoese;

import tools.Reporter;


/* this class proovides transformations between internal double representation of a
   datetime and the external representation as year month day hour min sec msec
   */
public class DateTime{

/** Represents the maximum representable instant.
  * Ensure the same representation as in the DateTime-Algebra of
  * the Secondo kernel.
  **/
private static final String endOfTime = "end of time";
/** String describing the minimum representable instant.
  * Ensure to use the same name as in the DateTime Algebra in the 
  * Secondo kernel.
   **/
private static final String beginOfTime = "begin of time"; 

/** Value of the maximum representable instant.
  * Ensure the same value as in the DateTime-Algebra of
  * the Secondo kernel.
  **/
private static final long minInstant = -2450000;

/** Value of the minimum representable instant.
  * Ensure the same value as in the DateTime-Algebra of
  * the Secondo kernel.
  **/
private static final long maxInstant = 2450000;

/** The resolution of a day.
  * A single day is divided into DAY_RESOLUTION parts.
  **/
public static final long DAY_RESOLUTION = 86400000;  // 1 millisecond




/* convert the given values to a Double representation */
public static double convertToDouble(int year, int month, int day, int hour, int minute, int second, int millisec){
   double Days = (double) (JulianDate.toJulian(year,month,day));
   double ms   = (double) getMilliSecs(hour,minute,second,millisec)/86400000;
  return Days+ms;
}


public static int getMilliSecs(double time){
  int res =  (int)Math.round((time - (int)time)*86400000);
  if(time<0)
     return 86400000+res; //res<0
  else
     return  res;
}



public static int getDays(double time){
  int Days = (int)time;
  int rest = getMilliSecs(time);
  if(Days<0 & rest>0)
     Days--;
  return Days;
}

public static double getTime(int Days, int msecs){
  return Days+(double)msecs/86400000.0;
}

public static String getString(int Days,int msecs){
   if(Days<0 && msecs>0)
      Days--;
  int[] Greg = JulianDate.fromJulian(Days);
  String res = Greg[0]+"-";
  if(Greg[1]<10)
     res +="0";
  res+=Greg[1]+"-";
  if(Greg[2]<10)
     res+="0";
  res+=Greg[2]+"-";
  int rest = msecs;
  int ms = rest%1000;
  rest = rest/1000;
  int sec = rest%60;
  rest = rest /60;
  int min = rest%60;
  rest = rest/60;
  int hour=rest;
  if(hour<10)
     res+="0";
  res = res+hour+":";
  if(min<10)
     res = res +"0";
  res=res+min+":";
  if(sec<10)
    res=res+"0";
  res=res+sec+".";
  if(ms<100)
    res =res+"0";
  if(ms<10)
    res = res+"0";
  res = res+ms;
  return res;
}


/** returns a String representation for the given time */
public static String getString(double time){
  int Days = (int)time;
  int rest = getMilliSecs(time);
  return getString(Days,rest);
}

private static int getMilliSecs(int hour,int minute,int second,int millisecond){
   return ((hour*60+minute)*60+second)*1000+millisecond;
}


public static String getListString(int Days,int msecs,boolean asDuration){
  if(!asDuration){
     int[] Greg = JulianDate.fromJulian(Days);
     String res = "(instant ("+Greg[2]+" "+Greg[1]+" "+Greg[0]+"  ";
     int rest = msecs;
     int ms = rest%1000;
     rest = rest/1000;
     int sec = rest%60;
     rest = rest /60;
     int min = rest%60;
     rest = rest/60;
     int hour=rest;
     res = res+hour+" ";
     res=res+min+" ";
     res=res+sec+" ";
     res = res+ms +"))";
     return res;
  } else{ // as Duration
     return "(duration ("+Days+" "+msecs+"))";
  }
}

public static String getListStringOld(int Days,int msecs){
  int[] Greg = JulianDate.fromJulian(Days);
  String res = "(datetime "+Greg[2]+" "+Greg[1]+" "+Greg[0]+"  ";
  int rest = msecs;
  int ms = rest%1000;
  rest = rest/1000;
  int sec = rest%60;
  rest = rest /60;
  int min = rest%60;
  rest = rest/60;
  int hour=rest;
  res = res+hour+" ";
  res=res+min+" ";
  res=res+sec+" ";
  res = res+ms +")";
  return res;
}

/** returns a String representation for the given time */
public static String getListString(double time,boolean AsDuration){
  int Days = (int)time;
  int rest = getMilliSecs(time);
  return getListString(Days,rest,AsDuration);
}

/** returns a String representation for the given time */
public static String getListStringOld(double time){
  int Days = (int)time;
  int rest = getMilliSecs(time);
  return getListStringOld(Days,rest);
}



/** returns an array containing the day and the milliseconds of this day
  * if the Format is not correct null will be returned
  */
public static long[] getDayMillis(String DT){
  if(DT.equals(beginOfTime)){
     long[] res = new long[2];
     res[0] = minInstant;
     res[1] = 0;
     return res;
  }
  if(DT.equals(endOfTime)){
     long[] res = new long[2];
     res[0] = maxInstant;
     res[1] = DAY_RESOLUTION;
     return res;
  }

  char[] D = DT.toCharArray();
  int len = D.length;
  int i = 0;
  int year = 0;
  int digit = 0;
  //read year;
  while(i<len && D[i]!='-'){
    digit = getValue(D[i]);
    if(digit<0) return null;
    year = year*10+digit;
    i++;
  }
  if(i>=len) return null;
  // read month
  int month = 0;
  i++;
  while(i<len && D[i]!='-'){
    digit = getValue(D[i]);
    if(digit<0) return null;
    month = month*10+digit;
    i++;
  }
  if(i>=len) return null;
  // read day
  int day = 0;
  i++;
  while(i<len && D[i]!='-'){
    digit = getValue(D[i]);
    if(digit<0) return null;
    day = day*10+digit;
    i++;
  }
  if(i>=len){  // only date is allowed
     long[] res = new long[2];
     res[0] = (JulianDate.toJulian(year,month,day));
     res[1] = 0;
     if(res[0] < minInstant){
        res[0] = minInstant;
     } else if(res[0]>maxInstant){
        res[0] = maxInstant;
     }
     return res;
   }
  // read hour
  int hour = 0;
  i++;
  while(i<len && D[i]!=':'){
    digit = getValue(D[i]);
    if(digit<0) return null;
    hour = hour*10+digit;
    i++;
  }
  // read minute
  int min=0;
  int sec =0;
  int msec =0;
  if(i>=len) return null;
  // read minute
  i++;
  while(i<len && D[i]!=':'){
    digit = getValue(D[i]);
    if(digit<0) return null;
    min = min*10+digit;
    i++;
  }
  // read second
  i++;
  while(i<len && D[i]!='.'){
    digit = getValue(D[i]);
    if(digit<0) return null;
    sec = sec*10+digit;
    i++;
  }
  // read millisecond
  i++;
  while(i<len){
    digit = getValue(D[i]);
    if(digit<0) return null;
     msec = msec*10+digit;
    i++;
  }
  long[] res = new long[2];
  res[0] = (JulianDate.toJulian(year,month,day));
  res[1] = getMilliSecs(hour,min,sec,msec);
  if(res[0]<minInstant){
    Reporter.writeWarning("found instant outside the valid boundaries (changed)");
    res[0] = minInstant;
    res[1] = 0;
  } else if(res[0]>maxInstant){
    Reporter.writeWarning("found instant outside the valid boundaries (changed)");
    res[0] = maxInstant;
    res[1] = DAY_RESOLUTION;
  }
  return res;

}



/** return the datevalue for DT,
  * return 0 if format ist not correct
  * required format  YYYY-MM-DD-HH:mm:ss.mmm
  */
public static Double getDateTime(String DT){
  long[] tmp = getDayMillis(DT);
  if(tmp==null) return null;
  return new Double(getTime((int)tmp[0],(int)tmp[1]));

}

private static int getValue(char c){

  switch(c){
    case '0' : return 0;
    case '1' : return 1;
    case '2' : return 2;
    case '3' : return 3;
    case '4' : return 4;
    case '5' : return 5;
    case '6' : return 6;
    case '7' : return 7;
    case '8' : return 8;
    case '9' : return 9;
    case ' ' : return 0;
    default  : return -1;
  }

}




}



