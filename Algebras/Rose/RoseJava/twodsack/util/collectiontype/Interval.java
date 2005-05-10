package twodsack.util.collectiontype;

import twodsack.setelement.*;
import twodsack.util.number.*;

public class Interval {
  //supportive class for class SetOps
  //'right' has to be greater than or equal to 'left'
  
  //members
    public Rational left;
    public Rational right;
    public String mark;
    public Rational x;
    public Element ref;
    public int number;
    public boolean buddyOnSameX;    

    //constructors
    Interval() {
	//this.left = RationalFactory.constRational(0);
	//this.right = RationalFactory.constRational(0);
	//this.mark = "";
	//this.x = RationalFactory.constRational(0);
	//this.number = -1;
	//this.reftri = new Triangle();
    }
    
    public Interval(Rational l, Rational r, String m, Rational x, Element e, int n, boolean buddy) {
	//if (l.lessOrEqual(r)) {
	this.left = l;
	this.right = r;
	this.mark = m;
	this.x = x;
	this.ref = e;
	this.number = n;
	this.buddyOnSameX = buddy;
	//}//if
	//else { System.out.println("Error: Couldn't create interval."); }
    }//end constructor
    
    //methods
    /* IS NOT NEEDED CURRENTLY
      Interval copy () {
      //returns a copy of this.object
      return (new Interval(this.left.copy(),this.right.copy(),this.mark,this.x.copy(),(Element)this.ref,this.number));
      }//end constructor
    */
    
    public boolean equal(Interval in) {
	//returns true if this.object and 'in' are equal
	//false else
	//attention: checks only for the interval an NOT for other data
	
	if ((this.left.equal(in.left)) &&
	    (this.right.equal(in.right))) {
	    return true;
	}//if
	else { return false; }
    }//end method equal

    
    public boolean equalX(Interval in) {
	//returns true if both object's interval borders AND their
	//x-coordinate are equal
	//false else
	if (this.left.equal(in.left) &&
	    this.right.equal(in.right) &&
	    this.x.equal(in.x)) return true;
	else return false;
    }//end method equalX    


    public void print() {
	//prints the object's data
	System.out.print("Interval:");
	System.out.print(" down/left: "+this.left.toString());
	System.out.print(", top/right: "+this.right.toString());
	System.out.print(", mark: "+this.mark);
	System.out.print(", x: "+this.x.toString());
	System.out.print(", buddy: "+this.buddyOnSameX);
	System.out.println(", number: "+this.number);
	//System.out.println(" ref: ");
	//this.ref.print();
    }//end method print
  

    public byte comp(Interval in) {
	//returns 0 if this and in are equal
	//returns -1 if x of this is smaller
	//returns +1 if x of this is greater
	//if x is equal, left, right are compared in this order
	//if all coordinates are equal, sort by number
	byte res = this.x.comp(in.x);
	if (res != 0) return res;

	res = this.left.comp(in.left);
	if (res != 0) return res;

	res = this.right.comp(in.right);
	if (res != 0) return res;

	if (this.number < in.number) return -1;
	else return 1;
    }//end method comp

    public byte compY (Interval in) {
	byte res = this.left.comp(in.left);
	if (res != 0) return res;
	
	res = this.right.comp(in.right);
	if (res != 0) return res;
	
	if (this.number < in.number) return -1;
	else return 1;
    }//end method compY

}//end class Interval
