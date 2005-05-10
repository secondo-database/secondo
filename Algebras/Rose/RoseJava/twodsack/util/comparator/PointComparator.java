package twodsack.util.comparator;

import twodsack.setelement.datatype.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.collectiontype.*;
import java.util.*;
import java.io.Serializable;

public class PointComparator implements Comparator,Serializable {

    //methods
    public int compare(Object ino1, Object ino2) {
	//overwrites the original method

	//Object o1 = ((MultiSetEntry)ino1).value;
	//Object o2 = ((MultiSetEntry)ino2).value;

	if ((((MultiSetEntry)ino1).value instanceof Point) &&
	    (((MultiSetEntry)ino2).value instanceof Point))
	    return ((Point)((MultiSetEntry)ino1).value).compare((Point)((MultiSetEntry)ino2).value);
	else 
	    throw new WrongTypeException("in PointComparator: Expected Point - found: "+((MultiSetEntry)ino1).value.getClass()+"/"+((MultiSetEntry)ino2).value.getClass());
    
    }//end method compare

}//end class PointComparator