package bbox;
import java.io.*;

public class Point implements Serializable{

/** creates  a new Point instance at the given position */
public Point(double x, double y){
  this.x=x;
  this.y=y;
}

/** checks for equality with o */
public boolean equals(Object o){
  if(!(o instanceof Point))
     return false;
  Point P = (Point)o;
  return P.x==x && P.y==y;
}

/** return the x location of this point */
public double getX(){ return x;}

/** returns the y locataion of this Point */
public double getY(){ return y;}

/** returns a copy of this point */
public Point copy(){
   return new Point(x,y);
}

/** compares this with P */
public int compareTo(Point P){
   if(x<P.x)
      return -1;
   if(x>P.x)
      return 1;
   if(y<P.y)
      return -1;
   if(y>P.y)
       return 1;
   return 0;
}

/** reads this Point from its byte representation */
public static Point readFrom(byte[] buffer){
   try{
      ObjectInputStream ois;
      ois = new ObjectInputStream(new ByteArrayInputStream(buffer));
      Point res = (Point) ois.readObject();
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
  return (int) Math.abs(x*7+y);
}

private double x;
private double y;

}
