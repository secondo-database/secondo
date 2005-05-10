/*
 * CycleListList.java 2004-11-05
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util;

/**
 * This interface provides only one single mehtod, namely <code>compare</code>. <i>MSE</i> is short for
 * <i>MultiSetElement</i>. Each data type that shall be used together with a <code>MultiSet</code>
 * must implement this interface.
 */

public interface ComparableMSE {

    /**
     * Defines an order on all data types that implement this interface.
     * If properly implemented, returns 0 if <code>this</code> and <code>in</code> are equal,
     * -1 if <code>this</code> is smaller than <code>in</code> and 1 otherwise.
     * 
     * @param in the instance <code>this</code> shall be compared with
     * @return {0,-1,1} as <code>int</code>
     */
    public int compare(ComparableMSE in);

}