package fuzzyobjects.composite;


public class M9Int{

static final int INTERIOR =0;
static final int BOUNDARY =1;
static final int EXTERIOR =2;


public static final int DISJOINT  = 3;
public static final int MEET      = 4;
public static final int INSIDE    = 5;
public static final int CONTAINS  = 6;
public static final int COVEREDBY = 7;
public static final int COVERS    = 8;
public static final int EQUALS    = 9;
public static final int OVERLAP   = 10;
public static final int UNDEFINED = 11;

/** creates a new 9-intersection-matrix with false-entries */
M9Int(){
  for(int i=0;i<9;i++) Values[i]=false;
}


/** returns a readable representation from this */
public String toString(){
int rel = getRelation();
String result;
switch(rel){
  case 3 : result = "disjoint";break;
  case 4 : result = "meet";break;
  case 5 : result = "inside";break;
  case 6 : result = "contains";break;
  case 7 : result = "coveredBy";break;
  case 8 : result = "covers";break;
  case 9 : result = "equals";break;
  case 10: result = "overlap";break;
  case 11: result = "undefined";break;
  default : result="error";
}
 return result;
}


/** returns a readyble representatioon from Relation */
public static String toString(int Relation){
String result;
switch(Relation){
  case 3 : result = "disjoint";break;
  case 4 : result = "meet";break;
  case 5 : result = "inside";break;
  case 6 : result = "contains";break;
  case 7 : result = "coveredBy";break;
  case 8 : result = "covers";break;
  case 9 : result = "equals";break;
  case 10: result = "overlap";break;
  case 11: result = "undefined";break;
  default : result="error";
}
 return result;
}


/**
 * set the entry in this matrix
 * @param value : the value to set
 * @param PartA : the Part of the first object
 * @param PartB : the Part of the second object
 * a Part can be INTERIOR, EXTERIOR OR BOUNDARY defined
 * in this class
 */
void setValue(boolean value, int PartA , int PartB){
 if(!(PartA<0 | PartB<0 | PartA>2 | PartB>2))
   Values[3*PartA+PartB]=value;
}


/** returns the entry on (PartA,PartB) */
boolean getValue(int PartA, int PartB){
 if(PartA<0 | PartB<0 | PartA>2 | PartB>2)
   return false;
 else
   return Values[3*PartA+PartB];
}

/** set all entries on false */
void reset(){
  // set all intersections false
  for(int i=0;i<9;i++) Values[i]=false;
}


/** creates the symmetry matrix */
public void makeSym(){
  boolean[] SymValues = new boolean[9];
  SymValues[0] = Values[0];
  SymValues[1] = Values[3];
  SymValues[2] = Values[6];
  SymValues[3] = Values[1];
  SymValues[4] = Values[4];
  SymValues[5] = Values[7];
  SymValues[6] = Values[2];
  SymValues[7] = Values[5];
  SymValues[8] = Values[8];
  Values=SymValues;
}



/** returns the cluster of this matrix */
int getRelation(){
  // return the relation, in which template is this Matrix

if( !Values[0] & !Values[1] & !Values[3] & !Values[4] )
   return DISJOINT;

if( !Values[0] & ( Values[1] | Values[3] | Values[4]) )
   return MEET;

if( Values[0] & !Values[2] & Values[6] & !Values[4] )
   return INSIDE;

if( Values[0] & !Values[6] & Values[2] & !Values[4] )
  return CONTAINS;

if( Values[0] & !Values[2] & Values[6] & Values[4] )
   return COVEREDBY;

if( Values[0] & !Values[6] & Values[2] & Values[4] )
  return COVERS;

if(!Values[2] & !Values[2] & !Values[3] & !Values[5] &
   !Values[6] & !Values[7])
  return EQUALS;

if( Values[0] & Values[2] & Values[6] )
  return OVERLAP;

return UNDEFINED;

}

/**  the entries */
private boolean[] Values = new boolean[9];

}
