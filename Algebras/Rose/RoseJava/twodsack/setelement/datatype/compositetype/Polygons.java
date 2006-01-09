/*
 * Polygons.java 2004-11-04
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.setelement.datatype.compositetype;

import twodsack.io.*;
import twodsack.operation.basictypeoperation.*;
import twodsack.operation.setoperation.*;
import twodsack.set.*;
import twodsack.setelement.*;
import twodsack.setelement.datatype.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.*;
import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;
import twodsack.util.graph.*;
import twodsack.util.meshgenerator.*;
import twodsack.util.number.*;
import java.util.LinkedList;
import java.util.Iterator;
import java.util.ListIterator;
import java.lang.reflect.Method;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.Serializable;

/**
 * The <code>Polygons</code> class provides constructors and methods for the <code>Polygons</code> type.
 * A <code>Polygons</code> value represents one or more polygons which may have holes (and also may have
 * structures, e.g. 'islands' inside of holes). Instances of the type <code>Polygon</code> have different
 * representations: shape (set of segments) or triangles (set of triangles). A <code>Polygon</code> may be
 * constructed from either of those representations. When needed, the other representation is computed
 * automatically.
 */

public class Polygons extends Element implements Serializable {
    
    //members
    private static final PointComparator POINT_COMPARATOR = new PointComparator();
    private static final SegmentComparator SEGMENT_COMPARATOR = new SegmentComparator();
    private static final TriangleComparator TRIANGLE_COMPARATOR = new TriangleComparator();
    private static Segment PLUMBLINE = new Segment(0,0,1,1);

    private boolean trilistDefined;
    private TriMultiSet trilist;//list of triangles forming the polygons
    private boolean perimeterDefined;
    private double perimeter;
    private boolean areaDefined;
    private double area;
    private boolean borderDefined;
    private SegMultiSet border;
    private boolean bboxDefined;
    private Rect bbox;//bounding box

    /*    
     * constructors
     */
    /**
     * Default constructor which constructs an empty <code>Polygons</code> value.
     */
    public Polygons() {
	this.trilist = new TriMultiSet(new TriangleComparator());
	this.perimeter = -1;
	this.area = -1;
	this.border = new SegMultiSet(new SegmentComparator());
	this.bbox = new Rect();
	
	this.trilistDefined = false;
	this.perimeterDefined = false;
	this.areaDefined = false;
	this.borderDefined = false;
	this.bboxDefined = false;
    }
    
    /**
     * Using this constructor, a new instance of <code>Polygons</code> is built using a {@link TriMultiSet}.
     *
     * @param tl the set of triangles representing the area of the <code>Polygons</code> value
     */
    public Polygons(TriMultiSet tl) {
	this.trilist = tl;
	this.area = -1;
	this.border = new SegMultiSet(new SegmentComparator());
	this.perimeter = -1;
	this.bbox = new Rect();
	
	this.trilistDefined = true;
	this.perimeterDefined = false;
	this.areaDefined = false;
	this.borderDefined = false;
	this.bboxDefined = false;
    }
    

    /**
     * Using this constructor, a new instance of <code>Polygons</code> is built using a {@link SegMultiSet}.
     *
     * @param sl the set of segments representing the border of the <code>Polygons</code> value
     */

    public Polygons(SegMultiSet sl) {
	this.border = sl;
	this.trilist = new TriMultiSet(new TriangleComparator());
	this.area = -1;
	this.perimeter = -1;
	this.bbox = new Rect();

	this.trilistDefined = false;
	this.perimeterDefined = false;
	this.areaDefined = false;
	this.borderDefined = true;
	this.bboxDefined = false;
    }

    /*
     * methods
     */    
    /**
     * Computes the area of <code>this</code>.
     *
     * @return the area as double
     */
    private double computeArea(){
	double sum = 0;
	Triangle t = new Triangle();
	Iterator it = this.trilist.iterator();
	while (it.hasNext()) {
	    t = (Triangle)((MultiSetEntry)it.next()).value;
	    sum = sum + t.area();
	}//while
	return sum;
    }//end method area


    /**
     * Computes and sets the bounding box of <code>this</code>.
     */
    private void computeBbox() {
	if (!trilistDefined) {
	    this.trilist = computeTriangles(this.border);
	    this.trilistDefined = true;
	}//if	    
	if (this.trilist.isEmpty()) { }
	else {
	    Rational leftmost = ((Triangle)trilist.first()).rect().ulx;
	    Rational upmost = ((Triangle)trilist.first()).rect().uly;
	    Rational rightmost = ((Triangle)trilist.first()).rect().lrx;
	    Rational downmost = ((Triangle)trilist.first()).rect().lry;
	    
	    Iterator it = trilist.iterator();
	    Rect actrect;
	    while (it.hasNext()) {
		actrect = ((Triangle)((MultiSetEntry)it.next()).value).rect();
		if (actrect.ulx.less(leftmost)) {
		    leftmost = actrect.ulx; }
		if (actrect.uly.greater(upmost)) {
		    upmost = actrect.uly; }
		if (actrect.lrx.greater(rightmost)) {
		    rightmost = actrect.lrx; }
		if (actrect.lry.less(downmost)) {
		    downmost = actrect.lry; }
	    }//for i
	    
	    this.bbox = new Rect(leftmost,upmost,rightmost,downmost);
	    this.bboxDefined = true;
	}//else
    }//end method computeBbox


    /**
     * Computes and sets the bounding box of <code>this</code>.
     * 
     * @return bounding box of the <code>Polygons</code> value as a {@link Rect}.
     */
    public Rect rect() {
       	if (!this.bboxDefined) {
	    computeBbox();
	    this.bboxDefined = true;
	}//if
	return bbox;
    }//end method rect


    /**
     * Computes, sets and returns the perimeter of <code>this</code>.
     *
     * @return perimeter as double
     */
    private double computePerimeter(){
	if (!borderDefined) {
	    this.border = computeBorder();
	    borderDefined = true;
	}//if

	double sum = 0;
	Iterator it = border.iterator();
	Segment actSeg;
	while (it.hasNext()) {
	    actSeg = (Segment)((MultiSetEntry)it.next()).value;
	    sum = sum + actSeg.length();
	}//while
	
	return sum;
    }//end method computePerimeter


    /**
     * Computes and returns the border of <code>this</code>.
     * The border is computed from <code>trilist</code>.
     * NOTE: If trilist is empty, we will get an infinite loop!
     *
     * @return set of segments as {@link SegMultiSet}
     */
    private SegMultiSet computeBorder(){
	boolean useOverlapAtMINIMAL = true;
	boolean useOverlapAtUNIQUE = true;
	boolean bboxFilter = true;
	boolean computeMinimalSet = false;
	//return this.border = SupportOps.contour(this.trilist,useOverlapAtMINIMAL,useOverlapAtUNIQUE,bboxFilter,computeMinimalSet);
	return this.border = SupportOps.contour(this.trilist,true,true);
    }//end method computeBorder
    

    /**
     * Computes and returns a triangulation of a set of segments passed as {@link SegMultiSet}.
     * The triangulation is computed using a sweep line algorithm described by <code>Mehlhorn</code>.
     * Before the triangulation algorithm itself is executed, the set of single faces of the
     * represented polygons is computed. Then, for each face, the algorithm is executed.
     * The resulting sets of triangles are merged and then returned as result.
     * <p>Note: The decomposition is applied only when the borders of segment cycles meet in at least
     * one point, i.e. two faces meet or a hole meets the border of a polygon.
     *
     * Note: This method is deprecated. Use {@link #computeMesh} instead.
     *
     * @param border of the <code>Polygons</code> value as {@link SegMultiSet}
     * @return set of triangles as {@link TriMultiSet}
     * @see #computeMesh(SegMultiSet,boolean)
     */
    public static TriMultiSet computeTriangles(SegMultiSet border){
	if (border.isEmpty()) return new TriMultiSet(new TriangleComparator());

	//At first, check whether a decomposition to single faces
	//is necessary.
	TriMultiSet retList = new TriMultiSet(new TriangleComparator());
	ElemMultiSet allPoints = border.getAllPointsWithDuplicates();
	
	Iterator it = allPoints.iterator();
	boolean facesMustBeComputed = false;
	while (it.hasNext()) {
	    if (((MultiSetEntry)it.next()).number > 2) {
		facesMustBeComputed = true;
		break;
	    }//if
	}//while

	if (facesMustBeComputed) {
	  
	    //If needed, the polygons object (given by border) is decomposed to simple cycles.
	    //This is done by the method Triangle.partitionSegLists().
	    
	    //construct vertices from border
	    PointMultiSet vertices = border.getAllPoints();
	    
	    //construct edges from border
	    PairMultiSet edges = new PairMultiSet(new ElemPairComparator());
	    Segment actSeg;
	    Iterator bit = border.iterator();
	    while (bit.hasNext()) {
		actSeg = (Segment)((MultiSetEntry)bit.next()).value;
		edges.add(new ElemPair(actSeg.getStartpoint(),actSeg.getEndpoint()));
	    }//for i
	    
	    //construct graph
	    Graph polGraph = new Graph(vertices,edges);
	    
	    ElemMultiSetList decompPOL = polGraph.computeFaces();
	    
	    //Now, compute for every cycle found its triangulation.
	    for (int i = 0; i < decompPOL.size(); i++) {
		ElemMultiSet actList = (ElemMultiSet)(decompPOL.get(i));
		retList.addAll(computeTrianglesSUB(SegMultiSet.convert(actList)));
	    }//for i
	}//if
	else retList = computeTrianglesSUB(border);
	
	return retList;
    }//end method computeTriangles


