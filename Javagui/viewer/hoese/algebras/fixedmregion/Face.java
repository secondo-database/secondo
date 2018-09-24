package viewer.hoese.algebras.fixedmregion;

import java.awt.Polygon;
import java.util.LinkedList;
import java.util.List;

/**
 * Class Face represents a single face of a region.
 * This is a polygon with optionally one or more holes.
 *
 * @author Florian Heinz <fh@sysv.de>
 */
public class Face {
    /** The last point set when constructing a face from points */
    private Point lastPoint;
    /** List of segments of this face */
    private List<Seg> segs = new LinkedList();
    /** List of holes in this face */
    private List<Face> holes = new LinkedList();
    
    /**
     * Add the next point when constructing a face.
     * 
     * @param point Next point
     */
    public void addPoint(Point point) {
        if (lastPoint != null) {
            segs.add(new Seg(lastPoint, point));
        }
        lastPoint = point;
    }

    /**
     * Close the cycle.
     * Creates the last segment from the last point to the start point.
     */
    public void close() {
        if (segs.size() < 2) {
            return;
        }
        Seg s = segs.get(0);
        segs.add(new Seg(lastPoint, s.s));
    }
    
    /**
     * Get the list of segments of this face.
     * 
     * @return List of segments
     */
    public List<Seg> getSegments() {
        return segs;
    }
    
    /**
     * Create a nested list of segments of the main cycle.
     * 
     * @return Nested List of segments
     */
    public NL segsToNL() {
        NL ret = new NL();
        for (Seg s : segs) {
            NL nl2 = ret.nest();
            nl2.addNr(s.s.x);
            nl2.addNr(s.s.y);
        }
        
        return ret;
    }
    
    /**
     * Create a Nested List representation of this face (cycle and holes)
     * 
     * @return Nested List representation
     */
    public NL toNL() {
        NL ret = new NL();
        ret.addNL(segsToNL());
        
        for (Face f : holes) {
            ret.addNL(f.segsToNL());
        }
        
        return ret;
    }
    
    /**
     * Create a single cycle from its Nested List representation
     * 
     * @param nl Nested List representation
     * @return A Face constructed from this cycle
     */
    public static Face singleFromNL(NL nl) {
        Face f = new Face();
        
        for (int i = 0; i < nl.size(); i++) {
            NL n = nl.get(i);
            Point p = new Point(n.get(0).getNr(), n.get(1).getNr());
            f.addPoint(p);
        }
        f.close();
        return f;
    }
    
    /**
     * Create a face from its Nested List representation (cycle and holes)
     * 
     * @param nl Nested List representation
     * @return A Face (cycle and optional holes)
     */
    public static Face fromNL(NL nl) {
        Face f = singleFromNL(nl.get(0));
        for (int i = 1; i < nl.size(); i++)
            f.addHole(singleFromNL(nl.get(i)));
        
        return f;
    }

    /**
     * Add a new hole to this face.
     * 
     * @param hole Hole to add
     */
    public void addHole (Face hole) {
        holes.add(hole);
    }
    /**
     * Get all holes of this face.
     * 
     * @return List of holes
     */
    public List<Face> getHoles () {
        return holes;
    }


    private Polygon p;
    public boolean contains (Point point) {
	    if (p == null) {
		    p = new Polygon();
		    for (Seg s : segs) {
			    p.addPoint((int)s.s.x, (int)s.s.y);
		    }
	    }
	    return p.contains(point.x, point.y);
    }

    public boolean contains (Face f) {
	    return contains (f.getFirstPoint());
    }

    public Point getFirstPoint() {
	    return segs.get(0).s;
    }



    /**
     * Calculate the projection of this face unter transformation trans at
     * instant currentTime.
     * 
     * @param trans The transformation
     * @param currentTime Instant to project
     * @return resulting face
     */
    public Face project (FMRegionTrans trans, double currentTime) {
        Face ret = new Face();
        for (Seg s : segs) {
            Seg n = new FMSeg(s, trans).project((long)currentTime);
            ret.segs.add(n);
        }
        for (Face hole : holes) {
            ret.addHole(hole.project(trans, currentTime));
        }
        
        return ret;
    }
}
