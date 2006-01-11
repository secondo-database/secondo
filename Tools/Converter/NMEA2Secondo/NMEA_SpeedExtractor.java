import java.io.*;
import java.util.*;


/** little tool for extracting the speed information from a 
  * GPS Receiver. For Using this tool , the GPGGA as well as
  * the GPVTG data set are required.
  **/

public class NMEA_SpeedExtractor{
private static long days = 0;
private static long hours = 0; // difference between local time and utc in milliseconds
private static double lastTime = -100; // invalid value
private static final long DAYMILLIS=86400000;
private static boolean oldStyle=false;


private static void processData(String Time, String Speed){
   Long T = NMEA2MReal.getTime(Time);
   if(T==null){
       System.err.println("Error in computing time from: "+ Time);
       return;
   }
   double speed=0;
   try{
      speed = Double.parseDouble(Speed);
   } catch(Exception e){
     System.err.print("Error in computation of the height from "+Speed);
     System.err.println(" in line " + line);
     System.err.println("Line: "+currentLine);
     return;
   }
   double t = (T.doubleValue()+hours)/DAYMILLIS+days;
   if(t<lastTime){ // a new day
       days++;
       t += 1;
   }
   lastTime = t;

   unitWriter.appendPoint(speed,t);


}


private static void processLine(String Line){
    line++;
    currentLine=Line;
    if(Line.startsWith("$GPGGA")){
        MyStringTokenizer ST = new MyStringTokenizer(Line,',');
        ST.nextToken();
        LastTime = ST.nextToken();
    } else if(Line.startsWith("$GPVTG")){
        if(LastTime==null) return;
        MyStringTokenizer ST = new MyStringTokenizer(Line,',');
        ST.nextToken(); // gpgga
        ST.nextToken(); //
        ST.nextToken(); //
        ST.nextToken(); //
        ST.nextToken(); //
        ST.nextToken(); //
        ST.nextToken(); //
        String Speed = ST.nextToken();
        processData(LastTime,Speed);
        LastTime=null;
    }

}


public static void showUsage(){
    System.err.println("no help available");
    System.exit(0);

}

public static void main(String[] args){
   BufferedReader in=null;
   int start=0;
   if(args.length>0 && args[0].equals("--oldstyle")){
       oldStyle=true;
       start++;
   }

   if(args.length>start && !args[start].equals("-")){
       try{
         in = new BufferedReader(new InputStreamReader(new FileInputStream(args[start])));
       }catch(Exception e){
          System.err.println("Error in open file " + args[start]);
          System.exit(0);
       }

   } else{
     in=new BufferedReader(new InputStreamReader(System.in));
   }
   // evaluate the arguments
   boolean writeOnlyValue=false;
   try{
     int l = args.length;
     if(l>start){
       if(args[start].equals("-?") || args[start].equals("--help") || args[start].equals("-h"))
          showUsage();
     }

     if(l>start+1){
        if(args[start+1].toLowerCase().equals("file")){
            long ft = (new File(args[start])).lastModified();
            ft = ft / DAYMILLIS;
            /* 10959 : difference between Nullday used in Java ( 1970-01-01 ) and
                       nullday used in Secondo's DateTimeAlgebra (2000-01-03)
            */
            days = ft - 10959;
        } else
            days = Long.parseLong(args[start+1]);
     }
     if(l>start+2){
        if(args[start+2].toLowerCase().equals("local")){
           long date = (days+10959)*DAYMILLIS+43200000;  // to java nullday + 12 hours
           hours = java.util.TimeZone.getDefault().getOffset(date);
        }else
           hours=Integer.parseInt(args[start+2])*60*60*1000;
     }
     if(l>start+3)
        NMEA2MReal.UnitWriter.EPSILON=Double.parseDouble(args[start+3]);
     if(l>start+4){
        writeOnlyValue= "-value".equals(args[start+4]);
     }
   } catch(Exception e){
      showUsage();
   }
  if(!writeOnlyValue){
      System.err.println("convert data with ethe following parameters:");
      System.err.println(" file       : "+args[start]);
      System.err.println(" oldStyle   : " + oldStyle);
      System.err.println(" dayoffset  : " + days );
      System.err.println(" houroffset : " + hours );
      System.err.println(" epsilon    : " + NMEA2MReal.UnitWriter.EPSILON);
    }


   String line;
   // print header
   if(!writeOnlyValue){
     System.out.println("( OBJECT pm"+args[start].replace('.','_') + "()");
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
     if(oldStyle)
         System.out.println(" () )"); //  model  and close object
     else
         System.out.println(" )"); //  no model
     unitWriter.printStatistic();
   }
   else
     System.out.println();


}

private static String LastTime=null;
private static NMEA2MReal.UnitWriter unitWriter = new NMEA2MReal.UnitWriter();
private static int line=0; 
private static String currentLine;


}