    /**
     * Supportive method for method <code>computeTriangles</code>.
     * This method implements the plane-sweep algorithm.
     * The algorithm does not work for borders with cycles. Therefore the cycles must be removed before.
     * This can be done using the methods in class {@link Graph}.
     *
     * @param border the border of a single connected component as {@link SegMultiSet}
     * @return a triangulation as {@link TriMultiSet}
     * @throws NoProperCyclesException if the passed segment set doen't form proper cycles
     */
    private static TriMultiSet computeTrianglesSUB(SegMultiSet border) throws NoProperCyclesException {
	//System.out.println("Triangulator: Mehlhorn.");
	/*
	  DisplayGFX gfx = new DisplayGFX();	      
	  gfx.initWindow();
	  gfx.addSet(border);
	  gfx.showIt(false);
	  try { int data = System.in.read(); }
	  catch (Exception f) { System.exit(0); }
	  gfx.kill();
	*/

	if (border.isEmpty()) return new TriMultiSet(TRIANGLE_COMPARATOR);

	LinkedList borderVerts = new LinkedList(); //vertices of polygon border
	LinkedList xstruct = new LinkedList(); //x-structure for sweep line algo
	TriMultiSet tl = new TriMultiSet(TRIANGLE_COMPARATOR); //store the generated triangles in here
	
	//construct borderVerts from this.border
	Point new1;// = new Point();
	Point new2;// = new Point();
	boolean isnew1 = true;
	boolean isnew2 = true;
	
	PointMultiSet tempBV = new PointMultiSet(POINT_COMPARATOR);
	Iterator it = border.iterator();
	while (it.hasNext()) {
	    Segment actSeg = (Segment)((MultiSetEntry)it.next()).value;
	    tempBV.add(actSeg.getStartpoint());
	    tempBV.add(actSeg.getEndpoint());
	}//while
	
	tempBV = (PointMultiSet)SetOps.rdup(tempBV);
	it = tempBV.iterator();
	while (it.hasNext()) 
	    borderVerts.add(((MultiSetEntry)it.next()).value);
	
	//sort borderVerts
	//caution: this cannot be done by a simple mergesortX
	//because the sorting regarding Y must be reverse!
	
	SetOps.mergesortXY(borderVerts);
	xstruct = (LinkedList)borderVerts;
	
	//compute a graph from vertices to be able to compute the attribute of a vertex efficiently
	Graph polGraph = new Graph(border);

	//
	//DO THE SWEEP!!!
	//
	Point x;
	boolean first = false;
	Segment found1 = null;
	Segment found2 = null;
	Segment seg1,seg2;
	int pCUp = 0;
	int pCDown = 0;
	int yElemPos = 0;
	SweepStElem newElem;
	Rational angle = RationalFactory.constRational(0);
	LinkedList delList = new LinkedList();
	LinkedList delListInt = new LinkedList();
	String attribute = "";
	ListIterator xit = xstruct.listIterator(0);

	//init y-structure
	//ystruct should be organized that way that the element with the lowest y-value
	// is first
	LinkedList ystruct = new LinkedList();
	Segment actSeg;
	Vertex xVertex;

	//sweep starts
	while (xit.hasNext()) {
	    //check attribute of current Point
	    x = (Point)xit.next();
	    xVertex = polGraph.getVertex(x);
	    try {
		attribute = attribute(x,xVertex,polGraph);
	    } catch (NoProperCyclesException e) { throw e; }
	    
	    if (attribute == "start") {
		
		//find the corresponding segments seg1, seg2
		first = false;

		Vertex[] neighbours = polGraph.getNeighbours(xVertex);
		Point pd1 = (Point)neighbours[0].value;
		Point pd2 = (Point)neighbours[1].value;

		//build seg1,seg2 from found1,found2
		//such that seg1 is the "lower element" and x is its endpoint
		//and seg2 is the "higher element" and x is its startpoint
		byte compare = Mathset.pointPosition(x,pd1,pd2);
		
		boolean pd1first = false;
		
		//now build the segments
		if (compare == -1) {
		    pd1first = true;
		}//if
		else if (compare == 1) {
		}//if
		else if (compare == 0) {
		    
		    System.out.println("Error in Polygons.computeTriangles: a vertex was identified as 'start' but actually is not a start vertex!");
		    throw new RuntimeException("An error occurred in the 2DSACK package.");
		}//else	   
		
		//build three new sweepStructElements and
		//insert them in ystruct in the proper position
		SweepStElem top = new SweepStElem();
		top.is_top = true;
		SweepStElem bottom = new SweepStElem();
		bottom.is_bottom = true;
		SweepStElem start = new SweepStElem();
		if (pd1first) {
		    start.pointChain.add(pd1);
		    start.pointChain.add(x);
		    start.pointChain.add(pd2);
		}//if
		else {
		    start.pointChain.add(pd2);
		    start.pointChain.add(x);
		    start.pointChain.add(pd1);
		}//else
		
		start.rimoEl = x;
		start.in = true;
		SweepStElem mt = new SweepStElem();
		
		//find the proper position in ystruct to insert the new elements
		if (ystruct.isEmpty()) {
		    ystruct.add(bottom);
		    ystruct.add(start);
		    ystruct.add(top);
		}//if
		else {
		    yElemPos = interval3(x, ystruct);
		
		    SweepStElem yElem = (SweepStElem)ystruct.get(yElemPos);
		    if (yElem.pointChain.isEmpty()) {
			//x lies in an "out" interval of ystruct
			//insert the elements
			
			ystruct.add(yElemPos, mt);
			ystruct.add(yElemPos+1, start);
			ystruct.add(yElemPos+2, mt);
		    }//if
		    else {
			//x lies in an "in" interval of ystruct
			//add triangulation segments to tl
			Point lastP = yElem.rimoEl;
			
			//compute index of lastP
			int indexLP = -1;
						    
			for (int j = 0; j < yElem.pointChain.size(); j++) {
			    if (((Point)yElem.pointChain.get(j)).equal(lastP)) {
				indexLP = j;
				break;
			    }//if
			}//for j
			
			if (indexLP == -1) {
			    System.out.println(" FATAL ERROR! No rimoEl in pointChain.");
			    throw new RuntimeException("An error occurred in the 2DSACK package.");
			}//if
			//set pCUp,pCDown in case the loops aren't entered
			pCDown = indexLP;
			pCUp = indexLP;
			
			for (int j = indexLP; j > 1; j--) {
			    boolean compute = false;
			    if (Mathset.pointPosition(x,(Point)yElem.pointChain.get(j-1),(Point)yElem.pointChain.get(j)) == 1) { compute = true; }
			    if (compute) {
				Triangle nt = new Triangle(x,
							   (Point)yElem.pointChain.get(j-1),
							   (Point)yElem.pointChain.get(j));
				tl.add(nt);
				pCDown = j-1;//memorize the last valid position
			    }//if
			}//for j
			
			for (int j = indexLP; j < yElem.pointChain.size()-2; j++) {
			    boolean compute = false;
			    if (Mathset.pointPosition(x,(Point)yElem.pointChain.get(j+1),(Point)yElem.pointChain.get(j)) == -1) {compute = true; }
			
			    if (compute) {
				Triangle nt = new Triangle(x,
							   (Point)yElem.pointChain.get(j+1),
							   (Point)yElem.pointChain.get(j));
				tl.add(nt);
				pCUp = j+1;//memorize the last valid position
			    }//if
			}//for j
			
			//update ystruct
			//build two new SweepStElems and insert them in ystruct
			Point nb1 = pd1;
			Point nb2 = pd2;
			//swap if nb1.y > nb2.y
			if (Mathset.pointPosition(x,nb1,nb2) == -1) {
			    Point nb3 = nb1;
			    nb1 = nb2;
			    nb2 = nb3;
			}//if
			
			//build lower element
			//find the proper order for the elements and run the for-loop
			SweepStElem lower = new SweepStElem();
			
			if (border.contains(new Segment(x,nb1)) ||
			    border.contains(new Segment(nb1,x))) {
			    for (int j = pCDown; j > -1; j--) {
				lower.pointChain.addFirst((Point)yElem.pointChain.get(j));
			    }//for j
			}//if
			else {
			    for (int j = pCDown; j > -1; j--) {
				lower.pointChain.add((Point)yElem.pointChain.get(j));
			    }//for j
			}//else
			lower.pointChain.add(x);
			lower.pointChain.add(nb2);
			lower.rimoEl = x;
			
			//build upper element
			SweepStElem upper = new SweepStElem();
			
			if (border.contains(new Segment(x,nb2)) ||
			    border.contains(new Segment(nb2,x))) {
			    for (int j = pCUp; j < yElem.pointChain.size(); j++) {
				upper.pointChain.add((Point)yElem.pointChain.get(j));
			    }//for j
			}//if
			else {
			    for (int j = pCUp; j < yElem.pointChain.size(); j++) {
				upper.pointChain.addFirst((Point)yElem.pointChain.get(j));
			    }//for
			}//else
			upper.pointChain.addFirst(x);
			upper.pointChain.addFirst(nb1);
			upper.rimoEl = x;
			
			//insert elements in ystruct
			ystruct.set(yElemPos, lower);//replace "in"-element
			ystruct.add(yElemPos+1, mt);
			ystruct.add(yElemPos+2, upper);
			
		    }//else
		}//else
	    }//if
	    //now all operations for attribute=="start" are done
	    
	    else {
		
		if (attribute == "bend") {
		    //find corresponding SweepStElem
		    yElemPos = interval(x, ystruct);
		    
		    //BUGFIX: Sometimes an empty interval is returned, though the correct
		    //interval exists above or below the returned interval. So now determine
		    //whethter this is the case and fix it.
		    SweepStElem yElem = (SweepStElem)ystruct.get(yElemPos);
		    if (yElem.pointChain.isEmpty()) {
			if ((yElemPos < 3) && (yElem.pointChain.size() < 7)) {
			}//if
			else if ((yElemPos > 2) &&
				 (x.equal((Point)((SweepStElem)ystruct.get(yElemPos-1)).pointChain.getFirst()) ||
				  x.equal((Point)((SweepStElem)ystruct.get(yElemPos-1)).pointChain.getLast()))) {
			    yElemPos = yElemPos-1;
			    yElem = (SweepStElem)ystruct.get(yElemPos);
			}//if
			else if (x.equal((Point)((SweepStElem)ystruct.get(yElemPos+1)).pointChain.getFirst()) ||
				 x.equal((Point)((SweepStElem)ystruct.get(yElemPos+1)).pointChain.getLast())) {
			    yElemPos = yElemPos+1;
			    yElem = (SweepStElem)ystruct.get(yElemPos);
			}//if
			else {
			    System.out.println("NO(2)");
			    throw new RuntimeException("An error occurred in the 2DSACK package.");
			}//else
		    }//if
		    
		    //update ystruct
		    //add the "following" element of x to pointChain
		    //find the two neighbour points for x
		    
		    Vertex[] neighbours = polGraph.getNeighbours(xVertex);
		    Point np1 = (Point)neighbours[0].value;
		    Point np2 = (Point)neighbours[1].value;
		    Point xFoll = null;
		    boolean found = false;
		    
		    if (x.equal((Point)yElem.pointChain.getFirst())) {
			if (np1.equal((Point)yElem.pointChain.get(1))) {
			    xFoll = np2;
			    found = true;
			}//if
			else {
			    xFoll = np1;
			    found = true;
			}//else
		    }//if
		    if (x.equal((Point)yElem.pointChain.getLast())) {
			if (np1.equal((Point)yElem.pointChain.get(yElem.pointChain.size()-2))) {
			    xFoll = np2;
			    found = true;
			}//if
			else {
			    xFoll = np1;
			    found = true;
			}//else
		    }//if
		    
		    if (!found) {
			System.out.println("Polygons.computeTriangles: FATAL ERROR! Point from xstruct was not found in ystruct!");
			System.out.println("ystruct:");
			for (int ys = 0; ys < ystruct.size(); ys++) {
			    LinkedList actPC = ((SweepStElem)ystruct.get(ys)).pointChain;
			    for (int idx = 0; idx < actPC.size(); idx++) {
				((Point)actPC.get(idx)).print(); }}
			System.out.println();
			System.out.println("x:");
			x.print();
			throw new RuntimeException("An error occurred in the 2DSACK package.");
		    }
		    
		    //add xFoll to pointChain
		    if (x.equal((Point)yElem.pointChain.getFirst())) {
			yElem.pointChain.addFirst(xFoll);
		    }//if
		    else {
			yElem.pointChain.addLast(xFoll);
		    }//else
		    
		    yElem.rimoEl = x;
		    //now ystruct is updated
		    
		    //compute triangles
		    //if there's only one element to check then don't compute triangles:
		    boolean xposup = false;
		    int up = 0;
		    int down = 0;
		    
		    if (yElem.pointChain.size() > 4) {
			
			//find out, on which end of pointChain x lies
			if (x.equal((Point)yElem.pointChain.get(yElem.pointChain.size()-2))){
			    up = yElem.pointChain.size()-3;
			    down = 1;
			    xposup = true;//x is at the upper end of the pointChain(end)
			}//if
			else {
			    up = yElem.pointChain.size()-2;
			    down = 2;
			    xposup = false;//x is at the lower end of the pointChain(beginning)
			}//else
			
			//find out whether x is (in respect to the y value) on the upper side or 
			//the lower side of the interval
			//this must be done using the sweepline
			
			boolean xIsUpper = false;
			Segment intSeg;
			if (xposup) {
			    intSeg = new Segment((Point)yElem.pointChain.get(1),
						 (Point)yElem.pointChain.get(0));
			}//if
			else {
			    intSeg = new Segment((Point)yElem.pointChain.getLast(),
						 (Point)yElem.pointChain.get(yElem.pointChain.size()-2));
			}//else
			Rational minY,maxY;
			if (intSeg.getStartpoint().compY(intSeg.getEndpoint()) == -1) {
			    minY = intSeg.getStartpoint().y;
			    maxY = intSeg.getEndpoint().y;
			}//if
			else {
			    minY = intSeg.getEndpoint().y;
			    maxY = intSeg.getStartpoint().y;
			}//if
			Segment sweepline = new Segment(x.x,minY.minus(1),x.x,maxY.plus(1));
			Point intPoint = sweepline.intersection(intSeg);
			if (intPoint.compY(x) == -1) xIsUpper = true;
			else xIsUpper = false;
			
			if (xposup) {
			    for (int j = up; j > down; j--) {
				boolean compute = false;
				if (xIsUpper) {
				    if (Mathset.pointPosition(x,(Point)yElem.pointChain.get(j-1),(Point)yElem.pointChain.get(j)) == 1) compute = true; }
				else {
				    if (Mathset.pointPosition(x,(Point)yElem.pointChain.get(j-1),(Point)yElem.pointChain.get(j)) == -1) compute = true; }
				if (compute) {
				    Triangle nt = new Triangle(x,
							       (Point)yElem.pointChain.get(j-1),
							       (Point)yElem.pointChain.get(j));
				    tl.add(nt);
			
				    //now add points to delList
				    delList.add(yElem.pointChain.get(j));
				    delListInt.add(new Integer(j));
				}//if
				else { break; }
			    }//for j
			}// if
			else {
			    for (int j = down; j < up; j++) {
				boolean compute = false;
				if (xIsUpper) {
				    if (Mathset.pointPosition(x,(Point)yElem.pointChain.get(j+1),(Point)yElem.pointChain.get(j)) == 1) compute = true; }
				else {
				    if (Mathset.pointPosition(x,(Point)yElem.pointChain.get(j+1),(Point)yElem.pointChain.get(j)) == -1) compute = true; }
				if (compute) {
				    Triangle nt = new Triangle(x,
							       (Point)yElem.pointChain.get(j+1),
							       (Point)yElem.pointChain.get(j));
				    tl.add(nt);
				
				    //now add points to delList
				    delListInt.add(new Integer(j));
				    delList.add(yElem.pointChain.get(j));
				}//if
				else { break; }
			    }//for
			}//else
			
		    }//if	     
		    		    
		    //sort delListInt
		    for (int si = 0; si < delListInt.size()-1; si++) {
			Integer actIntI = (Integer)delListInt.get(si);
			for (int sj = si; sj < delListInt.size(); sj++) {
			    Integer actIntJ = (Integer)delListInt.get(sj);
			    if (actIntI.compareTo(actIntJ) < 0) {
				Integer swap = actIntI;
				delListInt.set(si,actIntJ);
				delListInt.set(sj,swap);
			    }//if
			}//for sj
		    }//for si
		    
		    //now delete elements
		    for (int j = 0; j < delListInt.size(); j++) {
			yElem.pointChain.remove(((Integer)delListInt.get(j)).intValue());
		    }//for j
		    delListInt.clear();
		    delList.clear();
		    
		    
		    //It may happen that a point with attribute "bend" is part
		    //of two yElems. Here usually the first one is taken and the
		    //triangles are computed. We need to check wether we have that
		    //case here and combine both pointChains of that two yElems.
		    //Up to now we observed only one case in which this should
		    //be checked and this will be fixed here.
		    
		    if (ystruct.size() > yElemPos+3) {
			SweepStElem current = yElem;
			SweepStElem next = (SweepStElem)ystruct.get(yElemPos+2);//there is an mt element between them
			
			Point currentLast = (Point)current.pointChain.getLast();
			Point currentLastButOne = (Point)current.pointChain.get(current.pointChain.size()-2);
			Point nextFirst = (Point)next.pointChain.getFirst();
			Point nextSecond = (Point)next.pointChain.get(1);
			if(currentLast.equal(nextSecond) &&
			   currentLastButOne.equal(nextFirst)) {
			    //now we have exactly that case
			    //add points of next to current
			    for (int ad = 2; ad < next.pointChain.size(); ad++) {
				current.pointChain.add((Point)next.pointChain.get(ad));
			    }//for ad
			    
			    //remove yElems
			    ystruct.remove(yElemPos+2);
			    ystruct.remove(yElemPos+1); //remove mt element
			}//if
		    }//if
		    
		}//if			
		
		
		//now all operations for attribute=="bend" are done
		
		
		else {
		    
		    if (attribute == "end") {
			//find the corresponding segments seg1, seg2
			first = false;

			//find the proper position in ystruct
			yElemPos = interval(x, ystruct);
			
			SweepStElem yElem = (SweepStElem)ystruct.get(yElemPos);
			//determine whether x really lies in the interval and not only on the border
			
			if (!yElem.pointChain.isEmpty()) {
			    if (x.equal((Point)yElem.pointChain.getFirst()) &&
				x.equal((Point)yElem.pointChain.getLast())) {
				//x lies on the left and on the right border
				//leftright = true;
			    }//if
			    else {
				if (x.equal((Point)yElem.pointChain.getFirst())) {
				    //x lies on the left border
				    yElemPos--;
				    yElem = (SweepStElem)ystruct.get(yElemPos);
				}//if
				else {
				    if (x.equal((Point)yElem.pointChain.getLast())) {
					//x lies on the right border
					yElemPos++;
					yElem = (SweepStElem)ystruct.get(yElemPos);
				    }//if
				}//else
			    }//else
			}//if
			
			if (yElem.pointChain.isEmpty()) {
			    //x lies in an "out" interval of ystruct
			    //compute triangulation segments
			
			    //find out on which border of the neighbour swstelems the
			    //point lies and set prev,foll
			    SweepStElem prev = (SweepStElem)ystruct.get(yElemPos-1);
			    SweepStElem foll = (SweepStElem)ystruct.get(yElemPos+1);
			    boolean prevLiesTop = false;
			    boolean follLiesTop = false;
			    boolean computedAnyPrev = false;
			    boolean computedAnyFoll = false;
			    if (((Point)prev.pointChain.getFirst()).equal(x)) { prevLiesTop = true; }
			    if (((Point)foll.pointChain.getFirst()).equal(x)) { follLiesTop = true; }
			    
			    //depending on prevLiesTop,follLiesTop the pointChains are
			    //parsed from beginning to end or reverse
			    
			    int pCprev = -1;
			    int pCfoll = -1;
			    if (prevLiesTop) { pCprev = 1; }
			    else { pCprev = prev.pointChain.size()-2; }
			    if (follLiesTop) { pCfoll = 1; }
			    else { pCfoll = foll.pointChain.size()-2; }
			    
			    //compute triangles for prev
			    if (prevLiesTop) {
				for (int j = 1; j < prev.pointChain.size()-2; j++) {
				    
				    boolean compute = false;
				    if (Mathset.pointPosition(x,(Point)prev.pointChain.get(j+1),(Point)prev.pointChain.get(j)) == -1) { compute = true; }
				    if (compute) {
					Triangle nt = new Triangle(x,
								   (Point)prev.pointChain.get(j+1),
								   (Point)prev.pointChain.get(j));
					tl.add(nt);
				
					pCprev = j+1;
					computedAnyPrev = true;
				    }//if
				    else { break; }
				}//for j
			    }//if
			    else {
				for (int j = prev.pointChain.size()-2; j > 1; j--) {
				    boolean compute = false;
				    if (Mathset.pointPosition(x,(Point)prev.pointChain.get(j-1),(Point)prev.pointChain.get(j)) == 1) { compute = true; }
				    if (compute) {
					Triangle nt = new Triangle(x,
								   (Point)prev.pointChain.get(j-1),
								   (Point)prev.pointChain.get(j));
					tl.add(nt);
					pCprev = j-1;
					computedAnyPrev = true;
				    }//if
				    else { break; }
				}//for j
			    }//else
			    
			    
			    //compute triangles for foll
			    if (!follLiesTop) {
				for (int j = foll.pointChain.size()-2; j > 1; j--) {
				    boolean compute = false;
				    if (Mathset.pointPosition(x,(Point)foll.pointChain.get(j-1),(Point)foll.pointChain.get(j)) == 1) { compute = true; }
				    if (compute) {
					Triangle nt = new Triangle(x,
								   (Point)foll.pointChain.get(j-1),
								   (Point)foll.pointChain.get(j));
					tl.add(nt);
					pCfoll = j-1;
					computedAnyFoll = true;
				    }//if
				    else { break; }
				}//for j
			    }//if
			    else {
				for (int j = 1; j < foll.pointChain.size()-2; j++) {
				    boolean compute = false;
				    if (Mathset.pointPosition(x,(Point)foll.pointChain.get(j+1),(Point)foll.pointChain.get(j)) == -1) { compute = true; }
				    if (compute) {
			    Triangle nt = new Triangle(x,
						       (Point)foll.pointChain.get(j+1),
						       (Point)foll.pointChain.get(j));
			    tl.add(nt);
			    pCfoll = j+1;
			    computedAnyFoll = true;
				    }//if
				    else
					break;
				}//for j
				
			    }//else
			    
			    			    
			    //update ystruct
			    //build new SweepStElem with the remaining points
			    //and substitute it with the old three elements
			    //new = n
			    //(mt a mt b mt) -> (mt n mt)
			    
			    newElem = new SweepStElem();
			    newElem.rimoEl = x;
			    //first add all remaining elements of prev
			    if (!computedAnyPrev) {
				if (prevLiesTop) {
				    for (int j = prev.pointChain.size()-1; j > 0; j--) {
					newElem.pointChain.add(((Point)prev.pointChain.get(j)).copy());
				    }//for j
				}//if
				else {
				    for (int j = 0; j < prev.pointChain.size()-1; j++) {
					newElem.pointChain.add(((Point)prev.pointChain.get(j)).copy());
				    }//for j
				}//if
			    }//if
			    else {
				if (prevLiesTop) {
				    for (int j = prev.pointChain.size()-1; j > pCprev-1; j--) {
					newElem.pointChain.add(((Point)prev.pointChain.get(j)).copy());
				    }//for j
				}//if
				else {
				    for (int j = 0; j < pCprev+1; j++) {
					newElem.pointChain.add(((Point)prev.pointChain.get(j)).copy());
				    }//for j
				}//else
			    }//else
			    
			    //now add x
			    newElem.pointChain.add((Point)x.copy());
			    
			    //finally add all remaining elements of foll
			    if (!computedAnyFoll) {
				if (follLiesTop) {
				    for (int j = 1; j < foll.pointChain.size(); j++) {
					newElem.pointChain.add(((Point)foll.pointChain.get(j)).copy());
				    }//for j
				}//if
				else {
				    for (int j = foll.pointChain.size()-2; j > -1; j--) {
					newElem.pointChain.add(((Point)foll.pointChain.get(j)).copy());
				    }//for j
				}//else
			    }//if
			    else {
				if (follLiesTop) {
				    for (int j = pCfoll; j < foll.pointChain.size(); j++) {
					newElem.pointChain.add(((Point)foll.pointChain.get(j)).copy());
				    }//for j
				}//if
				else {
				    for (int j = pCfoll; j >= 0; j--) {
					newElem.pointChain.add(((Point)foll.pointChain.get(j)).copy());
				    }//for j
				}//else
			    }//else
			    
			    //SPECIAL CASE: A certain case is not included in the algorithm described
			    //by Mehlhorn. A point with attribute 'end' may be found as lying in an 
			    //'out'-interval (as it is implemented now) but in fact is lying in an 'in'-
			    //interval. This is correctly found when using the interval2-method and 
			    //checking, whether both endpoints of the interval have the same endpoint.
			    //If not, we found the special case. This is implemented in the following.
			    
			    //find x in pointChain
			    int xPos = -1;
			    for (int pc = 1; pc < newElem.pointChain.size()-2; pc++) {
				if (x.equal((Point)newElem.pointChain.get(pc))) {
				    xPos = pc;
				    break;
				}//if
			    }//for pc
			    
			    //generate all possible triangles for special case.
			    //The special case can be detected, when examining the intersection points
			    //of the outermost segments of the interval with the sweep-line. If x lies
			    //in the middle of the other points, it lies in an "out"-interval. Thus
			    //the special case occurs in the other case.
			    if (newElem.pointChain.size() > 4) {
				//constructing intersection points
				//find max/min y values
				
				Point p01 = (Point)newElem.pointChain.getFirst();
				Point p02 = (Point)newElem.pointChain.get(1);
				Point p03 = (Point)newElem.pointChain.get(newElem.pointChain.size()-2);
				Point p04 = (Point)newElem.pointChain.getLast();
				LinkedList pl = new LinkedList();
				pl.add(p01);
				pl.add(p02);
				pl.add(p03);
				pl.add(p04);
				//sort in respect to y value
				Point actPoint = null;
				for (int m = 0; m < pl.size()-1; m++) {
				    actPoint = (Point)pl.get(m);
				    for (int n = m; n < pl.size(); n++) {
					if (actPoint.compY((Point)pl.get(n)) == 1) {
					    Point swap = actPoint;
					    pl.set(m,pl.get(n));
					    pl.set(n,swap);
					    actPoint = (Point)pl.get(m);
					}//if
				    }//for n
				}//for m
				
				Rational minY,maxY;
				minY = ((Point)pl.getFirst()).y.minus(1);
				maxY = ((Point)pl.getLast()).y.plus(1);
				
				//constructing sweepline
				Segment sweepline = new Segment(x.x,minY,x.x,maxY);
				
				//constructing intersection points
				Point int1 = sweepline.intersection(new Segment(p01,p02));
				Point int2 = sweepline.intersection(new Segment(p03,p04));
				
				//now determine, whether we have a special case
				
				if ((x.y.less(int1.y) && x.y.less(int2.y)) ||
				    (x.y.greater(int1.y) && x.y.greater(int2.y))) {				    
				    
				    boolean topdown = false;
				    if (((Point)newElem.pointChain.get(1)).compY((Point)newElem.pointChain.get(newElem.pointChain.size()-2)) == 1) {
					topdown = true;
				    }//if
				    
				    
				    //for the pointPosition test we have to find out, 
				    //whether we are going top-down or vice-versa
				    //use topdown for this
				    delList = new LinkedList();
				    if (topdown) {
					for (int k = 1; k < newElem.pointChain.size()-3; k++) {
					    Point p1 = (Point)newElem.pointChain.get(k);
					    Point p2 = (Point)newElem.pointChain.get(k+1);
					    Point p3 = (Point)newElem.pointChain.get(k+2);

					    if (Mathset.pointPosition(p2,p1,p3) == 1) {
						Triangle nt = new Triangle(p1,p2,p3);
						tl.add(nt);
						delList.add(p2);
					    }//if
					}//for k
					//deleting points using delList
					boolean found = false;
					for (int k = 0; k < delList.size(); k++) {
					    Point delPoint = (Point)delList.get(k);
					    found = false;
					    for (int lc = 1; lc < newElem.pointChain.size()-2; lc++) {
						if (((Point)newElem.pointChain.get(lc)).equal(delPoint)) {
						    found = true;
						    newElem.pointChain.remove(lc);
						    break;
						}//if
					    }//for lc
					    if (found) delList.remove(k);
					    
					    else {
						System.out.println("Error in P.computeTriangles: Didn't find point (to remove) in pointChain.");
						throw new RuntimeException("An error occurred in the 2DSACK package.");
					    }//else
					}//for k
				    }//if
				    else {
					for (int k = newElem.pointChain.size()-3; k > 1; k--) {
					    Point p1 = (Point)newElem.pointChain.get(k);
					    Point p2 = (Point)newElem.pointChain.get(k-1);
					    Point p3 = (Point)newElem.pointChain.get(k-2);

					    if (Mathset.pointPosition(p2,p1,p3) == -1) {
						Triangle nt = new Triangle(p1,p2,p3);
						tl.add(nt);
						delList.add(p2);
					    }//if
					}//for k
				    }//else
				    
				    //deleting points using delList
				    boolean found = false;
				    for (int k = 0; k < delList.size(); k++) {
					Point delPoint = (Point)delList.get(k);
					found = false;
					for (int lc = 1; lc < newElem.pointChain.size()-2; lc++) {
					    if (((Point)newElem.pointChain.get(lc)).equal(delPoint)) {
						found = true;
						newElem.pointChain.remove(lc);
						break;
					    }//if
					}//for lc
					if (found) delList.remove(k);
					else {
					    System.out.println("Error in P.computeTriangles: Didn't find point (to remove) in pointChain.");
					    throw new RuntimeException("An error occurred in the 2DSACK package.");
					}//else
				    }//for k
				    
				    //set new rimoEl
				    //find rimoEl
				    Point rmL = (Point)newElem.pointChain.get(1);;
				    for (int pc = 1; pc < newElem.pointChain.size()-2; pc++) {
					if (rmL.compX((Point)newElem.pointChain.get(pc)) == -1) {
					    rmL = (Point)newElem.pointChain.get(pc); }
				    }//for pc
				    newElem.rimoEl = rmL;
				}//if
			    }//if	   

			    ystruct.set(yElemPos,newElem);
			    ystruct.remove(yElemPos+1);
			    ystruct.remove(yElemPos-1);
			}//if
			
			else {
			    //x lies in an "in" interval of ystruct
			    
			    //compute triangles
			    
			    for (int j = 1; j < yElem.pointChain.size()-2; j++) {
				Triangle nt = new Triangle(x,
							   (Point)yElem.pointChain.get(j),
							   (Point)yElem.pointChain.get(j+1));
				tl.add(nt);
				
				//now add points to delList
				if (!(((Point)yElem.pointChain.get(j+1)).equal((Point)yElem.pointChain.getLast()))) { delList.add(yElem.pointChain.get(j+1)); }
			    }//for
			    
			    //update ystruct
			    //remove points in delList from yElem
			    //buggy? delete all(!) elements which are not first or last
			    
			    //??? delete this yElem and one of the neighbour mt-elements from ystruct

			    ystruct.remove(yElemPos);
			    ystruct.remove(yElemPos-1);
			}//else
		    }//if
		    //now all operations for attribute=="end" are done
		    
		}//else
	    }//else
	    
	    //clean up ystruct: delete rows of mt-elements
	    if (ystruct.size() > 1) {
		for (int j = ystruct.size()-1; j > 0; j--) {
		    if ((((SweepStElem)ystruct.get(j)).pointChain.size() == 0) &&
			(((SweepStElem)ystruct.get(j-1)).pointChain.size() == 0)) {
			ystruct.remove(j);
		    }//if
		}//for j
	    }//if
	    
	}//for i
	//now the sweep is done
	
	return tl;
    }//end method comuteTrianglesSUB
    
