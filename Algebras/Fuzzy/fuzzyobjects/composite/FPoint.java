package fuzzyobjects.composite;

import fuzzyobjects.basic.*;
import fuzzyobjects.simple.*;
import java.util.Vector;
import sj.lang.ListExpr;
import java.io.*;

/**
 * this class provides a implementation of
 * fuzzy spatial points
 * @author Thomas Behr
 */

public class FPoint implements CompositeObject{

/** the factor of scale */
protected double SF;
/**
  * the set of fuzzy elementary objects
  * this set must be ordered
  */
protected SortedObjects  feP;     // the fuzzy elementary points

/** the Bounding box of this FPoint */
protected BoundingBox BB = new BoundingBox();

/** set a new factor of scale
  * @param SF the new factor, must be greater then 0
  * @result true if sucessfull
  */
public boolean setSF(double SF){
 if (SF>0){
   this.SF = SF;
   return true;
 }
 else return false;
}


/* returns the number of containing fuzzy elementary points */
public int getSize(){
   return feP.getSize();
}

public fEPoint getPointAt(int index){
   if(index<0 || index >=feP.getSize())
       return null;
   else
       return (fEPoint) feP.get(index);
}

/**
 * creates a new FPoint
 * whith given factor of scale
 */
public FPoint(double scale){
   feP = new SortedObjects();
   SF  = scale;
   no++;
}

/**
 * creates a new FPoint
 * with a factor of scale of 1.0
 */
public FPoint() {
    this(1.0);
}

/**
 * returns the current factor of scale
 */
public double getSF(){ return SF;}

/** returns the bounding box of this point */
public BoundingBox getBoundingBox(){ return BB; }

/**
 * returns the dimension of a point,
 * i.e. 0
 */
public int getDim() { return 0; }

/**
 * check whether this is a valid point
 * i.e. here is ckecked whether all containing fuzzy elementary points
 * are valid and the factor of scale is greater then 0
 */
public boolean isValid(){
 return SF>0 & feP.allValid();
}

/**
 * returns a readable representation of this fPoint
 */
public String toString(){
  return  "FPoint  (SF="+SF+")\n"+feP.toString();
}


/** prints out a representation of this FPoint to 
  * the standard-oiutput
  * it's for debuggng only 
  */
public void print(){
   System.out.println(this);
}


/** computes the bounding boxc of this point */
private void computeBoundingBox(){
if(feP.isEmpty())
  BB.setBox(0,0,0,0);
else{
  fEPoint FP = (fEPoint) feP.get(0);
  int minX = FP.getMinX();
  int minY = FP.getMinY();
  int maxX = FP.getMaxX();
  int maxY = FP.getMaxY();
  int cX,cY;
  for (int i=1;i<feP.getSize();i++){
    FP = (fEPoint) feP.get(i);
    cX = FP.getMinX();
    cY = FP.getMinY();
    if(cX < minX)
      minX = cX;
    if(cY < minY)
      minY = cY;
    cX = FP.getMaxX();
    cY = FP.getMaxY();
    if(cX > maxX)
      maxX = cX;
    if(cY > maxY)
      maxY = cY;
  }
  BB.setBox(minX,minY,maxX,maxY);
}
}

/**
 * inserted a new fuzzy elementary point to this FPoint
 * @param SP the fEPoint to inserted
 * @return <ul>
 *            <li> true if SP is not containing in original FPoint
 *            <li> false if SP is in FPoint, SP is not inserted
 *         </ul>
 */
public boolean add(fEPoint SP){

boolean first = feP.isEmpty();
boolean ok = feP.insert(SP.copy());
if(ok) { // update bounding box
 if(!first){
    int minX = BB.getMinX();
    int minY = BB.getMinY();
    int maxX = BB.getMaxX();
    int maxY = BB.getMaxY();
    if(minX>SP.getMinX())
      minX = SP.getMinX();
    if(minY>SP.getMinY())
      minY = SP.getMinY();
    if(maxX<SP.getMaxX())
      maxX = SP.getMaxX();
    if(maxY<SP.getMaxY())
      maxY = SP.getMaxY();
    BB.setBox(minX,minY,maxX,maxY);
 }
 else
   BB.setBox(SP.getMinX(),SP.getMinY(),SP.getMaxX(),SP.getMaxY());
}
return ok;

}

/**
 * updated a fEPoint containing in this FPoint
 * @param SP the fEPoint to updated
 * @return <ul>
 *           <li> true if SP is containing in this
 *           <li> false SP is not a element of this, nothing to update
 *         </ul>
 */
public boolean update(fEPoint SP){
   if(feP.update(SP.copy())){
     int minX = BB.getMinX();
     int minY = BB.getMinY();
     int maxX = BB.getMaxX();
     int maxY = BB.getMaxY();
     if(minX>SP.getMinX())
        minX = SP.getMinX();
     if(minY>SP.getMinY())
        minY = SP.getMinY();
     if(maxX<SP.getMaxX())
        maxX = SP.getMaxX();
     if(maxY<SP.getMaxY())
        maxY = SP.getMaxY();
     BB.setBox(minX,minY,maxX,maxY);
     return true;
   }
   return false;
}

/**
 * updated a containing fEPoint
 * if SP is not containing in this, SP is added to this
 * @param SP the fEPoint to updated/added
 */
public void overwrite(fEPoint SP){
 boolean first = feP.isEmpty();
 if (!feP.update(SP))
    feP.insert(SP);
 if(!first){
   int minX = BB.getMinX();
   int minY = BB.getMinY();
   int maxX = BB.getMaxX();
   int maxY = BB.getMaxY();
   if(minX>SP.getMinX())
      minX = SP.getMinX();
   if(minY>SP.getMinY())
      minY = SP.getMinY();
   if(maxX<SP.getMaxX())
      maxX = SP.getMaxX();
   if(maxY<SP.getMaxY())
      maxY = SP.getMaxY();
   BB.setBox(minX,minY,maxX,maxY);
 }
 else
   BB.setBox(SP.getMinX(),SP.getMinY(),SP.getMaxX(),SP.getMaxY());
}

/**
 * creates a new FPoint and invokes <a href="#overwrite(fEPoint)">
 * overwrite(fEPoint) </A>
 * @return true if (x,y,Z) is a valid fEPoint
 */
public boolean overwrite(int x, int y , double Z){
   fEPoint P = new fEPoint(x,y,Z);
   boolean result = P.isValid();
   if (result)
      overwrite(P);
   return result;
}

/**
 * deletes a fEPoint with Basic BP from this
 * @return true if this contains BP
 */
public boolean delete(BasicPoint BP){
   boolean result = feP.delete(BP);
   if(result & (
                 BP.getMinX()==BB.getMinX() |
                 BP.getMaxX()==BB.getMaxX() |
                 BP.getMinY()==BB.getMinY() |
                 BP.getMaxY()==BB.getMaxY() )  )
        computeBoundingBox();
   return result;
}

/**
 * computes all membershipvalues of this on (x,y)
 */
public double[] ZRel(double x, double y) {
 double[] Rel = new double[1];
 Rel[0] = 0.0;
 if ( (int)x == x  && (int)y==y){  // x and y are integers
     fEPoint P = (fEPoint) feP.search( new BasicPoint((int)x,(int)y) );
     if (P!=null)
        Rel[0] = P.getZ();
 }
 return Rel;
}  // ZRel


/**
 * computes the minimal membershipvalue on (x,y)
 */
public double minZfkt(double x, double y){
  double[] Rel = ZRel(x,y);
  double min = Rel[0];
  for(int i=1;i<Rel.length;i++)
    if(min>Rel[i])
      min =Rel[i];
  return min;
}

/**
 * computes the maximal membership-value on (x,y)
 */
public double maxZfkt(double x, double y){
  double[] Rel = ZRel(x,y);
  double max = Rel[0];
  for(int i=1;i<Rel.length;i++)
    if(max<Rel[i])
      max =Rel[i];
  return max;
}

/**
 * computes the middle membership-value on (x,y)
 */
public double midZfkt(double x, double y){
 double[] Rel = ZRel(x,y);
 double sum =0;
 for(int i=0;i<Rel.length;i++)
    sum += Rel[i];
 return sum / Rel.length;
}

/**
 * computes the minimal membership-value on BP
 */
public double minZfkt(BasicPoint BP){
  double[] Rel = ZRel(BP);
  double min = Rel[0];
  for(int i=1;i<Rel.length;i++)
    if(min>Rel[i])
      min =Rel[i];
  return min;
}

/** computes the maximal membership-value on BP */
public double maxZfkt(BasicPoint BP){
  double[] Rel = ZRel(BP);
  double max = Rel[0];
  for(int i=1;i<Rel.length;i++)
    if(max<Rel[i])
      max =Rel[i];
  return max;
}

/** computes the middle membership-value on BP */
public double midZfkt(BasicPoint BP){
 double[] Rel = ZRel(BP);
 double sum =0;
 for(int i=0;i<Rel.length;i++)
   sum += Rel[i];
 return sum/Rel.length;
}

/** computes all membership-values on BP */
public double[] ZRel(BasicPoint BP){
 double[] result = new double[1];
 result[0]=0;
 fEPoint P = (fEPoint) feP.search(BP);
 if(P!=null)
    result[0] = P.getZ();
 return result;
}


/**
 * computes the memebrship-value on (x,y) with basic BP
 */
public double Zfkt(double x, double y, BasicPoint BP){
double result=0.0;
if ( (int)x == x  && (int) y ==y) 
  if (BP.getX()==x && BP.getY()==y) {
    fEPoint P = (fEPoint) feP.search(BP);
    if (P!=null)
       result = P.getZ();
  }
 return result;
}

/**
 * computes the memebrship-value on BP1 with basic BP2
 */
public double Zfkt( BasicPoint BP1, BasicPoint BP2){
 double result=0.0;
 if( BP1.equals(BP2) ) {
   Object O = feP.search(BP1);
   if (O!=null){
       fEPoint P = (fEPoint) O;
       result = P.getZ();
   }
 }
 return result;
}

/**
 * computes all BasicPoints containing in this
 */
public BasicPoint[] basics() {
  BasicPoint[] result = new BasicPoint[feP.getSize()];
  if(feP.isEmpty())
     return result;
  else{
    for(int i=0;i<feP.getSize();i++)
      result[i] = (BasicPoint) ((fEPoint)feP.get(i)).basic();
   }
     return result;
} // basics


/**
 * computes the maximal membership-value containing in this
 */
public double maxZ(){
 double max = 0.0;
 for(int i=0;i<feP.getSize();i++)
   if (max < ((fEPoint)feP.get(i)).getZ())
     max = ((fEPoint)feP.get(i)).getZ();
  return max;
} // maxZ

/**
 * computes the minimal membership-value containing in this
 */
public double minZ(){
  if (feP.isEmpty())
    return 0;
  else {
    double min =1;
    for(int i=0;i<feP.getSize();i++)
      if (min> ((fEPoint)feP.get(i)).getZ())
        min = ((fEPoint)feP.get(i)).getZ();
    return min;
  }
} // minZ

/**
 * returns a copy of this
 */
public FPoint copy(){
  FPoint C = new FPoint(SF);
  C.feP = feP.copy();
  C.BB = BB.copy();
  return C;
}

/**
 * check whether this is containing elements
 */
public boolean isEmpty(){
 return feP.isEmpty();
}


/**
 * set the set of fEPoints from P1 equal to the set of P2
 */
protected static void setfeP(FPoint P1, FPoint P2){
   P1.feP = P2.feP;
   P1.BB  = P2.BB;
}


/*************************************************************
   Operators
 *************************************************************/

/** computes the union from this with With */
public FPoint union(FPoint With){
  return  operator(With,UNION);
}

/** computes the scaled_union from this with With */
public FPoint scaledUnion(FPoint With){
  FPoint result =  operator(With,SCALEDUNION);
  result.norm();
  return result;
}

/** computes the intersection from this with With */
public FPoint intersection(FPoint With){
   return operator(With,INTERSECTION);
}

/** computes the scaled_intersection from this with With */
public FPoint scaledIntersection(FPoint With){
  FPoint result = operator(With,SCALEDINTERSECTION);
  result.norm();
  return result;
}

/** computes the operator add from this with With */
public FPoint add(FPoint With){
   return operator(With,ADD);
}

/** computes the operator scaled_add from this with With */
public FPoint scaledAdd(FPoint With){
  FPoint result = operator(With,SCALEDADD);
  result.norm();
  return result;
}

/** computes the difference from this with With */
public FPoint difference(FPoint With){
   return operator(With,SUBTRACT);
}

/** computes the scaled_difference from this with With */
public FPoint scaledDifference(FPoint With){
  FPoint result = operator(With,SCALEDDIFFERENCE);
  result.norm();
  return result;
}

/** computes the alpha-cut of this */
public FPoint alphaCut(double alpha, boolean strong){
 FPoint result = new FPoint(SF);
 fEPoint Current;
 boolean  isValid;
 double Z;
 for(int i=0; i<feP.getSize(); i++){
    Current = (fEPoint) feP.get(i);
    Z = Current.getZ();
    if (strong)
       isValid = Z>alpha;
    else
       isValid = Z>= alpha;

    if(isValid)
       result.feP.insert(Current.copy());
 }
 result.computeBoundingBox();
 return result;
}


/** computes the number of elements in this */
public double basicCard(){
   return feP.getSize();
}

/** computes the weighted number of elements in this */
public double card(){
  double result = 0;

  for(int i=0;i<feP.getSize();i++)
      result += ((fEPoint) feP.get(i)).getZ();
  return result;
}

/**
 * computes the similarity in the basic from this whith P2
 */
public double basicSimilar(FPoint P2){
     FPoint Funion         = this.union(P2);
     FPoint Fintersection  = this.intersection(P2);
     if (Funion.isEmpty())
        return 1.0;
     else
        return Fintersection.basicCard() / Funion.basicCard();
}

/**
 * computes the similarity of this with P2
 */
public double similar(FPoint P2){
     FPoint Funion         = this.union(P2);
     FPoint Fintersection  = this.intersection(P2);
     if (Funion.isEmpty())
        return 1.0;
     else
        return Fintersection.card() / Funion.card();
}

/**
 * make this sharp
 * i.e. set all membershipvalues on 1.0
 */
public FPoint sharp(){
 FPoint result = new FPoint(1);
 fEPoint Current;

 for(int i=0;i<feP.getSize();i++){
   Current = new fEPoint((BasicPoint) (feP.get(i)).basic(),1);
   result.add(Current);
 }
  return result;
}


/**
 * computes the operator mid
 */
public static FPoint mid(FPoint[] Pts){

if(Pts.length==0) return null;

FPoint result    = new FPoint(1);
int[] current    = new int[Pts.length];
int[] max        = new int[Pts.length];
boolean[] ready  = new boolean[Pts.length];
boolean allready  = true;

// initialize the variables
for(int i=0;i<Pts.length;i++){
   current[i]=0;
   max[i]= Pts[i].feP.getSize();
   ready[i] = current[i]<max[i];
   if(!ready[i]) allready=false;
}

Vector allSmallest = new Vector(); // the smallest coordinate
Vector Numbers     = new Vector(); // the positions in Pts

while(!allready){
  allready=true;
  allSmallest = new Vector();
  Numbers = new Vector();
  // search all smallest Triangles
  fEPoint currentP;
  BasicPoint compareP;
  for(int i=0;i<Pts.length;i++){
     if(current[i]<max[i]) {
       allready=false;
       currentP = (fEPoint) Pts[i].feP.get(current[i]);

       if(allSmallest.size()==0) {
          allSmallest.add(currentP);
          Numbers.add(new Integer(i));
       }
       else {
        compareP = (BasicPoint) ((fEPoint) allSmallest.get(0)).basic();
        int comp =  currentP.basic().compareTo(compareP);
        if(comp==0) {  // a Triangle with smallest Basic
          allSmallest.add(currentP);
          Numbers.add(new Integer(i));
        }
        if(comp<0) {  // new smallest Triangle
         allSmallest = new Vector();
         Numbers = new Vector();
         allSmallest.add(currentP);
         Numbers.add(new Integer(i));
        }
        // in the case comp>0 is nothing to do
      } // not the first coordinate
    } // current Point contains unprocessing coordinates
  } // for all Pts

  // all smallest Points are in Vector allSmallest and
  // the positions of this one are in Numbers

  if(!allready){
    double Z=0;
    BasicPoint BP = (BasicPoint) ((fEPoint)allSmallest.get(0)).basic();
    fEPoint CP;
    for(int i=0;i<allSmallest.size();i++){
       // update numbers
       int c = ((Integer)Numbers.get(i)).intValue();
       current[c]++;
       CP = (fEPoint) allSmallest.get(i);
       Z += CP.getZ();
    }
    Z = Z / Pts.length;
    result.feP.insert( new fEPoint(BP,Z));
  }

}  // while not all ready
 result.computeBoundingBox();
 return result;
}

/**
 * computes the operator scaled_mid
 */
public static FPoint scaledMid(FPoint[] Pts){
if(Pts.length==0) return null;

FPoint result    = new FPoint(1);
int[] current    = new int[Pts.length];
int[] max        = new int[Pts.length];
boolean[] ready  = new boolean[Pts.length];
boolean allready  = true;

// initialize the variables
for(int i=0;i<Pts.length;i++){
   current[i]=0;
   max[i]= Pts[i].feP.getSize();
   ready[i] = current[i]<max[i];
   if(!ready[i]) allready=false;
}

Vector allSmallest = new Vector(); // the smallest coordinate
Vector Numbers     = new Vector(); // the positions in Pts

while(!allready){
  allready=true;
  allSmallest = new Vector();
  Numbers = new Vector();
  // search all smallest Triangles
  fEPoint currentP;
  BasicPoint compareP;
  for(int i=0;i<Pts.length;i++){
     if(current[i]<max[i]) {
       allready=false;
       currentP = (fEPoint) Pts[i].feP.get(current[i]);

       if(allSmallest.size()==0) {
          allSmallest.add(currentP);
          Numbers.add(new Integer(i));
       }
       else {
        compareP = (BasicPoint) ((fEPoint) allSmallest.get(0)).basic();
        int comp =  currentP.basic().compareTo(compareP);
        if(comp==0) {  // a Triangle with smallest Basic
          allSmallest.add(currentP);
          Numbers.add(new Integer(i));
        }
        if(comp<0) {  // new smallest Triangle
         allSmallest = new Vector();
         Numbers = new Vector();
         allSmallest.add(currentP);
         Numbers.add(new Integer(i));
        }
        // in the case comp>0 is nothing to do
      } // not the first coordinate
    } // current Point contains unprocessing coordinates
  } // for all Pts

  // all smallest Points are in Vector allSmallest and
  // the positions of this one are in Numbers

  if(!allready){
    double Z=0;
    BasicPoint BP = (BasicPoint) ((fEPoint)allSmallest.get(0)).basic();
    fEPoint CP;
    for(int i=0;i<allSmallest.size();i++){
       // update numbers
       int c = ((Integer)Numbers.get(i)).intValue();
       current[c]++;
       CP = (fEPoint) allSmallest.get(i);
       Z += CP.getZ()*Pts[c].SF;
    }
    Z = Z / Pts.length;
    result.feP.insert( new fEPoint(BP,Z));
  }

}  // while not all ready
 result.norm();
 result.computeBoundingBox();
 return result;
}

/**
 * process elements of two FEPoints
 * F1 and F2 must have the same basic
 * or can be null
 * @param F? the element of FPoint P?
 * @param scale? the factor of scale from FPoint P?
 * @param goal here is saving the result of this method
 * @param Operator the applying operator
 */
private void processElements(fEPoint F1, double scale1,
                             fEPoint F2, double scale2,
                             FPoint Goal,
                             int Operator){

// 1 input parameter can be null
// if both fTriangles not null, then they must have the same basic

  if( F1==null & F2==null) return;


  double Z;
  fEPoint newFEP;

  switch (Operator){

     case UNION  :  {  // the union of 2 Points ignoring SFs

                      if(F1==null)
                          Goal.feP.insert(F2.copy());
                      else
                         if(F2==null)
                           Goal.feP.insert(F1.copy());
                         else { // both fEPoints are not null
                           Z = Math.max(F1.getZ(),F2.getZ());
                           newFEP = new fEPoint((BasicPoint)F1.basic(),Z);
                           Goal.feP.insert(newFEP);
                         } // else
                    }
                    break;

    case INTERSECTION :
                    {  if (F1==null | F2==null)
                         ;
                       else { // both are not null
                          Z = Math.min(F1.getZ(),F2.getZ());
                          newFEP = new fEPoint((BasicPoint)F1.basic(),Z);
                          Goal.feP.insert(newFEP);
                       }
                    } break;

    case ADD     :  {
                      if(F1==null)
                          Goal.feP.insert(F2.copy());
                      else
                         if(F2==null)
                           Goal.feP.insert(F1.copy());
                         else { // both fEPoints are not null
                           Z = Math.min(1,F1.getZ()+F2.getZ());
                           newFEP = new fEPoint((BasicPoint)F1.basic(),Z);
                           Goal.feP.insert(newFEP);
                         } // else
                    }
                    break;

    case SUBTRACT :{
                        if(F1 == null)
                            ;
                        else
                           if(F2==null)
                             Goal.feP.insert(F1.copy());
                           else  {   // both not null
                              Z = Math.max(0,F1.getZ()-F2.getZ());
                              if (Z>0) {
                                newFEP=new fEPoint((BasicPoint)F1.basic(),Z);
                                Goal.feP.insert(newFEP);
                              }
                           }
                       } break;


      case SCALEDUNION    :
                      { fEPoint newfePoint;
                        if (F1==null)
                          newfePoint = new fEPoint((BasicPoint)F2.basic(),
                                                    F2.getZ()*scale2);
                         else
                           if (F2==null)
                              newfePoint=new fEPoint((BasicPoint)F1.basic(),
                                                      F1.getZ()*scale1);
                           else {
                             Z = Math.max(F1.getZ()*scale1,F2.getZ()*scale2);
                             newfePoint = new fEPoint((BasicPoint)F1.basic(),
                                                       Z);
                           }  // else
                        Goal.add(newfePoint);
                      } break;   // scaled union


      case SCALEDINTERSECTION :  if (F1==null || F2==null)
                                         ;
                                   else {
                                     Z = Math.min(F1.getZ()*scale1,
                                                  F2.getZ()*scale2);

                                    Goal.add(new fEPoint( (BasicPoint)
                                                            F1.basic(),Z));
                                   } break;

      case SCALEDADD : {
                          if(F1==null)
                             Goal.add(new fEPoint( (BasicPoint)F2.basic(),
                                                    F2.getZ()*scale2));
                          else
                            if(F2==null)
                              Goal.add(new fEPoint( (BasicPoint)F1.basic(),
                                                     F1.getZ()*scale1 ));
                            else {
                              Goal.add(new fEPoint((BasicPoint)F1.basic(),
                                                    F1.getZ()*scale1 +
                                                    F2.getZ()*scale2 ));
                            }
                        } break;

      case SCALEDDIFFERENCE   :
                       {
                          if(F1==null)
                             Goal.add(new fEPoint( (BasicPoint) F2.basic(),
                                                    -F2.getZ()*scale2 ));
                          else
                            if(F2==null)
                              Goal.add(new fEPoint( (BasicPoint) F1.basic(),
                                                     F1.getZ()*scale1 ));
                            else {
                              Goal.add(new fEPoint( (BasicPoint) F1.basic(),
                                                      F1.getZ()*scale1 -
                                                      F2.getZ()*scale2 ));
                            }
                        } break;

    default      : System.out.println("unimplementierter operator");


  } // switch

} // processElements

/**
 * the 'template' for many operators
 * @param FP the second operand
 * @param op the operator
 */
protected FPoint operator(FPoint FP, int op){
// a kind of mergesort

int my     = 0;
int fromFP = 0;   // already processed

int maxMy     = feP.getSize();   // numbers of elements
int maxFromFP = FP.feP.getSize();
FPoint result = new FPoint(1);

fEPoint myFirst=null;  // the first unprocessed elements
fEPoint FPFirst=null;

if(maxMy>0)              // needed if a empty point is involve
   myFirst= (fEPoint) feP.get(0);
if(maxFromFP>0)
   FPFirst = (fEPoint) FP.feP.get(0);

if (maxMy >0 && maxFromFP>0){
   myFirst = (fEPoint) feP.get(my);
   FPFirst = (fEPoint) FP.feP.get(fromFP);
   int compareResult;

   while(my<maxMy && fromFP<maxFromFP){ // both sets have unprocessed elements

      compareResult = myFirst.basic().compareTo(FPFirst.basic());
      if(compareResult < 0) {
         processElements(myFirst,SF,null,FP.SF,result,op);
         my++;
         if (my<maxMy)
             myFirst = (fEPoint) feP.get(my);
      }
      else if(compareResult > 0){
            processElements(null,SF,FPFirst,FP.SF,result,op);
            fromFP++;
            if(fromFP<maxFromFP)
               FPFirst = (fEPoint) FP.feP.get(fromFP);
           }
           else {     // elements have the same basic
             processElements(myFirst,SF,FPFirst,FP.SF,result,op);
             my++;
             fromFP++;
             if (my<maxMy)
               myFirst = (fEPoint) feP.get(my);
             if (fromFP<maxFromFP)
               FPFirst = (fEPoint) FP.feP.get(fromFP);
           }
   } // while
} // if

// elements from one (or both) regions are processed

while(my < maxMy){    // this have still elements
   processElements(myFirst,SF,null,FP.SF,result,op);
   my++;
   if (my<maxMy)
      myFirst = (fEPoint) feP.get(my);
}

while (fromFP < maxFromFP){  // FP have still elements
   processElements(null,SF,FPFirst,FP.SF,result,op);
   fromFP++;
   if(fromFP<maxFromFP)
      FPFirst = (fEPoint) FP.feP.get(fromFP);
}
  result.computeBoundingBox();
  return result;
}

/**
 * normalize this
 */
private void norm(){

 if (isEmpty()) return;  // nothing to do

 // first compute Zmin and Zmax
 double Zmin = 0;
 double Zmax = 0;
 fEPoint Current;

 for (int i=0; i< feP.getSize();i++){
   Current = (fEPoint) feP.get(i);
   if(Current.getZ()>Zmax)
      Zmax = Current.getZ();
   if(Current.getZ()<Zmin)
      Zmin = Current.getZ();
 }

 if(Zmin > 0) Zmin=0;

 if(Zmax==0 & Zmin==0)
    feP.makeEmpty();
 else{
    double SFnew = Zmax - Zmin;
    SortedObjects newfePs= new SortedObjects();
    double Z;
    fEPoint fePnew;

    for(int i=0;i<feP.getSize();i++){
       Current = (fEPoint) feP.get(i);
       Z = Current.getZ();
       fePnew = new fEPoint((BasicPoint) Current.basic(),(Z-Zmin)/SFnew);
       if (fePnew.getZ() >0)
          newfePs.insert(fePnew);
   } // for
   SF = SFnew*SF;
   feP = newfePs;
 }
 computeBoundingBox();
}


/****************************************************************
 *                                                              *
 *                  topological relationships                   *
 *                                                              *
 ****************************************************************/


/****************************************************************
*                                                               *
*           Topology in the basic                               *
*                                                               *
*****************************************************************/

/**
 * returns a String representing the topological relationship
 * between this and P2
 * if fuzzy is false the membershipvalues are not evaluated
 */
public String basicTopolRelationString(FPoint P2){
  M9Int matrix = basicTopolRelation(P2);
  String result;
  if (matrix ==null)
    result="undefined";
  else{
    result = matrix.toString();
  }
  return result;
}


// relations between 2 points

/**
  * compute the topological relations of the basic between
  * this and P2
  */
M9Int basicTopolRelation(FPoint P2){

 if(this.isEmpty() | P2.isEmpty()) return null;

 M9Int result = new M9Int();
 // the standard-entries for 2 points
 result.setValue(false,M9Int.BOUNDARY,M9Int.INTERIOR);
 result.setValue(false,M9Int.BOUNDARY,M9Int.BOUNDARY);
 result.setValue(false,M9Int.BOUNDARY,M9Int.EXTERIOR);
 result.setValue(false,M9Int.INTERIOR,M9Int.BOUNDARY);
 result.setValue(false,M9Int.EXTERIOR,M9Int.BOUNDARY);
 result.setValue(true,M9Int.EXTERIOR,M9Int.EXTERIOR);

 // compute the next intersections
 // merge sort like

 int currentThis=0;
 int currentP2=0;
 int maxThis = feP.getSize();
 int maxP2   = P2.feP.getSize();

 fEPoint FirstThis = (fEPoint)feP.get(0);  // the first unprocessed Elements
 fEPoint FirstP2   = (fEPoint)P2.feP.get(0);
 int compareResult;

 boolean ready = false; // all possible intersections are true ?

 while(currentThis<maxThis & currentP2<maxP2 & !ready){
   compareResult =  FirstThis.basic().compareTo(FirstP2.basic());

   if(compareResult<0){
     // a (basic)Point of this is in exterior of P2
     result.setValue(true,M9Int.INTERIOR,M9Int.EXTERIOR);
     currentThis++;
     if (currentThis<maxThis)
        FirstThis = (fEPoint) feP.get(currentThis);
   }

   if(compareResult>0){
     // a (basic)Point of P2 is in the exterior of this
     result.setValue(true,M9Int.EXTERIOR,M9Int.INTERIOR);
     currentP2++;
     if(currentP2<maxP2)
        FirstP2 = (fEPoint) P2.feP.get(currentP2);
   }

   if(compareResult==0){
    // this and P2 have a common coordinate
    result.setValue(true,M9Int.INTERIOR,M9Int.INTERIOR);
    currentThis++;
    currentP2++;
    if(currentThis<maxThis)
       FirstThis=(fEPoint) feP.get(currentThis);
    if(currentP2<maxP2)
       FirstP2=(fEPoint) P2.feP.get(currentP2);
   } // if common basic

   ready = result.getValue(M9Int.INTERIOR,M9Int.INTERIOR) &
           result.getValue(M9Int.INTERIOR,M9Int.EXTERIOR) &
           result.getValue(M9Int.EXTERIOR,M9Int.INTERIOR);
 } // while (both Points have components and not ready)

 if( !ready & currentThis<maxThis){
  // this have coordinates (not in P2)
  result.setValue(true,M9Int.INTERIOR,M9Int.EXTERIOR);
 }
 if(!ready & currentP2<maxP2){
   // P2 have coordinates in exterior of this
   result.setValue(true,M9Int.EXTERIOR,M9Int.INTERIOR);
 }
 return result;
}


/**
  * computes the topological relationship
  * between this and Line in their basic
  */

M9Int basicTopolRelation(FLine Line){
 if(this.isEmpty() | Line.isEmpty()) return  null;

 M9Int result = new M9Int();

 // the "standard"-values between Point and line
 result.setValue(true,M9Int.EXTERIOR,M9Int.EXTERIOR);
 result.setValue(false,M9Int.BOUNDARY,M9Int.INTERIOR);
 result.setValue(false,M9Int.BOUNDARY,M9Int.BOUNDARY);
 result.setValue(false,M9Int.BOUNDARY,M9Int.EXTERIOR);
 result.setValue(true,M9Int.EXTERIOR,M9Int.INTERIOR);

 // compute the intersections of interior of this
 // with the parts of Line

 boolean ready=false;
 int number;
 fEPoint currentFP;
 BasicPoint currentBP;

 for(int i=0;i<feP.getSize() & !ready ;i++){
   currentFP = (fEPoint) feP.get(i);
   currentBP = (BasicPoint) currentFP.basic();
   number = Line.numberOfSegments(currentBP);
   if(number==0){
     result.setValue(true,M9Int.INTERIOR,M9Int.EXTERIOR);
   }

   if(number==1){
     result.setValue(true,M9Int.INTERIOR,M9Int.BOUNDARY);
   }

   if(number>1){
      result.setValue(true,M9Int.INTERIOR,M9Int.INTERIOR);
   }

   ready = result.getValue(M9Int.INTERIOR,M9Int.INTERIOR) &
           result.getValue(M9Int.INTERIOR,M9Int.BOUNDARY) &
           result.getValue(M9Int.INTERIOR,M9Int.EXTERIOR);
   // all 3 entrys are true, this is no change more possible
 }

 // intersection exterior of Point / boundary of Line
 FPoint LB = Line.boundary();
 ready =false;
 fEPoint inThis;

 for(int i=0;i<LB.feP.getSize()&!ready ;i++){
   currentBP = (BasicPoint) LB.feP.get(i).basic();
   inThis=(fEPoint)feP.search(currentBP);
   if(inThis==null){
        result.setValue(true,M9Int.EXTERIOR,M9Int.BOUNDARY);
        ready = true;
   }
 }  // for

 return result;
}


/**
  * computes the topological relationship between the basic
  * of this and the basic of a Region
  */
M9Int basicTopolRelation(FRegion Region){

 if ( isEmpty() | Region.isEmpty()) return null;

 M9Int result = new M9Int();

 // this holds ever between Point and Region

 result.setValue(false,M9Int.BOUNDARY,M9Int.INTERIOR);
 result.setValue(false,M9Int.BOUNDARY,M9Int.BOUNDARY);
 result.setValue(false,M9Int.BOUNDARY,M9Int.EXTERIOR);
 result.setValue(true,M9Int.EXTERIOR,M9Int.INTERIOR);
 result.setValue(true,M9Int.EXTERIOR,M9Int.BOUNDARY);
 result.setValue(true,M9Int.EXTERIOR,M9Int.EXTERIOR);

 fEPoint    currentFP;
 BasicPoint currentBP;
 double Z1,Z2;
 int number;
 boolean ready=false;

 for(int i=0;i<feP.getSize()&!ready;i++){
    currentFP = (fEPoint) feP.get(i);
    currentBP = (BasicPoint) currentFP.basic();
    number = Region.numberOfTriangles(currentBP);
    if(number==0){  // a coordinate in exterior of region
      result.setValue(true,M9Int.INTERIOR,M9Int.EXTERIOR);
    }
    else{ // intersects a true part of region
      if(Region.onBoundary(currentBP))   // intersects boundary
         result.setValue(true,M9Int.INTERIOR,M9Int.BOUNDARY);
      else  // in interior of basic of region
            result.setValue(true,M9Int.INTERIOR,M9Int.INTERIOR);
    } // intersects a true part
   ready = result.getValue(M9Int.INTERIOR,M9Int.INTERIOR) &
           result.getValue(M9Int.INTERIOR,M9Int.BOUNDARY) &
           result.getValue(M9Int.INTERIOR,M9Int.EXTERIOR);
   // all possible intersections are true => no more changes
 } // for 

return result;
}



/**
  * returns the topological relationship between this and a
  *composite Object(FPoint,FLine or FRegion)
  **/
public M9Int basicTopolRelation(CompositeObject CO){
if(CO instanceof FPoint)
   return basicTopolRelation( (FPoint) CO);
if(CO instanceof FLine)
   return basicTopolRelation( (FLine) CO);
if( CO instanceof FRegion)
   return basicTopolRelation((FRegion) CO);
return null;
}



/*******************************************************************
 *                                                                 *
 *               Topology on fuzzy objects                         *
 *                                                                 *
 *******************************************************************/

/** computes the fuzzy top. Relation between this and P2 */
public FuzzyTopRel topolRelation(FPoint P2){
  FuzzyTopRel result = new FuzzyTopRel();
  int maxThis = feP.getSize();
  int maxP2   = P2.feP.getSize();
  int currentThis = 0;
  int currentP2   = 0;
  int compare;
  BasicPoint BP1,BP2;
  fEPoint Q1;
  fEPoint Q2;
  double Z1,Z2;
  while( currentThis<maxThis & currentP2<maxP2){
     Q1 =(fEPoint) feP.get(currentThis);
     BP1 = (BasicPoint)Q1.basic();
     Q2 = (fEPoint)P2.feP.get(currentP2);
     BP2 = (BasicPoint)Q2.basic();

     compare = BP1.compareTo(BP2);
     if(compare <0){
        currentThis++;
     }
     if(compare >0){
        currentP2++;
     }
     if(compare==0){
         Z1 = Q1.getZ();
         Z2 = Q2.getZ();
         // by definition are Z1 and Z2 greater then 0
         if(Z1==Z2)
            result.setValue(3,true);
         if(Z1>Z2)
            result.setValue(4,true);
         if(Z2>Z1)
            result.setValue(5,true);
       currentThis++;
       currentP2++;
     } // compare==0
  } // while
  return result;
}


/** computes the fuzzy top. Relation between this and L */
public FuzzyTopRel topolRelation(FLine L){
FuzzyTopRel result = new FuzzyTopRel();
int max     = feP.getSize();
BasicPoint BP;
fEPoint    FP;
double Z1,Z2;
for(int current=0; current<max;current++){
   FP = (fEPoint)feP.get(current);
   BP = (BasicPoint)FP.basic();
   if(L.contains(BP)){
      Z1 = FP.getZ();
      Z2 = L.maxZfkt(BP);
      if(Z2>0){
         if(Z1==Z2)
            result.setValue(3,true);
         if(Z1>Z2)
            result.setValue(4,true);
         if(Z2>Z1)
            result.setValue(5,true);
      }
      else {  // Z2==0
        result.setValue(1,true);
      } // else
   } // if
} // for
return result;
} // topolRelation


/** computes the fuzzy top. Relation between this and L */
public FuzzyTopRel topolRelation(FRegion R){
FuzzyTopRel result = new FuzzyTopRel();
int max     = feP.getSize();
BasicPoint BP;
fEPoint    FP;
double Z1,Z2;
for(int current=0; current<max;current++){
   FP = (fEPoint)feP.get(current);
   BP = (BasicPoint)FP.basic();
   if(R.contains(BP)){
      Z1 = FP.getZ();
      Z2 = R.maxZfkt(BP);
      if(Z2>0){
         if(Z1==Z2)
            result.setValue(3,true);
         if(Z1>Z2)
            result.setValue(4,true);
         if(Z2>Z1)
            result.setValue(5,true);
      }
      else {  // Z2==0
        result.setValue(1,true);
      } // else
   } // if
} // for
return result;
} // topolRelation


/** computes the topological relationship between this and CO */
public FuzzyTopRel topolRelation(CompositeObject CO){
 if(CO instanceof FPoint)
    return topolRelation((FPoint)CO);
 if(CO instanceof FLine)
    return topolRelation((FLine)CO);
 if(CO instanceof FRegion)
    return topolRelation((FRegion)CO);
 return null;
}


/** returns the ListExpr representation of this FPoint 
  * (SF , (<PointList>))
  */
public ListExpr toListExpr(){
  // first create the PointList
  ListExpr Points;
  ListExpr Last=null;
  if(feP.getSize()==0)
     Points = ListExpr.theEmptyList();
  else {
     Points = ListExpr.oneElemList(((fEPoint)feP.get(0)).toListExpr());
     Last = Points;
  }
  fEPoint NextPoint;
  for(int i=1;i<feP.getSize();i++){
     NextPoint = (fEPoint) feP.get(i);
     Last=ListExpr.append(Last,NextPoint.toListExpr());
  }
  return ListExpr.twoElemList( ListExpr.realAtom((float)SF),Points);
}

/** returns a String representation of the corresponding ListExpr*/
public String toListString(){
  return toListExpr().writeListExprToString();
}


/** creates a new ListEXpr <type,value>*/
public ListExpr toTypedListExpr(){
  return ListExpr.twoElemList( ListExpr.symbolAtom("fpoint"),toListExpr());
}


/** read the FPoint from a String representation of a ListExpr
  * @return true if List is a String of a ListExpr containing a correct FPoint
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


/** compares two FPoints */
public int compareTo(FPoint P){
   if(SF<P.SF)
      return -1;
   if(SF>P.SF)
      return 1;
   return feP.compareTo(P.feP);
}



/** read this FPoint from a ListExpr
  * @return true if LE is a valid Representaion of a FPoint
  * all valid Points of this List are insertet
  */
public boolean readFromListExpr(ListExpr LE){
  SF = 1.0;
  feP.makeEmpty();
  computeBoundingBox();
  if(LE==null)
     return false;
  if(LE.listLength()!=2)
     return false;
  ListExpr SFList = LE.first();
  if( !( SFList.isAtom() && (SFList.atomType()==ListExpr.INT_ATOM ||
         SFList.atomType()==ListExpr.REAL_ATOM)))
     return false;
  double z= SFList.atomType()==ListExpr.INT_ATOM ? SFList.intValue() :
                                                   SFList.realValue();
  if(z<=0)
     return false;
  this.SF = z;


  ListExpr Points = LE.second();
  fEPoint P;
  boolean ok = true;
  while( !Points.isEmpty() & ok) {
    P = new fEPoint(0,0,0);

    if(P.readFromListExpr(Points.first())){
       add(P);

       Points=Points.rest();
    }
    else{

       ok = false;
    }

  }
  return ok;
}


/** this method is used for reading a fuzzy point from a byte array;
  * returns null if the construction of the object failed
  */
public static FPoint readFrom(byte[] buffer){
   try{
      ObjectInputStream ois;
      ois = new ObjectInputStream(new ByteArrayInputStream(buffer));
      FPoint res = (FPoint) ois.readObject();
      ois.close();
      return res;
   } catch(Exception e){
         return null;
     }
}


/** this method serialized an object */
public  byte[] writeToByteArray(){

  try{
     ByteArrayOutputStream byteout;
     byteout = new ByteArrayOutputStream(feP.getSize()*16+25);
     ObjectOutputStream objectout = new ObjectOutputStream(byteout);
     objectout.writeObject(this);
     objectout.flush();
     byte[] res = byteout.toByteArray();
     objectout.close();
     return  res;
  } catch(Exception e){
     return null;
  }
}

/** computes a hash-value for this FPoint */
public int getHashValue(){
  return Math.abs((BB.getMaxX()-BB.getMinX())*
                  (BB.getMaxY()-BB.getMinY())+BB.getMinX()+BB.getMinY());
}


/** save the byte representation of this object in a file,
  *for debugging only
  */
public void save(){
  String FileName = "fpoint"+no;
  try{
     OutputStream out = new FileOutputStream(FileName);
     byte[] content = writeToByteArray();
     System.out.println("write "+content.length+" bytes to "+FileName);
     for(int i=0;i<content.length;i++){
         out.write(content[i]);
     }
     out.flush();
     out.close();
  }catch(Exception e){
     System.out.println("error in writing fpoint");
  }
}


// define constants for the operators
private static final int UNION = 0;             // union based on max
private static final int INTERSECTION=1;        // difference based on min
private static final int ADD=2;                 // addition with cut if >1
private static final int SUBTRACT=3;            // substraction with cut if<0

private static final int SCALEDUNION=4;
private static final int SCALEDINTERSECTION=5;
private static final int SCALEDADD=6;
private static final int SCALEDDIFFERENCE=7;

private static int no = 0;


} // FPoint;




