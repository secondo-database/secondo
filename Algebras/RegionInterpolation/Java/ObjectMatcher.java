package movingregion;

import java.awt.*;
import java.util.*;

/**
  * This class contains the code to discover which cycles in two sets
  * of cycles that overlap, and run the moving cycle creation algorithm
  * found in the TriangleRep class.
  *
  * @author Erlend Tøssebro
  */
public class ObjectMatcher {
  Vector trianglereps;  // A list of graph representations of moving cycles
  Vector regions0;      // A list of the cycles in the first snapshot
  Vector regions1;      // a list of the cycles in the second snapshot
  Vector createdfrom;   // A list indicating which cycles each moving cycle
                        // was created from.

  double min_overlap;
  double min_overlap_match;
  double min_overlap_conc;

  /**
    * This class represents an overlap graph edge. The overlap graph used
    * by the object matcher is different from that used by the moving
    * cycle creator.
    *
    * @author Erlend Tøssebro
    */
  private class OverlapEdge {
    public int overlapswith;
    public double overlap;

    /**
      * This constructor creates an overlap graph edge using the
      * given parameters.
      *
      * @param ov Which object the edge points to. The object is represented
      *           as an integer index into the list of objects. The object
      *           that the edge is coming from is the object which contains
      *           this overlap edge object.
      * @param overl How much the object indicated by <code>ov</code> overlaps
      *              the other object.
      */
    public OverlapEdge(int ov, double overl) {
      overlapswith = ov;
      overlap = overl;
    }
  }

  /**
    * Comparator which creates an ordering of LineWA objects based on their
    * x-coordinates.
    */
  private class xSorter implements Comparator {

    /**
      * This function compares two <code>LineWA</code> objects based on
      * their x-coordinates.
      *
      * @param o1 The first object
      * @param o2 The second object
      *
      * @return 1 if <code>o1</code> greater than <code>o2</code>, 0 if they
      *         are equal and -1 if <code>o2</code> greater than
      *         <code>o1</code>.
      */
    public int compare(Object o1, Object o2) {
      LineWA l1, l2;
      l1 = (LineWA)o1;
      l2 = (LineWA)o2;
      if (l1.x > l2.x) return(1);
      if (l1.x == l2.x) return(0);
      return(-1);
    }
  }

  /**
    * Class for storing a single cycle with overlap edges
    */
  private class Cycle {
    public LineWA[] shape;           // The shape of the cycle
    public Rectangle boundingBox;    // The smallest bounding box for the cycle
    public Vector overlaps;          // List of overlap graph edges
    public int time;                 // Which snapshot the cycle belongs to
    public boolean ismatched;        // Has this cycle has been matched?

    /**
      * Creates a new <code>Cycle</code> object.
      *
      * @param outline The shape of the cycle given as a <code>LineWA</code>
      *                array.
      * @param time Which snapshot this cycle belongs to. May be either
      *             0 or 1.
      */
    public Cycle(LineWA[] outline, int time) {
      int x1,y1,x2,y2;
      shape = outline;
      x1 = Integer.MAX_VALUE;
      y1 = Integer.MAX_VALUE;
      x2 = 0;
      y2 = 0;
      for (int a=0;a<shape.length;a++) {
	if (shape[a].x < x1) x1 = shape[a].x;
	if (shape[a].y < y1) y1 = shape[a].y;
	if (shape[a].x > x2) x2 = shape[a].x;
	if (shape[a].y > y2) y2 = shape[a].y;
      }
      boundingBox = new Rectangle(x1,y1,x2-x1,y2-y1);
      overlaps = new Vector();
      this.time = time;
      ismatched = false;
    }

    /**
      * Adds an overlap graph edge to this node
      *
      * @param ov Which cycle the overlap graph edge should point to.
      * @param overl How much this cycle overlaps the other cycle.
      */
    public void addOverlapEdge(int ov, double overl) {
      overlaps.add(new OverlapEdge(ov,overl));
    }