    /**
     * Supportive method for method <code>computeTrianglesSUB</code>.
     * Gets a {@link Point} of the Polygons' border and a {@link LinkedList}, which represents
     * the sweep status structure (SSS). The SSS looks like this:
     * <p>
     * [empty list] [pn...pm] [empty list] ... [empty list] [pp...pq] [emptylist]
     * <p>
     * The return value is the position of the interval that includes the point <code>p</code>.
     * Note, that the first
     * and last elements of such intervals in the SSS are not really part of the interval. Therefore,
     * if e.g. <code>p</code> = <code>pn</code> then 0 is returned as index of the first (empty) interval.
     *
     * @param p the query point
     * @param ystruct the SSS for the sweep-line algorithm
     * @return the index of the proper interval as int
     */
    private static int interval(Point p, LinkedList ystruct) {
	boolean foundOne = false;
	boolean foundTwo = false;
	int marker = 0;
	int marker2 = 0;
	
	if (ystruct.isEmpty()) {
	    ystruct.add(new SweepStElem());
	    return 0;
	}//if

	//Search the intervals borders for the element
	//One element may occur two times on the borders, which means
	//it is the connecting element. Hence we search for the element
	//two times.
	for (int i = 0; i < ystruct.size(); i++) {
	    SweepStElem actSSE = (SweepStElem)ystruct.get(i);
	    if (!actSSE.pointChain.isEmpty()) {
		if (((Point)actSSE.pointChain.getFirst()).equal(p)) {
		    //first = true;
		    if (!foundOne) { 
			marker = i;
			foundOne = true;
		    }//if
		    else {
			marker2 = i;
			foundTwo = true;
		    }//else
		}//if
		if (((Point)actSSE.pointChain.getLast()).equal(p)) {
		    //last = true;
		    if (!foundOne) {
			marker = i; 
			foundOne = true;
		    }//if
		    else {
			marker2 = i;
			foundTwo = true;
		    }//else
		}//if
		if (((Point)actSSE.pointChain.get(1)).equal(p)) {
		    //second = true;
		    if (!foundOne) {
			marker = i;
			foundOne = true;
		    }//if
		    else {
			marker2 = i; 
			foundTwo = true;
		    }//if
		}//if
		if (((Point)actSSE.pointChain.get(actSSE.pointChain.size()-2)).equal(p)) {
		    //lastButOne = true;
		    if (!foundOne) {
			marker = i;
			foundOne = true;
		    }//if
		    else {
			marker2 = i;
			foundTwo = true;
		    }//else
		}//if
		if (foundTwo) { break; }
	    }//if
	}//for i

	if (foundOne || foundTwo) {
	    if (foundOne && foundTwo) {
		//we found the element two times
		if (marker == marker2) {
		    //both indices are equal, so everythings allright
		    return marker;
		}//if
		if (marker2 != (marker+2)) {
		    //indices should be equal or have a difference of 2
		    //if not, there is a serious error
		    System.out.println("P.interval: 1+1=3? While searching for the proper interval to insert a new SweepStElem, two intervals were found, but they are too far away from eachother!");
		    System.out.println("marker:"+marker+", marker2:"+marker2);
		    throw new RuntimeException("An error occurred in the 2DSACK package.");
		}//if
		return marker+1;
	    }//if
	    return marker;
	}//if

	//so p is not lying on the boundary of one of the intervals
	//now it is checked whether its y-coordinate is lying inside of
	//the boundaries
	else {
	    boolean inside = false;
	    boolean outside = false;
	    Rational ypos = p.y;

	    for (int i = 0; i < ystruct.size(); i++) {
		SweepStElem actSSE = (SweepStElem)ystruct.get(i);
		if (!actSSE.pointChain.isEmpty()) {
		    //does p lie in the "in" interval?
		    Point g11 = (Point)actSSE.pointChain.get(0);
		    Point g12 = (Point)actSSE.pointChain.get(1);
		    Point g21 = (Point)actSSE.pointChain.getLast();
		    Point g22 = (Point)actSSE.pointChain.get(actSSE.pointChain.size()-2);
		    if (((Mathset.pointPosition(g11,g12,p) == -1) &&
			 (Mathset.pointPosition(g21,g22,p) == 1)) ||
			((Mathset.pointPosition(g11,g12,p) == 1) &&
			 (Mathset.pointPosition(g21,g22,p) == -1))) {
			marker = i;
			inside = true;
			break;
		    }//if
		    //ypos is smaller than the next not mt interval,
		    //so return position of last mt interval
		    if (ypos.less(((Point)actSSE.pointChain.get(1)).y) &&
			ypos.less(((Point)actSSE.pointChain.get(actSSE.pointChain.size()-2)).y)) {
			if (i > 0) {
			    marker = i-1;
			    outside = true;
			    break;
			}//if
		    }//if
		}//if
	    }//for i
	    if (inside || outside) {
		return marker;
	    }//if
	    
	}//else

	return ystruct.size()-1;
    }//end method interval
		    
			
    /**
     * Alternative method for interval (supportive method for computeTrianglesSUB).
     * The signature is the same, but it is implemented to handle one special case,
     * namely <code>attribute = start</code>. It is not clear whether it should be used
     * in other cases, too, and whether it works correct.
     *
     * @param p query point
     * @param ll sweep status structure
     * @return interval position as int
     */
    private static int interval2(Point p, LinkedList ll) {
	Rational ypos = RationalFactory.constRational(p.y);
	
	if (ll.isEmpty()) {
	    //ll is empty, so add an empty element to ll and return it
	    SweepStElem newOne = new SweepStElem();
	    ll.add(newOne);
	    return 0;
	}//if
	else {
	    
	    //there is a problem with the borders of intervals:
	    //it may occur that a point is supposed to lie in an interval
	    //that has the proper boundaries but that's only for the reason
	    //that not the proper boundaries but the following points on the
	    //boundaries are stored! for this reason we first check whether
	    //a given point really lies in an interval (by checking all(!)
	    //points of the pointchain). this takes a while but prevents errors.
	    //solution: don't check the given first and last element but the elements
	    //on positions +1 and -1 respectively
	    
	    boolean found = false;
	    
	    for (int i = 0; i < ll.size(); i++) {
		if (!(((SweepStElem)ll.get(i)).pointChain.isEmpty())) {
		    for (int j = 1; j < ((SweepStElem)ll.get(i)).pointChain.size()-1; j++) {
			if (p.equal((Point)((SweepStElem)ll.get(i)).pointChain.get(j))) {
			    found = true;
			    break;
			}//if
		    }//for j
		    if (found) { 
			return i;
		    }//if
		}//if
	    }//for i
	    
	    //p was not found so it may lie on the interval boundaries
	    //repeat the search
	    
	    for (int i = 0; i < ll.size(); i++) {
		if (!(((SweepStElem)ll.get(i)).pointChain.isEmpty())) {
		    for (int j = 0; j < ((SweepStElem)ll.get(i)).pointChain.size(); j++) {
			if (p.equal((Point)((SweepStElem)ll.get(i)).pointChain.get(j))) {
			    found = true;
			    break;
			}//if
		    }//for j
		    if (found) { 
			return i;
		    }//if
		}//if
	    }//for i
	    
	    //if the element was not found, do the old code:
	    if (!found) {
		for (int i = 0; i < ll.size(); i++) {
		    if (!(((SweepStElem)ll.get(i)).pointChain.isEmpty())) {
			//p lies in the "in" interval
			if (((ypos.greater(((Point)((SweepStElem)ll.get(i)).pointChain.get(0)).y)) &&
			     (ypos.less(((Point)((SweepStElem)ll.get(i)).pointChain.get(((SweepStElem)ll.get(i)).pointChain.size()-1)).y))) ||
			    ((ypos.equal(((Point)((SweepStElem)ll.get(i)).pointChain.getFirst()).y)) ||
			     (ypos.equal(((Point)((SweepStElem)ll.get(i)).pointChain.getLast()).y))))
			    {
				return i;
			    }//if
			//p is smaller than the next "in" interval, so return the last "out" interval
			if ((ypos.less(((Point)((SweepStElem)ll.get(i)).pointChain.get(0)).y)) &&
			    (ypos.less(((Point)((SweepStElem)ll.get(i)).pointChain.get(((SweepStElem)ll.get(i)).pointChain.size()-1)).y))) {
			    { 
				if (i > 0) {
				    return (i-1); } }//if
			}//if
		    }//if
		}//for i
	    }//else
	    //no proper interval was found so return the last interval
	    //(which hopefully is an "out" interval)

	    return (ll.size()-1);
	}//else
    }//end method interval
    

