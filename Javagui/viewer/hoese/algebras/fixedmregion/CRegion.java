package viewer.hoese.algebras.fixedmregion;

import java.util.LinkedList;
import java.util.List;

/**
 * Class CRegion respresents a set of CFaces.
 * 
 * @author Florian Heinz <fh@sysv.de>
 */
public class CRegion {
    /** The list of CFaces in this CRegion */
    private List<CFace> faces = new LinkedList();
    
    /**
     * Construct an empty CRegion
     */
    public CRegion () {
    }
    
    /**
     * Construct a CRegion from a single CFace
     * 
     * @param face CFace in this CRegion 
     */
    public CRegion (CFace face) {
        faces.add(face);
    }
    
    /**
     * Add a new CFace to this CRegion.
     * 
     * @param face CFace to add 
     */
    public void addFace (CFace face) {
        faces.add(face);
    }
    
    /**
     * Return all CFaces in this CRegion
     * 
     * @return List of CFaces
     */
    public List<CFace> getFaces() {
        return faces;
    }

    /**
     * Create a CRegion from its nested list representation.
     * 
     * @param nl nested list representation of this CRegion
     * @return CRegion object
     */
    public static CRegion deserialize(NL nl) {
        CRegion  r2 = new CRegion();
        
        for (int i = 0; i < nl.size(); i++) {
            r2.addFace(CFace.deserialize(nl.get(i)));
        }
        
        return r2;
    }

    /**
     * Convert this CRegion to a nested list representation.
     * 
     * @return nested list representation of this CRegion
     */
    public NL serialize() {
        NL nl = new NL(), ret = nl;
        for (CFace f : faces) {
            nl.addNL(f.toNL());
        }
        
        return ret;
    }
}
