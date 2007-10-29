//package movingregion;

import java.util.*;
import java.io.*;

/**
 * This class is used to create a time slice of a moving cycle. It takes
 * two snapshots of a moving cycle and constructs and stores a graph
 * representation of the moving cycle.
 *
 * @author Erlend Tøssebro
 */
public class TriangleRep
{
    private Vector pointList;   // List of points with neighbour lists
    public double min_overlap; //Minimum overlap to be included in overlap graph
    public double min_overlap_match;// Minimum overlap to be considered match
    public int[] endpoints;         // The first points in the two snapshots
    ConvexHullTreeNode root1, root2;
    
    
    /**
     * Constructor without parameters. Makes an empty triangle
     * representation.
     */
    public TriangleRep()
    {
        pointList = new Vector();
        min_overlap = 2.0;
        min_overlap_match = 40.0;
        endpoints = new int[4];
        endpoints[0] = 0;
        endpoints[1] = 0;
        endpoints[2] = 0;
        endpoints[3] = 0;
    }
    
    /**
     * Constructs the graph representation from two arbitrarily shaped
     * polygons represented as lists of lines.
     *
     * @param poly1 The first snapshot represented as a list of line segments
     * @param poly2 The second snapshot represented as a list of line segments
     * @param minOverlMatch How much overlap between concavities is
     *                      necessary for them to be matched to each other.
     */
    public TriangleRep(LineWA[] poly1, LineWA[] poly2, double minOverlMatch)
    {
        //ConvexHullTreeNode root1, root2;  // The roots of convex hull reps. of
        endpoints = new int[4];           //  the polygons.
        endpoints[0] = 0;
        endpoints[1] = 0;
        endpoints[2] = 0;
        endpoints[3] = 0;
        // Check for degenerate regions, and call appropriate procedure:
        if (poly2.length == 1)
        {
            matchCycleToPoint(poly1, poly2, 0, minOverlMatch);
            return;
        }
        if (poly1.length == 1)
        {
            matchCycleToPoint(poly2, poly1, 1, minOverlMatch);
            return;
        }
        root1 = new ConvexHullTreeNode(poly1, 0,null);
        root2 = new ConvexHullTreeNode(poly2, 0,null);
        TriRepUtil.computeOverlapGraph(root1, root2, minOverlMatch);
        try
        {
            createGeneralTriangleRep(root1, root2, minOverlMatch);
        }
        catch (UnsupportedOperationException e)
        {}
    }
    
    public ConvexHullTreeNode getConvexHullTree(int time)
    {
        if(time==0)
            return(root1);
        else
            return(root2);
    }
    
    /**
     * Constructs the graph representation from two arbitrarily shaped
     * polygons represented by convex hull trees.
     *
     * @param root1 The root of the convex hull tree representing the
     *              first snapshot.
     * @param root2 The root of the convex hull tree representing the
     *              second snapshot.
     * @param minOverlMatch How much overlap between concavities is
     *                      necessary for them to be matched to each other.
     */
    public TriangleRep(ConvexHullTreeNode root1, ConvexHullTreeNode root2,
            double minOverlMatch)
    {
        endpoints = new int[4];
        endpoints[0] = 0;
        endpoints[1] = 0;
        endpoints[2] = 0;
        endpoints[3] = 0;
        createGeneralTriangleRep(root1, root2, minOverlMatch);
    }
    
    /**
     * Creates the graph representation for the case where one of the
     * cycles is only a point.
     *
     * @param cycle The cycle that is not a point.
     * @param point The point represented as a list of lines with only
     *              one element.
     * @param time Indicates whether the parameter <code>cycle</code>
     *             is the first or second snapshot.
     * @param minmatch How much overlap between concavities is
     *                 necessary for them to be matched to each other.
     */
    private void matchCycleToPoint(LineWA[] cycle, LineWA[] point, int time, double minmatch)
    {
        if (time == 0)
        {
            insertSnapshots(cycle,point,minmatch);
            for (int a=0;a<pointList.size()-1;a++)
            {
                insertLine(a,pointList.size()-1);
            }
        }
        else
        {
            insertSnapshots(point,cycle,minmatch);
            for (int a=1;a<pointList.size();a++)
            {
                insertLine(a,0);
            }
        }
    }
    
    /**
     * Inserts the triangles which are left after the concavities have
     * been inserted.
     *
     * @param poly1 The first snapshot represented as a list of line segments
     * @param poly2 The second snapshot represented as a list of line segments
     */
    private void createConvexTriangleRep(LineWA[] poly1, LineWA[] poly2)
    {
        int i1, i2, maxi1, maxi2, nexti,nvi,i1disp,i2disp, noneighbours,t,nindex;
        int poly2begindex, nexti2, nexi1disp, nexti2disp,ci1,ci2;
        int[] neighbours,position,convert1, convert2;
        double angle1, angle2;
        boolean equalmatch;
        boolean hasmatched;
        i1 = 0;
        i2 = 0;
        ci1 = 0;
        ci2 = 0;
        equalmatch = false;
        maxi1 = poly1.length;
        maxi2 = poly2.length;
        TriRepUtil.computeLineAngles(poly1);
        TriRepUtil.computeLineAngles(poly2);
        poly2begindex = findPoint(poly2[0].x,poly2[0].y,1);
        if (poly2begindex == -1)
        {
            throw(new TriRepCreationException("Couldn't find first point in poly2 in triangle rep!"));
        }
        // Build conversion vectors;
        convert1 = new int[poly1.length];
        convert2 = new int[poly2.length];
        for (int a=0;a<poly1.length;a++)
        {
            convert1[a] = findPoint(poly1[a].x, poly1[a].y,0);
        }
        for (int a=0;a<poly2.length;a++)
        {
            convert2[a] = findPoint(poly2[a].x, poly2[a].y,1);
        }
        // Find starting points
        while ((i1 < maxi1) || (i2 < maxi2))
        {
            if (i1<maxi1)
            {
                angle1 = poly1[i1].angle;
            }
            else
            {
                angle1 = 2*Math.PI + poly1[i1 % maxi1].angle;
            }
            if (i2<maxi2)
            {
                angle2 = poly2[i2].angle;
            }
            else
            {
                angle2 = 2*Math.PI + poly2[i2 % maxi2].angle;
            }
            if (((angle1 < angle2) || ((angle1 == angle2) && (!equalmatch))) &&
                    (i1 < maxi1))
            {
                nexti = i1+1;
                noneighbours = getNumberOfNeighbours(convert1[i1]);
                // If it has more than 2 neighbours, it is the boundary of a conc.
                if (noneighbours > 2)
                {
                    // Go to the final node of the concavity (or chain of concavities)
                    noneighbours = getNumberOfNeighbours(convert1[nexti % maxi1]);
                    while ((noneighbours > 2) && !neighbours(convert1[i1 % maxi1], convert1[nexti % maxi1]))
                    {
                        i1 = nexti;
                        nexti++;
                        noneighbours = getNumberOfNeighbours(convert1[nexti % maxi1]);
                    }
                    neighbours = getNeighbours(convert1[i1 % maxi1]);
                    position = getPosition(convert1[i1 % maxi1]);
                    t = position[2];
                    nindex = -1;
                    i2 = -1;
                    for (int a=0;a<neighbours.length;a++)
                    {
                        nindex = neighbours[a];
                        position = getPosition(nindex);
                        if (position[2]!=t)
                        {
                            for (int b=0;b<convert2.length;b++)
                            {
                                if (convert2[b] == nindex)
                                {
                                    i2 = b;
                                    break;
                                }
                            }
                            if (i2 != -1) break;
                        }
                    }
                    if (position[2] == t)
                    {
                        throw(new TriRepCreationException("TriangleRep.java 129:3 neighbours with the same time!"));
                    }
                    // find nindex in convert2 array
                    if (i2 == -1)
                    {
                        throw(new TriRepCreationException("TriangleRep,java 133:Couldn't find point in conversion list!"));
                    }
                    angle1 = poly1[i1 % maxi1].angle;
                    angle2 = poly2[i2 % maxi2].angle;
                    if (angle2 > Math.PI+angle1) i1 += maxi1;
                    break; // while ((i1 < maxi1) || ....
                }
                else
                {
                    i1++;
                }
            }
            else
            {
                nexti = i2+1;
                noneighbours = getNumberOfNeighbours(convert2[i2]);
                // If it has more than 2 neighbours, it is the boundary of a conc.
                if (noneighbours > 2)
                {
                    // Go to the final node of the concavity (or chain of concavities)
                    noneighbours = getNumberOfNeighbours(convert2[nexti % maxi2]);
                    while ((noneighbours > 2) && !neighbours(convert2[i2 % maxi2], convert2[nexti % maxi2]))
                    {
                        i2 = nexti;
                        nexti++;
                        noneighbours = getNumberOfNeighbours(convert2[nexti % maxi2]);
                    }
                    neighbours = getNeighbours(convert2[i2 % maxi2]);
                    position = getPosition(convert2[i2 % maxi2]);
                    t = position[2];
                    nindex = -1;
                    i1 = -1;
                    for (int a=0;a<neighbours.length;a++)
                    {
                        nindex = neighbours[a];
                        position = getPosition(nindex);
                        if (position[2]!=t)
                        {
                            for (int b=0;b<convert1.length;b++)
                            {
                                if (convert1[b] == nindex)
                                {
                                    i1 = b;
                                    break;
                                }
                            }
                            if (i1 != -1) break;
                        }
                    }
                    if (position[2] == t)
                    {
                        throw(new TriRepCreationException("TriangleRep.java 170: 3 neighbours, all with the same time!"));
                    }
                    // Finde displacement (number of deleted points) between 0
                    // and nindex.
                    if (i1 == -1)
                    {
                        throw(new TriRepCreationException("TriangleRep.java 175: Couldn't find point in conversion list!"));
                    }
                    angle1 = poly1[i1 % maxi1].angle;
                    angle2 = poly2[i2 % maxi2].angle;
                    if (angle1 > Math.PI+angle2) i2 += maxi2;
                    break; // while ((i1 < maxi1) || ....
                }
                else
                {
                    i2++;
                }
            }
        }
        // By this point, i1 and i2 should be at the ending points of a concavity.
        // Match triangles which have not already been matched.
        while ((ci1 < maxi1) || (ci2 < maxi2))
        {
            hasmatched = false;
            if (i1<maxi1)
            {
                angle1 = poly1[i1].angle;
            }
            else
            {
                angle1 = 2*Math.PI*(i1 / maxi1) + poly1[i1 % maxi1].angle;
            }
            if (i2<maxi2)
            {
                angle2 = poly2[i2].angle;
            }
            else
            {
                angle2 = 2*Math.PI*(i2 / maxi2) + poly2[i2 % maxi2].angle;
            }
            if ((angle1 < angle2) || ((angle1 == angle2) && (!equalmatch)))
            {
                nexti = i1+1;
                if (neighbours(convert1[i1 % maxi1],convert1[nexti % maxi1]))
                {
                    nvi = i2;
                    insertLine(convert1[nexti % maxi1], convert2[nvi % maxi2]);
                    i1++;
                    ci1++;
                    hasmatched = true;
                }
            }
            if (hasmatched == false)
            {
                nexti = i2+1;
                if (neighbours(convert2[i2 % maxi2],convert2[nexti % maxi2]))
                {
                    nvi = i1;
                    insertLine(convert2[nexti % maxi2], convert1[nvi % maxi1]);
                    i2++;
                    ci2++;
                    hasmatched = true;
                }
            }
            if (hasmatched == false)
            {
                nexti = i1+1;
                if (neighbours(convert1[i1 % maxi1],convert1[nexti % maxi1]))
                {
                    nvi = i2;
                    insertLine(convert1[nexti % maxi1], convert2[nvi % maxi2]);
                    i1++;
                    ci1++;
                    hasmatched = true;
                }
            }
            if (hasmatched == false)
            {
                i1++;
                i2++;
                ci1++;
                ci2++;
            }
            if (angle1 == angle2)
            {
                equalmatch = !equalmatch;
            }
            else equalmatch = false;
        }
    }
    
