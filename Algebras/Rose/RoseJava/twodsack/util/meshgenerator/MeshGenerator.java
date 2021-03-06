/*
 * MeshGenerator.java 2004-11-09
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.util.meshgenerator;

import java.io.*;
import twodsack.io.*;
import twodsack.operation.setoperation.*;
import twodsack.set.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.*;
import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;
import twodsack.util.graph.*;

import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.LinkedList;

/**
 * This class provides the communication with an external meshing algorithm.
 * The meshing algorithm is 
 * written in C. This C function is used via the JNI (Java Native Interface).
 */
public class MeshGenerator {
    /*
     * members
     */
    /**
     * The static member variable <tt>GENERATOR</tt> decides which of two different mesh generator implementations is used.
     * The valid values are <tt>"Triangle"</tt>, <tt>"NetGen"</tt> and <tt>"Mehlhorn"</tt>.<br>
     * The default value is "Mehlhorn".
     */
    //public static String GENERATOR = "Triangle";
    //public static String GENERATOR = "NetGen";
    public static String GENERATOR = "Mehlhorn";

    /*
     * constructors
     */
    /**
     * The standard constructor.
     */
    public MeshGenerator(){}


    /**
     * Loads the MeshGenerator libraries.
     * These are *.dll files generated from the original C/C++ files. The loaded libraries are <tt>MeshGenerator.dll</tt>,
     * <tt>ThirdPartyCode.dll</tt> and <tt>MGNetGen.dll</tt>.
     */
    static {
	//System.out.println(System.getProperty("java.library.path")); //show the library path where the dll-files should be
	//load libraries dependant on system
	String os = System.getProperty("os.name");
	if (os.equals(new String("Linux"))) {
	    System.loadLibrary("MeshGenerator");
	    System.loadLibrary("MeshGeneratorNetGen");
	} else {
	    //must be "Windows"
	    System.loadLibrary("MeshGenerator");
	    System.loadLibrary("ThirdPartyCode");
	    System.loadLibrary("MGNetGen");
	}//else
    }

    private static final int NUMBER_OF_BUCKETS = 499; //choose a prime number

    //declare native methods
    String arguments; //p = reads a poly file, q = quality mesh refinement
    double[] pointlist;
    double[] pointattributelist;
    int[] pointmarkerlist;
    int numberofpoints;
    int numberofpointattributes;
    
    int[]  trianglelist;
    double[] triangleattributelist;
    double[] trianglearealist;
    int[] neighborlist;
    int numberoftriangles;
    int numberofcorners;
    int numberoftriangleattributes;
    
    int[] segmentlist;
    int[] segmentmarkerlist;
    int numberofsegments;
    
    int numberofholes;
    double[] holelist;
    
    int numberofregions;
    double[] regionlist;
    
    /**
     * A native method that calls the C meshing function when <tt>GENERATOR</tt> is set to <tt>"Triangle"</tt>.
     * It only consists of the method declaration, i.e. the signature of the method. For detailed
     * information about all parameters have a look at <code>triangle.h</code>.
     @param arguments the command line switches that can be given to the C function
     @param pointlist the list of points [p1x,p1y,p2x,p2y...pnx,pny]
     @param pointattributelist list of point attriutes (not needed here)
     @param pointmarkerlist list of point markers (not needed here)
     @param numberofpointattributes number of point attributes (not needed here)
     @param trianglelist list of triangles, where the points are actually indices to the pointlist array,
     (idx1,idx5,idx6, idx5,idx6,idx7, ...)
     @param triangleattributelist list of triangle attributes (not needed here)
     @param trieanlgearealist list of triangle areas (not needed here)
     @param neighborlist list with information about neighbourhood (not needed here)
     @param numberoftriangles number of triangles
     @param numberofcorners number of corners (not needed here).
     @param numberoftriangleattributes number of triangle attributes (not needed here). Should be 0.
     @param segmentlist list of segments, where the points are actually indices to the pointlist array,
     (idx1,idx5, idx5,idx6, ...)
     @param segmentmarkerlist list of segment markers (not needed here)
     @param numberofsegments number of segments
     @param numberofholes number of holes of the polygon
     @param holelist a list of points lying inside of the holes, represented as pairs of doubles, one 
     point for every hole: (hole1pointx,hole1pointy, hole2pointx,hole2pointy, ...)
     @param numberofregions number of regions
     @param regionlist list of regional attributes and area constraints (not needed)
     */
    private native double[] triangulate (String arguments,
					double[] pointlist,
					double[] pointattributelist,
					int[] pointmarkerlist,
					int numberofpoints,
					int numberofpointattributes,
					int[] trianglelist,
					double[] triangleattributelist,
					double[] trianglearealist,
					int[] neighborlist,
					int numberoftriangles,
					int numberofcorners,
					int numberoftriangleattributes,
					int[] segmentlist,
					int[] segmentmarkerlist,
					int numberofsegments,
					int numberofholes,
					double[] holelist,
					int numberofregions,
					double[] regionlist
					);
    

