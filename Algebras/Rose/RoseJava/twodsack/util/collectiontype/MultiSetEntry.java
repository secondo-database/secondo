package twodsack.util.collectiontype;

import twodsack.setelement.datatype.*;
import twodsack.util.*;
import java.io.*;

public class MultiSetEntry implements Comparable,Serializable {

    //members
    public Object value; //object to store
    public int number; //number of same objects

    //constructors
    public MultiSetEntry () {
	this.value = null;
	this.number = -1;
    }

    public MultiSetEntry (Object o, int number) {
	this.value = o;
	this.number = number;
    }

    //methods
    public int compareTo(Object inMSE) {
	if (!(inMSE instanceof MultiSetEntry)) throw new WrongTypeException("Expected "+this.getClass()+", found "+inMSE.getClass());
	MultiSetEntry mse = (MultiSetEntry)inMSE;
	if (mse.value instanceof ComparableMSE) {
	    return ((ComparableMSE)this.value).compare((ComparableMSE)(mse.value));
	}
	else throw new WrongTypeException("Expected class ComparableMSE, found "+mse.value.getClass());
    }//end method compareTo

}//end class MultiSetEntry