    /**
     * Finds whether a concavity matches one or more objects in a list, and
     * inserts these matches into the triangle representation.
     *
     * @param child1 The concavity to be considered.
     * @param children2 The concavities that the function should try to
     *                  match to <code>child1</code>.
     * @param minOverlMatch How much overlap between concavities is
     *                      necessary for them to be matched to each other.
     * @param root1 The root of the convex hull tree to which <code>child1
     *              </code> belongs.
     * @param root2 The root of the convex hull tree to which <code>children2
     *              </code> belong.
     * @param switched Indicates whether child1 belongs to the first or second
     *                 snapshot.
     */
    private void matchObjects(ConvexHullTreeNode child1,
            ConvexHullTreeNode[] children2,
            double minOverlMatch,
            ConvexHullTreeNode root1,
            ConvexHullTreeNode root2,
            boolean switched)
    {
        OverlapGraphEdge[] overlaps, overlaps2;
        Vector meetcriterion;
        TriangleRep concavity;
        Integer tmpvalue;
        int sumofoverlaps,nopoints,t1,index1,nextindex,index;
        ConvexHullTreeNode[] overlapswith;
        ConvexHullTreeNode tmpnode;
        LineWA[] deletelist;
        LineWA point;
        PointWNL tmppoint,point1,next;
        // Find overlapping nodes.
        overlaps = child1.getOverlapEdges();
        // If there is a one to one correspondence, create triangle
        // representation in subnodes and join them together
        // (It checks both nodes for this criterion.)
        
        if ((overlaps.length) == 1)
        {
            overlaps2 = overlaps[0].overlapswith.getOverlapEdges();
            if ((overlaps2.length == 1) && !(overlaps[0].overlapswith.isMatched()))
            {
                index = TriRepUtil.indexOf(children2, overlaps[0].overlapswith);
                if (index != -1)
                {
                    if (!switched)
                    {
                        concavity = new TriangleRep(child1, overlaps[0].overlapswith, minOverlMatch);
                    }
                    else
                    {
                        concavity = new TriangleRep(overlaps[0].overlapswith, child1, minOverlMatch);
                    }
                    
                    if (!switched)
                    {
                        if (!insertConcavity(concavity,root1,root2))
                        {
                            System.out.println(concavity.endpoints[0]);
                            System.out.println(concavity.endpoints[1]);
                            System.out.println(concavity.endpoints[2]);
                            System.out.println(concavity.endpoints[3]);
                            System.out.println(concavity.getNumberOfPoints());
                            throw(new TriRepCreationException("TriangleRep 288:Could not find enough matching points!"));
                        }
                    }
                    else
                    {
                        if (!insertConcavity(concavity,root2,root1))
                        {
                            throw(new TriRepCreationException("TriangleRep 292:Could not find enough matching points!"));
                        }
                    }
                    //	  System.out.println("Matching concavities");
                    child1.setMatched(true);
                    (overlaps[0].overlapswith).setMatched(true);
                    return;
                }
            }
        }
        // Code for handling multiple overlaps involving a single object in the
        // first snapshot and multiple objects in the second snapshot
        if ((overlaps.length) > 1)
        {
            // Create list of which of the overlapping objects overlap by more
            // than the minimum requirement:
            meetcriterion = new Vector();
            sumofoverlaps = 0;
            for (int b=0;b<overlaps.length;b++)
            {
                if (!(overlaps[b].overlapswith.isMatched()))
                {
                    overlaps2 = overlaps[b].overlapswith.getOverlapEdges();
                    if (overlaps2[overlaps2.length-1].overlapswith == child1)
                    {
                        index = TriRepUtil.indexOf(children2, overlaps[b].overlapswith);
                        if (index != -1)
                        {
                            meetcriterion.add(overlaps[b].overlapswith);
                            sumofoverlaps += overlaps[b].overlap;
                        }
                    }
                }
            }
            
            // Check whether the regions which meet the criterion overlap more
            // than min_overlap_match percent of children1[a].
            if (sumofoverlaps >= min_overlap_match)
            {
                // If only one object met the first criterion, use the procedure
                // for a single overlap:
                if (meetcriterion.size() == 1)
                {
                    if (!switched)
                    {
                        concavity = new TriangleRep(child1, (ConvexHullTreeNode)meetcriterion.elementAt(0), minOverlMatch);
                    }
                    else
                    {
                        concavity = new TriangleRep((ConvexHullTreeNode)meetcriterion.elementAt(0),child1, minOverlMatch);
                    }
                    if (!switched)
                    {
                        if (!insertConcavity(concavity,root1,root2))
                        {
                            throw(new TriRepCreationException("TriangleRep 310:Could not find any matches!"));
                        }
                    }
                    else
                    {
                        if (!insertConcavity(concavity,root2,root1))
                        {
                            throw(new TriRepCreationException("TriangleRep 314:Could not find any matches!"));
                        }
                    }
                    child1.setMatched(true);
                    tmpnode = (ConvexHullTreeNode)meetcriterion.elementAt(0);
                    tmpnode.setMatched(true);
                    return;
                }
                // Else rearrange the convex hull tree of the node with more than
                // one overlap so that there is a one-to-one correspondence of
                // tree nodes, and then treat this as a match.
                overlapswith = new ConvexHullTreeNode[meetcriterion.size()];
                meetcriterion.toArray(overlapswith);
                deletelist = root2.joinChildren(overlapswith);
                if (deletelist == null)
                {
                    System.exit(1);
                }
                // Remove nodes in trianglerep which were removed in overlap graph:
                nopoints = pointList.size();
                if (!switched)
                {
                    t1=1;
                }
                else t1=0;
                for (int d=0;d<deletelist.length;d++)
                {
                    // Finds point in point list.
                    point = deletelist[d];
                    point1 = null;
                    for (int a=0;a<nopoints;a++)
                    {
                        tmppoint = (PointWNL)pointList.elementAt(a);
                        if (tmppoint == null) continue;
                        if ((tmppoint.x == point.x) && (tmppoint.y == point.y) && (tmppoint.t == t1))
                        {
                            index1 = a;
                            point1 = tmppoint;
                        }
                    }
                    if (point1 == null)
                    {
                        throw(new TriRepCreationException("TriangleRep 344: Unable to find point in trianglerep."));
                    }
                    nextindex = 0;
                    next = null;
                    for (int a=0;a<point1.neighbours.size();a++)
                    {
                        tmpvalue = (Integer)point1.neighbours.elementAt(a);
                        tmppoint = (PointWNL)pointList.elementAt(tmpvalue.intValue());
                        if (tmppoint.t == point1.t)
                        {
                            nextindex = tmpvalue.intValue();
                            next = tmppoint;
                            break;
                        }
                    }
                    if (next == null)
                    {
                        throw(new TriRepCreationException("TriangleRep.java 358: Couldn't find next point!"));
                    }
                    joinPoints(point1, next);
                }
                // Recompute overlap graph for this node and child nodes
                TriRepUtil.computeOverlapGraph(root1, root2, min_overlap);
                // Call this function again with the modified convex hull tree and
                // overlap graph.
                matchObjects(child1,root2.getChildren(),minOverlMatch,root1, root2, switched);
                return;
            }
        }
    }
    
