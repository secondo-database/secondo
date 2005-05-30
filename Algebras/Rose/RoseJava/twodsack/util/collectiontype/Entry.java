/*
 * Entry.java 2005-05-11
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.collectiontype;


/**
 * Instances of Entry are used to store elements for a {@link twodsack.util.collection.ProLinkedList}. It has a field to store an object and two
 * additional fields to store pointers to other Entry instances.
 */
public class Entry {

    /*
     * fields
     */
    /**
     * The <i>object</i> is stored in <tt>value</tt>.
     */
    public Object value;


    /**
     * A pointer to the previous entry.
     */
    public Entry prev;

    /**
     * A pointer to the next entry.
     */
    public Entry next;

    /*
     * constructors
     */
    /**
     * Constructs a new Entry and sets its value field to <i>val</i>.
     *
     * @param val this is stored in the new instance
     */
    public Entry (Object val) {
	this.value = val;
	this.prev = null;
	this.next = null;
    }
   
    /*
     * methods
     */
    /**
     * Prints the data of <i>this</i> to standard output.
     */
    public void print() {
	System.out.println("\nEntry: value: "+this.value+", prev: "+this.prev+", next: "+this.next);
    }//end method print


    /**
     * Returns a copy of <i>this</i>.
     * This is a shallow copy of <i>this</i>. The stored object is not copied.
     *
     * @return the copy
     */
    public Entry copy () {
	Entry copy = null;
	copy = new Entry(this.value);
	copy.prev = this.prev;
	copy.next = this.next;
	return copy;
    }//end method copy

}//end class Entry
