/*
 * RationalDouble.java 2005-05-13
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.number;

import java.io.*;


/**
 * This class extends the abstract class {@link Rational}.
 * It implements the Rational type on the base of <tt>double</tt> values. In fact, a Rational is
 * represented by a double. Of cause, it is well-known that computation with doubles is not precise. Hence, a deviation value is used to 
 * define a certain deviation which is allowed. So, for two Rational values <tt>P</tt> and <tt>Q</tt>, <tt>P</tt> == <tt>Q</tt> if
 * <tt>P</tt> > <tt>Q</tt>-<tt>deviation AND P</tt> < <tt>Q</tt> + <tt>deviation</tt>.<p>
 * By default, the value for deviation is set to 0.0000000001. However, the deviation value can be set t0 0.<p>
 * Computations are executed as described above, if the <tt>PRECISE</tt> flag is set to <tt>true</tt> (default = <tt>false</tt>).
 * If <tt>PRECISE = false</tt>, two values stored in other fields, namely <tt>DEVIATION_DOUBLE</tt> and <tt>DEVIATION_DOUBLE_MINUS</tt> are used.
 * These are double values which are used the same way as the <tt>deviation</tt> value above, but the computation is much faster. All critical computations
 * are executed as computations on doubles, then.<p>
 * There is only one double (numerator) field inside of this class which stores the Rational. The denominator is supposed to be 1 always.
 */
public class RationalDouble extends Rational implements Serializable{

    /*
     * members
     */
    static RationalDouble deviation = new RationalDouble(0.0000000001); //allowed deviation for comparisons to be equal; nine 0s
    static double DEVIATION_DOUBLE = 0.000001; //seven 0s
    static double DEVIATION_DOUBLE_NEG = -0.000001; //seven 0s
    static boolean PRECISE = false;
    private double d;	//numerator
    
    /*
     * constructors
     */
    /**
     * Constructs a new Rational from <tt>n</tt>.
     *
     * @param n the numerator
     */
    public RationalDouble(int n) { this.d = (double)n; }
   

    /**
     * Constructs a new Rational from <tt>f</tt>.
     *
     * @param f the numerator
     */
    public RationalDouble(double f) { this.d = f; }
    

    /**
     * Constructs a new Rational from <tt>r</tt>.
     *
     * @param r the numerator
     */
    public RationalDouble(Rational r) { this.d = ((RationalDouble)r).d; }


    /**
     * Constructs a new Rational from numerator and denominator.
     * The numerator is set to <tt>num</tt>/<tt>den</tt>
     * @param num the numerator
     * @param den the denominator
     */
    public RationalDouble(int num, int den) {
	this.d = ((double)num/den);
    }
    

    /*
     * methods
     */
    /**
     * Returns the numerator of the Rational.
     *
     * @return the numerator as <tt>int</tt>
     */
    public int getNumerator() {
	int dLength = String.valueOf((int)d).length();
	int multiple = 0;
	if (dLength < 9) {
	    multiple = 9-dLength;
	    return (int)(d*Math.pow(10,multiple));
	}//if
	else return (int)d;
    }//end method getNumerator


    /**
     * Returns the denominator of the Rational.
     *
     * @return the denominator as <tt>int</tt>
     */
    public int getDenominator() {
	int dLength = String.valueOf((int)d).length();
	int multiple = 0;
	if (dLength < 9) {
	    multiple = 9-dLength;
	    return (int)(Math.pow(10,multiple));
	}//if
	else return 1;
    }//end method getDenominator
    

    /**
     * Sets <i>this</i> to <tt>r</tt>.
     *
     * @param r the new Rational value <tt>r</tt>
     */
    public void assign (Rational r) { this.d = ((RationalDouble)r).d; }