    /**
     * Inserts the snapshots into the triangle representation. Does not create
     * lines between points in different snapshots.
     *
     * @param poly1 The first snapshot represented as a list of line segments
     * @param poly2 The second snapshot represented as a list of line segments
     * @param minOverlMatch How much overlap between concavities is
     *                      necessary for them to be matched to each other.
     */
    private void insertSnapshots(LineWA[] poly1, LineWA[] poly2, double minOverlMatch)
    {
        int maxi1,maxi2;
        min_overlap = 2.0;
        pointList = new Vector();
        min_overlap_match = minOverlMatch;
        maxi1 = poly1.length;
        maxi2 = poly2.length;
        for (int a=0;a<maxi1;a++)
        {
            insertPoint(poly1[a].x, poly1[a].y, 0);
            if (a!=0) insertLine(a-1,a);
        }
        insertLine(maxi1-1,0);
        for (int b=0;b<maxi2;b++)
        {
            insertPoint(poly2[b].x, poly2[b].y, 1);
            if (b!=0) insertLine(b+maxi1-1,b+maxi1);
        }
        insertLine(maxi1+maxi2-1,maxi1);
    }
    
    /**
     * Creates a triangle representation from two arbitrarily shaped
     * objects represented by convex hull trees.
     *
     * @param root1 The root of the convex hull tree representing the
     *              first snapshot.
     * @param root2 The root of the convex hull tree representing the
     *              second snapshot.
     * @param minOverlMatch How much overlap between concavities is
     *                      necessary for them to be matched to each other.
     */
    private void createGeneralTriangleRep(ConvexHullTreeNode root1,
            ConvexHullTreeNode root2,
            double minOverlMatch)
    {
        ConvexHullTreeNode[] children1, children2, overlapswith;
        int smallestpoint,nopoints, nopoints2;
        // Insert the two snapshots into the triangle representation
        insertSnapshots(root1.getOrderedOutLine(), root2.getOrderedOutLine(), minOverlMatch);
        smallestpoint = root1.getSmallestPoint();
        nopoints = root1.numberOfLines();
        endpoints[0] = ((nopoints-smallestpoint) % nopoints);
        endpoints[1] = ((endpoints[0]+nopoints-1) % nopoints);
        smallestpoint = root2.getSmallestPoint();
        nopoints2 = root2.numberOfLines();
        endpoints[2] = ((nopoints2-smallestpoint) % nopoints2);
        endpoints[3] = ((endpoints[2]+nopoints2-1) % nopoints2);
        endpoints[2] += nopoints;
        endpoints[3] += nopoints;
        
        // Find concavities:
        children1 = root1.getChildren();
        children2 = root2.getChildren();
        // Find matches for concavities in the first snapshot
        for (int a=0;a<children1.length;a++)
        {
            if (!(children1[a].isMatched()))
            {
                matchObjects(children1[a], children2, minOverlMatch, root1, root2,false);
            }
        }
        // Must update children lists because of possible changes:
        children1 = root1.getChildren();
        children2 = root2.getChildren();
        // Find matches for concavities in the second snapshot which haven't
        // already been matched.
        for (int a=0;a<children2.length;a++)
        {
            if (!(children2[a].isMatched()))
            {
                matchObjects(children2[a], children1, minOverlMatch, root2, root1,true);
            }
        }
        // Must update children lists because of possible changes:
        children1 = root1.getChildren();
        children2 = root2.getChildren();
        // Create triangles for unmatched points.
        createConvexTriangleRep(root1.getOrderedOutLine(), root2.getOrderedOutLine());
        // Match unmatched objects in the first snapshot to points
        for (int a=0;a<children1.length;a++)
        {
            if (!(children1[a].isMatched()))
            {
                matchObjectToPoint(children1[a], root1, 0);
                children1[a].setMatched(true);
            }
        }
        // Match unmatched objects in the second snapshot to points
        for (int a=0;a<children2.length;a++)
        {
            if (!(children2[a].isMatched()))
            {
                matchObjectToPoint(children2[a], root2, 1);
                children2[a].setMatched(true);
            }
        }
    }
    
    /**
     * Matches a convex hull tree node to a point.
     *
     * @param child1 The convex hull tree node to be matched.
     * @param root1 The root of the convex hull tree to which child1 belongs
     * @param obj Indicates whether child1 belongs to the first or second
     *            snapshot.
     */
    private void matchObjectToPoint(ConvexHullTreeNode child1,
            ConvexHullTreeNode root1, int obj)
    {
        LineWA[] lineforchild, linesinconc;
        int[] pointindexes;
        lineforchild = root1.getLineForChild(child1);
        pointindexes = getCorrespondingPoint(lineforchild[0].x,
                lineforchild[0].y, lineforchild[1].x,
                lineforchild[1].y, obj);
        if (pointindexes == null)
        {
            throw(new TriRepCreationException("TriangleRep.java 451: Couldn't find a third point!"));
        }
        linesinconc = child1.getLines();
        insertHalfConcavity(linesinconc, pointindexes[0],
                pointindexes[1], pointindexes[2], obj);
    }
    
    /**
     * Insert a concavity in one snapshot that is matched to a point
     * in the other into the graph representation.
     *
     * @param lines The concavity represented as a list of lines
     * @param p1 The first point in the line which is to be replaced
     *           by the concavity.
     * @param p2 The second point in the line which is to be replaced
     *           by the concavity.
     * @param copoint The point in the other snapshot which is the
     *                "third corner of the triangle" with respect to
     *                <code>p1</code> and <code>p2</code>.
     * @param t Indicates which snapshot the concavity belongs to.
     */
    public void insertHalfConcavity(LineWA[] lines, int p1,
            int p2, int copoint, int t)
    {
        int cpoint, prevpoint, noinserts;
        removeLine(p1, p2);
        // lines[0] is the same point as p1, and lines[max] is p2.
        cpoint = insertPoint(lines[1].x, lines[1].y, t);
        insertLine(cpoint, p1);
        insertLine(cpoint, copoint);
        prevpoint = cpoint;
        noinserts = lines.length-1;
        for (int a=2;a<noinserts;a++)
        {
            cpoint = insertPoint(lines[a].x, lines[a].y, t);
            insertLine(cpoint, prevpoint);
            insertLine(cpoint, copoint);
            prevpoint = cpoint;
        }
        insertLine(cpoint, p2);
    }
    
    /**
     * Inserts a point into the graph structure.
     *
     * @param x The x-coordinate of the point
     * @param y The y-coordinate of the point
     * @param t Which snapshot the point belongs to.
     *
     * @return The index of the point in the point list
     */
    public int insertPoint(int x, int y, int t)
    {
        PointWNL newpoint;
        newpoint = new PointWNL(x,y,t);
        pointList.addElement(newpoint);
        return(pointList.size()-1);
    }
    
