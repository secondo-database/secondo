package twodsack.util.number;

import java.io.*;

public class RationalDouble extends Rational implements Serializable{
    //this is actually a double implementation!!!

    static final RationalDouble deriv = new RationalDouble(0.0000000001); //allowed derivation for comparisons to be equal
    static final double DERIV_DOUBLE = 0.00000001;
    static final double DERIV_DOUBLE_NEG = -0.00000001;
    protected static boolean PRECISE = false;

    //original value is 0.0000000001

    //Members
    //private static final Rational deriv = Algebra.deriv;

    private double d;	//numerator
    
    //Constructors
    public RationalDouble(int n) { this.d = (double)n; }
   
    public RationalDouble(double f) { this.d = f; }
    
    public RationalDouble(Rational r) { this.d = ((RationalDouble)r).d; }

    public RationalDouble(int num, int den) {
	//System.out.println("RD.const(i,i): num: "+num+", den: "+den);
	this.d = ((double)num/den);
	/*
	System.out.println("RD.const(i,i): this: "+this);
	System.out.println("RD.const(i,i): d: "+this.d);
	System.out.println("RD.const(i,i): numerator: "+this.getNumerator());
	System.out.println("RD.const(i,i): denominator: "+this.getDenominator());
	*/
    }
    
    //Methods

    public int getNumerator() {
	//returns the numerator as an int
	//NOTE: this is only for the Nestedlist conversion
	//we have a loss of precision here!
	//int dLength = String.valueOf(d).length();
	int dLength = String.valueOf((int)d).length();
	//System.out.println("dLength: "+dLength+" string: "+String.valueOf(d));
	int multiple = 0;
	if (dLength < 9) {
	    multiple = 9-dLength;
	    return (int)(d*Math.pow(10,multiple));
	}//if
	else return (int)d;
    }//end method getNumerator


    public int getDenominator() {
	//returns the denumerator as an int
	//the value of d is normally 1, but dependent of n
	//it may change. We try to preserve as many digits
	//after the dot as possible.
	//int dLength = String.valueOf(d).length();
	int dLength = String.valueOf((int)d).length();
	int multiple = 0;
	if (dLength < 9) {
	    multiple = 9-dLength;
	    return (int)(Math.pow(10,multiple));
	}//if
	else return 1;
    }//end method getDenominator
    
    public void assign (Rational r) { this.d = ((RationalDouble)r).d; }

    public void assign (int i) { this.d = i; }

    public void assign (double d) { this.d = d; }

    public Rational times (Rational r) { return new RationalDouble(this.d * ((RationalDouble)r).d); }
    
    public Rational times (int i) { return new RationalDouble(this.d * i); }

    public Rational times (Rational r, Rational in) {
	((RationalDouble)in).d = (this.d * ((RationalDouble)r).d);
	return in;
    }
  
    public Rational dividedby (Rational r) { return new RationalDouble(this.d / ((RationalDouble)r).d); }
    
    public Rational dividedby (int i) { return new RationalDouble(this.d / i); }

    public Rational dividedby (Rational r, Rational in) {
	((RationalDouble)in).d = (this.d / ((RationalDouble)r).d);
	return in;
    }

    public Rational plus (Rational r) { return new RationalDouble(this.d + ((RationalDouble)r).d); }

    public Rational plus (int i) { return new RationalDouble(this.d + i); }
    
    public Rational plus (Rational r, Rational in) {
	((RationalDouble)in).d = (this.d + ((RationalDouble)r).d);
	return in;
    }

    public Rational minus (Rational r) { return new RationalDouble(this.d - ((RationalDouble)r).d); }
    
    public Rational minus (int i) { return new RationalDouble(this.d - i); }
    
    public Rational minus (Rational r, Rational in) {
	((RationalDouble)in).d = (this.d - ((RationalDouble)r).d);
	return in;
    }

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
    
    
    public byte comp (Rational r) {
	if (this.equal(r)) return 0;
	if (this.d < ((RationalDouble)r).d) return -1;
	else return 1;
    }//end method comp
    
    public boolean lessOrEqual (Rational r) {
	if (PRECISE)
	    return (this.equal(r) || this.less(r));
	else if (this.d - ((RationalDouble)r).d < DERIV_DOUBLE)
	    return true;
	return false;
    }//end method lessOrEqual
    
    public boolean greaterOrEqual (Rational r) {
	if (PRECISE)
	    return (this.equal(r) || this.greater(r));
	else if (this.d - ((RationalDouble)r).d >= DERIV_DOUBLE_NEG)
	    return true;
	return false;
    }//end method greaterOrEqual
    
    public int getInt() { return (int)this.d; }
    public double getDouble() { return this.d; }
    
    public String toString() { return String.valueOf(this.d); }
    
    public Rational copy() {return new RationalDouble(this); }
    
    public Rational abs() {
	//if (this.d < 0) { return this.times(-1); }
	if (this.d < 0) this.d = this.d * (-1);
	return this;
    }//end method abs

    public void round (int digits) {
	//rounds this to the number of digits behind the point
	//digits must be a in a range from 0..9
	if (digits<0 || digits>9) 
	    throw new WrongDigitValueException("RationalDouble: Wrong value for digits. Must lie in range (0..9)!");
	int times = (int)Math.pow(10,digits);
	double temp = Math.round(this.d * times);
	this.d = temp/times;
    }//end method round

    public void setPrecision (Boolean precise) {
	PRECISE = precise.booleanValue();
    }//end method setPrecision

}//end class RationalDouble
