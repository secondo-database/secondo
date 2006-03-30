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

import java.util.Vector;
import sj.lang.ListExpr;
import tools.Reporter;

/* to do :
   - accelerate the search by including a vector of
     summarized time intervals
*/

public class CompositeMove implements Move{

/** creates a undefined composite move */
public CompositeMove(){
   this(-1);
}

/** creates a undefined composite move with the given
  * initial capacity for sub moves
  */

public CompositeMove(int InitialCapacity){
   interval = null;
   defined = false;
   subMoves = InitialCapacity<=0? new Vector() : new Vector(InitialCapacity);
   summarizedIntervals = InitialCapacity<=0? new Vector() : new Vector(InitialCapacity);
}

/** returns true if this composite move is defined */
public boolean isDefined(){
   return defined;
}

/** returns the total relative interval of this move */
public RelInterval getInterval(){
   return interval;
}

public Object getObjectAt(Time T){
   if(!defined){
     Reporter.debug("CompositeMove.getObjectAt called on undefined instance");
     return null;
   }
   if(!this.interval.contains(T)){
      return null;
   }
   if(Time.ZeroTime.compareTo(T)>0){
      return ((Move) subMoves.get(0)).getObjectAt(T);
   }
   int minIndex = ((Move) subMoves.get(0)).getInterval().isLeftInfinite()?1:0;
   Move M = (Move) subMoves.get(minIndex);
   if(M.getInterval().contains(T))
      return M.getObjectAt(T);
   minIndex++;
   int maxIndex=subMoves.size();
   int midIndex = (maxIndex+minIndex)/2;
   RelInterval MidInterval = (RelInterval) summarizedIntervals.get(midIndex);
   RelInterval PreMidInterval = (RelInterval) summarizedIntervals.get(midIndex-1);
   boolean CMid = MidInterval.contains(T);
   boolean CPre = PreMidInterval.contains(T);
   while(CPre || !CMid){
      if(CPre){
        maxIndex = midIndex-1;
      }else { // !CMid
        minIndex = midIndex+1;
      }
      midIndex = (maxIndex+minIndex)/2;
      MidInterval = (RelInterval) summarizedIntervals.get(midIndex);
      PreMidInterval = (RelInterval) summarizedIntervals.get(midIndex-1);
      CMid = MidInterval.contains(T);
      CPre = PreMidInterval.contains(T);
   }
   M = (Move) subMoves.get(midIndex);
   return M.getObjectAt(T.minus(PreMidInterval.getLength()));
}


/** returns the number of cointained moves */
public int getNumberOfMoves(){
    return subMoves.size();
}

/** returns the Move at the specific index
  * returns null if this index is not contained
  */
public Move getMoveNo(int index){
   if(index<0 || index >=subMoves.size())
      return null;
   return (Move) subMoves.get(index);
}


private void setUndefined(){
    defined = false;
    interval=null;
    subMoves.clear();
}

public boolean readFrom(ListExpr LE, Class linearClass){
   if(LE.listLength()!=2){
     Reporter.debug("CompositeMove.readFrom : wrong ListLength \n"+
                    "exected 2, received : "+LE.listLength());
     setUndefined();
     return false;
   }
   if(LE.first().atomType()!=ListExpr.SYMBOL_ATOM ||
      LE.second().atomType() != ListExpr.NO_ATOM){
      Reporter.debug("CompositeMove.readFrom : wrong types of the sublists");
     setUndefined();
     return false;
   }
   if(!LE.first().symbolValue().equals("composite")){
     Reporter.debug("CompositeMove.readFrom : wrong type description");
     setUndefined();
   }

   int length = LE.second().listLength();
   if(length<1){
      Reporter.debug("CompositeMove.readFrom : wrong length of the value list");
      setUndefined();
      return false;
   }
   ListExpr SubMoves = LE.second();
   subMoves.ensureCapacity(SubMoves.listLength()+1);
   summarizedIntervals.ensureCapacity(SubMoves.listLength()+1);
   boolean ok = true;
   while(!SubMoves.isEmpty() && ok){
     ListExpr SML = SubMoves.first();
     SubMoves = SubMoves.rest();
     if(SML.listLength()<1 || SML.first().atomType()!=ListExpr.SYMBOL_ATOM){
       ok = false;
     }else{
       String type = SML.first().symbolValue();
       if(!type.equals("period") && !type.equals("linear")){
            Reporter.debug("CompositeMove.readFrom : found unknown type of sub move");
            ok = false;
       } else{
         try{
            Move SM;
	    if(type.equals("period"))
               SM = new PeriodMove();
   	    else
	       SM = (Move) linearClass.newInstance();
            ok = SM.readFrom(SML,linearClass);
            if(!ok){
                Reporter.debug("CompositeMove.readFrom : error in reading submove ");
            }
            if(ok)
	       ok = append(SM);
	    if(!ok){
	       Reporter.debug("CompositeMOve.readFRom: error in appending submove");
      }

	 }catch(Exception e){
	     Reporter.debug("CompositeMove.readFrom : can't create the Submove");
       Reporter.debug(e);
       ok = false;
	 }
       }
     }
    } // while
    if(!ok){
      Reporter.debug("CompositeMove.readFrom : error in reading submoves");
     setUndefined();
     return false;
   }
   return true;

}


/** append the specified Move
  * if the interval can't be appendend null is returned
  */
public boolean append(Move M){
  if(M==null){
    Reporter.debug("CompositeMove.append: try to append null");
     return false;
  }
  if(!defined){
     this.interval = M.getInterval().copy();
     boundingBox = M.getBoundingBox();
     subMoves.add(M);
     summarizedIntervals.add(interval.copy());
     defined=true;
     return true;
  }
  RelInterval Minterval = M.getInterval();
  if(!this.interval.canAppended(Minterval)){
     Reporter.debug("CompositeMove.append: submove interval collides with this interval");
     return false;
  }
  subMoves.add(M);
  this.interval.append(Minterval);
  summarizedIntervals.add(interval.copy());
  if(boundingBox!=null)
     boundingBox = boundingBox.union(M.getBoundingBox());
  return true;
}

public BBox getBoundingBox(){
   return boundingBox;
}

private boolean defined;
private RelInterval interval;
private Vector  subMoves;
private Vector  summarizedIntervals;
private BBox boundingBox;
}