    /**
     * Inserts a line from p1 to p2 into the graph. p1 and p2 must have been
     * registered previously.
     *
     * @param p1 The starting point of the line
     * @param p2 The ending point of the line
     */
    public void insertLine(int p1, int p2)
    {
        PointWNL point1, point2;
        point1 = (PointWNL)pointList.elementAt(p1);
        point2 = (PointWNL)pointList.elementAt(p2);
        point1.neighbours.addElement(new Integer(p2));
        point2.neighbours.addElement(new Integer(p1));
    }
    
    /**
     * Removes a point from the list of neighbours of another point.
     *
     * @param neighbourlist The list of neighbours from which the point
     *                      should be removed
     * @param point The index of the point to be removed.
     *
     * @return TRUE if the point was removed, FALSE if it wasn't in the
     *         neighbour list.
     */
    private boolean removeNeighbour(Vector neighbourlist, int point)
    {
        boolean retvalue;
        int length = neighbourlist.size();
        Integer value;
        retvalue = false;
        for (int a=0;a<length;a++)
        {
            value = (Integer)neighbourlist.elementAt(a);
            if (value.intValue() == point)
            {
                neighbourlist.removeElementAt(a);
                length--;
                retvalue = true;
            }
        }
        return(retvalue);
    }
    
    /**
     * Removes a previously inserted line from the graph structure.
     *
     * @param p1 The index of the starting point of the line.
     * @param p2 The index of the ending point of the line.
     *
     * @return FALSE if there is no such line, TRUE otherwise.
     */
    public boolean removeLine(int p1, int p2)
    {
        PointWNL point1, point2;
        boolean retvalue;
        point1 = (PointWNL)pointList.elementAt(p1);
        point2 = (PointWNL)pointList.elementAt(p2);
        if ((point1 == null) || (point2 == null)) return(false);
        retvalue = removeNeighbour(point1.neighbours, p2);
        retvalue &= removeNeighbour(point2.neighbours, p1);
        return(retvalue);
    }
    
    /**
     * Removes a point from the graph structure. To maintain the numbering
     * of the points, the point is set to null instead of actually deleted.
     * This function also removes all neighbour references to the point.
     *
     * @param p1 The index of the point to be removed
     *
     * @return TRUE if the point was successfully removed, FALSE otherwise.
     */
    public boolean removePoint(int p1)
    {
        PointWNL point;
        Integer tmpvalue;
        point = (PointWNL)pointList.elementAt(p1);
        if (point == null) return(false);
        // Remove all neighbour references to this point
        for (int a=0;a<point.neighbours.size();a++)
        {
            tmpvalue = (Integer)point.neighbours.elementAt(a);
            removeLine(p1,tmpvalue.intValue());
        }
        // Remove  the point itself
        pointList.set(p1,null);
        return(true);
    }
    
    /**
     * Joins two points. Removes point 1 and adds it's neighbours to the list of
     * neighbours in point 2. Also updates the references of these
     * neighbours. This function is used to remove multiple points with the
     * same position.
     *
     * @param point1 The first point to be joined.
     * @param point2 The second point to be joined.
     */
    private void joinPoints(PointWNL point1, PointWNL point2)
    {
        int nopoints, noneighbours, p1, p2, nbindex;
        Integer tmppoint;
        PointWNL neighbour;
        p1 = pointList.indexOf(point1);
        p2 = pointList.indexOf(point2);
        nopoints = point1.neighbours.size();
        // For each neighbour of point 1
        for (int a=0;a<nopoints;a++)
        {
            // Insert the index of the point into the neighbour list of point 2
            point2.neighbours.addElement(point1.neighbours.elementAt(a));
            // Change the references in the neighbouring point to point to point 2
            // instead of point 1.
            tmppoint = (Integer)point1.neighbours.elementAt(a);
            neighbour = (PointWNL)pointList.elementAt(tmppoint.intValue());
            noneighbours = neighbour.neighbours.size();
            for (int b=0;b<noneighbours;b++)
            {
                tmppoint = (Integer)neighbour.neighbours.elementAt(b);
                if (tmppoint.intValue() == p1)
                {
                    neighbour.neighbours.set(b, new Integer(p2));
                }
            }
            TriRepUtil.removeDuplicates(neighbour.neighbours);
        }
        TriRepUtil.removeDuplicates(point2.neighbours);
        if (endpoints[0] == p1) endpoints[0] = p2;
        if (endpoints[1] == p1) endpoints[1] = p2;
        if (endpoints[2] == p1) endpoints[2] = p2;
        if (endpoints[3] == p1) endpoints[3] = p2;
        // Check the neighbour list of point 2 for references to itself:
        // (These will occur if points 1 and 2 were neighbours.)
        for (int a=0;a<point2.neighbours.size();a++)
        {
            tmppoint = (Integer)point2.neighbours.elementAt(a);
            if ((tmppoint.intValue() == p2) || (tmppoint.intValue() == p1))
            {
                point2.neighbours.removeElementAt(a);
                a--;
            }
        }
        removePoint(p1);
    }
    
    /**
     * Returns the position of a point in space-time as an integer array.
     *
     * @param p1 The point.
     *
     * @return An array containing the position of the point.
     *         position 0 = x,
     *         position 1 = y,
     *         position 2 = t.
     *         If the point no longer exists, it returns an array with three
     *         -1 values.
     */
    public int[] getPosition(int p1)
    {
        int[] coords = new int[3];
        PointWNL point = (PointWNL)pointList.elementAt(p1);
        if (point == null)
        {
            coords[0] = -1;
            coords[1] = -1;
            coords[2] = -1;
            return(coords);
        }
        coords[0] = point.x;
        coords[1] = point.y;
        coords[2] = point.t;
        return(coords);
    }
    
    /**
     * This function is used to find which points are the neighbours of a
     * given point.
     *
     * @param p1 The index of the point.
     *
     * @return An array of integers, which contains the indexes of the
     *         neighbours of the point.
     */
    public int[] getNeighbours(int p1)
    {
        PointWNL point = (PointWNL)pointList.elementAt(p1);
        Integer temp;
        int arraysize = point.neighbours.size();
        int [] returnarray = new int[arraysize];
        for (int a=0;a<arraysize;a++)
        {
            temp = (Integer)point.neighbours.elementAt(a);
            returnarray[a] = temp.intValue();
        }
        return(returnarray);
    }
    
    /**
     * Test whether two points are neighbours.
     *
     * @param p1 The first point.
     * @param p2 The second point.
     *
     * @return TRUE if there is a line in the graph from p1 to p2,
     *         FALSE otherwise.
     */
    public boolean neighbours(int p1, int p2)
    {
        int[] neighbours;
        int index;
        neighbours = getNeighbours(p1);
        index = -1;
        for (int a=0;a<neighbours.length;a++)
        {
            if (neighbours[a] == p2)
            {
                index = a;
                break;
            }
        }
        if (index == -1) return(false);
        return(true);
    }
    
    /**
     * Get the number of neighbours that a given point has.
     *
     * @param p1 the point.
     *
     * @return The number of lines in the graph which go to/from p1.
     */
    public int getNumberOfNeighbours(int p1)
    {
        PointWNL point = (PointWNL)pointList.elementAt(p1);
        return(point.neighbours.size());
    }
    
    /**
     * This function is used to find which points are the neighbours of a
     * given point.
     *
     * @param p1 The point given as a PointWNL object.
     *
     * @return An array of integers, which contains the indexes of the
     *         neighbours of the point.
     */
    public int[] getNeighbours(Object p)
    {
        Integer temp;
        PointWNL point = (PointWNL)p;
        int arraysize = point.neighbours.size();
        int [] returnarray = new int[arraysize];
        for (int a=0;a<arraysize;a++)
        {
            temp = (Integer)point.neighbours.elementAt(a);
            returnarray[a] = temp.intValue();
        }
        return(returnarray);
    }
    
    /**
     * Determines how many points there are in the graph representation.
     * This number includes points which have been inserted and subsequently
     * deleted.
     *
     * @return The number of elements in the point list.
     */
    public int getNumberOfPoints()
    {
        return(pointList.size());
    }
    
  /*
    A collection of functions which are used to convert between points and
    integer index values. These are used by the insertConcavity function.
   */
    
    /**
     * This function returns the list of points as an array of objects. Because
     * the points are really <code>PointWNL</code> objects, they cannot be
     * interpreted by the functions in other classes.
     *
     * @return the list of points.
     */
    public Object[] getPoints()
    {
        return(pointList.toArray());
    }
    
    public Object getPoint(int index)
    {
        return(pointList.elementAt(index));
    }
    
