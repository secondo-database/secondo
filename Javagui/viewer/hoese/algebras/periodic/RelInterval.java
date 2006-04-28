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
import tools.Reporter;

/** This class is the implementation of a relative interval.
  */
public class RelInterval{

/* creates a interval with length 0 */
public RelInterval(){
   length = new Time();;
   leftClosed=true;
   rightClosed=true;
}

/** sets this interval to the given values.
  * @return true if successful
  */
public boolean set(Time length, boolean leftClosed, boolean rightClosed){
   if(length.compareTo(Time.ZeroTime)<0)  // wrong input
      return false;
   // if the length is zero the interval must be closed on both sides
   if(length.compareTo(Time.ZeroTime)==0 & !(leftClosed && rightClosed))
      return false;
   this.length = length;
   this.leftClosed = leftClosed;
   this.rightClosed = rightClosed;
   return true;
}

/** returns the length of this interval,
  */
public Time getLength(){
   return length;
}

/** sets the length of this interval.
  * returns false if  T is smaller than zero or
  * the T is equal to zero and the interval is not closed
  * in any direction.
  */
public boolean setLength(Time T){
   if(T.compareTo(Time.ZeroTime)<0){
      return false;
   }
   if(T.compareTo(Time.ZeroTime)==0 && ! (leftClosed && rightClosed))
      return false;
   length = T;
   return true;
}

/** returns the if this interval is left closed */
public boolean isLeftClosed(){
   return leftClosed;
}

/** returns the if this interval is right closed */
public boolean isRightClosed(){
   return rightClosed;
}


/** appends interval to this */
public boolean append(RelInterval interval){
  if(!canAppended(interval)){
     Reporter.debug("try to append "+interval+" at "+this);
     return false;
  }
  if(this.length==null){
     this.length = interval.length.copy();
  }else{
    this.length = this.length.add(interval.length);
  }
  rightClosed = interval.rightClosed;
  return true;
}

/** returns true if interval can appended to this */
public boolean canAppended(RelInterval interval){
   return rightClosed^interval.leftClosed;
}

/** returns true if T is contained in this interval */
public boolean contains(Time T){
   int comp = T.compareTo(Time.ZeroTime);
   if(comp < 0){
     return false;
   }
   if(comp == 0){
     return leftClosed;
   }
   if(length.compareTo(T) > 0){
     return true;
   }
   if(length.compareTo(T)==0){
      return rightClosed;
   }
   return false;
}


/** returns the procentual part of T's position within this interval*/
public double where(Time T){
   if(!contains(T))
      return -1.0;
   return T.getDouble() / length.getDouble();
}

/** returns a depth clone of this */
public RelInterval copy(){
   RelInterval res = new RelInterval();
   res.length = length;
   res.leftClosed = leftClosed;
   res.rightClosed = rightClosed;
   return res;
}

/** reads this interval from a LE.
  * if LE don't represent a valid interval, the value of this
  * will be unchanged and null is returned.
  */
public boolean readFrom(ListExpr LE){
  if(LE.listLength()!=2){
     Reporter.debug("RelInterval.readFrom :: wrong list length ");
     return false;
  }
  // check the type
  if(LE.first().atomType()!=ListExpr.SYMBOL_ATOM || !LE.first().symbolValue().equals("rinterval")){
     Reporter.debug("RelInterval.readFrom :: Typidentifier 'rinterval' not found");
     return false;
  }
  ListExpr Value = LE.second();
  if(Value.listLength()!=3){
     Reporter.debug("RelInterval.readFrom :: wrong list length of the value ");
     return false;
  }
  // check the List
  if(Value.first().atomType()!=ListExpr.BOOL_ATOM ||
     Value.second().atomType()!=ListExpr.BOOL_ATOM ||
     Value.third().atomType()!=ListExpr.NO_ATOM){
       Reporter.debug("RelInterval.readFrom ::the Value List contains invalid types ");
       return false;
   }
   boolean LC = Value.first().boolValue();
   boolean RC = Value.second().boolValue();
   boolean ok = true;
   Time L = null;
   L = new Time();
   ok = L.readFrom(Value.third());
   if(!ok){
      Reporter.debug("RelInterval.readFrom :: Error in reading time value "+Value.third().writeListExprToString());
      return false;
   }
   leftClosed=LC;
   rightClosed=RC;
   length = L;
   return true;
}


/** Extends this interval to the factor-multiple of the old length.
  */
public void mul(int factor){
  if(factor <=0){
    return;
  }
  length = length.mul(factor);
}


/** returns the minimal value which is needed to
  * extend this interval so that this interval contains
  * the given Time Value. <br>
  * If no such an factor exists (e.g. T==0 && !leftClosed) -1 is returned
  */
public long getExtensionFactor(Time T){
   if(contains(T))
     return 1;
   long resvalue =1;
   if(T.compareTo(Time.ZeroTime)<=0){ // we can't find such a factor
     resvalue = -1;
   } else if(T.compareTo(Time.ZeroTime)==0){
     resvalue =  -1;
   } else {
        long Dms = length.getMilliseconds();
        long Tms = T.getMilliseconds();
        long frac = Tms/Dms;
        if(Tms%Dms==0 && rightClosed) frac--;
        resvalue = frac+1;
   }
   return resvalue;

}

public String toString(){
  String res = leftClosed? "[":"]";
  res = res + "0" + " , ";
  res = res + (""+length.toDurationString());
  res = res + (rightClosed? "]" : "[");
  return res;
}

public String getListExprString(){
   String ts =" TRUE ";
   String fs =" FALSE ";
   String res ="(rinterval (";
   res += leftClosed? ts : fs;
   res += rightClosed? ts : fs;
   res+= length==null? " () " : length.getListExprString(false);
   res += "))";
   return res;
}

private Time length;
private boolean leftClosed;
private boolean rightClosed;

}
