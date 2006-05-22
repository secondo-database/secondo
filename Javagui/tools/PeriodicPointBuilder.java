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

package tools;
import sj.lang.ListExpr;
import java.io.*;
import java.util.*;
import viewer.hoese.DateTime;
import viewer.hoese.algebras.periodic.*;

public class PeriodicPointBuilder{

private int line_number=0;

/** Reads the next line from R ignoring empty lines.
 *
 **/
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

/** Writes a relation containing a set of periodic points to the
 * standard output.
 *
 * @param NoT: number of trains to create
 * @param tD: time difference between the trains
 * @param start: start time of the first train
 * @param rep: number of repetitions of each train
 * @param F: file containing the source 
 * @param halfyear: flag indicating whether only a single rep time repetition
 *                  or a complete halfyear should be covered.
**/
public void printPeriodicPoint(int NoT,int tD,Time start,int rep, File F, boolean halfyear){
  point firstPoint=null;
  boolean first = true;
  try{
     line_number=0;
     BufferedReader R = new BufferedReader(new FileReader(F));
     Vector PointLists = new Vector();
     Vector Intervals = new Vector();
     RelInterval completeInterval = new RelInterval();
     completeInterval.set(new Time(),true,false);
     System.out.println("("); // start the object
     System.out.println("    (rel(tuple((No int)(Trip pmpoint))))"); // the type
     System.out.println("    ("); // open value list
     // read the pointlist for this train
     while(R.ready()){
        pointlist pl = new pointlist();
        String line;
        line = readLine(R);// search a non-comment non-empty line
        if(line!=null){
           int secinterval=0;
           try{
              secinterval = Integer.parseInt(line);
           }catch(Exception e){
             System.err.println("error in parsing interval " +line_number);
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
              if(p.isDefined()){
                 pl.add(p);
                 if(first){ // store the very first point
                    first = false;
                    firstPoint = new point(0,0);
                    firstPoint.equalize(p); 
                 }
               }
               else{
                  System.err.println("undefined point at line "+line_number+"  :"+line);
                  System.exit(0);
               }
              line = readLine(R);
           }
           Time T = new Time();
           T.set(0,secinterval*1000);
           RelInterval I = new RelInterval();
           I.set(T,true,false);
           completeInterval.set(completeInterval.getLength().add(T),true,false);
           PointLists.add(pl);
           Intervals.add(I);
        }
     }
     R.close();
   
     completeInterval.set(completeInterval.getLength().mul(2*rep),true,false); 
 
     pointlist Waiting = new pointlist();
     Waiting.add(firstPoint);
     Waiting.add(firstPoint);
     Time oneDay = new Time(1,0);
     Time restDay = oneDay.minus(completeInterval.getLength());
     RelInterval wInterval = new RelInterval();
     wInterval.set(restDay,true,false); 
     
     RelInterval weekend = new RelInterval();
     weekend.set(new Time(2,0),true,false);


     Time Difference = new Time();
     Difference.set(0,tD*1000);
     int index;
     int size = PointLists.size();
     for(int train=0; train<NoT; train++){ // for every train
         System.out.println("      ("); // open tuple
         System.out.println("         "+train); // write the number of train
         // write the train
         System.out.println("          (");
         System.out.println("             "+start.getListExprString(true));
         if(halfyear){
             if(train%2==0){ // trains staying on weekend
                System.out.println("(period ( 21 "); // 21 weeks
                System.out.println("(composite (");  
             } else{ // trains running on weekend
                System.out.println("( period ( 147 "); // 147 days
                System.out.println("(composite (");
             }
         }

         start.addInternal(Difference); // compute the start time for the next train
         System.out.println(" ( period ( "+rep ); // number of repeatations
         System.out.println("     (composite (");
         // write the single runs in forward direction
         for(int i=0;i<size;i++){
            index = i;
            pointlist pl =  (pointlist) PointLists.get(index);
            RelInterval  interval = (RelInterval) Intervals.get(index);
            String units = pl.getUnitsString(interval,false);
            System.out.println(units);
         }
         // write single runs backwards
         for(int i=size-1; i>=0;i--){
            index = i;
            pointlist pl =  (pointlist) PointLists.get(index);
            RelInterval  interval = (RelInterval) Intervals.get(index);
            String units = pl.getUnitsString(interval,true);
            System.out.println(units);
         }
         System.out.println(" ))))"); // close composite , period
         if(halfyear){ // fill up 24 hours
             String units = Waiting.getUnitsString(wInterval,false); // staying after work 
             System.out.println(units);
         } 
        


         if(halfyear){
           if(train%2==0){
               String units = Waiting.getUnitsString(weekend,false); // staying on weekend
               System.out.println(units);
           }
           System.out.println("))))"); // close composite and period
         } 

         System.out.println("))"); //train,tuple
     }
     System.out.println("))"); // close value,object

  }catch(Exception e){
     System.err.println("an error is occurred");
     e.printStackTrace();
     System.exit(1);
  }
}

/** writes the usage of this toll to the standard output and exists the program **/
private static void wrongParameter(String Message){
    System.out.println(Message);
    System.out.println(" Usage : java tools.PeriodicPointBuilder NoT tD start rep FileName");
    System.out.println("where ");
    System.out.println(" NoT   : number of trains ");
    System.out.println(" tD    : Distance between 2 trains in sec ");
    System.out.println(" start : the start time of the first train in yyyy-mm-dd-hh:min:sec");
    System.out.println(" rep   : number cycles for a single train ");
    System.out.println(" NoT   : File to process ");
    System.exit(1);
}

/** main function **/
public static void main(String[] args){
    if(args.length!=5){
       wrongParameter("wrong number of arguments");
    }
    int NoT,tD,rep;
    NoT=tD=rep=0;
    try{
      NoT = Integer.parseInt(args[0]);
      if(NoT<=0) wrongParameter("number of trains has to be greater than zero");
      tD  = Integer.parseInt(args[1]);
      if(tD<=0) wrongParameter("time difference has to be greater than zero");
      rep = Integer.parseInt(args[3]);
      if(rep<=0) wrongParameter("number of cycles has to be greater than zero");
    }catch(Exception e){
       wrongParameter(""+e);
    }
    Time start = new Time();
    if(!start.readFrom(args[2]))
      wrongParameter("invalid format for Time ("+args[2]+")");
    File F = new File(args[4]);
    if(!F.exists())
       wrongParameter("File "+args[4]+" not found");

    (new PeriodicPointBuilder()).printPeriodicPoint(NoT,tD,start,rep,F,true);
}


/** A simple point class.
**/
private class point{

