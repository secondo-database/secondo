/*
 * Rational.java 2005-05-13
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.number;

/**
 * The Rational class is an abstract class that defines a set of methods. All those methods must be implemented by a class that implements
 * a Rational type for the 2DSACK package. How a Rational type is implemented is left to the implementor. He may implement is as a pair
 * of integers, as double or whatever. However, some of the methods are especially for Rational types (such as getDenominator) and
 * they must be implemented.<p>
 * Additionally to the methods, a variable named <i>deriv</i> must be defined. It determines a derivation value, which is needed for 
 * equality checks. This variable may be zero. The type of the <i>deriv</i> variable must be the type of the implementing class, e.g. 
 * if your class is called RationalFloat, then there must be a field <br>
 * <i>static final RationalFloat deriv = new RationalFloat(0);</i><p>
 * Furthermore, there must be constructors for the following types:<p>
 * <ul>
 * <li>int
 * <li>double
 * <li>Rational
 * <li>int x int
 * </ul>
 */
abstract public class Rational {
    /**
     * Returns the numerator of the Rational.
     *
     * @return the numerator as int
     */
    abstract public int getNumerator();


    /**
     * Returns the denominator of the Rational.
     *
     * @return the denominator as int
     */
    abstract public int getDenominator();


    /**
     * Sets <i>this</i> to r.
     *
     * @param r the new Rational value r
     */
    abstract public void assign(Rational r);
    

    /**
     * Sets <i>this</i> to i.
     *
     * @param i the new Rational value i
     */
    abstract public void assign(int i);


    /**
     * Sets <i>this</i> to d.
     *
     * @param d the new Rational value d
     */
    abstract public void assign(double d);


    /**
     * Returns <i>this</i> * r.
     *
     * @param r the second factor
     * @return product of this and r
     */
    abstract public Rational times(Rational r);


    /**
     * Returns <i>this</i> * i.
     *
     * @param i the second factor
     */
    abstract public Rational times(int i);


    /**
     * Returns <i>this</i> * r.
     * Stores the result in <i>in</i>.
     *
     * @param r the second factor
     * @param in the result is stored in this variable
     * @return this * r
     */
    abstract public Rational times(Rational r, Rational in);


    /**
     * Returns <i>this</i> : r.
     *
     * @param r the divisor
     * @return this : r
     */
    abstract public Rational dividedby (Rational r);


    /**
     * Returns <i>this</i> : i.
     *
     * @param i the divisor
     * @return this : i
     */
    abstract public Rational dividedby (int i);


    /**
     * Returns <i>this</i> : r.
     * The result is stored in <i>in</i>.
     *
     * @param r the divisor
     * @param in the result is stored in this variable
     * @return this : r
     */
    abstract public Rational dividedby (Rational r, Rational in);


    /**
     * Returns <i>this</i> + r.
     *
     * @param r the summand
     * @return this + r
     */
    abstract public Rational plus(Rational r);
    

    /**
     * Returns <i>this</i> + i.
     *
     * @param i the summand
     * @return this + i
     */
    abstract public Rational plus(int i);


    /**
     * Returns <i>this</i> + r.
     * The result is stored in in.
     *
     * @param r the summand
     * @param in the result is stored in this variable
     * @return this + r
     */
    abstract public Rational plus(Rational r, Rational in);


    /**
     * Returns <i>this</i> - r.
     *
     * @param r the minuend
     * @return this - r
     */
    abstract public Rational minus(Rational r);


    /**
     * Returns <i>this</i> - i.
     *
     * @param i the minuend
     * @return this - i
     */
    abstract public Rational minus(int i);


    /**
     * Returns <i>this</i> - r.
     * The result is stored in the variable <i>in</i>.
     *
     * @param r the minuend
     * @param in the result is stored in this variable
     * @return this - r
     */
    abstract public Rational minus(Rational r, Rational in);


    /**
     * Returns true, if <i>this</i> is less than r.
     *
     * @param r the Rational to compare with
     * @return true, if this < r
     */
    abstract public boolean less(Rational r);


    /**
     * Returns true, if <i>this</i> is less than i.
     *
     * @param i the int to compare with
     * @return true, if this < i
     */
    abstract public boolean less(int i);


    /**
     * Returns true, if <i>this</i> is equal to r.
     *
     * @param r the Rational to compare with
     * @return true, if this = r
     */
    abstract public boolean equal(Rational r);


    /**
     * Returns true, if <i>this</i> is equal to i.
     *
     * @param i the int to compare with
     * @return true, if this = i
     */
    abstract public boolean equal(int i);


    /**
     * Returns true, if <i>this</i> is greater than r.
     *
     * @param r the Rational to compare with
     * @return true, if this > r
     */
    abstract public boolean greater(Rational r);


    /**
     * Returns true, if <i>this</i> is greater than i.
     *
     * @param i the int to compare with
     * @return true, if this > i
     */
    abstract public boolean greater(int i);


    /**
     * Compares <i>this</i> and r and returns one of {0, 1, -1}.
     * Returns 0, if <i>this</i> = r<p>
     * Returns -1, if <i>this</i> < r<p>
     * Returns 1 otherwise
     *
     * @param r the Rational to compare with
     * @return one of {0, 1, -1} as byte
     */
    abstract public byte comp(Rational r);


    /**
     * Returns true, if <i>this</i> <= r.
     *
     * @param r the Rational to compare with
     * @return true if this <= r
     */
    abstract public boolean lessOrEqual(Rational r);


    /**
     * Returns true, if <i>this</i> >= r.
     *
     * @param r the Rational to compare with
     * @return true if this >= r
     */
    abstract public boolean greaterOrEqual(Rational r);


    /**
     * Returns <i>this</i> as int.
     *
     * @return this as int
     */
    abstract public int getInt();


    /**
     * Returns <i>this</i> as double.
     *
     * @return this as double
     */
    abstract public double getDouble();


    /**
     * Converts <i>this</i> to a String.
     *
     * @return this as String
     */
    abstract public String toString();


    /**
     * Returns a copy of this.
     *
     * @return the copy
     */
    abstract public Rational copy();


    /**
     * Returns the absolute value of <i>this</i>.
     *
     * @return |this|
     */
    abstract public Rational abs();


    /**
     * Rounds <i>this</i> to <i>i</i> digits.
     *
     * @param i the number of digits
     */
    abstract public void round(int i);


    /**
     * Sets an field of the class to <i>b</i>.
     * The implementor can decide, whether the class should have a 'precise' and a 'less precise' implementation. By using this method
     * a flag can be set to use the more or less precise version. PRECISE=true means, that the derivation value is automatically set to 0.
     *
     * @param b PRECISE is set to this value
     */
    abstract public void setPrecision(Boolean b);
    
}//end class Rational
