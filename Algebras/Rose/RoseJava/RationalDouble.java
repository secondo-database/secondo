import java.io.*;

public class RationalDouble extends Rational implements Serializable{
    //this is actually a double implementation!!!

    static final RationalDouble deriv = new RationalDouble(0.0000000001); //allowed derivation for comparisons to be equal
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

    public Rational times (Rational r) { return new RationalDouble(this.d * ((RationalDouble)r).d); }
    
    public Rational times (int i) { return new RationalDouble(this.d * i); }
  
    public Rational dividedby (Rational r) { return new RationalDouble(this.d / ((RationalDouble)r).d); }
    
    public Rational dividedby (int i) { return new RationalDouble(this.d / i); }

    public Rational plus (Rational r) { return new RationalDouble(this.d + ((RationalDouble)r).d); }

    public Rational plus (int i) { return new RationalDouble(this.d + i); }
    
    public Rational minus (Rational r) { return new RationalDouble(this.d - ((RationalDouble)r).d); }
    
    public Rational minus (int i) { return new RationalDouble(this.d - i); }
    
    public boolean less (Rational r) {
	if (this.equal(r)) return false;
	else return (this.d < ((RationalDouble)r).d); }
    
    public boolean less (int i) { 
	if (this.equal(i)) return false;
	return (this.d < i); }
    
    public boolean equal (Rational r) {
	//Rational erg = (this.minus(r)).abs();
	double erg = Math.abs(this.d - ((RationalDouble)r).d);
	//System.out.println("R.e: erg:"+erg);
	//if (erg.d < Algebra.deriv.d) return true;
	if (erg < deriv.d) return true;
	else return false; }
    
    public boolean equal (int i) { 
	//Rational erg = (this.minus(i)).abs();
	double erg = Math.abs(this.d - i);
	//if (erg.less(Algebra.deriv)) return true;
	if (erg < deriv.d) return true;
	//return (this.d == i); }
	else return false; }
    
    public boolean greater (Rational r) { 
	if (this.equal(r)) return false;
	else return (this.d > ((RationalDouble)r).d); }
    
    public boolean greater (int i) { 
	if (this.equal(i)) return false;
	return (this.d > i); }
    
    public byte comp (Rational r) {
	if (this.equal(r)) return 0;
	//if (this.d == r.d) return 0;
	if (this.d < ((RationalDouble)r).d) return -1;
	//if (this.less(r)) return -1;
	else return 1; }
    
    public boolean lessOrEqual (Rational r) {
	return (this.equal(r) || this.less(r));
    }//end method lessOrEqual
    
    public boolean greaterOrEqual (Rational r) {
	return (this.equal(r) || this.greater(r));
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
}
