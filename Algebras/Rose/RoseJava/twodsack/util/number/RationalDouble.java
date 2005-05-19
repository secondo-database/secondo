/*
 * RationalDouble.java 2005-05-13
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.number;

import java.io.*;


/**
 * This class extends the abstract class {@link Rational}. It implements the Rational type on the base of double values. In fact, a Rational is
 * represented by a double. Of cause, it is well-known that computation with doubles is not precise. Hence, a derivation value is used to 
 * define a certain derivation which is allowed. So, for two Rational values P and Q, P == Q if P > Q-deriv AND P < Q+deriv.<p>
 * By default, the value for deriv is set to 0.0000000001.<p>
 * There is only one double (numerator) field inside of this class which stores the Rational. The denominator is supposed to be 1 always.
 */
public class RationalDouble extends Rational implements Serializable{

    /*
     * members
     */
    static final RationalDouble deriv = new RationalDouble(0.0000000001); //allowed derivation for comparisons to be equal
    static final double DERIV_DOUBLE = 0.00000001;
    static final double DERIV_DOUBLE_NEG = -0.00000001;
    protected static boolean PRECISE = false;
    private double d;	//numerator
    
    /*
     * constructors
     */
    /**
     * Constructs a new Rational from n.
     *
     * @param n the numerator
     */
    public RationalDouble(int n) { this.d = (double)n; }
   

    /**
     * Constructs a new Rational from f.
     *
     * @param f the numerator
     */
    public RationalDouble(double f) { this.d = f; }
    

    /**
     * Constructs a new Rational from r.
     *
     * @param r the numerator
     */
    public RationalDouble(Rational r) { this.d = ((RationalDouble)r).d; }


    /**
     * Constructs a new Rational from numerator and denominator.
     * The numerator is set to num/den
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
     * @return the numerator as int
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
     * @return the denominator as int
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
     * Sets <i>this</i> to r.
     *
     * @param r the new Rational value r
     */
    public void assign (Rational r) { this.d = ((RationalDouble)r).d; }


    /**
     * Sets <i>this</i> to i.
     *
     * @param i the new Rational value i
     */
    public void assign (int i) { this.d = i; }

    
    /**
     * Sets <i>this</i> to d.
     *
     * @param d the new Rational value d
     */
    public void assign (double d) { this.d = d; }


    /**
     * Returns <i>this</i> * r.
     *
     * @param r the second factor
     * @return product of this and r
     */
    public Rational times (Rational r) { return new RationalDouble(this.d * ((RationalDouble)r).d); }
    

    /**
     * Returns <i>this</i> * i.
     *
     * @param i the second factor
     */
    public Rational times (int i) { return new RationalDouble(this.d * i); }


    /**
     * Returns <i>this</i> * r.
     * Stores the result in <i>in</i>.
     *
     * @param r the second factor
     * @param in the result is stored in this variable
     * @return this * r
     */
    public Rational times (Rational r, Rational in) {
	((RationalDouble)in).d = (this.d * ((RationalDouble)r).d);
	return in;
    }//end method times
  

    /**
     * Returns <i>this</i> : r.
     *
     * @param r the divisor
     * @return this : r
     */
    public Rational dividedby (Rational r) { return new RationalDouble(this.d / ((RationalDouble)r).d); }
    

    /**
     * Returns <i>this</i> : i.
     *
     * @param i the divisor
     * @return this : i
     */
    public Rational dividedby (int i) { return new RationalDouble(this.d / i); }


    /**
     * Returns <i>this</i> : r.
     * The result is stored in <i>in</i>.
     *
     * @param r the divisor
     * @param in the result is stored in this variable
     * @return this : r
     */
    public Rational dividedby (Rational r, Rational in) {
	((RationalDouble)in).d = (this.d / ((RationalDouble)r).d);
	return in;
    }


    /**
     * Returns <i>this</i> + r.
     *
     * @param r the summand
     * @return this + r
     */
    public Rational plus (Rational r) { return new RationalDouble(this.d + ((RationalDouble)r).d); }


    /**
     * Returns <i>this</i> + i.
     *
     * @param i the summand
     * @return this + i
     */
    public Rational plus (int i) { return new RationalDouble(this.d + i); }
    

    /**
     * Returns <i>this</i> + r.
     * The result is stored in in.
     *
     * @param r the summand
     * @param in the result is stored in this variable
     * @return this + r
     */
    public Rational plus (Rational r, Rational in) {
	((RationalDouble)in).d = (this.d + ((RationalDouble)r).d);
	return in;
    }//end method plus


    /**
     * Returns <i>this</i> - r.
     *
     * @param r the minuend
     * @return this - r
     */
    public Rational minus (Rational r) { return new RationalDouble(this.d - ((RationalDouble)r).d); }
    
    
    /**
     * Returns <i>this</i> - i.
     *
     * @param i the minuend
     * @return this - i
     */
    public Rational minus (int i) { return new RationalDouble(this.d - i); }
    

    /**
     * Returns <i>this</i> - r.
     * The result is stored in the variable <i>in</i>.
     *
     * @param r the minuend
     * @param in the result is stored in this variable
     * @return this - r
     */
    public Rational minus (Rational r, Rational in) {
	((RationalDouble)in).d = (this.d - ((RationalDouble)r).d);
	return in;
    }//end method minus


    /**
     * Returns true, if <i>this</i> is less than r.
     *
     * @param r the Rational to compare with
     * @return true, if this < r
     */
    public boolean less (Rational r) {
	if (PRECISE) {
	    if (this.equal(r)) return false;
	    else return (this.d < ((RationalDouble)r).d);
	}//if
	else {
	    if ((this.d - ((RationalDouble)r).d < DERIV_DOUBLE) &&
		(this.d - ((RationalDouble)r).d > DERIV_DOUBLE_NEG))
		return false;
	    else return (this.d < ((RationalDouble)r).d);
	}//else
    }//end method less

    

