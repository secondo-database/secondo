package viewer.hoese.algebras.periodic;

import sj.lang.ListExpr;

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
   leftInfinite = false;
   rightInfinite = false;
   return true;
}

/** returns the length of this interval,
  * if this Duration is infinite, null is returned
  */
public Time getLength(){
   if(leftInfinite || rightInfinite)
      return null;
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
   rightInfinite=false;
   leftInfinite=false;
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

/** returns the if this interval is left infinite */
public boolean isLeftInfinite(){
   return leftInfinite;
}

/** returns the if this interval is right infinite */
public boolean isRightInfinite(){
   return rightInfinite;
}

/** sets this interval to be left infinite with the given value
  * for the right closure
  */
public boolean setLeftInfinite(boolean rightClosed){
    leftInfinite=true;
    this.rightClosed=rightClosed;
    return true;
}

/** sets this interval to be right infinite with the given value
  * for the left closure
  */
public boolean setRightInfinite(boolean leftClosed){
    length=null;
    rightInfinite=true;
    this.leftClosed=leftClosed;
    return true;
}

/** creates a interval which is infinite in any direction
  * @return allways true
  */
public boolean setLeftRightInfinite(){
   length = null;
   leftClosed = false;
   rightClosed = false;
   leftInfinite = true;
   rightInfinite = true;
   return true;
}


/** appends interval to this */
public boolean append(RelInterval interval){
  if(!canAppended(interval)){
     if(Environment.DEBUG_MODE)
        System.err.println("try to append "+interval+" at "+this);
     return false;
  }
  if(this.length==null){
     this.length = interval.length.copy();
  }else{
    this.length = this.length.add(interval.length);
  }
  rightClosed = interval.rightClosed;
  rightInfinite = interval.rightInfinite;
  return true;
}

/** returns true if interval can appended to this */
public boolean canAppended(RelInterval interval){
   return rightClosed^interval.leftClosed &&
          !rightInfinite & !interval.leftInfinite;
}

/** returns true if T is contained in this interval */
public boolean contains(Time T){
   int comp = T.compareTo(Time.ZeroTime);
   if(comp < 0){
     return leftInfinite;
   }
   if(comp == 0){
     return leftClosed;
   }
   if(rightInfinite){
      return true;
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
   if(leftInfinite | rightInfinite)
      return 0.0;
   return T.getDouble() / length.getDouble();
}

/** returns a depth clone of this */
public RelInterval copy(){
   RelInterval res = new RelInterval();
   res.length = length;
   res.leftClosed = leftClosed;
   res.rightClosed = rightClosed;
   res.leftInfinite = leftInfinite;
   res.rightInfinite = rightInfinite;
   return res;
}

/** reads this interval from a LE.
  * if LE don't represent a valid interval, the value of this
  * will be unchanged and null is returned.
  */
public boolean readFrom(ListExpr LE){
  if(LE.listLength()!=2){
     if(Environment.DEBUG_MODE)
       System.err.println("RelInterval.readFrom :: wrong list length ");
     return false;
  }
  // check the type
  if(LE.first().atomType()!=LE.SYMBOL_ATOM || !LE.first().symbolValue().equals("rinterval")){
     if(Environment.DEBUG_MODE){
        System.err.println("RelInterval.readFrom :: Typidentifier 'rinterval' not found");
     }
     return false;
  }
  ListExpr Value = LE.second();
  if(Value.listLength()!=5){
     if(Environment.DEBUG_MODE)
        System.err.println("RelInterval.readFrom :: wrong list length of the value ");
     return false;
  }
  // check the List
  if(Value.first().atomType()!=LE.BOOL_ATOM ||
     Value.second().atomType()!=LE.BOOL_ATOM ||
     Value.third().atomType()!=LE.BOOL_ATOM ||
     Value.fourth().atomType()!=LE.BOOL_ATOM ||
     Value.fifth().atomType()!=LE.NO_ATOM){
       if(Environment.DEBUG_MODE){
         System.err.println("RelInterval.readFrom ::the Value List contains invalid types ");
       }
       return false;
   }
   boolean LC = Value.first().boolValue();
   boolean RC = Value.second().boolValue();
   boolean LI = Value.third().boolValue();
   boolean RI = Value.fourth().boolValue();
   boolean ok = true;
   Time L = null;
   if(!RI & !LI){
      L = new Time();
      ok = L.readFrom(Value.fifth());
   }
   if(!ok){
      if(Environment.DEBUG_MODE)
         System.err.println("RelInterval.readFrom :: Error in reading time value "+Value.fifth().writeListExprToString());
      return false;
   }
   leftClosed=LC;
   rightClosed=RC;
   leftInfinite = LI;
   rightInfinite = RI;
   length = L;
   return true;
}


/** Extends this interval to the factor-multiple of the old length.
  * If this interval is infinite the call of this function will have
  * not an effect. factor has to be greater than zero.
  */
public void mul(int factor){
  if(factor <=0){
    return;
  }
  if(leftInfinite || rightInfinite){
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
     resvalue = leftInfinite?1:-1;
   } else if(T.compareTo(Time.ZeroTime)==0){
     resvalue =  leftClosed?1:-1;
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
  res = res + (leftInfinite? "$" : "0") + " , ";
  res = res + (rightInfinite? "$" : (""+length.toDurationString()));
  res = res + (rightClosed? "]" : "[");
  return res;
}

public String getListExprString(){
   String ts =" TRUE ";
   String fs =" FALSE ";
   String res ="(rinterval (";
   res += leftClosed? ts : fs;
   res += rightClosed? ts : fs;
   res += leftInfinite? ts : fs;
   res += rightInfinite? ts : fs;
   res+= length==null? " () " : length.getListExprString(false);
   res += "))";
   return res;
}

private Time length;
private boolean leftClosed;
private boolean rightClosed;
private boolean leftInfinite;
private boolean rightInfinite;

}
