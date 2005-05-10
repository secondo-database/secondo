/*
 * LeftJoinPair.java 2005-05-03
 *
 * Dirk Ansorge FernUniversitaet Hagen
 *
 */

package twodsack.setelement;

import twodsack.set.*;
import twodsack.setelement.datatype.*;
import twodsack.util.*;

/**
 * The LeftJoinPair implements a special pair of objects. Every instance of this class is a pair <code>(Element x ElemMultiSet)</code>.
 * Generally, the first partner is somehow related to the objects in the second partner. Instances are most commonly generated
 * by a set operation in {@link SetOps} like leftOuterJoin(). In there, a predicate expresses the relation between the Element
 * and the ElemMultiSet. The predicate itself is not stored inside of a LeftJoinPair.<p>
 * Usually, instances of this class are stored in a LeftJoinPairMultiSet.
 */
public class LeftJoinPair implements ComparableMSE{
    /*
     * fields
     */
    public Element element;
    public ElemMultiSet elemSet;

    /*
     * constructors
     */
    /**
     * The 'empty' constructor.
     */
    public LeftJoinPair() {};


    /**
     * Constructs a new instance of LeftJoinPair and directly assigns the element and multi set.
     *
     * @param el is assigned to this.element
     * @param ell is assigned to this.elemSet
     */
    public LeftJoinPair(Element el, ElemMultiSet ell) {
	element = el;
	elemSet = ell;
    }

    /*
     * methods
     */
    /**
     * Returns a <i>deep</i> copy of <i>this</i>.
     * This means, that real copies of this.element and this.elemSet are constructed.
     *
     * @return the copy
     */
    public LeftJoinPair copy() {
	LeftJoinPair copy = new LeftJoinPair();
	copy.element = this.element.copy();
	copy.elemSet = this.elemSet.copy();

	return copy;
    }//end method copy


    /**
     * Returns one of {0, 1, -1} depending on the result of the compare method.
     * The implementation of this method takes us of the compare method for this.element. Hence, two instances of 
     * type LeftJoinPair are sorted only by their first element and not by the multi set.
     *
     * @param inO must be another object of type LeftJoinPair
     * @return {0, 1, -1} as int
     * @throws WrongTypeException if inO.element has not the same type as this.inO.element or if inO is not of type
     * LeftJoinPair
     */
    public int compare(ComparableMSE inO) throws WrongTypeException {
	//comment missing
	if (!(inO instanceof LeftJoinPair)) {
	    throw new WrongTypeException("Error in LeftJoinPair.compareTo: Expected type LeftJoinPair, but found "+inO.getClass()); }
	
	Element firstE = ((LeftJoinPair)inO).element;
	return this.element.compare(firstE);
    }//end method compareTo
	
}//end class LeftJoinPair