    /**
      * Retrieves the overlap graph edges going from this cycle.
      *
      * @return An array of overlap graph edges containing all the edges
      *         that go from this node.
      */
    public OverlapEdge[] getOverlapEdges() {
      OverlapEdge[] retval;
      retval = new OverlapEdge[overlaps.size()];
      overlaps.toArray(retval);
      return(retval);
    }

  }

  /** 
    * Constructor without parameters. Initializes everything to default
    * values
    */
  public ObjectMatcher() {
    trianglereps = new Vector();
    regions0 = new Vector();
    regions1 = new Vector();
    createdfrom = new Vector();
    min_overlap = 2.0;
    min_overlap_match = 80.0;
    min_overlap_conc = 40.0;
  }

  /**
    * Constructor with only the matching criterion parameters.
    *
    * @param minoverl The minimum overlap between two cycles that is
    *                 required for an overlap graph edge to be inserted
    *                 between the two.
    * @param minoverlmatch The minimum overlap between two concavities
    *                      for the interpolator to match them.
    * @param minoverlconc The minimum overlap between two concavities for
    *                     an overlap graph edge to be inserted between them
    *                     by the concavity matcher.
    */
  public ObjectMatcher(double minoverl, double minoverlmatch,
		       double minoverlconc) {
    trianglereps = new Vector();
    regions0 = new Vector();
    regions1 = new Vector();
    createdfrom = new Vector();
    min_overlap = minoverl;
    min_overlap_match = minoverlmatch;
    min_overlap_conc = minoverlconc;
  }

  /** 
    * Constructor with all parameters. Initializes the class variables to
    * the input values and runs the matching algorithm.
    *
    * @param mregions0 The first snapshot represented as a double array
    *                  of <code>LineWA</code> objects. The outer array
    *                  represents the cycles and the inner array the
    *                  individual lines in each cycle.
    * @param mregions1 The second snapshot represented as a double array
    *                  of <code>LineWA</code> objects.
    * @param minoverl The minimum overlap between two cycles that is
    *                 required for an overlap graph edge to be inserted
    *                 between the two.
    * @param minoverlmatch The minimum overlap between two concavities
    *                      for the interpolator to match them.
    * @param minoverlconc The minimum overlap between two concavities for
    *                     an overlap graph edge to be inserted between them
    *                     by the concavity matcher.
    */
  public ObjectMatcher(LineWA[][] mregions0, LineWA[][] mregions1,
		       double minoverl, double minoverlmatch,
		       double minoverlconc) {
    regions0 = new Vector();
    trianglereps = new Vector();
    for (int a=0;a<mregions0.length;a++) {
      regions0.add(new Cycle(mregions0[a], 0));
    }
    regions1 = new Vector();
    for (int a=0;a<mregions1.length;a++) {
      regions1.add(new Cycle(mregions1[a], 1));
    }
    createdfrom = new Vector();
    min_overlap = minoverl;
    min_overlap_match = minoverlmatch;
    min_overlap_conc = minoverlconc;
    matchCycles();
  }

  /** 
    * Adds a cycle to one of the snapshots. This function may also
    * move and rotate the cycle before inserting it.
    *
    * @param cycle The cycle represented as a <code>LineWA</code> array.
    * @param time Which snapshot the cycle should be added to. This may be
    *             either 0 or 1.
    * @param dx How much the cycle should be moved in the x direction.
    * @param dy How much the cycle should be moved in the y direction.
    * @param rot How many radians the cycle should be rotated.
    */
  public void addCycle(LineWA[] cycle, int time, int dx, int dy, double rot) {
    Cycle newcyc;
    newcyc = new Cycle(cycle, time);
    rotateCycle(newcyc, rot);
    translateCycle(newcyc, dx, dy);
    if (time == 0) {
      regions0.add(newcyc);
    } else {
      regions1.add(newcyc);
    }
  }

