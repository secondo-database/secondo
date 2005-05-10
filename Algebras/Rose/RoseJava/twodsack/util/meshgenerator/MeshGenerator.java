/*
 * MeshGenerator.java 2004-11-09
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.util.meshgenerator;

import twodsack.operation.setoperation.*;
import twodsack.set.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.*;
import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;

import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.LinkedList;

/**
 * This class provides the communication with an external meshing algorithm. The meshing algorithm is 
 * written in C. This C functions are used via the JNI (Java Native Interface).
 */
public class MeshGenerator {

    /**
     * Loads the MeshGenerator library.
     * This is an *.dll file generated from the original C file.
     */
    static {
	System.loadLibrary("MeshGenerator");
    }

    static final int NUMBER_OF_BUCKETS = 499; //choose a prime number
    protected static int CALLS_OF_MESH1 = 0;
    protected static int CALLS_OF_MESH2 = 0;

    //declare native method
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
     * The native method that calls the C function.
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
     * @see computeMeshForSingleCycleHoles
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
	Enumeration enum = pointsHT.elements();
	double[] pointlist = new double[counter*2];
	Object elem;
	while (enum.hasMoreElements()) {
	    elem = enum.nextElement();
	    pointlist[((PointLink)elem).number*2] = ((PointLink)elem).linkedPoint.x.getDouble();
	    pointlist[((PointLink)elem).number*2+1] = ((PointLink)elem).linkedPoint.y.getDouble();
	}//while enum

	
	/* OLD CODE with O(n²) time complexity
	//compute set of (single) points of border
	PointMultiSet points = border.getAllPoints();
	//construct an array with the real points in it; this is needed for
	//construction of segmentarray
	Point[] pointsArr = new Point[points.size()*2];
	Iterator it = points.iterator();
	Point actPoint;
	//Segment actSeg;
	pointlist = new double[points.size()*2];
	int arrayIndex = 0;
	while (it.hasNext()) {
	    actPoint = (Point)((MultiSetEntry)it.next()).value;
	    pointlist[arrayIndex] = actPoint.x.getDouble();
	    arrayIndex++;
	    pointlist[arrayIndex] = actPoint.y.getDouble();
	    arrayIndex++;
	}//while

	it = points.iterator();
	arrayIndex = 0;
	while (it.hasNext()) {
	    pointsArr[arrayIndex] = (Point)((MultiSetEntry)it.next()).value;
	    arrayIndex++;
	}//while
	*/

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
	

	/* old code: new is shown above using hashtables
	//compute segmentlist:
	//segmentlist is an array of Integer values where each value is 
	//the index of the proper point in pointlist
	//CAUTION: The _name_ of the first point (at array index 0) must be 1 !
	it = border.iterator();
	Segment actSeg;
	System.out.println("MeshGenerator.computeMesh(): construction of segmentlist is very time-consuming. Can be improved!");
	int count = 0;
	int pos = 0;
	segmentlist = new int[border.size()*2];
	while (it.hasNext()) {
	    actSeg = (Segment)((MultiSetEntry)it.next()).value;
	    //find actSeg.getStartpoint() in pointsArr
	    for (int idx = 0; idx < pointsArr.length; idx++) {
		if (pointsArr[idx].equal(actSeg.getStartpoint())) {
		    pos = idx;
		    break;
		}//if
	    }//for
	    //set idx
	    segmentlist[count] = pos+1;
	    count++;
	    //find actSeg.endpoint in pointsArr
	    for (int idx = 0; idx < pointsArr.length; idx++) {
		if (pointsArr[idx].equal(actSeg.getEndpoint())) {
		    pos = idx;
		    break;
		}//if
	    }//for
	    segmentlist[count] = pos+1;
	    count++;
	}//while
	*/	

	segmentmarkerlist = null;
	numberofsegments = segmentlist.length/2;
	
	//System.out.println("MeshGenerator.computeMesh(): Currently the triangulation of polygons with holes has experimental status.");
	numberofholes = 0;
	holelist = null;

	numberofregions = 0;
	regionlist = null;

	//call C-code
	CALLS_OF_MESH1++;
	//System.out.println("Meshgenerator.java: Calling C++ function MeshGenerator.triangulate() for SingleCycle.");
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