    /**
     * A native method that calls the C++ meshing function when <tt>GENERATOR</tt> is set to <tt>"NetGen"</tt>.
     * It only consists of the method declaration, i.e. the signature of the method. For detailed
     * information about all parameters have a look at {@link computeMeshWithNetGenHoles}
     * @param pointsArray all points of the polygon in order [x0,y0, ... ,xn,yn]
     * @param numberOfPoints the total number of points
     * @param lengthArray stores the length of the cycles
     * @param lengthes the size of the lengthArray
     * @param directionArray stores a boolean array for every cycle which indicates, how it is directed
     * @param numberOfDirections the size of the directionArray
     * @return an array which stores the point coordinates of the resulting triangles
     */
    private native double[] triangulateNetGen (double[] pointsArray, int numberOfPoints,
					       int[] lengthArray, int lenghtes,
					       boolean[] directionArray, int numberOfDirections);

    /**
     * Another native method which is used to free the memory used by JNI after the mesh was computed (using Triangle as mesher).
     */
    private native void freeMemory();


    /**
     * A native method that frees the memory used by JNI after a mesh was computed with the NetGen mesher.
     */
    private native void freeMemoryNetGen();

    
    /**
     * Computes a triangulation for a single polygon with any number of holes.
     * The polygon must be given as an <tt>CycleList</tt> that consist of a list of cycles. The first cycle is always the outer cycle of the
     * polygon, whereas all further cycles are holes inside of that outer cycle. The list structure is<p>
     * ( (segments of outer cycle) (segments of 1st hole) ... (segments of nth hole) )<p>
     * From the list, three arrays are computed, which are 1) a point array, 2) a length array and 3) a direction array. The point array simply stores the point
     * coordinates for all cycle points. They are stored in order. The second array stores the lengthes of the cycles, i.e. if the first (face) cycle
     * has 10 points (and therefore uses the array positions 0..19 for in the point array), it stores the integer value 20 (which can be used as
     * an array index for the beginning of the next cycle, if any). The third array stores the 'direction' value for each cycle. The direction of a 
     * cycle tells wether the points are given in clockwise (<tt>true</tt>) or counterclockwise (<tt>false</tt>) order.
     *
     * @param borderCycles the list of cycles representing a polygon
     * @param qualityMesh not implemented for NetGen mesher
     * @return the set of triangles for the polygon
     */
    public TriMultiSet computeMeshWithNetGenHoles(CycleList borderCycles, boolean qualityMesh) {
	int numberOfCycles = borderCycles.size();
	int totalNumberOfPoints = 0;
	int[] lengthArray = new int[numberOfCycles];
	int actSize;
	for (int i = 0; i < borderCycles.size(); i++) {
	    actSize = ((LinkedList)borderCycles.get(i)).size();
	    totalNumberOfPoints += actSize;
	    lengthArray[i] = actSize;
	}//for i

	double[] pointsArray = new double[totalNumberOfPoints*2];
	boolean[] directionArray = new boolean[numberOfCycles];
	LinkedList actCycle;
	Iterator it;
	int arrCount = 0;

	//Traverse the list of cycles and compute the data for the three arrays.
	for (int i = 0; i < numberOfCycles; i++) {
	    actCycle = (LinkedList)borderCycles.get(i);
	    
	    //First, the smallest point is extracted from 'border' together with both segments it connects.
	    //Then, a cycle is computed from 'border'. Depending on the collected points and data it can be
	    //analyzed how the cycle should be _directed_. The direction is stored in the directionArray.
	    //Point coordinates are stored in the pointsArray and cycles lengthes are stored in the lengthArray.
	    
	    boolean direction = false; //false=counterclockwise, true=clockwise
	    Point p = ((Segment)actCycle.getFirst()).getStartpoint(); //smallest point of border
	    Segment smallSeg1 = (Segment)actCycle.getFirst();
	    Segment smallSeg2 = (Segment)actCycle.getLast();
	    boolean seg1vert = smallSeg1.getStartpoint().x.equal(smallSeg1.getEndpoint().x);
	    boolean seg2vert = smallSeg2.getStartpoint().x.equal(smallSeg2.getEndpoint().x);
	    Point a = smallSeg1.theOtherOne(p);
	    Point b = smallSeg2.theOtherOne(p);
	
	    if (seg1vert)
		if (a.y.greater(p.y))
		    direction = false;
		else
		    direction = true;
	    if (seg2vert)
		if (p.y.greater(b.y))
		    direction = false;
		else
		    direction = true;
	    
	    double mpa = ((a.y.minus(p.y)).dividedby(a.x.minus(p.x))).getDouble();
	    double mpb = ((b.y.minus(p.y)).dividedby(b.y.minus(p.y))).getDouble();
	    if (mpa > mpb)
		direction = false;
	    else
		direction = true;
	    
	    //set direction value
	    //invert value if cycle is a hole, i.e. i > 0
	    if (i == 0)
		directionArray[i] = direction;
	    else
		directionArray[i] = !direction;

	    //construct the double array
	    it = actCycle.listIterator(0);
	    Segment actSeg;

	    //store initial point in paramPoints
	    actSeg = (Segment)it.next();
	    pointsArray[arrCount] = actSeg.getStartpoint().x.getDouble();
	    pointsArray[arrCount+1] = actSeg.getStartpoint().y.getDouble();
	    arrCount = arrCount+2;

	    while (it.hasNext()) {
		actSeg = (Segment)it.next();
		pointsArray[arrCount] = actSeg.getStartpoint().x.getDouble();
		pointsArray[arrCount+1] = actSeg.getStartpoint().y.getDouble();
		arrCount = arrCount+2;
	    }//while    
	}//for i
			  
	double[] triResultList = null;

	try {
	    triResultList = new MeshGenerator().triangulateNetGen(pointsArray,pointsArray.length,lengthArray,lengthArray.length,directionArray,directionArray.length);
	} catch (Exception e) {
	    System.out.println("MG.computeMeshWithNetGen: An exception was thrown while executing native triangulateNetGen().");
	}//catch

	TriMultiSet resSet = buildTriangleSet(triResultList);

	return resSet;	
    }//end method computeMeshWithNetGenHoles

    
    /**
     * Compute a triangulation for a polygon without holes.
     * The polygon consists of only one single cycle, namely the face/outer cycle.
     *
     * @param border the border of the polygon
     * @param qualityMesh currently not implemented for NetGen mesher
     * @return the set of triangles for the polygon
     */
    public TriMultiSet computeMeshWithNetGen(SegMultiSet border, boolean qualityMesh) {
	//construct a graph from border
	Graph borderGraph = new Graph(border);
	
	//compute cyclelist
	CycleList cycles = borderGraph.computeFaceCycles();
	if (cycles.size() > 1) {
	    System.out.println("The number of cycles that can be processed in this method must be 1. The actual data has "+cycles.size()+" cycles.");
	    return new TriMultiSet(new TriangleComparator());
	}//if

	return computeMeshWithNetGenHoles(cycles,false);
    }//end method computMeshWithNetGen


