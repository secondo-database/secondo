package fuzzyobjects.composite;

import fuzzyobjects.basic.*;
import fuzzyobjects.simple.*;
import java.util.Vector;
import sj.lang.ListExpr;

/**
 * this class provides a implementation of fuzzy Lines
 * in the X-triangulation
 * @author Thomas Behr
 */
public class FLine implements CompositeObject{

/**
 * the factor of scale
 */
protected double SF;        

/** the fuzzy segments */
protected SortedObjects  fSeg;  


/** returns the SortedObjects of this Line */
SortedObjects getSortedObjects(){
  return fSeg;
}



/** the bounding box */
protected BoundingBox BB = new BoundingBox();


/** returns the bounding box of this line */
public BoundingBox getBoundingBox(){ return BB; }


/** overwrite the given Path by a new FLine by
  * linear aprox.
  */
public boolean overwritePath(Path P, double Start, double End){
 if(P==null | Start<0 | Start>1 | End<0 | End>1)
   return false;
 else{
   boolean removePath = (Start+End==0);
   double length = P.getLength();
   double currentLength = 0;
   int max = P.getNumberOfSegments();
   double delta = End-Start;
   double CLength;
   BasicSegment C;
   double M1,M2;
   BasicPoint BP;
   for(int i=0;i<max;i++){
     C  = P.getSegment(i);
     BP = P.getPoint(i);
     CLength = C.length();
     if (removePath){
       fSeg.delete(C);
     }
     else {
       M1 = Start+delta*(currentLength/length);
       M2 = Start+delta*((currentLength+CLength)/length);
       if(BP.equals( (BasicPoint) C.getEP1()))
          overwrite(new fSegment(C,M1,M2));
       else
          overwrite(new fSegment(C,M2,M1));
     }
     currentLength += CLength;
   }
  return true;
 } // else
}


/** computes the bounding box of this line */
private void computeBoundingBox(){
if(fSeg.isEmpty())
  BB.setBox(0,0,0,0);
else{
  SimpleObject FS = fSeg.get(0);
  int minX = FS.getMinX();
  int minY = FS.getMinY();
  int maxX = FS.getMaxX();
  int maxY = FS.getMaxY();
  int cX,cY;
  for (int i=1;i<fSeg.getSize();i++){
    FS = fSeg.get(i);
    cX = FS.getMinX();
    cY = FS.getMinY();
    if(cX < minX)
      minX = cX;
    if(cY < minY)
      minY = cY;
    cX = FS.getMaxX();
    cY = FS.getMaxY();
    if(cX > maxX)
      maxX = cX;
    if(cY > maxY)
      maxY = cY;
  }
  BB.setBox(minX,minY,maxX,maxY);
}
}

/**
 * creates a new FLine
 * with scale as factor of scale
 */
public FLine(double scale){
   fSeg = new SortedObjects();
   SF  = scale;
}

/** set the new factor of scale */
public boolean setSF(double SF){
  if(SF>0){
    this.SF=SF;
    return true;
  }
  else return false;
}


/**
 * creates a new FLine
 * with 1 as factor of scale
 */
public FLine() {
    this(1.0);
}

/** get the factor of scale */
public double getSF(){ return SF; }

/**
 * get the dimension of a line
 * i.e. 1
 */
public int getDim(){ return 1; }

/**
 * is this a valid line in the X-triangulation ?
 * i.e. check whether SF greater then 0 and all
 * containing fuzzy segments are valid
 */
public boolean isValid(){
  return SF>0 & fSeg.allValid();
}


/**
 * returns a readable String of this FLine
 */
public String toString(){
  return "FLine (SF="+SF+")\n" + fSeg.toString();
}

/**
 * adds a new fuzzy Segment to this Line
 * if this FLine contains a segment whith same basic as fS
 * the fS not added and the result ist false
 */
public boolean add(fSegment fS){
   boolean first = fSeg.isEmpty();
   boolean ok = fSeg.insert(fS.copy());
  if(ok) {   // update bounding box
    if(!first){
       int minX = BB.getMinX();
       int minY = BB.getMinY();
       int maxX = BB.getMaxX();
       int maxY = BB.getMaxY();
       if(minX>fS.getMinX())
          minX = fS.getMinX();
       if(minY>fS.getMinY())
          minY = fS.getMinY();
       if(maxX<fS.getMaxX())
         maxX = fS.getMaxX();
       if(maxY<fS.getMaxY())
          maxY = fS.getMaxY();
       BB.setBox(minX,minY,maxX,maxY);
    }
    else
      BB.setBox(fS.getMinX(),fS.getMinY(),fS.getMaxX(),fS.getMaxY());
  }   
  return ok;
}

/**
 * updated a fuzzy Segments in this fLine
 * if not a segment whith same basic as fS in this
 * FLine then the result is false
 */
public boolean update(fSegment fS){
   return fSeg.update(fS.copy());
}

/**
 * updated/added a fuzzy Segment of this FLine
 * if this FLine contains a Segment wthis same basic as fS
 * then this Segment was updates
 * else fS is added to this FLine
 */
public void overwrite(fSegment fS){
   boolean first = fSeg.isEmpty();
   if (! fSeg.update(fS))
        fSeg.insert(fS);
   if(!first){
     int minX = BB.getMinX();
     int minY = BB.getMinY();
     int maxX = BB.getMaxX();
     int maxY = BB.getMaxY();
     if(minX>fS.getMinX())
        minX = fS.getMinX();
     if(minY>fS.getMinY())
        minY = fS.getMinY();
     if(maxX<fS.getMaxX())
       maxX = fS.getMaxX();
     if(maxY<fS.getMaxY())
       maxY = fS.getMaxY();
     BB.setBox(minX,minY,maxX,maxY);
   }
   else
      BB.setBox(fS.getMinX(),fS.getMinY(),fS.getMaxX(),fS.getMaxY());
}

/**
 * creates a new fuzzy Segments from the Parameters and
 * if this yield a valid fuzzy segment
 * then invoked <A href="#overwrite(fSegment)">
 * overwrite(fSegment) </A>
 */
public boolean overwrite(int x1,int y1,double Z1,
                         int x2,int y2,double Z2 ){

 fSegment fS = new fSegment(x1,y1,Z1, x2,y2,Z2);

 if (fS.isValid()){
   overwrite(fS);
 return true;
 }
 else return false;
}

/**
 * deletes the fuzzy Segment from this FLine
 * with basic BS
 * returns false if this FLine not contains such fSegment
 */
public boolean delete(BasicSegment BS){
  boolean ok = fSeg.delete(BS);
   if(ok & ( BS.getMinX()==BB.getMinX() |
             BS.getMaxX()==BB.getMaxX() |
             BS.getMinY()==BB.getMinY() |
             BS.getMaxY()==BB.getMaxY() )  )
        computeBoundingBox();
   return ok;
}

/**
 * computes all membershipvalues on (x,y)
 */
public double[] ZRel(double x, double y) {
double[] result;
BasicSegment[] Segs = BasicSegment.getSegments(x,y);
if(Segs == null){
   result = new double[1];
   result[0]=0;
}
else{
  Vector tmp = new Vector();
  fSegment current;
  Double Z;
  for(int i=0;i<Segs.length;i++){
      current = (fSegment) fSeg.search(Segs[i]);
      if (current!=null){
         Z = new Double(current.Zfkt(x,y));
         if(!tmp.contains(Z))
            tmp.add(Z);
      }
      else{
        if(!tmp.contains(new Double(0)))
           tmp.add(new Double(0));
     }
  }

  if(tmp.size()==0){
    result = new double[1];
    result[0]=0;
  }
  else{
     result=new double[tmp.size()];
     for(int i=0;i<tmp.size();i++)
        result[i] = ((Double)tmp.get(i)).doubleValue();
  }
}
return result;
}  // ZRel



/** computes all membershipvalues on BP */
public double[] ZRel(BasicPoint BP){
Vector tmp = new Vector();
BasicPoint[] Neightboors = BP.getNeightboors();

for(int i=0;i<Neightboors.length;i++){
  fSegment fS= (fSegment) fSeg.search(new BasicSegment(BP,Neightboors[i]));
  if(fS!=null)
    tmp.add(new Double( fS.Zfkt(BP)));
} // for all Neighboors

double[] result = new double[tmp.size()];
for(int j=0;j<tmp.size();j++)
  result[j] =  ( (Double) tmp.get(j)).doubleValue();

return result;
}

/** computes the maximal membershipvalue on BP */
public double maxZfkt(BasicPoint BP){
double[] cand = ZRel(BP);
double result = 0;
for(int i=0;i<cand.length;i++)
  if(cand[i]>result)
    result=cand[i];
return result;
}


/** computes the maximal membershipvalue on (x,y) */
public double maxZfkt(double x, double y){
  double[] values = ZRel(x,y);
  double result =0;
  for(int i=0;i<values.length;i++)
    if (values[i]>result) result = values[i];
  return result;
}

/** computes the minimal membershipvalue on (x,y) */
public double minZfkt(double x, double y){
  double[] values = ZRel(x,y);
  double result =1;
  for(int i=0;i<values.length;i++)
    if (values[i]<result) result = values[i];
  return result;
}

/** computes the middle membershipvalue on (x,y) */
public double midZfkt(double x, double y){
  double[] values = ZRel(x,y);
  double result =0;
  for(int i=0;i<values.length;i++)
     result += values[i];
  result = result / values.length;
  return result;
}


/** computes the membershipvalue on BP by given basic element */
public double Zfkt(BasicPoint BP, BasicSegment BS){

 fSegment Fs = (fSegment) fSeg.search(BS);
 double result = 0;

 if(Fs!=null) {
   result = Fs.Zfkt(BP);
 }
 return result;
}

/** computes the membershipvalue on (x,y) by given basic segment */
public double Zfkt(double x, double y, BasicSegment BS){
 double result = 0;
 if (BS.contains(x,y)){
    fSegment Fs = (fSegment) fSeg.search(BS);
    if(Fs!=null) {
       result = Fs.Zfkt(x,y);
    }
 }
 return result;
}


/** returns all containing basic segments of this FLine
  * the Segments of result are sorted*/
public BasicSegment[] basics() {
  BasicSegment[] result = new BasicSegment[fSeg.getSize()];
  if(fSeg.isEmpty())
     return result;
  else{
    for(int i=0;i<fSeg.getSize();i++)
      result[i] = (BasicSegment) ((fSegment)fSeg.get(i)).basic();
    }
     return result; 
} // basics


/** returns the maximal membershipvalue in this FLine */
public double maxZ(){
 double max = 0.0;
 fSegment current;
 for(int i=0;i<fSeg.getSize();i++){
    current = (fSegment) fSeg.get(i);
     if (max < Math.max(current.getZ1(),current.getZ2() ))
        max = Math.max(current.getZ1(), current.getZ2());
  }
  return max;
} // maxZ

/** returns the minimal membershipvalue in this FLine */
public double minZ(){
 double min = 1.0;
 fSegment current;
 for(int i=0;i<fSeg.getSize();i++){
    current = (fSegment) fSeg.get(i);
     if (min > Math.min(current.getZ1(),current.getZ2()) )
         min = Math.min(current.getZ1(), current.getZ2());
  }
  return min;
} // minZ

/** returns a copy from this FLine */
public FLine copy(){
  FLine C = new FLine(SF);
  C.fSeg = fSeg.copy();
  C.BB = BB.copy();
  return C;
}

/** check whether this FLine contains elements */
public boolean isEmpty(){
 return fSeg.isEmpty();
}


/** set the containing segments from this on the same
  * as L2's one
  */
protected static void setfSeg(FLine L1, FLine L2){
  L1.fSeg = L2.fSeg;
  L1.BB = L2.BB;
}



/** computes the selfcuts of this Line */
public BasicPoint[] selfcuts(){
BasicSegment Current;
BasicPoint BP;
int        n;
boolean    found;
Vector tmp = new Vector(fSeg.getSize());
 for(int i=0;i<fSeg.getSize();i++){
    Current = (BasicSegment) ((fSegment)fSeg.get(i)).basic();
    BP = Current.getEP1();
    n = numberOfSegments(BP);
    if(n>2){
       found=false;
       for(int j=0;j<tmp.size();j++)
          if( ( (BasicPoint) tmp.get(j)).equals(BP))
              found=true;
       if(!found)
         tmp.add(BP);
    }
    BP = Current.getEP2();
    n = numberOfSegments(BP);
    if(n>2){
       found=false;
       for(int j=0;j<tmp.size();j++)
          if( ( (BasicPoint) tmp.get(j)).equals(BP))
              found=true;
       if(!found)
         tmp.add(BP);
    } // if
 } // for all containing segments

 BasicPoint[] result = new BasicPoint[tmp.size()];
 for(int i=0;i<result.length;i++)
   result[i] = (BasicPoint) tmp.get(i);
 return result;
} // selfcuts


/*************************************************************
   Operators
 *************************************************************/

/** the union of 2 FLines */
public FLine union(FLine With){
  return  operator(With,UNION);
}

/** the scaled union of 2 FLines */
public FLine scaledUnion(FLine With){
  FLine result =  operator(With,SCALEDUNION);
  result.norm();
  return result;
}

/** the intersectionof 2 FLines */
public FLine intersection(FLine With){
   return operator(With,INTERSECTION);
}

/** the scaled intersection of 2 FLines */
public FLine scaledIntersection(FLine With){
  FLine result = operator(With,SCALEDINTERSECTION);
  result.norm();
  return result;
}

/** added With to this FLine */
public FLine add(FLine With){
   return operator(With,ADD);
}

/** added scaled Witdh to this FLine */
public FLine scaledAdd(FLine With){
  FLine result = operator(With,SCALEDADD);
  result.norm();
  return result;
}

/** computes the difference from this FLine and With */
public FLine difference(FLine With){
   return operator(With,SUBTRACT);
}

/** computes the scaled difference from this FLine and With */
public FLine scaledDifference(FLine With){
  FLine result = operator(With,SCALEDDIFFERENCE);
  result.norm();
  return result;
}

/**
  * computes the alpha-cut of this FLine
  * i.e. removes all FSegments whith a middle membershipvalue
  * less (less or equal) as alpha
  */
public FLine alphaCut(double alpha, boolean strong){
 FLine CutL = new FLine(SF);
 fSegment Current;
 boolean  isValid;
 double midZ;

 for(int i=0; i<fSeg.getSize(); i++){
    Current = (fSegment) fSeg.get(i);
    midZ = (Current.getZ1()+ Current.getZ2())/2;
    if (strong)
       isValid = midZ>alpha;
    else
       isValid = midZ>= alpha;

    if(isValid)
       CutL.fSeg.insert(Current.copy());
 }
 CutL.computeBoundingBox();
 return CutL;
}

/**
 * computes the length of the basic of this FLine
 */
public double basicLen(){
 double sum =  0.0;
 for(int i=0;i<fSeg.getSize();i++){
    sum += ((BasicSegment) fSeg.get(i).basic()).length();
 }
 return sum;
}

/**
 * computes the len of the 3d-structure of this FLine
 */
public double len3D(){
 double sum = 0.0;
 for(int i=0;i<fSeg.getSize();i++){
  sum += ((fSegment) fSeg.get(i)).length();
 }
 return sum;
}

/**
 * computes the weigthed length of this FLine
 */
public double length(){
  double sum = 0.0;
  double Z;
  fSegment Current;
  for(int i=0;i<fSeg.getSize();i++){
    Current = (fSegment) fSeg.get(i);
    Z = (Current.getZ1() + Current.getZ2())/2;
    sum +=  Z*( (BasicSegment) Current.basic()).length();
  }
  return sum;
}


/**
 * computes how similar is the basic of 2 FLines
 */
public double basicSimilar(FLine L2){
     FLine Funion         = this.union(L2);
     FLine Fintersection  = this.intersection(L2);
     if (Funion.isEmpty())
        return 1.0;
     else
        return Fintersection.basicLen() / Funion.basicLen();
}

/**
 * how similar are 2 FLines ?
 */
public double similar(FLine L2){
     FLine Funion         = this.union(L2);
     FLine Fintersection  = this.intersection(L2);
     if (Funion.isEmpty())
        return 1.0;
     else
        return Fintersection.length() / Funion.length();
}


/**
 * make this FLine sharp
 * i.e. the membershipvalues of the result are ever 1.0
 */
public FLine sharp(){
 FLine result = new FLine(1);
 fSegment CurrentSegment;
 for(int i=0;i<fSeg.getSize();i++){
   CurrentSegment = new fSegment( (BasicSegment) (fSeg.get(i).basic()),1,1);
   result.add(CurrentSegment);
 }
  return result;
}


/**
 * computes the boundary of this FLine
 */
public FPoint boundary(){
 return endPoints();
}

/**
 * computes the boundary of this FLine
 */
public FPoint endPoints(){
FPoint result = new FPoint(1);

BasicSegment current;
BasicPoint EP1;
BasicPoint EP2;
BasicPoint[] Neightboors;
boolean found;

for(int i=0;i<fSeg.getSize();i++){
 // a endPoint of each segment is a candidate
 current = (BasicSegment) fSeg.get(i).basic();
 EP1 = current.getEP1();
 EP2 = current.getEP2();
 // check EP1
 Neightboors = EP1.getNeightboors();
 found=false;
 for(int j=0;j<Neightboors.length&!found;j++){
   if(!EP2.equals(Neightboors[j])) { // exclude current Basicsegment
     found = fSeg.search(new BasicSegment(EP1,Neightboors[j]))!=null;
   }
 }
 if(!found)  // EP1 is a endpoint
   result.add(new fEPoint(EP1,1));
 // check EP2
 Neightboors = EP2.getNeightboors();
 found=false;
 for(int j=0;j<Neightboors.length&!found;j++){
   if(!EP1.equals(Neightboors[j])) {
     found = fSeg.search(new BasicSegment(EP2,Neightboors[j]))!=null;
   }
 }
 if(!found) // EP2 is a endpoint
   result.add(new fEPoint(EP2,1));

}

return result;
}


/**
 * computed the maximal connected part of this FLine
 * containing FS
 */
private void getComponent(fSegment FS, FLine result){
// computes a connected part of a line

 BasicSegment BS = (BasicSegment) FS.basic();
 result.add(FS);
 BasicSegment[] Neightboors = BS.getNeightboors();

 for(int i=0;i<Neightboors.length;i++) {
    if(result.fSeg.getPos(Neightboors[i])<0){      // not in result
      fSegment NB = (fSegment) this.fSeg.search(Neightboors[i]);
      if(NB!=null){
        getComponent(NB,result);
      }
    }
 }

}


public boolean equals(FLine L2){
  return fSeg.equals(L2.fSeg) & SF==L2.SF;
}

/**
 * computes the connected parts of this FLine
 */
public FLine[] faces(){
  if(isEmpty()) return null;
  Vector tmp = new Vector();  // for the faces ;
                              // use Vector hence number of faces unknow
  FLine Copy = this.copy();

  while( ! Copy.isEmpty()){
    FLine nextFace = new FLine(SF);
    fSegment First = (fSegment) Copy.fSeg.get(0);
    getComponent(First,nextFace);
    tmp.add(nextFace);
    Copy = Copy.difference(nextFace);
  }

  FLine[] result = new FLine[tmp.size()];
  for(int i=0;i<tmp.size();i++)
     result[i] = (FLine) tmp.get(i);

  return result;
}


/**
 * check whether this FLine contains BP
 */
public boolean contains(BasicPoint BP){
 //  returns true, if BP a Point of this Line
 return numberOfSegments(BP)>0;
}

/**
 * check whether BP is on boundary of this FLine
 */
public boolean isEndPoint(BasicPoint BP){
  return numberOfSegments(BP)==1;
}

/**
 * returns the number of Basicsegments containing in this FLine
 * having BP as endpoint
 */
public int numberOfSegments(BasicPoint BP){
 BasicPoint[] Neightboors = BP.getNeightboors();
 int found =0;
 for(int i=0;i<Neightboors.length;i++)
    if (fSeg.search(new BasicSegment(BP,Neightboors[i]))!=null)
        found ++;
 return found;
}

/**
 * a implementation of the mid-operator for a set of FLines
 */
public static FLine mid(FLine[] Lns){
if(Lns.length==0) return null;
FLine result    = new FLine(1);
int[] current     = new int[Lns.length];
int[] max         =  new int[Lns.length];
boolean[] ready   = new boolean[Lns.length];
boolean allready  = true;

// initialize the variables
for(int i=0;i<Lns.length;i++){
   current[i]=0;
   max[i]= Lns[i].fSeg.getSize();
   ready[i] = current[i]<max[i];
   if(!ready[i]) allready=false;
}

Vector allSmallest = new Vector(); // the smallest Segments
Vector Numbers     = new Vector(); // the positions in Lns

while(!allready){
  allready=true;
  allSmallest = new Vector();
  Numbers = new Vector();
  // search all smallest Segment
  fSegment currentS;
  BasicSegment compareS;
  for(int i=0;i<Lns.length;i++){
     if(current[i]<max[i]) {
       allready=false;
       currentS = (fSegment) Lns[i].fSeg.get(current[i]);
       if(allSmallest.size()==0) {
          allSmallest.add(currentS);
          Numbers.add(new Integer(i));
       }
       else {
        compareS = (BasicSegment) ((fSegment) allSmallest.get(0)).basic();
        int comp = currentS.basic().compareTo(compareS);
        if(comp==0) {  // a Segement with smallest Basic
          allSmallest.add(currentS);
          Numbers.add(new Integer(i));
        }
        if(comp<0) {  // new smallest Segment
         allSmallest = new Vector();
         Numbers = new Vector();
         allSmallest.add(currentS);
         Numbers.add(new Integer(i));
        }
        // in the case comp>0 is nothing to do
      } // not the first Segment
    } // current Line contains unprocessing Segments
  } // for all Lines

  if(!allready){
    double Z1=0,Z2=0;
    BasicSegment BS = (BasicSegment) ((fSegment) allSmallest.get(0)).basic();
    fSegment CS;
    for(int i=0;i<allSmallest.size();i++){
       // update numbers
       int c = ((Integer)Numbers.get(i)).intValue();
       current[c]++;
       CS = (fSegment) allSmallest.get(i);
       Z1 += CS.getZ1();
       Z2 += CS.getZ2();
    }
    Z1 = Z1 / Lns.length;
    Z2 = Z2 / Lns.length;
    result.fSeg.insert( new fSegment(BS,Z1,Z2));
  }

}  // while not all ready
 result.computeBoundingBox();
 return result;
}

/**
 * a implementation of the scaled mid - operator for a set of FLines
 */
public static FLine scaledMid(FLine[] Lns){
if(Lns.length==0) return null;
FLine result    = new FLine(1);
int[] current     = new int[Lns.length];
int[] max         =  new int[Lns.length];
boolean[] ready   = new boolean[Lns.length];
boolean allready  = true;

// initialize the variables
for(int i=0;i<Lns.length;i++){
   current[i]=0;
   max[i]= Lns[i].fSeg.getSize();
   ready[i] = current[i]<max[i];
   if(!ready[i]) allready=false;
}

Vector allSmallest = new Vector(); // the smallest Segments
Vector Numbers     = new Vector(); // the positions in Lns

while(!allready){
  allready=true;
  allSmallest = new Vector();
  Numbers = new Vector();
  // search all smallest Segment
  fSegment currentS;
  BasicSegment compareS;
  for(int i=0;i<Lns.length;i++){
     if(current[i]<max[i]) {
       allready=false;
       currentS = (fSegment) Lns[i].fSeg.get(current[i]);
       if(allSmallest.size()==0) {
          allSmallest.add(currentS);
          Numbers.add(new Integer(i));
       }
       else {
        compareS = (BasicSegment) ((fSegment) allSmallest.get(0)).basic();
        int comp = currentS.basic().compareTo(compareS);
        if(comp==0) {  // a Segement with smallest Basic
          allSmallest.add(currentS);
          Numbers.add(new Integer(i));
        }
        if(comp<0) {  // new smallest Segment
         allSmallest = new Vector();
         Numbers = new Vector();
         allSmallest.add(currentS);
         Numbers.add(new Integer(i));
        }
        // in the case comp>0 is nothing to do
      } // not the first Segment
    } // current Line contains unprocessing Segments
  } // for all Lines

  if(!allready){
    double Z1=0,Z2=0;
    BasicSegment BS = (BasicSegment) ((fSegment) allSmallest.get(0)).basic();
    fSegment CS;
    for(int i=0;i<allSmallest.size();i++){
       // update numbers
       int c = ((Integer)Numbers.get(i)).intValue();
       current[c]++;
       CS = (fSegment) allSmallest.get(i);
       Z1 += CS.getZ1()*Lns[c].SF;
       Z2 += CS.getZ2()*Lns[c].SF;
    }
    Z1 = Z1 / Lns.length;
    Z2 = Z2 / Lns.length;
    result.fSeg.insert( new fSegment(BS,Z1,Z2));
  }

}  // while not all ready
 result.norm();
 result.computeBoundingBox();
 return result;
}

/**
 * computes the common points of 2 FLines
 * i.e. Points containing in this and L2  <b>and </b>
 * not  endpoint of a common segment of this FLines
 */
public FPoint commonPoints(FLine L2){
FPoint result = new FPoint();
FLine SI = operator(L2,SHARPINTERSECTION);
BasicSegment current;
BasicPoint BP1,BP2;

for(int i=0;i<fSeg.getSize();i++){
    current = (BasicSegment) fSeg.get(i).basic();
    BP1 = current.getEP1();
    BP2 = current.getEP2();
    if (  L2.contains(BP1)   && // a common point
          !SI.contains(BP1) )    // not a point of a common Line
       result.add(new fEPoint(BP1,1));
    if(  L2.contains(BP2)  &&
        !SI.contains(BP2) )
       result.add(new fEPoint(BP2,1));
} // for

return result;

}


/************************************************************
   end of operators
 *************************************************************/

/** process a single Segment for many operators */
private void processElements(fSegment F1, double scale1,
                             fSegment F2, double scale2,
                             FLine Goal,
                             int Operator){

// 1 input parameter can be null 
// if both fTriangles not null, then they must have the same basic

  if( F1==null & F2==null) return;

  double Z1,Z2;
  fSegment newFS;

  switch (Operator){

     case UNION  :  {  // the union of 2 Lines ignoring SFs

                      if(F1==null)
                          Goal.fSeg.insert(F2.copy());
                      else
                         if(F2==null)
                           Goal.fSeg.insert(F1.copy());
                         else { // both fTriangles are not null
                           Z1 = Math.max(F1.getZ1(),F2.getZ1());
                           Z2 = Math.max(F1.getZ2(),F2.getZ2());
                           newFS = new fSegment((BasicSegment)
                                                 F1.basic(),
                                                 Z1,Z2);
                           Goal.fSeg.insert(newFS);

                         } // else

                    }
                    break;

    case INTERSECTION :
                    {  if (F1==null | F2==null)
                         ;  
                       else { // both are not null
                           Z1 = Math.min(F1.getZ1(),F2.getZ1());
                           Z2 = Math.min(F1.getZ2(),F2.getZ2());
                           if(Z1+Z2>0){
                              newFS = new fSegment((BasicSegment)
                                                    F1.basic(),
                                                    Z1,Z2 );
                              Goal.fSeg.insert(newFS);
                           }
                       }

                    } break;

    case ADD     :  {
                      if(F1==null)
                          Goal.fSeg.insert(F2.copy());
                      else
                         if(F2==null)
                           Goal.fSeg.insert(F1.copy());
                         else { // both fSegments are not null
                           Z1 = Math.min(1,F1.getZ1()+F2.getZ1());
                           Z2 = Math.min(1,F1.getZ2()+F2.getZ2());
                           newFS = new fSegment((BasicSegment)
                                                 F1.basic(),
                                                 Z1,Z2);
                           Goal.fSeg.insert(newFS);

                         } // else

                    }
                    break;

    case SUBTRACT :{
                        if(F1 == null)
                            ;
                        else
                           if(F2==null)
                             Goal.fSeg.insert(F1.copy());
                           else  {   // both not null
                              Z1 = Math.max(0,F1.getZ1()-F2.getZ1());
                              Z2 = Math.max(0,F1.getZ2()-F2.getZ2());
                              if ((Z1+Z2)>0) {
                                newFS = new fSegment( (BasicSegment)
                                                       F1.basic(),Z1,Z2);
                                Goal.fSeg.insert(newFS);
                              }
                           }
                       } break;


        case SCALEDUNION    :
                      { fSegment newSegment;
                        if (F1==null) 
                           newSegment = new fSegment( (BasicSegment)
                                                       F2.basic(),
                                                       F2.getZ1()*scale2,
                                                       F2.getZ2()*scale2);
                         else
                           if (F2==null)
                              newSegment = new fSegment( (BasicSegment)
                                                          F1.basic(),
                                                          F1.getZ1()*scale1,
                                                          F1.getZ2()*scale1);
                           else {
                             Z1 = Math.max(F1.getZ1()*scale1,
                                                  F2.getZ1()*scale2);
                             Z2 = Math.max(F1.getZ2()*scale1,
                                                  F2.getZ2()*scale2);

                             newSegment = new fSegment( (BasicSegment)
                                                           F1.basic(),
                                                           Z1,Z2);
                           }  // else
                       Goal.add(newSegment);

                      } break;   // scaled union

     
        case SCALEDINTERSECTION :  if (F1==null || F2==null)
                                         ;
                                   else {
                                     Z1 = Math.min(F1.getZ1()*scale1,
                                                         F2.getZ1()*scale2);
                                     Z2 = Math.min(F1.getZ2()*scale1,
                                                         F2.getZ2()*scale2);

                                    Goal.add(new fSegment( (BasicSegment)
                                                            F1.basic(),
                                                            Z1,Z2));
                                   } break;

        case SCALEDADD : {
                          if(F1==null)
                             Goal.add(new fSegment( (BasicSegment)
                                                     F2.basic(),
                                                     F2.getZ1()*scale2,
                                                     F2.getZ2()*scale2));
                          else
                            if(F2==null)
                              Goal.add(new fSegment( (BasicSegment)
                                                      F1.basic(),
                                                      F1.getZ1()*scale1,
                                                      F1.getZ2()*scale1));
                            else {

                              Goal.add(new fSegment( (BasicSegment)
                                                      F1.basic(),
                                                      F1.getZ1()*scale1 +
                                                      F2.getZ1()*scale2 ,
                                                      F1.getZ2()*scale1 +
                                                      F2.getZ2()*scale2 ));

                            }

                        } break;


        case SCALEDDIFFERENCE   :
                       {
                          if(F1==null)
                             Goal.add(new fSegment( (BasicSegment)
                                                     F2.basic(),
                                                     -F2.getZ1()*scale2,
                                                     -F2.getZ2()*scale2));
                          else
                            if(F2==null)
                              Goal.add(new fSegment( (BasicSegment)
                                                      F1.basic(),
                                                      F1.getZ1()*scale1,
                                                      F1.getZ2()*scale1));
                            else {

                              Goal.add(new fSegment( (BasicSegment)
                                                      F1.basic(),
                                                      F1.getZ1()*scale1 -
                                                      F2.getZ1()*scale2 ,
                                                      F1.getZ2()*scale1 -
                                                      F2.getZ2()*scale2 ));


                            }

                        } break;

    case SHARPINTERSECTION :
                    {  if (F1==null | F2==null)
                         ;  
                       else {
                         newFS = new fSegment((BasicSegment) F1.basic(),1,1);
                         Goal.fSeg.insert(newFS);
                       }
                    } break;

   

    default      : System.out.println("unimplementierter operator");


  } // switch

} // processElements


/** the 'template' for many operators */
protected FLine operator(FLine FL, int op){
// a kind of mergesort

int my     = 0;
int fromFL = 0;   // already processed

int maxMy     = fSeg.getSize();   // numbers of elements
int maxFromFL = FL.fSeg.getSize();
FLine result = new FLine(1);

   fSegment myFirst=null;  // the first unprocessed elements
   fSegment FLFirst=null;

if(maxMy>0)
   myFirst = (fSegment) fSeg.get(0);
if(maxFromFL>0)
   FLFirst = (fSegment) FL.fSeg.get(0);

if (maxMy >0 && maxFromFL>0){
   myFirst = (fSegment) fSeg.get(my);    
   FLFirst = (fSegment) FL.fSeg.get(fromFL);

   int compareResult;

   while(my<maxMy && fromFL<maxFromFL){  // both sets have unprocessed elements

      compareResult = myFirst.basic().compareTo(FLFirst.basic());
      if(compareResult < 0) {
         processElements(myFirst,SF,null,FL.SF,result,op);
         my++;
         if (my<maxMy)
             myFirst = (fSegment) fSeg.get(my);
      }
      else if(compareResult > 0){
            processElements(null,SF,FLFirst,FL.SF,result,op);
            fromFL++;
            if(fromFL<maxFromFL)
               FLFirst = (fSegment) FL.fSeg.get(fromFL);
           }
           else {     // elements have the same basic
             processElements(myFirst,SF,FLFirst,FL.SF,result,op);
             my++;
             fromFL++;
             if (my<maxMy)
               myFirst = (fSegment) fSeg.get(my);
             if (fromFL<maxFromFL)
               FLFirst = (fSegment) FL.fSeg.get(fromFL);
           }
   } // while
} // if 

// elements from one (or both) regions are processed


while(my < maxMy){    // this have still elements
   processElements(myFirst,SF,null,FL.SF,result,op);
   my++;
   if (my<maxMy)
      myFirst = (fSegment) fSeg.get(my);
}


while (fromFL < maxFromFL){  // FL have still elements
   processElements(null,SF,FLFirst,FL.SF,result,op);
   fromFL++;
   if(fromFL<maxFromFL)
      FLFirst = (fSegment) FL.fSeg.get(fromFL);
}
  result.computeBoundingBox();
  return result;

}

/** normalize this FLine */
private void norm(){

 if (isEmpty()) return;  // nothing to do
 // first compute Zmin and Zmax
 double Zmin = 0;
 double Zmax = 0;
 fSegment Current;

 for (int i=0; i< fSeg.getSize();i++){
   Current = (fSegment) fSeg.get(i);
   if(Current.getMaxZ()>Zmax)
      Zmax = Current.getMaxZ();
   if(Current.getMinZ()<Zmin)
      Zmin = Current.getMinZ();
 }

 if(Zmin > 0) Zmin=0;
 
 if(Zmax==0 & Zmin==0)
    fSeg.makeEmpty();
 else{
    double SFnew = Zmax - Zmin;
    SortedObjects newfSeg = new SortedObjects();
    double Z1,Z2,Z3;
    fSegment fSnew;

    for(int i=0;i<fSeg.getSize();i++){
       Current = (fSegment) fSeg.get(i);
       Z1 = Current.getZ1();
       Z2 = Current.getZ2();
       fSnew = new fSegment( (BasicSegment) Current.basic(),
                             (Z1-Zmin)/SFnew ,
                             (Z2-Zmin)/SFnew );
       if (fSnew.getMaxZ() >0)
          newfSeg.insert(fSnew);
   } // for
   SF = SFnew*SF;
   fSeg = newfSeg;
 }
 computeBoundingBox();
}



/******************************************************************
 *                                                                *
 *              topological Relationships                         *
 *                                                                *
 ******************************************************************/


/******************************************************************
 *                                                                *
 *             Topology in the basic                              *
 *                                                                *
 ******************************************************************/


/** helping method to compute the top Rel in the basic */
private static void checkPointsBasic(
                          int Segs1, int Segs2,   
                          M9Int goal ){           

/* this method checks the intersection of two lines in a BasicPoint BP
   Paramters :
      Segs1, Segs2 : numberOfSegments of each Line in BP
      Z1,Z2        : maxZfkt(BP) for each Line
      fuzzy        : basic or fuzzy  intersections ?
      goal         : here saving the result(s)
*/

if(Segs1==0) {    // exterior of L1
  if(Segs2==0)  // exterior
     goal.setValue(true,M9Int.EXTERIOR,M9Int.EXTERIOR);
  if(Segs2==1)  // boundary
     goal.setValue(true,M9Int.EXTERIOR,M9Int.BOUNDARY);
  if(Segs2>1){   // interior
      goal.setValue(true,M9Int.EXTERIOR,M9Int.INTERIOR);
  } // interior of l2
} // exterior of L1

if(Segs1==1) {     // boundary of L1
 if(Segs2==0)
    goal.setValue(true,M9Int.BOUNDARY,M9Int.EXTERIOR);
 if(Segs2==1)
    goal.setValue(true,M9Int.BOUNDARY,M9Int.BOUNDARY);
 if(Segs2>1){   // interior of L2
    goal.setValue(true,M9Int.BOUNDARY,M9Int.INTERIOR);
 }// interior of L2
} // boundary of L1

if(Segs1>1)   {     // a interior Point of Line 1
  if(Segs2==0){
    goal.setValue(true,M9Int.INTERIOR,M9Int.EXTERIOR);
  }  // Segs1==0

  if(Segs2==1){  // boundary of L2
    goal.setValue(true,M9Int.INTERIOR,M9Int.BOUNDARY);
  } // boundary of L2

 if(Segs2>1){    // intersection of both interiors
   goal.setValue(true,M9Int.INTERIOR,M9Int.INTERIOR);
 } // both interiors

} // interior of L1

} // checkPointsBasic



/** helping method to compute top Rel in the basic */
private static void checkEndPoints(FLine L1,FLine L2,
                                  BasicPoint BP,
                                  M9Int goal){
int Segs1 = L1.numberOfSegments(BP);
int Segs2 = L2.numberOfSegments(BP);
checkPointsBasic(Segs1,Segs2,goal);

}


/**
 * computed the 9-Intersection-matrix from this FLine and L2
 */
M9Int basicTopolRelation(FLine L2){

M9Int result = new M9Int();

if (this.isEmpty() || L2.isEmpty()) return null;

result.setValue(true,M9Int.EXTERIOR,M9Int.EXTERIOR);

int currentThis=0;
int currentL2=0;
int maxThis = fSeg.getSize();
int maxL2   = L2.fSeg.getSize();

fSegment     thisFirst;
fSegment     L2First;
BasicSegment thisBasic;
BasicSegment L2Basic;
BasicSegment Smallest=null;
BasicPoint   BP1,BP2;
int compare;
int thisSegs;
int L2Segs;

while(currentThis<maxThis & currentL2<maxL2){
     thisFirst = (fSegment) fSeg.get(currentThis);
     L2First   = (fSegment) L2.fSeg.get(currentL2);
     thisBasic = (BasicSegment) thisFirst.basic();
     L2Basic   = (BasicSegment) L2First.basic();
     compare = (thisBasic.compareTo(L2Basic));

     if( compare==0)  {    // a common Segment
        currentThis++;
        currentL2++;
        result.setValue(true,M9Int.INTERIOR,M9Int.INTERIOR);
        Smallest=thisBasic; // special role of endpoints
     }
     if(compare < 0) { // process single segment of this
        result.setValue(true,M9Int.INTERIOR,M9Int.EXTERIOR);
        Smallest=thisBasic;
        currentThis++;
     }
     if(compare > 0) {  // process single Segment of L2
       result.setValue(true,M9Int.EXTERIOR,M9Int.INTERIOR);
       Smallest=L2Basic;
       currentL2++;
     }
     // process Endpoints of Segment(s)
     BP1 = Smallest.getEP1();
     BP2 = Smallest.getEP2();
     checkEndPoints(this,L2,BP1,result);
     checkEndPoints(this,L2,BP2,result);
}// while

while(currentThis<maxThis){
  thisFirst = (fSegment) fSeg.get(currentThis);
  currentThis++;
  thisBasic = (BasicSegment) thisFirst.basic();
  result.setValue(true,M9Int.INTERIOR,M9Int.EXTERIOR);
  BP1 = thisBasic.getEP1();
  BP2 = thisBasic.getEP2();
  checkEndPoints(this,L2,BP1,result);
  checkEndPoints(this,L2,BP2,result);
}

while(currentL2<maxL2){
  L2First = (fSegment) L2.fSeg.get(currentL2);
  currentL2++;
  L2Basic = (BasicSegment) L2First.basic();
  result.setValue(true,M9Int.EXTERIOR,M9Int.INTERIOR);
  BP1 = L2Basic.getEP1();
  BP2 = L2Basic.getEP2();
  checkEndPoints(this,L2,BP1,result);
  checkEndPoints(this,L2,BP2,result);
}

return result;

} // basicTopolRelation




/** computes the 9-intersection-matrix from this and R */
M9Int basicTopolRelation(FRegion R){

M9Int result  = new M9Int();

// the 'defaults'
result.setValue(true,M9Int.EXTERIOR,M9Int.EXTERIOR);
result.setValue(true,M9Int.EXTERIOR,M9Int.INTERIOR);


fSegment     currentSeg;
BasicSegment currentBS;
int TriNumberSeg;
int SegNumber1,SegNumber2;
BasicPoint BP1,BP2;
boolean onBound1,onBound2;

for(int i=0;i<fSeg.getSize();i++){
     currentSeg = (fSegment) fSeg.get(i);
     currentBS  = (BasicSegment) currentSeg.basic();
     BP1 = currentBS.getEP1();
     BP2 = currentBS.getEP2();
     SegNumber1 = numberOfSegments(BP1);
     SegNumber2 = numberOfSegments(BP2);
     TriNumberSeg = R.numberOfTriangles(currentBS);
     onBound1 = R.onBoundary(BP1);
     onBound2 = R.onBoundary(BP2);

     if(TriNumberSeg==0) { //segment is in exterior of R
        result.setValue(true,M9Int.INTERIOR,M9Int.EXTERIOR);
        if(onBound1){
           if(SegNumber1==1)
             result.setValue(true,M9Int.BOUNDARY,M9Int.BOUNDARY);
           else  // a interior point of the Line
               result.setValue(true,M9Int.INTERIOR,M9Int.BOUNDARY);
        } // onBound1

        if(onBound2)  {
           if(SegNumber2==1)
              result.setValue(true,M9Int.BOUNDARY,M9Int.BOUNDARY);
           else
              result.setValue(true,M9Int.INTERIOR,M9Int.BOUNDARY);
        } // onBound2
     } // TriNumberSeg==0

     if(TriNumberSeg==1) {  // Segment on boundary of R
       result.setValue(true,M9Int.INTERIOR,M9Int.BOUNDARY);
       if(SegNumber1==1 | SegNumber2==1)
          result.setValue(true,M9Int.BOUNDARY,M9Int.BOUNDARY);
     }

     if(TriNumberSeg==2){ // Segment in interior of R
        result.setValue(true,M9Int.INTERIOR,M9Int.INTERIOR);
        if(onBound1){
           if(SegNumber1==1)
             result.setValue(true,M9Int.BOUNDARY,M9Int.BOUNDARY);
           else  // a interior point of the Line
             result.setValue(true,M9Int.INTERIOR,M9Int.BOUNDARY);
        } // onBound1

        if(onBound2)  {
           if(SegNumber2==1)
              result.setValue(true,M9Int.BOUNDARY,M9Int.BOUNDARY);
           else
              result.setValue(true,M9Int.INTERIOR,M9Int.BOUNDARY);
        } // onBound2
     } // TriNumberSeg==2
}

// a boundary-Segment of R in exterior of L

FLine  RegionBoundary = R.boundary();
boolean found=false;
for(int i=0;i<RegionBoundary.fSeg.getSize() & !found;i++){
  currentBS = (BasicSegment) RegionBoundary.fSeg.get(i).basic();
  if (fSeg.search(currentBS)==null){
     result.setValue(true,M9Int.INTERIOR,M9Int.EXTERIOR);
     found = true;
  }
}

return result;
}


/** compute the 9-intersection-matrix from this and CO */
public M9Int basicTopolRelation(CompositeObject CO){

if(CO instanceof FPoint){
    M9Int result = (( (FPoint)CO).basicTopolRelation(this));
    result.makeSym();
    return result;
  }
if(CO instanceof FLine)
    return basicTopolRelation( (FLine) CO);
if(CO instanceof FRegion)
    return basicTopolRelation( (FRegion) CO);
return null;

}



/********************************************************************
 *
 *                 Topology of fuzzy objects
 *
 ********************************************************************/

/**
  * returns the membership-value on given pos on a given
  * BasicSegment <br>
  * pos must be in [0,1]
  */
double getMaxValue(BasicSegment BS, double pos){
if(pos<0 || pos>1)
  return 0;
fSegment FS = (fSegment) fSeg.search(BS);
if(FS==null)
  return 0;

double Z1,Z2;
Z1 = FS.getZ1();
Z2 = FS.getZ2();
return Z1+pos*(Z2-Z1);
}



/** compute the top Rel between this and L */
public FuzzyTopRel topolRelation(FLine L){
int currentThis=0;
int currentL=0;
int maxThis=fSeg.getSize();
int maxL = fSeg.getSize();
fSegment fST,fSL;
BasicSegment BST,BSL;
int compare;
BasicPoint BP1,BP2;
double Z1T,Z2T,Z1L,Z2L;
FuzzyTopRel result = new FuzzyTopRel();

while(currentThis<maxThis & currentL<maxL){
    fST = (fSegment) fSeg.get(currentThis);
    BST = (BasicSegment) fST.basic();
    fSL = (fSegment) fSeg.get(currentL);
    BSL = (BasicSegment) fSL.basic();
    compare = BST.compareTo(BSL);
    if(compare<0){ // test on boundary of L
       currentThis++;
       BP1 = BST.getEP1();
       BP2 = BST.getEP2();
       if(L.contains(BP1)){
         Z1T = fST.getZ1();
         Z1L = L.maxZfkt(BP1);
         result.computeValue(Z1T,Z1L);     
       }
       if(L.contains(BP2)){
          Z1T = fST.getZ2();
          Z1L = L.maxZfkt(BP2);
          result.computeValue(Z1T,Z1L);
       }
    }
    if(compare>0){ // test on boundary of this
       currentL++;
       BP1 = BSL.getEP1();
       BP2 = BSL.getEP2();
       if(this.contains(BP1)){
         Z1L = fSL.getZ1();
         Z1T = this.maxZfkt(BP1);
         result.computeValue(Z1T,Z1L);     
       }
       if(this.contains(BP2)){
          Z1L = fSL.getZ2();
          Z1T = this.maxZfkt(BP2);
          result.computeValue(Z1T,Z1L);
       }
     }
     if(compare==0){ // common segment
        currentL++;
        currentThis++;
        Z1T = fST.getZ1();
        Z2T = fST.getZ2();
        Z1L = fSL.getZ1();
        Z2L = fSL.getZ2();


        if( Z1T==Z1L | Z2T==Z2L){
            result.computeValue(Z1T,Z1L);
            result.computeValue(Z2T,Z2L);
          }
        else{
          if( (Z1T>Z1L & Z2T>Z2L) | (Z1T<Z1L & Z2T<Z2L) )
             result.computeValue(Z1T,Z1L);
          else{  // the functions are crossing
             result.computeValue(Z1T,Z1L);
             result.computeValue(Z2T,Z2L);
             result.computeValue(0.5,0.5); // the intersection
             if(Z1T==0 | Z2T==0)
               result.computeValue(0.1,0.2);
             if(Z1L==0 | Z2L==0)
               result.computeValue(0.2,0.1);
          } // else
        }  // else
     }  // compare==0
} // while

while(currentThis<maxThis){
  fST = (fSegment) fSeg.get(currentThis);
  BST = (BasicSegment) fST.basic();
  BP1 = BST.getEP1();
  BP2 = BST.getEP2();
  currentThis++;
  if(L.contains(BP1)) {
     Z1T = fST.getZ1();
     Z1L = L.maxZfkt(BP1);
     result.computeValue(Z1T,Z1L);     
  }
  if(L.contains(BP2)){
     Z1T = fST.getZ2();
     Z1L = L.maxZfkt(BP2);
     result.computeValue(Z1T,Z1L);
  }
}

while(currentL<maxL){
  fSL = (fSegment) fSeg.get(currentL);
  BSL = (BasicSegment) fSL.basic();
  BP1 = BSL.getEP1();
  BP2 = BSL.getEP2();
  currentL++;
  if(this.contains(BP1)){
     Z1L = fSL.getZ1();
     Z1T = this.maxZfkt(BP1);
     result.computeValue(Z1T,Z1L);     
  }
  if(this.contains(BP2)){
     Z1L = fSL.getZ2();
     Z1T = this.maxZfkt(BP2);
     result.computeValue(Z1T,Z1L);
  }
} // while
 return result;
} // topolRelation



/** computes the top Rel between this Line and R */
public FuzzyTopRel topolRelation(FRegion R){

FuzzyTopRel result = new FuzzyTopRel();
int max = fSeg.getSize();
fSegment FS;
BasicSegment BS;
BasicPoint BP1,BP2;
double Z1L,Z2L,Z1R,Z2R;
for(int current=0;current<max;current++){
 FS = (fSegment) fSeg.get(current);
 BS = (BasicSegment) FS.basic();
 BP1 = BS.getEP1();
 BP2 = BS.getEP2();
 Z1L = FS.getZ1();
 Z2L = FS.getZ2();
 if(R.contains(BS)){
   double values[] = R.getValues(BS);
   if(values.length==2){
     Z1R = values[0];
     Z2R = values[1];

     if(Z1L!=Z1R)                       // only inner values
        result.computeValue(Z1L,Z1R);

     if(Z2L!=Z2R)                       // only inner values
        result.computeValue(Z2L,Z2R);

     if( (Z1L-Z1R)*(Z2L-Z2R)<0 ) // crossing => equal value
        result.computeValue(0.5,0.5);

     if( (Z1L==Z1R) & (Z2L==Z2R) ) // equals segments
        result.computeValue(0.5,0.5);
   }
   else{ // in the regions exists crossing triangles over this BS
     Z1R = values[0];
     Z2R = values[1];
     double ZMR = values[3];
     double ZML = Z1L + values[2]*(Z2L-Z1L);
     if(Z1L!=Z1R)                      // only inner values
        result.computeValue(Z1L,Z1R);
     result.computeValue(ZML,ZMR);
     if(Z2L!=Z2R)
        result.computeValue(Z2L,Z2R);
     if( ((Z1L-Z1R)*(ZML-ZMR)<0) || ((ZML-ZMR)*(Z2L-Z2R)<0) )
        result.computeValue(0.5,0.5);
   }
 } // R contains Segment
 if(R.contains(BP1)){
    Z1R = R.maxZfkt(BP1);
    result.computeValue(Z1L,Z1R);
 }
 if(R.contains(BP2)){
    Z2R = R.maxZfkt(BP2);
    result.computeValue(Z2L,Z2R);
 }
} // for

return result;
}



/** computes the top.rel. between this and CO */
public FuzzyTopRel topolRelation(CompositeObject CO){
 FuzzyTopRel result;
 if(CO instanceof FPoint){
     result = ((FPoint) CO).topolRelation(this);
     result.makeSym();
     return result;
 }
 if(CO instanceof FLine)
     return topolRelation((FLine) CO);
 if(CO instanceof FRegion)
     return topolRelation((FRegion) CO);
 return null;
}


/** returns the ListExpr representation of this FLine
  * (SF , (<SegmentList>))
  */
public ListExpr toListExpr(){
  // first create the SegmentList
  ListExpr Segments;
  ListExpr Last=null;
  if(fSeg.getSize()==0)
     Segments = ListExpr.theEmptyList();
  else {
     Segments = ListExpr.oneElemList(((fSegment)fSeg.get(0)).toListExpr()); 
     Last = Segments;
  }
  fSegment NextSegment; 
  for(int i=1;i<fSeg.getSize();i++){
     NextSegment = (fSegment) fSeg.get(i);
     Last=ListExpr.append(Last,NextSegment.toListExpr());
  }
  return ListExpr.twoElemList( ListExpr.realAtom((float)SF),Segments);
}

/** creates a new ListEXpr <type,value>*/
public ListExpr toTypedListExpr(){
  return ListExpr.twoElemList( ListExpr.symbolAtom("fline"),toListExpr());
}

/** read this FLine from a ListExpr
  * @return true if LE is a valid Representaion of a FLine
  * all valid Segments of this List are inserted
  */
public boolean readFromListExpr(ListExpr LE){
  SF = 1.0;
  fSeg.makeEmpty();
  computeBoundingBox();
  if(LE==null)
     return false;
  if(LE.listLength()!=2)
     return false;
  ListExpr SFList = LE.first();
  if( !( SFList.isAtom() && (SFList.atomType()==ListExpr.INT_ATOM || SFList.atomType()==ListExpr.REAL_ATOM)))
     return false;
  double z= SFList.atomType()==ListExpr.INT_ATOM ? SFList.intValue() : SFList.realValue();
  if(z<=0)
     return false;
  this.SF = z;

  ListExpr Segments = LE.second();
  fSegment S;
  boolean ok = true; 
  while( !Segments.isEmpty() & ok) {
    S = new fSegment(0,0,0,0,0,0);
    if(S.readFromListExpr(Segments.first())){
       add(S);
       Segments=Segments.rest();
    }
    else
       ok = false;

  }
  return ok;

}

/** returns a String representation of the corresponding ListExpr*/
public String toListString(){
  return toListExpr().writeListExprToString();
}

/** read the FLine from a String representation of a ListExpr 
  * @return true if List is a String of a ListExpr containing a correct FLine
  */
public boolean readFromListString(String List){
  ListExpr LE = new ListExpr();
  if(LE.readFromString(List)!=0){
     return false;
  }
  else{
     return readFromListExpr(LE); 
 }
}




// define constants for the operators
private static final int UNION = 0;             // union based on max
private static final int INTERSECTION=1;   // difference based on min
private static final int ADD=2;                 // addition with cut if >1
private static final int SUBTRACT=3;            // substraction with cut if<0

private static final int SCALEDUNION=4;
private static final int SCALEDINTERSECTION=5;
private static final int SCALEDADD=6;
private static final int SCALEDDIFFERENCE=7;


private static final int SHARPINTERSECTION=8;  // == intersection(sharp(L1),sharp(L2))

} // FLine;


