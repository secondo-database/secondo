package viewer.hoese.algebras.fixedmregion;

import java.awt.Color;
import java.util.LinkedList;
import java.util.List;

/**
 * A CFace is a Face with curved segments.
 * Several CFace objects make up a CRegion.
 * 
 * @author Florian Heinz <fh@sysv.de>
 */
public class CFace {
    /** List of holes in this CFace */
    private List<CFace> holes = new LinkedList();
    /** List of RCurve segments for this cycle */
    private List<RCurve> rs = new LinkedList();
    
    /**
     * Add the next curve segment to this CFace cycle.
     * 
     * @param rcurve RCurve to add
     */
    public void addRCurve (RCurve rcurve) {
        rs.add(rcurve);
    }
    
    /**
     * Get all RCurves from this cycle.
     * 
     * @return List of RCurves
     */
    public List<RCurve> getRCurves() {
        return rs;
    }
    
    /**
     * Add a new hole to this CFace.
     * 
     * @param hole hole to add
     */
    public void addHole(CFace hole) {
        holes.add(hole);
    }

    /**
     * Get all holes from this CFace
     * 
     * @return List of holes
     */
    public List<CFace> getHoles() {
        return holes;
    }
    
    /**
     * Construct a CFace with a single cycle from its Nested List representation
     * 
     * @param nl Nested List representation
     * @return Constructed CFace
     */
    private static CFace deserializeSingleFace(NL nl) {
        CFace cface = new CFace();
        for (int i = 0; i < nl.size(); i++) {
            cface.addRCurve(RCurve.fromNL(nl.get(i)));
        }
        
        return cface;
    }
        
    /**
     * Construct a CFace with optional holes from its Nested List
     * representation.
     * 
     * @param nl Nested List representation
     * @return CFace (with holes)
     */
    public static CFace deserialize(NL nl) {
        CFace f2 = deserializeSingleFace(nl.get(0));
        
        for (int i = 1; i < nl.size(); i++) {
            CFace hole = deserializeSingleFace(nl.get(i));
            f2.addHole(hole);
        }
        
        return f2;
    }
    
    /**
     * Convert this CFace to its Nested List representation.
     * 
     * @return Nested List representation of this CFace
     */
    public NL toNL () {
        NL nl = new NL();
        NL n = nl.nest();
        for (RCurve r : rs)
            n.addNL(r.toNL());
        for (CFace f : holes) {
            nl.addNL(f.toNL().get(0));
        }
        
        return nl;
    }
}
