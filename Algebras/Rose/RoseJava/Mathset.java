import java.lang.Math.*;

class Mathset {
  //provides some mathematical methodes for Points(interpreted as vectors)

  //variables
    //static final Rational deriv = Algebra.deriv;

  //constructors

  //methods
  public static Rational length(Point v) {
    //computes the length of a vector
      //caution: this is not precisely computed!!!
    Rational l = new Rational(Math.sqrt((v.x.times(v.x).plus(v.y.times(v.y))).getDouble()));
    return l;
  }//end method length

    public static double lengthD(Point v) {
	//computes the length of a vector
	//caution: the length is computed as double
	double l = Math.sqrt(v.x.getDouble() * v.x.getDouble() + v.y.getDouble() * v.y.getDouble());
	return l;
    }//end method lengthD

  public static Point sum(Point v1, Point v2) {
    //returns the sum of two vectors
    Point v = new Point((v1.x.plus(v2.x)),(v1.y.plus(v2.y)));
    return v;
  }//end method sum

  public static Point diff(Point v1, Point v2) {
    //returns the difference of two vectors
    Point v = new Point((v1.x.minus(v2.x)),(v1.y.minus(v2.y)));
    return v;
  }//end method diff

  public static Rational prod(Point v1, Point v2) {
    //returns the product of the vectors v1, v2
    Rational e = new Rational((v1.x.times(v2.x)).plus(v1.y.times(v2.y)));
    return e;
  }//end method prod


    public static Point mulfac(Point p, Rational fac) {
	//multiplies p with fac
	return new Point(p.x.times(fac),p.y.times(fac));
    }//end method mulfac


    public static Point addfac(Point p, double fac) {
	//add fac to p
	return new Point(p.x.plus(new Rational(fac)),p.y.plus(new Rational(fac)));
    }//end method addfac


    public static Point subfac(Point p, double fac) {
	//subtracts fac from p
	return new Point(p.x.minus(new Rational(fac)),p.y.minus(new Rational(fac)));
    }//end method subfac


