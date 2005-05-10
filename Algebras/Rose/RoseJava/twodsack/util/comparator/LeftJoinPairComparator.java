package twodsack.util.comparator;

import twodsack.setelement.*;
import twodsack.setelement.datatype.*;
import twodsack.util.collectiontype.*;

import java.util.*;

public class LeftJoinPairComparator implements Comparator {
    
    //methods
    public int compare (Object ino1, Object ino2) {
	//overwrites the original method

	if ((((MultiSetEntry)ino1).value instanceof LeftJoinPair) &&
	    (((MultiSetEntry)ino2).value instanceof LeftJoinPair)) {
	    LeftJoinPair ljp1 = (LeftJoinPair)((MultiSetEntry)ino1).value;
	    LeftJoinPair ljp2 = (LeftJoinPair)((MultiSetEntry)ino2).value;
	    return ljp1.element.compare(ljp2.element);
	} else {
	    throw new WrongTypeException("Exception in LeftJoinPairComparator: Expected LeftJoinPair - found: "+((MultiSetEntry)ino1).value.getClass()+"/"+((MultiSetEntry)ino2).value.getClass());
	}//else
    }//end method compare
}//end class LeftJoinPairComparator