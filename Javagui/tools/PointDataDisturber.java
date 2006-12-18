
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
import java.util.Vector;


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

/* data section */

/** Stream for writing log messages **/
private PrintStream log = System.err;

/** Comtains the periodic moving point representing the
  * the clean source of the disturbed data.
  **/
private  Dsplpmpoint pmpoint=null; // contains the point data

/** The time difference between two measures. **/
private  Time defaultDiff = new Time(0,1000);   // 1 second

/** Stream in which the output is written. **/
private  PrintStream out=System.out;     // the output stream

/** The maximum error.
  * All produces errors will be smaller or equal to this 
  * value.
  **/
private double maxError;

/** Defines the possible change of the error between two measures.
  **/
private double maxErrorDiff; 


/** If this is set to true, the maximum error measure depends on the
  * length of the trajectory to the last measure point
  * The length is multiplied to maxErrorDiff.
  */
private boolean relativeError = true;

/** Restrictts the number od measures.
  * If this value is zero, the complete source is scanned.
  **/
private int maxMeasures = 0; 


/** Stores the last original point.
  * This value is reuired for computing a relative error between
  * two measures and also not to change the error if the point is
  * staying.
  **/
private java.awt.geom.Point2D.Double lastPoint; 

/** Stores the last applied error in x  and y dimension.
  **/
private java.awt.geom.Point2D.Double lastError; 


/** Number of data to delete.
  * This value holds the number of measure which is to remove.
  **/
private int removeSequence = 0;
/** If a complete sequence is removed, its maximum length is 
  * given by this value.
  **/
private int maxSequence = 50;


/** Probability to change the current error per measure */
private double changeErrorProb = 0.3;

/** Probability to delete measures.  **/
private double removeProb = 0.01;  
/** Probability to delete a complete Sequence of measures. **/
private double removeSeqProb = 0.0001; 


/** The unitwriter.
  * This objects is responsible for converting streams of measures 
  * into upoint lists.
  **/
private UnitWriter writer;

/** The delayManager.
  * This member controls the creation and realization of delay events.
  **/
private DelayManager delayManager= new DelayManager();


/* some statistical information */
/** Number of measures. **/
private int measures;
/** Number of skipped (removed) units */
private int skippedUnits;

/** zerotime for easy checks **/
private static final Time ZEROTIME = new Time(0,0);



/* public section */


/** Creates a new Instance of this class.*/
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


/** Sets the time difference between 
  * two measures 
  **/
public void setTimeDiff(Time t){
  if(t.compareTo(ZEROTIME)>0){ 
     defaultDiff.equalize(t);
  } else {
     log.println("PointDataDisturber time difference <= 0");     
  }
}

/** Sets the maximum error and the maximum error diff.
  *@param absolute: the maximum error which can be produced 
  *@param errorDiff: the maximum error drevation betweem two measures
  **/
public void setMaxErrors(double absolute,double errorDiff){
  if((absolute<0) || (errorDiff<0)){
      log.println("invalid value for maximum error or maximum error difference");
      return;
  }
  if(errorDiff>absolute){
     log.println("the absolute error has to be greater than the error difference");
     return;
  }
  maxError = absolute;
  maxErrorDiff = errorDiff;
}

/** Enables the computation of relative errors.
  * If on is set to true, the maximum error derivation is computed by
  * multiplying the distance to the last position whith the maximum error
  * difference. Otherwise the maximum error difference is an absolute value.
  **/
public void enableRelativeError(boolean on){
   relativeError=true; 
}

/** Reduces the number of measures.
  * Sets the maximum number of measures. If count is smaller or equal to
  * zero, the complete source point will be processed.
  **/
public void setMaximumMeasures(int count){
    maxMeasures = Math.max(0,count);
}

/** Sets the maximum length of a removes sequence.
  **/
public void setMaxSeqLength(int length){
   if(length<=0){
      log.println("maximum sequence length has to be greater than zero");
   } else {
      maxSequence = length;
   }
}

/** Sets the probablity to change the current error between two 
  * measures.
  * @param prob: the probability in [0,1]
  **/
public void setChangeErrorProb(double prob){
  if( (prob<=0) || (prob>1)){
      log.println("setChangeErrorProb called with invalid value ");
  } else {
      changeErrorProb = prob;
 }
}

/** Sets the probability to remove  measures.
  * This value determines the probability for both, removing single 
  * measures and to remove whole chains of measures.
  **/
