/*
 * HashEntry.java 2006-01-06
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.collectiontype;

/**
 * This class implements a wrapper for elements that are stored into hashtables. Each entry has two elements, a key and a value.
 * A HashEntry instance is stored in a hashtable using its key.
 */
public class HashEntry {

    /*
     * members
     */
    /**
     * The key of the HashEntry instance.
     */
    public Object key;


    /**
     * The value of the HashEntry instance.
     */
    public Object value;


    /*
     * constructors
     */
    /**
     * Don't use this constructor!
     */
    private HashEntry() {}


    /**
     * Constructs a new HashEntry instance from a key and a value.
     *
     * @param key the key
     * @param value the value
     */
    public HashEntry(Object key, Object value) {
	this.key = key;
	this.value = value;
    }

    /*
     * methods
     */
    /**
     * Compares the two keys using the <tt>equals</tt> function.
     * The <tt>equals</tt> function has to be implemented for every key, that is used in HashEntry instances.
     * 
     * @return true, if the keys are equal.
     */
    public boolean equalKey(Object key) {
	if (this.key.equals(key))
	    return true;
	return false;
    }//end method equalKey


    /**
     * Compares two HashEntry instances using the <tt>equals</tt> function.
     * The <tt>equals</tt> function has to be implemented for every key and value that are used in HashEntry instances.
     *
     * @param key the key
     * @param value the value
     * @return true, if the objects are equal
     */
    public boolean equal(Object key, Object value) {
	if (this.key.equals(key) && this.value.equals(value))
	    return true;
	return false;
    }//end method equal

}//end class HashEntry
