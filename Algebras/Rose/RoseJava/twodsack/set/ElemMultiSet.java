/*
 * ElemMultiSet.java 2004-11-05
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.set;

import twodsack.setelement.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.collection.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;
import java.util.Iterator;
import java.util.TreeSet;
import java.util.Comparator;

/**
 * This class provides an extension of {@link MultiSet} that is only for type {@link Element}.
 * This means, that only elements of type <code>Element</code> can be stored in this structure.
 * In exchange, all operations defined for this type work as intended.
 */
public class ElemMultiSet extends MultiSet {

    /* 
     * constructors
     */

    /**
     * Standard constructor which uses a comparator to guarantee a total order on all elements.
     *
     * @param co the comparator as {@link Comparator}
     */
    public ElemMultiSet (Comparator co) {
	super(co);
    }

    /*
     * methods
     */
    
    /**
     * Returns a <i>real</i> copy of <code>this</code>, not only a clone.
     * That means, that every single element of the structure is copied.
     * <p>See also method <code>clone</code>
     *
     * @return the copy
     */
    public ElemMultiSet copy() {
	ElemMultiSet retSet = new ElemMultiSet(new ElemComparator());
	Iterator it = this.iterator();
	MultiSetEntry mse;
	Element actEl;
	while (it.hasNext()) {
	    mse = (MultiSetEntry)it.next();
	    Element elemCopy = ((Element)mse.value).copy();
	    retSet.add(elemCopy,mse.number);
	}//while
	return retSet;
    }//end method copy
    
    
    /**
     * Returns a clone of the structure.
     * <p>See also method <code>copy</code>.
     *
     * @return the clone as <code>Object</code>
     */
    public Object clone() {
	//System.out.println("entering EMS.clone()...");
	ElemMultiSet retSet = new ElemMultiSet(new ElemComparator());
	retSet.setTreeSet((TreeSet)super.treeSet().clone());
	return retSet;
    }//end method clone

    
    /**
     * Prints the elements of the structure to standard output.
     *
     */
    public void print() {
	if (this.isEmpty()) System.out.println("ElemMultiSet is empty.\n");
	else {
	    Iterator it = this.iterator();
	    while (it.hasNext()) {
		MultiSetEntry actEntry = (MultiSetEntry)it.next();
		for (int i = 0; i < actEntry.number; i++)
		    ((Element)actEntry.value).print();
	    }//while
	}//else
    }//end method print
	

    /**
     * Returns the bounding box of the complete set of elements.
     *
     * @param ems the set of elements
     * @return the minimum bounding box
     */
    public Rect rect () {
	Rect r = new Rect();
	if (this.isEmpty()) return r;
	Iterator it = this.iterator();
	Element actEl = (Element)this.first();
	Rect actRect = actEl.rect();
	r.ulx = actRect.ulx;
	r.uly = actRect.uly;
	r.llx = actRect.llx;
	r.lly = actRect.lly;
	r.urx = actRect.urx;
	r.ury = actRect.ury;
	r.lrx = actRect.lrx;
	r.lry = actRect.lry;
	if (it.hasNext()) it.next();
	while (it.hasNext()) {
	    actEl = (Element)((MultiSetEntry)it.next()).value;
	    actRect = actEl.rect();
	    if (actRect.ulx.less(r.ulx)) r.ulx = actRect.ulx;
	    if (actRect.uly.greater(r.uly)) r.uly = actRect.uly;
	    if (actRect.llx.less(r.llx)) r.llx = actRect.llx;
	    if (actRect.lly.less(r.lly)) r.lly = actRect.lly;
	    if (actRect.urx.greater(r.urx)) r.urx = actRect.urx;
	    if (actRect.ury.greater(r.ury)) r.ury = actRect.ury;
	    if (actRect.lrx.greater(r.lrx)) r.lrx = actRect.lrx;
	    if (actRect.lry.less(r.lry)) r.lry = actRect.lry;
	}//while it
	return r;
    }//end method rect

}//end class ElemMultiSet
