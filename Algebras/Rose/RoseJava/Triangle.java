import java.lang.Math.*;
import java.util.*;
import java.lang.reflect.*;

class Triangle extends Element{

    //members
    protected Point[] vertices = new Point[3];
    //CAUTION: perhaps we should use the same implementation for this set as used in every other set for compatibility
    //protected double perimeter;
    //protected double area;
    //private Rect bbox; //bounding box

  //constructors
  public Triangle() {
    //fill in dummy values1
    Point dum0 = new Point(0.0, 0.0);
    Point dum1 = new Point(1.0, 0.0);
    Point dum2 = new Point(0.0, 1.0);
    this.vertices[0] = dum0;
    this.vertices[1] = dum1;
    this.vertices[2] = dum2;
    //this.perimeter = perimeter();
    //this.area = area();
    //computeBbox();
  }

  public Triangle(Point p1, Point p2, Point p3) {
    if (isTriangle(p1,p2,p3))
      {
	this.vertices[0] = (Point)p1.copy();
	this.vertices[1] = (Point)p2.copy();
	this.vertices[2] = (Point)p3.copy();
	//this.perimeter = perimeter();
	//this.area = area();
	//computeBbox();
	//System.out.println("--> generated new triangle");
	//this.print();
	//this.bbox.print();
      }
    else System.out.println("ERROR. No triangle.");
  }
  
  public Triangle(Segment s, Point p) {
    if (isTriangle(p,s.startpoint,s.endpoint))
      {
	this.vertices[0] = (Point)s.startpoint.copy();
	this.vertices[1] = (Point)s.endpoint.copy();
	this.vertices[2] = (Point)p.copy();
	//this.perimeter = perimeter();
	//this.area = area();
	//computeBbox();
      }
    else {
	System.out.println("ERROR. Tried to build a 'bad' triangle.");
	System.exit(0);
    }//else
  }
  
  public Triangle(Segment s1, Segment s2) {
      boolean ssEqual = false;
      
      ssEqual = s1.equal(s2);
      
      if (!ssEqual &&
	  SegSeg_Ops.formALine(s1,s2) &&
	  (!(SegSeg_Ops.formASegment(s1,s2))))
	  {
	      if (s1.startpoint.equal(s2.startpoint))
		  {
		      this.vertices[0] = (Point)s1.startpoint.copy();
		      this.vertices[1] = (Point)s2.endpoint.copy();
		      this.vertices[2] = (Point)s1.endpoint.copy();
		  }
	      else if (s1.endpoint.equal(s2.startpoint))
		  {
		      this.vertices[0] = (Point)s1.endpoint.copy();
		      this.vertices[1] = (Point)s1.startpoint.copy();
		      this.vertices[2] = (Point)s2.endpoint.copy();
		  }
	      else if (s1.endpoint.equal(s2.endpoint))
		  {
		      this.vertices[0] = (Point)s1.endpoint.copy();
		      this.vertices[1] = (Point)s1.startpoint.copy();
		      this.vertices[2] = (Point)s2.startpoint.copy();
		  }
	      else if (s1.startpoint.equal(s2.endpoint))
		  {
		      this.vertices[0] = (Point)s1.startpoint.copy();
		      this.vertices[1] = (Point)s1.endpoint.copy();
		      this.vertices[2] = (Point)s2.startpoint.copy();
		  }
	      //this.perimeter = perimeter();
	      //this.area = area();
	      //computeBbox();
	  }
      else
	  {
	      //System.out.println();
	      //System.out.print(s1.equal(s2));
	      //System.out.print(SegSeg_Ops.formALine(s1,s2));
	      //System.out.println(SegSeg_Ops.formASegment(s1,s2));
	      System.out.println("Error (class Triangle). Segment was not created.");
	      System.exit(0);
	  }//else
  }
      

    //methods
    public double perimeter() {
	//computes the perimeter of the triangle
	double sum = 0;
	Segment s1 = new Segment(vertices[0], vertices[1]);
	Segment s2 = new Segment(vertices[1], vertices[2]);
	Segment s3 = new Segment(vertices[2], vertices[0]);
	//sum = s1.length.plus(s2.length.plus(s3.length));
	sum = s1.length() + s2.length() + s3.length();
	return sum;
    } //end method perimeter