    /**
     * Sets <i>this</i> to <tt>i</tt>.
     *
     * @param i the new Rational value <tt>i</tt>
     */
    public void assign (int i) { this.d = i; }

    
    /**
     * Sets <i>this</i> to <tt>d</tt>.
     *
     * @param d the new Rational value <tt>d</tt>
     */
    public void assign (double d) { this.d = d; }


    /**
     * Returns <i>this</i> * r.
     *
     * @param r the second factor
     * @return product of <i>this</i> and <tt>r</tt>
     */
    public Rational times (Rational r) { return new RationalDouble(this.d * ((RationalDouble)r).d); }
    

    /**
     * Returns <i>this</i> * <tt>i</tt>.
     *
     * @param i the second factor
     */
    public Rational times (int i) { return new RationalDouble(this.d * i); }


    /**
     * Returns <i>this</i> * <tt>r</tt>.
     * Stores the result in <i>in</i>.
     *
     * @param r the second factor
     * @param in the result is stored in this variable
     * @return <i>this</i> * <tt>r</tt>
     */
    public Rational times (Rational r, Rational in) {
	((RationalDouble)in).d = (this.d * ((RationalDouble)r).d);
	return in;
    }//end method times
  

    /**
     * Returns <i>this</i> : <tt>r</tt>.
     *
     * @param r the divisor
     * @return <i>this</i> : <tt>r</tt>
     */
    public Rational dividedby (Rational r) { return new RationalDouble(this.d / ((RationalDouble)r).d); }
    

    /**
     * Returns <i>this</i> : <tt>i</tt>.
     *
     * @param i the divisor
     * @return <i>this</i> : <tt>i</tt>
     */
    public Rational dividedby (int i) { return new RationalDouble(this.d / i); }


    /**
     * Returns <i>this</i> : <tt>r</tt>.
     * The result is stored in <i>in</i>.
     *
     * @param r the divisor
     * @param in the result is stored in this variable
     * @return <i>this</i> : <tt>r</tt>
     */
    public Rational dividedby (Rational r, Rational in) {
	((RationalDouble)in).d = (this.d / ((RationalDouble)r).d);
	return in;
    }


    /**
     * Returns <i>this</i> + <tt>r</tt>.
     *
     * @param r the summand
     * @return <i>this</i> + <tt>r</tt>
     */
    public Rational plus (Rational r) { return new RationalDouble(this.d + ((RationalDouble)r).d); }


    /**
     * Returns <i>this</i> + <tt>i</tt>.
     *
     * @param i the summand
     * @return <i>this</i> + <tt>i</tt>
     */
    public Rational plus (int i) { return new RationalDouble(this.d + i); }
    

    /**
     * Returns <i>this</i> + <tt>r</tt>.
     * The result is stored in <tt>in</tt>.
     *
     * @param r the summand
     * @param in the result is stored in this variable
     * @return <i>this</i> + <tt>r</tt>
     */
    public Rational plus (Rational r, Rational in) {
	((RationalDouble)in).d = (this.d + ((RationalDouble)r).d);
	return in;
    }//end method plus


    /**
     * Returns <i>this</i> - <tt>r</tt>.
     *
     * @param r the minuend
     * @return <i>this</i> - <tt>r</tt>
     */
    public Rational minus (Rational r) { return new RationalDouble(this.d - ((RationalDouble)r).d); }
    
    
    /**
     * Returns <i>this</i> - <tt>i</tt>.
     *
     * @param i the minuend
     * @return <i>this</i> - <tt>i</tt>
     */
    public Rational minus (int i) { return new RationalDouble(this.d - i); }
    

    /**
     * Returns <i>this</i> - <tt>r</tt>.
     * The result is stored in the variable <i>in</i>.
     *
     * @param r the minuend
     * @param in the result is stored in this variable
     * @return <i>this</i> - <tt>r</tt>
     */
    public Rational minus (Rational r, Rational in) {
	((RationalDouble)in).d = (this.d - ((RationalDouble)r).d);
	return in;
    }//end method minus


