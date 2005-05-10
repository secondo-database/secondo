package twodsack.util.comparator;

import twodsack.setelement.datatype.*;
import twodsack.util.collectiontype.*;
import java.util.*;

public class IvlMergeComparator implements Comparator {
    //This method is ment to be used in SetOps.finalSort.

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


    public IvlMergeComparator(boolean meet) {
	this.meet = meet;
    }


    /*
     * methods
     */
    public int compare(Object ino1, Object ino2) {
	//overwrites the original method
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
			//if (i1.mark == "blueleft" || i1.mark == "greenleft") return -1;
			//else return 1;
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