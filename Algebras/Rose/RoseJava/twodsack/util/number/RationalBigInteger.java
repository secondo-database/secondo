/*
 * RationalBigInteger.java 2005-05-13
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.number;

import java.io.*;
import java.math.*;

/**
 * The RationalBigInteger class is an extension of the abstract {@link Rational} class.
 * It implements Rational numbers using {@link java.math.BigInteger}s for the 
 * numerator and denominator. All numbers can be computed with absolute precision. Unfortunately, the values for numerator and denominator
 * can get really big. Hence, when using this implementation the execution of operations will get very slow.<p>
 * Note, that the values for numerator and denominator must lie in the range of ordinary Integer values. Otherwise a {@link RationalOverflowException}
 * is thrown.
 */
public class RationalBigInteger extends Rational implements Serializable{
    /*
     * If you want to change the number of digits for the Rational converter, change the value
     * <tt>NUM_DIGITS</tt> in the members section.
     * The deviation value for an allowed deviation can be set by setting a value for 'deviation'.
     */
    /*
     * fields
     */
    static final int NUM_DIGITS = 7; //number of digits used right of the decimal point
    static RationalBigInteger deviation = new RationalBigInteger(0,1);
    static double DEVIATION_DOUBLE = 0;
    static double DEVIATION_DOUBLE_NEG = 0;
    
    /**
     * A flag for 'precise' or 'not so precise' computation. If <tt>true</tt>, everything is computed using RationalBigIntegers. Otherwise,
     * at some places a faster implementation using Doubles is used.
     */
    static boolean PRECISE = false;

    private BigInteger n;	//numerator
    private BigInteger d;	//denominator
    

    /*
     * constructors
     */
    /**
     * Constructs a new instance from two BigInteger values.
     *
     * @param n the numerator
     * @param d the denominator
     * @throws DivisionByZeroException if <tt>d == 0</tt>
     * @throws RationalOverflowException if <tt>n</tt>, <tt>d</tt> are out of Integer range
     */
    private RationalBigInteger(BigInteger n, BigInteger d) throws DivisionByZeroException, RationalOverflowException {
	if (d.equals(BigInteger.ZERO)) {
	    throw new DivisionByZeroException("Tried to construct RationalBigInteger("+n.toString()+","+d.toString()+").");
	}//if
	//cancel is now done in the other operations
	if (n.divide(d).compareTo(new BigInteger(String.valueOf(2147483647))) == 1) {
	    throw new RationalOverflowException("Value is higher than integer range (2147483647): "+n.divide(d)); }
	this.n = n;
	this.d = d;
	this.cancel();
    }
    

    /**
     * Constructs a new instance from an <tt>int</tt> value.
     * 
     * @param n the nominator; denominator is set to 1
     */
    public RationalBigInteger(int n) {
	this(new BigInteger(String.valueOf(n)), new BigInteger(String.valueOf(1)));}
    

    /**
     * Constructs a new instance from a <tt>double</tt> value.
     * Seven fraction digits used.
     * @param f the nominator
     */
    public RationalBigInteger(double f) {
	this(new BigInteger(String.valueOf((int) Math.round(f * 10000000))),new BigInteger(String.valueOf(10000000)));
    }


    /**
     * Constructs a new instance from a {@link Rational} value.
     * 
     * @param r the new Rational value
     */    
    public RationalBigInteger(Rational r) {
	this.n = ((RationalBigInteger)r).n;
	this.d = ((RationalBigInteger)r).d;
    }


    /**
     * Constructs a new instance from two <tt>int</tt> values
     *
     * @param nIn the numerator
     * @param dIn the denominator
     */
    public RationalBigInteger(int nIn, int dIn) {
	this(new BigInteger(String.valueOf(nIn)), new BigInteger(String.valueOf(dIn))); }
    