    /**
     * Alternative method for <code>interval</code> and <code>interval2</code>.
     * Supportive method for <code>computeTrianglesSUB</code>.
     * It (naturally) has the same signature as both method mentionend above.
     * This method uses a completely different approach than both others: It uses
     * a sweepline to find the proper interval in the interval list in the sweep status structure.
     * Both other methods are supposed not to work correct in all special cases.
     *
     * @param p query point
     * @param ll sweep status structure
     * @return the index of the correct interval as int
     */
    private static int interval3(Point p, LinkedList ll) {
	//compute a list of pairs which are the _real_ borders of the intervals
	//this means the intersection points of the sweep line and the 'border' segments

	LinkedList realInts = new LinkedList();
	ListIterator lit = ll.listIterator(0);
	SweepStElem actSSE = null;
	while (lit.hasNext()) {
	    actSSE = (SweepStElem)lit.next();
	    if (actSSE.pointChain.isEmpty()) {
		realInts.add(new ElemPair());
	    }//if
	    else {
		//compute sweepline
		LinkedList pll = new LinkedList();
		pll.add(actSSE.pointChain.getFirst());
		pll.add(actSSE.pointChain.get(1));
		pll.add(actSSE.pointChain.get(actSSE.pointChain.size()-1));
		pll.add(actSSE.pointChain.get(actSSE.pointChain.size()-2));
		//find min,max in respect to y coordinate
		Point max = (Point)pll.getFirst();
		Point min = (Point)pll.getFirst();
		for (int i = 1; i < pll.size(); i++) {
		    if (((Point)pll.get(i)).compY(min) == -1) min = (Point)pll.get(i);
		    else if (((Point)pll.get(i)).compY(max) == 1) max = (Point)pll.get(i);
		}//for i
		Segment sweepline = new Segment(p.x,min.y.minus(1),p.x,max.y.plus(1));

		//compute both intersection points

		//CAUTION: The computation of intersection points using the sweepline
		//doesn't work if the segments are vertical! This must be checked first.

		Point int1 = null;
		if (((Point)actSSE.pointChain.getFirst()).compX((Point)actSSE.pointChain.get(1)) == 0) {
		    int1 = (Point)actSSE.pointChain.getFirst();
		}//if
		else {
		    int1 = sweepline.intersection(new Segment((Point)actSSE.pointChain.getFirst(),
								    (Point)actSSE.pointChain.get(1)));
		}//else
		Point int2 = null;
		if (((Point)actSSE.pointChain.get(actSSE.pointChain.size()-1)).compX((Point)actSSE.pointChain.get(actSSE.pointChain.size()-2)) == 0) {
		    int2 = (Point)actSSE.pointChain.get(actSSE.pointChain.size()-1);
		}//if
		else {		    
		    int2 = sweepline.intersection(new Segment((Point)actSSE.pointChain.get(actSSE.pointChain.size()-1),
								    (Point)actSSE.pointChain.get(actSSE.pointChain.size()-2)));
		}//else
		
		//construct elemPair and add it to realInts
		if (int1.compY(int2) == -1) realInts.add(new ElemPair(int1,int2));
		else realInts.add(new ElemPair(int2,int1));
	    }//else
	}//while lit

	//now check, whether p _really_ lies in an interval or not and return the proper value
	//It is assumed, that the intervals are sorted correctly!
	lit = realInts.listIterator(0);
	while (lit.hasNext()) {
	    //get actual interval:
	    ElemPair actPair = (ElemPair)lit.next();
	    if (actPair.first != null) {
		//if interval is not mt read borders
		Point first = (Point)actPair.first;
		Point second = (Point)actPair.second;

		//compute compare values
		int pCompY1st = p.compY(first);
		int pCompY2nd = p.compY(second);

		//point p lies in between the borders, return interval index:
		if ((pCompY1st == 1) && (pCompY2nd == -1)) return lit.nextIndex()-1;

		//point p is found as first and last point of actual interval AND
		//it is first and last point of the previous non-mt interval
		//then return index of the previous mt interval
		else if (((pCompY1st == 0) || (pCompY2nd == 0)) &&
			 (lit.nextIndex()-3 > 0) &&
			 ((p.compY(((ElemPair)realInts.get(lit.nextIndex()-3)).first) == 0) ||
			  (p.compY(((ElemPair)realInts.get(lit.nextIndex()-3)).second) == 0))) return lit.nextIndex()-2;
		
		//point p is found as first and last point of actual interval
		//return interval index:
		else if ((pCompY1st == 0) && (pCompY2nd == 0)) return lit.nextIndex()-1;

		//p is found as first
		else if (((pCompY1st == 0) && (pCompY2nd != 0)) ||
			 ((pCompY1st != 0) && (pCompY2nd == 0))) return lit.nextIndex()-1;

		//y coordinate of p is smaller than interval borders
		//return index of previous mt interval: 
		else if ((pCompY1st < 0) && (pCompY2nd < 0)) return lit.nextIndex()-2;
		
	    }//if
	}//while lit

	//no proper interval was found, so return last index
	return realInts.size()-1;
    }//end method interval3
	    
						 
    /**
     * Determines whether a query point has the attribute "start", "end" or "bend".
     * This is a supportive method for computeTrianglesSUB.
     * The result is computed by examining the neighbour points of query point <code>x</code>
     * on the border.
     *
     * @param x the query point
     * @param xVertex the vertex for <tt>x</tt> in <tt>graph</tt>
     * @param graph the graph representing the region
     * @return the proper result, which is "start", "end" or "bend"
     * @throws NoProperCyclesException if the segments don't form proper cycles
     */
    private static String attribute(Point x, Vertex xVertex, Graph graph) throws NoProperCyclesException {
	//get the vertex from graph
	//Vertex xVertex = graph.getVertex(x);
	
	//get neighbours from graph
	Vertex[] neighbours = graph.getNeighbours(xVertex);
	
	//if neighbours has more or less than two elements, exit
	if (neighbours.length != 2) {
	    /* DON'T DELETE THIS: IT CAN BE USED FOR DEBUGGING
	      System.out.println("Polygons.attribute: Border doesn't form proper cycles. Exit.");
	      System.out.println("x: "+x);
	      System.out.println("neighbour's array:");
	      for (int i = 0; i < neighbours.length; i++) {
	      System.out.println("["+i+"] "+neighbours[i].value);
	      }//for i
	    */
	    throw new NoProperCyclesException("The passed set of segments doesn't form proper cycles.");
	}//if

	//extract both vertices from array
	Vertex vert1 = neighbours[0];
	Vertex vert2 = neighbours[1];
	
	//compute comparison values
	Point point1 = (Point)vert1.value;
	Point point2 = (Point)vert2.value;
	int quadFirst = quadrant(x,point1);
	int quadSecond = quadrant(x,point2);
	int vertFirst = vertical(x,point1);
	int vertSecond = vertical(x,point2);

	//depending on comparison value, return correct attribute;
	//32 different cases exist, where 8 cases are for START, 
	//another 8 cases are for END; all other cases are BEND cases
	if ((quadFirst == 2 && quadSecond == 2) || //case 8
	    ((quadFirst == 2 && vertSecond == 2) || (quadSecond == 2 && vertFirst == 2)) || //case 9
	    ((quadFirst == 2 && quadSecond == 4) || (quadFirst == 4 && quadSecond == 2)) || //case 10
	    ((quadFirst == 2 && vertSecond == 3) || (quadSecond == 2 && vertFirst == 3)) || //case 11
	    ((quadFirst == 4 && vertSecond == 2) || (quadSecond == 4 && vertFirst == 2)) || //case 15
	    ((vertFirst == 2 && vertSecond == 3) || (vertFirst == 3 && vertSecond == 2)) || //case 16
	    (quadFirst == 4 && quadSecond == 4) || //case 20
	    ((quadFirst == 4 && vertSecond == 3)  || (quadSecond == 4 && vertFirst == 3))) //case 21
	    return "start";

	if (((quadFirst == 3 && vertSecond == 1) || (quadSecond == 3 && vertFirst == 1)) || //case 5
	    ((vertFirst == 1 && vertSecond == 4) || (vertFirst == 4 && vertSecond == 1)) || //case 6
	    ((quadFirst == 1 && vertSecond == 1) || (quadSecond == 1 && vertFirst == 1)) || //case 7
	    (quadFirst == 3 && quadSecond == 3) || //case 28
	    ((quadFirst == 3 && vertSecond == 4) || (quadSecond == 3 && vertFirst == 4)) || //case 29
	    ((quadFirst == 1 && quadSecond == 3) || (quadFirst == 3 && quadSecond == 1)) || //case 30
	    ((quadFirst == 1 && vertSecond == 4) || (quadSecond == 1 && vertFirst == 4)) || //case 31
	    (quadFirst == 1 && quadSecond == 1)) //case 32
	    return "end";
	
	return "bend";
    }//end method attribute
  

