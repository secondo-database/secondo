package twodsack.util.collectiontype;

import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.number.*;

public class Line {

    //fields
    public Rational m;
    public Rational b;
    //public Rational slope;
    public boolean vert;
    public Segment seg;
    public int number;
    Rational mTMP1;
    Rational mTMP2;
    Rational bTMP;

    //constructors
    private Line() {};
    
    public Line(Segment seg, int number) {
	this.seg = seg;
	this.number = number;
	if (seg.getStartpoint().x.equal(seg.getEndpoint().x)) {
	    this.m = null;
	    this.b = null;
	    //this.slope = null;
	    this.vert = true;
	}//if vertical
	else {
	    this.vert = false;
	    this.mTMP1 = RationalFactory.constRational(0);
	    this.mTMP2 = RationalFactory.constRational(0);
	    this.bTMP = RationalFactory.constRational(0);
	    this.m = seg.getStartpoint().y.minus(seg.getEndpoint().y).dividedby(seg.getStartpoint().x.minus(seg.getEndpoint().x));
	    //this.m.assign((seg.startpoint.y.minus(seg.endpoint.y,mTMP1)).dividedby((seg.startpoint.x.minus(seg.endpoint.x,mTMP2)),mTMP2));
	    //this.b.assign(seg.startpoint.y.minus(this.m.times(seg.startpoint.x,bTMP),bTMP));
	    this.b = seg.getStartpoint().y.minus(this.m.times(seg.getStartpoint().x));
	}//else
    }


}//end class Line