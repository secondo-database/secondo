import java.util.*;

class PointSeg_Ops {

  //variables
    //Caution: Use the same values as in Mathset!!!
    static final double DERIV_DOUBLE = 0.00001;
    static final double DERIV_DOUBLE_NEG = -0.00001;
    static final double DERIV_DOUBLE_PLUS1 = 1.00001;

  //constructors

  //methods
    public static boolean liesOn(Point p, Segment s) {
	//true if the point lies on the segment, false else
	//CAUTION: here an overflow may happen!
	//patched with derivation value
	//CAUTION: changed to DERIV lately. May have bugs!
	//newest version: computation depends on RationalFactory.PRECISE
	//if true, DERIV is used.
	//otherwise doubles are used
	
	//System.out.println("PSO.liesON:");
	//p.print();
	//s.print();

	//Rational DERIV = RationalFactory.readDeriv();

	if (RationalFactory.readPrecise()) {
	    //PRECISE == true, use Deriv
	    Rational DERIV = RationalFactory.readDeriv();
	    if (isEndpoint(p,s)) { return true; }
	    Point s1p = Mathset.diff(p,s.startpoint);
	    Point s1s2 = Mathset.diff(s.endpoint,s.startpoint);
	    int count = 0;
	    boolean s1pX0 = s1p.x.equal(0);
	    boolean s1pY0 = s1p.y.equal(0);
	    if (s1pX0 && s1pY0) return true;
	    
	    Rational t1 = RationalFactory.constRational(0);//must be set to 0
	    Rational t2 = RationalFactory.constRational(0);//dito
	    if (!s1pX0) t1 = s1s2.x.dividedby(s1p.x);
	    if (!s1pY0) t2 = s1s2.y.dividedby(s1p.y);

	    Rational t1MINt2 = t1.minus(t2);
	    boolean t1t2equal = t1MINt2.less(DERIV) && t1MINt2.greater(DERIV.times(-1));
	    
	    //System.out.println("PPS.liesOn: t1: "+t1+", t2: "+t2+", t1t2equal: "+t1t2equal);

	    if (!(s1pX0 || s1pY0) && !t1t2equal) {
		//System.out.println("false case 1");
		return false; }

	    Rational s1s2compXVal = (s1p.x.times(t1)).minus(s1s2.x);
	    boolean s1s2compX = (DERIV.equal(s1s2compXVal)) || 
		(s1s2compXVal.less(DERIV) && s1s2compXVal.greater(DERIV.times(-1)));
	    Rational s1s2compYVal = (s1p.y.times(t2)).minus(s1s2.y);
	    boolean s1s2compY = (DERIV.equal(s1s2compYVal)) ||
		(s1s2compYVal.less(DERIV) && s1s2compYVal.greater(DERIV.times(-1)));

	    //System.out.println("s1s2compXVal: "+s1s2compXVal+", s1s2compYVal: "+s1s2compYVal);
	    //System.out.println("t1: "+t1+", t2: "+t2);
	    //System.out.println("DERIV: "+DERIV);

	    boolean t1valid = t1.equal(0) || !(t1.less(DERIV.plus(1)));
	    boolean t2valid = t2.equal(0) || !(t2.less(DERIV.plus(1)));

	    //System.out.println("s1s2compX: "+s1s2compX+", s1s2compY: "+s1s2compY+", t1valid: "+t1valid+", t2valid: "+t2valid);

	    if (s1s2compX && s1s2compY &&
		t1valid && t2valid) return true;
	    return false;
	}//if
	else {
	    //PRECISE == false
	    if (isEndpoint(p,s)) { return true; }
	    Point s1p = Mathset.diff(p,s.startpoint);
	    Point s1s2 = Mathset.diff(s.endpoint,s.startpoint);
	    //System.out.println("s1p:"); s1p.print();
	    //System.out.println("s1s2:"); s1s2.print();
	    int count = 0;
	    boolean s1pX0 = s1p.x.equal(0);
	    boolean s1pY0 = s1p.y.equal(0);
	    if (s1pX0 && s1pY0) return true;
	    
	    //Rational t1 = RationalFactory.constRational(0);//must be set to 0
	    double t1 = 0;
	    //Rational t2 = RationalFactory.constRational(0);//dito
	    double t2 = 0;
	    //if (!s1pX0) t1 = s1s2.x.dividedby(s1p.x);
	    if (!s1pX0) t1 = s1s2.x.getDouble() / s1p.x.getDouble();
	    //if (!s1pY0) t2 = s1s2.y.dividedby(s1p.y);
	    if (!s1pY0) t2 = s1s2.y.getDouble() / s1p.y.getDouble();
	    
	    boolean t1t2equal = ((t1 - t2) < DERIV_DOUBLE) && ((t1 - t2) > DERIV_DOUBLE_NEG);
	    //boolean t1t2equal = (t1.minus(t2)).less(DERIV) && (t1.minus(t2)).greater(DERIV.times(-1));
	    
	    //System.out.println("PPS.liesOn1: t1: "+t1+", t2: "+t2+", t1t2equal: "+t1t2equal);
	    
	    
	    ////if (!(s1pX0 || s1pY0) && !t1.equal(t2)) { 
	    if (!(s1pX0 || s1pY0) && !t1t2equal) {
		//System.out.println("false case1");
		return false; }
	    
	    boolean s1s2compX = (((s1p.x.getDouble() * t1 - s1s2.x.getDouble()) < DERIV_DOUBLE) &&
				 ((s1p.x.getDouble() * t1 - s1s2.x.getDouble()) > DERIV_DOUBLE_NEG));
	    //Rational s1s2compXVal = (s1p.x.times(t1)).minus(s1s2.x);
	    //boolean s1s2compX = s1s2compXVal.less(DERIV) && s1s2compXVal.greater(DERIV.times(-1));
	    boolean s1s2compY = (((s1p.y.getDouble() * t2 - s1s2.y.getDouble()) < DERIV_DOUBLE) &&
				 ((s1p.y.getDouble() * t2 - s1s2.y.getDouble()) > DERIV_DOUBLE_NEG));
	    //Rational s1s2compYVal = (s1p.y.times(t2)).minus(s1s2.y);
	    //boolean s1s2compY = s1s2compYVal.less(DERIV) && s1s2compYVal.greater(DERIV.times(-1));
	    //System.out.println("s1s2compX: "+s1s2compX+", s1s2compY: "+s1s2compY);

	    boolean t1valid = (t1 == 0) || !(t1 < DERIV_DOUBLE_PLUS1);
	    //boolean t1valid = t1.equal(0) || !(t1.less(DERIV.plus(1)));
	    boolean t2valid = (t2 == 0) || !(t2 < DERIV_DOUBLE_PLUS1);
	    //boolean t2valid = t2.equal(0) || !(t2.less(DERIV.plus(1)));
	    
	    //System.out.println("s1s2compX: "+s1s2compX+", s1s2compY: "+s1s2compY+", t1valid: "+t1valid+", t2valid: "+t2valid);
	    
	    ////if ((s1p.x.times(t1).equal(s1s2.x)) &&
	    ////  (s1p.y.times(t2).equal(s1s2.y)) &&
	    ////  !t1.less(1) && !t2.less(1)) { return true; }
	    if (s1s2compX && s1s2compY &&
		t1valid && t2valid) return true;
	    //System.out.println("false case2");
	    return false;
	}//else
    } //end method liesOn


  public static boolean isEndpoint(Point p, Segment s) {
    //true if the point is an endpoint of the segment, false else
      //boolean b = false;
      //b = (p.equal(s.endpoint) || p.equal(s.startpoint));
      //return b;
      return (p.equal(s.endpoint) || p.equal(s.startpoint));
  } //end method isEndpoint


    public static Rational dist(Point p, Segment s){
	//return the distance between p and s
	
	if (liesOn(p,s)) { return (RationalFactory.constRational(0)); }//if
	LinkedList distlist = new LinkedList();
	
	distlist.add(s.startpoint.dist(p));
	distlist.add(s.endpoint.dist(p));
	Point proj = Mathset.projectionPointLine(p,s.startpoint,s.endpoint);
	if (PointSeg_Ops.liesOn(proj,s)) { distlist.add(proj.dist(p)); }
	
	Rational min = (Rational)distlist.getFirst();
	for (int i = 1; i < distlist.size(); i++) {
	   if (((Rational)distlist.get(i)).less(min)) {
		min = (Rational)distlist.get(i); }//if
	}//for i

	return min.copy();
    }//end method dist

} //end class PointSeg_Ops
