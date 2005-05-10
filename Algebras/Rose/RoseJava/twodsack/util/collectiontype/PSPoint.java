package twodsack.util.collectiontype;

import twodsack.setelement.datatype.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.*;

public class PSPoint implements ComparableMSE {
    //PlaneSweepPoint

    public Point point;
    public int number; //mostly used for storing an index of an arry 
    public int number2; //dito
    public boolean isStartpoint; //isStartpoint of segment
    public boolean isIntPoint; //is intersection point

    public PSPoint(Point p, int num, int num2, boolean m, boolean i) {
	this.point = p;
	this.number = num;
	this.number2 = num2;
	this.isStartpoint = m;
	this.isIntPoint = i;
    }

    public int compare(ComparableMSE e) throws WrongTypeException {
	//markTSet and inside are NOT used to define a complete order
	if (e instanceof PSPoint) {
	    PSPoint p = (PSPoint)e;
	    int res = this.point.x.comp(p.point.x);
	    if (res == 0) res = this.point.y.comp(p.point.y);
	    if (res != 0) return res;
	    if (res == 0) {
		if (this.isStartpoint && !p.isStartpoint) return -1;
		else if (!this.isStartpoint && p.isStartpoint) return  1; }
	    if (res == 0) {
		if (this.isIntPoint && !p.isIntPoint) return -1;
		else if (!this.isIntPoint && p.isIntPoint) return 1; }
	    if (res == 0) {
		if (this.number < p.number) return -1;
		else return 1; }
	    
	    System.out.println("uncaught Case in PSPoint.compare.");
	    this.print();
	    p.print();
	    System.exit(0);
	    return 0;
	}
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }

    public void print () {
	System.out.println("PSPoint: ("+this.point.x+"/"+this.point.y+"), number: "+this.number+", mark: "+isStartpoint+", intpoint: "+isIntPoint);
    }

    public String toString() {
	return new String("PSPoint: ("+this.point.x+"/"+this.point.y+"), number: "+this.number+", mark: "+isStartpoint+", intpoint: "+isIntPoint);
    }//end method toString
    
}//end class PSPoint