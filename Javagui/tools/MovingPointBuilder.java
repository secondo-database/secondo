package tools;
import sj.lang.ListExpr;
import java.io.*;
import java.util.*;
import viewer.hoese.DateTime;

public class MovingPointBuilder{

private int line_number=0;

private String readLine(BufferedReader R) throws IOException{
  String line;
  do{
   line = R.readLine();
   if(line==null)
      return null;
   line = line.trim();
   line_number++;
  } while(line.equals("") | line.startsWith("#"));
  return line;

}

/*
   - FileName   : the File containing datas
   - offsetTime : the starttime of the first moving point
   - repeats    : number of repeats
   - distance   : time distance between repeats
*/
public MovingPointBuilder(String FileName, double offsetTime,boolean reverse,int repeats,int distance){
  try{
     BufferedReader R = new BufferedReader(new FileReader(FileName));
     Vector PointLists = new Vector();
     Vector Durations = new Vector();
     while(R.ready()){
	pointlist pl = new pointlist(reverse);
	String line;
	line = readLine(R);
	if(line!=null){
	   int secduration=0;
	   try{
              secduration = Integer.parseInt(line);
           }catch(Exception e){
             System.err.println("error in parsing duration " +line_number);
	     System.exit(0);
	   }
	   line = readLine(R);
	   if(line==null || !line.equals("<points>")){
              System.err.println("wrong fileformat"+line_number);
	      System.exit(1);
	   }
	   line = readLine(R);
	   while(!line.equals("</points>")){
	      point p = new point(line);
	      if(p.isDefined())
                 pl.add(p);
	       else{
	          System.err.println("undefined point at line "+line_number+"  :"+line);
		  System.exit(0);
	       }
	      line = readLine(R);
           }
	   double duration = DateTime.getTime(0,secduration*1000);
	   PointLists.add(pl);
           Durations.add(new Double(duration));
	}
     }

     // output the result
     System.out.println("(");  // open object
     System.out.println("   (rel (tuple ( (zug movingpoint))))"); // type description
     System.out.println("(");  // open value list


     int index;
     int size = PointLists.size();
     double td = DateTime.getTime(0,distance*1000);

     for(int r=0;r<=repeats;r++){
        double CurrentTime = offsetTime+r*td;
        System.out.println("( ("); // open tuple and attribute one
        for(int i=0;i<size;i++){
            index = reverse ? size-1-i : i;
            pointlist pl =  (pointlist) PointLists.get(index);
            double duration = ((Double) Durations.get(index)).doubleValue();
	    String units = pl.getUnitsString(CurrentTime,duration);
	    CurrentTime += duration;
	    System.out.println(units+"\n");
        }
        System.out.println("))"); // close attribute one and tuple
      }

     System.out.println("\n ))"); // close valuelist and object
  }catch(Exception e){
     System.err.println("an error is occurred");
     e.printStackTrace();
     System.exit(1);
  }
}


public static void main(String[] args){
  if(args.length<2 | args.length>5){
     System.out.println("usage: java MovingPointBuilder [-r] FileName offsetTime [repeats distance]");
     System.exit(1);
  }
  int no_options=0;
  boolean reverse=false;
  if(args.length==3 | args.length==5){
      if(!args[0].toLowerCase().equals("-r")){
         System.out.println("usage: java MovingPointBuilder [-r] FileName offsetTime [repeats distance]");
         System.exit(1);
      }
      no_options=1;
      reverse=true;
  }
  Double Offset = DateTime.getDateTime(args[1+no_options]);
  if(Offset==null){
     System.err.println("Timeformat: YYYY-MM-DD-hh:mm:ss:mmm");
     System.exit(0);
  }

  int repeats=0;
  int distance=0;
  if(args.length>3){
      try{
         int r = Integer.parseInt(args[no_options+2]);
         int d = Integer.parseInt(args[no_options+3]);
	 if(r<0){
	    System.err.println("repeats must be greater then zero");
	    System.exit(1);
	 }
	 if(r>0){
	    repeats=r;
	    if(d<=0){
	       System.err.println("distance must be greater then zero");
	       System.exit(1);
	    }
            distance = d;
	 }
      }
      catch(Exception e){
        System.err.println(" repeats and distance must be integer values");
	System.exit(1);
      }
  }


  MovingPointBuilder UB = new MovingPointBuilder(args[no_options],Offset.doubleValue(),reverse,repeats,distance);
}

private class point{

  public point(String s){
    s = s.trim();
    int i = s.indexOf(",");
    defined = false;
    if(i<0){
      return;
    }
    try{
       String s1 = s.substring(0,i);
       String s2 = s.substring(i+1,s.length());
       x = Integer.parseInt(s1);
       y = Integer.parseInt(s2);
       defined=true;
    }catch(Exception e){System.err.println("error in parsing point");}
  }
  public point(int x, int y){
      this.x = x;
      this.y = y;
      defined =true;
   }

   public boolean isDefined(){
      return defined;
   }

   public double distance(point p2){
     return Math.sqrt( (this.x-p2.x)*(this.x-p2.x) + (this.y-p2.y)*(this.y-p2.y));
   }

   public String getListString(){
     return "("+x+" "+y+")";
   }

   public String toString(){
      return ""+x +" "+y;
   }

   private double x;
   private double y;
   private boolean defined;
}

private class pointlist{
  public pointlist(boolean reverse){
     this.reverse = reverse;
  }

  public void add(point p){
     PL.add(p);
  }

  public double length(){
    if(PL.size()<2)
       return 0.0;
    double l=0;
    point p1;
    point p2;
    for(int i=0;i<PL.size()-1;i++){
       p1 = (point) PL.get(i);
       p2 = (point) PL.get(i+1);
       l += p1.distance(p2);
    }
    return l;
  }



  public int getSize(){
     return PL.size();
  }

  public point getPointAt(int index){
    if(reverse)
       index = PL.size()-1-index;
    return (point) PL.get(index);
  }

  public String getUnitsString(double starttime, double duration){
     if(PL.size()<2)
        return null;

      double L = length();
      String res ="";
      double currentTime=starttime;
      double dist = 0;
      double currentduration;
      double allLength=0;
      point p1,p2;
      for(int i=0;i<PL.size()-1;i++){
            p1 = (point) getPointAt(i);
	    p2 = (point) getPointAt(i+1);
            dist = p1.distance(p2);
	    if(L==0)
	       currentduration = duration;
	    else
               currentduration  = duration*dist/L;
	    res +="( \n"; // open unit
            res += "( \n   (datetime " + currentTime +")"+"\n"; // open interval
	    currentTime += currentduration;
            res += "   (datetime " + currentTime +")"+"\n";
	    res += "   TRUE \n     FALSE )\n";  // close interval
            res += "  ( "+p1+" "+p2+"))\n\n";   // open points close points close unit
      }
      return res;
  }

  private Vector PL= new Vector();
  private boolean reverse;

}


}
