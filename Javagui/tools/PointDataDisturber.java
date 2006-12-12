
//This file is part of SECONDO.

//Copyright (C) 2004-2006, University in Hagen, Faculty of Mathematics and Computer Science, 
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

import viewer.hoese.algebras.Dsplpmpoint;
import viewer.hoese.algebras.periodic.*;
import sj.lang.ListExpr;
import java.io.*;


// This class takes a file containing a single Periodic Moving Point and
// generates a disturbed linear moving point from it.
// In other words, this class tries to generate GPS like data from a perfectly
// running point. 
// Actions are: 
// 0. Get the perfect data each second
// 1. disturb the locations  [ nearly ever ] = simulate measure errors
// 2. remove some single measure points [rare] = simulate bad satelite view
// 3. remove chains of measures [very rare] = simulate no view to a satellite (under a brigde) 

public class PointDataDisturber {


private  Dsplpmpoint pmpoint=null; // contains the point data

private  Time defaultDiff = new Time(0,1000);   // 1 second

private  PrintStream out=System.out;     // the output stream

private double maxError; // the maximum error from the original data

private double maxErrorDiff; // the maximum error difference between two measuresa


/** if this is set to true, he maximum error measure depends on the
  * length of the trajectory to the last measure point
  * The length is multiplied to maxErrorDiff.
  */
private boolean relativeError = true;


private int maxUnits = 12000; // for debugging only // restricts the numbe rof created units


/** stores the last original point **/
private java.awt.geom.Point2D.Double lastPoint; 
/** stores the last applied error in x  and y dimension **/
private java.awt.geom.Point2D.Double lastError; 


/** number of data to delete **/
private int removeSequence = 0;
private int maxSequence = 50;

/** Probability to delete measures  **/
private double removeOneProb = 0.01;  // 1 %
/** Probability to delete a complete Sequence of measures **/
private double removeSeqProb = 0.0001; 


/** the unitwriter */
private UnitWriter writer;

/** the delayManager **/
private DelayManager delayManager= new DelayManager();


/* some statistical information */
/** number of measures **/
private int measures;
/** numbe rof skipped units */
private int skippedUnits;


public PointDataDisturber(){
   pmpoint = null;
   defaultDiff = new Time(0,1000);
   out = System.out;
   maxError=10;
   maxErrorDiff=0.1;
   lastPoint = null;
   lastError = null;
   removeSequence = 0;
   maxSequence = 50;
   writer = new UnitWriter(out,0.0);
   measures=0;
   skippedUnits=0;
}


private void writeStatistic(){
  System.err.println("measures : " + measures);
  System.err.println("skipped units : " + skippedUnits);
  writer.printStatistic();
}


private void addLocation(Time t, java.awt.geom.Point2D.Double p){
  measures++;

  // first, change the error
  double ex=0;
  double ey=0;

  if(lastError==null){ // initialize the error
    ex = maxError*Math.random()-maxError*0.5;
    ey = maxError*Math.random()-maxError*0.5;
    lastError= new java.awt.geom.Point2D.Double(ex,ey);
  } else{
      double dx=0;
      double dy=0;
      if(!lastPoint.equals(p)){ // only change the error if the point is moving
        if(relativeError){
          double length = p.distance(lastPoint);
          double med = length*maxErrorDiff;
          dx = med * 2 * Math.random()  - med;
          dy = med * 2 * Math.random()  - med;
        }else{
          dx = maxErrorDiff * 2 * Math.random()  - maxErrorDiff;
          dy = maxErrorDiff * 2 * Math.random()  - maxErrorDiff;
        }
      }
      ex = lastError.getX()+dx;
      ey = lastError.getY()+dy;
 }
 
 // keep the error in the allowed range
 if(ex>maxError){
    ex = maxError;
 }
 if(ex<-maxError){
    ex = -maxError;
 }

 if(ey>maxError){
    ey = maxError;
 }
 if(ey<-maxError){
    ey = -maxError;
 }


 // store the last values of position and error
 lastPoint = p;
 lastError.setLocation(ex,ey); 
 
 if(removeSequence>0){ // within a removed Sequence
    skippedUnits++;
    removeSequence--;
    return;
 }
 
 double remove = Math.random();  
 if(remove < removeSeqProb){ // remove a whole sequence
    removeSequence = (int)(maxSequence*Math.random());
    skippedUnits++;
    return;
 } 
 if(remove < removeOneProb){ // ignore this measure
     skippedUnits++;
     return;
 }

 // create an inexact measure
 double x = p.getX()+ex;
 double y = p.getY()+ey;
 writer.addMeasure(t,x,y); 
 

}


private void writeHeader(){
  out.println("( mpoint (");
}

private void writeRest(){
  out.println("))"); // close value, close mpoint
}



/** Write the generated data to out
**/
private void createData(){
 Time currentTime = new Time(pmpoint.getMinTime());
 Time max = pmpoint.getMaxTime();
 int writtenUnits=0;
   while((currentTime.compareTo(max)<=0)  && (writtenUnits<maxUnits)){
       Time actualTime=delayManager.getTime(currentTime);
   //    System.err.println("currentTime : " + currentTime+" actualTime "+ actualTime);
   //    Time delay = currentTime.minus(actualTime);
   //    System.err.println("delay is " + delay);

       addLocation(currentTime,pmpoint.getPosition(actualTime));
       currentTime.addInternal(defaultDiff);
       writtenUnits++;
   }
   writer.finish();
}

public static void main(String[] args){
  // the first argument is the file to process
  // in this first implementation we don't use other arguments
  // but use default values
  if(args.length<1){
     System.err.println("Missing filename");
     System.exit(1);
  }
  // get the arguments
  String filename = args[0];



  File F = new File(filename);
  if(!F.exists()){
     System.err.println("File " + filename +" not found");
     System.exit(1);
  }
  
  ListExpr pmpointList = new ListExpr();
  if(pmpointList.readFromFile(filename)!=0){
     System.err.println("Error in Parsing File");
     System.exit(1);
  }

  PointDataDisturber pdd= new  PointDataDisturber();


  pdd.pmpoint = new Dsplpmpoint(); 

  pdd.pmpoint.init(ListExpr.symbolAtom("pmpoint"),pmpointList,null);
  if(pdd.pmpoint.getError()){
     System.err.println("Error in list format");
     System.exit(1); 
  }
  pdd.writeHeader();
  pdd.createData();
  pdd.writeRest();

  pdd.writeStatistic();


}



private static class DelayManager{



private Time scheduledDelay= new Time(0,0);
private Time lastReceivedTime;
private Time lastSendTime;
private double probability = 0.01; // create an event in 1% 

/** maximum dela yin seconds **/
private double maxDelay = 200;

private int state = STATE_NORMAL;

/** returns the current state as String **/
public String getState(int stateCode){
   switch(stateCode){
      case STATE_NORMAL: return "NORMAL";
      case STATE_STOP: return "STOP";
      case STATE_DECELERATION: return "DECELERATION";
      case STATE_HURRY: return "HURRY";
      default: return "UNKNOWN";
   }

}

/** prints the state to cerr **/
private void printState(){
  System.err.println("state "+getState(state));
  System.err.println("lastReceivedTime " + lastReceivedTime);
  System.err.println("lastSendTime " + lastSendTime);
  System.err.println("scheduledDelay " + scheduledDelay.toDurationString()); 
}


/** checks whether origTime &lt;= sendTime **/
private void checkSendTime(Time origTime, Time sendTime){
   if(sendTime==null) return;
   int cmp = origTime.compareTo(sendTime);
   if(cmp<0){
        System.err.println("wrong computation:  sendTime > origTime");
        printState();
        System.exit(1);
   }
}


/** returns the delayed time for a given one */
public Time getTime(Time origTime){

   if(lastReceivedTime==null){ //first call
        lastReceivedTime = origTime.copy();
        lastSendTime = origTime.copy();
        checkSendTime(origTime,lastSendTime);
        return lastSendTime;
   }
   if(origTime.compareTo(lastReceivedTime)<=0){
     System.err.println("DelayManager: wrong order of instants !!");
     return origTime;
   }

   createEvent(); // randomized creation of an event


   if(state==STATE_NORMAL){
       lastSendTime = origTime.copy();
       lastReceivedTime = origTime.copy();
       checkSendTime(origTime,lastSendTime);
       return origTime;
   }

   // process a stop event
   if(state==STATE_STOP){
      // leave the stop state
      if(origTime.minus(lastSendTime).compareTo(scheduledDelay)>=0){
        System.err.println("Change state to HURRY");
        state = STATE_HURRY;
      }      
      lastReceivedTime = origTime.copy();
      checkSendTime(origTime,lastSendTime);
      return lastSendTime.copy(); 
   }

   // process a deceleration
   if(state==STATE_DECELERATION){
     if(origTime.minus(lastSendTime).compareTo(scheduledDelay)>=0){
        state = STATE_HURRY;
        System.err.println("Change state to HURRY");
     }      
     // compute the deceleration between 10 and 90 percent
     double dec = 0.1+Math.random()*0.8;
     Time origDelta = origTime.minus(lastReceivedTime);
     Time delta = origDelta.mul(dec);
     lastSendTime.addInternal(delta);
     lastReceivedTime = origTime.copy();
     checkSendTime(origTime,lastSendTime);
     return lastSendTime.copy(); 
   }

   // process hurry 
   if(state==STATE_HURRY){
     // compute the acceleration between 10 and 20 percent
     double dec = 1.1+Math.random()+0.1;
     Time origDelta = origTime.minus(lastReceivedTime);
     Time delta = origDelta.mul(dec);
     lastSendTime.addInternal(delta);
     // acceleration finished
     if(lastSendTime.compareTo(origTime)>=0){
         lastSendTime.equalize(origTime);
         state = STATE_NORMAL;
         System.err.println("Change state to NORMAL");
     }
     lastReceivedTime = origTime.copy();
     checkSendTime(origTime,lastSendTime);
     return lastSendTime.copy(); 
   }
   lastReceivedTime = origTime.copy();
   System.err.println("Unknown state " + state);
   return null;

}

/** creates a new exception 
  **/
private void createEvent(){
   if(state!=STATE_NORMAL){ // don't create an event in othe states
     return;
   }
   // cancel randomized
   double create= Math.random();
   if(create > probability){
       return;
   }   

   // randomize the value of delay
   int delay = (int)(maxDelay * Math.random() * 1000);
   scheduledDelay = new Time(0,delay); 

   // flip a coin for the kind of event
   if(Math.random()<0.5){
       System.err.println("create a stop of " + (delay/1000) +" seconds");
       state = STATE_STOP;
   } else{
       System.err.println("create a deceleration of " + (delay/1000) +" seconds");
       state = STATE_DECELERATION;
   }
}



/** normal state - no delay **/
private static final int STATE_NORMAL=0; 
/** unscheduled stop until delay is reached **/
private static final int STATE_STOP=1;
/** deceleration until delay is reached **/
private static final int STATE_DECELERATION=2;
/** state to reach the normal position **/
private static final int STATE_HURRY=3;


}





private static class UnitWriter{

private PrintStream out;


public UnitWriter(PrintStream out, double epsilon){
   this.out=out;
   this.epsilon = epsilon;
}


/** Appends a new point in time
  **/
boolean addMeasure(Time T,double x, double y){
  double t = T.getDouble();
  if(!initialized){ // the first point
    x1 = x;
    y1 = y;
    t1 = T.copy();
    initialized = true;
    return true; 
  }

  if(T.compareTo(t1)<=0){ // time must be more than before
     System.err.println("UnitWriter: wrong order of measures detected");
     System.err.println("last time :"+t1+" ne time "+T);
     return false;
  }

  if(!isComplete){ // only one point is stored
    x2 = x;
    y2 = y;
    t2 = T.copy();
    Time dT = t2.minus(t1);
    double dt = dT.getDouble();
    // we compute the change of the coordinates within 1000 milliseconds
    dX = (x2-x1)/dt;
    dY = (y2-y1)/dt;
    isComplete=true;
    return true;
  }
  // check whether the new points are a extension
  // of the existing unit
  Time dT = (T.minus(t1));
  double dt = dT.getDouble();
  // compute the expected position of the points  
  double ex = x1 + dX*dt;
  double ey = y1 + dY*dt;
  //System.err.println("*********************");
  //System.err.println("x =" +x +"   ex = "+ex);
  //System.err.println("y =" +y +"   ey = "+ey);
  //System.err.println("*********************");
  


  if(Math.abs(ex-x)<=epsilon && Math.abs(ey-y)<=epsilon){ 
     // the expected points a near enough to ignore 
     // intermediate points
     t2 = T.copy();
     x2 = x;
     y2 = y;
     skippedPoints++;
     return true;
  }
  // the new point is not a extension of the movement
  // write the old unit
  write();
  // and build a new one
  x1 = x2;
  y1 = y2;
  t1 = t2;
  x2 = x;
  y2 = y;
  t2 = T.copy();
  written = false;
  dT = (t2.minus(t1));
  dt = dT.getDouble();
  dX = (x2-x1)/dt;
  dY = (y2-y1)/dt;
  isComplete=true;
  return true;
}


/** writes the last measure **/
private boolean finish(){
   return write();
}



/** Writes this unit to the standard output **/
private boolean write(){
   if(!isComplete)
      return false;
    out.print("    ("); // open unit
    out.print("("); // open interval
    out.print("\""+t1+"\"  \""+t2+"\" ");
    out.print(" TRUE FALSE"); // closeness of interval
    out.print(")"); // close interval
    out.print("("); // open map
    out.print(x1+" "+y1+" "+x2+" "+y2); // the map
    out.print(")"); // close map
    out.println(")"); // close unit
    written = true;
    writtenUnits++;
    return true;
}


private void printStatistic(){
   System.err.println("written Units  : " + writtenUnits );
   System.err.println("skipped Points : " + skippedPoints);
}

// is true if the first point in time is stored
private boolean initialized = false;
private boolean isComplete = false;
private double lastTime=0.0;
private double x1 = 0.0;
private double y1 = 0.0;
private Time t1 = new Time(0,0);

private double x2 = 0.0;
private double y2 = 0.0;
private Time t2 = new Time(0,0);
private double dX = 0.0;
private double dY = 0.0;
private boolean written = false;

private  double epsilon=0.00000;

private  long writtenUnits = 0;
private  long skippedPoints = 0;

}



}


