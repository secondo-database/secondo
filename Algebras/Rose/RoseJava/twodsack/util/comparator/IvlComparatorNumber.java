package twodsack.util.comparator;

import twodsack.setelement.datatype.*;
import twodsack.util.collectiontype.*;
import java.util.*;

public class IvlComparatorNumber implements Comparator {
    
    //methods
    public int compare(Object ino1, Object ino2) {
	if ((ino1 instanceof Interval) &&
	    ino2 instanceof Interval)
	    if (((Interval)ino1).number < ((Interval)ino2).number) return -1;
	    else if (((Interval)ino1).number > ((Interval)ino2).number) return 1;
	    else return 0;
	else
	    throw new WrongTypeException("in IvlNumberComparator: Expected Interval - found: "+ino1.getClass()+"/"+ino2.getClass());
    }//end method compare
}//end class IvlNumberComparator
		