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

package fuzzyobjects.composite;


/**
 * this class represents a fuzzy topological
 * Relationship
 * they are 6 entries
 * position 0 : Z1 == Z2 == 0
 * position 1 : Z1 >  Z2 == 0
 * position 2 : Z2 >  Z1 == 0
 * position 3 : Z1 == Z1 > 0
 * position 4 : Z1 > Z2 > 0
 * position 5 : Z2 > Z1 > 0
 */
public class FuzzyTopRel{

/**
 * creates a new FuzzyTopRel with
 * 6 0-entries
 */
public FuzzyTopRel(){
    Values = new boolean[6];
    for(int i=0;i<6;i++)
      Values[i] = false;
}

/**
 * get then entry on number;
 * number must be in {0,...,5}
 */
public boolean getValue(int number){
   if(number<0 | number>5)
      return false;
   else
      return Values[number];
}

/**
  * set the entry on position number on value
  */
public void setValue(int number, boolean value){
 if(number>=0 & number<6)
    Values[number] = value;
}


/** returns a readable representation of this */
public String toString(){
 if(isDisjoint())    return "disjoint";
 if(isEmpty())       return "empty";
 if(isEqual())       return "equal";
 if(isOver())        return "over";
 if(isUnder())       return "under";
 if(isOverEqual())   return "overequal";
 if(isUnderEqual())  return "underequal";
 if(isOverlap())     return "overlap";
 return "";
}

/** make the symmetry relation */
public void makeSym(){
 boolean[] V = new boolean[6];
  V[0] = Values[0];
  V[1] = Values[2];
  V[2] = Values[1];
  V[3] = Values[3];
  V[4] = Values[5];
  V[5] = Values[4];
  Values = V;
}


/** returns the cluster of this */
public int getCluster(){
 if(isDisjoint())    return DISJOINT;
 if(isEmpty())       return EMPTY;
 if(isEqual())       return EQUAL;
 if(isOver())        return OVER;
 if(isUnder())       return UNDER;
 if(isOverEqual())   return OVEREQUAL;
 if(isUnderEqual())  return UNDEREQUAL;
 if(isOverlap())     return OVERLAP;
 return -1;
}


/** is this in the empty-cluster ? */
public boolean isEmpty(){
 boolean result = true;
 for(int i=0;i<6;i++)
   if(Values[i])
      result = false;
 return result;
}

/** is this in the disjoint-cluster ? */
public boolean isDisjoint(){
 boolean result = true;
 for(int i=3;i<6;i++)
   if(Values[i])
      result = false;
 result = result & (Values[0] | Values[1] | Values[2]);
 return result;
}

/** is this in the equal-cluster ? */
public boolean isEqual(){
 boolean result = true;
 for(int i=1;i<6;i++)   // Values[0] can be 0 or 1
   if(i!=3){
      if(Values[i])
         result = false;
   }
   else {
      if(!Values[i])
         result = false;
   }
 return result;
}

/** is this in the over-cluster ? */
public boolean isOver(){
 return  ! Values[0] & !Values[2] & !Values[3]
           & Values[4] & !Values[5];
}

/** is this in the under-cluster ? */
public boolean isUnder(){
 return !Values[0] & !Values[1] & !Values[3] &
        !Values[4] & Values[5];
}

/** is this in the overequal-cluster ? */
public boolean isOverEqual(){
 return !Values[2] & !Values[5] &
        (   (Values[3] & Values[4] ) |
            (Values[3] & Values[1] ) |
            (Values[4] & Values[0] ) );
}

/** is this in the underequal-cluster ? */
public boolean isUnderEqual(){
 return !Values[1] & !Values[4] &
        (  ( Values[3] & Values[5] ) |
           ( Values[3] & Values[2] ) |
           ( Values[5] & Values[0] ) );
}

/** is this in the overlap-cluster ? */
public boolean isOverlap(){
 return  (Values[4] & Values[5]) |
         (Values[4] & Values[2]) |
         (Values[5] & Values[1]) |
         (Values[1] & Values[2] & (Values[3]
          | Values[4] | Values[5]));
}


/** set the entry for given values */
public void computeValue(double Z1, double Z2){
if(Z1==0 & Z2==0){
  Values[0]=true;
  return;
}
if(Z1>0 & Z2==0){
  Values[1]=true;
  return;
}
if(Z1==0 & Z2>0){
   Values[2]=true;
   return;
}
// Z1 and Z2 are greater then 0
if(Z1==Z2)
   Values[3]=true;
if(Z1>Z2)
   Values[4] = true;
if(Z1<Z2)
   Values[5] = true;

}


public final int EMPTY      = 0;
public final int OVERLAP    = 1;
public final int OVEREQUAL  = 2;
public final int UNDER      = 3;
public final int UNDEREQUAL = 4;
public final int OVER       = 5;
public final int DISJOINT   = 6;
public final int EQUAL      = 7;


private boolean[] Values;


}