    /**
     * Returns true, if <i>this</i> is less than i.
     *
     * @param i the int to compare with
     * @return true, if this < i
     */
    public boolean less (int i) { 
	if (PRECISE) {
	    if (this.equal(i)) return false;
	    return (this.d < i);
	}//if
	else {
	    if ((this.d - i < DERIV_DOUBLE) &&
		(this.d - i > DERIV_DOUBLE_NEG))
		return false;
	    else return (this.d < i);
	}//else
    }//end method less
    

    /**
     * Returns true, if <i>this</i> is equal to r.
     *
     * @param r the Rational to compare with
     * @return true, if this = r
     */
    public boolean equal (Rational r) {
	if (PRECISE) {
	    double erg = Math.abs(this.d - ((RationalDouble)r).d);
	    if (erg < deriv.d) return true;
	    else return false;
	}//if
	else {
	    if ((this.d - ((RationalDouble)r).d < DERIV_DOUBLE) &&
		(this.d - ((RationalDouble)r).d > DERIV_DOUBLE_NEG))
		return true;
	    else return false;
	}//else
    }//end method equal
	    
    
    /**
     * Returns true, if <i>this</i> is equal to i.
     *
     * @param i the int to compare with
     * @return true, if this = i
     */
    public boolean equal (int i) {
	if (PRECISE) {
	    double erg = Math.abs(this.d - i);
	    if (erg < deriv.d) return true;
	    else return false;
	}//if
	else {
	    if ((this.d - i < DERIV_DOUBLE) &&
		(this.d - i > DERIV_DOUBLE_NEG))
		return true;
	    else return false;
	}//else
    }//end method equal
    

    /**
     * Returns true, if <i>this</i> is greater than r.
     *
     * @param r the Rational to compare with
     * @return true, if this > r
     */
    public boolean greater (Rational r) { 
	if (PRECISE) {
	    if (this.equal(r)) return false;
	    else return (this.d > ((RationalDouble)r).d);
	}//if
	else {
	    if ((this.d - ((RationalDouble)r).d < DERIV_DOUBLE) &&
		(this.d - ((RationalDouble)r).d > DERIV_DOUBLE_NEG))
		return false;
	    else return (this.d > ((RationalDouble)r).d);
	}//else
    }//end method greater
    

     /**
      * Returns true, if <i>this</i> is greater than i.
      *
      * @param i the int to compare with
      * @return true, if this > i
      */
    public boolean greater (int i) { 
	if (PRECISE) {
	    if (this.equal(i)) return false;
	    return (this.d > i);
	}//if
	else {
	    if ((this.d - i < DERIV_DOUBLE) &&
		(this.d - i > DERIV_DOUBLE_NEG))
		return false;
	    else return (this.d > i);
	}//else
    }//end method greater
    
    
    /**
     * Compares <i>this</i> and r and returns one of {0, 1, -1}.
     * Returns 0, if <i>this</i> = r<p>
     * Returns -1, if <i>this</i> < r<p>
     * Returns 1 otherwise
     *
     * @param r the Rational to compare with
     * @return one of {0, 1, -1} as byte
     */
    public byte comp (Rational r) {
	if (this.equal(r)) return 0;
	if (this.d < ((RationalDouble)r).d) return -1;
	else return 1;
    }//end method comp
    

    /**
     * Returns true, if <i>this</i> <= r.
     *
     * @param r the Rational to compare with
     * @return true if this <= r
     */
    public boolean lessOrEqual (Rational r) {
	if (PRECISE)
	    return (this.equal(r) || this.less(r));
	else if (this.d - ((RationalDouble)r).d < DERIV_DOUBLE)
	    return true;
	return false;
    }//end method lessOrEqual
    

    /**
     * Returns true, if <i>this</i> >= r.
     *
     * @param r the Rational to compare with
     * @return true if this >= r
     */
    public boolean greaterOrEqual (Rational r) {
	if (PRECISE)
	    return (this.equal(r) || this.greater(r));
	else if (this.d - ((RationalDouble)r).d >= DERIV_DOUBLE_NEG)
	    return true;
	return false;
    }//end method greaterOrEqual
    
    
    /**
     * Returns <i>this</i> as int.
     *
     * @return this as int
     */
    public int getInt() { return (int)this.d; }


    /**
     * Returns <i>this</i> as double.
     *
     * @return this as double
     */
    public double getDouble() { return this.d; }


    /**
     * Converts <i>this</i> to a String.
     *
     * @return this as String
     */
    public String toString() { return String.valueOf(this.d); }
    

    /**
     * Returns a copy of this.
     *
     * @return the copy
     */
    public Rational copy() {return new RationalDouble(this); }
    

    /**
     * Returns the absolute value of <i>this</i>.
     *
     * @return |this|
     */
    public Rational abs() {
	if (this.d < 0) this.d = this.d * (-1);
	return this;
    }//end method abs


    /**
     * Rounds <i>this</i> to <i>i</i> digits.
     *
     * @param digits the number of digits
     * @throws WrongDigitValueException if 0 > digits > 9
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
     * a flag can be set to use the more or less precise version. PRECISE=true means, that the derivation value is automatically set to 0.
     *
     * @param precise PRECISE is set to this value
     */
    public void setPrecision (Boolean precise) {
	PRECISE = precise.booleanValue();
    }//end method setPrecision

}//end class RationalDouble
