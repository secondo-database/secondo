/*
 * PairSet.java 2005-11-09
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.collection;

import twodsack.set.*;
import twodsack.util.comparator.*;

/**
 * Stored in instances of this class are five sets. All those sets are return values of the <tt>overlappingPairs</tt> method which is 
 * implemented in the class {@link twodsack.operations.setoperation.SupportOps}. Those three sets are: 1) a set of triangles 2) another
 * set of triangles and 3) a <tt>PairMultiSet</tt>, where the pairs are of type <tt>TriMultiSet x TriMultiSet</tt>. 4) and 5) are sets
 * of triangles again.
 */
public class PairSet {
    /*
     * members
     */
    final static TriangleComparator TRIANGLE_COMPARATOR = new TriangleComparator();
    final static ElemPairComparator ELEMPAIR_COMPARATOR = new ElemPairComparator();
    /**
     * The first set of triangles.
     */
    public TriMultiSet firstSet;

    /**
     * The second set of triangles.
     */
    public TriMultiSet secondSet;

    /**
     * The third set of triangles.
     */
    public TriMultiSet thirdSet;

    /**
     * The fourth set of triangles.
     */
    public TriMultiSet fourthSet;


    /**
     * The set of pairs <tt>TriMultiSet x TriMultiSet</tt>.
     */
    public PairMultiSet pairSet;
    

    /*
     * constructors
     */
    /**
     * The 'empty' constructor.
     *
     * Initializes all data structures.
     */
    public PairSet() {
	firstSet = new TriMultiSet(TRIANGLE_COMPARATOR);
	secondSet = new TriMultiSet(TRIANGLE_COMPARATOR);
	thirdSet = new TriMultiSet(TRIANGLE_COMPARATOR);
	fourthSet = new TriMultiSet(TRIANGLE_COMPARATOR);
	pairSet = new PairMultiSet(ELEMPAIR_COMPARATOR);
    }

    /*
     * operations
     */
}//end class PairSet
