package viewer.viewer3d.objects;



public class BoundingBox3D{

public BoundingBox3D(){
  reset();
}


public String toString(){
  return "("+minx+","+miny+","+minz+")  --> ("+maxx+","+maxy+","+maxz+")";
}

public boolean set(double minx,double miny,double minz,
           double maxx,double maxy,double maxz){
  if(minx<=maxx && miny<=maxy && minz<=maxz){
    this.minx = minx;
    this.miny = miny;
    this.minz = minz;
    this.maxx = maxx;
    this.maxy = maxy;
    this.maxz = maxz;
    return true;
  }
  else
    return false;
}


public double getCenterX(){
   return (maxx+minx)/2;
}

public double getCenterY(){
   return (maxy+miny)/2;
}

public double getCenterZ(){
   return (maxz+minz)/2;
}


public double getMinX(){ return minx;}
public double getMinY(){ return miny;}
public double getMinZ(){ return minz;}

public double getMaxX(){ return maxx;}
public double getMaxY(){ return maxy;}
public double getMaxZ(){ return maxz;}



public void extend(BoundingBox3D BB){
  this.minx = Math.min(this.minx,BB.minx);
  this.miny = Math.min(this.miny,BB.miny);
  this.minz = Math.min(this.minz,BB.minz);
  this.maxx = Math.max(this.maxx,BB.maxx);
  this.maxy = Math.max(this.maxy,BB.maxy);
  this.maxz = Math.max(this.maxz,BB.maxz);  
}

public void equalize(BoundingBox3D BB){
  this.minx = BB.minx;
  this.miny = BB.miny;
  this.minz = BB.minz;
  this.maxx = BB.maxx;
  this.maxy = BB.maxy;
  this.maxz = BB.maxz;
}



public void reset(){
  minx=miny=minz=0;
  maxx=maxy=maxz=0;
}

private double minx,miny,minz;
private double maxx,maxy,maxz;

}