public void setRemoveProb(double prob){
   if(prob>=1 ){
      log.println("the probability to remove a measure has to be smaller than 1");
   } else {
      removeProb = prob;
   }
}

/** Determines the relation between removing single measures or
  * to remove whole chains of measures.
  * A value of 0 means, only single measures will be removed,
  * in contrast a value of 1 forces the remove of complete chains.
  * @param prob: the probability in range [0,1]
  **/
public void setRemoveSeqProb(double prob){
   if(prob>=1 ){
      log.println("the probability to remove a measure has to be smaller than 1");
   } else {
      removeSeqProb = prob;
   }
}

/** Sets the probability of delay events.
  *@param prob: the probability in [0,1]
   **/
public void setEventProb(double prob){
   delayManager.setEventProb(prob);
}

/** Sets the relation between stop events and deceleration events.
  *@param prob: the probability in [0,1]
  **/
public void setStopProb(double prob){
   delayManager.setStopProb(prob);
}

/** Sets the maximum delay per event.
  **/
public void setMaxDelay(double delay){
   delayManager.setMaxDelay(delay);
}

/** Sets the deceleration factors.
  * The deceleration is between the minDec-multiple
  * and maxDec-multiple of the original speed.
  *@param minDec: minimum deceleration in range [0,1]
  *@param maxDec: maximum deceleration in range [0,1]
  **/
public void setDecs(double minDec, double maxDec){
   delayManager.setDecs(minDec, maxDec);
}

/** sets the factors for the acceleration.
  * The acceleration is between the (1+minAcc)-multiple 
  * and the (1+maxAcc)-multiple of the original speed.
  *@param minAcc: minimum acceleration (&gt;0)
  *@param maxAcc: maximum acceleration (&gt;minAcc)
  **/
public void setAccs(double minAcc, double maxAcc){
   delayManager.setAccs(minAcc,maxAcc);
}


/** Sets the stream where logging messages are printed. **/
public void setLog(PrintStream log){
  if(log!=null){
      this.log = log;
      writer.setLog(log);
      delayManager.setLog(log);
  }
}

/** Sets the stream for printing the generated data.
  **/
public void setOut(PrintStream out){
   this.out = out;
   writer.setOut(out);
}

/** Sets the tolerance which is used to connect the current
  * created unit with the new measure.
  **/
public void setWriterTolerance(double tolerance){
   this.writer.setTolerance(tolerance);
}

/** Prints out some statistical information the the log stream **/
public void writeStatistic(){
  log.println("measures : " + measures);
  log.println("skipped units : " + skippedUnits);
  writer.printStatistic();
}

/** Converts the data given by the list.
  **/
public boolean processList(ListExpr theList){
  log.println("start to process a new list ");
  int length = theList.listLength();
  ListExpr first = theList.first();
  // first case, a single (type value) list
  if(length==2){
    if(first.atomType()==ListExpr.SYMBOL_ATOM && first.symbolValue().equals("pmpoint")){
        log.println("pmpoint detected in list ");
        out.println("(mpoint ");
        boolean res = processValueList(theList.second());
        out.println(")");
        return res;
    } else if((first.atomType()==ListExpr.NO_ATOM) && 
               (first.listLength()==2) && 
               (first.first().atomType()==ListExpr.SYMBOL_ATOM) &&
               (first.first().symbolValue().equals("rel"))){
           log.println("relation detected in list");
           out.println("(");
           boolean res = processRelList(first,theList.second());
           out.println(")");
           return res;
   } else {
        theList.writeTo(out);
        log.println("List is not a peridic moving point");
        return true;
    }
  }
  // object list
  if(length==5 && first.atomType()==ListExpr.SYMBOL_ATOM && first.symbolValue().equals("OBJECT")){
      log.println("object styled list detected");
      return processObjectList(theList);
  }


  // Database
  if(length==4 && first.atomType()==ListExpr.SYMBOL_ATOM && first.symbolValue().equals("DATABASE")){
     boolean res = true;
     log.println("database list found");
     out.println("( DATABASE ");
     theList.second().writeTo(out); // DB-Name
     theList.third().writeTo(out);  // types
     ListExpr objects = theList.fourth(); // all objects
     if(objects.atomType()!=ListExpr.NO_ATOM || objects.isEmpty()){
         log.println("wrong list format for database objects found");
         objects.writeTo(out);
     } else if(objects.first().atomType()!=ListExpr.SYMBOL_ATOM ||
           !objects.first().symbolValue().equals("OBJECTS")){
              log.println("wrong list format for database objects found");
               objects.writeTo(out);
     } else {
       out.println("( OBJECTS");
         objects = objects.rest();
         while(!objects.isEmpty()){
              if (!processObjectList(objects.first())){
                  res = false;
              }
              objects = objects.rest();
         }
       out.println(")"); 
     }

     out.println(")"); // close database
     return res; 
  }
  log.println("unsupported list detected");
  theList.writeTo(out);
  return true;
}