  /**
    * Computes the overlap graphs between the two sets of cycles already
    * stored in this object.
    */
  public void computeOverlapGraph() {
    int a,b;
    Cycle ra,rb;
    double[] overlaps;
    for (a=0;a<regions0.size();a++) {
      ra = (Cycle)regions0.elementAt(a);
      for (b=0;b<regions1.size();b++) {
	rb = (Cycle)regions1.elementAt(b);
	if (ra.boundingBox.intersects(rb.boundingBox)) {
	  overlaps = TriRepUtil.convexHullDistance(ra.shape, rb.shape);
	  if ((overlaps[0] >= min_overlap) && (overlaps[1] >= min_overlap)) {
	    ra.addOverlapEdge(b,overlaps[0]);
	    rb.addOverlapEdge(a,overlaps[1]);
	  }
	}
      }
    }
  }

  /**
    * Computes the square of a number
    *
    * @param x the number
    *
    * @return The square of the number
    */
  private double sqr(double x) {
    return(x*x);
  }

  /**
    * This function finds a pair of integer indices giving the positions
    * of the points in two cycles that are closest together.
    *
    * @param list1 The first cycle represented as a <code>LineWA</code>
    *              array.
    * @param list2 The second cycle represented as a <code>LineWA</code>
    *              array.
    *
    * @return An array containing two integers. The first is the index of
    *         the point in <code>list1</code> that is closest to the second
    *         cycle. The second is the index of the point in <code>list2</code>
    *         that is closest to the first cycle.
    */
  private int[] findClosestPoints(LineWA[] list1, LineWA[] list2) {
    int[] closestpoints;
    LineWA[] slist1,slist2;
    double mindistance;
    double distance;
    int index1, index2, cp1, cp2;
    xSorter xs;
    closestpoints = new int[2];
    
    slist1 = new LineWA[list1.length];
    slist2 = new LineWA[list2.length];
    xs = new xSorter();
    for (int a=0;a<list1.length;a++) {
      slist1[a] = list1[a];
    }
    for (int a=0;a<list2.length;a++) {
      slist2[a] = list2[a];
    }
    Arrays.sort(slist1, xs);
    Arrays.sort(slist2, xs);
    cp1 = 0;
    cp2 = 0;
    mindistance = Math.sqrt(sqr((double)(slist1[0].x-slist2[0].x)) + sqr((double)(slist1[0].y-slist2[0].y)));
    for (int a=0;a<slist1.length;a++) {
      index1 = Math.abs(Arrays.binarySearch(slist2,new LineWA(slist1[a].x-(int)mindistance,0), xs))-1;
      index2 = Math.abs(Arrays.binarySearch(slist2,new LineWA(slist1[a].x+(int)mindistance,0), xs));
      if (index1 < 0) index1 = 0;
      if (index2 > slist2.length) index2 = slist2.length;
      for (int b=index1;b<index2;b++) {
	distance = Math.sqrt(sqr((double)(slist1[a].x-slist2[b].x)) + sqr((double)(slist1[a].y-slist2[b].y)));
	if (distance < mindistance) {
	  mindistance = distance;
	  cp1 = a;
	  cp2 = b;
	}
      }
    }
    for (int a=0;a<list1.length;a++) {
      if (list1[a] == slist1[cp1]) {
	closestpoints[0] = a;
	break;
      }
    }
    for (int b=0;b<list2.length;b++) {
      if (list2[b] == slist2[cp2]) {
	closestpoints[1] = b;
	break;
      }
    }
    return(closestpoints);
  }

