/*
//paragraph [1] title: [{\Large \bf ]	[}]

[1] JNIExample Algebra -- PointJNITest.java

Janary, 28th, 2003 Mirco G[ue]nster and Ismail Zerrad.

This class which represents a PointJNITest in a two-dimensional
space is used in the JNIExample Algebra.

*/

import java.io.*;

class PointJNITest {

    //members
    public RationalJNITest x; //x coordinate
    public RationalJNITest y; //y coordinate
    
    //constructors
    public PointJNITest() {
	//fill in dummy values
	this.x = new RationalJNITest(0);
	this.y = new RationalJNITest(0);
    }
    
    public PointJNITest(double x, double y) {
	this.x = new RationalJNITest(x);
	this.y = new RationalJNITest(y);
    }

    public PointJNITest(RationalJNITest x, RationalJNITest y) {
	this.x = x.copy();
	this.y = y.copy();
    }
    
    //methods
    public PointJNITest set(double x, double y) {
	//sets coordinates to x,y
	this.x = new RationalJNITest(x);
	this.y = new RationalJNITest(y);
	return this;
    }//end method set

    public PointJNITest set(RationalJNITest x, RationalJNITest y) {
	//sets coordinates to x,y
	this.x = x.copy();
	this.y = y.copy();
	return this;
    }//end method set
    
    public boolean equal(PointJNITest e) {
	//returns true if the coordinates of p are equal to the coordinates of this
	PointJNITest p = (PointJNITest)e;
	if ((this.x.equal(p.x)) && (this.y.equal(p.y))) { return true; }
	else { return false; }
    }//end method equal
    
    public byte compY(PointJNITest e) {
	//compares the y-coordinates of the topmost points of the
	//given object and THIS.object and returns
	//-1, if THIS.object has a smaller y-coordinate
	//0, if the y-coordinates are equal
	//+1, if THIS.object has a greater y-coordinate
	PointJNITest p = (PointJNITest)e;
	if (this.y.less(p.y)) { return -1; }
	if (this.y.equal(p.y)) { return 0; }
	if (this.y.greater(p.y)) { return 1; }
	System.out.println("Error(class PointJNITest) ...");
	return 0;
    }//end method compY
    
    public byte compX(PointJNITest e) {
	//compares the x-coordinates of the leftmost points of the
	//given object and THIS.object and returns
	//-1, if THIS.object has a smaller x-coordinate
	//0, if the x-coordinates are equal
	//+1, if THIS.object has a greater x-coordinate
	PointJNITest p = (PointJNITest)e;
	if (this.x.less(p.x)) { return -1; }
	if (this.x.equal(p.x)) { return 0; }
	if (this.x.greater(p.x)) { return 1; }
	System.out.println("Error in class PointJNITest!");
	return 0;
    }//end method compX
    
    public void print() {
	//prints the object's data
	System.out.println("PointJNITest: ("+this.x.toString()+", "+this.y.toString()+")");
    }//end method print

    public RationalJNITest dist(PointJNITest p) {
	//returns the distance between this.object and e
	//caution: there is a problem with length, we use lengthD
	//instead which uses Doubles
	PointJNITest diff = new PointJNITest(x.minus(p.x), y.minus(p.y));
	// diff contains the difference vector of both points now.
	double len = 
	    Math.sqrt(diff.x.times(diff.x).getDouble() 
		      + diff.y.times(diff.y).getDouble());
	return new RationalJNITest(len);
    }//end method dist
    
}//end class PointJNITest
