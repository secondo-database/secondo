package extern.shapereader;

import sj.lang.*;
import java.util.Vector;
import extern.numericreader.*;


// todo-list
// - insert conversions for dangling shapetypes
//     - describe the external representation of such objects


public class Record{


public boolean readFrom(byte[] Buffer){
  LE =null;
  if(Buffer.length<4)
     return false;
  type = NumericReader.getIntLittle(Buffer,0);
  if(type==0)
      return readNullShape(Buffer);
  if(type==1)
      return readPoint(Buffer);
  if(type==8)
      return readPoints(Buffer);
  if(type==3)
      return readPolyLine(Buffer);
  if(type==5)
      return readPolygon(Buffer);
  return false;
}


public int getType(){
   return type;
}

public ListExpr getValueList(){
    return LE;
}

public BoundingBox getBoundingBox(){
   return BBox;
}


private boolean readNullShape(byte[] Buffer){
  LE = ListExpr.theEmptyList();
  BBox = new BoundingBox(0,0,0,0);
  return true;
}

private boolean readPoint(byte[] Buffer){
   if(Buffer.length<20)
      return false;
   double x = NumericReader.getDoubleLittle(Buffer,4);
   double y = NumericReader.getDoubleLittle(Buffer,12);
   BBox = new BoundingBox(x,y,x,y);
   LE = ListExpr.twoElemList( ListExpr.realAtom((float)x),
                              ListExpr.realAtom((float)y));
   return true;
}


private boolean readPoints(byte[] Buffer){
   if(Buffer.length<40)
       return false;
   double xmin = NumericReader.getDoubleLittle(Buffer,4);
   double ymin = NumericReader.getDoubleLittle(Buffer,12);
   double xmax = NumericReader.getDoubleLittle(Buffer,20);
   double ymax = NumericReader.getDoubleLittle(Buffer,28);
   int numpoints = NumericReader.getIntLittle(Buffer,36);
   if(Buffer.length-40 < 8*numpoints){
       System.err.println("wrong numpoints in Multipoint");
       numpoints = (Buffer.length-40) /8;
   }
   BBox = new BoundingBox(xmin,ymin,xmax,ymax);
   if(numpoints==0){
      LE = ListExpr.theEmptyList();
      return true;
   }
   double x = NumericReader.getDoubleLittle(Buffer,40);
   double y = NumericReader.getDoubleLittle(Buffer,48);
   LE = ListExpr.oneElemList( ListExpr.twoElemList(
                ListExpr.realAtom((float)x),
                ListExpr.realAtom((float)y)));
   ListExpr Last = LE;
   for(int i=1;i<numpoints;i++){
      x = NumericReader.getDoubleLittle(Buffer,40+16*i);
      y = NumericReader.getDoubleLittle(Buffer,48+16*i);
      ListExpr P = ListExpr.twoElemList( ListExpr.realAtom((float)x),
                                         ListExpr.realAtom((float)y));
     Last = ListExpr.append(Last,P);
   }
   return true;
}



private boolean readPolyLine(byte[] Buffer){
  if(Buffer.length<44)
      return false;
  double xmin = NumericReader.getDoubleLittle(Buffer,4);
  double ymin = NumericReader.getDoubleLittle(Buffer,12);
  double xmax = NumericReader.getDoubleLittle(Buffer,20);
  double ymax = NumericReader.getDoubleLittle(Buffer,28);
  int NumParts = NumericReader.getIntLittle(Buffer,36);
  int NumPoints = NumericReader.getIntLittle(Buffer,40);
  int[] Parts = new int[NumParts];
  for(int i=0;i<NumParts;i++)
     Parts[i] = NumericReader.getIntLittle(Buffer,44+4*i);
  double[][] Points = new double[2][NumPoints];
  for(int i=0;i<NumPoints;i++){
      Points[0][i] = NumericReader.getDoubleLittle(Buffer,44+4*NumParts+16*i);
      Points[1][i] = NumericReader.getDoubleLittle(Buffer,52+4*NumParts+16*i);
  }
 BBox = new BoundingBox(xmin,ymin,xmax,ymax);


 ListExpr TMP =null;
 ListExpr Last = null;
 int PartNum = 0;
 for(int i=0;i<NumPoints-1;i++){
       if(i+1==Parts[PartNum])
          PartNum++;
       else{
          double x1 = Points[0][i];
          double y1 = Points[1][i];
          double x2 = Points[0][i+1];
          double y2 = Points[1][i+1];
          if(x1!=x2 | y1!=y2){
             ListExpr Seg = ListExpr.fourElemList(ListExpr.realAtom((float)x1),
	                                          ListExpr.realAtom((float)y1),
					          ListExpr.realAtom((float)x2),
					          ListExpr.realAtom((float)y2));
             if(TMP==null){
   	        TMP = ListExpr.oneElemList(Seg);
                Last = TMP;
	     }else{
	        Last = ListExpr.append(Last,Seg);
	     }

          }
       }
    }

  LE = TMP;
  return true;
}



/** returns true if Hole is contained in Face
  * it is only checked if the first point in Hole is
  * contained in Face because we going out from a
  * valid shapefile. A valid shapefile has no intersecting cycles
  */
private static boolean isHoleFrom(Cycle2D Hole,Cycle2D Face){
  if(Hole==null | Face==null)
     return false;
  double x = Hole.getFirstX();
  double y = Hole.getFirstY();
  return Face.contains(x,y);
}



private boolean readPolygon(byte[] Buffer){
  if(Buffer.length<44)
      return false;
  double xmin = NumericReader.getDoubleLittle(Buffer,4);
  double ymin = NumericReader.getDoubleLittle(Buffer,12);
  double xmax = NumericReader.getDoubleLittle(Buffer,20);
  double ymax = NumericReader.getDoubleLittle(Buffer,28);
  int NumParts = NumericReader.getIntLittle(Buffer,36);
  int NumPoints = NumericReader.getIntLittle(Buffer,40);
  int[] Parts = new int[NumParts+1];
  for(int i=0;i<NumParts;i++)
     Parts[i] = NumericReader.getIntLittle(Buffer,44+4*i);
  Parts[NumParts] = NumPoints; // after the last point
  double[][] Points = new double[2][NumPoints];
  for(int i=0;i<NumPoints;i++){
      Points[0][i] = NumericReader.getDoubleLittle(Buffer,44+4*NumParts+16*i);
      Points[1][i] = NumericReader.getDoubleLittle(Buffer,52+4*NumParts+16*i);
  }
 BBox = new BoundingBox(xmin,ymin,xmax,ymax);

 Vector Faces = new Vector(NumParts);
 Vector Holes = new Vector(NumParts);
 Cycle2D C;
 for(int i=0;i<NumParts;i++){
     C = new Cycle2D();
     for(int k=Parts[i];k<Parts[i+1]-1;k++)
        C.append(Points[0][k],Points[1][k]);
     if(C.isValidPolygon()){
        if(C.getDirection()==Cycle2D.CLOCKWISE)
	   Faces.add(C);
	else
	   Holes.add(C);
     }
 }

 if(Faces.size()==0){
    LE = ListExpr.theEmptyList();
    return true;
 }


 Vector FaceHoles = new Vector(Faces.size());
 int usedHoles = 0;
 int foundHoles = Holes.size();
 for(int i=0;i<Faces.size();i++){
     Vector Hs= new Vector(Holes.size());
     Cycle2D Face = (Cycle2D) Faces.get(i);
     for(int j=0;j<Holes.size();j++){
         Cycle2D Hole = (Cycle2D) Holes.get(j);
         if(isHoleFrom(Hole,Face)){
           if(Hs.size()==0){  // the first hole
              usedHoles++;
	      Hs.add(Hole);
	   }
	   else{    // avoid holes in holes ever take the biggest hole
             int m=0;
	     boolean ok = true;
	     while(m<Hs.size() & ok){
                Cycle2D H2 = (Cycle2D) Hs.get(m);
                if(isHoleFrom(Hole,H2))  // the new hole is contained in a existing hole
		   ok=false;
		else if(isHoleFrom(H2,Hole)){ // a existing hole is contained in the new hole
		   Hs.remove(m);
		   usedHoles--;
		   System.out.println("found hole in another one");
		}
		else  // holes are disjunct
		   m++;
	     }
	     if(ok){
	       usedHoles++;
	       Hs.add(Hole);
	     } else{
	       System.out.println("found hole in another one");
	     }
	   }
	 }
     }
     FaceHoles.add(Hs);
 }

 if(foundHoles!=usedHoles){
   System.out.println("readPolygon : found "+foundHoles+" holes but used "+usedHoles+" holes");
 }



 ListExpr Next = ListExpr.oneElemList( ((Cycle2D)Faces.get(0)).getList());
 ListExpr LastCycle = Next; // add Holes
 Vector Hs = (Vector) FaceHoles.get(0);
 for(int i=0;i<Hs.size();i++){
    Cycle2D H = (Cycle2D)Hs.get(i);
    LastCycle = ListExpr.append(LastCycle,H.getList());
 }


 LE = ListExpr.oneElemList(Next);
 ListExpr Last = LE;
 for(int i=1;i<Faces.size();i++){
     Next = ListExpr.oneElemList(((Cycle2D)Faces.get(i)).getList());  // without holes
     LastCycle = Next;
     Hs = (Vector) FaceHoles.get(i);  // add Holes
     for(int m=0;m<Hs.size();m++){
       Cycle2D H = (Cycle2D)Hs.get(m);
       LastCycle = ListExpr.append(LastCycle,H.getList());
     }
     Last = ListExpr.append(Last,Next);
 }

return true;

 }





private ListExpr LE=null;
private int type = -1;
private BoundingBox BBox;

}
