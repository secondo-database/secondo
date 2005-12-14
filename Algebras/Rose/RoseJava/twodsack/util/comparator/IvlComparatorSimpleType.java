/*
 * IvlComparatorSimpleType.java 2005-11-30
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.comparator;

import twodsack.setelement.datatype.*;
import twodsack.util.collectiontype.*;
import java.util.*;

/**
 *  This class implements a comparator for Intervals. It implements only one single method and is used with any kind of collections
 * that need comparators. The objects that are accepted by the {@link #compare(Object,Object)} method are of type 
 * {@link twodsack.util.collectiontype.Interval}. If not, an exception is thrown.<p>
 * The comparator's constructor needs a flag when construced: The <i>meet</i> flag indicates, how intervals which have the same
 * x coordinate are sorted. If <tt>meet==true</tt>, for two such intervals which have the same x coordinate and one is a left and the other
 * is a right interval, the left interval is defined to be smaller than the other one. If <tt>meet==true</tt>, the right interval is defined
 * to be smaller than the left interval.
 */
public class IvlComparatorSimpleType implements Comparator {

    /*
     * fields
     */
    private boolean meet;

    /*
     * constructors
     */
    /**
     * The standard constructor.
     */
    public IvlComparatorSimpleType(){}


    /**
     * Constructs a new IvlComparatorSimpleType instance with a value for the flag <i>meet</i>.
     *
     * @param meet flag which tells how intervals with the same x coordinate shall be sorted
     */
    public IvlComparatorSimpleType(boolean meet) {
	this.meet = meet;
    }

    /*
     * methods
     */
    /**
     * Compares two objects and returns one of {0, 1, -1}.
     * Compares the intervals. For the compare operation, some rules are defined:
     * <p><ol>
     * <li>An interval with a smaller x coordinate is always smaller than the other.
     * <li>"blue" is considered to be smaller than "green".
     * <li>If <tt>meet==true</tt> intervals are always sorted that way, that the most possible 'overlaps' of intervals occur; if <tt>meet==false</tt>
     * intervals are sorted such, that a minimal number of 'overlaps' occurs.
     * </ol><p>
     * Returns 0, if both intervals are equal.<p>
     * Returns -1, if <tt>ino1.o</tt> is smaller.<p>
     * Returns 1 otherwise
     *
     * @param ino1 the first object
     * @param ino2 the second object
     * @return one of  {0, 1, -1} as int
     * @throws WrongTypeException if <tt>ino1.o</tt> or <tt>ino2.o</tt> is not of type {@link twodsack.util.collectiontype.Interval}
     */
    public int compare (Object ino1, Object ino2) throws WrongTypeException {
	//For three segments s1=(1,1,3,1),s2=(3,0,3,2),s3=(3,1,5,1), the order is
	//for meet=false (s1.r/l means right/left interval border):
	//s1.l,s1.r,s2.l,s2.r,s3.l,s3.r
	//for meet=true:
	//s1.l,s2.l,s3.l,s1.r,s2.r,s3.r

	if(ino1 instanceof Interval &&
	   ino2 instanceof Interval) {
	    Interval i1 = (Interval)ino1;
	    Interval i2 = (Interval)ino2;

	    if (meet) {
		if (!i1.x.equal(i2.x)) return i1.comp(i2);
		else {
		    //intervals have same number
		    if (i1.number == i2.number)
			if (i1.mark == "blueleft" || i1.mark == "greenleft") return -1;
			else return 1;
		    else {
			//intervals have different numbers; compare for buddy flag
			if (!i1.buddyOnSameX && i2.buddyOnSameX)
			    if (i2.mark == "blueleft" || i2.mark == "greenleft") return 1;
			    else return -1;
			
			else if ((i1.buddyOnSameX && i2.buddyOnSameX) ||
				 (!i1.buddyOnSameX && !i2.buddyOnSameX)) {
			    if (i1.mark == i2.mark) return i1.comp(i2);
			    else if ((i1.mark == "blueleft" || i1.mark == "greenleft") &&
				     (i2.mark == "blueleft" || i2.mark == "greenleft"))
				if (i1.mark == "blueleft") return -1;
				else return 1;
			    else if ((i1.mark == "blueright" || i1.mark == "greenright") &&
				     (i2.mark == "blueright" || i2.mark == "greenright"))
				if (i1.mark == "blueright") return -1;
				else return 1;
			    else if ((i1.mark == "blueleft" || i1.mark == "greenleft") &&
				     (i2.mark == "blueright" || i2.mark == "greenright"))
				return -1;
			    else return 1;
			}//else if

			else if (i1.buddyOnSameX && !i2.buddyOnSameX)
			    if (i1.mark == "blueleft" || i1.mark == "greenleft") return -1;
			    else return 1;//-1;
		    }//else
		}//else
		System.out.println("IvlComparator: Uncaught case."); i1.print(); i2.print(); System.exit(0);
	    }//if meet

	    else {
		if (!i1.x.equal(i2.x)) return i1.comp(i2);
		else {
		    //intervals have same number
		    if (i1.number == i2.number) 
			if (i1.mark == "blueleft" || i1.mark == "greenleft") return -1;
			else return 1;
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
			    else if (i1.mark == "blueleft" || i1.mark == "greenleft") return 1;
			    else return -1;
			
			else if (i1.buddyOnSameX && !i2.buddyOnSameX) 
			    if (i2.mark == "blueleft" || i2.mark == "greenleft") return -1;
			    else return 1;
		    }//else
		}//else
		System.out.println("IvlComparator: Uncaught case."); i1.print(); i2.print(); System.exit(0);
	    }//else if meet
	}//if
	else
	    throw new WrongTypeException("Exception in IvlComparatorSimpleType: Expected type Interval - found: "+ino1.getClass()+"/"+ino2.getClass());
	return 0;
    }//end method compare

}//end class IvlComparatorSimpleType
