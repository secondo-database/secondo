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


public static final long DAY_RESOLUTION = 86400000;  // 1 millisecond


}