  /** creates a new point instance from the string.
    * If the format of the string soes not build a valid point
    * this means x,y, the resulting point will be undefined 
    **/
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
       x = Double.parseDouble(s1);
       y = Double.parseDouble(s2);
       defined=true;
    }catch(Exception e){System.err.println("error in parsing point");}
  }

  /** creates a defined point from the parameters**/
  public point(double x, double y){
      this.x = x;
      this.y = y;
      defined =true;
   }

   /** Returns the defined state of this point **/
   public boolean isDefined(){
      return defined;
   }

   /** Returns the distance between two points **/
   public double distance(point p2){
     return Math.sqrt((p2.x-x)*(p2.x-x)+(p2.y-y)*(p2.y-y));
   }


   /** Returns the point as a string in nested list format **/
   public String getListString(){
     return "("+x+" "+y+")";
   }

   /** Returns a string representation of this point **/
   public String toString(){
      return ""+x +" "+y;
   }

   /** copies the value from the argument **/
   public void equalize(point p){
      this.x = p.x;
      this.y = p.y;
      this.defined = p.defined;
   }

/** the x coordinate **/
   private double x;
/** the y coordinate **/
   private double y;
/** the defined flag **/
   private boolean defined;
}

/** Class representing a sequence of points **/
private class pointlist{

  /** creates a new empty pointlist **/
  public pointlist(){
  }

  /** Appends a new point to the list **/
  public void add(point p){
     PL.add(p);
  }

  /** Computes the length of this sequence as sum of the 
    * euclidean length of the contained segments 
    **/
  public double length(){
    if(PL.size()<2)
       return 0;
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

  /** Returns the number of the contained points **/
  public int getSize(){
     return PL.size();
  }

  /** Returns the point at the specified position **/
  public point getPointAt(int index){
    if(reverse)
       index = PL.size()-1-index;
    return (point) PL.get(index);
  }

  /** Returns the nested list representation as string of
    * a single unit of a periodic moving point moving on this
    * pointlist during the given interval. The speed will be constant
    * during the whole trip.
    *
    **/
  public String getUnitsString(RelInterval I,boolean reverse){
     if(PL.size()<2)
      return null;
      double  L = length();
      double dist = 0;

      point p1,p2;
      Time AllT = I.getLength().copy();
      RelInterval CurrentI = I.copy();
      Time T = new Time();
      String res = "";
      for(int i=0;i<PL.size()-1;i++){
            if(i>0) res += "\n";
            res +="(linear (";
            if(reverse){
               p1 = (point) getPointAt(PL.size()-(i+1));
               p2 = (point) getPointAt(PL.size()-(i+2));
            } else{
               p1 = (point) getPointAt(i);
               p2 = (point) getPointAt(i+1);
            }
            dist = p1.distance(p2);
            if(L==0)
               CurrentI = I;
            else{
               double P = (dist/L);
               CurrentI.setLength(I.getLength().mul(P));
            }
            res+=(CurrentI.getListExprString());
            res += "  ( "+p1+")( "+p2+")))";   // open points close points close unit
      }
      return res;
  }

  private Vector PL= new Vector();
  private boolean reverse = false;
}

}