    /**
     * Computes a mesh for a polygon (consisting of a single cycle) given by <code>border</code>.
     * Inside of this method all parametric values for the C function are computed and collected.
     * Afterwards the C function <code>triangulate</code> is called (indirectly via the 
     * MeshGenerator.dll). 
     * <p>Note, that this method <i>only</i> works for a polygon represented by a single cycle and
     * <i>must not</i> have holes.
     *
     * @param border the border of a polygon to be 'meshed'
     * @param qualityMesh <code>true</code> if a quality mesh shall be generated, if <code>false</code>
     *                    a simple triangulation is computed
     * @return the mesh, i.e. the set of triangles as <code>TriMultiSet</code>
     * @see #computeMeshForSingleCycleHoles(CycleList,boolean)
     */
    public TriMultiSet computeMeshForSingleCycle (SegMultiSet border,boolean qualityMesh) {
	//a certain set of variables is needed for the C-method
	//compute these variables
	if (qualityMesh) this.arguments = "pqQ";
	else this.arguments = "pQ";

	//pointlist: an array of the (border) points of the form 
	//[x-coord p1][y-coord p1]...[xcoord pn][ycoord pn]
	
	/*
	 * new implementation using class PointLink and hashing
	 */
	//construct the hash table
	int initialCapacity = NUMBER_OF_BUCKETS;
	Hashtable pointsHT = new Hashtable(initialCapacity);
	
	//fill in the points as pointers to original points
	//segmentlist can be computed in parallel
	int[] segmentlist = new int[border.size()*2];
	Iterator sit = border.iterator();
	PointLink htEntry;
	Segment actSeg;
	int counter = 0;
	int segmentCounter = 0;
	while (sit.hasNext()) {
	    actSeg = (Segment)((MultiSetEntry)sit.next()).value;
	    htEntry = new PointLink(actSeg.getStartpoint());
	    //check whether htEntry already exists in hash table
	    //if not, give it a number and insert it
	    //put the proper counter number as index in segmentlist
	    if (!pointsHT.containsKey(htEntry.linkedPoint)) {
		htEntry.number = counter;
		//note: the 'names' aka numbers of points are numbered beginning with 1
		//therefore, we need a "counter+1" here
		segmentlist[segmentCounter] = counter+1;
		segmentCounter++;
		counter++;
		pointsHT.put(htEntry.linkedPoint,htEntry);
	    }//if
	    else {
		//get the number of the point from hash table
		htEntry = (PointLink)pointsHT.get(actSeg.getStartpoint());
		segmentlist[segmentCounter] = htEntry.number+1;
		segmentCounter++;
	    }//else

	    //now do the same as above for the other point of the segment
	    htEntry = new PointLink(actSeg.getEndpoint());
	    if (!pointsHT.containsKey(htEntry.linkedPoint)) {
		htEntry.number = counter;
		segmentlist[segmentCounter] = counter+1;
		segmentCounter++;
		counter++;
		pointsHT.put(htEntry.linkedPoint,htEntry);
	    }//if
	    else {
		htEntry = (PointLink)pointsHT.get(actSeg.getEndpoint());
		segmentlist[segmentCounter] = htEntry.number+1;
		segmentCounter++;
	    }//else
		
	}//while sit

	//construct pointlist from elements of the hashtable
	Enumeration enumr = pointsHT.elements();
	double[] pointlist = new double[counter*2];
	Object elem;
	while (enumr.hasMoreElements()) {
	    elem = enumr.nextElement();
	    pointlist[((PointLink)elem).number*2] = ((PointLink)elem).linkedPoint.x.getDouble();
	    pointlist[((PointLink)elem).number*2+1] = ((PointLink)elem).linkedPoint.y.getDouble();
	}//while enumr

	pointattributelist = new double[pointlist.length/2];
	pointmarkerlist = new int[pointlist.length/2];
	numberofpoints = pointlist.length/2;
	numberofpointattributes = 0;

	trianglelist = null;
	triangleattributelist = null;
	trianglearealist = null;
	neighborlist = null;
	numberoftriangles = 0;
	numberofcorners = 0;
	numberoftriangleattributes = 0;
	
	segmentmarkerlist = null;
	numberofsegments = segmentlist.length/2;
	
	numberofholes = 0;
	holelist = null;

	numberofregions = 0;
	regionlist = null;

	//call C-code
	double[] triResultList = null;
	try {
	    triResultList = new MeshGenerator().triangulate(arguments,
							    pointlist,
							    pointattributelist,
							    pointmarkerlist,
							    numberofpoints,
							    numberofpointattributes,
							    trianglelist,
							    triangleattributelist,
							    trianglearealist,
							    neighborlist,
							    numberoftriangles,
							    numberofcorners,
							    numberoftriangleattributes,
							    segmentlist,
							    segmentmarkerlist,
							    numberofsegments,
							    numberofholes,
							    holelist,
							    numberofregions,
							    regionlist);
	} catch (Exception e) {
	    System.out.println("\nException caught in C++ mesh generator. Returning empty value.");
	    return new TriMultiSet(new TriangleComparator());
	}//catch

	return buildTriangleSet(triResultList);
    }//end method computeMeshForSingleCycle
    