    /**
     * Returns <tt>true</tt>, if <i>this</i> is less than <tt>r</tt>.
     *
     * @param r the Rational to compare with
     * @return <tt>true</tt>, if <i>this</i> < <tt>r</tt>
     */
    public boolean less (Rational r) {
	if (PRECISE) {
	    if (this.equal(r)) return false;
	    else return (this.d < ((RationalDouble)r).d);
	}//if
	else {
	    if ((this.d - ((RationalDouble)r).d < DEVIATION_DOUBLE) &&
		(this.d - ((RationalDouble)r).d > DEVIATION_DOUBLE_NEG))
		return false;
	    else return (this.d < ((RationalDouble)r).d);
	}//else
    }//end method less

    

    /**
     * Returns <tt>true</tt>, if <i>this</i> is less than <tt>i</tt>.
     *
     * @param i the <tt>int</tt> to compare with
     * @return <tt>true</tt>, if <i>this</i> < <tt>i</tt>
     */
    public boolean less (int i) { 
	if (PRECISE) {
	    if (this.equal(i)) return false;
	    return (this.d < i);
	}//if
	else {
	    if ((this.d - i < DEVIATION_DOUBLE) &&
		(this.d - i > DEVIATION_DOUBLE_NEG))
		return false;
	    else return (this.d < i);
	}//else
    }//end method less
    

    /**
     * Returns <tt>true</tt>, if <i>this</i> is equal to <tt>r</tt>.
     *
     * @param r the Rational to compare with
     * @return <tt>true</tt>, if <i>this</i> = <tt>r</tt>
     */
    public boolean equal (Rational r) {
	if (PRECISE) {
	    double erg = Math.abs(this.d - ((RationalDouble)r).d);
	    if (erg < deviation.d) return true;
	    else return false;
	}//if
	else {
	    if ((this.d - ((RationalDouble)r).d < DEVIATION_DOUBLE) &&
		(this.d - ((RationalDouble)r).d > DEVIATION_DOUBLE_NEG))
		return true;
	    else return false;
	}//else
    }//end method equal
	    
    
    /**
     * Returns <tt>true</tt>, if <i>this</i> is equal to <tt>i</tt>.
     *
     * @param i the <tt>int</tt> to compare with
     * @return <tt>true</tt>, if <i>this</i> = <tt>i</tt>
     */
    public boolean equal (int i) {
	if (PRECISE) {
	    double erg = Math.abs(this.d - i);
	    if (erg < deviation.d) return true;
	    else return false;
	}//if
	else {
	    if ((this.d - i < DEVIATION_DOUBLE) &&
		(this.d - i > DEVIATION_DOUBLE_NEG))
		return true;
	    else return false;
	}//else
    }//end method equal
    

    /**
     * Returns <tt>true</tt>, if <i>this</i> is greater than <tt>r</tt>.
     *
     * @param r the Rational to compare with
     * @return <tt>true</tt>, if <i>this</i> > <tt>r</tt>
     */
    public boolean greater (Rational r) { 
	if (PRECISE) {
	    if (this.equal(r)) return false;
	    else return (this.d > ((RationalDouble)r).d);
	}//if
	else {
	    if ((this.d - ((RationalDouble)r).d < DEVIATION_DOUBLE) &&
		(this.d - ((RationalDouble)r).d > DEVIATION_DOUBLE_NEG))
		return false;
	    else return (this.d > ((RationalDouble)r).d);
	}//else
    }//end method greater
    

