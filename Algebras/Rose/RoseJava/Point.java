import java.io.*;

class Point extends Element implements Serializable {
    
    //members
    public Rational x; //x coordinate
    public Rational y; //y coordinate
    //private Rect bbox; //bounding box
    //public String name; //name
    
    //constructors
    public Point() {
	//fill in dummy values
	this.x = RationalFactory.constRational(0);
	this.y = RationalFactory.constRational(0);
	//update();
    }
    
    public Point(double x, double y) {
	this.x = RationalFactory.constRational(x);
	this.y = RationalFactory.constRational(y);
	//update();
    }
    
    public Point(Rational x, Rational y) {
	this.x = x.copy();
	this.y = y.copy();
	//update();
    }
    
    //methods
    public Point set(double x, double y) {
	//sets coordinates to x,y
	this.x = RationalFactory.constRational(x);
	this.y = RationalFactory.constRational(y);
	//update();
	return this;
    }//end method set
    
    
    public Point set(Rational x, Rational y) {
	//sets coordinates to x,y
	this.x = x.copy();
	this.y = y.copy();
	//update();
	return this;
    }//end method set
    

    public Element copy(){
	//returns a copy of the point
	Point copy = new Point(this.x,this.y);
	//copy.update();
	return copy;
    }//end method copy
    
    
    public boolean equal(Element e) throws WrongTypeException {
	//returns true if the coordinates of p are equal to the coordinates of this
	if (e instanceof Point) {
	    Point p = (Point)e;
	    if ((this.x.equal(p.x)) && (this.y.equal(p.y))) { return true; }
	    else { return false; }
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method equal
    

    //public void update(){
	//updates bbox
	//computeBbox();
    //}//end method update
    

    public byte compY(Element e) throws WrongTypeException {
	//compares the y-coordinates of the topmost points of the
	//given object and THIS.object and returns
	//-1, if THIS.object has a smaller y-coordinate
	//0, if the y-coordinates are equal
	//+1, if THIS.object has a greater y-coordinate
	if (e instanceof Point) {
	    Point p = (Point)e;
	    return this.y.comp(p.y);
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method compY
    

    public byte compX(Element e) throws WrongTypeException {
	//compares the x-coordinates of the leftmost points of the
	//given object and THIS.object and returns
	//-1, if THIS.object has a smaller x-coordinate
	//0, if the x-coordinates are equal
	//+1, if THIS.object has a greater x-coordinate
	if (e instanceof Point) {
	    Point p = (Point)e;
	    return this.x.comp(p.x);
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method compX
    
    
    public byte compare(Element e) throws WrongTypeException {
	//...
	if (e instanceof Point) {
	    Point p = (Point)e;
	    byte res = this.x.comp(p.x);
	    if (res == 0) res = this.y.comp(p.y);
	    return res;
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method compare


    public Rect computeBbox() {
	//computes and sets bbox
	return (new Rect(this.x,this.y,this.x,this.y));
    }//end computeBbox
    
    
    public void print() {
	//prints the object's data
	System.out.println("Point: ("+this.x.toString()+", "+this.y.toString()+")");
    }//end method print
    
    
    public boolean intersects(Element e) throws WrongTypeException {
	//returns true if this.object and e intersect
	if (e instanceof Point) {
	    Point p = (Point)e;
	    if ((this.x.equal(p.x)) && (this.y.equal(p.y))) { return true; }
	    else { return false; }
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method intersects
    
    
    public Rect rect() {
	//returns the bounding box of this.object
	return computeBbox();
    }//end method rect

    
    public Rational dist(Element e) throws WrongTypeException {
	//returns the distance between this.object and e
	//caution: there is a problem with length, we use lengthD
	//instead which uses Doubles
	if (e instanceof Point) {
	    Point p = (Point)e;
	    return RationalFactory.constRational(Mathset.lengthD(Mathset.diff(this,p)));
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method dist
    
    
    protected void zoom (Rational fact) {
	//multiplies the coordinates with fact
	this.x = this.x.times(fact);
	this.y = this.y.times(fact);
	//this.update();
    }//end method zoom
    
    
}//end class Point
