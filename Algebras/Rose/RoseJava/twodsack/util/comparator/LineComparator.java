package twodsack.util.comparator;

import twodsack.setelement.datatype.*;
import twodsack.util.collectiontype.*;
import twodsack.util.number.*;
import java.util.*;
import java.io.*;

public class LineComparator implements Comparator,Serializable{

    //fields
    public Rational x;
    public Rational yValue;
    public Line line;
    static Rational bp = RationalFactory.constRational(0);
    static Rational bt = RationalFactory.constRational(0);
    static Rational b1 = RationalFactory.constRational(0);
    static Rational b2 = RationalFactory.constRational(0);
    //private Segment sweepLine;

    //constructors
    public LineComparator() {
	this.x = null;
	this.yValue = null;
	this.line = null;
	//this.sweepLine = new Segment();
    }
	
    public int compare(Object ino1, Object ino2) {
	//ino1 and ino2 shall be two Line instances
	//The point where the lines are intersecting the y-axis are computed.
	//Then,
	//-1 is returned, if the intersection point of ino1 and the y-axis is smaller
	//   than the intersection point of ino2 and the y-axis.
	//0  is returned, if both intersection points are equal and
	//1  otherwise
	if(ino1 instanceof Line &&
	   ino2 instanceof Line) {
	    Line l1 = (Line)ino1;
	    Line l2 = (Line)ino2;
	    if (l1.vert || l2.vert) {
		int res = l1.seg.getStartpoint().compY(l2.seg.getStartpoint());
		if (res != 0) return res;
		else if (l1.vert && !l2.vert) return 1;
		else if (l2.vert && !l1.vert) return -1;
		else return l1.seg.getEndpoint().compY(l2.seg.getEndpoint());
	    }//if
	    /*
	      sweepLine.set(new Point(this.x,l1.seg.rect().lly.minus(1)), new Point(this.x,l1.seg.rect().uly.plus(1)));
	      Rational b1 = (sweepLine.intersection(l1.seg)).y;
	    */
	    //Rational b1 = this.yValue;
	    this.b1.assign(evaluateLineFunc(l1));
	    //this.b1 = evaluateLineFunc(l1);
	    /*
	      sweepLine.set(new Point(this.x,l2.seg.rect().lly.minus(1)), new Point(this.x,l2.seg.rect().uly.plus(1)));
	      Rational b2 = (sweepLine.intersection(l2.seg)).y;
	    */
	    this.b2.assign(evaluateLineFunc(l2));
	    //this.b2 = evaluateLineFunc(l2);
	    //System.out.println("sweepLine: "+sweepLine);
	    //System.out.println("b1: "+b1+", b2: "+b2+", m1: "+l1.m+", m2: "+l2.m);
	    if (b1.less(b2)) return -1;
	    if (b1.greater(b2)) return 1;
	    if (l1.m.less(l2.m)) return -1;
	    if (l1.m.greater(l2.m)) return 1;
	    if (l1.seg.getEndpoint().x.less(l2.seg.getEndpoint().x)) return -1;
	    if (l1.seg.getEndpoint().x.greater(l2.seg.getEndpoint().x)) return 1;
	    return 0;
	}//if
	else 
	    throw new WrongTypeException("Exception in LineComparator: Expected type Line - found: "+((Entry)ino1).value.getClass()+"/"+((Entry)ino2).value.getClass());
	
    }//end method compare
    

    public void setX (Rational x) {
	this.x = x;
    }//end method setX

    /*
    public void setLine (Line l) {
	//sets this.line to l and computes actual y-value for this.x
	this.line = l;
	if (this.x.equal(line.seg.getStartpoint().x))
	    this.yValue = line.seg.getStartpoint().y;
	else 
	    this.yValue = line.m.times(this.x).plus(line.b);
    }//end method setLine
    */

    private Rational evaluateLineFunc(Line l) {
	//assumes that seg is a line and computes the y value for a given x
	Rational t1 = l.m.times(this.x,this.bt).plus(l.b,this.bp);
	return t1;
    }//end method evaluateLineFunc
    

}//end class LineComparator