package twodsack.util.comparator;

import twodsack.setelement.datatype.*;
import twodsack.util.collectiontype.*;
import java.util.*;

public class IvlComparatorSimple implements Comparator {
    //This comparator only uses the interval borders (left/right) and no other data.

    public int compare (Object ino1, Object ino2) {
	//overwrites the original method
	if ((((MultiSetEntry)ino1).value instanceof Interval) &&
	    (((MultiSetEntry)ino2).value instanceof Interval)) {
	    Interval i1 = (Interval)(((MultiSetEntry)ino1).value);
	    Interval i2 = (Interval)(((MultiSetEntry)ino2).value);

	    return i1.comp(i2);
	}//if
	else throw new WrongTypeException("Exception in IvlComparatorSimple: Expected type Interval - found: "+((MultiSetEntry)ino1).value.getClass()+"/"+((MultiSetEntry)ino2).value.getClass());
	//return 0;
    }//end method compare

}//end class IvlComparatorSimple