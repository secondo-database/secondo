package fuzzyobjects.simple;

import fuzzyobjects.basic.*;
import sj.lang.ListExpr;


/**
 * a class representing fuzzy elementary points
 * @author Thomas Behr
 */
public class fEPoint implements SimpleObject{    //simple Point

/**
 * creates a new fEPoint
 * no check of vadility
 * @param BP the Point representing the location of this point
 * @param Z  the fuzziness of this Point in (0,1]
 */
public fEPoint(BasicPoint BP, double Z){
  this.BP = BP;
  this.Z  = Z;
}

/** get the x value of this point */
public int getX(){
   return BP.getX();
}

/** get the y value of this point */
public int getY(){
    return BP.getY();
}

/**
 * creates a new FEPoint
 * no Check of vadility
 * @param x,y the location of this Point
 * @param Z the fuzziness of this point
 */
public fEPoint(int x, int y, double Z){
  this.BP = new BasicPoint(x,y);
  this.Z = Z;
}

/**
 * returns a copy of this
 * @return a new fEPoint whith same location and fuzziness as this
 */
public fEPoint copy() {
   return new fEPoint(BP.copy(),Z);
}

/** returns min X of the Bounding Box */
public int getMinX(){ return BP.getX(); }
/** returns max X of the Bounding Box */
public int getMaxX(){ return BP.getX(); }
/** return min Y of the Bounding Box */
public int getMinY(){ return BP.getY(); }
/** return max Y of the Bounding Box */
public int getMaxY(){ return BP.getY(); }

/**
 * returns a readable String of this Point
 * @return a String representing this Point
 */
public String toString() {
   return "SP : (" + BP +"," + Z +")";
}

/**
 * check wether this is a valid fEPoint
 * @return  true if location is valid and fuzziness is in (0,1]
 */
public boolean isValid() {
  return BP.isValid() && Z>0  && Z<=1;
}


/**
 * returns the fuzziness of this Point
 * @return the fuzziness of this Point
 */
public double getZ() {
     return Z;
}

/**
 * get the location of this Point
 * @returns the location of this point
 */
public BasicObject basic(){
     return BP.copy();
}

/**
 * check whether this is equal to SP
 * @param SP the Point to checked
 * @return true if this and SP have same location and fuzziness
 */
public boolean equal(fEPoint SP){
   return BP.equals(SP.BP) && (Z==SP.Z);
}


/**
 * sorts a array of fEPoints
 * @param fEPs the array to sorted
 * @return the array sorted by basic
 */
public static void sort(fEPoint[] fEPs) {

for (int i=0; i<fEPs.length;i++)
  for (int j=i+1; j<fEPs.length;j++)
     if ( fEPs[i].basic().compareTo(fEPs[j].basic())==1) {
       fEPoint temp = fEPs[i];
       fEPs[i] = fEPs[j];
       fEPs[j] = temp;
     }
} // sort



/** converts this FEPoint to ListExpr 
  */
public ListExpr toListExpr(){
  return ListExpr.threeElemList(ListExpr.intAtom(BP.getX()), 
                                ListExpr.intAtom(BP.getY()),
					  ListExpr.realAtom((float) Z)); 
}


/** set this fEPoint to values from LE 
  * if LE is not a valid fEPoint this fEPoint is not changed
  * @return true if LE represent a valid fEPoint , false otherwise 
  */
public boolean readFromListExpr(ListExpr LE){
   if (LE==null)
      return false;
   if(LE.listLength()!=3)
      return false;
   ListExpr LE1 = LE.first();
   ListExpr LE2 = LE.second();
   ListExpr LE3 = LE.third(); 
   int x,y;
   float z;     
   if(LE1.isAtom() && LE1.atomType()==ListExpr.INT_ATOM){
      x = LE1.intValue();
    }
   else
      return false;

   if(LE2.isAtom() && LE2.atomType()==ListExpr.INT_ATOM){
      y = LE2.intValue();
    }
    else
      return false;

    if(LE3.isAtom() && ( LE3.atomType()==ListExpr.INT_ATOM | LE3.atomType()==ListExpr.REAL_ATOM))
       if (LE3.atomType()==ListExpr.INT_ATOM){
          z=LE3.intValue();
       }
       else{
          z=LE3.realValue(); 
       }
    else
       return false;

    if(z<0 | z>1)
       return false;

    BasicPoint P = new BasicPoint(x,y);
    if (!P.isValid()){
       return false;
    }
    this.BP=P;
    this.Z = z; 
    return true;  
}



/** the location of this Point */
private BasicPoint BP;
/** the fuzziness of this Point */
private double  Z;   // degree of membership

} // class

