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

class SPoint {
    //SPoint is a Point without a bounding box
    //it is used to provide a Point datatype for Rect  
    
    //members
    public Rational x; //x coordinate
    public Rational y; //y coordinate
    //public Rect bbox; //bounding box
    //public String name; //name
    
    //constructors
    public SPoint() {
	//fill in dummy values
	this.x = RationalFactory.constRational(0);
	this.y = RationalFactory.constRational(0);
	//update();
    }
    
    public SPoint(Rational x, Rational y) {
	this.x = x;
	this.y = y;
	//update();
    }
    
    //methods
   
    
    public SPoint copy(){
	//returns a copy of the point
	SPoint copy = new SPoint(this.x.copy(),this.y.copy());
	//copy.update();
	return copy;
    }//end method copy
    
    public boolean equal(SPoint e){
	//returns true if the coordinates of p are equal to the coordinates of this
	if (e instanceof SPoint) {
	    SPoint p = (SPoint)e;
	    if ((this.x.equal(p.x)) && (this.y.equal(p.y))) { return true; }
	    else { return false; }
	}//if
	else System.out.println("Error(class SPoint): Wrong type!");
	System.exit(0);
	return false;
    }//end method equal
    
    public void update(){
	//updates bbox
	//compute_bbox();
    }//end method update
  
    public void print() {
	//prints the object's data
	System.out.println("Point: ("+this.x.toString()+", "+this.y.toString()+")");
    }//end method print
  

}//end class SPoint