    /**
     * This function checks whether a point is stored in a given index in the
     * point list.
     *
     * @param index The index.
     *
     * @return TRUE if a point is stored at that index in the point list,
     *         FALSE if the point at that position has been deleted.
     */
    public boolean pointExists(int index)
    {
        if (pointList.elementAt(index) == null)
        {
            return(false);
        }
        else return(true);
    }
    
    /**
     * This function is used to get the points in one snapshot from the
     * graph representation.
     *
     * @param time Indicates which snapshot should be returned.
     *
     * @return The points in the graph structure that belong to the given
     *         snapshot. They are returned in the order in which that were
     *         inserted, which is not necessarily counterclockwise.
     */
    public int[] getSnapshot(int time)
    {
        Vector tmpres;
        PointWNL point;
        int[] result;
        Integer tmpval;
        tmpres = new Vector();
        for (int a=0;a<pointList.size();a++)
        {
            point = (PointWNL)pointList.elementAt(a);
            if (point == null) continue;
            if (point.t == time) tmpres.add(new Integer(a));
        }
        result = new int[tmpres.size()];
        for (int a=0;a<result.length;a++)
        {
            tmpval = (Integer)tmpres.elementAt(a);
            result[a] = tmpval.intValue();
        }
        return(result);
    }
    
    /**
     * This function is used to get the points in one snapshot from the
     * graph representation and sort them so that they come in
     * counterclockwise order.
     *
     * @param time Indicates which snapshot should be returned.
     *
     * @return The points in the graph structure that belong to the given
     *         snapshot.
     */
    public int[] getOrderedSnapshot(int time)
    {
        int[] unorderedSnapshot;
        int[] neighbours;
        int point;
        int prev;
        int next;
        PointWNL cpoint, ppoint, npoint;
        int[] orderedSnapshot;
        int[] swSnapshot;
        int index,a,xvalue,l;
        // Initializing variables and finding the first point
        unorderedSnapshot = getSnapshot(time);
        orderedSnapshot = new int[unorderedSnapshot.length];
        index = 0;
        prev = unorderedSnapshot[0];
        orderedSnapshot[index]=prev;
        index++;
        neighbours = getNeighbours(unorderedSnapshot[0]);
        point = -1;
        // Finding the next point
        for (a=0;a<neighbours.length;a++)
        {
            cpoint = (PointWNL)pointList.elementAt(neighbours[a]);
            if (cpoint.t == time)
            {
                point = neighbours[a];
                break;
            }
        }
        // Finding the other points travelling in the same direction
        next = -1;
        while (index < orderedSnapshot.length)
        {
            neighbours = getNeighbours(point);
            for (a=0;a<neighbours.length;a++)
            {
                if (neighbours[a] == prev) continue;
                cpoint = (PointWNL)pointList.elementAt(neighbours[a]);
                if (cpoint.t == time)
                {
                    next = neighbours[a];
                    break;
                }
            }
            prev = point;
            orderedSnapshot[index]=prev;
            index++;
            if (next == -1)
            {
                throw (new TriRepException("TriangleRep 1040: Could not find next point in snapshot!"));
            }
            point = next;
        }
        // Checking whether the ordered snapshot is counterclockwise or not:
        // - finding the leftmost point
        index = -1;
        xvalue = Integer.MAX_VALUE;
        for (a=0;a<orderedSnapshot.length;a++)
        {
            cpoint = (PointWNL)pointList.elementAt(orderedSnapshot[a]);
            if (cpoint.x < xvalue)
            {
                index = a;
                xvalue = cpoint.x;
            }
        }
        // - Checking direction of travel
        cpoint = (PointWNL)pointList.elementAt(orderedSnapshot[index]);
        if (index > 0)
        {
            ppoint = (PointWNL)pointList.elementAt(orderedSnapshot[index-1]);
        }
        else ppoint = (PointWNL)pointList.elementAt(orderedSnapshot[orderedSnapshot.length-1]);
        if (index < orderedSnapshot.length-1)
        {
            npoint = (PointWNL)pointList.elementAt(orderedSnapshot[index+1]);
        }
        else npoint = (PointWNL)pointList.elementAt(orderedSnapshot[0]);
        if (((ppoint.y > cpoint.y) && (cpoint.y > npoint.y)) ||
                ((ppoint.y > cpoint.y) && (cpoint.x < npoint.x)) ||
                ((cpoint.y > npoint.y) && (ppoint.x > cpoint.x)))
        {
            return (orderedSnapshot);
        }
        else
        {
            l = orderedSnapshot.length;
            swSnapshot = new int[l];
            for (a=0;a<l;a++)
            {
                swSnapshot[a] = orderedSnapshot[l-a-1];
            }
            return(swSnapshot);
        }
    }
    
    /**
     * This function determines which point in the graph structure lies at
     * a given coordinate value.
     *
     * @param x The x-coordinate of the point
     * @param y The y-coordinate of the point
     * @param t Which snapshot the point belongs to.
     *
     * @return The index of the point in the point list. If there is no point
     *         at the given position, the function returns -1.
     */
    public int findPoint(int x1, int y1, int t1)
    {
        int index, nopoints;
        PointWNL tmppoint;
        index = -1;
        nopoints = pointList.size();
        for (int a=0;a<nopoints;a++)
        {
            tmppoint = (PointWNL)pointList.elementAt(a);
            if (tmppoint == null) continue;
            if ((tmppoint.x == x1) && (tmppoint.y == y1) && (tmppoint.t == t1))
            {
                index = a;
            }
            if (index != -1) break;
        }
        return(index);
    }
    
    /**
     * This function moves the coordinate values of each point
     * in the given snapshot. It is used after the interpolation to move
     * one of the snapshots back to it's original position.
     *
     * @param dx How far the snapshot should be moved in the x direction.
     * @param dy How far the snapshot should be moved in the y direction.
     * @param t Which snapshot should be moved.
     */
    public void translateSnapshot(int dx, int dy, int time)
    {
        PointWNL point;
        for (int a=0;a<pointList.size();a++)
        {
            point = (PointWNL)pointList.elementAt(a);
            if (point == null) continue;
            if (point.t == time)
            {
                point.x += dx;
                point.y += dy;
            }
        }
    }
    
    /**
     * Given two points in the same snapshot, finds the third point which
     * forms a triangle with the two points.
     *
     * @param x1 The x-coordinate of the first point.
     * @param y1 The y-coordinate of the first point.
     * @param x2 The x-coordinate of the second point.
     * @param y2 The y-coordinate of the second point.
     * @param t Which snapshot the points belong to.
     *
     * @return The integer indexes of all three points in the graph
     *         representation. If it doesn't find either point 1 or
     *         point 2, or it doesn't find a third point, it returns null.
     */
    public int[] getCorrespondingPoint(int x1, int y1, int x2, int y2, int t1)
    {
        int nopoints, index1, index2, newpoint;
        PointWNL tmppoint,point1, point2;
        Integer tmpvalue;
        int[] returnvalue;
        nopoints = pointList.size();
        index1 = -1;
        index2 = -1;
        point1 = null;
        point2 = null;
        // Tries to find the points in the triangle representation
        for (int a=0;a<nopoints;a++)
        {
            tmppoint = (PointWNL)pointList.elementAt(a);
            if (tmppoint == null) continue;
            if ((tmppoint.x == x1) && (tmppoint.y == y1) && (tmppoint.t == t1))
            {
                index1 = a;
                point1 = tmppoint;
            }
            if ((tmppoint.x == x2) && (tmppoint.y == y2) && (tmppoint.t == t1))
            {
                index2 = a;
                point2 = tmppoint;
            }
        }
        if ((index1 == -1) || (index2 == -1)) return(null);
        // Checks whether the two points are neighbours
        if (!point1.neighbours.contains(new Integer(index2)))
        {
            System.out.print("WARNING: TriangleRep 749: ");
            System.out.println("Tried to find corresponding point for non-neighbouring points!");
            return(null);
        }
        // Tries to find a third point which is a neighbour of both points
        nopoints = point1.neighbours.size();
        returnvalue = null;
        for (int a=0;a<nopoints;a++)
        {
            if (point2.neighbours.contains(point1.neighbours.elementAt(a)))
            {
                tmpvalue = (Integer)point1.neighbours.elementAt(a);
                tmppoint = (PointWNL)pointList.elementAt(tmpvalue.intValue());
                // For triangles, there may be two points which are neighbours of both
                // input points, only one of which is the correct corresponding point.
                // One can elimimate the second point by checking which point is a
                // neighbour of the third point in the triangle. (That is, which point
                // has five neighbours.)
                if (tmppoint.t != t1)
                {
                    if (returnvalue != null)
                    {
                        if (tmppoint.neighbours.size() < 5)
                        {
                            returnvalue[0] = index1;
                            returnvalue[1] = index2;
                            returnvalue[2] = tmpvalue.intValue();
                        }
                        continue;
                    }
                    returnvalue = new int[3];
                    returnvalue[0] = index1;
                    returnvalue[1] = index2;
                    returnvalue[2] = tmpvalue.intValue();
                }
            }
        }
        return(returnvalue);
    }
    
