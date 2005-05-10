package twodsack.util.comparator;

import twodsack.setelement.datatype.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.collectiontype.*;
import java.util.*;
import java.io.*;

public class SegmentComparator implements Comparator,Serializable {

    //methods
    public int compare(Object ino1, Object ino2) {
	//overwrites the original method

	//Object o1 = ((MultiSetEntry)ino1).value;
	//Object o2 = ((MultiSetEntry)ino2).value;

	if ((((MultiSetEntry)ino1).value instanceof Segment) &&
	    (((MultiSetEntry)ino2).value instanceof Segment))
	    return ((Segment)((MultiSetEntry)ino1).value).compare((Segment)((MultiSetEntry)ino2).value);
	else
	    throw new WrongTypeException("in SegmentComparator: Expected Segment/Segment - found: "+
					 ((MultiSetEntry)ino1).value.getClass()+
					 "/"+
					 ((MultiSetEntry)ino2).value.getClass());
	
    }//end method compare
    
}//end class SegmentComparator