  /**
    * This function produces a combined cycle from a set of cycles. It is
    * used when several cycles in one snapshot should be matched to only
    * one cycle in the other.
    *
    * @param meetcriterion A list of cycles indicated by their indexes in
    *        the list of cycles in this object.
    * @param ss Which snapshot the cycles belong to. May be either 0 or 1.
    *
    * @return A cycle representing a combination of the cycles given in
    *         <code>meetcriterion</code>. This combination is created by
    *         inserting lines between the cycles where they are closest.
    */
  private LineWA[] produceCombinedOutLine(Vector meetcriterion, int ss) {
    Vector regions;
    Vector matchedto;
    Vector cpoints;
    Vector combinedol;
    LineWA[] reg1;
    LineWA[] reg2;
    LineWA[] retvalue;
    Cycle cycle;
    int numcycles,x1,x2,y1,y2,creg;
    int[] closestpoints, cp;
    double distance;
    double mindist;
    Integer tmpval;
    cpoints = new Vector();
    if (ss == 1) {
      regions = regions0;
    } else regions = regions1;
    matchedto = new Vector();
    numcycles = meetcriterion.size();
    mindist = Double.MAX_VALUE;
    creg = -1;
    cp = new int[2];
    cp[0] = 0;
    cp[1] = 0;
    for (int a=0;a<numcycles;a++) {
      tmpval = (Integer)meetcriterion.elementAt(a);
      cycle = (Cycle)regions.elementAt(tmpval.intValue());
      reg1 = cycle.shape;
      for (int b=0;b<numcycles;b++) {
	if (a==b) continue;
	tmpval = (Integer)meetcriterion.elementAt(b);
	cycle = (Cycle)regions.elementAt(tmpval.intValue());
	reg2 = cycle.shape;
	closestpoints = findClosestPoints(reg1,reg2);
	x1 = reg1[closestpoints[0]].x;
	y1 = reg1[closestpoints[0]].y;
	x2 = reg2[closestpoints[1]].x;
	y2 = reg2[closestpoints[1]].y;
	distance = Math.sqrt(sqr((double)(x1-x2)) + sqr((double)(y1-y2)));
	if (distance < mindist) {
	  creg = b;
	  cp = closestpoints;
	  mindist = distance;
	}
      }
      matchedto.add(new Integer(creg));
      cpoints.add(cp);
      mindist = Double.MAX_VALUE;
    }
    combinedol = new Vector();
    outlineProducer(combinedol, meetcriterion, regions, matchedto, cpoints, 0, 0,-1);
    combinedol.removeElementAt(combinedol.size()-1);
    retvalue = new LineWA[combinedol.size()];
    combinedol.toArray(retvalue);
    TriRepUtil.printLineList(retvalue);
    return(retvalue);
  }

  /** Produces the outline of the conbined cycle based on the data generated
    * by produceCombinedOutline.
    *
    * @param preoutline empty vector which is modified by this function to
    *                   contain the combined outline.
    * @param meetcriterion A list of cycles indicated by their indexes in
    *                      the <code>regions</code> vector.
    * @param regions A pointer to the set of cycles to which the cycles in
    *                meetcriterion belong
    * @param matchedto Vector of integer indices into the <code>regions</code>
    *                  vector which tells which region is closest to the 
    *                  region of the same index in the 
    *                  <code>meetcriterion</code> list.
    * @param cpoints Vector of pairs of integers giving the indexes into the
    *                point lists of two matching cycles of the closest points.
    * @param cregion The index of the current region in the 
    *                <code>meetcriterion</code> list.
    * @param start Which point in the current region to start with
    * @param from The index of the last region in the
    *             <code>meetcriterion</code> list.
    */
  private void outlineProducer(Vector preoutline, Vector meetcriterion,
			       Vector regions, Vector matchedto,
			       Vector cpoints, int cregion,int start, 
			       int from) {
    Cycle thisregion;  // The shape of this cycle represented as a cycle var.
    LineWA[] regshape; // The shape of this cycle represented by a LineWA array
    int[] cp;
    int[] correlem;
    int[] oce;
    TreeMap corr;      // Tree containing pairs of (cycleindex, pointindex)
                       // indexed by pointindex in this object
    Integer thr;
    int stop,length, pos, thisindex;
    corr = new TreeMap();
    thr = (Integer)meetcriterion.elementAt(cregion);
    thisindex = thr.intValue();
    thisregion = (Cycle)regions.elementAt(thisindex);
    regshape = thisregion.shape;
    cp = (int[])cpoints.elementAt(cregion);
    stop = start + regshape.length + 1;
    length = regshape.length;
    correlem = new int[2];
    thr = (Integer)matchedto.elementAt(cregion);
    correlem[0] = thr.intValue();
    correlem[1] = cp[1];
    corr.put(new Integer(cp[0]), correlem);
    // Discovers all regions which are closer to this region than to all others.
    // puts these correspondences into a tree indexed by the closest point
    // in this cycle
    for (int a=0;a<matchedto.size();a++) {
      if (a == cregion) continue;
      thr = (Integer)matchedto.elementAt(a);
      if (thr.intValue() == cregion) {
	cp = (int[])cpoints.elementAt(a);
	if (corr.containsKey(new Integer(cp[1]))) {
	  oce = (int[])corr.get(new Integer(cp[1]));
	  if (oce[0] == a) continue;
	  correlem = new int[oce.length+2];
	  for (int i=0;i<oce.length;i++) {
	    correlem[i] = oce[i];
	  }
	  correlem[oce.length] = a;
	  correlem[oce.length+1] = cp[0];
	} else {
	  correlem = new int[2];
	  correlem[0] = a;
	  correlem[1] = cp[0];
	}
	corr.put(new Integer(cp[1]), correlem);
      }
    }
    // Adds line segments to preoutline and calls this function again if it
    // should go to another object.
    for (int a=start;a<stop;a++) {
      pos = a % length;
      preoutline.add(new LineWA(regshape[pos]));
      if (corr.containsKey(new Integer(pos))) {
	correlem = (int[])corr.remove(new Integer(pos));
	for (int i=0;i<correlem.length;i+=2) {
	  if (correlem[i] != from) {
	    outlineProducer(preoutline, meetcriterion, regions, matchedto, cpoints, correlem[i], correlem[i+1], thisindex);
	    preoutline.add(new LineWA(regshape[pos]));
	  }
	}
      }
    }
  }

