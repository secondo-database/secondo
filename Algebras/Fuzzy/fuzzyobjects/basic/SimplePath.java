package fuzzyobjects.basic;

import java.util.Vector;

/**
 * this class manage a serie of connected BasicPoints
 * the SimplePath has no selfcuts but can build a circle
 */
public class SimplePath extends Path{

/** creates a new  empty SimplePath */
public SimplePath(){
  super();
}


/** extend the SimplePath with P
  * the first Point of P must be equals to the last Point of this SimplePath,
  * this SimplePath can not be a circle
  * this SimplePath and P can not have cuts
  * @return true if sucessfull
  */

public boolean extend(SimplePath P){

int length  = Points.size();
int Plength = P.Points.size();

if( Plength==0)
  return true;           // no change this SimplePath

if (length>1 &&              // a circle?
    ((BasicPoint)Points.get(0)).equals((BasicPoint)Points.get(length-1)))
    return false;

BasicPoint OldLast  = (BasicPoint)(Points.get(length-1));
BasicPoint NewFirst = (BasicPoint)(P.Points.get(0));

if( !OldLast.equals(NewFirst))  // connected ?
    return false;


boolean cut = false;
BasicPoint Current;
BasicPoint PCurrent;

for(int i=0;i<length;i++){
  Current = (BasicPoint) Points.get(i);
  for(int j=1;j<Plength;j++){
    PCurrent=(BasicPoint) P.Points.get(j);
    if((i>0) || (j<(Plength-1)))
       cut = cut || Current.equals(PCurrent);
  }
}

if(cut)
   return false;

// extend  the SimplePath
for(int j=1;j<Plength;j++)
    Points.add(P.Points.get(j));

if(P.maxX>maxX)
  maxX = P.maxX;
if(P.minX<minX)
  minX=P.minX;
if(P.maxY>maxY)
  maxY=P.maxY;
if(P.minY<minY)
  minY=P.minY;

return true;
}


/** computes a simple Path between P1 and P2 */
public static SimplePath computeSimplePath(BasicPoint P1,BasicPoint P2){
 SimplePath  result = new SimplePath();
 Path help = Path.computePath(P1,P2);
 result.Points = help.Points;
 result.minX = help.minX;
 result.maxX = help.maxX;
 result.minY = help.minY;
 result.maxY = help.maxY;
 return result;
}


/** extend this path to BP */
public boolean extend(BasicPoint BP){

// the first Point ?
int size = Points.size();

if (BP==null)
  return false;

if(size==0){
  Points.add(BP);
  minX = BP.getX();
  maxX = minX;
  minY = BP.getY();
  maxY = minY;
  return true;
}

// this SimplePath is a circle => can not extended
if(size>1 && Points.get(0).equals(Points.get(size-1)))
  return false;

boolean cut = false;
for(int i=1;i<size;i++)
  if(Points.get(i).equals(BP))
    cut=true;

if(cut)
  return false;

if(!BP.neightbooring( (BasicPoint) Points.get(size-1)))
  return false;

Points.add(BP);
int x = BP.getX();
int y = BP.getY();

if(x<minX)
  minX = x;
if(x>maxX)
  maxX = x;
if(y<minY)
  minY=y;
if(y>maxY)
  maxY=y;

return true;

}


/** check: first Point is equal to the last Point */
public boolean isACircle(){

int s = Points.size();

// min 3 points and  first point is equal to the last point

return ( s>2) &&
       ( (BasicPoint) Points.get(0)).equals((BasicPoint) Points.get(s-1));

}


/**
 * returns a BasicTriangle in the set of enclosed Triangles
 */
public BasicTriangle getAInnerTriangle(){
if(! isACircle())
   return null;

BasicSegment[] S = computeSegments();
BasicSegment Smallest = S[0];
BasicTriangle First;
BasicTriangle[] N = Smallest.getTriangles();
if( N[0].compareTo(N[1])<0)
  First = N[1];
else
  First = N[0];

return First;

}

/** returns the ordered segments of this path */
private BasicSegment[] computeSegments(){

Vector Segments = new Vector(Points.size());
int nOS = getNumberOfSegments();
BasicSegment BS;
BasicSegment MidBS;
int min,max,mid,comp,pos;

for(int i=0;i<nOS;i++){
  BS = getSegment(i);
  // search the insert-position
  if(Segments.size()==0)
     Segments.add(BS);
  else{ // not empty Vector
     min=0;
     max=Segments.size()-1;
     while(min<max){
       mid = (min+max)/2;
       MidBS = (BasicSegment) Segments.get(mid);
       comp = BS.compareTo(MidBS);
       if(comp==0){
          pos=mid;
          min=mid;
          max=mid;
       }
       if(comp<0){
          max = mid-1;
       }
       if(comp>0){
         min = mid+1;
       }
     } // while

     pos   = min;

     MidBS = (BasicSegment) Segments.get(pos);
     if(MidBS.compareTo(BS)<0)
       pos++;

     Segments.add(pos,getSegment(i));
  } // else (not empty Vector)
}// for all Segments

BasicSegment[] result = new BasicSegment[Segments.size()];
for(int i=0;i<result.length;i++){
   result[i]    = (BasicSegment) Segments.get(i);
}
 return result;
}


/** computes the triangles containing in this Path
 * the Path must be a circle
 */
public BasicTriangle[] getEnclosedTriangles(){

if(! isACircle())
  return null;


BasicSegment[] S    = computeSegments();

SortedBasic SB = new SortedBasic();
for(int i=0;i<S.length;i++)
   SB.insert(S[i]);

Vector result = new Vector();
BasicSegment Smallest;
BasicTriangle Current;
BasicSegment[] Sides;
int Pos;

while(!SB.isEmpty()) {
  Smallest = (BasicSegment)SB.get(0);
  BasicTriangle[] N = Smallest.getTriangles();
  if( N[0].compareTo(N[1])<0)
      Current = N[1];
  else
      Current = N[0];
  // Current is the smallest enclosed triangle
  result.add(Current);
  Sides = Current.getSides();
  for(int i=0;i<Sides.length;i++){
     Pos = SB.getPos(Sides[i]);
     if(Pos<0)
        SB.insert(Sides[i]);
     else
        SB.delete(Pos);
  } // for
} // while


BasicTriangle[] ret = new BasicTriangle[result.size()];
for(int i=0;i<ret.length;i++)
   ret[i] = (BasicTriangle) result.get(i);

return ret;

}


/** computes enclosed Triangles */
private void  computeEnclosedTriangles(BasicTriangle   Current,
                                       BasicSegment[]  Segments,
                                       boolean[]       used,
                                       Vector          result){


BasicTriangle test;  // Triangle allready in result
for(int i=0;i<result.size();i++){
  test = (BasicTriangle) result.get(i);
  if(test.equals(Current)){
    return;
  }
}


BasicSegment[] Sides = Current.getSides();
int pos1 = getPos(Segments,Sides[0]);
int pos2 = getPos(Segments,Sides[1]);
int pos3 = getPos(Segments,Sides[2]);

if( ( pos1>-1 && used[pos1]) |          // current in exterior
    ( pos2>-1 && used[pos2]) |
    ( pos3>-1 && used[pos3]) ) {
    return;
}

// set all used Segments
if( pos1>-1) used[pos1] = true;
if( pos2>-1) used[pos2] = true;
if( pos3>-1) used[pos3] = true;

// add to result

result.add(Current);

BasicTriangle[] Neightboors = Current.getRealNeightboors();
for(int i=0;i<Neightboors.length;i++)
  computeEnclosedTriangles(Neightboors[i],Segments,used,result);
}




/** search the pos from BS in Segs , when Segs is ordered */
private static int getPos(BasicSegment[] Segs,BasicSegment BS){

BasicSegment Current;
for(int i=0;i<Segs.length;i++){
  Current = Segs[i];
  if(Current.equals(BS))
     return i;
}
return -1;
  
}

/**
  * split the Path on BP
  * this is the path from 1 to BP
  * and result ist the path from BP to end
  * if BP not in this Path then is the result empty
  */
public SimplePath split(BasicPoint BP){
 boolean found = false;
 Vector FPoints = new Vector();
 SimplePath result = new SimplePath();
 BasicPoint Current;
 for(int i=0;i<Points.size();i++){
    Current = (BasicPoint) Points.get(i);
    if(!found)
      if(Current.equals(BP)){
        FPoints.add(Current);
        found=true;
        result.Points.add(Current);
      }
      else
        FPoints.add(Current);
    else // found
      result.Points.add(Current);
 }
 Points = FPoints;
 return result;
}





} // class
