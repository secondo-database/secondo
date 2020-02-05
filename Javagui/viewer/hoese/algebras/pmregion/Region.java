package viewer.hoese.algebras.pmregion;

import java.util.ArrayList;
import java.util.List;

/**
 * Class Region represents a set of Faces.
 *
 * @author Florian Heinz <fh@sysv.de>
 */
public class Region {
    /** List of faces in this region */
    List<Face> fcs = new ArrayList();
    
    /**
     * Add a new face to this region.
     * 
     * @param face new face to add 
     */
    public void addFace (Face face) {
        fcs.add(face);
    }
    
    /**
     * Get all faces of this region.
     * 
     * @return List of faces
     */
    public List<Face> getFaces () {
        return fcs;
    }

    public void fixTopology () {
	    List<Face> remove = new ArrayList();
	    for (Face f1 : fcs) {
		    for (Face f2 : fcs) {
			    if (f1 == f2)
				    continue;
			    if (f1.contains(f2)) {
				    f1.addHole(f2);
				    remove.add(f2);
			    }
		    }
	    }
	    for (Face f : remove) {
		    fcs.remove(f);
	    }
    }
    
    /**
     * Create a Region from nested list representation.
     * 
     * @param nl nested list representation of a region
     * @return Region object
     */
    public Region deserialize(NL nl) {
        Region r = new Region();
        for (int i = 0; i < nl.size(); i++) {
            r.addFace(Face.fromNL(nl.get(i)));
        }
        
        return r;
    }

    /**
     * Return nested list representation of this Region
     * 
     * @return nested list representation
     */
    public NL serialize() {
        NL nl = new NL();
        
        for (Face f : fcs) {
            nl.addNL(f.toNL());
        }
        
        return nl;
    }
}