  /**
    * Matches all cycles in one snapshot which have matches in the other
    * snapshot and adds the graph representations to the
    * <code>trianglereps</code> list.
    *
    * @param whichregion Which snapshot should be matched. May be either
    *                    0 or 1.
    */
  public void findMatches(int whichregion) {
    Cycle cregion, matchregion;
    OverlapEdge edge,edge2;
    OverlapEdge[] edges2;
    double overlap1, overlap2;
    Vector meetcriterion,reg0,reg1;
    int sumofoverlaps, dx, dy, minx,miny,maxx,maxy;
    int[][] matches;
    LineWA[] sumofoutlines;
    Integer tmpval;
    TriangleRep trirep;
    if (whichregion == 0) {
      reg0 = regions0;
      reg1 = regions1;
    } else {
      reg0 = regions1;
      reg1 = regions0;
    }
    for (int a=0;a<reg0.size();a++) {
      cregion = (Cycle)reg0.elementAt(a);
      if (cregion.ismatched) continue;
      if (cregion.overlaps.size() == 1) {
	edge = (OverlapEdge)cregion.overlaps.elementAt(0);
	overlap1 = edge.overlap;
	matchregion = (Cycle)reg1.elementAt(edge.overlapswith);
	if ((matchregion.overlaps.size() == 1) && !(matchregion.ismatched)) {
	  edge2 = (OverlapEdge)matchregion.overlaps.elementAt(0);
	  overlap2 = edge2.overlap;
	  if ((overlap1 >= min_overlap_match) && 
	      (overlap2 >= min_overlap_match)) {
	    cregion.ismatched = true;
	    matchregion.ismatched  = true;
	    dx = (int)(matchregion.boundingBox.getCenterX() - cregion.boundingBox.getCenterX());
	    dy = (int)(matchregion.boundingBox.getCenterY() - cregion.boundingBox.getCenterY());
	    for (int n=0;n<matchregion.shape.length;n++) {
	      matchregion.shape[n].x-=dx;
	      matchregion.shape[n].y-=dy;
	    }
	    matches = new int[2][1];
	    matches[whichregion][0] = a;
	    matches[(whichregion+1)%2][0] = edge.overlapswith;
	    createdfrom.add(matches);
	    if (whichregion == 0) {
	      trirep = new TriangleRep(cregion.shape, 
				       matchregion.shape, min_overlap_conc);
	      trirep.translateSnapshot(dx,dy,1);
	      trianglereps.add(trirep);
	    } else {
	      trirep = new TriangleRep(matchregion.shape, 
				       cregion.shape, min_overlap_conc);
	      trirep.translateSnapshot(dx,dy,0);
	      trianglereps.add(trirep);
	    }
	  }
	}
      }
      if (cregion.overlaps.size() > 1) {
	meetcriterion = new Vector();
	sumofoverlaps = 0;
	for (int b=0;b<cregion.overlaps.size();b++) {
	  edge = (OverlapEdge)cregion.overlaps.elementAt(b);
	  overlap1 = edge.overlap;
	  matchregion = (Cycle)reg1.elementAt(edge.overlapswith);
	  if (!(matchregion.ismatched)) {
	    edges2 = matchregion.getOverlapEdges();
	    for (int c=0;c<edges2.length;c++) {
	      if ((edges2[c].overlapswith == a) && 
		  (edges2[c].overlap >=min_overlap_match)) {
		meetcriterion.add(new Integer(edge.overlapswith));
		sumofoverlaps += edge.overlap;
		break;
	      }
	    }
	  }
	}
	if (sumofoverlaps >= min_overlap_match) {
	  sumofoutlines = produceCombinedOutLine(meetcriterion, whichregion);
	  matches = new int[2][];
	  matches[whichregion] = new int[1];
	  matches[(whichregion+1)%2] = new int[meetcriterion.size()];
	  matches[whichregion][0] = a;
	  for (int n=0;n<meetcriterion.size();n++) {
	    tmpval = (Integer)meetcriterion.elementAt(n);
	    matches[(whichregion+1)%2][n] = tmpval.intValue();
	  }
	  createdfrom.add(matches);
	  minx = Integer.MAX_VALUE;
	  miny = Integer.MAX_VALUE;
	  maxx = 0;
	  maxy = 0;
	  for (int n=0;n<sumofoutlines.length;n++) {
	    if (sumofoutlines[n].x < minx) minx = sumofoutlines[n].x;
	    if (sumofoutlines[n].y < miny) miny = sumofoutlines[n].y;
	    if (sumofoutlines[n].x > maxx) maxx = sumofoutlines[n].x;
	    if (sumofoutlines[n].y > maxy) maxy = sumofoutlines[n].y;
	  }
	  dx = (int)(((maxx+minx)/2) - cregion.boundingBox.getCenterX());
	  dy = (int)(((maxy+miny)/2) - cregion.boundingBox.getCenterY());
	  for (int n=0;n<sumofoutlines.length;n++) {
	    sumofoutlines[n].x-=dx;
	    sumofoutlines[n].y-=dy;
	  }
	  if (whichregion == 0) {
	    trirep = new TriangleRep(cregion.shape, sumofoutlines,
				     min_overlap_conc);
	    trirep.translateSnapshot(dx,dy,1);
	    trianglereps.add(trirep);
	  } else {	    
	    trirep = new TriangleRep(sumofoutlines, cregion.shape,
				     min_overlap_conc);
	    trirep.translateSnapshot(dx,dy,0);
	    trianglereps.add(trirep);
	  }
	  cregion.ismatched = true;
	  for (int n=0;n<meetcriterion.size();n++) {
	    tmpval = (Integer)meetcriterion.elementAt(n);
	    matchregion = (Cycle)reg1.elementAt(tmpval.intValue());
	    matchregion.ismatched = true;
	  }
	}
      }
    }
  }