    /**
     * For two points <tt>x,p</tt> returns the quadrant of <tt>p</tt> with <tt>x</tt> the origin of the coordinate system.
     * Returns 0 if <tt>p</tt> lies on one of the axis.
     * @param x the coordinate system's origin
     * @param p the query point
     * @return 1..4 if the point lies in one quadrant; 0 if the point lies on one axis 
     */
    private static int quadrant(Point x, Point p) {
	if (p.x.less(x.x)) {
	    if (p.y.greater(x.y))
		return 1;
	    if (p.y.less(x.y))
		return 3;
	    else return 0;
	}
	if (p.x.greater(x.x)) {
	    if (p.y.greater(x.y))
		return 2;
	    if (p.y.less(x.y))
		return 4;
	}//if
	return 0;
    }//end method quadrant

    
    /**
     * For two points <tt>x,p</tt> return a number for a part of the coordinate system's axis on which <tt>p</tt> lies.
     * <tt>x</tt> is the coordinate system's origin. The positive part of the y-axis has number 1. The other numbers
     * are counted clockwise.
     *
     * @param x the coordinate system's origin
     * @param p the query point
     * @return 1..4 if the point lies on one axis; 0 if both points are identical
     */
    private static int vertical(Point x, Point p) {
	if (p.x.equal(x.x)) {
	    if (p.y.greater(x.y))
		return 1;
	    if (p.y.less(x.y))
		return 3;
	    else return 0;
	}//if
	if (p.y.equal(x.y)) {
	    if (p.x.less(x.x))
		return 4;
	    if (p.x.greater(x.x))
		return 2;
	}//if
	return 0;
    }//end method vertical

    
    /**
     * Returns the area of <code>this</code>.
     * If the area was not computed up to the request, it is computed and stored in a class field.
     *
     * @return the area as double
     */
    public double area(){
	if (!this.areaDefined) {
	    this.area = computeArea();
	    this.areaDefined = true;
	}//if
	return this.area;
    }//end method get_area
  

    /**
     * Returns the perimeter of <code>this</code>.
     * If the perimeter was not computed up to the request, it is computed and stored in a class field.
     *
     * @return the perimeter as double
     */
    public double perimeter(){
	if (!this.perimeterDefined) {
	    this.perimeter = computePerimeter();
	    this.perimeterDefined = true;
	}//if
	return this.perimeter;
    }//end method get_perimeter