  public double area() {
      //computes the area of the triangle
      //caution: this is not computed precisely
      /*
	Point A = (Point)vertices[0].copy();
	Point B = (Point)vertices[1].copy();
	Point C = (Point)vertices[2].copy();
	Segment a = new Segment(B,C);
	Segment c = new Segment(A,B);
	Segment b = new Segment(A,C);
	//double alpha = Mathset.angleD(vertices[1],vertices[2]);
	double alpha = Mathset.angleD(Mathset.diff(A,B),Mathset.diff(C,A));
	System.out.println("alpha: "+alpha);
	//System.out.println("alpha: "+alpha);
	//alpha = Math.toDegrees(alpha);
	Rational sinus = new Rational(Math.sin(Math.toRadians(alpha)));
	System.out.println("sinus: "+sinus);
	this.area = new Rational((new Rational(0.5)).times(b.length).times(c.length).times(sinus));
	//Rational alpha = new Rational(Mathset.angle(vertices[1],vertices[2]));
	//area = (new Rational(0.5)).times(b.length.times(c.length.times(new Rational(Math.sin(alpha.getDouble())))));
	return area;
      */

      //alternative implementation
      SegList segs = this.segments();
      Segment a = (Segment)segs.get(0); //System.out.println("a.length: "+a.length);
      Segment b = (Segment)segs.get(1); //System.out.println("b.length: "+b.length);
      Segment c = (Segment)segs.get(2); //System.out.println("c.length: "+c.length);
      double semiperim = 0.5 * this.perimeter(); //System.out.println("semiperim: "+semiperim);
      double op1 = semiperim - a.length(); //System.out.println("op1: "+op1);
      double op2 = semiperim - b.length(); //System.out.println("op2: "+op2);
      double op3 = semiperim - c.length(); //System.out.println("op3: "+op3);
      double op4 = semiperim * op1 * op2 * op3; //System.out.println("op4: "+op4);
      double op5 = Math.sqrt(op4); //System.out.println("op5: "+op5);
      //double area = Math.sqrt(((((semiperim
      //			  .times(semiperim.minus(a.length)))
      //			 .times(semiperim.minus(b.length)))
      //			.times(semiperim.minus(c.length)))).getDouble());
      //System.out.println("Triangle.area: "+op5);
      return op5;

  } //end method area
  
  public Triangle set (Point p1, Point p2, Point p3){
    //sets the vertices of the triangle
    this.vertices[0] = (Point)p1.copy();
    this.vertices[1] = (Point)p2.copy();
    this.vertices[2] = (Point)p3.copy();
    //this.area = area();
    //this.perimeter = perimeter();
    //computeBbox();
    return this;
  }//end method set

  public Triangle set (Point[] p){
    //sets the vertices of the triangle
    this.vertices[0] = (Point)p[0].copy();
    this.vertices[1] = (Point)p[1].copy();
    this.vertices[2] = (Point)p[2].copy();
    //this.area = area();
    //this.perimeter = perimeter();
    //computeBbox();
    return this;
  }//end method set 
  
  public Point[] vertices(){
    //returns a Point array with the three vertices
    Point[] pl = new Point[3];
    pl[0] = (Point)this.vertices[0].copy();
    pl[1] = (Point)this.vertices[1].copy();
    pl[2] = (Point)this.vertices[2].copy();
    return pl;
  }//end method vertices


    public PointList vertexlist(){
	//returns a PointList with the three vertices
	PointList retList = new PointList();
	retList.add((Point)this.vertices[0].copy());
	retList.add((Point)this.vertices[1].copy());
	retList.add((Point)this.vertices[2].copy());
	return retList;
    }//end method vertices

  public boolean isTriangle(Point p1, Point p2, Point p3){
    //true, if none of p1,p2,p3 are equal, false else
      
      if(!(p1.equal(p2)) &&
	 !(p1.equal(p3)) &&
	 !(p2.equal(p3)))
	  { return true; }
      else {
	  System.out.println(" tried to build bad triangle: p1("+p1.x.toString()+","+p1.y.toString()+"), p2("+p2.x.toString()+","+p2.y.toString()+"), p3("+p3.x.toString()+","+p3.y.toString()+")");
	  return false;
      }//else
  }//end method isTriangle

  public Element copy(){
    //returns a copy of the triangle
    Triangle copy = new Triangle();
    copy.vertices[0] = (Point)this.vertices[0].copy();
    copy.vertices[1] = (Point)this.vertices[1].copy();
    copy.vertices[2] = (Point)this.vertices[2].copy();
    //copy.perimeter = this.perimeter;
    //copy.area = this.area;
    //copy.bbox = this.bbox.copy();

    return copy;
  }//end method copy

    public SegList segments(){
	//returns the triangle's segments as a set
	SegList retList = new SegList();
	retList.add(new Segment(this.vertices[0].x,this.vertices[0].y,
				this.vertices[1].x,this.vertices[1].y));
	retList.add(new Segment(this.vertices[1].x,this.vertices[1].y,
				this.vertices[2].x,this.vertices[2].y));
	retList.add(new Segment(this.vertices[2].x,this.vertices[2].y,
				this.vertices[0].x,this.vertices[0].y));
	return retList;
    }//end method segments