    public static boolean linearly_dependent(Segment s1, Segment s2) {
	//true, if the vectors given by s1,s2 are linear dependant, false else
	//caution: division by zero may occur!!!
	
	//System.out.println("MS.lindep");
	
	//new implementation
	Point sv1 = new Point(s1.endpoint.x.minus(s1.startpoint.x),
			      s1.endpoint.y.minus(s1.startpoint.y));
	Point sv2 = new Point(s2.endpoint.x.minus(s2.startpoint.x),
			      s2.endpoint.y.minus(s2.startpoint.y));
	
	//System.out.println("MS.sv1:"); sv1.print();
	//System.out.println("MS.sv2:"); sv2.print();
	
	//int count = 0;
	boolean sv2X0 = sv2.x.equal(0);
	boolean sv2Y0 = sv2.y.equal(0);
	
	if (sv2X0 && sv2Y0) return true;
	
	//frome here: changed double back to Rational (15-07-03)
	Rational t1 = new Rational(0);
	//double t1 = 0;
	Rational t2 = new Rational(0);
	//double t2 = 0;
	
	if (!sv2X0) t1 = sv1.x.dividedby(sv2.x);
	//if (!sv2X0) t1 = sv1.x.getDouble() / sv2.x.getDouble();
	if (!sv2Y0) t2 = sv1.y.dividedby(sv2.y);
	//if (!sv2Y0) t2 = sv1.y.getDouble() / sv2.y.getDouble();
	
	//old:
	//boolean t1t2equal = (((t1 - t2) < Algebra.deriv.getDouble()) &&
	//			   ((t1 - t2) > Algebra.deriv.getDouble()));
	//new:
	boolean t1t2equal = false;
	if (t1.minus(t2).equal(0)) t1t2equal = true;
	//if ((t1 - t2) == 0) t1t2equal = true;
	else {
	    Rational zwires = (t1.minus(t2)).abs();
	    //double zwires = Math.abs(t1-t2);
	    //System.out.println("t1 - t2 = "+zwires);
	    if (zwires.less(Algebra.deriv)) t1t2equal = true;
	    //if (zwires < Algebra.deriv.getDouble()) t1t2equal = true;
	    else t1t2equal = false;
	}//else
	
	
	//System.out.println("t1: "+t1+", t2: "+t2+", equal: "+t1t2equal);

	//if (!(sv2X0 || sv2Y0) && !t1.equal(t2)) {
	if (!(sv2X0 || sv2Y0) && !t1t2equal) {
	    //System.out.println("false case2");
	    return false; }

	//if (t1.equal(t2)) return true;
	if (t1t2equal && !(t1.equal(0) && t2.equal(0))) return true;
	//if (t1t2equal && !(t1 == 0 && t2 == 0)) return true;
	
	boolean compsv1x = (sv2.x.times(t1).minus(sv1.x)).abs().lessOrEqual(Algebra.deriv);
	//boolean compsv1x = (((sv2.x.getDouble() * t1 - sv1.x.getDouble()) < Algebra.deriv.getDouble()) &&
	//		  ((sv2.x.getDouble() * t1 - sv1.x.getDouble()) > (Algebra.deriv.times(-1)).getDouble()));
	boolean compsv1y = (sv2.y.times(t2).minus(sv1.y)).abs().lessOrEqual(Algebra.deriv);
	//boolean compsv1y = (((sv2.y.getDouble() * t2 - sv1.y.getDouble()) < Algebra.deriv.getDouble()) &&
	//		  ((sv2.y.getDouble() * t2 - sv1.y.getDouble()) > (Algebra.deriv.times(-1)).getDouble()));

	
	/*
	  System.out.println("sv2.x*t1-sv1.x = "+sv2.x.times(t1).minus(sv1.x));
	  System.out.println("sv2.y*t2-sv1.y = "+sv2.y.times(t2).minus(sv1.y));
	  System.out.println("compsv1x: "+compsv1x+", compsv1y: "+compsv1y);
	*/

	if (compsv1x && compsv1y) return true;
	//if (sv2.x.times(t1).equal(sv1.x) &&
	//	  sv2.y.times(t2).equal(sv1.y)) { return true; }
	//System.out.println("false case3");
	return false;
	/*
	  if (t1.equal(t2)) { return true; }
	  else { return false; }
	*/

      /* old implementation
      Rational grad1 = new Rational(0);
      Rational grad2 = new Rational(0);
      int num = 0;
      boolean exc = false;
      
      try { grad1 = s1.gradient(); }
      catch (Exception e) {
	  num++;
	  exc = true;
	  //System.out.println("grad1 = infinite");
      }//catch
      try { grad2 = s2.gradient(); }
      catch (Exception e){
	  num++;
	  exc = true;
	  //System.out.println("grad2 = infinite");
      }//catch
      
      System.out.println("Mathset.linearly_dependent: grad1:"+grad1+", grad2:"+grad2+" ---> "+grad1.equal(grad2));
      

      if (!exc) { if (grad1.equal(grad2)) { return true; } }
      else { if (num == 2) { return true; } }
      return false;
      */
  }//end method linear_dependant


  public static Rational angle(Point p1, Point p2) {
    //computes the angle between two vectors using the scalar product
      //caution: this is not computed precisely
    Rational e = new Rational(prod(p1,p2).dividedby(length(p1).times(length(p2))));
    //System.out.println("e: "+e);
    e = new Rational(Math.acos(e.getDouble()));
    //System.out.println("acos(e):"+e);
    e = new Rational(Math.toDegrees(e.getDouble()));
    //System.out.println("toDegrees(e):"+e);
    return e;
  }//end method angle

    public static double angleD(Point p1, Point p2) {
	//computes the angle between two vectors using the scalar product
	//caution: this is computed as 'double'
	double e = (prod(p1,p2)).getDouble() / (lengthD(p1) * lengthD(p2));
	e = Math.acos(e);
	e = Math.toDegrees(e);
	return e;
    }//end method angleD

  public static Point normalize(Point p){
    //returns the normalized vector p
    p.x = p.x.dividedby(length(p));
    p.y = p.y.dividedby(length(p));
    return p;
  }//end method normalize

    /*
      public static Rational distanceLinePoint(Point g1, Point g2, Point p) {
      //returns the distance from p to line build by g1,g2
      //caution: this is NOT the distance to a segment
      //caution: this is not computed precisely
      Point dir = diff(g2,g1);
      System.out.println("  dir("+dir.x.toString()+","+dir.y.toString()+")");
      Point solder = new Point();
      Point x0 = new Point();
      Rational t0 = new Rational(0);
      t0 = prod(diff(p,g1),dir).dividedby((length(dir).times(length(dir))));
      System.out.println("  t0 = "+t0.toString());
      x0.x = g1.x.plus(t0.times(g2.x));
      x0.y = g1.y.plus(t0.times(g2.y));
      System.out.println("  x0("+x0.x.toString()+","+x0.y.toString()+")");
      solder = diff(x0,p);
      System.out.println("  solder("+solder.x.toString()+","+solder.y.toString()+")");
      return new Rational(Math.sqrt(((solder.x.times(solder.x)).plus((solder.y.times(solder.y)))).getDouble()));   
      }//end method distanceSegPoint
    */

