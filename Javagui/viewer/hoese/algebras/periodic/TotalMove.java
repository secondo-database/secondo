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

public class TotalMove implements Move{

/** created a undefined Move */
public TotalMove(){
   move = null;
   defined = false;
}

/** returns the objects at T.
  * if the object is not defined at this time null is returned
  */
public Object getObjectAt(Time T){
   if(!defined){
      if(Environment.DEBUG_MODE)
         System.err.println("TotalMove: getObjectAt called when undefined");
      return null;
   }
   Time T2 = T.minus(start);
   if(!move.getInterval().contains(T2)){
      return null;
   }
   return move.getObjectAt(T2);
}

/** returns the interval of this Move
  * returns null if this is not defined
  */
public RelInterval getInterval(){
   if(!defined)
      return  null;
   return move.getInterval();
}

/** reads this Move from the LE.
  * if LE not represents a valid move this will be undefined
  */
public boolean readFrom(ListExpr LE, Class linearClass){
   if(LE.listLength()!=2){
      if(Environment.DEBUG_MODE)
         System.err.println("TotalMove.readFrom :: wrong length of the list ");
      setUndefined();
      return false;
   }
   Object O;
   try{
     O = linearClass.newInstance();
     if(!(O instanceof LinearMove)){
        if(Environment.DEBUG_MODE)
            System.err.println("TotalMove.readFrom :: LinearClass creates not an instance of LinearMove");
        setUndefined();
        return false;
     }
   }catch(Exception e){
     if(Environment.DEBUG_MODE){
        System.err.println(e);
        System.err.println("TotalMove.readFrom :: error in creating linear move of Class "+linearClass);
	e.printStackTrace();
     }
     setUndefined();
     return false;
   }
   LinearMove LM = (LinearMove) O;

   String Type = LM.getName();
   Time T = new Time();
   if(LE.listLength()!=2){
      if(Environment.DEBUG_MODE)
         System.err.println("TotalMove.readFrom :: Value list has a wrong listLength()");
      setUndefined();
      return false;
   }
   if(!T.readFrom(LE.first())){
      if(Environment.DEBUG_MODE){
         System.err.println("TotalMove.readFrom :: The start time can't be readed");
	 System.err.println("List is :" + LE.first().writeListExprToString());
      }
      setUndefined();
      return false;
   }
   Move move;
   if(LE.second().listLength()<1){
      if(Environment.DEBUG_MODE)
         System.err.println("TotalMove.readFrom :: wrong list length for submove");
      setUndefined();
      return false;
   }
   if(LE.second().first().atomType()!=LE.SYMBOL_ATOM){
      if(Environment.DEBUG_MODE)
         System.err.println("TotalMove.readFrom :: Wrong list type for subtype type descriptor");
      setUndefined();
      return false;
   }
   String typedes = LE.second().first().symbolValue();
   if(!typedes.equals("period") && !typedes.equals("linear") && !typedes.equals("composite")){
      if(Environment.DEBUG_MODE)
         System.err.println("TotalMove.readFrom :: unknown value for type descriptor ("+typedes+")");
      setUndefined();
      return false;
   }
   if(typedes.equals("linear"))
      move = LM;
   else if(typedes.equals("period"))
      move = new PeriodMove();
   else
      move = new CompositeMove();
   if(!move.readFrom(LE.second(),linearClass)){
      if(Environment.DEBUG_MODE)
         System.err.println("TotalMove.readFrom :: Error in Reading the Move");
      setUndefined();
      return false;
   }
   this.defined=true;
   this.start = T;
   this.move = move;
   return true;
}

public BBox getBoundingBox(){
   if(!defined){
      if(Environment.DEBUG_MODE)
         System.err.println("getBounding Box called with undefined TotalMove");
      return null;
   }
   return move.getBoundingBox();

}

public Time getStartTime(){
   return start;
}

/** sets this to be undefined */
private void setUndefined(){
   defined = false;
   start = null;
   move = null;
}

private boolean defined;
private Time start;
private Move move;

}