    /**
     * Computes a triangle set from an array of doubles.
     * The C function returns a double array with the coordinates of the triangles. This mehtod
     * simply generates triangles from this array and puts them in a <code>TriMultiSet</code>.
     *
     * @param inArr the double array with the point coordinates
     * @return set of triangles
     */
    private TriMultiSet buildTriangleSet (double[] inArr) {
	TriMultiSet retSet = new TriMultiSet(new TriangleComparator());
	
	if (inArr == null || inArr.length == 0) return retSet;
	
	int count = 0;
	int count3 = 0;
	Point p1,p2,p3;
	Triangle t;
	while ((count) < inArr.length) {
	    p1 = new Point(inArr[count+0],inArr[count+1]);
	    p2 = new Point(inArr[count+2],inArr[count+3]);
	    p3 = new Point(inArr[count+4],inArr[count+5]);
	    t = new Triangle(p1,p2,p3);
	    retSet.add(t);
	    count = count+6;
	}//while

	//free the memory of the point array
	try {
	    if (GENERATOR == "Triangle")
		(new MeshGenerator()).freeMemory();
	    else
		(new MeshGenerator()).freeMemoryNetGen();
	} catch (Exception e) {
	    System.out.println("Exception caught in MeshGenerator.buildTriangleSet. Problems with freeing memory.");
	}//catch
	
	return retSet;
    }//end method buildTriangleSet

    
    /**
     * Computes a mesh for a polygon (consisting of a single cycle with holes) given by <code>borderCycles</code>.
     * Inside of this method all parametric values for the C function are computed and collected.
     * Afterwards the C function <code>triangulate</code> is calles (indirectly via the MeshGenerator.dll).
     * <p>Note, that the border must consist of <i>one</i> face with additional cycles that represent
     * holes.
     *
     * @param borderCycles the border of a polygon to be 'meshed'
     * @param qualityMesh <code>true</code> if a quality mesh shall be generated, if <code>false</code>
     *                    a simple triangulation is computed
     * @return the mesh, i.e. the set of triangles as <code>TriMultiSet</code>
     * @see #computeMeshForSingleCycle(SegMultiSet,boolean)
     */
    public TriMultiSet computeMeshForSingleCycleHoles (CycleList borderCycles, boolean qualityMesh) {
	//a certain set of variables is needed for the C-method
	//compute these variables

	if (qualityMesh) this.arguments = "pqQ";
	else this.arguments = "pQ";

	//pointlist: an array of the (border) points of the form 
	//[x-coord p1][y-coord p1]...[xcoord pn][ycoord pn]
	
	/*
	 * new implementation using class PointLink and hashing
	 */
	//count the number of segments in borderCycles
	Iterator it = borderCycles.iterator();
	Iterator it2;
	LinkedList actList;
	int segCount = 0;
	while (it.hasNext()) {
	    actList = (LinkedList)it.next();
	    it2 = actList.iterator();
	    while (it2.hasNext()) {
		it2.next();
		segCount++;
	    }//while it2
	}//while it

	//construct a hash table
	int initialCapacity = NUMBER_OF_BUCKETS;
	Hashtable pointsHT = new Hashtable(initialCapacity);
	
	//fill in the points as pointers to original points
	//segmentlist can be computed in parallel
	int[]segmentlist = new int[segCount*2];
	Iterator sit = borderCycles.iterator();
	Iterator cit;
	PointLink htEntry;
	Segment actSeg;
	LinkedList actCyc;
	int counter = 0;
	int segmentCounter = 0;
	
	while (sit.hasNext()) {
	    actCyc = (LinkedList)sit.next();
	    cit = actCyc.iterator();
	    while (cit.hasNext()) {
		actSeg = (Segment)cit.next();
		htEntry = new PointLink(actSeg.getStartpoint());
		//check whether htEntry already exists in hash table
		//if not, give it a number and insert it
		//put the proper counter number as index in segmentlist
		if (!pointsHT.containsKey(htEntry.linkedPoint)) {
		    htEntry.number = counter;
		    //note: the 'names' aka numbers of points are numbered beginning with 1
		    //therefore, we need a "counter+1" here
		    segmentlist[segmentCounter] = counter+1;
		    segmentCounter++;
		    counter++;
		    pointsHT.put(htEntry.linkedPoint,htEntry);
		}//if
		else {
		    //get the number of the point from hash table
		    htEntry = (PointLink)pointsHT.get(actSeg.getStartpoint());
		    segmentlist[segmentCounter] = htEntry.number+1;
		    segmentCounter++;
		}//else
		
		//now do the same as above for the other point of the segment
		htEntry = new PointLink(actSeg.getEndpoint());
		if (!pointsHT.containsKey(htEntry.linkedPoint)) {
		    htEntry.number = counter;
		    segmentlist[segmentCounter] = counter+1;
		    segmentCounter++;
		    counter++;
		    pointsHT.put(htEntry.linkedPoint,htEntry);
		}//if
		else {
		    htEntry = (PointLink)pointsHT.get(actSeg.getEndpoint());
		    segmentlist[segmentCounter] = htEntry.number+1;
		    segmentCounter++;
		}//else
	    }//while cit
	}//while sit
	
	//construct pointlist from elements of the hashtable
	Enumeration enumr = pointsHT.elements();
	double[] pointlist = new double[counter*2];
	Object elem;
	while (enumr.hasMoreElements()) {
	    elem = enumr.nextElement();
	    pointlist[((PointLink)elem).number*2] = ((PointLink)elem).linkedPoint.x.getDouble();
	    pointlist[((PointLink)elem).number*2+1] = ((PointLink)elem).linkedPoint.y.getDouble();
	}//while enumr


	pointattributelist = new double[pointlist.length/2];
	pointmarkerlist = new int[pointlist.length/2];
	numberofpoints = pointlist.length/2;
	numberofpointattributes = 0;

	trianglelist = null;
	triangleattributelist = null;
	trianglearealist = null;
	neighborlist = null;
	numberoftriangles = 0;
	numberofcorners = 0;
	numberoftriangleattributes = 0;
	

	segmentmarkerlist = null;
	numberofsegments = segmentlist.length/2;
	
	numberofholes = borderCycles.size()-1;
	if ((numberofholes) == 0) holelist = null;
	else {
	    //compute a point inside of each hole
	    Point[] pointsForHoles = new Point[numberofholes];
	    for (int holeCount = 0; holeCount < numberofholes; holeCount++) {
		pointsForHoles[holeCount] = computePointForHole((LinkedList)borderCycles.get(holeCount+1));
	    }//for holeCount
	    
	    //write double values for hole points to holelist
	    int arrIdx = 0;
	    holelist = new double[numberofholes*2];
	    Point actPo;
	    for (int holeCount = 0; holeCount < pointsForHoles.length; holeCount++) {
		actPo = pointsForHoles[holeCount];
		holelist[arrIdx] = actPo.x.getDouble();
		holelist[arrIdx+1] = actPo.y.getDouble();
		arrIdx = arrIdx+2;
	    }//for holeCount
	}//else

	numberofregions = 0;
	regionlist = null;

	//System.out.println("numberofholes: "+numberofholes);

	//call C-code
	double[] triResultList = new MeshGenerator().triangulate(arguments,
								 pointlist,
								 pointattributelist,
								 pointmarkerlist,
								 numberofpoints,
								 numberofpointattributes,
								 trianglelist,
								 triangleattributelist,
								 trianglearealist,
								 neighborlist,
								 numberoftriangles,
								 numberofcorners,
								 numberoftriangleattributes,
								 segmentlist,
								 segmentmarkerlist,
								 numberofsegments,
								 numberofholes,
								 holelist,
								 numberofregions,
								 regionlist);


	return buildTriangleSet(triResultList);
    }//end method computeMeshForSingleCycleHoles


    /**
     * Computes a point that lies inside of the cycle specified by passed <code>SegMultiSet</code>.
     * The point computed is the center of the first triangle returned by a triangulation of the
     * border.
     *
     * @param border the border of the cycle as <code>SegMultiSet</code>
     * @return a point lying inside of <code>border</code>
     */
    private static Point computePointForHole (LinkedList border) {
	MeshGenerator myMG = new MeshGenerator();
	TriMultiSet tms = myMG.computeMeshForSingleCycle(SegMultiSet.convert(SupportOps.convert(border)),false);
	Point[] firstVertices = ((Triangle)tms.first()).vertices();
	Point center = Mathset.center(firstVertices[0],firstVertices[1],firstVertices[2]);
	return center;
    }//end method computePointForHole

}//end class MeshGenerator
