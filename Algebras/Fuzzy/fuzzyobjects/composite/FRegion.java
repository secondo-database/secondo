package fuzzyobjects.composite;

import fuzzyobjects.basic.*;
import fuzzyobjects.simple.*;
import java.util.Vector;
import sj.lang.ListExpr;
import java.io.*;
/**
 * this class provides a implementation of fuzzy regions
 * in the X-triangulation
 * @author Thomas Behr
 */

public class FRegion implements CompositeObject{

/** the factor of scale */
protected double SF;              // factor of scale

/** the set of fuzzy triangles */
protected SortedObjects  fTs;     //  the fuzzy triangles

/** the Bounding Box */
protected BoundingBox  BB = new BoundingBox();

/** returns the bounding box of this region */
public BoundingBox getBoundingBox(){ return BB; }

/** returns the number of containing triangles */
public int getSize(){
  return fTs.getSize();
}

/** returns the triangle at given position */
public fTriangle getTriangleAt(int index){
  if(index<0 || index >=fTs.getSize())
     return null;
  return (fTriangle) fTs.get(index);
}

/* remove all containibng triangles */
public void clear(){
   fTs.makeEmpty();
}

/** computes the boundinmg box of this region */
private void computeBoundingBox(){
if(fTs.isEmpty())
  BB.setBox(0,0,0,0);
else{
  SimpleObject FT = fTs.get(0);
  int minX = FT.getMinX();
  int minY = FT.getMinY();
  int maxX = FT.getMaxX();
  int maxY = FT.getMaxY();
  int cX,cY;
  for (int i=1;i<fTs.getSize();i++){
    FT = fTs.get(i);
    cX = FT.getMinX();
    cY = FT.getMinY();
    if(cX < minX)
      minX = cX;
    if(cY < minY)
      minY = cY;
    cX = FT.getMaxX();
    cY = FT.getMaxY();
    if(cX > maxX)
      maxX = cX;
    if(cY > maxY)
      maxY = cY;
  }
  BB.setBox(minX,minY,maxX,maxY);
}
}

/**
 * creates a new FRegion with given factor of scale
 */
public FRegion(double scale){
   fTs = new SortedObjects();
   SF  = scale;
}

/**
 * creates a new fuzzy Region with factor of scale 1.0
 */
public FRegion() {
    this(1.0);
}



/**
 * returns a readable representation of this FRegion
 */
public String toString(){
   return "FRegion (SF="+SF+")\n"+fTs.toString();
}

/** returns the factor of scale */
public double getSF(){ return SF;}

/** return the dimension of a region, i.e. 2 */
public int getDim(){ return 2; }

/** set the factor of scale */
public boolean setSF(double SF){
  if(SF>0){
    this.SF=SF;
    return true;
  }
  else return false;
}

/**
 * add a new fuzzy triangle to this FRegion
 * returns false if this FRegion allready contains
 * a fTriangle whith same basic as fT
 */
public boolean add(fTriangle fT){
   boolean first = fTs.isEmpty();
   boolean ok = fTs.insert(fT.copy());
   if(ok) {   // update bounding box
     if(!first){
       int minX = BB.getMinX();
       int minY = BB.getMinY();
       int maxX = BB.getMaxX();
       int maxY = BB.getMaxY();
       if(minX>fT.getMinX())
          minX = fT.getMinX();
       if(minY>fT.getMinY())
          minY = fT.getMinY();
       if(maxX<fT.getMaxX())
         maxX = fT.getMaxX();
       if(maxY<fT.getMaxY())
          maxY = fT.getMaxY();
       BB.setBox(minX,minY,maxX,maxY);
    }
    else
      BB.setBox(fT.getMinX(),fT.getMinY(),fT.getMaxX(),fT.getMaxY());
  }
 return ok;
}

/**
 * updated a fTriangle containing in this FRegion
 * returns false if this FRegion not contains a
 * fTriangle whith same basic as fT
 */
public boolean update(fTriangle fT){
   return fTs.update(fT.copy());
}

/**
 * updated / added a fuzzy Triangle
 * if this FRegion allready contains a fTriangle whith same
 * basic as fT then it was updated
 * else fT is added to this FRegion
 */
public void overwrite(fTriangle fT){
   boolean first = fTs.isEmpty();
   if(fT.getMaxZ()==0){
      delete((BasicTriangle) fT.basic());
   }
   else{
      if (!fTs.update(fT))
         fTs.insert(fT);
      if(!first){
        int minX = BB.getMinX();
        int minY = BB.getMinY();
        int maxX = BB.getMaxX();
        int maxY = BB.getMaxY();
        if(minX>fT.getMinX())
           minX = fT.getMinX();
        if(minY>fT.getMinY())
           minY = fT.getMinY();
        if(maxX<fT.getMaxX())
          maxX = fT.getMaxX();
        if(maxY<fT.getMaxY())
           maxY = fT.getMaxY();
        BB.setBox(minX,minY,maxX,maxY);
      }
      else
        BB.setBox(fT.getMinX(),fT.getMinY(),fT.getMaxX(),fT.getMaxY());
  }
 }

/**
 * is this a valid FRegion,
 * i.e. all containing fTriangles are valid and
 * factor of scale is greater then 0
 */
public boolean isValid(){
  boolean ok = SF>0;
  return ok & fTs.allValid();
}


/**
 * creates a new FTriangle and invokes
 * <a href="#overwrite(fTriangle)"> overwrite(fTriangle) </A>
 * if it is valid
 */
public boolean overwrite( int x1, int y1, double Z1,
                       int x2, int y2, double Z2,
                       int x3, int y3, double Z3 ){

 fTriangle FT = new fTriangle(x1,y1,Z1,x2,y2,Z2,x3,y3,Z3);
 if(Z1==0 & Z2==0 & Z3==0){
   BasicPoint P1 = new BasicPoint(x1,y1);
   BasicPoint P2 = new BasicPoint(x2,y2);
   BasicPoint P3 = new BasicPoint(x3,y3);
   delete(new BasicTriangle(P1,P2,P3));
   return true;
 }
 else{
    boolean result= FT.isValid();
    if (result)
       overwrite(FT);
    return result;
 }
}

/**
 * deletes the fTriangle whith basic BT from this FRegion
 * returns false if this FRegion not contains a such fTriangle
 */
public boolean delete(BasicTriangle BT){
   boolean ok = fTs.delete(BT);
   if(ok & ( BT.getMinX()==BB.getMinX() |
             BT.getMaxX()==BB.getMaxX() |
             BT.getMinY()==BB.getMinY() |
             BT.getMaxY()==BB.getMaxY() )  )
        computeBoundingBox();
   return ok;
}

/**
 * is this FRegion equals to FR ?
 */
public boolean equals(FRegion FR){
 boolean ok;
 if(FR==null){
   ok=false;
 }
 else if(this==FR) // the same reference
   ok=true;
 else
   ok=  SF == FR.SF && fTs.equals(FR.fTs) ;
 return ok;
}


/** computes all membershipvalues on (x,y) */

public double[] ZRel(double x, double y) {

Vector tmp = new Vector();
Double Z;
BasicTriangle[] Bts = BasicTriangle.getTriangles(x,y);

fTriangle current;
for(int i=0;i<Bts.length;i++){
    current = (fTriangle) fTs.search(Bts[i]);
    if(current!=null){
      Z = new Double(current.Zfkt(x,y));
      if(!tmp.contains(Z))
        tmp.add(Z);
    }
    else{
       Z = new Double(0);
       if(!tmp.contains(Z))
          tmp.add(Z);
    }
}

double[] result;
if(tmp.size()==0){
  result=new double[1];
  result[0] = 0;
}
else{
  result = new double[tmp.size()];
  for(int i=0;i<tmp.size();i++)
     result[i] = ((Double)tmp.get(i)).doubleValue();
}
return result;
}  // ZRel


/** computes all membershipvalues on BP */
public double[] ZRel(BasicPoint BP){
Vector tmp=new Vector(10); // contains all Triangles connected with BP
BasicPoint[] Neightboors = BP.getNeightboors();
// find all pairs of connected Neightboors

 for(int i=0;i<Neightboors.length-1;i++)
   for(int j=i+1;j<Neightboors.length;j++)
      if( Neightboors[i].neightbooring(Neightboors[j]))
        tmp.add(new BasicTriangle(BP,Neightboors[i],Neightboors[j]));

Vector tmpResult = new Vector();
fTriangle currentfT;
for(int k=0;k<Neightboors.length;k++){
   currentfT = (fTriangle) fTs.search((BasicTriangle) tmp.get(k));
   if(currentfT!=null)
     tmpResult.add(new Double(currentfT.Zfkt(BP)));
   else
     tmpResult.add(new Double(0));
}

double[] result = new double[tmpResult.size()];
for(int i=0;i<tmpResult.size();i++){
   result[i] = ((Double) tmpResult.get(i)).doubleValue();
}
return result;
}


/** computes the maximal membershipvalue on BP */
public double maxZfkt(BasicPoint BP){
  double[] tmp = ZRel(BP);
  double result = 0;
  for(int i=0;i<tmp.length;i++)
     if(result<tmp[i]) result = tmp[i];
  return result;
}

/** computes the membershipvalue by given basicelement */
public double Zfkt(BasicPoint BP, BasicTriangle BT){
  double result = 0.0;
  fTriangle FT = (fTriangle) fTs.search(BT);
  if (FT!=null)
     result= FT.Zfkt(BP);
  return result;
}

/** computes the membership-value on (x,y) by give BasicTriangle */
public double Zfkt(double x, double y, BasicTriangle BT){
 if (BT.contains(x,y)){
   fTriangle FT = (fTriangle) fTs.search(BT);
   if(FT!=null)
     return FT.Zfkt(x,y);
   else
     return 0.0;
 }
 else return 0.0;
}

/** returns the mimimum membership-value on (x,y) */
public double minZfkt(double x, double y){
 double[] cands = ZRel(x,y);
 double min = cands[0];
 for(int i=1;i<cands.length;i++)
   if(cands[i]<min)
      min = cands[i];
 return min;
}

/** returns the maximum membership-value on (x,y) */
public double maxZfkt(double x, double y){
 double[] cands = ZRel(x,y);
 double max = cands[0];
 for(int i=1;i<cands.length;i++)
   if(cands[i]<max)
      max = cands[i];
 return max;
}

/** returns the middle membership-value on (x,y) */
public double midZfkt(double x, double y){
 double[] cands = ZRel(x,y);
 double sum = 0;
 for(int i=0;i<cands.length;i++)
   sum += cands[i];
 return sum/cands.length;
}


/** computes all basicTriangles containing in this fRegion */
public BasicTriangle[] basics() {
  BasicTriangle[] result = new BasicTriangle[fTs.getSize()];
  if(! fTs.isEmpty()) {
    for(int i=0;i<fTs.getSize();i++)
      result[i] = (BasicTriangle) ((fTriangle)fTs.get(i)).basic();
  }
  return result; 
} // basics

/** computes the maximal membershipvalue */
public double maxZ(){
 double max = 0.0;
 double currentmax;       
 double Z1,Z2,Z3;
 fTriangle current;
 for(int i=0;i<fTs.getSize();i++){
    current = (fTriangle) fTs.get(i);
    Z1 = current.getZ1();
    Z2 = current.getZ2();
    Z3 = current.getZ3();
    currentmax = ( Math.max(Z1,Math.max(Z2,Z3)));
    if (max < currentmax)
        max = currentmax;
  }
  return max;
} // maxZ

/** computes the minimal membershipvalue */
public double minZ(){
 double min = 1.0;
 if(fTs.getSize()==0)
   min=0.0;
 double currentmin;       
 double Z1,Z2,Z3;
 fTriangle current;
 for(int i=0;i<fTs.getSize();i++){
    current = (fTriangle) fTs.get(i);
    Z1 = current.getZ1();
    Z2 = current.getZ2();
    Z3 = current.getZ3();
    currentmin = ( Math.min(Z1,Math.min(Z2,Z3)));
    if (min > currentmin)
        min = currentmin;
  }
  return min;
} // minZ


/** returns a copy of this FRegion */
public FRegion copy(){
  FRegion C = new FRegion(SF);
  C.fTs = fTs.copy();
  C.BB = BB.copy();
  return C;
}

/** contains this FRegion elements ? */
public boolean isEmpty(){
 return fTs.isEmpty();
}


/*************************************************************
   Operators
 *************************************************************/

/** the operator union */
public FRegion union(FRegion With){
  return  operator(With,UNION);
}

/** the operator scaled union */
public FRegion scaledUnion(FRegion With){
  FRegion result =  operator(With,SCALEDUNION);
  result.norm();
  return result;
}


/** the operator intersection */
public FRegion intersection(FRegion With){
   return operator(With,INTERSECTION);
}

/** the operator scaled intersection */
public FRegion scaledIntersection(FRegion With){
  FRegion result = operator(With,SCALEDINTERSECTION);
  result.norm();
  return result;
}

/** the add-operator */
public FRegion add(FRegion With){
   return operator(With,ADD);
}

/** the operator scaled add */
public FRegion scaledAdd(FRegion With){
  FRegion result = operator(With,SCALEDADD);
  result.norm();
  return result;
}

/** the difference-operator */
public FRegion difference(FRegion With){
   return operator(With,SUBTRACT);
}

/** the operator scaled difference */
public FRegion scaledDifference(FRegion With){
  FRegion result = operator(With,SCALEDDIFFERENCE);
  result.norm();
  return result;
}

/** the alpha-cut of this FRegion */
public FRegion alphaCut(double alpha, boolean strong){
 FRegion CutR = new FRegion(SF);
 fTriangle Current;
 boolean  isValid;
 double midZ;

 for(int i=0; i<fTs.getSize(); i++){
    Current = (fTriangle) fTs.get(i);
    midZ = (Current.getZ1()+ Current.getZ2() + Current.getZ3())/3;
    if (strong)
       isValid = midZ>alpha;
    else
       isValid = midZ>= alpha;

    if(isValid)
       CutR.fTs.insert(Current.copy());
 }
 CutR.computeBoundingBox();
 return CutR;
}

/** computes the area of the basic from this FRegion */
public double basicArea(){
    double a = fuzzyobjects.Params.a;
    double b = fuzzyobjects.Params.b;
    return (a*b/4)*fTs.getSize();
}

/** computes the surface of the 3d-structure of this FRegion */
public double area3D(){
  double result =0;
  for(int i=0;i<fTs.getSize();i++)
     result += ((fTriangle)fTs.get(i)).area3D();
  return result;
}

/** the weigthted area of this FRegion */
public double area(){
  double result = 0;
  for(int i=0;i<fTs.getSize();i++)
      result += ((fTriangle)fTs.get(i)).volume();
  return result;
}

/** how similar are 2 fregion in their basic */
public double basicSimilar(FRegion F2){
     FRegion Funion         = this.union(F2);
     FRegion Fintersection  = this.intersection(F2);
     if (Funion.isEmpty())
        return 1.0;
     else
        return Fintersection.basicArea() / Funion.basicArea();
}

/** how similar are 2 FRegions */
public double similar(FRegion F2){
     FRegion Funion         = this.union(F2);
     FRegion Fintersection  = this.intersection(F2);
     if (Funion.isEmpty())
        return 1.0;
     else
        return Fintersection.area() / Funion.area();
}


/**
 * returns a sharp Fregion
 * all membershipvalues are 1.0
 */
public FRegion sharp(){
 FRegion result = new FRegion(1);
 fTriangle CurrentTriangle;
 for(int i=0;i<fTs.getSize();i++){
   CurrentTriangle = new fTriangle( (BasicTriangle) ((fTriangle)
                                     fTs.get(i)).basic(),1,1,1);
   result.add(CurrentTriangle);
 }
  return result;
}

/** computes the boundary of this fRegion */
public FLine boundary(){

FLine result = new FLine(1);
BasicTriangle CurrentBT;
BasicTriangle[] Neightboors;
BasicSegment BS;

for(int i=0;i<fTs.getSize();i++){
   CurrentBT = (BasicTriangle) ( (fTriangle) fTs.get(i)).basic();
   Neightboors = CurrentBT.getNeightboors();

   for(int j=0;j<Neightboors.length;j++) {
      if ( fTs.search(Neightboors[j])==null ) {
          BS = CurrentBT.commonSegment(Neightboors[j]);
          result.add(new fSegment(BS,1,1));
      }
   }
}
return result;
}

/**
  * computes the maximal connected part of a fRegion whith
  * given start location
  */
/*
private void getComponent(fTriangle FT, FRegion result){
 BasicTriangle BT = (BasicTriangle) FT.basic();
 if (result.fTs.getPos(BT)>0) return;

 result.add(FT);
 BasicTriangle[] Neightboors = BT.getNeightboors();

 for (int i=0;i<Neightboors.length;i++) {
    if(result.fTs.getPos(Neightboors[i])<0){      // not in result 
      fTriangle NB = (fTriangle) fTs.search(Neightboors[i]);
       // in current Region ?
      if(NB!=null){
        getComponent(NB,result);
      }
    }
 }

}
*/

private void getComponent(fTriangle FT, FRegion result){

  Vector List = new Vector(fTs.getSize());
  fTriangle Current;
  boolean found;
  fTriangle N;
  BasicTriangle CurrentBasic;
  BasicTriangle[] Neightboors;
  BasicTriangle Next;

  Current = FT;

  do{
     CurrentBasic = (BasicTriangle) Current.basic();
     result.add(Current);
     Neightboors = CurrentBasic.getNeightboors();
     for(int i=0;i<Neightboors.length;i++){  // for all Neightboors
        found=false;
        for(int j=0;j<List.size();j++){
           if(Neightboors[i].equals((BasicTriangle) List.get(j))) 
              found=true;
        }

        if(!found && result.fTs.search(Neightboors[i])!=null)
           found = true;

        if(!found && fTs.getPos(Neightboors[i])>=0)
           List.add(Neightboors[i]);

     }// for all Neightboors

     if(List.size()>0){
        Next = (BasicTriangle) List.get(0);
        List.remove(0);
        Current=(fTriangle) fTs.search(Next);
     }
     else
       Current = null;
  } while( Current!=null);

}


/** returns the connected components of this FRegion */
public FRegion[] faces(){
  if(isEmpty()) return null;
  Vector tmp = new Vector();  // for the faces ;
                              // use Vector hence number of faces unknow
  FRegion Copy = this.copy();

  while( ! Copy.isEmpty()){
    FRegion nextFace = new FRegion(SF);
    fTriangle First = (fTriangle) Copy.fTs.get(0);
    getComponent(First,nextFace);
    tmp.add(nextFace);
    Copy = Copy.difference(nextFace);
  }

  FRegion[] result = new FRegion[tmp.size()];
  for(int i=0;i<tmp.size();i++)
     result[i] = (FRegion) tmp.get(i);

  return result;
}

/** the mid-operator */
public static FRegion mid(FRegion[] Regs){

if(Regs.length==0) return null;
FRegion result    = new FRegion(1);
int[] current     = new int[Regs.length];
int[] max         =  new int[Regs.length];
boolean[] ready   = new boolean[Regs.length];
boolean allready  = true;

// initialize the variables
for(int i=0;i<Regs.length;i++){
   current[i]=0;
   max[i]= Regs[i].fTs.getSize();
   ready[i] = current[i]>=max[i];
   if(!ready[i]) allready=false;
}


Vector allSmallest = new Vector(); // the smallest Triangles
Vector Numbers     = new Vector(); // the positions in Regs

while(!allready){
  allready=true;
  allSmallest = new Vector();
  Numbers = new Vector();
  // search all smallest Triangles
  fTriangle currentT;
  BasicTriangle compareT;
  for(int i=0;i<Regs.length;i++){
     if(current[i]<max[i]) {
       allready=false;
       currentT = (fTriangle) Regs[i].fTs.get(current[i]);
       if(allSmallest.size()==0) {
          allSmallest.add(currentT);
          Numbers.add(new Integer(i));
       }
       else {
        compareT = (BasicTriangle) ((fTriangle) allSmallest.get(0)).basic();
        int comp = currentT.basic().compareTo(compareT);
        if(comp==0) {  // a Triangle with smallest Basic
          allSmallest.add(currentT);
          Numbers.add(new Integer(i));
        }
        if(comp<0) {  // new smallest Triangle
         allSmallest = new Vector();
         Numbers = new Vector();
         allSmallest.add(currentT);
         Numbers.add(new Integer(i));
        }
        // in the case comp>0 is nothing to do
      } // not the first triangle
    } // current Reg contains unprocessing triangle(s)
  } // for all Regs

  if(!allready){
    double Z1=0,Z2=0,Z3=0;
    BasicTriangle BT = (BasicTriangle) (
                            (fTriangle) allSmallest.get(0)).basic();
    fTriangle CT;
    for(int i=0;i<allSmallest.size();i++){
       // update numbers
       int c = ((Integer)Numbers.get(i)).intValue();
       current[c]++;
       CT = (fTriangle) allSmallest.get(i);
       Z1 += CT.getZ1();
       Z2 += CT.getZ2();
       Z3 += CT.getZ3();
    }
    Z1 = Z1 / Regs.length;
    Z2 = Z2 / Regs.length;
    Z3 = Z3 / Regs.length;
    result.fTs.insert( new fTriangle(BT,Z1,Z2,Z3));
  }

}  // while not all ready
 result.computeBoundingBox();
 return result;
}


/** the scaled-mid-operator */
public static FRegion scaledMid(FRegion[] Regs){

if(Regs.length==0) return null;
FRegion result    = new FRegion(1);
int[] current     = new int[Regs.length];
int[] max         = new int[Regs.length];
boolean[] ready   = new boolean[Regs.length];
boolean allready  = true;

// initialize the variables
for(int i=0;i<Regs.length;i++){
   current[i]=0;
   max[i]= Regs[i].fTs.getSize();
   ready[i] = current[i]<max[i];
   if(!ready[i]) allready=false;
}

Vector allSmallest = new Vector(); // the smallest Triangles
Vector Numbers     = new Vector(); // the positions in Regs

while(!allready){
  allready=true;
  allSmallest = new Vector();
  Numbers = new Vector();
  // search all smallest Triangles
  fTriangle currentT;
  BasicTriangle compareT;
  for(int i=0;i<Regs.length;i++){
     if(current[i]<max[i]) {
       allready=false;
       currentT = (fTriangle) Regs[i].fTs.get(current[i]);
       if(allSmallest.size()==0) {
          allSmallest.add(currentT);
          Numbers.add(new Integer(i));
       }
       else {
        compareT = (BasicTriangle) ((fTriangle) allSmallest.get(0)).basic();
        int comp = currentT.basic().compareTo(compareT);
        if(comp==0) {  // a Triangle with smallest Basic
          allSmallest.add(currentT);
          Numbers.add(new Integer(i));
        }
        if(comp<0) {  // new smallest Triangle
         allSmallest = new Vector();
         Numbers = new Vector();
         allSmallest.add(currentT);
         Numbers.add(new Integer(i));
        }
        // in the case comp>0 is nothing to do
      } // not the first triangle
    } // current Reg contains unprocessing triangle(s)
  } // for all Regs

  if(!allready){
    double Z1=0,Z2=0,Z3=0;
    BasicTriangle BT = (BasicTriangle) ((fTriangle)
                                allSmallest.get(0)).basic();
    fTriangle CT;
    for(int i=0;i<allSmallest.size();i++){
       // update numbers
       int c = ((Integer)Numbers.get(i)).intValue();
       CT = (fTriangle) allSmallest.get(i);
       Z1 += CT.getZ1()*Regs[c].SF;
       Z2 += CT.getZ2()*Regs[c].SF;
       Z3 += CT.getZ3()*Regs[c].SF;
    }
    Z1 = Z1 / Regs.length;
    Z2 = Z2 / Regs.length;
    Z3 = Z3 / Regs.length;
    result.fTs.insert( new fTriangle(BT,Z1,Z2,Z3));
  }

}  // while not all ready
 result.norm();
 result.computeBoundingBox();
 return result;
}

/**
 * returns the number of triangles containing in this
 * fRegion having BP as cornerpoint
 */
public int numberOfTriangles(BasicPoint BP){

// returns number of Triangles which have BP as cornerpoint

Vector tmp=new Vector(10); // contains all Triangles connected with BP

BasicPoint[] Neightboors = BP.getNeightboors();
// find all pairs of connected Neightboors

 for(int i=0;i<Neightboors.length-1;i++)
   for(int j=i+1;j<Neightboors.length;j++)
      if( Neightboors[i].neightbooring(Neightboors[j]))
        tmp.add(new BasicTriangle(BP,Neightboors[i],Neightboors[j]));

// try to find this triangles
int result=0;
BasicTriangle BT;
for(int k=0;k<tmp.size();k++){
  BT = (BasicTriangle) tmp.get(k);
  if( fTs.search(BT)!=null )
    result++;
}

return result;
}

/**
 * returns the number of triangles containing in this FRegion
 * having BS as side
 */
public int numberOfTriangles(BasicSegment BS){
  BasicTriangle[] BTs = BS.getTriangles();
  int result=0;
  for(int i=0;i<BTs.length;i++)
    if (fTs.search(BTs[i])!=null) result++;

  return result;
}

/** is BS a segment on boundary of this FRegion ? */
public boolean onBoundary(BasicSegment BS){
  return numberOfTriangles(BS)==1;
}

/** is BP on boundary of this fRegion ? */
public boolean onBoundary(BasicPoint BP){
// a Point is on a Boundary of a Region R <=>
// exists a Segment  S : BP isendpoint of BP and S is on the Boundary of R

BasicPoint[] Neightboors = BP.getNeightboors();
BasicSegment BS;
boolean found=false;

for(int i=0;i<Neightboors.length&!found;i++){
   BS = new BasicSegment(BP,Neightboors[i]);
   found = onBoundary(BS);
}
  return found;
}



/** returns commonLines which are not part of a common area */
public FLine commonLines(FRegion R2){

int max = fTs.getSize();
FLine result = new FLine();
fTriangle FT;
BasicTriangle BT;
BasicSegment[]  Sides;
BasicTriangle[] BTs;
boolean CommonTriangle;
boolean here,inR2;

for(int i=0;i<max;i++){
  FT = (fTriangle) fTs.get(i);
  BT = (BasicTriangle) FT.basic();
  Sides = BT.getSides();
  for(int j=0;j<Sides.length;j++){
    if(R2.contains(Sides[j])) { // else is nothing to do
      BTs=Sides[j].getTriangles();
      CommonTriangle=false;
      for(int k=0;k<BTs.length;k++){
        here = BTs[k].equals(BT) || this.contains(BTs[k]);
        inR2 = R2.contains(BTs[k]);
        if(here & inR2)     // exists a common area
           CommonTriangle=true;
      }
      if(!CommonTriangle)
        result.add(new fSegment(Sides[j],1,1));
    } //if
  }// for
} // for

return result;
}



/** returns the common points which not are a part of a common segment */
public FPoint commonPoints(FRegion R2){

int max = fTs.getSize();
FPoint result = new FPoint();
fTriangle FT;
BasicTriangle BT;
BasicPoint[]  Corners;
BasicSegment[] Segs;
boolean CommonSegment;
boolean here,inR2;

for(int i=0;i<max;i++){
  FT = (fTriangle) fTs.get(i);
  BT = (BasicTriangle) FT.basic();
  Corners = BT.getBasicPoints();
  for(int j=0;j<Corners.length;j++){
    if(R2.contains(Corners[j])) { // else is nothing to do
      Segs= BasicSegment.getSegments(Corners[j]);
      CommonSegment=false;
      for(int k=0;k<Segs.length;k++){
        here =  this.contains(Segs[k]);
        inR2 = R2.contains(Segs[k]);
        if(here & inR2)     // exists a common area
           CommonSegment=true;
      }
      if(!CommonSegment)
        result.add(new fEPoint(Corners[j],1));
    } //if
  }// for
} // for

return result;

}



/**
  * converts a fLine with propertys of a circled Simple path to a
  * Simple Path
  */
private SimplePath convertToSimplePath(FLine L){

SortedObjects fSegs = L.getSortedObjects();

fSegment Current = (fSegment) fSegs.get(0);
BasicSegment CurrentB = (BasicSegment) Current.basic();
SimplePath result = new SimplePath();
result.extend(CurrentB.getEP1());
result.extend(CurrentB.getEP2());

do{

  BasicPoint BP = result.getLastPoint();
  BasicSegment[] ConnectedSegment = BasicSegment.getSegments(BP);

  int j=-1;
  for(int i=0;i<ConnectedSegment.length;i++){
    BasicSegment BS = ConnectedSegment[i];
    if( !BS.equals(CurrentB) && fSegs.search(BS)!=null )
      j=i;
  }

  BasicPoint BP1,BP2;
  BP1 = ConnectedSegment[j].getEP1();
  BP2 = ConnectedSegment[j].getEP2();
  if(BP1.equals(BP))
     result.extend(BP2);
  else
    result.extend(BP1);
  CurrentB= ConnectedSegment[j]; // explore the next segment
}
while(!result.isACircle());

return result;
}




/** splits a connected Line into SimplePaths */
private SimplePath[] splitLine(FLine L){
 SimplePath[] result;
 BasicPoint[] cuts = L.selfcuts();
 if(cuts.length==0){
    result = new SimplePath[1];
    SimplePath P = convertToSimplePath(L);
    result[0] = P;
    return result;
 }
 else{
   Vector tmp = new Vector();
   // compute simple Path from a selfcut to another one
   SortedObjects fSegs = L.getSortedObjects(); // the segments

   boolean[] used = new boolean[fSegs.getSize()]; // mark used segments
   for(int i=0;i<used.length;i++) // no segment is used
      used[i] = false;

   BasicSegment[] NextSegments; // the segments from a selfcut

   int pos; // the pos of a segment in fSegs

   for(int i=0;i<cuts.length;i++){ // from a selfcut
      NextSegments = BasicSegment.getSegments(cuts[i]);
      for(int j=0;j<NextSegments.length;j++){ // explore all Segs
        pos = fSegs.getPos(NextSegments[j]);
        if( pos>=0 && !used[pos] ) {
           BasicSegment CurrentB = NextSegments[j];
           SimplePath nextPath = new SimplePath();
           nextPath.extend(cuts[i]);
           if(CurrentB.getEP1().equals(cuts[i]))
               nextPath.extend(CurrentB.getEP2());
           else
               nextPath.extend(CurrentB.getEP1());
           used[pos] = true;
           boolean ready = false;
           BasicPoint La = nextPath.getLastPoint();
           for(int s=0;s<cuts.length;s++)
             if(La.equals(cuts[s]))
                ready = true;
           if(!ready) {
             do{
                BasicPoint BP = nextPath.getLastPoint();
                BasicSegment[] ConnectedSegment;
		ConnectedSegment = BasicSegment.getSegments(BP);
                int k=-1;
                int foundPos;
                BasicSegment BS;
                do {
                   k++;
                   BS = ConnectedSegment[k];
                   foundPos = fSegs.getPos(BS);
                } while( BS.equals(CurrentB) || foundPos<0);
                used[foundPos] = true;
                BasicPoint BP1,BP2;
                BP1 = ConnectedSegment[k].getEP1();
                BP2 = ConnectedSegment[k].getEP2();
                BasicPoint Last;
                if(BP1.equals(BP))
                  Last = BP2;
                else
                  Last = BP1;
                nextPath.extend(Last);
                CurrentB= ConnectedSegment[k]; // explore the next segment
                for (int n=0;n<cuts.length;n++)
                   if(Last.equals(cuts[n]))
                      ready = true;
             } while(!ready);
           } // if(!ready)
           tmp.add(nextPath);
        } // if(!used && found)
      } // for(each startsegment) 
    } // for each selfcut

   // build circles from the Paths
   Vector tmp2 = new Vector(); // contains the result
   Vector tmp3 = new Vector();   // contains not circled path 

   for(int o=0;o<tmp.size();o++)
     if(((SimplePath) tmp.get(o)).isACircle())
        tmp2.add(tmp.get(o));
     else
        tmp3.add(tmp.get(o));


   // mark all simple path as 'not used'
   boolean[] usedPath= new boolean[tmp3.size()];
   for(int o=0;o<usedPath.length;o++)
      usedPath[o] = false;


   SimplePath CurrentPath;
   Vector Points;
   for(int o=0;o<tmp3.size();o++){
     if(!usedPath[o]){
        CurrentPath = (SimplePath) tmp3.get(o);
        Points = new Vector();
        usedPath[o] = true;
        BasicPoint First = CurrentPath.getFirstPoint();
        BasicPoint Last = CurrentPath.getLastPoint();
        Points.add(First);
        Points.add(Last);
        boolean ready=false;

        while(!ready) {
           // search a Path extending CurrentPath
           SimplePath EPath=null;
           BasicPoint F2= null;
           BasicPoint L2= null;
           boolean found = false;

           for(int p=0;p<tmp3.size() & !found; p++){
             if(!usedPath[p]){
               EPath = (SimplePath) tmp3.get(p);
               F2 = EPath.getFirstPoint();
               L2 = EPath.getLastPoint();
               if( F2.equals(Last) || L2.equals(Last)   ){ 
                  found=true;
                  usedPath[p] = true;
               }
             }
           }// for 

           // EPath is a Path extendend CurrentPath
           if(L2.equals(Last)){
              EPath.invert();  // flip the Path
              BasicPoint H = L2;
              L2 = F2;
              F2 = H;
           }

           if(First.equals(L2)){
             CurrentPath.extend(EPath);
             tmp2.add(CurrentPath);
             ready=true;
           }
           else{
             // test for a circle
             boolean circletest=false;
             for(int q=1;q<Points.size() & !circletest;q++){
                if(L2.equals((BasicPoint) Points.get(q)))
                   circletest=true;
             } // for
             if(circletest){
               SimplePath Rest = CurrentPath.split(L2);
               Rest.extend(EPath);
               tmp2.add(Rest);
               Last = CurrentPath.getLastPoint();
               // update Points
               int r=Points.size()-1;
               BasicPoint L3 = (BasicPoint) Points.get(r);
               while(!L3.equals(Last)){
                 Points.remove(r);
                 r--;
                 L3 = (BasicPoint) Points.get(r);
               }
             }
             else{
                CurrentPath.extend(EPath);
                Last = L2;
                Points.add(Last);
             }

           } // else (not a 'big' circle)
         } // while !ready
     } // Path not used
   } // for all Paths


   result = new SimplePath[tmp2.size()];
   for(int r=0;r<result.length;r++)
      result[r] = (SimplePath) tmp2.get(r);
   return result;
} // not a simple circle

}


/** compute the simplePaths from the boundary of a connected FRegion */
private SimplePath[] computeSimplePathsOfAFace(FRegion F){

FLine Bound = F.boundary();
FLine[] BoundFaces = Bound.faces();

Vector tmp=new Vector(); // store founded SimplePaths
for(int i=0;i<BoundFaces.length;i++){
  SimplePath[] tmp2 = splitLine(BoundFaces[i]);
  for(int j=0;j<tmp2.length;j++){
    tmp.add(tmp2[j]);
  }
}


SimplePath[] result = new SimplePath[tmp.size()];
for(int j=0;j<tmp.size();j++)
  result[j]=(SimplePath) tmp.get(j);
return result;
}


/** computes the holes of a FRegion */
public FRegion holes(){

FRegion result = new FRegion();
if(this.isEmpty())
   return result;

FRegion[] Faces = this.faces();
for(int i=0;i<Faces.length;i++){
    SimplePath[] Paths = computeSimplePathsOfAFace(Faces[i]);
    for(int j=0;j<Paths.length;j++){
      BasicTriangle BT = Paths[j].getAInnerTriangle();
      if(BT!=null){
        if(fTs.search(BT)==null) {  // is a hole
          BasicTriangle[] BTs = Paths[j].getEnclosedTriangles();
          for(int k=0;k<BTs.length;k++)
            result.add(new fTriangle(BTs[k],1,1,1));
        } // if
      }//if
    } // for
} // for

FRegion S = this.sharp();
result = result.difference(S);
return result;

}

/** returns the boundary without holes */
public FLine contour(){
 FLine B1 = this.boundary();
 FLine B2 = this.holes().boundary();
 return B1.difference(B2);
}




/************************************************************
   end of operators
 *************************************************************/



/**
 * set the containing Friangles from R1 to the same
 * trianglers as R2
 */
protected static void setFts(FRegion R1, FRegion R2){
   R1.fTs = R2.fTs;
   R1.BB = R2.BB;
}



/**
  * process a single triangle for many oparators
  */
private void processElements(fTriangle F1, double scale1,
                             fTriangle F2, double scale2,
                             FRegion Goal,
                             int Operator){

// 1 input parameter can be null 
// if both fTriangles not null, then they must have the same basic

  if( F1==null & F2==null) return;


  double Z1,Z2,Z3;
  fTriangle newFT;

  switch (Operator){

     case UNION  :  {  // the union of 2 regions ignoring SFs

                      if(F1==null)
                          Goal.fTs.insert(F2.copy());
                      else
                         if(F2==null)
                           Goal.fTs.insert(F1.copy());
                         else { // both fTriangles are not null
                           Z1 = Math.max(F1.getZ1(),F2.getZ1());
                           Z2 = Math.max(F1.getZ2(),F2.getZ2());
                           Z3 = Math.max(F1.getZ3(),F2.getZ3());
                           newFT = new fTriangle((BasicTriangle)
                                                   F1.basic(),
                                                   Z1,Z2,Z3    );
                           Goal.fTs.insert(newFT);

                         } // else

                    }
                    break;

    case INTERSECTION :
                    {  if (F1==null | F2==null)
                         ;  
                       else { // both are not null
                           Z1 = Math.min(F1.getZ1(),F2.getZ1());
                           Z2 = Math.min(F1.getZ2(),F2.getZ2());
                           Z3 = Math.min(F1.getZ3(),F2.getZ3());
                       if(Z1+Z2+Z3>0){
                           newFT = new fTriangle((BasicTriangle)
                                                      F1.basic(),
                                                      Z1,Z2,Z3    );
                           Goal.fTs.insert(newFT);
                       }
                       }

                    } break;

    case ADD     :  {
                      if(F1==null)
                          Goal.fTs.insert(F2.copy());
                      else
                         if(F2==null)
                           Goal.fTs.insert(F1.copy());
                         else { // both fTriangles are not null
                           Z1 = Math.min(1,F1.getZ1()+F2.getZ1());
                           Z2 = Math.min(1,F1.getZ2()+F2.getZ2());
                           Z3 = Math.min(1,F1.getZ3()+F2.getZ3());
                           newFT = new fTriangle((BasicTriangle)
                                                   F1.basic(),
                                                   Z1,Z2,Z3    );
                           Goal.fTs.insert(newFT);

                         } // else

                    }
                    break;

    case SUBTRACT :{
                        if(F1 == null)
                            ;
                        else
                           if(F2==null)
                             Goal.fTs.insert(F1.copy());
                           else  {   // both not null
                              Z1 = Math.max(0,F1.getZ1()-F2.getZ1());
                              Z2 = Math.max(0,F1.getZ2()-F2.getZ2());
                              Z3 = Math.max(0,F1.getZ3()-F2.getZ3());
                              if ((Z1+Z2+Z3)>0) {
                                newFT = new fTriangle( (BasicTriangle)
                                                       F1.basic(),Z1,Z2,Z3);
                                Goal.fTs.insert(newFT);
                              }
                           }
                       } break;


        case SCALEDUNION    :
                      { fTriangle newTriangle;
                        if (F1==null) 
                           newTriangle = new fTriangle( (BasicTriangle)
                                                        F2.basic(),
                                                      F2.getZ1()*scale2,
                                                      F2.getZ2()*scale2,
                                                      F2.getZ3()*scale2 );
                         else
                           if (F2==null)
                              newTriangle = new fTriangle( (BasicTriangle)
                                                           F1.basic(),
                                                          F1.getZ1()*scale1,
                                                          F1.getZ2()*scale1,
                                                          F1.getZ3()*scale1);
                           else {
                             Z1 = Math.max(F1.getZ1()*scale1,
                                                  F2.getZ1()*scale2);
                             Z2 = Math.max(F1.getZ2()*scale1,
                                                  F2.getZ2()*scale2);
                             Z3 = Math.max(F1.getZ3()*scale1,
                                                  F2.getZ3()*scale2);

                             newTriangle = new fTriangle( (BasicTriangle)
                                                           F1.basic(),
                                                           Z1,Z2,Z3);
                           }  // else
                       Goal.add(newTriangle);

                      } break;   // scaled union

     
        case SCALEDINTERSECTION :  if (F1==null || F2==null)
                                         ;
                                   else {
                                     Z1 = Math.min(F1.getZ1()*scale1,
                                                         F2.getZ1()*scale2);
                                     Z2 = Math.min(F1.getZ2()*scale1,
                                                         F2.getZ2()*scale2);
                                     Z3 = Math.min(F1.getZ3()*scale1,
                                                         F2.getZ3()*scale2);

                                    Goal.add(new fTriangle( (BasicTriangle)
                                                            F1.basic(),
                                                            Z1,Z2,Z3));
                                   } break;

        case SCALEDADD : {
                          if(F1==null)
                             Goal.add(new fTriangle( (BasicTriangle)
                                                     F2.basic(),
                                                     F2.getZ1()*scale2,
                                                     F2.getZ2()*scale2,
                                                     F2.getZ3()*scale2));
                          else
                            if(F2==null)
                              Goal.add(new fTriangle( (BasicTriangle)
                                                      F1.basic(),
                                                      F1.getZ1()*scale1,
                                                      F1.getZ2()*scale1,
                                                      F1.getZ3()*scale1));
                            else {

                              Goal.add(new fTriangle( (BasicTriangle)
                                                      F1.basic(),
                                                      F1.getZ1()*scale1 +
                                                      F2.getZ1()*scale2 ,
                                                      F1.getZ2()*scale1 +
                                                      F2.getZ2()*scale2 ,
                                                      F1.getZ3()*scale1 +
                                                      F2.getZ3()*scale2 ));


                            }

                        } break;


        case SCALEDDIFFERENCE   :
                       {
                          if(F1==null)
                             Goal.add(new fTriangle( (BasicTriangle)
                                                     F2.basic(),
                                                     -F2.getZ1()*scale2,
                                                     -F2.getZ2()*scale2,
                                                     -F2.getZ3()*scale2));
                          else
                            if(F2==null)
                              Goal.add(new fTriangle( (BasicTriangle)
                                                      F1.basic(),
                                                      F1.getZ1()*scale1,
                                                      F1.getZ2()*scale1,
                                                      F1.getZ3()*scale1));
                            else {

                              Goal.add(new fTriangle( (BasicTriangle)
                                                      F1.basic(),
                                                      F1.getZ1()*scale1 -
                                                      F2.getZ1()*scale2 ,
                                                      F1.getZ2()*scale1 -
                                                      F2.getZ2()*scale2 ,
                                                      F1.getZ3()*scale1 -
                                                      F2.getZ3()*scale2 ));


                            }

                        } break;

   

    default      : System.out.println("unimplementierter operator");


  } // switch

} // processElements


/** the 'template' for many operators */
protected FRegion operator(FRegion FR, int op){
// a kind of mergesort

int my     = 0;
int fromFR = 0;   // already processed

int maxMy     = fTs.getSize();   // numbers of elements
int maxFromFR = FR.fTs.getSize();
FRegion result = new FRegion(1);

fTriangle myFirst=null;  // the first unprocessed elements
fTriangle FRFirst=null;

if(maxMy>0)
   myFirst = (fTriangle) fTs.get(0);
if(maxFromFR>0)
   FRFirst = (fTriangle) FR.fTs.get(0);

if (maxMy >0 && maxFromFR>0){
   myFirst = (fTriangle) fTs.get(my);    
   FRFirst = (fTriangle) FR.fTs.get(fromFR);
   int compareResult;
   while(my<maxMy && fromFR<maxFromFR){
      // both sets have unprocessed elements
      compareResult = myFirst.basic().compareTo(FRFirst.basic());
      if(compareResult < 0) {
         processElements(myFirst,SF,null,FR.SF,result,op);
         my++;
         if (my<maxMy)
             myFirst = (fTriangle) fTs.get(my);
      }
      else if(compareResult > 0){
            processElements(null,SF,FRFirst,FR.SF,result,op);
            fromFR++;
            if(fromFR<maxFromFR)
               FRFirst = (fTriangle) FR.fTs.get(fromFR);
           }
           else {     // elements have the same basic
             processElements(myFirst,SF,FRFirst,FR.SF,result,op);
             my++;
             fromFR++;
             if (my<maxMy)
               myFirst = (fTriangle) fTs.get(my);
             if (fromFR<maxFromFR)
               FRFirst = (fTriangle) FR.fTs.get(fromFR);
           }
   } // while
} // if 

// elements from one (or both) regions are processed

while(my < maxMy){    // this have still elements
   processElements(myFirst,SF,null,FR.SF,result,op);
   my++;
   if (my<maxMy)
      myFirst = (fTriangle) fTs.get(my);
}

while (fromFR < maxFromFR){  // FR have still elements
   processElements(null,SF,FRFirst,FR.SF,result,op);
   fromFR++;
   if(fromFR<maxFromFR)
      FRFirst = (fTriangle) FR.fTs.get(fromFR);
}
  result.computeBoundingBox();
  return result;
}

/** normalize this FRegion */
private void norm(){

 if (isEmpty()) return;  // nothing to do

 // first compute Zmin and Zmax
 double Zmin = 0;
 double Zmax = 0;
 fTriangle Current;

 for (int i=0; i< fTs.getSize();i++){
   Current = (fTriangle) fTs.get(i);
   if(Current.getMaxZ()>Zmax)
      Zmax = Current.getMaxZ();
   if(Current.getMinZ()<Zmin)
      Zmin = Current.getMinZ();
 }

 if(Zmin > 0) Zmin=0;
 
 if(Zmax==0 & Zmin==0)
    fTs.makeEmpty();
 else{
    double SFnew = Zmax - Zmin;
    SortedObjects newfTs= new SortedObjects();
    double Z1,Z2,Z3;
    fTriangle fTnew;

    for(int i=0;i<fTs.getSize();i++){
       Current = (fTriangle) fTs.get(i);
       Z1 = Current.getZ1();
       Z2 = Current.getZ2();
       Z3 = Current.getZ3();
       fTnew = new fTriangle( (BasicTriangle) Current.basic(),
                             (Z1-Zmin)/SFnew ,
                             (Z2-Zmin)/SFnew,
                             (Z3-Zmin)/SFnew );
       if (fTnew.getMaxZ() >0)
          newfTs.insert(fTnew);
   } // for
   SF = SFnew*SF;
   fTs = newfTs;
 }
 computeBoundingBox();
}


/** check wether this FRegions contains BP */
public boolean contains(BasicPoint BP){
 return numberOfTriangles(BP)>0;
}

/** check wether this FRegion contains BS */
public boolean contains(BasicSegment BS){
 return numberOfTriangles(BS)>0;
}

/** check wether this FRegion contains BT */
public boolean contains(BasicTriangle BT){
  return fTs.search(BT)!=null;
}

/*************************************************************************
 *                                                                       *
 *                   Topology of the  basic                              *
 *                                                                       *
 *************************************************************************/



/** this method is helping for basicTopolRelation */
private static void processTriangle(BasicTriangle T,FRegion R1,FRegion R2,
                                    M9Int goal){
BasicPoint[] BP = new BasicPoint[3];
boolean onR1,onR2,onBoundary1,onBoundary2;

   // process points of minimum Triangle
   BP[0] = T.getCP1();
   BP[1] = T.getCP2();
   BP[2] = T.getCP3();

   for(int i=0;i<3;i++){
     onBoundary1 = R1.onBoundary(BP[i]);
     onBoundary2 = R2.onBoundary(BP[i]);
     onR1 = R1.contains(BP[i]);
     onR2 = R2.contains(BP[i]);

     if(onR1 & onR2){  // else is nothing to do
        if(onBoundary1)
           if(onBoundary2)
              goal.setValue(true,M9Int.BOUNDARY,M9Int.BOUNDARY);
           else
              goal.setValue(true,M9Int.BOUNDARY,M9Int.INTERIOR);
        else
           if(onBoundary2)
              goal.setValue(true,M9Int.INTERIOR,M9Int.BOUNDARY);
           else
              goal.setValue(true,M9Int.INTERIOR,M9Int.INTERIOR);
      }  // if common Point
   } // for
} // processTriangle



/** computes the 9-intersection-matrix for 2 fregions */
M9Int basicTopolRelation(FRegion R2){

 M9Int result = new M9Int();
 result.setValue(true,M9Int.EXTERIOR,M9Int.EXTERIOR); // this holds ever

 boolean ready=false; // all checked intersections are true
 int currentThis=0;
 int currentR2 = 0;
 int maxThis = fTs.getSize();
 int maxR2 = R2.fTs.getSize();
 int compareResult;
 fTriangle C1,C2;
 BasicTriangle MinBasic;
 fTriangle minT=null;
 BasicPoint[] BP = new BasicPoint[3];
 boolean onBoundary1,onBoundary2,
         onThis,onR2;

while( (currentThis<maxThis)  & (currentR2<maxR2) & !ready) {

   C1 = (fTriangle) fTs.get(currentThis);
   C2 = (fTriangle) R2.fTs.get(currentR2);

   compareResult=(C1.basic().compareTo(C2.basic()));

   if(compareResult<0){
      result.setValue(true,M9Int.INTERIOR,M9Int.EXTERIOR);
      currentThis++;
      minT=C1;
   }
   if(compareResult>0){
      result.setValue(true,M9Int.EXTERIOR,M9Int.INTERIOR);
      currentR2++;
      minT=C2;
   }
   if(compareResult==0){
     currentThis++;
     currentR2++;
     minT=C1;
     result.setValue(true,M9Int.INTERIOR,M9Int.INTERIOR);
   }
   processTriangle( (BasicTriangle) minT.basic(),this,R2,result);

}  // while


if( currentThis<maxThis){
  result.setValue(true,M9Int.INTERIOR,M9Int.EXTERIOR);
  while(currentThis<maxThis){
    minT = (fTriangle)fTs.get(currentThis);
    MinBasic = (BasicTriangle) minT.basic();
    currentThis++;
    processTriangle(MinBasic,this,R2,result);
  }
}

if( currentR2<maxR2){
  result.setValue(true,M9Int.EXTERIOR,M9Int.INTERIOR);
  while(currentR2<maxR2){
    minT = (fTriangle)R2.fTs.get(currentR2);
    MinBasic = (BasicTriangle) minT.basic();
    currentR2++;
    processTriangle(MinBasic,this,R2,result);
  } // while
} // if

return result;
}


/** computes the topology for this region with another fuzzy object */
public M9Int basicTopolRelation(CompositeObject CO){

M9Int result;
if( (CO instanceof FPoint) | (CO instanceof FLine)) {
   result = CO.basicTopolRelation(this);
   result.makeSym();
   return result;
}
if( CO instanceof FRegion)
   return basicTopolRelation( (FRegion) CO);
return null;
}


/*****************************************************************
 *                                                               *
 *           topology on fuzzy objects                           *
 *                                                               *
 *****************************************************************/

/**
  * this function get membershipvalues and a cut-position
  * for a given BasicSegment 
  * <br><ul>
  *       <li> result[0] : the membership-value for the first endpoint of
  *                         the BasicSegment (maximum of all fSegments
  *                         with the same basic as BS </li>
  *        <li> result[1] : the same for the second endpoint  </li>
  *        <li> if the length of the result greater then 2 then
  *             exists two crossing fSgements in this region with same
  *             basic as BS.
  *             <ul>
  *                <li> result[2] where is the cross point </li>
  *                <li> result[3] the membershipvalue over the cross
  *                     point </li>
  *            </ul>
  *        </li>
  *      </ul>
  */
double[] getValues(BasicSegment BS){

double[] result;
BasicTriangle[] BTs = BS.getTriangles(); // exact 2 Triangles
fTriangle FT1 = (fTriangle) fTs.search(BTs[0]);
fTriangle FT2 = (fTriangle) fTs.search(BTs[1]);

if( FT1==null || FT2==null){
  result = new double[2];
  if(FT1==null){
    if(FT2==null){
      result[0] = 0;
      result[1] = 0;
    }
    else{  //FT2!=null
      result[0] = FT2.Zfkt(BS.getEP1());
      result[1] = FT2.Zfkt(BS.getEP2());
    }
  }
  else{  // FT1!=null
    result[0] = FT1.Zfkt(BS.getEP1());
    result[1] = FT1.Zfkt(BS.getEP2());
  }
}
else{  // both fTriangles exists
  double Z1_1 = FT1.Zfkt(BS.getEP1());
  double Z1_2 = FT1.Zfkt(BS.getEP2());
  double Z2_1 = FT2.Zfkt(BS.getEP1());
  double Z2_2 = FT2.Zfkt(BS.getEP2());
  if( (Z1_1-Z2_1) * (Z1_2-Z2_2)>=0) {   // no crossing
    result = new double[2];
    result[0] = Math.max(Z1_1,Z2_1);
    result[1] = Math.max(Z1_2,Z2_2);
  }
  else{ // the segments are crossing
    result = new double[4];
    result[0] = Math.max(Z1_1,Z2_1);
    result[1] = Math.max(Z1_2,Z2_2);
    // compute the cross-position
    double pos = (Z2_1-Z1_1) / (Z1_2-Z1_1-Z2_2+Z2_1);
    double value = Z1_1 + pos*(Z1_2-Z1_1);
    result[2] = pos;
    result[3] = value;
  }
} // else 
return result;
}

/**
  * returns the max value from both segments on given pos
  * pos must be in [0,1]
  */
double getMaxValue(BasicSegment BS ,double pos){

if( (pos<0) || (pos >1))
  return 0;

BasicTriangle[] BTs = BS.getTriangles(); // exact 2 Triangles
fTriangle FT1 = (fTriangle) fTs.search(BTs[0]);
fTriangle FT2 = (fTriangle) fTs.search(BTs[1]);
double Z1_1,Z1_2,Z2_1,Z2_2; // values on the endpoints
double V1,V2;               //values on given Position
if(FT1==null){
   Z1_1=0;
   Z1_2=0;
}
else{
   Z1_1 = FT1.Zfkt(BS.getEP1());
   Z1_2 = FT1.Zfkt(BS.getEP2());
}
V1 = Z1_1 + pos*(Z1_2-Z1_1);

if(FT2==null){
  Z2_1=0;
  Z2_2=0;
}
else{
   Z2_1 = FT2.Zfkt(BS.getEP1());
   Z2_2 = FT2.Zfkt(BS.getEP2());
}
V2 = Z2_1 + pos*(Z2_2-Z2_1);
return Math.max(V1,V2);
}




/** computes the topological relationship between this and R2 */
public FuzzyTopRel topolRelation(FRegion R2){

FuzzyTopRel result = new FuzzyTopRel();

fTriangle     Current;
BasicTriangle CurrentBasic;
BasicPoint[]    Corners;
BasicSegment[]  Sides;
fTriangle     FT2;

for(int i=0;i<fTs.getSize();i++){
  Current = (fTriangle) fTs.get(i);
  CurrentBasic = (BasicTriangle) Current.basic();
  Corners = CurrentBasic.getBasicPoints();
  Sides   = CurrentBasic.getSides();
  double Z1_1,Z1_2,Z1_3;
  double Z2_1,Z2_2,Z2_3;
  double M1,M2;
  double VM1_1,VM1_2,VM2_1,VM2_2;
  double[] values1;
  double[] values2;

  // first  process CornerPoints
  for(int cp=0;cp<Corners.length;cp++){
     if(R2.contains(Corners[cp])){
        Z1_1 = maxZfkt(Corners[cp]);
        Z2_1 = R2.maxZfkt(Corners[cp]);
        result.computeValue(Z1_1,Z2_1);
     }
  } // process cornerpoints

  // second process Sides
  for(int s=0;s<Sides.length;s++){
     if(R2.contains(Sides[s])){
        values1 = getValues(Sides[s]);
        values2 = R2.getValues(Sides[s]);
        Z1_1 = values1[0];
        Z1_2 = values1[1];
        Z2_1 = values2[0];
        Z2_2 = values2[1];
        if(Z1_1!=Z2_1)
           result.computeValue(Z1_1,Z2_1);
        if(Z1_2!=Z2_2)
           result.computeValue(Z1_1,Z2_1);

        if( (values1.length==2) & (values2.length==2)) {
           if( (Z1_1==Z2_1) & (Z1_2==Z2_2)) // equals segments
              result.computeValue(0.5,0.5);
           if( (Z1_1-Z2_1)*(Z1_2-Z2_2)<0)  // crossing segments
              result.computeValue(0.5,0.5);
        }  // in both region are nor crossing segments
        else if( values1.length==2){
           M2 = values2[2];
           VM2_2 = values2[3];
           VM1_2 = getMaxValue(Sides[s],M2);
           result.computeValue(VM1_2,VM2_2);
           // explore crossing segment parts
           if( (Z1_1-Z2_1)*(VM1_2-VM2_2) <0)
              result.computeValue(0.5,0.5);
           if( (VM1_2-VM2_2)*(Z1_2-Z2_2) <0)
              result.computeValue(0.5,0.5);
        }
        else if( values2.length==2){
           M1 = values1[2];
           VM1_1 = values1[3];
           VM2_1 = R2.getMaxValue(Sides[s],M1);
           result.computeValue(VM1_1,VM2_1);
           // crossing segments
           if( (Z1_1-Z2_1)*(VM1_1-VM2_1)<0)
              result.computeValue(0.5,0.5);
           if( (VM1_1-VM2_1)*(Z1_2-Z2_2)<0)
              result.computeValue(0.5,0.5);
        }
        else { // both region having crossing segments

           if(values1[2]<=values2[2]) {
             M1    = values1[2];
             M2    = values2[2];
             VM1_1 = values1[3];
             VM1_2 = getMaxValue(Sides[s],M2);
             VM2_1 = R2.getMaxValue(Sides[s],M1);
             VM2_2 = values2[3];
           }
           else {
             M1 = values2[2];
             M2 = values1[2];
             VM1_1 = getMaxValue(Sides[s],M1);
             VM1_2 = values1[3];
             VM2_1 = values2[3];
             VM2_2 = R2.getMaxValue(Sides[s],M2);
           }
           result.computeValue(VM1_1,VM2_1);
           result.computeValue(VM1_2,VM2_2);

           // explore crosses
           if( (Z1_1-Z2_1)*(VM1_1-VM2_1) <0)
              result.computeValue(0.5,0.5);
           if( (M1!=M2) && ( (VM1_1-VM2_1)*(VM1_2-VM2_2)<0) )
              result.computeValue(0.5,0.5);
           if( (VM1_2-VM2_2)*(Z1_2-Z2_2)<0)
              result.computeValue(0.5,0.5);

        } // else (both region have crossing segments)

     }  // if contains segment
  } // process sides

  // process inner triangles
  FT2 = (fTriangle) R2.fTs.search(CurrentBasic);
  if(FT2!=null){
     Z1_1 = Current.getZ1();
     Z1_2 = Current.getZ2();
     Z1_3 = Current.getZ3();
     Z2_1 = FT2.getZ1();
     Z2_2 = FT2.getZ2();
     Z2_3 = FT2.getZ3();

     if( (Z1_1<Z2_1 & Z1_2<Z2_2 & Z1_3<Z2_3) |    // all under or
         (Z1_1>Z2_1 & Z1_2>Z2_2 & Z1_3>Z2_3)  )   // all over
        result.computeValue(Z1_1,Z2_1);
     else
        if(Z1_1==Z2_1 & Z1_2==Z2_2 & Z1_3==Z2_3)
          result.computeValue(0.5,0.5);
        else { // the triangles are crossing; this covers all cases
          result.computeValue(0.3,0.7);
          result.computeValue(0.7,0.3);
          result.computeValue(0.5,0.5);
        }
  }

} // for all containing TRiangles

return result;

} // topolRelation



/** compute the topological relationship between this and CO */
public FuzzyTopRel topolRelation(CompositeObject CO){
 FuzzyTopRel result;
 if(CO instanceof FPoint){
    result = ((FPoint)CO).topolRelation(this);
    result.makeSym();
    return result;
 }
 if(CO instanceof FLine){
    result = ((FLine)CO).topolRelation(this);
    result.makeSym();
    return result;
 }
 if(CO instanceof FRegion){
    return topolRelation((FRegion) CO);
 }

 return null;
}




/** returns the ListExpr representation of this FRegion
  * (SF , (<TriangleList>))
  */
public ListExpr toListExpr(){
  // first create the SegmentList
  ListExpr Triangles;
  ListExpr Last=null;
  if(fTs.getSize()==0)
     Triangles = ListExpr.theEmptyList();
  else {
     Triangles = ListExpr.oneElemList(((fTriangle)fTs.get(0)).toListExpr()); 
     Last = Triangles;
  }
  fTriangle NextTriangle; 
  for(int i=1;i<fTs.getSize();i++){
     NextTriangle = (fTriangle) fTs.get(i);
     Last=ListExpr.append(Last,NextTriangle.toListExpr());
  }
  return ListExpr.twoElemList( ListExpr.realAtom((float)SF),Triangles);
}

/** creates a new ListEXpr <type,value>*/
public ListExpr toTypedListExpr(){
  return ListExpr.twoElemList( ListExpr.symbolAtom("fregion"),toListExpr());
}


/** read this FRegion from a ListExpr
  * @return true if LE is a valid Representaion of a FRegion
  * all valid Triangles of this List are inserted
  */
public boolean readFromListExpr(ListExpr LE){
  SF = 1.0;
  fTs.makeEmpty();
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


  ListExpr Triangles = LE.second();
  fTriangle T;
  boolean ok = true; 
  while( !Triangles.isEmpty() & ok) {
    T = new fTriangle(0,0,0,0,0,0,0,0,0);
    if(T.readFromListExpr(Triangles.first())){
       add(T);
       Triangles=Triangles.rest();
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


/** read the FRegion from a String representation of a ListExpr
  * @return true if List is a String of a ListExpr containing
  * a correct FRegion
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

/** this method is used for reading a fuzzy region from a byte array;
  * returns null if the construction of the object failed
  */
public static FRegion readFrom(byte[] buffer){
   try{
      ObjectInputStream ois;
      ois = new ObjectInputStream(new ByteArrayInputStream(buffer));
      FRegion res = (FRegion) ois.readObject();
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
     byteout = new ByteArrayOutputStream(fTs.getSize()*16+25);
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





// define constants for the operators
private static final int UNION = 0;             // union based on max
private static final int INTERSECTION=1;   // difference based on min
private static final int ADD=2;                 // addition with cut if >1
private static final int SUBTRACT=3;            // substraction with cut if<0

private static final int SCALEDUNION=4;
private static final int SCALEDINTERSECTION=5;
private static final int SCALEDADD=6;
private static final int SCALEDDIFFERENCE=7;

} // FRegion;

