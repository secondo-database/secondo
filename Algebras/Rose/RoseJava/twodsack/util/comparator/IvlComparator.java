package twodsack.util.comparator;

import twodsack.setelement.datatype.*;
import twodsack.util.collectiontype.*;


import java.util.*;

public class IvlComparator implements Comparator {
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
    private IvlComparator() {}


    public IvlComparator(boolean meet) {
	this.meet = meet;
    }


    /*
     * methods
     */
    public int compare(Object ino1, Object ino2) {
	//overwrites the original method
	//For three segments s1=(1,1,3,1),s2=(3,0,3,2),s3=(3,1,5,1), the order is
	//for meet=false (s1.r/l means right/left interval border):
	//s1.l,s1.r,s2.l,s2.r,s3.l,s3.r
	//for meet=true:
	//s1.l,s2.l,s3.l,s1.r,s2.r,s3.r

	if((((MultiSetEntry)ino1).value instanceof Interval) &&
	   (((MultiSetEntry)ino2).value instanceof Interval)) {
	    Interval i1 = (Interval)(((MultiSetEntry)ino1).value);
	    Interval i2 = (Interval)(((MultiSetEntry)ino2).value);

	    //System.out.println("\ncomparing:"); i1.print(); i2.print();

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
				

			    /*
			    if ((i1.mark == "blueleft" && i2.mark == "greenleft") ||
				(i1.mark == "blueleft" && i2.mark == "greenright") ||
				(i1.mark == "blueright" && i2.mark == "greenright")) return -1;
			    else if (i1.mark == i2.mark) { return i1.comp(i2); }
			    //else return 1;
			    else if ((i1.mark == "blueleft" && i2.mark == "blueright") ||
				     (i1.mark == "greenleft" && i2.mark == "greenright")) return -1;
			    else return 1;
			    */
			}//else if

			else if (i1.buddyOnSameX && !i2.buddyOnSameX)
			    if (i1.mark == "blueleft" || i1.mark == "greenleft") return -1;//1;
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
			    else if (i1.mark == "blueleft" || i1.mark == "greenleft") return 1;//-1;
			    else return -1;//1;
			
			else if (i1.buddyOnSameX && !i2.buddyOnSameX) 
			    if (i2.mark == "blueleft" || i2.mark == "greenleft") return -1;
			    else return 1;
		    }//else
		}//else
		System.out.println("IvlComparator: Uncaught case."); i1.print(); i2.print(); System.exit(0);
	    }//else if meet
	}//if
	else
	    throw new WrongTypeException("Exception in IvlComparator: Expected type Interval - found: "+((MultiSetEntry)ino1).value.getClass()+"/"+((MultiSetEntry)ino2).value.getClass());
	return 0;
    }//end method compare
}//end class IvlComparator