    /*
     * methods
     */
    /**
     * Returns the numerator of the Rational.
     *
     * @return the numerator as <tt>int</tt>
     */
    public int getNumerator() {
	BigInteger hi = new BigInteger(String.valueOf(2147483647));
	BigInteger lo = new BigInteger(String.valueOf(-2147483648));
	if (n.compareTo(hi) == 1 || n.compareTo(lo) == -1 ||
	    d.compareTo(hi) == 1 || d.compareTo(lo) == -1) {
	    BigDecimal res = (new BigDecimal(n,NUM_DIGITS)).divide(new BigDecimal(d,NUM_DIGITS),BigDecimal.ROUND_UP);
	    res = res.movePointRight(NUM_DIGITS);
	    return res.intValue();
	}//if
	else return n.intValue();
    }//end method getNumerator


    /**
     * Returns the denominator of the Rational.
     *
     * @return the denominator as <tt>int</tt>
     */ 
    public int getDenominator() {
	BigInteger hi = new BigInteger(String.valueOf(2147483647));
	BigInteger lo = new BigInteger(String.valueOf(-2147483648));
	if (d.compareTo(hi) == 1 || d.compareTo(lo) == -1 ||
	    n.compareTo(hi) == 1 || n.compareTo(lo) == -1) {
	    return 1;
	}//if
	else return d.intValue();
    }//end method getDenominator
    

    /**
     * Sets <i>this</i> to r.
     *
     * @param r the new Rational value r
     */
    public void assign(Rational r) {
	n = ((RationalBigInteger)r).n;
	d = ((RationalBigInteger)r).d;
    }//end method assign
    

    /**
     * Sets <i>this</i> to <tt>i</tt>.
     *
     * @param i the new Rational value <tt>i</tt>
     */
    public void assign(int i) {
	n = new BigInteger(String.valueOf(i));
	d = new BigInteger(String.valueOf(1));
    }//end method assign
    

     /**
     * Sets <i>this</i> to <tt>d</tt>.
     *
     * @param d the new Rational value <tt>d</tt>
     */
    public void assign(double d) {
	this.n = new BigInteger(String.valueOf((int) Math.round(d * 10000000)));
	this.d = new BigInteger(String.valueOf(10000000));
    }//end method assign


     /**
     * Returns <i>this</i> * <tt>r</tt>.
     *
     * @param r the second factor
     * @return product of this and <tt>r</tt>
     */
    public Rational times (Rational r) {
	return new RationalBigInteger(n.multiply(((RationalBigInteger)r).n),
				      d.multiply(((RationalBigInteger)r).d));
    }//end method times
    

    /**
     * Returns <i>this</i> * <tt>i</tt>.
     *
     * @param i the second factor
     */
    public Rational times (int i) {
	return new RationalBigInteger(n.multiply(new BigInteger(String.valueOf(i))),d);
    }//end method times


    /**
     * Returns <i>this</i> * <tt>r</tt>.
     * Stores the result in <i>in</i>.
     *
     * @param r the second factor
     * @param in the result is stored in this variable
     * @return <i>this</i> * <tt>r</tt>
     */
    public Rational times (Rational r, Rational in) {
	((RationalBigInteger)in).n = n.multiply(((RationalBigInteger)r).n);
	((RationalBigInteger)in).d = d.multiply(((RationalBigInteger)r).d);
	return in;
    }
    

    /**
     * Returns <i>this</i> : <tt>r</tt>.
     *
     * @param r the divisor
     * @return <i>this</i> : <tt>r</tt>
     */
    public Rational dividedby (Rational r) {
	return new RationalBigInteger(n.multiply(((RationalBigInteger)r).d),
				      d.multiply(((RationalBigInteger)r).n));
    }//end method dividedby
    

     /**
      * Returns <i>this</i> : <tt>i</tt>.
      *
      * @param i the divisor
      * @return <i>this</i> : <tt>i</tt>
      */
    public Rational dividedby (int i) {
	return new RationalBigInteger(n,d.multiply(new BigInteger(String.valueOf(i))));
    }//end method dividedby
    
    
     /**
      * Returns <i>this</i> : <tt>r</tt>.
      * The result is stored in <tt>in</tt>.
      *
      * @param r the divisor
      * @param in the result is stored in this variable
      * @return <i>this</i> : <tt>r</tt>
      */
    public Rational dividedby (Rational r, Rational in) {
	((RationalBigInteger)in).n = n.multiply(((RationalBigInteger)r).d);
	((RationalBigInteger)in).d = d.multiply(((RationalBigInteger)r).n);
	return in;
    }

    
    /**
     * Returns <i>this</i> + <tt>r</tt>.
     *
     * @param r the summand
     * @return <i>this</i> + <tt>r</tt>
     */
    public Rational plus (Rational r) {
	return new RationalBigInteger(n.multiply(((RationalBigInteger)r).d).add(d.multiply(((RationalBigInteger)r).n)),d.multiply(((RationalBigInteger)r).d));
    }//end method plus
    