    /**
     * Given two points in the same snapshot, finds the third point which
     * forms a triangle with the two points.
     *
     * @param index1 The index pf the first point.
     * @param index2 The index of the second point.
     *
     * @return The index of the third point. It returns -1 if no neighbour
     *         was found, the original points are not neighbours, or the
     *         original points have different time values.
     */
    public int getCorrespondingPoint(int index1, int index2)
    {
        int nopoints, returnvalue;
        Integer tmpvalue;
        PointWNL point1, point2, tmppoint;
        point1 = (PointWNL)pointList.elementAt(index1);
        point2 = (PointWNL)pointList.elementAt(index2);
        returnvalue = -1;
        if (point1.t != point2.t) return(-1);
        if (!point1.neighbours.contains(new Integer(index2)))
        {
            throw(new TriRepCreationException("TriangleRep.java 802: Tried to find corresponding point for non-neighbouring points!"));
        }
        nopoints = point1.neighbours.size();
        for (int a=0;a<nopoints;a++)
        {
            if (point2.neighbours.contains(point1.neighbours.elementAt(a)))
            {
                tmpvalue = (Integer)point1.neighbours.elementAt(a);
                tmppoint = (PointWNL)pointList.elementAt(tmpvalue.intValue());
                if (tmppoint.t != point1.t)
                {
                    if (returnvalue != -1)
                    {
                        if (tmppoint.neighbours.size() < 5)
                        {
                            returnvalue = tmpvalue.intValue();
                        }
                        continue;
                    }
                    returnvalue = tmpvalue.intValue();
                }
            }
        }
        return(returnvalue);
    }
    
    /**
     * Inserts a concavity into this graph representation by removing two
     * lines from the graph representation and replacing
     * them with the graph representation of the concavity object. The lines
     * which are removed are those that match with lines in the concavity object.
     * If there are no such matches, the function does nothing.
     *
     * @param concavity The concavity to be inserted.
     * @param root1 The root of the first convex hull tree.
     * @param root2 The root of the second convex hull tree.
     *
     * @return TRUE if the function successfully inserted the concavity,
     *         FALSE otherwise.
     */
    public boolean insertConcavity(TriangleRep concavity,
            ConvexHullTreeNode root1,
            ConvexHullTreeNode root2)
    {
        Vector cspoints;      // Stores the points which are common to both trireps
        Vector deletedlines;  // Stores the lines which are deleted
        Vector endpoints;     // Stores the endpoints of the concavity and object
        Vector tmplist;
        Vector csinthis[];
        Object[] points;
        LineWA[] chlines;
        Integer tmpvalue, tmpvalue2;
        int nopoints, nopoints2, pointref1, nindex, counter, pointref2;
        int[] tmppoint1, tmppoint2;
        int[] neighbours, linecounters;
        int a,b, indexvalue, deletevalue, cindex,distx, disty, distt, rootpos;
        PointWNL newpoint, point, point2, tmppoint;
        cspoints = new Vector();
        nopoints = pointList.size();
        nopoints2 = concavity.getNumberOfPoints();
        csinthis = new Vector[2];
        csinthis[0] = new Vector();
        csinthis[1] = new Vector();
        // Builds list of points common to both triangle representations.
        // Points are common if they have the same position.
        for (a=0;a<nopoints;a++)
        {
            tmppoint1 = getPosition(a);
            if (tmppoint1[0] == -1) continue;
            for (b=0;b<4;b++)
            {
                tmppoint2 = concavity.getPosition(concavity.endpoints[b]);
                if (tmppoint2[0] == -1)
                {
                    throw(new TriRepCreationException("TriangleRep.java 891: A registered endpoint has been deleted!"));
                }
                if ((tmppoint1[0] == tmppoint2[0]) && (tmppoint1[1] == tmppoint2[1])
                && (tmppoint1[2] == tmppoint2[2]))
                {
                    // Check whether the points found are on the convex hull or not.
                    // If not, they should not be counted.
                    rootpos = -1;
                    chlines = root1.getOutLine();
                    for (int c=0;c<chlines.length;c++)
                    {
                        if ((chlines[c].x == tmppoint1[0]) && (chlines[c].y == tmppoint1[1])
                        && (tmppoint1[2] == 0))
                        {
                            rootpos = c;
                            break;
                        }
                    }
                    if (rootpos == -1)
                    {
                        chlines = root2.getOutLine();
                        for (int c=0;c<chlines.length;c++)
                        {
                            if ((chlines[c].x == tmppoint1[0]) && (chlines[c].y == tmppoint1[1])
                            && (tmppoint1[2] == 1))
                            {
                                rootpos = c;
                                break;
                            }
                        }
                    }
                    if (rootpos != -1)
                    {
                        cspoints.addElement(new Integer(a));
                        cspoints.addElement(new Integer(concavity.endpoints[b]));
                        csinthis[tmppoint1[2]].addElement(new Integer(a));
                    }
                }
            }
        }
        if (cspoints.size() != 8)
        {
            System.out.println(cspoints.size());
            return(false);
        }
        // Remove all lines which go between points in the cspoints list
        // Removing lines in this object
        deletedlines = new Vector();
        for (a=0;a<cspoints.size();a+=2)
        {
            pointref1 = ((Integer)cspoints.elementAt(a)).intValue();
            neighbours = getNeighbours(pointref1);
            for (b=0;b<neighbours.length;b++)
            {
                nindex = cspoints.indexOf(new Integer(neighbours[b]));
                while (nindex != -1)
                {
                    if ((nindex % 2) == 0)
                    {
                        if (removeLine(neighbours[b],pointref1))
                        {
                            deletedlines.addElement(new Integer(neighbours[b]));
                            deletedlines.addElement(new Integer(pointref1));
                            break;
                        }
                    }
                    nindex = cspoints.indexOf(new Integer(neighbours[b]), nindex+1);
                }
            }
        }
        // Removing lines in the other TriangleRep object
        for (a=1;a<cspoints.size();a+=2)
        {
            pointref1 = ((Integer)cspoints.elementAt(a)).intValue();
            neighbours = concavity.getNeighbours(pointref1);
            for (b=0;b<neighbours.length;b++)
            {
                nindex = cspoints.indexOf(new Integer(neighbours[b]));
                while (nindex != -1)
                {
                    if ((nindex % 2) == 1)
                    {
                        if (concavity.removeLine(neighbours[b],pointref1))
                        {
                            deletedlines.addElement(new Integer(neighbours[b] + nopoints));
                            deletedlines.addElement(new Integer(pointref1 + nopoints));
                            break;
                        }
                    }
                    nindex = cspoints.indexOf(new Integer(neighbours[b]), nindex+1);
                }
            }
        }
        // If there are points with no lines going out from them, remove them.
        for (a=0;a<cspoints.size();a+=2)
        {
            pointref1 = ((Integer)cspoints.elementAt(a)).intValue();
            if (getNumberOfNeighbours(pointref1) == 0)
            {
                removePoint(pointref1);
            }
        }
        for (a=1;a<cspoints.size();a+=2)
        {
            pointref1 = ((Integer)cspoints.elementAt(a)).intValue();
            if (concavity.getNumberOfNeighbours(pointref1) == 0)
            {
                concavity.removePoint(pointref1);
            }
        }
        // Insert the remainder of the concavity triangle representation.
        nopoints = pointList.size();
        points = concavity.getPoints();
        for (a=0;a<points.length;a++)
        {
            if (points[a] != null)
            {
                point = (PointWNL)points[a];
                newpoint = new PointWNL();
                newpoint.x = point.x;
                newpoint.y = point.y;
                newpoint.t = point.t;
                for (b=0;b<point.neighbours.size();b++)
                {
                    tmpvalue = (Integer)point.neighbours.elementAt(b);
                    indexvalue = tmpvalue.intValue();
                    indexvalue += nopoints;
                    newpoint.neighbours.addElement(new Integer(indexvalue));
                }
                pointList.addElement(newpoint);
            }
            else pointList.addElement(null);
        }
        // Find the end points of the former triangle representations
        // (The points at which they meet).
        // These are all the points which have only one neighbour with the same
        // time. Other points have two.
        nopoints = pointList.size();
        endpoints = new Vector();
        for (a=0;a<nopoints;a++)
        {
            point = (PointWNL)pointList.elementAt(a);
            if (point == null) continue;
            nopoints2 = point.neighbours.size();
            counter = 0;
            for (b=0;b<nopoints2;b++)
            {
                tmpvalue = (Integer)point.neighbours.elementAt(b);
                tmppoint = (PointWNL)pointList.elementAt(tmpvalue.intValue());
                if (tmppoint.t == point.t) counter++;
            }
            if (counter < 2) endpoints.addElement(point);
        }
        // Remove duplicate endpoints
        nopoints = endpoints.size();
        for (a=0;a<nopoints;a++)
        {
            point = (PointWNL)endpoints.elementAt(a);
            for (b=a+1;b<nopoints;b++)
            {
                point2 = (PointWNL)endpoints.elementAt(b);
                if ((point.x == point2.x) && (point.y == point2.y) &&
                        (point.t == point2.t))
                {
                    joinPoints(point, point2);
                }
            }
        }
        linecounters = new int[pointList.size()];
        for (a=0;a<pointList.size();a++)
        {
            linecounters[a] = 0;
        }
        // Reinsert lines connecting corresponding endpoints into the
        // representation.
        for (a=0;a<nopoints;a++)
        {
            point = (PointWNL)endpoints.elementAt(a);
            if (point == null) continue;
            pointref1 = pointList.indexOf(point);
            if (pointref1 == -1) continue;
            nindex = deletedlines.indexOf(new Integer(pointref1));
            while (nindex != -1)
            {
                if ((nindex % 2) == 0)
                {
                    tmpvalue = (Integer)deletedlines.elementAt(nindex+1);
                    deletevalue = nindex;
                }
                else
                {
                    tmpvalue = (Integer)deletedlines.elementAt(nindex-1);
                    deletevalue = nindex-1;
                }
                if (tmpvalue == null)
                {
                    throw(new TriRepCreationException("TriangleRep.java 986: tmpvalue is null!"));
                }
                pointref2 = tmpvalue.intValue();
                point2 = (PointWNL)pointList.elementAt(pointref2);
                if (endpoints.contains(point2))
                {
                    if (point.t != point2.t)
                    {
                        insertLine(pointref1, pointref2);
                        linecounters[pointref1]++;
                        linecounters[pointref2]++;
                    }
                    deletedlines.remove(nindex);
                    deletedlines.remove(deletevalue);
                }
                nindex = deletedlines.indexOf(new Integer(pointref1));
            }
        }
        // Remove duplicates in neighbour lists
        for (a=0;a<pointList.size();a++)
        {
            point = (PointWNL)pointList.elementAt(a);
            if (point == null) continue;
            TriRepUtil.removeDuplicates(point.neighbours);
        }
        // Remove lines that shouldn't have been reinserted.
        // (I probably should find a better algorithm for this. One line
        // may be deleted, reinserted, and then deleted again! :-)
        tmplist = new Vector();
        for (a=0;a<pointList.size();a++)
        {
            if (linecounters[a]>1)
            {
                tmplist.addElement(new Integer(a));
            }
        }
    /*    for (a=0;a<tmplist.size();a++) {
      tmpvalue = (Integer)tmplist.elementAt(a);
      if (a+1 < tmplist.size()) {
        tmpvalue2 = (Integer)tmplist.elementAt(a+1);
      } else continue;
      if (removeLine(tmpvalue.intValue(), tmpvalue2.intValue())) {
        tmplist.removeElementAt(a);
        tmplist.removeElementAt(a);
        a--;
      }
      }*/
        for (a=0;a<tmplist.size();a++)
        {
            tmpvalue = (Integer)tmplist.elementAt(a);
            for (b=a+1;b<tmplist.size();b++)
            {
                tmpvalue2 = (Integer)tmplist.elementAt(b);
                if(removeLine(tmpvalue.intValue(), tmpvalue2.intValue()))
                {
                    linecounters[tmpvalue2.intValue()]--;
                    if (linecounters[tmpvalue2.intValue()] <= 1)
                    {
                        tmplist.removeElementAt(b);
                        b--;
                    }
                    linecounters[tmpvalue.intValue()]--;
                    if (linecounters[tmpvalue.intValue()] <= 1)
                    {
                        tmplist.removeElementAt(a);
                        a--;
                        break;  // when a is deleted, begin testing with the next a.
                    }
                }
            }
        }
        return(true);
    }
    