	//System.out.println("received pointlist from native code (length: "+triResultList.length+")");
	//System.out.println("print received list: ");
	//for (int i = 0; i < triResultList.length; i++) {
	//    System.out.println("["+i+"]: "+triResultList[i]); }
	//System.out.println("forced EXIT.");
	
	
	//System.out.println("Meshgenerator.java: Returned from calling C++ function MeshGenerator.triangulate().");

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
     * @see computeMeshForSingleCycle
     */
    public TriMultiSet computeMeshForSingleCycleHoles (CycleList borderCycles, boolean qualityMesh) {
	//System.out.println("entering computeMeshForSingleCycleHoles...");
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
	Enumeration enum = pointsHT.elements();
	double[] pointlist = new double[counter*2];
	Object elem;
	while (enum.hasMoreElements()) {
	    elem = enum.nextElement();
	    pointlist[((PointLink)elem).number*2] = ((PointLink)elem).linkedPoint.x.getDouble();
	    pointlist[((PointLink)elem).number*2+1] = ((PointLink)elem).linkedPoint.y.getDouble();
	}//while enum

	/* OLD IMPLEMENTATION with O(n²) time complexity*/
	/*
	//join all segments of borderCycle to one SegMultiSet
	SegMultiSet border = new SegMultiSet(new SegmentComparator());
	Iterator it = borderCycles.iterator();
	Iterator it2;
	LinkedList actList;
	while (it.hasNext()) {
	    actList = (LinkedList)it.next();
	    it2 = actList.iterator();
	    while (it2.hasNext()) {
		border.add(it2.next()); }
	}//while it

	//compute set of (single) points of border
	PointMultiSet points = border.getAllPoints();
	//construct an array with the real points in it; this is needed for
	//construction of segmentarray
	Point[] pointsArr = new Point[points.size()*2];
	it = points.iterator();
	Point actPoint;
	//Segment actSeg;
	pointlist = new double[points.size()*2];
	int arrayIndex = 0;
	while (it.hasNext()) {
	    actPoint = (Point)((MultiSetEntry)it.next()).value;
	    pointlist[arrayIndex] = actPoint.x.getDouble();
	    arrayIndex++;
	    pointlist[arrayIndex] = actPoint.y.getDouble();
	    arrayIndex++;
	}//while

	it = points.iterator();
	arrayIndex = 0;
	while (it.hasNext()) {
	    pointsArr[arrayIndex] = (Point)((MultiSetEntry)it.next()).value;
	    arrayIndex++;
	}//while
	*/

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
	

	/* OLD IMPLEMENTATION */
	/*
	//compute segmentlist:
	//segmentlist is an array of Integer values where each value is 
	//the index of the proper point in pointlist
	//CAUTION: The _name_ of the first point (at array index 0) must be 1 !
	it = border.iterator();
	Segment actSeg;
	System.out.println("MeshGenerator.computeMesh(): construction of segmentlist is very time-consuming. Can be improved!");
	int count = 0;
	int pos = 0;
	segmentlist = new int[border.size()*2];
	while (it.hasNext()) {
	    actSeg = (Segment)((MultiSetEntry)it.next()).value;
	    //find actSeg.getStartpoint() in pointsArr
	    for (int idx = 0; idx < pointsArr.length; idx++) {
		if (pointsArr[idx].equal(actSeg.getStartpoint())) {
		    pos = idx;
		    break;
		}//if
	    }//for
	    //set idx
	    segmentlist[count] = pos+1;
	    count++;
	    //find actSeg.endpoint in pointsArr
	    for (int idx = 0; idx < pointsArr.length; idx++) {
		if (pointsArr[idx].equal(actSeg.getEndpoint())) {
		    pos = idx;
		    break;
		}//if
	    }//for
	    segmentlist[count] = pos+1;
	    count++;
	}//while
	*/

	segmentmarkerlist = null;
	numberofsegments = segmentlist.length/2;
	
	//System.out.println("MeshGenerator.computeMesh(): Currently the triangulation of polygons with holes has experimental status.");
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
	/*
	System.out.println("\npointlist ("+pointlist.length+"):");
	for (int i = 0; i < pointlist.length; i++) {
	    System.out.println("No."+(i/2+1)+" ("+pointlist[i]+", "+pointlist[i+1]+")");
	    i++;
	}//for i
	System.out.println("\nsegmentlist ("+segmentlist.length+"):");
	for (int i = 0; i < segmentlist.length; i++) {
	    System.out.println("No."+(i/2)+" ("+segmentlist[i]+", "+segmentlist[i+1]+")");
	    i++;
	}//for i
	if (numberofholes > 0) {
	    System.out.println("\nholelist ("+holelist.length+"):");
	    for (int i = 0; i < holelist.length; i++) {
		System.out.println("No."+(i/2)+" ("+holelist[i]+", "+holelist[i+1]+")");
		i++;
	    }//for i
	}//if
	//System.exit(0);
	*/
	//call C-code
	CALLS_OF_MESH2++;
	
	//System.out.println("Meshgenerator.java: Calling C++ function MeshGenerator.triangulate() for SingeCycleHoles.");
	
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

	//System.out.println("received pointlist from native code (length: "+triResultList.length+")");
	//System.out.println("print received list: ");
	//for (int i = 0; i < triResultList.length; i++) {
	//    System.out.println("["+i+"]: "+triResultList[i]); }
	//System.out.println("forced EXIT.");
	

	//System.out.println("Meshgenerator.java: Returned from calling C++ function MeshGenerator.triangulate().");

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