     /**
      * Returns <tt>true</tt>, if <i>this</i> is greater than <tt>i</tt>.
      *
      * @param i the <tt>int</tt> to compare with
      * @return <tt>true</tt>, if <i>this</i> > <tt>i</tt>
      */
    public boolean greater (int i) { 
	if (PRECISE) {
	    if (this.equal(i)) return false;
	    return (this.d > i);
	}//if
	else {
	    if ((this.d - i < DEVIATION_DOUBLE) &&
		(this.d - i > DEVIATION_DOUBLE_NEG))
		return false;
	    else return (this.d > i);
	}//else
    }//end method greater
    
    
    /**
     * Compares <i>this</i> and r and returns one of {0, 1, -1}.<p>
     * Returns 0, if <i>this</i> = r<p>
     * Returns -1, if <i>this</i> < r<p>
     * Returns 1 otherwise
     *
     * @param r the Rational to compare with
     * @return one of {0, 1, -1} as <tt>byte</tt>
     */
    public byte comp (Rational r) {
	if (this.equal(r)) return 0;
	if (this.d < ((RationalDouble)r).d) return -1;
	else return 1;
    }//end method comp
    

    /**
     * Returns <tt>true</tt>, if <i>this</i> <= <tt>r</tt>.
     *
     * @param r the Rational to compare with
     * @return <tt>true</tt> if <i>this</i> <= <tt>r</tt>
     */
    public boolean lessOrEqual (Rational r) {
	if (PRECISE)
	    return (this.equal(r) || this.less(r));
	else if (this.d - ((RationalDouble)r).d < DEVIATION_DOUBLE)
	    return true;
	return false;
    }//end method lessOrEqual
    

    /**
     * Returns <tt>true</tt>, if <i>this</i> >= <tt>r</tt>.
     *
     * @param r the Rational to compare with
     * @return <tt>true</tt> if <i>this</i> >= <tt>r</tt>
     */
    public boolean greaterOrEqual (Rational r) {
	if (PRECISE)
	    return (this.equal(r) || this.greater(r));
	else if (this.d - ((RationalDouble)r).d >= DEVIATION_DOUBLE_NEG)
	    return true;
	return false;
    }//end method greaterOrEqual
    
    
    /**
     * Returns <i>this</i> as <tt>int</tt>.
     *
     * @return <i>this</i> as <tt>int</tt>
     */
    public int getInt() { return (int)this.d; }


    /**
     * Returns <i>this</i> as <tt>double</tt>.
     *
     * @return <i>this</i> as <tt>double</tt>
     */
    public double getDouble() { return this.d; }


    /**
     * Converts <i>this</i> to a String.
     *
     * @return <i>this</i> as String
     */
    public String toString() { return String.valueOf(this.d); }
    

    /**
     * Returns a copy of <i>this</i>.
     *
     * @return the copy
     */
    public Rational copy() {return new RationalDouble(this); }
    

    /**
     * Returns the absolute value of <i>this</i>.
     *
     * @return |<i>this</i>|
     */
    public Rational abs() {
	if (this.d < 0) this.d = this.d * (-1);
	return this;
    }//end method abs


    /**
     * Rounds <i>this</i> to <i>i</i> <tt>digits</tt>.
     *
     * @param digits the number of digits
     * @throws WrongDigitValueException if 0 > <tt>digits</tt> > 9
     */
    public void round (int digits) throws WrongDigitValueException {
	if (digits<0 || digits>9) 
	    throw new WrongDigitValueException("RationalDouble: Wrong value for digits. Must lie in range (0..9)!");
	int times = (int)Math.pow(10,digits);
	double temp = Math.round(this.d * times);
	this.d = temp/times;
    }//end method round


    /**
     * Sets an field of the class to <i>b</i>.
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
	this.deviation = (RationalDouble)r;
    }//end method setDeviation


     /**
     * Sets the deviation values <tt>DEVIATION_DOUBLE</tt> and <tt>DEVIATION_DOUBLE_NEG</tt>.
     * This value is used for <tt>PRECISE = false</tt>.
     * <tt>DEVIATION_DOUBLE</tt> is set to <tt>d</tt> and <tt>DEVIATION_DOUBLE_NEG</tt> is set to <tt>-d</tt>.
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
    
}//end class RationalDouble