  public boolean equal(Element trin) throws WrongTypeException {
    //returns true, if this.triangle and tin are equal
    if (trin instanceof Triangle) {
      Triangle tin = (Triangle)trin;
      SegList sl01 = this.segments();
      SegList sl02 = tin.segments();
      Segment[] slA1 = new Segment[3];
      slA1 = (Segment[])sl01.toArray(slA1);
      Segment[] slA2 = new Segment[3];
      slA2 = (Segment[])sl02.toArray(slA2);
      if ((slA1[0].equal(slA2[0]) ||
	   slA1[0].equal(slA2[1]) ||
	   slA1[0].equal(slA2[2])) &&
	  (slA1[1].equal(slA2[0]) ||
	   slA1[1].equal(slA2[1]) ||
	   slA1[1].equal(slA2[2])) &&
	  (slA1[2].equal(slA2[0]) ||
	   slA1[2].equal(slA2[1]) ||
	   slA1[2].equal(slA2[2])))
	  return true;
      //return SetOps.equal(sl01,sl02);
      else return false;
    }//if
    else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+trin.getClass()); }
  }//end method equal
    

    public byte compY (Element e) throws WrongTypeException {
	//compares the y-coordinates of the topmost points of the
	//given object and THIS.object and returns
	//-1, if THIS.object has a smaller y-coordinate
	//0, if the y-coordinates are equal
	//+1, if THIS.object has a greater y-coordinate
	//if the y-coordinates of the first point are equal
	//compare the coordinates of the next one etc.
	if (e instanceof Triangle) {
	    Triangle t = (Triangle)e;

	    PointList thisV = new PointList();
	    PointList tinV = new PointList();
	    for (int i = 0; i < 3; i++) {
		thisV.add(this.vertices[i]);
		tinV.add(t.vertices[i]);
	    }//for i
	    SetOps.quicksortX(thisV);
	    SetOps.quicksortX(tinV);
	    if (((Point)thisV.getFirst()).compY((Point)tinV.getFirst()) != 0) {
		return ((Point)thisV.getFirst()).compY((Point)tinV.getFirst());
	    }//if
	    else if (((Point)thisV.get(1)).compY((Point)tinV.get(1)) != 0) {
		return ((Point)thisV.get(1)).compY((Point)tinV.get(1));
	    }//else
	    else if (((Point)thisV.get(2)).compY((Point)tinV.get(2)) != 0) {
		return ((Point)thisV.get(2)).compY((Point)tinV.get(2));
	    }//else
	    else {
		//both triangles are equal
		return 0;
	    }//else
	}//if

	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method compY


    public byte compX(Element e) throws WrongTypeException {
	//compares the x-coordinates of the leftmost points of the
	//given object and THIS.object and returns
	//-1, if THIS.object has a smaller x-coordinate
	//0, if the x-coordinates are equal
	//+1, if THIS.object has a greater x-coordinate
	//if the x-coordinates of the first point are equal
	//compare the coordinates of the next one etc.
	if (e instanceof Triangle) {
	    Triangle t = (Triangle)e;
	    
	    PointList thisV = new PointList(); 
	    PointList tinV = new PointList();
	    for (int i = 0; i < 3; i++) {
		thisV.add(this.vertices[i]);
		tinV.add(t.vertices[i]);
	    }//for i
	    SetOps.quicksortX(thisV); 
	    SetOps.quicksortX(tinV); 
	    if (((Point)thisV.getFirst()).compX((Point)tinV.getFirst()) != 0) {
		return ((Point)thisV.getFirst()).compX((Point)tinV.getFirst());
	    }//if
	    else if (((Point)thisV.get(1)).compX((Point)tinV.get(1)) != 0) {
		return ((Point)thisV.get(1)).compX((Point)tinV.get(1));
	    }//else
	    else if (((Point)thisV.get(2)).compX((Point)tinV.get(2)) != 0) {
		return ((Point)thisV.get(2)).compX((Point)tinV.get(2));
	    }//else
	    else {
		//both triangles are equal
		return 0;
	    }//else
	}//if
	
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method compX
    

    public byte compare (Element e) throws WrongTypeException {
	//...
	if (e instanceof Triangle) {
	    Triangle t = (Triangle)e;
	    PointList thisV = new PointList();
	    PointList tinV = new PointList();
	    for (int i = 0; i < 3; i++) {
		thisV.add(this.vertices[i]);
		tinV.add(t.vertices[i]); }
	    SetOps.quicksortX(thisV);
	    SetOps.quicksortX(tinV);
	    byte res = ((Point)thisV.getFirst()).compare((Point)tinV.getFirst());
	    if (res == 0) {
		res = ((Point)thisV.get(1)).compare((Point)tinV.get(1));
		if (res == 0) res = ((Point)thisV.get(2)).compare((Point)tinV.get(2));
	    }//if
	    return res;
	}//if
	
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }  
    }//end method compare

  public void update() {
    //updates area and perimeter
    //this.perimeter = perimeter();
    //this.area = area();
    //computeBbox();
  }//end method update

  public Rect computeBbox() {
    //computes and sets bbox
    Rational tmp = vertices[0].x;
    Rational hxv;//highest x value
    Rational lxv;//lowest x value
    Rational hyv;//highest y value
    Rational lyv;//lowest y value
    
    //compute hxv
    for (int i = 1; i < 3; i++) {
      if (tmp.less(vertices[i].x)) { tmp = vertices[i].x; }
    }//for
    hxv = tmp;//.copy();

    //compute lxv
    tmp = vertices[0].x;    
    for (int i = 1; i < 3; i++) {
      if (tmp.greater(vertices[i].x)) { tmp = vertices[i].x; }
    }//for
    lxv = tmp;//.copy();
    
    //compute hyv
    tmp = vertices[0].y;
    for (int i = 1; i < 3; i++) {
      if (tmp.less(vertices[i].y)) { tmp = vertices[i].y; }
    }//for
    hyv = tmp;//.copy();

    //compute lyv
    tmp = vertices[0].y;
    for (int i = 1; i < 3; i++) {
      if (tmp.greater(vertices[i].y)) { tmp = vertices[i].y; }
    }//for
    lyv = tmp;//.copy();
  
    //set rectangle
    return new Rect(lxv,hyv,hxv,lyv);
  }//end method computeBbox

  public void print() {
    //prints the object's data
    System.out.print("triangle:");
    System.out.print(" ("+vertices[0].x.toString()+", "+vertices[0].y.toString()+")");
    System.out.print(" ("+vertices[1].x.toString()+", "+vertices[1].y.toString()+")");
    System.out.println(" ("+vertices[2].x.toString()+", "+vertices[2].y.toString()+")");
  }//end method print


  public boolean intersects(Element e) throws WrongTypeException {
      //returns true if this.object and e intersect
      //intersection means: both objects have a common area
      //caution: it seems that this operations is not a pintersects!
      if (e instanceof Triangle) {
	  Triangle t;// = new Triangle();
	  t = (Triangle)e;
	  SegList t1segs = this.segments();
	  SegList t2segs = t.segments();
	  boolean intersect = false;
	  
	  if (this.noPointsInside(t) > 0) {
	      return true; 
	  }//if
	  
	  for (int i = 0; i < 3; i++) {
	      for (int j = 0; j < 3; j++) {
		  if (((Segment)t1segs.get(i)).intersects((Segment)t2segs.get(j))) {
		      intersect = true;
		      return intersect;
		  }//if
	      }//for j
	  }//for
	  return intersect;
      }//if
      else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
  }//end method intersects


    public boolean pintersects(Triangle tin){
	//returns true if this and tin have a common area
	if (this.equal(tin)) { return true; }
	if (TriTri_Ops.inside(this,tin) ||
		TriTri_Ops.inside(tin,this)) { return true; }
	SegList thisSegs = this.segments();
	SegList tinSegs = tin.segments();
	Iterator it1 = thisSegs.listIterator(0);
	Iterator it2;
	Segment actSeg;
	while (it1.hasNext()) {
	    actSeg = (Segment)it1.next();
	    it2 = tinSegs.listIterator(0);
	    while (it2.hasNext()) {
		if (actSeg.pintersects((Segment)it2.next())) { return true; }
	    }//while
	}//while

	return false;
    }//end method pintersects


    protected int noPointsInside(Triangle tin){
	//checks how many of tin's vertices lie inside of this.objects
	//and returns that number
	
	Point[] tinPl = new Point[3];
	tinPl = tin.vertices();
	int numInside = 0;

	if (PointTri_Ops.inside(tinPl[0],this)) { numInside++; }
	if (PointTri_Ops.inside(tinPl[1],this)) { numInside++; }
	if (PointTri_Ops.inside(tinPl[2],this)) { numInside++; }
	
	return numInside;
    }//end method noPointsInside


    /*
      private static int noSegsCrossed(Triangle tin1,Triangle tin2) {
      //supportive method for minus
      //returns the number of segments of tin1
      //crossed by tin2
      
      int number = 0;
      Segment[] tin1Segs = new Segment[3];
      Segment[] tin2Segs = new Segment[3];
      tin1Segs = tin1.getSegs();
      tin2Segs = tin2.getSegs();
      
      for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
      if (tin1Segs[i].intersects(tin2Segs[j])) {
      number++;
      break;
      }//if
      }//for j
      }//for i
      
      return number;
      }//end methodnoSegsCrossed
    */

    public TriList intersection (Triangle tin) {
	//returns the set of triangles resulting from the intersection of t1, t2
	//System.out.println("entering T.intersection...");
	//System.out.println("***********************");
	//this.print();
	//tin.print();
	TriList retList = new TriList();
	if (TriTri_Ops.inside(this,tin)) {
	    //System.out.println("T.intersection: t1 fully lies inside of t2");
	    retList.add(this.copy());
	    return retList;
	}//if
	if (!this.intersects(tin)) {
	    //System.out.println("T.intersection: t1 doesn't intersect t2");
	    return retList;
	}//if
	
	//System.out.println("first checks passed..");

	PointList thisPoints = this.vertexlist();
	PointList tinPoints = tin.vertexlist();
	SegList segs = computeSegList(this,tin);
	PointList intPoints = new PointList();
	PointList allPoints = new PointList();
	SegList chosenSegs = new SegList();
	//System.out.println("second tests passed...");
	//compute intPoints
	Class c = (new Segment()).getClass();
	try {
	    Method m = c.getMethod("endpoints",null);
	    allPoints = PointList.convert(SetOps.map(segs,m));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	allPoints = PointList.convert(SetOps.rdup(allPoints));
	intPoints = PointList.convert(SetOps.difference(allPoints,thisPoints));
	intPoints = PointList.convert(SetOps.difference(intPoints,tinPoints));
	//add to intPoints all equal points of this,tin
	intPoints.addAll(SetOps.intersection(thisPoints,tinPoints));
	//add to intPoints all points which lie on the border of the other triangle
	for (int i = 0; i < 3; i++) {
	    if (PointTri_Ops.liesOnBorder((Point)thisPoints.get(i),tin)) {
		intPoints.add(((Point)thisPoints.get(i)).copy());
	    }//if
	    if (PointTri_Ops.liesOnBorder((Point)tinPoints.get(i),this)) {
		intPoints.add(((Point)tinPoints.get(i)).copy());
	    }//if
	}//for i

	intPoints = PointList.convert(SetOps.rdup(intPoints));
	//System.out.println("computed intPoints");
	//intPoints.print();
	//System.exit(0);
				      
	//check the points of all segments wether their vertices are
	//of this, tin or intPoints
	for (int i = 0; i < segs.size(); i++) {
	    Segment actSeg = (Segment)segs.get(i);
	    Point p1 = actSeg.getStartpoint();
	    Point p2 = actSeg.getEndpoint();
	    boolean p1insideA = PointTri_Ops.inside(p1,this);
	    boolean p1insideB = PointTri_Ops.inside(p1,tin);
	    boolean p2insideA = PointTri_Ops.inside(p2,this);
	    boolean p2insideB = PointTri_Ops.inside(p2,tin);
	    boolean p1elementX = false;
	    if (intPoints.contains(p1) > -1) { p1elementX = true; }
	    boolean p2elementX = false;
	    if (intPoints.contains(p2) > -1) { p2elementX = true; }
	    boolean p1vertexA = PointTri_Ops.isVertex(p1,this);
	    boolean p1vertexB = PointTri_Ops.isVertex(p1,tin);
	    boolean p2vertexA = PointTri_Ops.isVertex(p2,this);
	    boolean p2vertexB = PointTri_Ops.isVertex(p2,tin);
	    boolean p1onBorderA = PointTri_Ops.liesOnBorder(p1,this);
	    boolean p1onBorderB = PointTri_Ops.liesOnBorder(p1,tin);
	    boolean p2onBorderA = PointTri_Ops.liesOnBorder(p2,this);
	    boolean p2onBorderB = PointTri_Ops.liesOnBorder(p2,tin);
	    
	    if (p1vertexA && p2vertexA && p1insideB && p2insideB) {
		//System.out.println("case1");
		chosenSegs.add(actSeg.copy());
	    }//if
	    else if (p1vertexB && p2vertexB &&
		     ((p1insideA && p2insideA) && (!p1vertexA || !p2vertexA))) {
		//System.out.println("case2");
		chosenSegs.add(actSeg.copy());
	    }//if
	    else if ((p1vertexA && p2elementX && p1insideB) ||
		     (p1elementX && p2vertexA && p2insideB)) {
		//System.out.println("case3");
		chosenSegs.add(actSeg.copy());
	    }//if
	    else if ((p1vertexB && p2elementX && p1insideA) ||
		     (p1elementX && p2vertexB && p2insideA)) {
		//System.out.println("case4");
		chosenSegs.add(actSeg.copy());
	    }//if
	    else if (p1elementX && p2elementX) {
		//System.out.println("case5");
		chosenSegs.add(actSeg.copy());
	    }//if
	    else {
		//System.out.println("this case isn't covered...");
	    }//else
	}//for i
	if (chosenSegs.size() < 3) {
	    System.out.println("Error in Triangle.intersection: chosenSegs < 3");
	    System.exit(0);
	}//if

	//System.out.println("segs chosen...");
	//System.out.println("chosenSegs:"); chosenSegs.print();
	//System.out.println("\n\n");
	if (!chosenSegs.isEmpty()) {
	    retList = (new Polygons(chosenSegs)).triangles();
	}//if

	//System.out.println("leaving T.intersection.");
	return retList;
    }//end method intersection


    public TriList plus (Triangle tin) {
	//returns the set of triangles resulting from adding t2 to t1
	TriList retList = new TriList();
	if (TriTri_Ops.inside(this,tin)) {
	    //System.out.println("T.plus: t1 fully lies inside of t2");
	    retList.add(tin.copy());
	    return retList;
	}//if
	if (TriTri_Ops.inside(tin,this)) {
	    //System.out.println("T.plus: t2 fully lies inside of t1");
	    retList.add(this.copy());
	    return retList;
	}//if
	if (!this.intersects(tin)) {
	    //System.out.println("T.plus: t1 doesn't intersect t2");
	    retList.add(tin.copy());
	    retList.add(this.copy());
	    return retList;
	}//if

	PointList thisPoints = this.vertexlist();
	PointList tinPoints = tin.vertexlist();
	SegList segs = computeSegList(this,tin);
	PointList intPoints = new PointList();
	PointList allPoints = new PointList();
	SegList chosenSegs = new SegList();
	//compute intPoints
	Class c = (new Segment()).getClass();
	try {
	    Method m = c.getMethod("endpoints",null);
	    allPoints = PointList.convert(SetOps.map(segs,m));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	allPoints = PointList.convert(SetOps.rdup(allPoints));
	intPoints = PointList.convert(SetOps.difference(allPoints,thisPoints));
	intPoints = PointList.convert(SetOps.difference(intPoints,tinPoints));
	//add to intPoints all equal points of this,tin
	intPoints.addAll(SetOps.intersection(thisPoints,tinPoints));
	//add to intPoints all points which lie on the border of the other triangle
	for (int i = 0; i < 3; i++) {
	    if (PointTri_Ops.liesOnBorder((Point)thisPoints.get(i),tin)) {
		intPoints.add(((Point)thisPoints.get(i)).copy());
	    }//if
	    if (PointTri_Ops.liesOnBorder((Point)tinPoints.get(i),this)) {
		intPoints.add(((Point)tinPoints.get(i)).copy());
	    }//if
	}//for i

	intPoints = PointList.convert(SetOps.rdup(intPoints));
	//System.out.println("computed intPoints");
	//intPoints.print();
				      
	//check the points of all segments wether their vertices are
	//of this, tin or intPoints
	for (int i = 0; i < segs.size(); i++) {
	    Segment actSeg = (Segment)segs.get(i);
	    Point p1 = actSeg.getStartpoint();
	    Point p2 = actSeg.getEndpoint();
	    boolean p1insideA = PointTri_Ops.inside(p1,this);
	    boolean p1insideB = PointTri_Ops.inside(p1,tin);
	    boolean p2insideA = PointTri_Ops.inside(p2,this);
	    boolean p2insideB = PointTri_Ops.inside(p2,tin);
	    boolean p1elementX = false;
	    if (intPoints.contains(p1) > -1) { p1elementX = true; }
	    boolean p2elementX = false;
	    if (intPoints.contains(p2) > -1) { p2elementX = true; }
	    boolean p1vertexA = PointTri_Ops.isVertex(p1,this);
	    boolean p1vertexB = PointTri_Ops.isVertex(p1,tin);
	    boolean p2vertexA = PointTri_Ops.isVertex(p2,this);
	    boolean p2vertexB = PointTri_Ops.isVertex(p2,tin);
	    boolean p1onBorderA = PointTri_Ops.liesOnBorder(p1,this);
	    boolean p1onBorderB = PointTri_Ops.liesOnBorder(p1,tin);
	    boolean p2onBorderA = PointTri_Ops.liesOnBorder(p2,this);
	    boolean p2onBorderB = PointTri_Ops.liesOnBorder(p2,tin);

	    //System.out.println("\nactSeg("+i+"):"); actSeg.print();

	    if (p1vertexA && p2vertexA && !p1insideB && !p2insideB) {
		//System.out.println("case1");
		chosenSegs.add(actSeg.copy());
	    }//if
	    
	    else if (p1vertexB && p2vertexB && !p1insideB && !p2insideB) {
		//System.out.println("case2");
		chosenSegs.add(actSeg.copy());
	    }//if

	    else if ((p1vertexA && p2elementX) &&
		     !p1insideB && !p2insideB && !(p1onBorderB && p2onBorderB)) {
		//System.out.println("case3");
		chosenSegs.add(actSeg.copy());
	    }//if

	    else if ((p1vertexB && p2elementX) &&
		     !p1insideA && !p2insideA && !(p1onBorderA && p2onBorderA)) {
		//System.out.println("case4");
		chosenSegs.add(actSeg.copy());
	    }//if

	    else if ((p1vertexA && p2vertexB) ||
		     (p1vertexB && p2vertexA)) {
		//System.out.println("case5");
		chosenSegs.add(actSeg.copy());
	    }//if
	
	    else {
		//System.out.println("this case isn't covered...");
	    }//else
	}//for i

	//System.out.println("chosenSegs:"); chosenSegs.print();
	//System.out.println(); System.out.println();
	if (!chosenSegs.isEmpty()) {
	    retList = (new Polygons(chosenSegs)).triangles();
	}//if
	
	return retList;
    }//end method plus


    public TriList minus (Triangle tin) {
	//retuns the set of triangles resulting from subtracting tin from this
	//System.out.println("entering T.minus...");
	TriList retList = new TriList();
	if (TriTri_Ops.inside(this,tin)) {
	    //System.out.println("T.minus: t1 lies fully inside of t2");
	    return retList;
	}//if
	if (!this.pintersects(tin)) {
	    //System.out.println("T.minus: t1 doesn't intersect t2");
	    retList.add(this.copy());
	    return retList;
	}//if
	
	PointList thisPoints = this.vertexlist();
	PointList tinPoints = tin.vertexlist();
	SegList segs = computeSegList(this,tin);
	PointList intPoints = new PointList();
	PointList allPoints = new PointList();
	SegList chosenSegs = new SegList();
	//compute intPoints
	Class c = (new Segment()).getClass();
	try {
	    Method m = c.getMethod("endpoints",null);
	    allPoints = PointList.convert(SetOps.map(segs,m));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	//System.out.println("allPoints:"); allPoints.print();
	allPoints = PointList.convert(SetOps.rdup(allPoints));
	//System.out.println("computed allPoints"); allPoints.print();
	//System.out.println("this.points"); this.print();
	//System.out.println("tin"); tin.print();
	intPoints = PointList.convert(SetOps.difference(allPoints,thisPoints));
	intPoints = PointList.convert(SetOps.difference(intPoints,tinPoints));
	//add to intPoints all equal points of this,tin
	intPoints.addAll(SetOps.intersection(thisPoints,tinPoints));

	//add to intPoints all points which lie on the border of the other triangle
	for (int i = 0; i < 3; i++) {
	    if (PointTri_Ops.liesOnBorder((Point)thisPoints.get(i),tin)) {
		intPoints.add(((Point)thisPoints.get(i)).copy());
		//System.out.println("added point"); ((Point)intPoints.getLast()).print();
	    }//if
	    if (PointTri_Ops.liesOnBorder((Point)tinPoints.get(i),this)) {
		intPoints.add(((Point)tinPoints.get(i)).copy());
		//System.out.println("added point"); ((Point)intPoints.getLast()).print();
	    }//if
	}//for i

	intPoints = PointList.convert(SetOps.rdup(intPoints));
	//System.out.println("T.minus.computed intPoints");
	//intPoints.print();

	//System.out.println("segs:"); segs.print();
	//check the points of all segments wether their vertices are
	//of this, tin or intPoints
	for (int i = 0; i < segs.size(); i++) {

	    //System.out.println("\nactual segment:"+(i+1));
	    //((Segment)segs.get(i)).print();

	    Segment actSeg = (Segment)segs.get(i);
	    Point p1 = actSeg.getStartpoint();
	    Point p2 = actSeg.getEndpoint();
	    boolean p1insideA = PointTri_Ops.inside(p1,this);
	    boolean p1insideB = PointTri_Ops.inside(p1,tin);
	    boolean p2insideA = PointTri_Ops.inside(p2,this);
	    boolean p2insideB = PointTri_Ops.inside(p2,tin);
	    boolean p1elementX = false;
	    if (intPoints.contains(p1) > -1) { p1elementX = true; }
	    //System.out.println("p1elementX:"+p1elementX);
	    boolean p2elementX = false;
	    if (intPoints.contains(p2) > -1) { p2elementX = true; }
	    //System.out.println("p2elementX:"+p2elementX);
	    boolean p1vertexA = PointTri_Ops.isVertex(p1,this);
	    boolean p1vertexB = PointTri_Ops.isVertex(p1,tin);
	    boolean p2vertexA = PointTri_Ops.isVertex(p2,this);
	    boolean p2vertexB = PointTri_Ops.isVertex(p2,tin);
	    boolean p1onBorderA = PointTri_Ops.liesOnBorder(p1,this);
	    boolean p1onBorderB = PointTri_Ops.liesOnBorder(p1,tin);
	    boolean p2onBorderA = PointTri_Ops.liesOnBorder(p2,this);
	    boolean p2onBorderB = PointTri_Ops.liesOnBorder(p2,tin);
	    
	    
	    if (p1vertexA && p2vertexA && !p1elementX && !p2elementX &&
		((!p1vertexB || !p2vertexB) && (!p1insideB || !p2insideB))) {
		//System.out.println("case1");
		chosenSegs.add(actSeg.copy());
	    }//if
	    
	    else if ((p1vertexB && p2vertexB) &&
		     ((!p1insideA || !p2insideA) && (p1insideA && p2insideA))) {
		//System.out.println("case2");
		chosenSegs.add(actSeg.copy());
	    }//if

	    else if (((p1vertexA && p2elementX && !p1insideB) ||
		      (p2vertexA && p1elementX && !p2insideB)) &&
		     !(p1elementX && p1elementX)) {
		//System.out.println("case3");
		chosenSegs.add(actSeg.copy());
	    }//if

	    else if ((p1vertexB && p2elementX && p1insideA) ||
		     (p2vertexB && p1elementX && p2insideA)) {
		//System.out.println("case4");
		chosenSegs.add(actSeg.copy());
	    }//if

	    else if (p1elementX && p2elementX && !SegTri_Ops.overlapsBorder(actSeg,this)) {
		//System.out.println("case5");
		chosenSegs.add(actSeg.copy());
	    }//if
	    
	    else if (p1vertexB && p2vertexB && p1insideA && p2insideA) {
		//System.out.println("case6");
		chosenSegs.add(actSeg.copy());
	    }//if
	    
	    else if ((p1vertexA && !p1elementX && !p1insideB) ||
		     (p2vertexA && !p2elementX && !p2insideB)) {
		//System.out.println("case7");
		chosenSegs.add(actSeg.copy());
	    }//if

	    else {
		//System.out.println("this case isn't covered...");
	    }//else
	}//for i

	//System.out.println("chosenSegs:"); chosenSegs.print();
	//System.out.println(); System.out.println();

	if (!chosenSegs.isEmpty()) {
	    retList = (new Polygons(chosenSegs)).triangles();
	}//if

	return retList;
    }//end method minus


    public Rect rect() {
	//returns the bounding box of this.object
	//return this.bbox.copy();
	return this.computeBbox();
    }//end method rect

    public Rational dist(Element e) throws WrongTypeException {
	//returns the distance between this.object and e
	if (e instanceof Triangle) {
	    Triangle t = (Triangle)e;
	    LinkedList distlist = new LinkedList();
	    for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
		    distlist.add(((Segment)this.segments().get(i)).dist((Segment)t.segments().get(j)));
		    //System.out.println("["+i+"]["+j+"]: "+this.getSegs()[i].dist(t.getSegs()[j]));
		    //this.getSegs()[i].print();
		    //t.getSegs()[j].print();
		    //System.out.println();
		}//for j
	    }//for i
	    
	    Rational min = new Rational(0);
	    min = (Rational)distlist.getFirst();
	    for (int i = 0; i < distlist.size(); i++) {
		if (((Rational)distlist.get(i)).less(min)) {
		    min = (Rational)distlist.get(i); }//if
	    }//for i

	    return min.copy();
	    
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method dist


    private static SegList computeSegList (Triangle t1, Triangle t2) {
	//computes a set of segments which is generated from intersecting t1,t2
	//if two segments of t1,t2 intersect, they are splitted
	//double segments are removed
	//System.out.println("entering T.computeSegList");
	SegList t1segs = new SegList();
	SegList t2segs = new SegList();
	t1segs = t1.segments();
	t2segs = t2.segments();

	//System.out.println("split triangles");

	//split the triangles in sets of segments
	//compute these sets
	SegList segs = new SegList();
	segs.addAll(t1segs);
	segs.addAll(t2segs);
	Class c = (new SegSeg_Ops()).getClass();
	Class c2 = (new Segment()).getClass();
	Class[] paramList = new Class[2];
	Class[] paramList2 = new Class[1];
	paramList[0] = (new Segment()).getClass();
	paramList[1] = (new Segment()).getClass();
	try {
	    /*
	    System.out.println("********testing overlapReduce*********");
	    long timeX1 = System.currentTimeMillis();
	    for (int i = 0; i < 50; i++) {
		paramList2[0] = Class.forName("Element");
		Method methX = c.getMethod("split",paramList);
		Method predX = c2.getMethod("pintersects",paramList2);
		SegList nsegs = SegList.convert(SetOps.overlapReduce(segs,predX,methX));
		//System.out.println("\nnsegs["+i+"]:");
		//nsegs.print();
	    }//for
	    long timeX2 = System.currentTimeMillis();
	    System.out.println("costs for overlapReduce: "+((timeX2-timeX1)/50)+"ms");
	    System.exit(0);
	    */
	    paramList2[0] = Class.forName("Element");
	    //System.out.println("doing first reduce...");
	    Method meth = c.getMethod("split",paramList);
	    Method pred1 = c.getMethod("overlap",paramList);
	    //long time1 = System.currentTimeMillis();
	    //segs = (SegList)SetOps.reduce(segs,pred1,meth);
	    segs = (SegList)SetOps.overlapReduce(segs,pred1,meth,false);
	    //long time2 = System.currentTimeMillis();
	    //segs.print();
	    //System.out.println("first reduce done");
	    //System.out.println("doing second reduce");
	    Method pred2 = c2.getMethod("pintersects",paramList2);
	    //segs = (SegList)SetOps.reduce(segs,pred2,meth);
	    segs = SegList.convert(SetOps.overlapReduce(segs,pred2,meth,false));
	    //long time3 = System.currentTimeMillis();
	    //segs.print();
	    //System.exit(0);
	    //System.out.println("second reduce done");
	    //segs.print();
	    //System.exit(0);
	    Method pred3 = c.getMethod("pmeet",paramList);
	    //System.out.println("doing third reduce");
	    //segs = (SegList)SetOps.reduce(segs,pred3,meth);
	    segs = (SegList)SetOps.overlapReduce(segs,pred3,meth,true);
	    //long time4 = System.currentTimeMillis();
	    //System.out.println("reduce1: "+(time2-time1)+"ms, reduce2: "+(time3-time2)+"ms, reduce3: "+(time4-time3)+"ms");
	    //segs.print();
	    //System.out.println("third reduce done");
	    //System.exit(0);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch

	//System.out.println("full list of segments:");
	//segs.print();
	//System.out.println("leaving t.computeSegLists.");
	return segs;
    }//end method computeSegLists

    
    protected void zoom (Rational fact) {
	//multiplies this.coordinates with fact
	this.vertices[0].zoom(fact);
	this.vertices[1].zoom(fact);
	this.vertices[2].zoom(fact);
	this.update();
    }//end method zoom

  
} //end class Triangle
  
