/*
 * SweepStElem.java 2005-05-06
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.setelement.datatype.compositetype;


import twodsack.setelement.datatype.basicdatatype.*;
import java.util.*;

/*
 * A SweepStElem is a data structure which is needed in the sweep line algorithm in Polygons.triangulate. In that sweep
 * line algorithm, the sweep line intersects the polygon's border in several points (at least two). These points are 
 * connected by a set of segments which are a part of the polygon's border. Since the polygon must not be convex, the
 * number of points intersection the sweep line doesn't have to be 2 but may be a multiple of it. Then, in the status
 * structure, more than one of these border parts have to be stored. These parts are stored inside of a SweepStElem together
 * with some more information about the border, like the rightmost point of the border, whether is is the top or bottom 
 * border part in the list of SweepStElem instance etc.
 */
class SweepStElem {
    /*
     * fields
     */
    boolean is_top;
    boolean is_bottom;
    LinkedList pointChain;
    boolean in; //true if attribute "in polygons" holds
    String col; //may be "red","blue", "both", ""
    Point rimoEl; //rightmost element
    
    /*
     * The 'empty' constructor.
     * Initializes all fields.
     */
    SweepStElem() {
	boolean is_top = false;
	boolean is_bottom = false;
	pointChain = new LinkedList();
	in = false;
	col = "";
	rimoEl = new Point();
    }
    
    /*
     * Prints the data of <i>this</i> to standard output.
     */
    void print() {
	//prints the elements attributes
	System.out.println("SweepStElem:");
	System.out.println("is_top:"+is_top+", is_bottom:"+is_bottom+", in:"+in+", col:"+col);
	System.out.println("pointChain:"+pointChain.size());
	for (int i = 0; i < pointChain.size(); i++) {
	    ((Point)pointChain.get(i)).print(); }
	System.out.println("rimoEl:");
	rimoEl.print();
    }//end method print

}//end class SweepStElem