    /**
     * Writes the individual triangular surfaces containing one line from
     * the given snapshot into the VRML file.
     *
     * @param time The snapshot.
     * @param fs The file to write to.
     * @param convertlist List which converts indexes in the point list of
     *                    the graph structure to indexes in the point list
     *                    of the VRML file.
     *
     * @throws IOException If the function cannot write to the file.
     */
    public void writeTriangles(int time, OutputStreamWriter fs,
            Vector convertlist) throws IOException
    {
        PointWNL point, prev, next, tmppoint, start, corr;
        Integer tmpvalue;
        int[] snap1;
        int nextindex, corrindex, pointindex, previndex;
        Vector pointorder;
        pointorder = new Vector();
        snap1 = getSnapshot(time);
        previndex = snap1[0];
        prev = (PointWNL)pointList.elementAt(previndex);
        start = prev;
        point = null;
        pointindex = -1;
        for (int a=0;a<prev.neighbours.size();a++)
        {
            tmpvalue = (Integer)prev.neighbours.elementAt(a);
            tmppoint = (PointWNL)pointList.elementAt(tmpvalue.intValue());
            if (tmppoint.t == prev.t)
            {
                pointindex = tmpvalue.intValue();
                point = tmppoint;
                break;
            }
        }
        if (point == null)
        {
            throw(new TriRepException("TriangleRep.java 1074: Couldn't find neighbour with same time value!"));
        }
        //    System.out.println("Before loop");
        do
        {
            // Insert triangle in vrml file
            fs.write("         ");
            corrindex = getCorrespondingPoint(previndex, pointindex);
            if (corrindex != -1)
            {
                corr = (PointWNL)pointList.elementAt(corrindex);
                tmpvalue = (Integer)convertlist.elementAt(previndex);
                fs.write(tmpvalue.toString() + " ");
                tmpvalue = (Integer)convertlist.elementAt(pointindex);
                fs.write(tmpvalue.toString() + " ");
                tmpvalue = (Integer)convertlist.elementAt(corrindex);
                fs.write(tmpvalue.toString() + " -1\n");
                //	System.out.println("Have written to file");
            }
            // Find next point
            next = null;
            nextindex = -1;
            for (int a=0;a<point.neighbours.size();a++)
            {
                tmpvalue = (Integer)point.neighbours.elementAt(a);
                tmppoint = (PointWNL)pointList.elementAt(tmpvalue.intValue());
                if ((tmppoint.t == point.t) && (tmppoint != prev))
                {
                    nextindex = tmpvalue.intValue();
                    next = tmppoint;
                    break;
                }
            }
            if (next == null)
            {
                throw(new TriRepException("TriangleRep.java 1104: Couldn't find next point!"));
            }
            pointorder.add(new Integer(previndex));
            prev = point;
            previndex = pointindex;
            point = next;
            pointindex = nextindex;
        } while (prev != start);
    }
    
    public LineWA[][] getSection(double time)
    {
        Vector res=new Vector();
        int neig;
        int[] snap=this.getOrderedSnapshot(0);
        for (int i=0;i<snap.length;i++)
        {            
            neig=this.getCorrespondingPoint(snap[i],snap[(i+1)%snap.length]);
            PointWNL corrPoint=(PointWNL)this.getPoint(neig);
            PointWNL P1=(PointWNL)this.getPoint(snap[i]);
            PointWNL P2=(PointWNL)this.getPoint(snap[(i+1)%snap.length]);
            LineWA[] tmp=new LineWA[2];
            tmp[0]= new LineWA(((int)Math.round(P1.x+(corrPoint.x-P1.x)*time)),(int)Math.round(P1.y+(corrPoint.y-P1.y)*time));
            tmp[1]= new LineWA((int)Math.round(P2.x+(corrPoint.x-P2.x)*time),(int)Math.round(P2.y+(corrPoint.y-P2.y)*time));
            res.add(tmp);                        
        }        
        snap=this.getOrderedSnapshot(1);
        for (int i=0;i<snap.length;i++)
        {            
            neig=this.getCorrespondingPoint(snap[i],snap[(i+1)%snap.length]);
            PointWNL corrPoint=(PointWNL)this.getPoint(neig);
            PointWNL P1=(PointWNL)this.getPoint(snap[i]);
            PointWNL P2=(PointWNL)this.getPoint(snap[(i+1)%snap.length]);
            LineWA[] tmp=new LineWA[2];
            tmp[0]= new LineWA(((int)Math.round(corrPoint.x+(P1.x-corrPoint.x)*time)),(int)Math.round(corrPoint.y+(P1.y-corrPoint.y)*time));
            tmp[1]= new LineWA((int)Math.round(corrPoint.x+(P2.x-corrPoint.x)*time),(int)Math.round(corrPoint.y+(P2.y-corrPoint.y)*time));
            res.add(tmp);                        
        }
        LineWA[][] res2=new LineWA[res.size()][2];
        for(int i=0;i<res.size();i++)
        {
            res2[i]=(LineWA[])res.elementAt(i);
        }
        
        return(res2);
    }
    