    /*
      public static Point solderSegmentPoint(Segment s, Point p) {
      //returns the distance between s and p
      //caution: this is not computed precisely
      //caution: this returns the solder to a LINE and not to a segment
      //which means that the solder may not lie on the segment
      Point dir = diff(s.endpoint,s.startpoint);
      Point solder = new Point();
      Point x0 = new Point();
      Rational t0 = new Rational(0);
      t0 = prod(diff(p,s.startpoint),dir).dividedby((length(dir).times(length(dir))));
      x0.y = s.startpoint.x.plus(t0.times(s.endpoint.x));
      x0.y = s.startpoint.y.plus(t0.times(s.endpoint.y));
      solder = diff(x0,p);
      return (Point)solder.copy();
      }//end method solderSegmentPoint
    */

  public static byte pointPosition(Point g1, Point g2, Point p) {
    //returns 0 if p lies on the line formed by g1,g2
    //returns 1 if p lies on the right side of the line
    //returns -1 if p lies on the left side of the line
      //CAUTION: here an overflow may happen!
      //patched with doubles used in computation
      //CAUTION: this method is called MUCH too often:
      //e.g.: 60 times for SegTri_Ops.pintersects!!! check this
      //System.out.println("MS.pp");

    //Rational t1 = new Rational(0);
    Point s1;
    Point s2;
    s1 = diff(g2,g1); //System.out.println("s1: "); s1.print();
    s2 = diff(p,g1); //System.out.println("s2: "); s2.print();
      
    // new code
    //PATCH: because we don't need to know the exact
    //value of t0, we don't compute it with Rationals,
    //but with doubles. We produce a warning, if the 
    //resulting value is near to 0.
    //System.out.println("s1.x:"+s1.x+", s1.double: "+s1.x.getDouble());
    double s1x = s1.x.getDouble(); //System.out.println("s1x: "+s1x);
    double s1y = s1.y.getDouble(); //System.out.println("s1y: "+s1y);
    double s2x = s2.x.getDouble(); //System.out.println("s2x: "+s2x);
    double s2y = s2.y.getDouble(); //System.out.println("s2y: "+s2y);
    double t0 = s1x*s2y - s1y*s2x;
    //double t0 = (s1.x.getDouble()*s2.y.getDouble())-(s1.y.getDouble()*s2.y.getDouble());
    //System.out.println("t0: "+t0);
    
    /*
    if ((t0 > -0.00001) && (t0 < 0.00001) && 
	!((s1x == 0) || (s2x == 0) || (s1y == 0) || (s2y == 0) ||
	((s1x == s1y) && (s2x == s2y)))) {
	System.out.println("Mathset:Warning. Possible error because of loss of precision. t0: "+t0+", g1: ("+g1.x+","+g1.y+"), g2: ("+g2.x+","+g2.y+"), p: ("+p.x+","+p.y+")");
	System.out.println("(cont...): s1: ("+s1x+","+s1y+"), s2: ("+s2x+","+s2y+")");
    }//if
    */
    
    if (t0 < 0) return 1;
    else if (t0 > 0) return -1;
    else return 0;
    
    /*
    // old but exact code
	Rational t0 = new Rational((s1.x.times(s2.y)).minus((s1.y.times(s2.x))));
	//System.out.println(" t0: "+t0);

	if (t0.less(0)) { return 1; }
	else {
	if (t0.greater(0)) { return -1; }
	}//else
	
	return 0;
	*/
  }//end method pointPosition

    public static Point projectionPointLine(Point p, Point a, Point b) {
	//returns the projection of point p on the line formed
	//by points a,b
	Point AProj;
	Point AB;
	Point AP;
	AB = diff(b,a);
	AP = diff(p,a);
	AProj = mulfac(AB,prod(AB,AP).dividedby(prod(AB,AB)));
	Point retPoint = sum(AProj,a);
	return retPoint;
    }//end method projectionPointLine

}//end class Mathset