  /**
    * This function rotates the given cycle around its center. The cycle
    * object given in the input is changed to reflect the rotation.
    *
    * @param cycle The cycle to be rotated.
    * @param angle The angle in radians that the cycle should be rotated.
    */
  private void rotateCycle(Cycle cycle, double angle) {
    int dx, dy;
    double r, theta;
    int newx, newy;
    LineWA[] ol;
    dx = (int)cycle.boundingBox.getCenterX();
    dy = (int)cycle.boundingBox.getCenterY();
    translateCycle(cycle, -dx, -dy);
    ol = cycle.shape;
    for (int a=0;a<ol.length;a++) {
      r = Math.sqrt(ol[a].x*ol[a].x + ol[a].y*ol[a].y);
      theta = Math.acos(ol[a].x/r);
      theta += angle;
      newx = (int)(Math.cos(theta)*r);
      newy = (int)(Math.sin(theta)*r);
      ol[a].x = newx;
      ol[a].y = newy;
    }
    translateCycle(cycle, dx, dy);
  }

  /**
    * Moves the cycle by dx and dy.The cycle
    * object given in the input is changed to reflect the movement.
    *
    * @param cycle The cycle to be moved.
    * @param dx How much the cycle should be moved in the x direction.
    * @param dy How much the cycle should be moved in the y direction.
    */
  private void translateCycle(Cycle cycle, int dx, int dy) {
    LineWA[] outline;
    outline = cycle.shape;
    for (int n=0;n<outline.length;n++) {
      outline[n].x+=dx;
      outline[n].y+=dy;
    }
  }