    /**
     * Creates a VRML file containing a VRML representation of the graph
     * (an indexedFaceSet).
     *
     * @param filename The file name of the VRML file.
     */
    public void saveAsVRML(String filename,int time) throws FileNotFoundException, IOException
    {
        FileOutputStream filestream;
        OutputStreamWriter fs;
        int cp, counter;
        Vector convertlist;
        PointWNL point;
        filestream = new FileOutputStream(filename);
        fs = new OutputStreamWriter(filestream);
        convertlist = new Vector();
        fs.write("#VRML V2.0 utf8\n\n");
        fs.write("Shape {\n");
        fs.write("   appearance Appearance {\n");
        fs.write("      material Material { diffuseColor 0.1 1 0 shininess .5 transparency 0.1}\n");
        fs.write("   }");
        fs.write("   geometry IndexedFaceSet {\n");
        fs.write("      coord Coordinate {\n");
        fs.write("         point [\n");
        counter = 0;
        for (int a=0;a<pointList.size();a++)
        {
            point = (PointWNL)pointList.elementAt(a);
            if (point == null)
            {
                convertlist.add(new Integer(-1));
                continue;
            }
            convertlist.add(new Integer(counter));
            counter++;
            fs.write("                ");
            fs.write(Float.toString((((float)point.x)/20)-5));
            fs.write(" ");
            fs.write(Float.toString((((float)point.y)/-20)-6));
            fs.write(" ");
            fs.write(Float.toString((((float)point.t))*time));
            fs.write("\n");
        }
        fs.write("               ]\n");
        fs.write("         }\n");
        fs.write("      coordIndex [\n");
        writeTriangles(0, fs, convertlist);
        writeTriangles(1, fs, convertlist);
        fs.write("      ]\n");
        fs.write("      solid FALSE\n");
        fs.write("   }\n");
        fs.write("}\n");
        
        convertlist = new Vector();
        fs.write("Shape {\n");
        fs.write("   appearance Appearance {\n");
        fs.write("      material Material { diffuseColor 1 0 0 shininess .5 transparency 0.4}\n");
        fs.write("   }");
        fs.write("   geometry IndexedFaceSet {\n");
        fs.write("      coord Coordinate {\n");
        fs.write("         point [\n");
        counter = 0;
        int counterPoly0=0;
        for (int a=0;a<pointList.size();a++)
        {
            point = (PointWNL)pointList.elementAt(a);
            if (point == null)
            {
                convertlist.add(new Integer(-1));
                continue;
            }
            convertlist.add(new Integer(counter));
            counter++;
            fs.write("                ");
            fs.write(Float.toString((((float)point.x)/20)-5));
            fs.write(" ");
            fs.write(Float.toString((((float)point.y)/-20)-6));
            fs.write(" ");
            fs.write(Float.toString((((float)point.t))*time));
            fs.write("\n");
            if(point.t==0) counterPoly0++;
        }
        fs.write("               ]\n");
        fs.write("         }\n");
        fs.write("      coordIndex [\n");
//        int[][] vertices=new int[counterPoly0][3];
//        int currentIndex;
//        int preindex=-1;
//        {
//            int j=0;
//            while(pointList.elementAt(j)==null||((PointWNL)pointList.elementAt(j)).t!=0)
//                j++;
//            vertices[0][0]=j;
//            vertices[0][1]=((PointWNL)pointList.elementAt(j)).x;
//            vertices[0][2]=((PointWNL)pointList.elementAt(j)).y;
//            currentIndex=j;
//        }
//        {
//            int i=1;
//            while(i<counterPoly0)
//            {
//                point = (PointWNL)pointList.elementAt(currentIndex);
//                if (point == null)
//                {
//                    continue;
//                }
//                for(int j=0;j<point.neighbours.size();j++)
//                {
//                    PointWNL point2 = (PointWNL)pointList.elementAt(((Integer)point.neighbours.elementAt(j)).intValue());
//                    if(point2.t==0&&((Integer)point.neighbours.elementAt(j)).intValue()!=preindex)
//                    {
//                        vertices[i][0]=((Integer)point.neighbours.elementAt(j)).intValue();
//                        vertices[i][1]=point2.x;
//                        vertices[i][2]=point2.y;
//                        preindex=currentIndex;
//                        currentIndex=((Integer)point.neighbours.elementAt(j)).intValue();
//                        i++;
//                        break;
//                    }
//                }
//            }
//        }
//
//        Convexer convexer=new Convexer(vertices);
        Convexer convexer=new Convexer(this.getOrderedSnapshot(0),this.pointList);
        System.out.println("Nächstaes Polygon:");
        convexer.writePolygone(fs,this.pointList);
        fs.write("      ]\n");
        fs.write("      solid FALSE\n");
        fs.write("   }\n");
        fs.write("}\n");
        
        convertlist = new Vector();
        fs.write("Shape {\n");
        fs.write("   appearance Appearance {\n");
        fs.write("      material Material { diffuseColor 0 .5 1 shininess .5 transparency 0.4}\n");
        fs.write("   }");
        fs.write("   geometry IndexedFaceSet {\n");
        fs.write("      coord Coordinate {\n");
        fs.write("         point [\n");
        counter = 0;
        int counterPoly1=0;
        for (int a=0;a<pointList.size();a++)
        {
            point = (PointWNL)pointList.elementAt(a);
            if (point == null)
            {
                convertlist.add(new Integer(-1));
                continue;
            }
            convertlist.add(new Integer(counter));
            counter++;
            fs.write("                ");
            fs.write(Float.toString((((float)point.x)/20)-5));
            fs.write(" ");
            fs.write(Float.toString((((float)point.y)/-20)-6));
            fs.write(" ");
            fs.write(Float.toString((((float)point.t))*time));
            fs.write("\n");
            if(point.t==1) counterPoly1++;
            
        }
        fs.write("               ]\n");
        fs.write("         }\n");
        fs.write("      coordIndex [\n");
//        vertices=new int[counterPoly1][3];
//        preindex=-1;
//        {
//            int j=0;
//            while(((PointWNL)pointList.elementAt(j)).t!=1)
//                j++;
//            vertices[0][0]=j;
//            vertices[0][1]=((PointWNL)pointList.elementAt(j)).x;
//            vertices[0][2]=((PointWNL)pointList.elementAt(j)).y;
//            currentIndex=j;
//        }
//        {
//            int i=1;
//            while(i<counterPoly1)
//            {
//                point = (PointWNL)pointList.elementAt(currentIndex);
//                if (point == null)
//                {
//
//                    continue;
//                }
//                for(int j=0;j<point.neighbours.size();j++)
//                {
//                    PointWNL point2 = (PointWNL)pointList.elementAt(((Integer)point.neighbours.elementAt(j)).intValue());
//                    if(point2.t==1&&((Integer)point.neighbours.elementAt(j)).intValue()!=preindex)
//                    {
//                        vertices[i][0]=((Integer)point.neighbours.elementAt(j)).intValue();
//                        vertices[i][1]=point2.x;
//                        vertices[i][2]=point2.y;
//                        preindex=currentIndex;
//                        currentIndex=((Integer)point.neighbours.elementAt(j)).intValue();
//                        i++;
//                        break;
//                    }
//                }
//            }
//        }
//
//        convexer=new Convexer(vertices);
        convexer=new Convexer(this.getOrderedSnapshot(1),this.pointList);
        convexer.writePolygone(fs,this.pointList);
        fs.write("      ]\n");
        fs.write("      solid FALSE\n");
        fs.write("   }\n");
        fs.write("}\n");
        fs.write("PointLight {\n");
        fs.write("   attenuation 3.999999e-2 0 0\n");
        fs.write("   location -100 -100 1\n");
        fs.write("}\n");
        fs.write("Background {\n");
        fs.write("   skyColor [\n");
        fs.write("   0.0 0.1 0.8,\n");
        fs.write("   0.0 0.5 1.0,\n");
        fs.write("   1.0 1.0 1.0]\n");
        fs.write("   skyAngle [0, 1.571]\n");
        fs.write("}\nViewpoint   {  orientation 0.25 -0.4 -0.8 2  position 0 0  1   }");
        fs.flush();
        filestream.close();
    }
    
    
}
