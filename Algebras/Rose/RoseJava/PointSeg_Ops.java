import java.util.*;

class PointSeg_Ops {

  //variables

  //constructors

  //methods
    public static boolean liesOn(Point p, Segment s) {
	//true if the point lies on the segment, false else
	//CAUTION: here an overflow may happen!
	//patched with doubles used in computation
	
	//System.out.println("PSO.liesON:");
	//p.print();
	//s.print();
	
	if (isEndpoint(p,s)) { return true; }
	Point s1p = Mathset.diff(p,s.startpoint);
	Point s1s2 = Mathset.diff(s.endpoint,s.startpoint);
	//System.out.println("s1p:"); s1p.print();
	//System.out.println("s1s2:"); s1s2.print();
	int count = 0;
	boolean s1pX0 = s1p.x.equal(0);
	boolean s1pY0 = s1p.y.equal(0);
	if (s1pX0 && s1pY0) return true;

	//Rational t1 = new Rational(0);//must be set to 0
	double t1 = 0;
	//Rational t2 = new Rational(0);//dito
	double t2 = 0;
	//if (!s1pX0) t1 = s1s2.x.dividedby(s1p.x);
	if (!s1pX0) t1 = s1s2.x.getDouble() / s1p.x.getDouble();
	//if (!s1pY0) t2 = s1s2.y.dividedby(s1p.y);
	if (!s1pY0) t2 = s1s2.y.getDouble() / s1p.y.getDouble();
	
	boolean t1t2equal = ((t1 - t2) < 0.0000000001) && ((t1 - t2) > -0.0000000001);

	//System.out.println("PPS.liesOn1: t1: "+t1+", t2: "+t2+", t1t2equal: "+t1t2equal);

	
	//if (!(s1pX0 || s1pY0) && !t1.equal(t2)) { 
	if (!(s1pX0 || s1pY0) && !t1t2equal) {
	    //System.out.println("false case1");
	    return false; }
     
	boolean s1s2compX = (((s1p.x.getDouble() * t1 - s1s2.x.getDouble()) < 0.0000000001) &&
			     ((s1p.x.getDouble() * t1 - s1s2.x.getDouble()) > -0.0000000001));
	boolean s1s2compY = (((s1p.y.getDouble() * t2 - s1s2.y.getDouble()) < 0.0000000001) &&
			     ((s1p.y.getDouble() * t2 - s1s2.y.getDouble()) > -0.0000000001));
	boolean t1valid = (t1 == 0) || !(t1 < 1.0000000001);
	boolean t2valid = (t2 == 0) || !(t2 < 1.0000000001);
	//System.out.println("s1s2compX: "+s1s2compX+", s1s2compY: "+s1s2compY+", t1valid: "+t1valid+", t2valid: "+t2valid);

	//if ((s1p.x.times(t1).equal(s1s2.x)) &&
	//  (s1p.y.times(t2).equal(s1s2.y)) &&
	//  !t1.less(1) && !t2.less(1)) { return true; }
	if (s1s2compX && s1s2compY &&
	    t1valid && t2valid) return true;
	//System.out.println("false case2");
	return false;

	/* old implementation 
	//System.out.println("\nentering PSO.liesOn...");
	if (PointSeg_Ops.isEndpoint(p,s)) { 
	    //System.out.println("PSO.liesOn: isEndpoint:");
	    //p.print(); s.print();
	    //System.out.println(".....");
	    return true; }
	Point proj;// = new Point();
	proj = Mathset.projectionPointLine(p,s.startpoint,s.endpoint);
	//System.out.print("proj: "); proj.print();
	Point diff;// = new Point();
	diff = Mathset.diff(proj,p);
	//System.out.print("diff:"); diff.print();
	//System.out.println("diff.length: "+Mathset.lengthD(diff));
	if (Mathset.lengthD(diff) == 0) {
	    //the point lies on the line but does it lie on the segment?
	    Point AB;// = new Point();
	    Point AP;// = new Point();
	    Rational t = new Rational(0);
	    AB = Mathset.diff(s.endpoint,s.startpoint);
	    AP = Mathset.diff(p,s.startpoint);
	    if (AB.x.equal(0) || AB.x.equal(AP.x)) {
		Rational help = new Rational(0);
		help = AB.x;//.copy();
		AB.x = AB.y;//.copy();
		AB.y = help;//.copy();
		help = AP.x;//.copy();
		AP.x = AP.y;//.copy();
		AP.y = help;//.copy();
		//System.out.println("**twist**");
	    }//if
	    t = AP.x.dividedby(AB.x);
	    //System.out.print("AB: "); AB.print();
	    //System.out.print("AP: "); AP.print();
	    System.out.println("PSO.liesOn: t: "+t.toString());
	    if (t.less(0) || t.greater(1)) { 
		System.out.println("PSO.liesON: false (case1)");
		return false; }
	    else { return true; }//if
	}//if
	else {
	    System.out.println("PSO.liesOn: false (case2)");
	    return false; }
		*/
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
	
	if (liesOn(p,s)) { return (new Rational(0)); }//if
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
