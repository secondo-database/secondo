package viewer.viewer3d.objects;


public class BoundingBox3D{

public BoundingBox3D(){
  reset();
}


public String toString(){
  return "("+minx+","+miny+","+minz+")  --> ("+maxx+","+maxy+","+maxz+")";
}

public boolean set(int minx,int miny,int minz,
           int maxx,int maxy,int maxz){
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

private int minx,miny,minz;
private int maxx,maxy,maxz;

}