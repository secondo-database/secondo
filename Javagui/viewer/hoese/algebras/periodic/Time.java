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

package viewer.hoese.algebras.periodic;

import sj.lang.ListExpr;
import viewer.hoese.DateTime;
import gui.Environment;
import tools.Reporter;

/** This class provides an abstraction of time
  */

public class Time{

/** creates the instant 0 */
public Time(){
   day = 0;
   milliseconds = 0;
}

/** creates a Time value from the arguments **/
public Time(long day, long milliseconds){
   set(day,milliseconds);
}

/** copy constructor **/
public Time(Time source){
   set(source.day,source.milliseconds);
}


/** sets the time */
public boolean set(long day, long milliseconds){
   this.day=day;
   this.milliseconds = milliseconds;
   correct();
   return true;
}


/** return the double value for this Time */
public double getDouble(){
   return   (double) day + (double)milliseconds/MILLISECONDS;
}

/** returns this time as milliseconds */
public long getMilliseconds(){
    return day*MILLISECONDS + milliseconds;
}

/** the familiar compareTo method */
public int compareTo(Time T){
  if(day<T.day) return -1;
  if(day>T.day) return 1;
  if(milliseconds<T.milliseconds) return -1;
  if(milliseconds>T.milliseconds) return 1;
  return 0;
}

/** reads this Time from a List.
  */
public boolean readFrom(ListExpr E){
  if(E.listLength()!=2){
    Reporter.debug("Time.readFrom: wrong ListLength");
    return false;
  }
  if(E.first().atomType()!=ListExpr.SYMBOL_ATOM){
     Reporter.debug("Time.readFrom: wrong list type of typedescriptor");
     return false;
  }
  if(!E.first().symbolValue().equals("instant") &&
     !E.first().symbolValue().equals("datetime") &&
     !E.first().symbolValue().equals("duration")){
    Reporter.debug("Time.readFrom wrong type value (expected" +
                   " 'datetime | instant | duration', received" +
                   (E.first().symbolValue())+")");
    return false;
  }
  ListExpr V = E.second();

  if(V.atomType()==ListExpr.STRING_ATOM){
    long[] DM = DateTime.getDayMillis(V.stringValue());
    if(DM==null){
        Reporter.debug("Time.readFrom: error in converting String to Time ");
        return false;
     }
     day = DM[0];
     milliseconds = DM[1];
     correct();
     return true;
  }
  if(V.listLength()==2){
     if(V.first().atomType()!=ListExpr.INT_ATOM || V.second().atomType()!=ListExpr.INT_ATOM){
         Reporter.debug("Error in Reading Time in Format Days Milliseconds");
	       return false;
      }
	day = V.first().intValue();
	milliseconds = V.second().intValue();
	correct();
	return true;
  }
  return false;

}

public void readFrom(double d){
    day =  (long)d;
    milliseconds = (long)((d-(double)day)*MILLISECONDS);
    correct();
}

public boolean readFrom(String value){
    long[] DM = DateTime.getDayMillis(value);
    if(DM==null){
       Reporter.debug("invalid Time format in Time.readFrom(String) :"+value);
       return false;
     }
    day = DM[0];
    milliseconds = DM[1];
    correct();
    return true;
}


/** computes the sum of this and T */
public Time add(Time T){
   Time res = new Time();
   res.day = day+T.day;
   res.milliseconds = milliseconds + T.milliseconds;
   res.correct();
   return res;
}

/** add T to this */
public void addInternal(Time T){
   day = day+T.day;
   milliseconds = milliseconds + T.milliseconds;
   correct();
}

/** computes the between this and T */
public Time minus(Time T){
   Time res = new Time();
   res.day = day-T.day;
   res.milliseconds = milliseconds-T.milliseconds;
   if(res.milliseconds<0){
     res.day--;
     res.milliseconds += 86400000;
   }
   return res;
}

/** computes the multiple of this Time instance */
public Time mul(int factor){
   Time res = new Time();
   res.day = day*factor;
   res.milliseconds = milliseconds*factor;
   res.correct();
   return res;
}

/** computes the multiple of this Time instance */
public Time mul(double factor){
   Time res = new Time();
   res.readFrom( getDouble()*factor);
   res.correct();
   return res;
}



public String getListExprString(boolean absolute){
   if(!absolute)
      return "( duration ("+day +" "+milliseconds+"))";
   else
      return "(instant \""+this+"\")";
}


/** This method ensures that the numerator is smaller than the
  * denominator by moving the integer-part of the fraction in the
  * field day.
  */
private Time correct(){
  // in a first step the milliseconds are moved to
  // be positive
  long oldms = milliseconds;
  long oldday = day;
  long dif1=0;
  long dif2=0;
  while(milliseconds < 0){
    dif1 = milliseconds/MILLISECONDS +1;
    day = (day - dif1);
    milliseconds = (milliseconds + dif1*MILLISECONDS);
  }
  // now we ensure that milliseconds are note greater than one day
  if(milliseconds>=MILLISECONDS){
     dif2 = milliseconds/MILLISECONDS;
     day = (day + dif2);
     milliseconds = (milliseconds - dif2*MILLISECONDS);
  }

  if(Environment.DEBUG_MODE){
     if(milliseconds<0 || milliseconds>=MILLISECONDS){
        Reporter.writeError("Time::Correct() has computes a wrong value \n"+
                            "Oldms :"+oldms+"\n"+
                            "Oldday :"+oldday+"\n"+
                            "new values : "+oldday+" : "+oldms);
     }
  }

  return this;
}

/** returns a proper clone of this instance */
public Time copy(){
   return new Time(day,milliseconds);
}


public String toString(){
   return DateTime.getString(day,milliseconds);
}

public String toDurationString(){
   long value = milliseconds;
   long millies = value % 1000;
   value = value/1000;
   long seconds = value % 60;
   value = value / 60;
   long minutes = value % 60;
   value = value / 60;
   long hours = value % 24;
   value = value / 24;
   return "" + day +"-"+hours+":"+minutes+":"+seconds+"."+millies;
}


public static final Time ZeroTime = new Time(0,0);

private int sign(int L){
   if(L<0) return -1;
   if(L==0) return 0;
   return 1;
}

public void equalize(Time T2){
   this.milliseconds=T2.milliseconds;
   this.day = T2.day;
}


private long milliseconds;
private long day;
public final static long MILLISECONDS=86400000;
}
