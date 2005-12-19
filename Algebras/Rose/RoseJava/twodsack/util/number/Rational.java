/*
 * Rational.java 2005-05-13
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.number;

/**
 * The Rational class is an abstract class that defines a set of methods.
 * All those methods must be implemented by a class that implements
 * a Rational type for the 2DSACK package. How a Rational type is implemented is left to the implementor. He may implement is as a pair
 * of integers, as double or whatever. However, some of the methods are especially for Rational types (such as {@link #getDenominator()}) and
 * they must be implemented.<p>
 * For equality checks, there are two mechanisms in the 2DSACK package. For a class implementing the Rational type, a boolean field named <tt>PRECISE</tt>
 * tells whether a number of type Rational (named <tt>deviation</tt>) or two numbers of type double (named <tt>DEVIATION_DOUBLE</tt> and <tt>DEVIATION_DOUBLE_NEG</tt>)
 * are used. Make sure, that those fields are implemented.
 *
 * Furthermore, there must be constructors for the following types:<p>
 * <ul><tt>
 * <li>int
 * <li>double
 * <li>Rational
 * <li>int x int
 * </tt></ul>
 */
abstract public class Rational {
    /*
     * constructors
     */
    /**
     * The standard constructor.
     */
    public Rational(){}

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
     * Sets <i>this</i> to <tt>r</tt>.
     *
     * @param r the new Rational value <tt>r</tt>
     */
    abstract public void assign(Rational r);
    

    /**
     * Sets <i>this</i> to <tt>i</tt>.
     *
     * @param i the new Rational value <tt>i</tt>
     */
    abstract public void assign(int i);


    /**
     * Sets <i>this</i> to <tt>d</tt>.
     *
     * @param d the new Rational value <tt>d</tt>
     */
    abstract public void assign(double d);


    /**
     * Returns <i>this</i> * <tt>r</tt>.
     *
     * @param r the second factor
     * @return product of this and <tt>r</tt>
     */
    abstract public Rational times(Rational r);


    /**
     * Returns <i>this</i> * <tt>i</tt>.
     *
     * @param i the second factor
     */
    abstract public Rational times(int i);


    /**
     * Returns <i>this</i> * <tt>r</tt>.
     * Stores the result in <tt>in</tt>.
     *
     * @param r the second factor
     * @param in the result is stored in this variable
     * @return this * <tt>r</tt>
     */
    abstract public Rational times(Rational r, Rational in);


    /**
     * Returns <i>this</i> : <tt>r</tt>.
     *
     * @param r the divisor
     * @return <i>this</i> : <tt>r</tt>
     */
    abstract public Rational dividedby (Rational r);


    /**
     * Returns <i>this</i> : <tt>i</tt>.
     *
     * @param i the divisor
     * @return <i>this</i> : <tt>i</tt>
     */
    abstract public Rational dividedby (int i);


    /**
     * Returns <i>this</i> : <tt>r</tt>.
     * The result is stored in <tt>in</tt>.
     *
     * @param r the divisor
     * @param in the result is stored in this variable
     * @return <i>this</i> : <tt>r</tt>
     */
    abstract public Rational dividedby (Rational r, Rational in);


    /**
     * Returns <i>this</i> + <tt>r</tt>.
     *
     * @param r the summand
     * @return <i>this</i> + <tt>r</tt>
     */
    abstract public Rational plus(Rational r);
    

    /**
     * Returns <i>this</i> + <tt>i</tt>.
     *
     * @param i the summand
     * @return <i>this</i> + <tt>i</tt>
     */
    abstract public Rational plus(int i);


    /**
     * Returns <i>this</i> + <tt>r</tt>.
     * The result is stored in <tt>in</tt>.
     *
     * @param r the summand
     * @param in the result is stored in this variable
     * @return <i>this</i> + <tt>r</tt>
     */
    abstract public Rational plus(Rational r, Rational in);


    /**
     * Returns <i>this</i> - <tt>r</tt>.
     *
     * @param r the minuend
     * @return <i>this</i> - <tt>r</tt>
     */
    abstract public Rational minus(Rational r);


    /**
     * Returns <i>this</i> - <tt>i</tt>.
     *
     * @param i the minuend
     * @return <i>this</i> - <tt>i</tt>
     */
    abstract public Rational minus(int i);


    /**
     * Returns <i>this</i> - <tt>r</tt>.
     * The result is stored in the variable <tt>in</tt>.
     *
     * @param r the minuend
     * @param in the result is stored in this variable
     * @return <i>this</i> - <tt>r</tt>
     */
    abstract public Rational minus(Rational r, Rational in);


