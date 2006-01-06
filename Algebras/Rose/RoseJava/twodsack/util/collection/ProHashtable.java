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
 *
 */
public class ProHashtable {
    /*
     * members
     */
    private int capacity;
    private ProLinkedList[] hashArray;
    private int numberOfEntries;

    static private ProLinkedList staticList = new ProLinkedList();
    static private ProListIterator lit1 = new ProListIterator(staticList,0);


    /*
     * constructors
     */
    private ProHashtable() {}
    
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


    public boolean isEmpty() {
	return (numberOfEntries == 0);
    }//end method isEmpty


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


    public int size() {
	return numberOfEntries;
    }//end method size


    public void clear() {
	for (int i = 0; i < capacity; i++) {
	    if (!(hashArray[i] == null || hashArray[i].isEmpty()))
		hashArray[i].clear();
	}//for i
	numberOfEntries = 0;
    }//end method clear

    
    private int computeHashValue(Object key) {
	int hashValue = key.hashCode();
	hashValue = hashValue % capacity;
	return hashValue;
    }//end method computeHashArrayAddress


}//end class ProHashtable