    /**
     * Returns <i>this</i> + <tt>i</tt>.
     *
     * @param i the summand
     * @return <i>this</i> + <tt>i</tt>
     */
    public Rational plus (int i) {
	return new RationalBigInteger(n.add(d.multiply(new BigInteger(String.valueOf(i)))),d);
    }//end method plus
    
    
    /**
     * Returns <i>this</i> + <tt>r</tt>.
     * The result is stored in <tt>i</tt>n.
     *
     * @param r the summand
     * @param in the result is stored in this variable
     * @return <i>this</i> + <tt>r</tt>
     */
    public Rational plus (Rational r, Rational in) {
	((RationalBigInteger)in).n = n.multiply(((RationalBigInteger)r).d).add(d.multiply(((RationalBigInteger)r).n));
	((RationalBigInteger)in).d = d.multiply(((RationalBigInteger)r).d);
	return in;
    }
    

    /**
     * Returns <i>this</i> - <tt>r</tt>.
     *
     * @param r the minuend
     * @return <i>this</i> - <tt>r</tt>
     */
    public Rational minus (Rational r) {
	return new RationalBigInteger(n.multiply(((RationalBigInteger)r).d).subtract(d.multiply(((RationalBigInteger)r).n)),d.multiply(((RationalBigInteger)r).d));
    }//end method minus
    
    
    /**
     * Returns <i>this</i> - <tt>i</tt>.
     *
     * @param i the minuend
     * @return <i>this</i> - <tt>i</tt>
     */
    public Rational minus (int i) {
	return new RationalBigInteger(n.subtract(d.multiply(new BigInteger(String.valueOf(i)))),d);
    }//end method minus
    
    
    /**
     * Returns <i>this</i> - <tt>r</tt>.
     * The result is stored in the variable <tt>in</tt>.
     *
     * @param r the minuend
     * @param in the result is stored in this variable
     * @return <i>this</i> - <tt>r</tt>
     */
    public Rational minus (Rational r, Rational in) {
	((RationalBigInteger)in).n = n.multiply(((RationalBigInteger)r).d).subtract(d.multiply(((RationalBigInteger)r).n));
	((RationalBigInteger)in).d = d.multiply(((RationalBigInteger)r).d);
	return in;
    }//end method minus
    

    /**
     * Returns <tt>true</tt>, if <i>this</i> is less than <tt>r</tt>.
     *
     * @param r the Rational to compare with
     * @return <tt>true</tt>, if <i>this</i> < <tt>r</tt>
     */
    public boolean less (Rational r) {
	return (this.n.multiply(((RationalBigInteger)r).d).compareTo(this.d.multiply(((RationalBigInteger)r).n)) == -1);
    }//end method less
    
    
    /**
     * Returns <tt>true</tt>, if <i>this</i> is less than <tt>i</tt>.
     *
     * @param i the int to compare with
     * @return <tt>true</tt>, if <i>this</i> < <tt>i</tt>
     */
    public boolean less (int i) {
	return (n.compareTo(new BigInteger(String.valueOf(i)).multiply(d)) == -1);
    }//end method less
    
    
    /**
     * Returns <tt>true</tt>, if <i>this</i> is equal to <tt>r</tt>.
     *
     * @param r the Rational to compare with
     * @return <tt>true</tt>, if <i>this</i> = <tt>r</tt>
     */
    public boolean equal (Rational r) {
	return (n.multiply(((RationalBigInteger)r).d).equals(d.multiply(((RationalBigInteger)r).n)));
    }//end method equal
    

