/*
 * IvlMergeComparator.java 2005-05-12
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.comparator;

import twodsack.setelement.datatype.*;
import twodsack.util.collectiontype.*;
import java.util.*;

/**
 * Implemented in this class is a comparator used in the merge() method for collections of type Interval. The intervals must be sorted in a 
 * certain way for that operation. This is done by using the compare() method of this class.<p>
 * The comparator's constructor needs a flag when construced: The <i>meet</i> flag indicates, how intervals which have the same
 * x coordinate are sorted. If meet==true, for two such intervals which have the same x coordinate and one is a left and the other
 * is a right interval, the left interval is defined to be smaller than the other one. If meet==true, the right interval is defined
 * to be smaller than the left interval.
 */
public class IvlMergeComparator implements Comparator {
    /*
     * fields
     */
    private boolean meet;
    
    /*
     * constructors
     */
    /**
     * This constructor must not be used.
     */
    private IvlMergeComparator() {}


    /**
     * Constructs a new comparator using the flag <i>meet</i>.
     *
     * @param meet the flag to indicate how intervals with the same x coordinate shall be sorted; currently, this is only implemented for meet=false
     */
    public IvlMergeComparator(boolean meet) {
	this.meet = meet;
    }


    /*
     * methods
     */
    /**
     * Compares two objects and returns one of {0, 1, -1}.
     * compares the objects stored in MultiSetEntry types. For the compare operation, some rules are defined:
     * <p><ol>
     * <li>An interval with a smaller x coordinate is alway smaller than the other interval.
     * <li>If both intervals have the same number, they are considered to be equal.
     * <li>If meet==true, intervals are alway sorted that way, that the most possible 'overlaps' of intervals occur; if meet==false
     * intervals are sorted such, that a minimal number of 'overlaps' occurs.
     * </ol>
     * Returns 0, if both intervals are equal.<p>
     * Returns -1, if ino1.o is smaller.<p>
     * Returns 1 otherwise.
     *
     * @param ino1 the first object
     * @param ino2 the second object
     * @return one of {0, 1, -1} as int
     * @throws WrongTypeException if ino1.o or ino2.o is not of type Interval
     */
    public int compare(Object ino1, Object ino2) {
	if((((MultiSetEntry)ino1).value instanceof Interval) &&
	   (((MultiSetEntry)ino2).value instanceof Interval)) {
	    Interval i1 = (Interval)(((MultiSetEntry)ino1).value);
	    Interval i2 = (Interval)(((MultiSetEntry)ino2).value);

	    if (meet) {
		System.out.println("IvlComparator: not yet implemented.");
		System.exit(0);
	    }//if meet

	    else {
		if (!i1.x.equal(i2.x)) return i1.comp(i2);
		else {
		    //intervals have same number
		    if (i1.number == i2.number) 
			return 0;
		    else {
			//intervals have different numbers; compare for buddy flag
			if (!i1.buddyOnSameX && i2.buddyOnSameX)
			    if (i1.mark == "blueleft" || i1.mark == "greenleft") return 1;
			    else return -1;
			
			else if ((!i1.buddyOnSameX && !i2.buddyOnSameX) ||
				 (i1.buddyOnSameX && i2.buddyOnSameX)) 
			    if (((i1.mark == "blueleft" || i1.mark == "greenleft") &&
				 (i2.mark == "blueleft" || i2.mark == "greenleft")) ||
				((i1.mark == "blueright" || i1.mark == "greenright") &&
				 (i2.mark == "blueright" || i2.mark == "greenright")))
				return i1.comp(i2);
			    else if (i1.mark == "blueleft" || i1.mark == "greenleft") return -1;
			    else return 1;
			
			else if (i1.buddyOnSameX && !i2.buddyOnSameX) 
			    if (i2.mark == "blueleft" || i2.mark == "greenleft") return -1;
			    else return 1;
		    }//else
		}//else

	    }//else if meet
	}//if
	else
	    throw new WrongTypeException("Exception in IvlMergeComparator: Expected type Interval - found: "+((MultiSetEntry)ino1).value.getClass()+"/"+((MultiSetEntry)ino2).value.getClass());
	return 0;
    }//end method compare

}//end class IvlMergeComparator
