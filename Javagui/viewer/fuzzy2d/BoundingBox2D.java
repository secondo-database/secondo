package viewer.fuzzy2d;

import viewer.viewer3d.objects.BoundingBox3D;

public class BoundingBox2D{


/** creates a new 2dim Bounding Box  with given values*/
public BoundingBox2D(double minx,double miny,double maxx,double maxy){
  super();
  setTo(minx,miny,maxx,maxy);
}


/** creates a new BoundingBox2D */
public BoundingBox2D(){
  this(0,0,0,0);
}

/** returns a readable represemntation of the BoundingBox */
public String toString(){
   return "[ ("+minx+","+miny+")->("+maxx+","+maxy+")]";
}


/** set the Values to a projection from BB3*/
public void readFrom(BoundingBox3D BB){
  minx = BB.getMinX();
  miny = BB.getMinY();
  maxx = BB.getMaxX();
  maxy = BB.getMaxY();
}


/** extends this BoundingBox with BB2*/
public void union(BoundingBox2D BB2){
  minx=min(minx,BB2.minx);
  miny=min(miny,BB2.miny);  
  maxx=max(maxx,BB2.maxx);
  maxy=max(maxy,BB2.maxy);
}


/** set the Values of this BoundingBox */
public void setTo(double minx,double miny,double maxx,double maxy){
  this.minx=min(minx,maxx); 
  this.miny=min(miny,maxy);
  this.maxx=max(minx,maxx);
  this.maxy=max(miny,maxy);
}


/** extends this Bounding Box so that (x,y) in inside from this Bounding Box */
public void include(double x , double y){
  minx = min(x,minx);
  maxx = max(x,maxx);
  miny = min(y,miny);
  maxy = max(y,maxy);


}


/** set the values frtom this to the values from BB2 */
public void equalize(BoundingBox2D BB2){
  minx = BB2.minx;
  maxx = BB2.maxx;
  miny = BB2.miny;
  maxy = BB2.maxy;
}


/** returns the width of this Bounding box */
public double getWidth(){ return maxx-minx;}

/** returns thze heigt of this Bounding Box */
public double getHeight(){return maxy-miny;}


public double getMinX(){return minx;}
public double getMinY(){return miny;}
public double getMaxX(){return maxx;}
public double getMaxY(){return maxy;}


/** returns true if this and BB2 have common points */
public boolean intersect(BoundingBox2D BB2){
  if(minx>BB2.maxx) return false; //right from BB2
  if(maxx<BB2.minx) return false; // left from BB2
  if(miny>BB2.maxy) return false; // above BB2
  if(maxy<BB2.miny) return false; // under BB2
  return true;
}


private double min(double x, double y){
  return (x<y)? x : y;
}
private double max(double x, double y){
  return (x>y)? x : y;
}

private double minx,miny;
private double maxx,maxy;

}