    /**
     * Returns <tt>true</tt>, if <i>this</i> is equal to <tt>i</tt>.
     *
     * @param i the int to compare with
     * @return <tt>true</tt>, if <i>this</i> = <tt>i</tt>
     */
    public boolean equal (int i) {
	return (n.equals(d.multiply(new BigInteger(String.valueOf(i)))));
    }//end method equal
    
    
    /**
     * Returns <tt>true</tt>, if <i>this</i> is greater than <tt>r</tt>.
     *
     * @param r the Rational to compare with
     * @return <tt>true</tt>, if <i>this</i> > <tt>r</tt>
     */
    public boolean greater (Rational r) {
	return (this.n.multiply(((RationalBigInteger)r).d).compareTo(this.d.multiply(((RationalBigInteger)r).n)) == 1);
    }//end method greater
    
    
    /**
     * Returns <tt>true</tt>, if <i>this</i> is greater than <tt>i</tt>.
     *
     * @param i the int to compare with
     * @return <tt>true</tt>, if <i>this</i> > <tt>i</tt>
     */
    public boolean greater (int i) {
	return (n.compareTo(new BigInteger(String.valueOf(i)).multiply(d)) == 1);
    }//end method greater
    
    
    /**
     * Compares <i>this</i> and <tt>r</tt> and returns one of {0, 1, -1}.<p>
     * Returns 0, if <i>this</i> = <tt>r</tt><p>
     * Returns -1, if <i>this</i> < <tt>r</tt><p>
     * Returns 1 otherwise
     *
     * @param r the Rational to compare with
     * @return one of {0, 1, -1} as <tt>byte</tt>
     */
    public byte comp (Rational r) {
	BigInteger res1 = n.multiply(((RationalBigInteger)r).d);
	BigInteger res2 = d.multiply(((RationalBigInteger)r).n);
	return (byte)res1.compareTo(res2);
    }//end method comp
    
    
    /**
     * Returns <tt>true</tt>, if <i>this</i> <= <tt>r</tt>.
     *
     * @param r the Rational to compare with
     * @return <tt>true</tt> if <i>this</i> <= <tt>r</tt>
     */
    public boolean lessOrEqual (Rational r) {
	return (this.less(r) || this.equal(r));
    }//end method lessOrEqual
    

    /**
     * Returns <tt>true</tt>, if <i>this</i> >= <tt>r</tt>.
     *
     * @param r the Rational to compare with
     * @return <tt>true</tt> if <i>this</i> >= <tt>r</tt>
     */
    public boolean greaterOrEqual (Rational r) {
	return (this.greater(r) || this.equal(r));
    }//end method greaterOrEqual
    
    
    /**
     * Returns <i>this</i> as <tt>int</tt>.
     * Of cause, this method cannot return a precise result. The result is rounded.
     *
     * @return <i>this</i> as <tt>int</tt>
     */
    public int getInt() {
	int res = (int)Math.round(this.n.doubleValue() / this.d.doubleValue());
	return res;
    }//end method getInt

    
   /**
     * Returns <i>this</i> as <tt>double</tt>.
     * Of cause, this method cannot return a precise result. The result is rounded.
     *
     * @return <i>this</i> as <tt>double</tt>
     */
    public double getDouble() {
	double res = this.n.doubleValue() / this.d.doubleValue();
	return res;
    }//end method getDouble
	

    /**
     * Converts <i>this</i> to a <tt>String</tt>.
     *
     * @return this as String
     */
    public String toString() {
	if (d.equals(BigInteger.ONE)) return String.valueOf(n); 
	if (n.divide(d).equals(BigInteger.ZERO)) {
	    return String.valueOf(n)+"/"+String.valueOf(d);}
	else {
	    byte res = (byte)n.compareTo(BigInteger.ZERO);
	    if (res >= 0) 
		return (n.divide(d).toString()+" "+n.divideAndRemainder(d)[1].toString()+
			"/"+d.toString());
	    else {
		RationalBigInteger cop = (RationalBigInteger)this.abs();
		return ("-"+cop.n.divide(cop.d).toString()+" "+cop.n.divideAndRemainder(cop.d)[1].toString()+"/"+cop.d.toString());
	    }//else
	}//else
    }//end method toString
    

