import java.lang.reflect.*;

class Circle extends Element {
    //instances of this class represent circles as sets of triangles
    
    //members
    public Point centre; //centre of the circle
    public Rational radius; //radius of the circle
    public double circumference; //circumference of the circle
    public double area; //area of the circle
    private TriList triList; //set of triangles representing the circle
    public int fineness; //positive int value >0 which defines how many triangles represent the circle
    private Rect bbox; //bounding box
    
    //constructors
    public Circle() {
	//fill in dummy values
	this.centre = new Point(0,0);
	this.radius = RationalFactory.constRational(1);
	this.triList = new TriList();
	this.fineness = 10;
	computeTriList();
	update(); //computes circumference, area, bbox
    }
    
    public Circle(Point cent, Rational rad, int fine) {
	this.centre = (Point)cent.copy();
	this.radius = rad.copy();
	if (fine <= 2) {
	    System.out.println("Error(class Circle): too small argument for fine");
	    System.exit(0);
	}//if
	this.fineness = fine;
	this.triList = new TriList();
	computeTriList();
	update();
    }
    
    //methods
    public Element copy() {
	return new Circle(this.centre, this.radius, this.fineness);
    }//end method copy
    
    public boolean equal(Element inElement) throws WrongTypeException {
	if (inElement instanceof Circle) {
	    Circle inCircle = (Circle)inElement;
	    SegList contourTHIS = new SegList();
	    SegList contourIN = new SegList();
	    Class c = (new Triangle()).getClass();
	    
	    try {
		Method m = c.getMethod("segments",null);
		contourTHIS = minimal(unique(SegList.convert(SetOps.map(triList,m))));
		contourIN = minimal(unique(SegList.convert(SetOps.map(inCircle.triList,m))));
	    }//try
	    catch (Exception e ) {
		System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		System.exit(0);
	    }//catch
	    
	    return SetOps.equal(contourTHIS,contourIN);
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+inElement.getClass()); }
    }//end method equal
    
    public byte compX (Element inElement) throws WrongTypeException {
	if (inElement instanceof Circle) {
	    Circle inCircle = (Circle)inElement;
	    if (this.centre.x.minus(this.radius).less(inCircle.centre.x.minus(inCircle.radius))) { return -1; }
	    else {
		if (this.centre.x.minus(this.radius).equal(inCircle.centre.x.minus(inCircle.radius))) { return 0; }
		else { return 1; }
	    }//else
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+inElement.getClass()); }
    }//end method compX
    
    public byte compY (Element inElement) throws WrongTypeException {
	if (inElement instanceof Circle) {
	    Circle inCircle = (Circle)inElement;
	    if (this.centre.y.minus(this.radius).less(inCircle.centre.y.minus(inCircle.radius))) { return -1; }
	    else {
		if (this.centre.y.minus(this.radius).equal(inCircle.centre.y.minus(inCircle.radius))) { return 0; }
		else { return 1; }
	    }//else	
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+inElement.getClass()); }
    }//end method compY
    
    public void update () {
	computeCircumference();
	computeArea();
    }//end method update
    
    public void computeBbox () {
	this.bbox = new Rect(centre.x.minus(radius),centre.y.plus(radius),centre.x.plus(radius),centre.y.minus(radius));
    }//end method computeBbox
    
    public Rect rect () {
	return this.bbox;
    }//end method rect
    
    public boolean intersects (Element inElement) throws WrongTypeException {
	if (inElement instanceof Circle) {
	    Circle inCircle = (Circle)inElement;
	    Class c = (new Triangle()).getClass();
	    PairList retList = new PairList();
	    Class[] paramList = new Class[1];
	    try {
		paramList[0] = Class.forName("Element");
		Method m = c.getMethod("intersects",paramList);
		retList = SetOps.join(this.triList,inCircle.triList,m);
	    }//try
	    catch (Exception e) {
		System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		System.exit(0);
	    }//catch
	    if (retList.isEmpty()) { return false; }
	    else { return true; }
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+inElement.getClass()); }
    }//end method intersects
    
    public Rational dist (Element inElement) throws WrongTypeException {
	if (inElement instanceof Circle) {
	    Circle inCircle = (Circle)inElement;
	    Rational retVal = RationalFactory.constRational(0);
	    Class c = (new Triangle()).getClass();
	    Class[] paramList = new Class[1];
	    
	    try {
		paramList[0] = Class.forName("Element");
		Method m = c.getMethod("dist",paramList);
		ElemPair retPair = SetOps.min(this.triList,inCircle.triList,m);
		retVal = retPair.first.dist(retPair.second);
	    }//try
	    catch (Exception e) {
		System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		System.exit(0);
	    }//catch
	    return retVal;
	}//if
	else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+inElement.getClass()); }
    }//end method dist
    
    public void print() {
	System.out.println("Circle:");
	System.out.println("circle.area: "+this.area);
	System.out.println("circle.circumference: "+this.circumference);
	System.out.println("circle.triList: ");
	triList.print();
    }//end method print
    
    public TriList triangles () {
	//returns the set of triangles representing the circle
	//this method is needed because triList has to be private, since the
	//triangle set is computed and must not be changed otherwise
	return triList;
    }//end method triangles
    
    private void computeCircumference() {
	SegList contour = new SegList();
	Class c = (new Triangle()).getClass();
	
	try {
	    Method m = c.getMethod("segments",null);
	    contour = minimal(unique(SegList.convert(SetOps.map(triList,m))));
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	
	c = (new Segment()).getClass();
	try {
	    Method m = c.getMethod("length",null);
	    this.circumference = SetOps.sum(contour,m);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
    }//end method computeCircumference
    
    private void computeArea() {
	//System.out.println("computeArea-call");
	Class c = (new Triangle()).getClass();
	try {
	    Method m = c.getMethod("area",null);
	    this.area = SetOps.sum(this.triList,m);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
    }//end method computeArea
    
    private void computeTriList() {
	//computes the triList
	PointList pointlist = new PointList();
	double param;
	//compute number of points defined by fineness
	param = 2*Math.PI / fineness;
	for (int i = 0; i < fineness; i++) {
	    pointlist.add(new Point(RationalFactory.constRational(radius.getDouble() * Math.sin(i*param)), RationalFactory.constRational(radius.getDouble() * Math.cos(i*param))));
	}//for i
	
	for (int i = 0; i < pointlist.size(); i++) {
	    pointlist.set(i,Mathset.sum(((Point)pointlist.get(i)),centre));
	}//for i

	//compute triangles
	this.triList = new TriList();
	for (int i = 0; i < fineness-1; i++) {
	    this.triList.add(new Triangle(this.centre,(Point)pointlist.get(i),(Point)pointlist.get(i+1)));
	}//for i
	this.triList.add(new Triangle(this.centre,(Point)pointlist.getLast(),(Point)pointlist.getFirst()));
    }//end method computeTriList
    
    private static SegList minimal (SegList inSegList) {
	//the same as used for the ROSE algebra
	SegList retList = new SegList();
	Class c = (new SegSeg_Ops()).getClass();
	Class segClass = (new Segment()).getClass();
	Class[] paramListSS = { segClass,segClass };
	try {
	    Method m1 = c.getMethod("adjacent",paramListSS);
	    Method m2 = c.getMethod("concat",paramListSS);
	    retList = (SegList)SetOps.reduce(inSegList,m1,m2);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retList;
    }//end method minimal
    
    private static SegList unique (SegList inSegList) {
	//the same as used for the ROSE algebra
	SegList retList = new SegList();
	Class c = (new SegSeg_Ops()).getClass();
	Class segClass = (new Segment()).getClass();
	Class[] paramListSS = { segClass,segClass };
	try {
	    Method m1 = c.getMethod("overlap",paramListSS);
	    Method m2 = c.getMethod("symDiff",paramListSS);
	    retList = (SegList)SetOps.reduce(inSegList,m1,m2);
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	}//catch
	return retList;
    }//end method minimal

    public byte compare (Element inC) {
	//...
	return 0;
    }//end method compare
    
}//end class Circle