  /** 
    * This function matches all unmatches cycles in the given snapshot to
    * points. Each snapshot is matched to a point lying at its geometric
    * center.
    *
    * @param whichregion The snapshot in which the unmatched cycles are.
    */
  private void matchToPoint(int whichregion) {
    Vector reg0;
    Cycle cregion;
    LineWA[] degenerateCycle;
    double x,y;
    if (whichregion == 0) {
      reg0 = regions0;
    } else {
      reg0 = regions1;
    }
    for (int a=0;a<reg0.size();a++) {
      cregion = (Cycle)reg0.elementAt(a);
      if (cregion.ismatched) continue;
      x = cregion.boundingBox.getCenterX();
      y = cregion.boundingBox.getCenterY();
      degenerateCycle = new LineWA[1];
      degenerateCycle[0] = new LineWA((int)x, (int)y);
      if (whichregion == 0) {
	trianglereps.add(new TriangleRep(cregion.shape, degenerateCycle,
					 min_overlap_conc));
      } else {
	trianglereps.add(new TriangleRep(degenerateCycle, cregion.shape,
					 min_overlap_conc));
      }
      cregion.ismatched = true;
    }
  }

  /**
    * Matches the cycles of the two consecutive snapshots stored in this
    * object.
    */
  public void matchCycles() {
    computeOverlapGraph();
    // Create triangle representations for the objects that match.
    findMatches(0);
    findMatches(1);
    // Create triangle representations for objects which don't match
    matchToPoint(0);
    matchToPoint(1);
  }

  /**
    * Gets the result of the interpolation. This function must be called
    * after the interpolation algorithm is run to get any meaningful
    * result. (The interpolation algorithm is run by calling the
    * <code>matchCycles</code> function or using the constructor with
    * all parameters.)
    *
    * @return The graph representations of all the moving cycles created by
    *         interpolating the snapshots stored in this object. The graph
    *         representations are stored in a <code>TriangleRep</code>
    *         array.
    */
  public TriangleRep[] getTriangleReps() {
    TriangleRep[] trireps;
    trireps = new TriangleRep[trianglereps.size()];
    trianglereps.toArray(trireps);
    return(trireps);
  }

  /**
    * This function returns which cycles were used to create each moving
    * cycle.
    *
    * @return A triple array of integers, which has the following structure:
    *         return_value[moving cycle][snapshot][index] = snapindex.
    *         <i>Moving cycle</i> is the index in the array returned by
    *         <code>getTriangleReps</code> of the moving cycle for which you
    *         want to know the snapshots. <i>Snapshot</i> is the snapshot, and
    *         <i>index</i> is the number in this list of the snapshot.
    *         <i>snapindex</i> is the index of the cycle in the list of
    *         cycles that makes up the snapshot.
    */
  public int[][][] getMatchEdges() {
    int[][][] retval;
    Cycle reg;
    retval = new int[createdfrom.size()][][];
    createdfrom.toArray(retval);
    return(retval);
  }
}