    /**
     * Returns the greatest commond divisor of <tt>a</tt>, <tt>b</tt>.
     *
     * @param a the first Rational
     * @param b the second Rational
     * @return the divisor as BigInteger
     */
    protected static BigInteger gcd(BigInteger a, BigInteger b) {
	if (b.equals(BigInteger.ZERO)) return a;
	else return gcd(b, a.divideAndRemainder(b)[1]);
    }//end method gcd


    /**
     * Cancels <i>this</i>.
     */
    protected void cancel(){
	BigInteger f = gcd(this.n,this.d);
	this.n = this.n.divide(f);
	this.d = this.d.divide(f);
	if (this.d.compareTo(BigInteger.ZERO) == -1) {
	    this.d = this.d.multiply(new BigInteger(String.valueOf(-1)));
	    this.n = this.n.multiply(new BigInteger(String.valueOf(-1)));
	}//if
    }//end method cancel
    

    /**
     * Returns a copy of <i>this</i>.
     *
     * @return the copy
     */
    public Rational copy() {return new RationalBigInteger(this); }
    

    /**
     * Returns the absolute value of <i>this</i>.
     *
     * @return |<i>this</i>|
     */
    public Rational abs() {
	Rational retVal = this.copy();
	if (this.less(0)) {
	    retVal = this.times(new RationalBigInteger(-1));
	}//if
	return retVal;
    }//end method abs


    /**
     * Rounds <i>this</i> to <i>i</i> digits.
     *
     * @param digits the number of digits
     * @throws WrongDigitValueException if the number of digits <tt>d</tt> is not 0 <= <tt>d</tt> <= 9
     */
    public void round (int digits) {
	if (digits<0 || digits>9)
	    throw new WrongDigitValueException("RationalBigInteger: Wrong value for digits. Must lie in range (0..9)!");
	double val = this.getDouble();
	int times = (int)Math.pow(10,digits);
	int temp = (int)Math.round(val * times);
	this.n = new BigInteger(String.valueOf(temp));
	this.d = new BigInteger(String.valueOf(times));
    }//end method round
	
    
    /**
     * Sets an field of the class to <tt>b</tt>.
     * The implementor can decide, whether the class should have a 'precise' and a 'less precise' implementation. By using this method
     * a flag can be set to use the more or less precise version. <tt>PRECISE == true</tt> means, that the deviation value is automatically set to 0.
     *
     * @param precise <tt>PRECISE</tt> is set to this value
     */
    public void setPrecision (Boolean precise) {
	PRECISE = precise.booleanValue();
    }//end method setPrecision
    

    /**
     * Returns the deviation value for computations with <tt>deviation = true</tt>.
     *
     * @return the deviation value
     */
    public Rational getDeviation() {
	return this.deviation;
    }//end method getDeviation


    /**
     * Sets the deviation value <tt>deviation</tt>.
     * This number is used for equality checks when <tt>PRECISE = true</tt>.
     *
     * @param r the new deviation value
     */
    public void setDeviation(Rational r) {
	this.deviation = (RationalBigInteger)r;
    }//end method setDeviation

    
    /**
     * Sets the deviation values <tt>DEVIATION_DOUBLE</tt> and <tt>DEVIATION_DOUBLE_NEG</tt>.
     * This value is used for <tt>PRECISE = false</tt>.
     * <tt>DEVIATION_DOUBLE</tt> is set to <tt>d</tt> and <tt>DEVIATIION_DOUBLE_NEG</tt> is set to <tt>-d</tt>.
     *
     * @param d the new deviation value
     */
    public void setDeviationDouble(Double d) {
	this.DEVIATION_DOUBLE = d.doubleValue();
	this.DEVIATION_DOUBLE_NEG = -1*d.doubleValue();
    }//end method setDeviationDouble


    /**
     * Returns the <tt>DEVIATION_DOUBLE</tt> value.
     *
     * @return the deviation value
     */
    public double getDeviationDouble() {
	return this.DEVIATION_DOUBLE;
    }//end method getDeviationDouble


    /**
     * Returns the <tt>DEVIATION_DOUBLE_NEG</tt> value.
     *
     * @return the deviation value.
     */
    public double getDeviationDoubleNeg() {
	return this.DEVIATION_DOUBLE_NEG;
    }//end method getDeviationDoubleNeg


}//end class RationalBigInteger
