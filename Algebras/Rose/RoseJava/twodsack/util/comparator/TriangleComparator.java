package twodsack.util.comparator;

import twodsack.setelement.datatype.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.collectiontype.*;

import java.util.*;
import java.io.*;

public class TriangleComparator implements Comparator,Serializable {
    
    //methods
    public int compare(Object ino1, Object ino2) {
	//overwrites the original method

	//Object o1 = ((MultiSetEntry)ino1).value;
	//Object o2 = ((MultiSetEntry)ino2).value;
	
	if ((((MultiSetEntry)ino1).value instanceof Triangle) &&
	    (((MultiSetEntry)ino2).value instanceof Triangle))
	    return ((Triangle)((MultiSetEntry)ino1).value).compare((Triangle)((MultiSetEntry)ino2).value);
	else
	    throw new WrongTypeException("in TriangleComparator: Expected Triangle - found :"+((MultiSetEntry)ino1).value.getClass()+"/"+((MultiSetEntry)ino2).value.getClass());
    }//end method compare

}//end class TriangleComparator