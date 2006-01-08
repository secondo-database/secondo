/*
 * ProHashtable.java 2006-01-06
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.util.collection;

import twodsack.util.collectiontype.*;
import twodsack.util.iterator.*;

import java.lang.*;

/**
 * This class implements hashtables. These hashtables are better than the original SUN Hashtables in two ways: It is possible to store
 * the same element multiple times. But, you cannot be sure to get exactly THAT copy back you put in before when asking for existance
 * of a copy. Furthermore, you can use the same key over and over again. When requesting the object for a certain key, you can choose
 * between two methods. <tt>getObjects</tt> returns a list of all objects with that key and <tt>getFirst</tt> returns the first
 * object which has the specified key.<p>
 * This implementtion NEVER rehashes and NEVER enlarges the hashtable. So think twice before specifying the initial capacity. Note: Every
 * bucket may hold any number of keys.<p>
 * This class needs two methods to be implemented for all objects and keys that should be used with this hashtable. The first method is
 * <tt>Object.equals(Object o)</tt>, which return <tt>true</tt> if both objects are equal. The other method is <tt>int hashCode()</tt>.
 * This method is already implemented by the <tt>Object</tt> class, but should be overwritten.
 */
public class ProHashtable {
    /*
     * members
     */
    /**
     * The capacity of the hashtable. This is the number of buckets.
     */
    private int capacity;


    /**
     * The array of buckets.
     */
    private ProLinkedList[] hashArray;


    /**
     * The number of entries is counted using this member.
     */
    private int numberOfEntries;


    /**
     * A static iterator is used in this class.
     * Using this ProListIterator, only ONE instance of iterators is used in this class. No more instances than THIS ONE.
     */
    static private ProLinkedList staticList = new ProLinkedList();
    static private ProListIterator lit1 = new ProListIterator(staticList,0);


    /*
     * constructors
     */
    /**
     * Don't use this constructor.
     */
    private ProHashtable() {}
    

    /**
     * Constructs a new ProHashtable using <tt>initialCapacity</tt>.
     * The initial capacity is the number of buckets, the hashtable has. In this implementation, this number is never changed.
     * If the load gets higher, the lists that are representing the buckets, get fuller. So choose the initial capacity wisely.
     *
     * @param initialCapacity the initial capacity
     */
    public ProHashtable(int initialCapacity) {
	if (initialCapacity < 0) 
	    throw new IllegalArgumentException("Initial capacity for ProHashtable must be > 0.");
	this.capacity = initialCapacity;
	this.hashArray = new ProLinkedList[initialCapacity];
	this.numberOfEntries = 0;
    }

    /*
     * methods
     */
    /**
     * Return <tt>true</tt>, if an objects with the specified key exists in the hashtable.
     *
     * @param key the key
     * @return <tt>true</tt>, if an object with that key exists
     */
    public boolean containsKey(Object key) {
	int hashValue = computeHashValue(key);
	if (hashArray[hashValue] == null || hashArray[hashValue].isEmpty())
	    return false;
	lit1.setList(hashArray[hashValue]);
	HashEntry actO;
	while (lit1.hasNext()) {
	    actO = (HashEntry)lit1.next();
	    if (actO.equalKey(key)) 
		return true;
	}//while lit1
	return false;
    }//end method containsKey


    /**
     * Returns all objects that have the same key.
     *
     * @param key the key
     *
     * @return all objects with the same key stored in a ProLinkedList
     */ 
    public ProLinkedList getObjects(Object key) {
	ProLinkedList pll = new ProLinkedList();
	int hashValue = computeHashValue(key);
	if (hashArray[hashValue] == null || hashArray[hashValue].isEmpty())
	    return pll;
	HashEntry actO;
	lit1.setList(hashArray[hashValue]);
	while (lit1.hasNext()) {
	    actO = (HashEntry)lit1.next();
	    if (actO.equalKey(key))
		pll.add(actO.value);
	}//while lit1
	return pll;
    }//end method getObjects

    /**
     * Returns only the first object found with the same key.
     * Returns <tt>null</tt> if no such object exists.
     *
     * @return an object with the same key stored in the hashtable
     */
    public Object getFirst(Object key) {
	Object ret = null;
	int hashValue = computeHashValue(key);
	if (hashArray[hashValue] == null || hashArray[hashValue].isEmpty())
	    return ret;
	HashEntry actO;
	lit1.setList(hashArray[hashValue]);
	while (lit1.hasNext()) {
	    actO = (HashEntry)lit1.next();
	    if (actO.equalKey(key))
		return actO.value;
	}//while lit1
	return ret;
    }//end method getFirst


    /**
     * Returns <tt>true</tt>, if the hashtable is empty.
     *
     * @return <tt>true</tt>, if the number of stored objects == 0
     */
    public boolean isEmpty() {
	return (numberOfEntries == 0);
    }//end method isEmpty


    /**
     * Stores the passed object in the hashtable using the key as address.
     *
     * @param key the key
     * @param value the object that has to be stored
     */
    public void put(Object key, Object value) {
	int hashValue = computeHashValue(key);
	if (hashArray[hashValue] == null)
	    hashArray[hashValue] = new ProLinkedList();
	hashArray[hashValue].add(new HashEntry(key,value));
	numberOfEntries++;
    }//end method put


    /**
     * Removes <i>all</i> entries which match the specified key and value.
     *
     * @param key the key of the value
     * @param value the object value
     */
    public void remove(Object key, Object value) {
	int hashValue = computeHashValue(key);
	if (hashArray[hashValue] == null || hashArray[hashValue].isEmpty()) return;
	lit1.setList(hashArray[hashValue]);
	HashEntry actO;
	while (lit1.hasNext()) {
	    actO = (HashEntry)lit1.next();
	    if (actO.equalKey(key)) {
		System.out.println("-->rm1");
		if (actO.equal(key,value)) {
		    System.out.println("-->rm2");
		    lit1.remove();
		    numberOfEntries--;
		}//if
	    }//if
	}//while lit1
    }//end method removes


    /**
     * Returns the number of stored objects.
     *
     * @return the number of objects
     */
    public int size() {
	return numberOfEntries;
    }//end method size


    /**
     * Empties the hashtable.
     */
    public void clear() {
	for (int i = 0; i < capacity; i++) {
	    if (!(hashArray[i] == null || hashArray[i].isEmpty()))
		hashArray[i].clear();
	}//for i
	numberOfEntries = 0;
    }//end method clear

    
    /**
     * Computes a hash value for a key.
     *
     * @param the key
     * @return the hashvalue = bucketnumber
     */
    private int computeHashValue(Object key) {
	int hashValue = key.hashCode();
	hashValue = hashValue % capacity;
	return hashValue;
    }//end method computeHashArrayAddress


}//end class ProHashtable
