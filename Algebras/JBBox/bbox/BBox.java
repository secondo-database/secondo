package bbox;
import java.io.*;

public class BBox implements Serializable{

/** creates  a new Point instance at the given position */
public BBox(){
  isempty=true;
  minX=maxX=minY=maxY=0.0;
}

public BBox(double minX, double maxX, double minY, double maxY){
   isempty=false;
   this.minX=Math.min(minX,maxX);
   this.minY=Math.min(minY,maxY);
   this.maxX=Math.max(maxX,minX);
   this.maxY=Math.max(maxY,minY);
}

/** check whether this box is empty */
public boolean isEmpty(){
   return isempty;
}


/** checks for equality with o */
public boolean equals(Object o){
  if(!(o instanceof BBox))
     return false;
  BBox B = (BBox)o;
  if(isempty!=B.isempty)
     return false;
  if(isempty) // both boxes are empty
     return true;
  return minX==B.minX && maxX==B.maxX &&
         minY==B.minY && maxY==B.maxY;
}

/** returns a copy of this BBox */
public BBox copy(){
   if(isempty)
      return new BBox();
   else
      return new BBox(minX,maxX,minY,maxY);
}

/** compares this with B */
public int compareTo(BBox B){
   if(isempty  && isempty)  return 0;
   if(isempty  && !B.isempty)   return -1;
   if(!isempty && B.isempty)   return 1;
   if(minX<B.minX)             return -1;
   if(minX>B.minX)             return 1;
   if(maxX<B.maxX)             return -1;
   if(maxX>B.maxX)             return 1;
   if(minY<B.minY)             return -1;
   if(minY>B.minY)             return 1;
   if(maxY<B.maxY)             return -1;
   if(maxY>B.maxY)             return 1;
   return 0;
}

/** checks wether P is contained in this box */
public boolean contains(Point P){
   double x = P.getX();
   double y = P.getY();
   return x>=minX && x<=maxX && y>=minY && y<=maxY;
}

/** computes the union of this Bbox with B */
public BBox union(BBox B){
   if(isempty && B.isempty)
      return new BBox();
   if(isempty)
      return B.copy();
   if(B.isempty)
      return copy();
   BBox res = new BBox();
   res.minX = Math.min(minX,B.minX);
   res.maxX = Math.max(maxX,B.maxX);
   res.minY = Math.min(minY,B.minY);
   res.maxY = Math.max(maxY,B.maxY);
   res.isempty=false;
   return res;
}

/** includes P in this bbox */
public BBox union(Point P){
  double x = P.getX();
  double y = P.getY();
  BBox res = new BBox();
  if(isempty){
    res.minX=res.maxX=x;
    res.minY=res.maxY=y;
    res.isempty=false;
  }else{
    res.minX=Math.min(minX,x);
    res.maxX=Math.max(maxX,x);
    res.minY=Math.min(minY,y);
    res.maxY=Math.max(maxY,y);
    res.isempty=false;
  }
  return res;
}

/** returns the size of this BBox */
public double size(){
   if(isempty) return -1;
   else return (maxX-minX)*(maxY-minY);
}

/** returns the intersection of this */
public BBox intersection(BBox B){
   if(isempty || B.isempty)
      return new BBox();
   BBox res = new BBox();
   res.minX = Math.max(minX,B.minX);
   res.minY = Math.max(minY,B.minY);
   res.maxX = Math.min(maxX,B.maxX);
   res.maxY = Math.min(maxY,B.maxY);
   res.isempty = (minX>maxY || minY>maxY);
   return res;
}

/** checks wether this and B2 share any point */
public boolean intersects(BBox B2){
   if(isempty || B2.isempty) return false;
   if(B2.minX > maxX) return false; // B2 is right
   if(B2.minY > maxY) return false; // B2 is above
   if(B2.maxX < minX) return false; // B2 is left
   if(B2.maxY < minY) return false; // B2 is under
   return true;
}


/** reads this BBox from its byte representation */
public static BBox readFrom(byte[] buffer){
   try{
      ObjectInputStream ois;
      ois = new ObjectInputStream(new ByteArrayInputStream(buffer));
      BBox res = (BBox) ois.readObject();
      ois.close();
      return res;
   } catch(Exception e){
         return null;
     }
}

/** writes this Point to its byte representation */
public  byte[] writeToByteArray(){
  try{
     ByteArrayOutputStream byteout = new ByteArrayOutputStream();
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


public int getHashValue(){
   return (int) Math.abs((maxX-minX)*7+(maxY-minY));
}

/** write this BBox to the standard output */
public void write(){
  if(isempty)
     System.out.print("[empty box]");
  else
     System.out.print("[("+minX+","+minY+")->("+maxX+","+maxY+")]");
}

private boolean isempty;
private double minX;
private double maxX;
private double minY;
private double maxY;

}
