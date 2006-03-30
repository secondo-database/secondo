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

public class PeriodMove implements Move{

public RelInterval getInterval(){ return interval;}

/** creates an undefined PeriodMove */
public PeriodMove(){
   defined = false;
}

/** sets this PeriodMove to an undefined state */
private void setUndefined(){
   defined=false;
   this.subMove = null;
   this.interval = null;
   this.repeatations=0;
}

/** sets this PeriodMove to the given values.
  * If the arguments don't decribe a valid PeriodMove,
  * the resulting state will be undefined and the returnvalue
  * will be false
  */
public boolean set(int repeatations, Move subMove){
   RelInterval smInterval = subMove.getInterval();
   if(smInterval.isLeftInfinite() || smInterval.isRightInfinite()){
      Reporter.debug("PeriodMove.set: try to repeat a infinite move ");
      setUndefined();
      return false;
   }
   if(repeatations==LEFTINFINITE){
      (interval = new RelInterval()).setLeftInfinite(smInterval.isRightClosed());
      this.repeatations = repeatations;
      this.subMove = subMove;
      defined = true;
      return true;
   }
   if(repeatations==RIGHTINFINITE){
      (interval =  new RelInterval()).setRightInfinite(smInterval.isLeftClosed());
      this.repeatations = repeatations;
      this.subMove = subMove;
      defined = true;
      return true;
   }
   if(repeatations<=0){
      Reporter.debug("PeriodMove.set: invlaid number of repeatation :"+repeatations);
      setUndefined();
      return false;
   }
   if(!(smInterval.isLeftClosed() ^ smInterval.isRightClosed())){
      Reporter.debug("PeriodMove.set: try to repeat an move with an completely open (closed) interval");
      setUndefined();
      return false;
   }
   this.repeatations = repeatations;
   this.interval = smInterval.copy();
   this.interval.setLength(this.interval.getLength().mul(repeatations));
   this.subMove = subMove;
   defined=true;
   return true;
}



/** returns the object at the given time.
  * If the object is not defined at this time null is returned.
  */
public Object getObjectAt(Time T){

   if(!defined){
     Reporter.debug("PeriodMove.getObjectAt on an undefined move called");
     return null;
   }

   if(!interval.contains(T)){
      return null;
   }
   RelInterval SMInterval = subMove.getInterval();
   Time SMLength = SMInterval.getLength().copy();
   if(T.compareTo(Time.ZeroTime)<0){ // must be left-infinite
      while(!SMInterval.contains(T)){ // has to be improve by direct computation
          T = T.add(SMLength);
      }
      return subMove.getObjectAt(T);
   }

   long Fact = SMInterval.getExtensionFactor(T);
   if(Fact<0){
      Reporter.debug("PeriodMove.getObjectAt ExtensionFactor <0 ");
      return null;
   }

   T = T.minus(SMLength.mul(Fact-1));

   return subMove.getObjectAt(T);
}

/** reads this PeriodMove from its nested list representation.
  * If the list don't represent a valid period move this will
  * be undefined and false is returned.
  */
public boolean readFrom(ListExpr LE,Class linearClass){
   if(LE.listLength()!=2){
      Reporter.debug("PeriodMove.readFrom :: wrong ListLength()");
      setUndefined();
      return false;
   }
   if(LE.first().atomType()!=ListExpr.SYMBOL_ATOM){
      Reporter.debug("PeriodMove.readFrom :: wrong type of typedescriptor");
      setUndefined();
      return false;
   }
   if(!LE.first().symbolValue().equals("period")){
      Reporter.debug("PeriodMove.readFrom :: wrong Value for typedescriptor");
      setUndefined();
      return false;
   }
   if(LE.second().atomType()!=ListExpr.NO_ATOM){
      Reporter.debug("PeriodMove.readFrom :: value is not a list");
      setUndefined();
      return false;
   }
   ListExpr Value = LE.second();
   if(Value.listLength()!=2){
      Reporter.debug("PeriodMove.readFrom :: wrong ListLength() for value list");
      setUndefined();
      return false;
   }
   if(Value.first().atomType()!=ListExpr.INT_ATOM){
      Reporter.debug("PeriodMove.readFrom :: wrong type for repeatations");
      setUndefined();
      return false;
   }
   int rep = Value.first().intValue();
   if(rep<=1 && rep!=LEFTINFINITE && rep!=RIGHTINFINITE){
      Reporter.debug("PeriodMove.readFrom :: not a valid number of repeatations");
      setUndefined();
      return false;
   }
   if(Value.second().atomType()!=ListExpr.NO_ATOM || Value.second().listLength()<1){
      Reporter.debug("PeriodMove.readFrom :: wrong list type for submove");
      setUndefined();
      return false;
   }
   if(Value.second().first().atomType()!=ListExpr.SYMBOL_ATOM){
      Reporter.debug("PeriodMove.readFrom :: wrong listtype for type descriptor of the submove");
      setUndefined();
      return false;
   }
   String s = Value.second().first().symbolValue();
   if(!s.equals("linear") && !s.equals("composite")){
      Reporter.debug("PeriodMove.readFrom :: wrong type descriptor for subtype");
      setUndefined();
      return false;
   }
   Move sm;
   try{
      if(s.equals("linear"))
         sm = (Move) linearClass.newInstance();
      else
         sm = new CompositeMove();
   }catch(Exception e){
      Reporter.debug("PeriodMove.readFrom :: error in creating submove\n"+e);
	    Reporter.debug(e);
      setUndefined();
      return false;
   }

   if(!sm.readFrom(Value.second(),linearClass)){
      Reporter.debug("PeriodMove.readFrom :: error in reading submove ");
      setUndefined();
      return false;
   }
   return set(rep,sm);
}

public BBox getBoundingBox(){
   if(!defined){
      Reporter.debug("PeriodMove.getBoundingBox with an undefined instance");
      return null;
   }
   if(subMove==null){
     Reporter.debug("PeriodMove.getBoundingBox without a submove (null)");
       return null;
   }
   return subMove.getBoundingBox();
}

public String toString(){
  return "PeriodMove  [defined ="+defined+"   repeats ="+repeatations+
         "   interval ="+interval + "\nSubmove ="+subMove;
}

private boolean defined;
private int repeatations;
private RelInterval interval;
private Move subMove;
private Class linearClass;
public static final int LEFTINFINITE=-1;
public static final int RIGHTINFINITE=-2;


}