    /**
     * Sets the set of triangles of <code>this</code> to <code>tlist</code>.
     * Using this method the representation of a <code>Polygons</code> instance can be changed.
     * 
     * @param tlist the new set of triangles as {@link TriMultiSet}
     */
    public void set(TriMultiSet tlist){
	this.trilist = (TriMultiSet)tlist.copy();
	this.trilistDefined = true;
	this.perimeterDefined = false;
	this.areaDefined = false;
	this.borderDefined = false;
	this.bboxDefined = false;
    }//end method set


    /**
     * Returns the border of <code>this</code>.
     * If the border was not computed up to the call of this method, it is computed from the
     * set of triangles, stored in a class field and is returned afterwards.
     * 
     * @return the border as {@link SegMultiSet}
     */
    public SegMultiSet border(){
	if (!this.borderDefined) {
	    this.border = computeBorder();
	    this.borderDefined = true;
	}//if
	return this.border;
    }//end method getBorder

    
    /**
     * Returns the set of triangles representing this <code>Polygons</code> instance.
     * If the triangles set was not computed up to the call of this method, it is computed from the border,
     * stored in a class field and is returned afterwards.
     *
     * @return triangle set as {@link TriMultiSet}
     */
    public TriMultiSet triangles(){
	if (!trilistDefined) {
	    this.trilist = computeTriangles(this.border);
	    this.trilistDefined = true;
	}//if
	TriMultiSet t = TriMultiSet.convert(this.trilist.copy());
	return t;
    }//end method triangles


    /**
     * Makes a real copy of <code>this</code>.
     * All of the class fields are copied (but NOT the bounding box). The copy is returned.
     * Note, that the copy has return type {@link Element}.
     *
     * @return the copy as {@link Element}
     */
    public Element copy(){
	Polygons copy = new Polygons();
	copy.trilist = (TriMultiSet)(this.trilist.copy());
	copy.perimeter = this.perimeter;
	copy.area = this.area;
	copy.border = (SegMultiSet)(this.border.copy());
	copy.trilistDefined = this.trilistDefined;
	copy.perimeterDefined = this.perimeterDefined;
	copy.areaDefined = this.areaDefined;
	copy.borderDefined = this.borderDefined;
	copy.bboxDefined = false;
	return copy;
    }//end method copy


    /**
     * Returns the set of points that are the edges of the polygons' border.
     *
     * @return the set of points as {@link PointMultiSet}
     */
    public PointMultiSet vertices() {
	if (!borderDefined) {
	    this.border = computeBorder();
	    this.borderDefined = true;
	}//if

	PointMultiSet pl = new PointMultiSet(new PointComparator());
	Iterator it = border.iterator();
	MultiSetEntry mse;
	Segment actSeg;
	while (it.hasNext()) {
	    mse = (MultiSetEntry)it.next();
	    actSeg = (Segment)mse.value;
	    pl.add(actSeg.getStartpoint());
	    pl.add(actSeg.getEndpoint());
	}//for i
	try {
	    pl = PointMultiSet.convert(SetOps.rdup(pl));
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in Polygons.vertices:");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Exception Cause: "+e.getCause());
	    System.out.println("Exception String: "+e.toString());
	    e.printStackTrace();
	    throw new RuntimeException("An error occurred in the 2DSACK package.");
	}//catch

