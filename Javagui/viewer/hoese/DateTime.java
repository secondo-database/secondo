/* file : viewer/hoese/DateTime.java
   last change: 8-2003 , Thomas Behr */
package viewer.hoese;


/* this class proovides transformations between internal double representation of a
   datetime and the external representation as year month day hour min sec msec
   */
public class DateTime{

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
  return Days+msecs/86400000;
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
  res+=Greg[2]+"  ";
  String TimeRes = "";
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


public static final long DAY_RESOLUTION = 86400000;  // 1 millisecond


}



