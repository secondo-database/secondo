package twodsack.util.comparator;

import twodsack.setelement.datatype.*;
import twodsack.util.collectiontype.*;
import java.util.*;

public class PSPointComparator implements Comparator {
    public int compare(Object ino1, Object ino2) {
	if ((((MultiSetEntry)ino1).value instanceof PSPoint) &&
	    (((MultiSetEntry)ino2).value instanceof PSPoint))
	    return ((PSPoint)((MultiSetEntry)ino1).value).compare((PSPoint)((MultiSetEntry)ino2).value);
	else 
	    throw new WrongTypeException("in PSPointComparator: Expected PSPoint - found: "+((MultiSetEntry)ino1).value.getClass()+"/"+((MultiSetEntry)ino2).value.getClass());
    }
}