/* private section */


/** Process a OBJECT styled list.  **/
private boolean processObjectList(ListExpr theList){
   int length = theList.listLength();
   if(length!=5){ // just copy the list
      log.println("invalid length for object representation ");
      theList.writeTo(out);
      return false;
   }

   // check for OBJECT
   ListExpr key = theList.first();
   if(key.atomType()!=ListExpr.SYMBOL_ATOM || !key.symbolValue().equals("OBJECT")){
      log.println("invalid object representation (wrong keyword)");
      theList.writeTo(out);
      return false;
   }

   ListExpr type = theList.fourth();
  
   // check for periodic moving point 
   if(type.atomType()==ListExpr.SYMBOL_ATOM && type.symbolValue().equals("pmpoint")){
       log.println("found a periodic point object");
       out.println("(OBJECT ");
       theList.second().writeTo(out);
       theList.third().writeTo(out);
       out.println(" mpoint ");  // change pmpoint to mpoint
       boolean res = processValueList(theList.fifth());
       out.println(")");
       return res;
   }

   // check for relation
   if((type.atomType()==ListExpr.NO_ATOM) && 
      (type.listLength()==2) &&
      (type.first().atomType()==ListExpr.SYMBOL_ATOM) &&
      (type.first().symbolValue().equals("rel"))){
      out.println("(OBJECT ");
      theList.second().writeTo(out);
      theList.third().writeTo(out);
      boolean res = processRelList(type,theList.fifth());
      out.println(")"); 
      return res;
   }

   log.println("unsupported object found");
   theList.writeTo(out);
   return true;
}


private boolean processRelList(ListExpr type, ListExpr value){
   log.println("process a relation");
   // analyse the type, also checks for correctness

  // check the type listlength
  if(type.listLength()!=2){
    log.println("invalid length for a relation type ");
    type.writeTo(out);
    value.writeTo(out);
    return false;
  }  
  // check the 'rel' keyword
  if(type.first().atomType()!=ListExpr.SYMBOL_ATOM ||
     !type.first().symbolValue().equals("rel")){
       log.println("invalid keyword for a relation type ");
       type.writeTo(out);
       value.writeTo(out);
       return false;
  }
  ListExpr tupleType = type.second();
  // check the correct structure for the tuple
  if(tupleType.listLength()!=2){
     log.println("invalid length for the tuple type detected");
     type.writeTo(out);
     value.writeTo(out);
     return false;
  }

  if((tupleType.first().atomType()!=ListExpr.SYMBOL_ATOM) ||
     (!tupleType.first().symbolValue().equals("tuple")) ||
     (tupleType.second().atomType()!=ListExpr.NO_ATOM)){
     log.println("invalid structure /keyword in tuple definition");
     type.writeTo(out);
     value.writeTo(out);
     return false;
  }

  // tuple seems to be correct, detect periodic moving point attributes
  // and store the appropriate indexes within an vector
  Vector pmpIndexes= new Vector();
  int index = 0;
  ListExpr attributes = tupleType.second();
  int no_attributes = attributes.listLength();
  while(!attributes.isEmpty()){
     ListExpr attr = attributes.first();
     attributes = attributes.rest();
     if( (attr.listLength()==2) &&
         (attr.second().atomType()==ListExpr.SYMBOL_ATOM) &&
         (attr.second().symbolValue().equals("pmpoint"))){
          pmpIndexes.add(new Integer(index));
     } 
     index++;
  }

  log.println(""+pmpIndexes.size()+" periodic moving point attributes detected");
  if(pmpIndexes.size()==0){ // keep lists unchanged
     type.writeTo(out);
     value.writeTo(out);
     return true;
  }

  // write the type to out (just opmpoint changed to Point)
  out.println("( rel (tuple ( ");
  index = 0;
  attributes = tupleType.second();
  while(!attributes.isEmpty()){
    ListExpr attr = attributes.first();
    attributes = attributes.rest();
    Integer ci = new Integer(index);
    ListExpr cl;
    if(pmpIndexes.contains(ci)){
         cl = ListExpr.twoElemList(attr.first(), ListExpr.symbolAtom("mpoint"));
    } else {
         cl = attr;
    }
    cl.writeTo(out);
    index++;
  }
  out.println(")))");  // close tuple, rel, and type


  // process the relation value

  if(value.atomType()!=ListExpr.NO_ATOM){
     log.println("invalid value list for relation ");
     value.writeTo(out);
     return false;
  }

  out.println("("); // open value list
  boolean res = true;
  while(!value.isEmpty()){
      ListExpr tuple = value.first();
      value = value.rest();
      if(tuple.listLength()!=no_attributes){
         log.println("error, length of tuple differs from it's type definition");
      } else { // process the tuple
        index = 0;
        out.println(" ( "); // open tuple
        while(!tuple.isEmpty()){
           ListExpr attr = tuple.first();
           tuple=tuple.rest();
           if(pmpIndexes.contains(new Integer(index))){
              if(! processValueList(attr)){
                res = false;
              }
           } else{
             attr.writeTo(out);
           }
           index++;
        }
        out.println(" )"); // close tuple
      }
  }
  out.println(")"); // close value list
  return res;
}


