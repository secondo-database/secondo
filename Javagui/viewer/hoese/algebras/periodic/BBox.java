//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package viewer.hoese.algebras.periodic;
import java.awt.geom.*;


public class BBox {

/** creates  a new Point instance at the given position */
public BBox(){
  defined=false;
  minX=maxX=minY=maxY=0.0;
}

public BBox(double minX, double maxX, double minY, double maxY){
   defined=true;
   this.minX=Math.min(minX,maxX);
   this.minY=Math.min(minY,maxY);
   this.maxX=Math.max(maxX,minX);
   this.maxY=Math.max(maxY,minY);
}

/** checks for equality with o */
public boolean equals(Object o){
  if(!(o instanceof BBox))
     return false;
  BBox B = (BBox)o;
  if(defined!=B.defined)
     return false;
  if(!defined)
     return true;
  return minX==B.minX && maxX==B.maxX &&
         minY==B.minY && maxY==B.maxY;
}

/** returns a copy of this BBox */
public BBox copy(){
   if(!defined)
      return new BBox();
   else
      return new BBox(minX,maxX,minY,maxY);
}

/** compares this with B */
public int compareTo(BBox B){
   if(!defined && !B.defined)  return 0;
   if(!defined && B.defined)   return -1;
   if(defined && !B.defined)   return 1;
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
public boolean contains(double x, double y){
   return x>=minX && x<=maxX && y>=minY && y<=maxY;
}

/** computes the union of this Bbox with B */
public BBox union(BBox B){
   if(!defined && !B.defined)
      return new BBox();
   if(!defined)
      return B.copy();
   if(!B.defined)
      return copy();
   BBox res = new BBox();
   res.minX = Math.min(minX,B.minX);
   res.maxX = Math.max(maxX,B.maxX);
   res.minY = Math.min(minY,B.minY);
   res.maxY = Math.max(maxY,B.maxY);
   res.defined=true;
   return res;
}

/** includes P in this bbox */
public BBox union(double x, double y){
  BBox res = new BBox();
  if(!defined){
    res.minX=maxX=x;
    res.minY=maxY=y;
    res.defined=true;
  }else{
    res.minX=Math.min(minX,x);
    res.maxX=Math.max(maxX,x);
    res.minY=Math.min(minY,y);
    res.maxY=Math.max(maxY,y);
    res.defined=true;
  }
  return res;
}

public void unionInternal(double x , double y){
  if(!defined){
    minX=maxX=x;
    minY=maxY=y;
    defined=true;
  }else{
    minX=Math.min(minX,x);
    maxX=Math.max(maxX,x);
    minY=Math.min(minY,y);
    maxY=Math.max(maxY,y);
  }
}

/** returns the size of this BBox */
public double size(){
   if(!defined) return -1;
   else return (maxX-minX)*(maxY-minY);
}

/** returns the intersection of this */
public BBox intersection(BBox B){
   if(!defined || !B.defined)
      return new BBox();
   BBox res = new BBox();
   res.minX = Math.max(minX,B.minX);
   res.minY = Math.max(minY,B.minY);
   res.maxX = Math.min(maxX,B.maxX);
   res.maxY = Math.min(maxY,B.maxY);
   res.defined= !(minX>maxY || minY>maxY);
   return res;
}


public Rectangle2D.Double toRectangle2D(){
   return new Rectangle2D.Double(minX,minY,maxX-minX,maxY-minY);
}

public String toString(){
   if(!defined)
      return "undefined BBox";
   return "[ ("+minX+" , "+minY+" ) -> ( "+maxX+" , "+maxY+" )]";
}

private boolean defined;
private double minX;
private double maxX;
private double minY;
private double maxY;

}