    /**
     * Returns <tt>true</tt>, if <i>this</i> is less than <tt>r</tt>.
     *
     * @param r the Rational to compare with
     * @return <tt>true</tt>, if <i>this</i> < <tt>r</tt>
     */
    abstract public boolean less(Rational r);


    /**
     * Returns <tt>true</tt>, if <i>this</i> is less than <tt>i</tt>.
     *
     * @param i the int to compare with
     * @return <tt>true</tt>, if <i>this</i> < <tt>i</tt>
     */
    abstract public boolean less(int i);


    /**
     * Returns true, if <i>this</i> is equal to <tt>r</tt>.
     *
     * @param r the Rational to compare with
     * @return <tt>true</tt>, if <i>this</i> = <tt>r</tt>
     */
    abstract public boolean equal(Rational r);


    /**
     * Returns <tt>true</tt>, if <i>this</i> is equal to <tt>i</tt>.
     *
     * @param i the int to compare with
     * @return <tt>true</tt>, if <i>this</i> = <tt>i</tt>
     */
    abstract public boolean equal(int i);


    /**
     * Returns <tt>true</tt>, if <i>this</i> is greater than <tt>r</tt>.
     *
     * @param r the Rational to compare with
     * @return <tt>true</tt>, if <i>this</i> > <tt>r</tt>
     */
    abstract public boolean greater(Rational r);


    /**
     * Returns <tt>true</tt>, if <i>this</i> is greater than <tt>i</tt>.
     *
     * @param i the int to compare with
     * @return <tt>true</tt>, if <i>this</i> > <tt>i</tt>
     */
    abstract public boolean greater(int i);


    /**
     * Compares <i>this</i> and r and returns one of {0, 1, -1}.<p>
     * Returns 0, if <i>this</i> = <tt>r</tt><p>
     * Returns -1, if <i>this</i> < <tt>r</tt><p>
     * Returns 1 otherwise
     *
     * @param r the Rational to compare with
     * @return one of {0, 1, -1} as byte
     */
    abstract public byte comp(Rational r);


    /**
     * Returns <tt>true</tt>, if <i>this</i> <= <tt>r</tt>.
     *
     * @param r the Rational to compare with
     * @return <tt>true</tt> if <i>this</i> <= <tt>r</tt>
     */
    abstract public boolean lessOrEqual(Rational r);


    /**
     * Returns <tt>true</tt>, if <i>this</i> >= <tt>r</tt>.
     *
     * @param r the Rational to compare with
     * @return <tt>true</tt> if <i>this</i> >= <tt>r</tt>
     */
    abstract public boolean greaterOrEqual(Rational r);


    /**
     * Returns <i>this</i> as int.
     *
     * @return <i>this</i> as int
     */
    abstract public int getInt();


    /**
     * Returns <i>this</i> as double.
     *
     * @return <i>this</i> as double
     */
    abstract public double getDouble();


    /**
     * Converts <i>this</i> to a String.
     *
     * @return <i>this</i> as String
     */
    abstract public String toString();


    /**
     * Returns a copy of <i>this</i>.
     *
     * @return the copy
     */
    abstract public Rational copy();


    /**
     * Returns the absolute value of <i>this</i>.
     *
     * @return |<i>this</i>|
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
     * a flag can be set to use the more or less precise version. <tt>PRECISE=true</tt> means, that the deviation value is automatically set to 0.
     *
     * @param b <tt>PRECISE</tt> is set to this value
     */
    abstract public void setPrecision(Boolean b);

    
    /**
     * Returns the deviation value for computations with <tt>deviation = true</tt>.
     *
     * @return the deviation value
     */
    abstract public Rational getDeviation();


    /**
     * Sets the deviation value <tt>deviation</tt>.
     * This number is used for equality checks when <tt>PRECISE = true</tt>.
     *
     * @param r the new deviation value
     */
    abstract public void setDeviation(Rational r);

    

    /**
     * Sets the deviation values <tt>DEVIATION_DOUBLE</tt> and <tt>DEVIATION_DOUBLE_NEG</tt>.
     * This value is used for <tt>PRECISE = false</tt>.
     * <tt>DEVIATION_DOUBLE</tt> is set to <tt>d</tt> and <tt>DEVIATION_DOUBLE_NEG</tt> is set to <tt>-d</tt>.
     *
     * @param d the new deviation value
     */
    abstract public void setDeviationDouble(Double d);


    /**
     * Returns the <tt>DEVIATION_DOUBLE</tt> value.
     *
     * @return the deviation value
     */
    abstract public double getDeviationDouble();


    /**
     * Returns the <tt>DEVIATION_DOUBLE_NEG</tt> value.
     *
     * @return the deviation value.
     */
    abstract public double getDeviationDoubleNeg();

}//end class Rational
