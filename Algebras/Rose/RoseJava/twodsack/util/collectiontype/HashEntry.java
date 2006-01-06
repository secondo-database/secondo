/*
 * HashEntry.java 2006-01-06
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.collectiontype;

public class HashEntry {

    /*
     * members
     */
    public Object key;
    public Object value;

    /*
     * constructors
     */
    private HashEntry() {}


    public HashEntry(Object key, Object value) {
	this.key = key;
	this.value = value;
    }

    /*
     * methods
     */
    public boolean equalKey(Object key) {
	if (this.key.equals(key))
	    return true;
	return false;
    }//end method equalKey


    public boolean equal(Object key, Object value) {
	if (this.key.equals(key) && this.value.equals(value))
	    return true;
	return false;
    }//end method equal

}//end class HashEntry