/** process the value list of a periodic moving point **/
private boolean processValueList( ListExpr value){
   log.println("\n\nstart conversion of a single periodic moving point \n\n");
   Dsplpmpoint pmp = new Dsplpmpoint();
   pmp.init(ListExpr.symbolAtom("pmpoint"),value,null);
   if(pmp.getError()){
      log.println("Invalid list representation of a periodic moving point");
      // print out an empty list
      out.println(" ( ) ");
      return false; 
   }
   setPMPoint(pmp);
   writeValue();
   return true;
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
      boolean changeError = Math.random()<changeErrorProb;
      if(!lastPoint.equals(p) && changeError){ // only change the error if the point is moving
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

 if(remove < removeProb){
    double removeSeq = Math.random();
    if(removeSeq < removeSeqProb){ // remove a whole sequence
         removeSequence = (int)(maxSequence*Math.random());
         skippedUnits++;
         return;
    } 
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
  // start a new conversion
 delayManager.reset();
 writer.reset();
 lastError=null;
 lastPoint= null;
 measures = 0;
 skippedUnits = 0;
 Time currentTime = new Time(pmpoint.getMinTime());
 Time max = pmpoint.getMaxTime();
 int writtenUnits=0;
   while((currentTime.compareTo(max)<=0)  && 
         ((writtenUnits<maxMeasures) || (maxMeasures<=0))){
       Time actualTime=delayManager.getTime(currentTime);
       addLocation(currentTime,pmpoint.getPosition(actualTime));
       currentTime.addInternal(defaultDiff);
       writtenUnits++;
   }
   writer.finish();
   writeStatistic();
}

/** Sets the data source for this object.
  **/
private void setPMPoint(Dsplpmpoint pmp){
   pmpoint = pmp;
}

/** print outs the generated data **/
private void writeValue(){
   out.println("(");
   createData();
   out.println(")");

}






private static class DelayManager{


private PrintStream log= System.err;
private Time scheduledDelay= new Time(0,0);
private Time lastReceivedTime;
private Time lastSendTime;
private double probability = 0.01; // create an event in 1% 
private double stopprob = 0.5;
private double minDec = 0.1;
private double maxDec = 0.9;
private double minAcc = 0.1;
private double maxAcc = 0.2;

/** maximum dela yin seconds **/
private double maxDelay = 200;

private int state = STATE_NORMAL;


// must be called before conversion starts
public void reset(){
    lastReceivedTime = null;
    lastSendTime = null;
    scheduledDelay = new Time(0,0);
    state = STATE_NORMAL;
}




/** Sets tbe probability for events. 
  * The value has to be in range [0.1].
  **/
public void setEventProb(double prob){
    if((prob<0) || (prob >1) ){
      return;
    }
    probability = prob;
}

/** sets the relation between stop and deceleration events **/
public void setStopProb(double prob){
   if((prob<0) || (prob>1)){
      return;
   }
   stopprob=prob;
}

/** Sets the values of minimum and maximum deceleration.
  * Both values has to be in range [0,1]. The value 0 means
  * thats the point stops. A value of 1 indicated a non deceleration
  * movement. 
  **/
public void setDecs(double minDec, double maxDec){
   if(minDec>maxDec){
      return;
   }
   if(minDec<0){
      return;
   }
   if(maxDec>1 || maxDec==0){
     return;
   }
   this.minDec=minDec;
   this.maxDec=maxDec;

}

/** Sets the values of minimum and maximum acceleration.
  * Both values has to be in range (0,..]. The value 0 means
  * thats the point goes in the same speed as without acceleration.
  * A value of 1 indicates double speed and so on. 
  **/
public void setAccs(double minAcc, double maxAcc){
   if(minAcc>maxAcc){
      return;
   }
   if(minAcc<0){
      return;
   }
   if(maxAcc==0){
     return;
   }
   this.minAcc=minAcc;
   this.maxAcc=maxAcc;
}



public void setMaxDelay(double seconds){
   if(maxDelay<1){
       return;
   }
   maxDelay = seconds;
}

public void setLog(PrintStream log){
   this.log = log;
}


/** returns the current state as String **/
private  String getState(int stateCode){
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
  log.println("state "+getState(state));
  log.println("lastReceivedTime " + lastReceivedTime);
  log.println("lastSendTime " + lastSendTime);
  log.println("scheduledDelay " + scheduledDelay.toDurationString()); 
}


/** checks whether origTime &lt;= sendTime **/
private void checkSendTime(Time origTime, Time sendTime){
   if(sendTime==null) return;
   int cmp = origTime.compareTo(sendTime);
   if(cmp<0){
        log.println("wrong computation:  sendTime > origTime");

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
   } else if(origTime.compareTo(lastReceivedTime)<=0){
     log.println("DelayManager: wrong order of instants !!");
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
        log.println("Change state to HURRY");
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
        log.println("Change state to HURRY");
     }      
     // compute the deceleration between 10 and 90 percent
     double dec = minDec+Math.random()*(maxDec-minDec);
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
     double dec = 1 + minAcc+Math.random()*(maxAcc-minAcc);
     Time origDelta = origTime.minus(lastReceivedTime);
     Time delta = origDelta.mul(dec);
     lastSendTime.addInternal(delta);
     // acceleration finished
     if(lastSendTime.compareTo(origTime)>=0){
         lastSendTime.equalize(origTime);
         state = STATE_NORMAL;
         log.println("Change state to NORMAL");
     }
     lastReceivedTime = origTime.copy();
     checkSendTime(origTime,lastSendTime);
     return lastSendTime.copy(); 
   }
   lastReceivedTime = origTime.copy();
   log.println("Unknown state " + state);
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
   if(Math.random()<stopprob){
       log.println("create a stop of " + (delay/1000) +" seconds");
       state = STATE_STOP;
   } else{
       log.println("create a deceleration of " + (delay/1000) +" seconds");
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
private PrintStream log = System.err;

public UnitWriter(PrintStream out, double epsilon){
   this.out=out;
   this.epsilon = epsilon;
}

/** sets the value of the error tolerance to conect units **/
public void setTolerance(double e){
     epsilon = Math.max(0,e);
}

/** sets the PrintStream for logging purposes **/
public void setLog(PrintStream log){
   this.log = log;
}

public void setOut(PrintStream out){
   this.out = out;
}


/** Appends a new point in time
  **/
boolean addMeasure(Time T,double x, double y){

  if(T.compareTo(t1)<=0){ // time must be more than before
     log.println("UnitWriter: wrong order of measures detected");
     log.println("last time :"+t1+" ne time "+T);
     return false;
  }

  double t = T.getDouble();
  if(!initialized){ // the first point
    x1 = x;
    y1 = y;
    t1 = T.copy();
    initialized = true;
    return true; 
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
   log.println("written Units  : " + writtenUnits );
   log.println("skipped Points : " + skippedPoints);

}

public void reset(){
  initialized = false;
  isComplete = false;
  lastTime = 0.0;
  x1 = 0.0;
  y1 = 0.0;
  t1 = new Time(0,0);
  x2 = 0.0;
  y2 = 0.0;
  t2 = new Time(0,0);
  dX = 0.0;
  dY = 0.0;
  written = false;
  writtenUnits = 0;
  skippedPoints = 0;
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