	return pl;
    }//end method vertices

    
    /**
     * Returns the distance between two instances of type <code>Polygons</code>.
     * The result is computed as the minimum distance between all pairs of 
     * two triangles of the triangle sets of both <code>Polygons</code>.
     *
     * @param  inElement the instance of <code>Polygons</code> as {@link Element}
     * @return the distance as {@link Rational}
     * @throws WrongTypeException if the passed <code>Element</code> is not of type
     *         <code>Polygons</code>
     */
    public Rational dist (Element inElement) throws WrongTypeException {
	if (inElement instanceof Polygons) {
	    Polygons inPol = (Polygons)inElement;
	    Rational retVal = RationalFactory.constRational(0);
	    Class c = (new Triangle()).getClass();
	    Class[] paramList = new Class[1];

	    //make sure, that trilists already exist for both objects
	    if (!this.trilistDefined) {
		this.trilist = computeTriangles(this.border);
		this.trilistDefined = true;
	    }//if
	    if (!inPol.trilistDefined) {
		inPol.trilist = computeTriangles(inPol.border);
		inPol.trilistDefined = true;
	    }//if

	    try {
		paramList[0] = Class.forName("twodsack.setelement.Element");
		Method m = c.getMethod("dist",paramList);
		ElemPair retPair = SetOps.min(this.trilist,inPol.trilist,m);
		retVal = retPair.first.dist(retPair.second);
	    }//try
	    catch (Exception e) {
		System.out.println("Exception was thrown in Polygons.dist:");
		System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		System.out.println("Exception cause: "+e.getCause());
		System.out.println("Exception String: "+e.toString());
		e.printStackTrace();
		throw new RuntimeException("An error occurred in the 2DSACK package.");
	    }//catch
	    return retVal;
	}//if
	else { throw new WrongTypeException("Expected class Polygons - found "+inElement.getClass()); }
    
    }//end method dist


    /**
     * Checks for intersection of two <code>Polygons</code> instances.
     * If both, <code>this</code> and passed instance have at least one pair of intersecting
     * triangles, <code>true</code> is returned. <tt>false</tt> otherwise.
     *
     * @param  inElement must be an instance of <code>Polygons</code>
     * @return <code>true</code>, if a pair of intersecting triangles exists,
     *         <code>false</code> otherwise
     * @throws WrongTypeException if <code>inElement</code> is not of
     *         type <code>Polygons</code>
     */
    public boolean intersects (Element inElement) throws WrongTypeException {
	if (inElement instanceof Polygons){
	    Polygons inPol = (Polygons)inElement;
	    Class c = (new Triangle()).getClass();
	    PairMultiSet retList = new PairMultiSet(new ElemPairComparator());
	    Class[] paramList = new Class[1];

	    //make sure, that trilists for both objects exist
	    if (!this.trilistDefined) {
		this.trilist = computeTriangles(this.border);
		this.trilistDefined = true;
	    }//if
	    if (!inPol.trilistDefined) {
		inPol.trilist = computeTriangles(inPol.border);
		inPol.trilistDefined = true;
	    }//if

	    try {
		paramList[0] = Class.forName("twodsack.setelement.Element");
		Method m = c.getMethod("intersects",paramList);
		retList = SetOps.overlapJoin(this.trilist,inPol.trilist,m,true,true,false,0);
	    } catch (Exception e) {
		System.out.println("Exception was thrown in Polygons.intersects:");
		System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
		System.out.println("Exceptoin cause: "+e.getCause());
		System.out.println("Exception string: "+e.toString());
		e.printStackTrace();
		throw new RuntimeException("An error occurred in the 2DSACK package.");
	    }//catch
	    if (retList.isEmpty()) { return false; }
	    else { return true; }
	}//if
	else { throw new WrongTypeException("Expected class Polygons - found "+inElement.getClass()); }
    }//end method intersects
    

    /**
     * Prints some of the polygons' values to the standard output.
     * The information written is <tt>perimeter</tt>, <tt>area</tt> and the <tt>triangle set</tt>.
     */
    public void print() {
	System.out.println("Polygons:");
	System.out.println("Perimeter: "+perimeter);
	System.out.println("Area: "+area);
	System.out.println("triangle set:");
	if (!this.trilistDefined) {
	    this.trilist = computeTriangles(this.border);
	    this.trilistDefined = true;
	}//if
	trilist.print();
	System.out.println();
    }//end method print


    /**
     * Returns a value {0, 1, -1} as compare value for two <code>Polygons</code> instances.
     * The comparison of two <tt>Polygons</tt> instances is based on the comparison of their borders. The (sorted) borders are traversed in 
     * parallel. If, at some point, the segment of one border is smaller than the actual segment of the other border, the Polygons instance
     * itself is smaller.<p>
     * Returns 0, if both borders are equal.<p>
     * Returns -1, if <i>this</i> has the 'smaller' border.<p>
     * Returns 1 otherwise.
     *
     * @param inElement the <code>Polygons</code> instance
     * @return 0,1,-1 depending on the mutual position of the instances
     * @throws WrongTypeException if the passed argument is not of type <code>Polygons</code>
     */
    public int compare (ComparableMSE inElement) throws WrongTypeException {
	if (inElement instanceof Polygons) {
	    Polygons inPol = (Polygons)inElement;
	    
	    //make sure that borders for both objects exist
	    if (!this.borderDefined) {
		this.border = this.computeBorder();
		this.borderDefined = true;
	    }//if
	    if (!inPol.borderDefined) {
		inPol.border = inPol.computeBorder();
		inPol.borderDefined = true;
	    }//if
	    
	    Object[] thisArr = this.border.toArray();
	    Object[] inPolArr = inPol.border.toArray();
	    
	    int minLength,res;
	    if (thisArr.length < inPolArr.length)
		minLength = thisArr.length;
	    else
		minLength = inPolArr.length;
	    
	    for (int i = 0; i < minLength; i++) {
		res = ((Segment)thisArr[i]).compare((Segment)inPolArr[i]);
		if (res != 0) return res;
	    }//for i
	    
	    //if all segments are equal and the lengthes are equal, too
	    if (thisArr.length == inPolArr.length)
		return 0;
	    if (thisArr.length < inPolArr.length)
		return -1;
	    else
		return 1;
	}
	else
	    throw new WrongTypeException("Expected class Polygons - found "+inElement.getClass());
    }//end method compare


    /**
     * Returns a value {0, 1, -1} as compare value for two <code>Polygons</code> instances.
     * The comparison of two <tt>Polygons</tt> instances is based on the comparison of their borders. The (sorted) borders are 
     * traversed in parallel. If, at some point, the segment of one border is smaller than the actual segment of the other border (
     * w.r.t. its x coordinate), the Polygons instance itself is said to be smaller.<p>
     * Returns 0, if both borders are equal.<p>
     * Returns -1, if <i>this</i> has the 'smaller' border.<p>
     * Returns 1 otherwise
     *
     * @param inElement the <code>Polygons</code> instance
     * @return 0,1,-1 depending ont he mutual position of the instances
     * @throws WrongTypeException if the passed argument is not of type <code>Polygons</code>
     */
    public byte compX (Element inElement) throws WrongTypeException {
	if (inElement instanceof Polygons) {
	    Polygons inPol = (Polygons)inElement;
	    
	    //make sure that borders for both objects exist
	    if (!this.borderDefined) {
		this.border = this.computeBorder();
		this.borderDefined = true;
	    }//if
	    if (!inPol.borderDefined) {
		inPol.border = inPol.computeBorder();
		inPol.borderDefined = true;
	    }//if
	    
	    Object[] thisArr = this.border.toArray();
	    Object[] inPolArr = inPol.border.toArray();
	    
	    int minLength,res;
	    if (thisArr.length < inPolArr.length)
		minLength = thisArr.length;
	    else
		minLength = inPolArr.length;
	    
	    for (int i = 0; i < minLength; i++) {
		res = ((Segment)thisArr[i]).compX((Segment)inPolArr[i]);
		if (res != 0) return (byte)res;
	    }//for i
	    
	    //if all segments are equal and the lengthes are equal, too
	    if (thisArr.length == inPolArr.length)
		return 0;
	    if (thisArr.length < inPolArr.length)
		return -1;
	    else
		return 1;
	}
	else
	    throw new WrongTypeException("Expected class Plygons - found "+inElement.getClass());
    }//end method compX


    /**
     * Returns a value {0, 1, -1} as compare value for two <code>Polygons</code> instances.
     * The comparison of two <tt>Polygons</tt> instances is based on the comparison of their borders. The (sorted) borders are 
     * traversed in parallel. If, at some point, the segment of one border is smaller than the actual segment of the other border (
     * w.r.t. its y coordinate), the Polygons instance itself is said to be smaller.<p>
     * Returns 0, if both borders are equal.<p>
     * Returns -1, if <i>this</i> has the 'smaller' border.<p>
     * Returns 1 otherwise
     *
     * @param inElement the <code>Polygons</code> instance
     * @return 0,1,-1 depending ont he mutual position of the instances
     * @throws WrongTypeException if the passed argument is not of type <code>Polygons</code>
     */
    public byte compY (Element inElement) throws WrongTypeException {
	if (inElement instanceof Polygons) {
	    Polygons inPol = (Polygons)inElement;
	    
	    //make sure that borders for both objects exist
	    if (!this.borderDefined) {
		this.border = this.computeBorder();
		this.borderDefined = true;
	    }//if
	    if (!inPol.borderDefined) {
		inPol.border = inPol.computeBorder();
		inPol.borderDefined = true;
	    }//if
	    
	    Object[] thisArr = this.border.toArray();
	    Object[] inPolArr = inPol.border.toArray();
	    
	    int minLength,res;
	    if (thisArr.length < inPolArr.length)
		minLength = thisArr.length;
	    else
		minLength = inPolArr.length;
	    
	    for (int i = 0; i < minLength; i++) {
		res = ((Segment)thisArr[i]).compY((Segment)inPolArr[i]);
		if (res != 0) return (byte)res;
	    }//for i
	    
	    //if all segments are equal and the lengthes are equal, too
	    if (thisArr.length == inPolArr.length)
		return 0;
	    if (thisArr.length < inPolArr.length)
		return -1;
	    else
		return 1;
	}
	else
	    throw new WrongTypeException("Expected class Polygons - found "+inElement.getClass());
    }//end method compY


    /**
     * Checks for equality of two <code>Polygons</code> instances.
     * The borders of both instances are compared. If they are equal, <code>true</code> is
     * returned. <code>false</code> otherwise.
     *
     * @param inElement the <code>Polygons</code> instance
     * @return <code>true</code> if equal, <code>false</code> otherwise
     * @throws WrongTypeException if the passed argument is not of type <code>Polygons</code>
     */
    public boolean equal (Element inElement) throws WrongTypeException {
	if (inElement instanceof Polygons) {
	    Polygons inPol = (Polygons)inElement;

	    //make sure that borders for both objects exist
	    if (!this.borderDefined) {
		this.border = this.computeBorder();
		this.borderDefined = true;
	    }//if
	    if (!inPol.borderDefined) {
		inPol.border = inPol.computeBorder();
		inPol.borderDefined = true;
	    }//if

	    if (SetOps.equal(this.border,inPol.border)) {
		return true;
	    }//if
	}//if
	else { throw new WrongTypeException("Expected class Polygon - found "+inElement.getClass()); }
	return false;
    }//end method equal
    

    /**
     * Sorts the segments of <code>inBorder</code> inBorder, starting with <code>x</code>.
     * The segments in <code>inBorder</code> must form one or more cycles. If so, this method sorts them
     * as shown below for a starting point <code>x</code>:
     * <p>
     * (x,a)(a,b)(b,c)...(y,x) (n,m)(m,o)(o,n) (p,q)...(r,p)
     * 
     * @param inBorder the list of segments to be sorted
     * @param x the starting point 
     * @return the sorted list of segments
     */
    private static LinkedList sortBorder(LinkedList inBorder, Point x) {
	if (inBorder.size() == 0) return new LinkedList();

	//construct new graph
	Graph myGraph = new Graph(SegMultiSet.convert(SupportOps.convert(inBorder)));
	//compute cycles of the graph
	CycleList cycles = myGraph.computeFaceCycles();
	//find the cycle with x and move it to first position of retlist
	ListIterator it1 = cycles.listIterator(0);
	LinkedList actList;
	ListIterator it2;
	LinkedList retList = new LinkedList();
	int xpos = -1;
	int cyclePos = -1;
	while (it1.hasNext()) {
	    actList = (LinkedList)it1.next();
	    it2 = actList.listIterator(0);
	    while (it2.hasNext()) {
		if (x.equal(((Segment)it2.next()).getStartpoint())) {
		    xpos = it2.nextIndex() -1;
		    cyclePos = it1.nextIndex() -1;
		    break;
		}//if
	    }//while it2
	}//while it1
	
	if (xpos == -1) {
	    System.out.println("Error in Polygons.sortBorder: Didn't find x in border structure.");
	    throw new RuntimeException("An error occurred in the 2DSACK package.");
	}//if

	//resort found cycle
	//move segments from found xpos up to end to new sorted list
	LinkedList resortedCycle = new LinkedList();
	actList = (LinkedList)cycles.get(cyclePos);
	for (int i = xpos; i < actList.size(); i++)
	    resortedCycle.add(actList.get(i));
	//move segments from 0 up to found xpos to new sorted list
	for (int i = 0; i < xpos; i++) 
	    resortedCycle.add(actList.get(i));
	
	//remove resorted cycle from cyclelist and add at it an first position
	//to cycles
	cycles.remove(cyclePos);
	cycles.addFirst(resortedCycle);
	//add all elements from all cycles to retList
	it1 = cycles.listIterator(0);
	while (it1.hasNext()) {
	    actList = (LinkedList)it1.next();
	    it2 = actList.listIterator(0);
	    while (it2.hasNext()) {
		retList.add(it2.next());
	    }//while it2
	}//while it1

	return retList;
    }//end method sortBorder

    
    /**
     * Returns the cycles of <code>this</code> as a <code>CycleListList</code>.
     * For every component (cycle) there is a single list in the resulting structure.
     * The first element of the <code>CycleListList</code> is the outer cycle of the 
     * <code>Polygons</code> value and all others are holes.
     * Elements of <code>CycleListList</code> are of type {@link Segment}.
     * Note: Structures inside of holes are not supported and may result in erroneous behaviour.
     *
     * @return the <code>CycleListList</code> representing the cycles of <code>this</code>
     */
    public CycleListList cyclesSegments() {
	//compute border from triangles if necessary

	if (!this.borderDefined) this.border = computeBorder();
	
	//if border has no elements, return empty structure
	if (this.border.isEmpty()) return new CycleListList();
	
	Graph myGraph = new Graph(this.border);
	CycleList cycList = myGraph.computeFaceCycles();

	//construct the resulting structure as follows:
	//A CycleListList is a list of CycleList(s) where each of the CycleLists
	//has at least one cycle (the outer cycle of a face). All following cycles
	//are inner cycles and therefore are holes. Since new faces inside of 
	//holes (islands) are not supported currently, no more checks are made for those inner
	//cycles.
	//The algorithm first stores the outmost cycle as the outer cycle of the
	//first face. All subsequent cycles are compared to every first cycle of 
	//the CycleLists stored in CycleListList. If it lies inside of an outer cycle,
	//it is stored as hole cycle in the appropriate CycleList.
	CycleListList retList = new CycleListList();
	CycleList firstList = new CycleList();

	//add first cycle of cycList
	firstList.add(cycList.getFirst());
	cycList.removeFirst();
	retList.add(firstList);
	
	//Store in triSetList the triangulations for every outer cycle
	//That way, they must be computed only once for every face.
	//Do this ONLY, if cycList has more than one cycle!
	if (cycList.size() > 0) {
	    LinkedList triSetList = new LinkedList();
	    MeshGenerator myMG = new MeshGenerator();
	    	    
	    TriMultiSet tmsFirst = null;
	    LinkedList bboxList = null;
	    if (!cycList.isEmpty()) {
		if (myMG.GENERATOR == "NetGen")
		    tmsFirst = myMG.computeMeshWithNetGenHoles(firstList,false);
		else if (myMG.GENERATOR == "Triangle") {
		    tmsFirst = myMG.computeMeshForSingleCycleHoles(firstList,false);
		} else if (myMG.GENERATOR == "Mehlhorn") {
		    tmsFirst = computeTrianglesSUB(CycleList.convert(firstList));
		} else {
		    System.out.println("You chose a non-existing triangulator/mesher.");
		    throw new RuntimeException("An error occurred in the 2DSACK package.");
		}//else
		
		triSetList.add(tmsFirst);
		
		//store in bboxList the bounding boxes for every outer cycle
		//bounding boxes are computed from the triangulations
		bboxList = new LinkedList();
		bboxList.add(tmsFirst.rect());
	    }//if	
	    
	    //now traverse the list and check all other cycles
	    Iterator it = cycList.iterator();
	    Iterator itOuterCycles;
	    CycleList actCL = null;
	    LinkedList actCycle;
	    LinkedList firstCycle;
	    Segment testSeg;
	    Point tp1,tp2;
	    boolean found;
	    TriMultiSet actTMS;
	    Iterator tit;
	    Rect actRect;
	    Iterator bit;
	    while (it.hasNext()) {
		found = false;
		actCycle = (LinkedList)it.next();
		itOuterCycles = retList.iterator();
		tit = triSetList.iterator();
		bit = bboxList.iterator();
		testSeg = (Segment)actCycle.getFirst();
		tp1 = testSeg.getStartpoint();
		tp2 = testSeg.getEndpoint();
		
		while (itOuterCycles.hasNext()) {
		    actCL = (CycleList)itOuterCycles.next();
		    actTMS = (TriMultiSet)tit.next();
		    actRect = (Rect)bit.next();
		    firstCycle = (LinkedList)actCL.getFirst();
		    
		    //first, check whether tp1 or tp2 lie inside of the bounding box,
		    //if true, check for inside

		    if (actRect.covers(tp1) &&
			(inside(tp1,actTMS) ||
			 inside(tp2,actTMS))) {
		    
			found = true;
			break;
		    }//if
		}//while itOuterCycles
		
		//if found=true, move cycle to appropriate CycleList

		if (found) {
		    actCL.add(actCycle);
		    it.remove();
		} else {
		    //found= false, cycle must be an outer cycle of a new face
		    //construct this new face as new CycleList
		    CycleList newFace = new CycleList();
		    newFace.add(actCycle);
		    retList.add(newFace);
		    it.remove();
		    
		    TriMultiSet newTMS = null;
		    
		    if (myMG.GENERATOR == "NetGen")
			newTMS = myMG.computeMeshWithNetGenHoles(newFace,false);
		    else if (myMG.GENERATOR == "Triangle") {
			newTMS = myMG.computeMeshForSingleCycleHoles(newFace,false);
		    } else if (myMG.GENERATOR == "Mehlhorn") {
			newTMS = computeTrianglesSUB(CycleList.convert(newFace));
		    } else {
			System.out.println("You chose a non-existing triangulator/mesher.");
			throw new RuntimeException("An error occurred in the 2DSACK package.");
		    }//else
		    
		    triSetList.add(newTMS);
		    
		    //construct the bounding box for the new outer cycle
		    Rect newRect = newTMS.rect();
		    bboxList.add(newRect);
		}//else
	    }//while it
	}//if cycList.size > 0
	
	return retList;
    }//end method cyclesSegments
	 

    /**
     * Re-implementation of cyclesSegments.
     */
    public CycleListList cyclesSegments2() {
	//compute border from triangles if necessary
	if (!this.borderDefined) this.border = computeBorder();
	
	//if border has no elements, return empty structure
	if (this.border.isEmpty()) return new CycleListList();
	
	Graph myGraph = new Graph(this.border);
	CycleList cycList = myGraph.computeFaceCycles();

	//construct the resulting structure as follows:
	//A CycleListList is a list of CycleList(s) where each of the CycleLists
	//has at least one cycle (the outer cycle of a face). All following cycles
	//are inner cycles and therefore are holes. Since new faces inside of 
	//holes (islands) are not supported currently, no more checks are made for those inner
	//cycles.
	//The algorithm first stores the outmost cycle as the outer cycle of the
	//first face. All subsequent cycles are compared to every first cycle of 
	//the CycleLists stored in CycleListList. If it lies inside of an outer cycle,
	//it is stored as hole cycle in the appropriate CycleList.
	CycleListList retList = new CycleListList();
	CycleList firstList = new CycleList();

	//add first cycle of cycList
	firstList.add(cycList.getFirst());
	cycList.removeFirst();
	retList.add(firstList);

	//if cycList no more entries, we're done
	if (cycList.size() > 0) {
	    //Store in bboxList the bbox for every outer cycle;
	    //store in smsList the SegMultiSets of every cycle.
	    LinkedList bboxList = new LinkedList();
	    LinkedList smsList = new LinkedList();
	    SegMultiSet actSMS = CycleList.convert(firstList);
	    smsList.add(actSMS);
	    bboxList.add(actSMS.rect());
	    
	    //now traverse the cycle list and check all other cycles
	    Iterator it = cycList.iterator();
	    Iterator itOuterCycles,bit,sit;
	    CycleList actCL = null;
	    LinkedList actCycle,firstCycle;
	    Segment testSeg;
	    Point tp1,tp2;
	    boolean found;
	    Rect actRect;
	    
	    while (it.hasNext()) {
		found = false;
		actCycle = (LinkedList)it.next();
		itOuterCycles = retList.iterator();
		bit = bboxList.iterator();
		sit = smsList.iterator();
		testSeg = (Segment)actCycle.getFirst();
		tp1 = testSeg.getStartpoint();
		tp2 = testSeg.getEndpoint();
		
		while (itOuterCycles.hasNext()) {
		    actCL = (CycleList)itOuterCycles.next();
		    actRect = (Rect)bit.next();
		    actSMS = (SegMultiSet)sit.next();
		    firstCycle = (LinkedList)actCL.getFirst();
		    
		    //first, check whether tp1 of tp2 lie inside of the bounding box;
		    //if true, check for inside
		    if (actRect.covers(tp1) &&
			(inside(tp1,actSMS) ||
			 inside(tp2,actSMS))) {
			found = true;
			break;
		    }//if
		}//while itOuterCycles
		
		//if found=true, move cycle to appropriate CycleList
		if (found) {
		    actCL.add(actCycle);
		    it.remove();
		} else {
		    //found=false, cycle must be an outer cycle of a new face;
		    //construct this new face as new CycleList
		    CycleList newFace = new CycleList();
		    newFace.add(actCycle);
		    retList.add(newFace);
		    it.remove();
		    
		    //store sms and bbox for the new face
		    actSMS = SegMultiSet.convert(SupportOps.convert(actCycle));
		    smsList.add(actSMS);
		    bboxList.add(actSMS.rect());
		}//else
	    }//while it
	}//if cycList.size > 0
	
	return retList;
    }//end method cyclesSegments2
	
	


    /**
     * Returns the cycles of <code>this</code> as lists of points.
     * A <code>Polygons</code> instance may be represented as set of cycles, which are sets of segments.
     * Now, all those segments are replaced by points. Thus, a cycle is broken up as shown below:
     * <p>
     * <tt>(a,b)(b,c)(c,a) -> (a,b,c)</tt>
     * <p>
     * Look for more information about the <code>CycleListList</code> structure in class method
     * <code>cyclesSegments</code>.<p>
     * Note: This method is usually used for the conversion to the nested list format used in SECONDO.
     *
     * @return sets of points representing the cycles of the <code>Polygons</code> instance.
     */
    public CycleListListPoints cyclesPoints () {
	CycleListListPoints retList = new CycleListListPoints();
	CycleListList cyc = this.cyclesSegments2();
	//CycleListList cyc2 = this.cyclesSegments2();
	
	//System.out.println("\ncyclesPoints.compare: ");
	//System.out.println(cyc.equal(cyc2));

	ListIterator lit1 = cyc.listIterator(0);
	CycleList actComp;
	while (lit1.hasNext()) {
	    actComp = (CycleList)lit1.next();
	    CycleList actCompPL = new CycleList();
	    LinkedList actCyc;
	    ListIterator lit2 = actComp.listIterator(0);
	    while (lit2.hasNext()) {
		actCyc = (LinkedList)lit2.next();
		actCyc = sortBorder(actCyc,((Segment)actCyc.getFirst()).getStartpoint());
		LinkedList actPL = generatePointList(actCyc);
		actCompPL.add(actPL);
	    }//while
	    retList.add(actCompPL); 
	}//while

	return retList;
    }//end method cyclesPoints

    
    /**
     * Generates a list of points for a cycle of segments.
     * For a cycle of segments (a,b)(b,c)(c,a), a point list (a,b,c) is generated.
     * Note: This is used in class method <code>cyclesPoints</code>.
     *
     * @param segList the list of segments
     * @return the list of points
     */
    private static LinkedList generatePointList (LinkedList segList) {
	LinkedList retList = new LinkedList();
	ListIterator lit = segList.listIterator(0);
	Segment actSeg;
	while (lit.hasNext()) {
	    actSeg = (Segment)lit.next();
	    retList.add(actSeg.getStartpoint().copy());
	}//while
	return retList;
    }//end method generatePointList
    

    /**
     * Computes and returns a triangulation of a set of segments passed as {@link SegMultiSet}.
     * The triangulation is computed using an external meshing algorithm written in C.
     * Before the meshing algorithm itself is executed, the set of single faces of the
     * represented polygons is computed. Then, for each face, the algorithm is executed.
     * The resulting sets of triangles are merged and then returned as result. Using the
     * switch <code>qualityMesh</code> a meshing algorithm or a simple Delauny triangulation
     * is used.
     *
     * @param border of the <code>Polygons</code> value as {@link SegMultiSet}
     * @param qualityMesh if <code>true</code>, use meshing algorith, else use Delauny triangulation
     * @return set of triangles as {@link TriMultiSet}
     * @see #computeTriangles(SegMultiSet)
     */
    public static TriMultiSet computeMesh(SegMultiSet border, boolean qualityMesh) {
	if (border.isEmpty()) return new TriMultiSet(new TriangleComparator());
	
	//compute the cycles of the polygon 
	Polygons myPOL = new Polygons(border);
	CycleListList polCLL = null;
	CycleListList polCLL2 = null;
	try {
	    polCLL = myPOL.cyclesSegments2();
	} catch (Exception e) {
	    e.printStackTrace();
	    System.out.println("\nException caught in Polygons.computeMesh(SegMultiSet,boolean): One reason may be that the segments don't form proper cycles.");
	    System.out.println("Returning empty object.");

	    return new TriMultiSet(new TriangleComparator());
	}//catch
	
	/* now every list in polCLL represents one face (at first position) and all holes
	 * in the following, i.e. (read it top-down)
	 * {
	 * (BorderOfFace1, Hole1, Hole2,..., HoleN),
	 * (BorderOfFace2, Hole1, Hole2,..., HoleM),
	 * ...
	 * (BorderOfFaceS, Hole1, Hole2,..., HoleT),
	 * }
	 * For each CycleList call the meshing algorithm
	 */

	TriMultiSet resultSet = new TriMultiSet(new TriangleComparator());
	Iterator it = polCLL.iterator();
	CycleList actCycleList;
	MeshGenerator myMG = new MeshGenerator();

	while (it.hasNext()) {
	    actCycleList = (CycleList)it.next();
	    if (myMG.GENERATOR == "NetGen")
		resultSet.addAll(myMG.computeMeshWithNetGenHoles(actCycleList,qualityMesh));
	    else if (myMG.GENERATOR == "Triangle") {
		resultSet.addAll(myMG.computeMeshForSingleCycleHoles(actCycleList,qualityMesh));
	    } else if (myMG.GENERATOR == "Mehlhorn") {
		resultSet.addAll(computeTrianglesSUB(CycleList.convert(actCycleList)));
	    } else {
		System.out.println("You chose a non-existing triangulator/mesher.");
		throw new RuntimeException("An error occurred in the 2DSACK package.");
	    }//else

	}//while
	
	return resultSet;
    }//end method computeMesh


    /**
     * Computes and returns a triangulation of a set of segments representing a cycle without holes.
     * The triangulation is computed using an external meshing algorithm written in C. Note: If
     * the number of cycles in the passed <code>border</code> is greater than 1, a 
     * <code>TooManyCyclesException</code> will be thrown.
     *
     * @param border of the <code>Polygons</code> value as {@link SegMultiSet}
     * @return set of triangles as {@link TriMultiSet}
     * @throws TooManyCyclesException if number of cycles > 1
     * @see #computeTriangles(SegMultiSet)
     * @see #computeMesh(SegMultiSet,boolean)
     */
    public static TriMultiSet computeMeshSingleCycle (SegMultiSet border) throws TooManyCyclesException {
	if (border.isEmpty()) return new TriMultiSet(new TriangleComparator());
	try {
	    //System.out.println("\n######################### Pol.computeMeshSingleCycle: calling MG.copmuteMeshWithNetGen.");
	    MeshGenerator myMG = new MeshGenerator();
	    TriMultiSet computedMesh = null;

	    //original line for Triangle
	    //TriMultiSet computedMesh = myMG.computeMeshForSingleCycle(border,false);

	    if (myMG.GENERATOR == "NetGen")
		//line that works for NetGen
		computedMesh = myMG.computeMeshWithNetGen(border,false);
	    else
		computedMesh = myMG.computeMeshForSingleCycle(border,false);

	    return computedMesh;
	} catch (Exception e) {throw new TooManyCyclesException("The number of cycles for this method is limited to 1. The polygon passed probably has more cycles."); }
    }//end method computeMeshSingleCycle


    /**
     * Returns <code>true</code> if a point lies inside of a simple polygon.
     *
     * @param inPoint the point to be tested
     * @param cycle the simple polygon to be tested
     * @return {<code>true</code>,<code>false</code>] depending on the mutual position
     *         of inPoint and cycle
     */
    private static boolean inside (Point inPoint, TriMultiSet tms) {
	Iterator it = tms.iterator();
	Triangle actTri;
	while (it.hasNext()) {
	    actTri = (Triangle)((MultiSetEntry)it.next()).value;
	    if (PointTri_Ops.inside(inPoint,actTri)) return true;
	}//while
	
	return false;
    }//end method inside


    /**
     * Returns <code>true</code>, if the point lies inside of the polygon formed by <code>sms</code>.
     * <code>s</code> is the border of the polygon as a {@link SegMultiSet}.
     * Is an implementation of the well known plumbline algorithm.
     *
     * @param point the point to check
     * @param sms the segment set forming the border of the polygon
     * @return <tt>true</tt>, if the point lies inside of the polygon and <i>not</i> on the border
     **/
    public static boolean inside (Point point, SegMultiSet sms) {
	int noTrueIntersections = 0;
	int noPossibleIntersections = 0;

	Rect smsBbox = sms.rect();
	
	if (point.x.less(smsBbox.llx) || point.x.greater(smsBbox.lrx) ||
	    point.y.less(smsBbox.lly) || point.y.greater(smsBbox.uly))
	    return false;
	
	PLUMBLINE.set(point,new Point(smsBbox.lrx.plus(1),point.y));

	Segment actSeg;
	
	do {
	    Iterator it = sms.iterator();
	    noPossibleIntersections = 0;
	    noTrueIntersections = 0;
	    while (it.hasNext()) {
		actSeg = (Segment)((MultiSetEntry)it.next()).value;
		if (PointSeg_Ops.liesOn(point,actSeg)) 
		    return false;
		else if (PointSeg_Ops.liesOn(actSeg.getStartpoint(),PLUMBLINE) ||
			 (PointSeg_Ops.liesOn(actSeg.getEndpoint(),PLUMBLINE))) {
		    noPossibleIntersections++;
		    PLUMBLINE.set(point,PLUMBLINE.getEndpoint().set(PLUMBLINE.getEndpoint().x,PLUMBLINE.getEndpoint().y.plus(1)));
		}//else
	    else if (PLUMBLINE.pintersects(actSeg))
		noTrueIntersections++;
	    }//while
	} while (noPossibleIntersections != 0);

	if (noPossibleIntersections != 0) {
	    System.out.println("\nPolygons.inside:");
	    System.out.println("noTrueIntersections: "+noTrueIntersections);
	    System.out.println("noPossibleIntersections: "+noPossibleIntersections);
	    SegMultiSet plSet = new SegMultiSet(new SegmentComparator());
	    plSet.add(PLUMBLINE);
	    DisplayGFX gfx1 = new DisplayGFX();
	    gfx1.initWindow();
	    gfx1.addSet(sms);
	    gfx1.addSet(plSet);
	    gfx1.showIt(false);
	    try { int data = System.in.read(); }
	    catch (Exception e) {
		e.printStackTrace();
		throw new RuntimeException("An error occurred in the 2DSACK package.");
	    }
	    gfx1.kill();
	    throw new RuntimeException("An error occurred in the 2DSACK package.");
	}//if
	
	if ((noTrueIntersections % 2) == 1) return true;
	else return false;
    }//end method inside


    /**
     * Returns the hashcode of the first triangle of the set representing the Polygons object.
     *
     * @return the hashcode
     */
    public int hashCode() {
	if (!trilistDefined) {
	    this.trilist = computeTriangles(this.border);
	    this.trilistDefined = true;
	}//if
	return this.trilist.first().hashCode();
    }//end method hashCode
	    

    /**
     * Returns <tt>true</tt>, if both polygons values are equal.
     * Same as method equal, but this one overwrites the Object.hashCode method.
     *
     * @param o the object to compare with
     * @return <tt>true</tt>, if both are equal
     */
    public boolean equals (Object o) {
	return (this.equal((Element)o));
    }//end method equals

} // end class Polygons


  
