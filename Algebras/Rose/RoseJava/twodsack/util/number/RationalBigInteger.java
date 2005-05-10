package twodsack.util.number;

import java.io.*;
import java.math.*;

public class RationalBigInteger extends Rational implements Serializable{
    //this is a Rational implementation using BigInteger
    //numerator and denominater are both BigIntegers and may have arbitrary length
    //however, the Rational given by n/d must lie in the range of integer
    //otherwise an exception (RationalOverflowException) is thrown

    /*
     * If you want to change the number of digits for the Rational converter, change the value
     * NUM_DIGITS in the members section.
     * The derivation value for an allowed derivation can be set by setting a value for 'deriv'.
     */

    //Members
    static final int NUM_DIGITS = 7; //number of digits used right of the decimal point
    static final RationalBigInteger deriv = new RationalBigInteger(0,1);
    protected static boolean PRECISE = false;

    private BigInteger n;	//numerator
    private BigInteger d;	//denominator
    
    //Constructors
    
    private RationalBigInteger(BigInteger n, BigInteger d) throws DivisionByZeroException{
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
    
    public RationalBigInteger(int n) {
	this(new BigInteger(String.valueOf(n)), new BigInteger(String.valueOf(1)));}
    
    public RationalBigInteger(double f) {
	this(new BigInteger(String.valueOf((int) Math.round(f * 10000000))),new BigInteger(String.valueOf(10000000)));
	//System.out.println("possible loss of precision: "+f+" -> "+this);
    }
    //seven fraction digits used, e.g. 98.1234567
    
    public RationalBigInteger(Rational r) {
	this.n = ((RationalBigInteger)r).n;
	this.d = ((RationalBigInteger)r).d;
    }


    public RationalBigInteger(int nIn, int dIn) {
	this(new BigInteger(String.valueOf(nIn)), new BigInteger(String.valueOf(dIn))); }
    
    //Methods
    
    public int getNumerator() {
	//returns the numerator as an int
	//NOTE: this is only for the NestedList conversion
	//we have a loss of precision here!
	BigInteger hi = new BigInteger(String.valueOf(2147483647));
	BigInteger lo = new BigInteger(String.valueOf(-2147483648));
	if (n.compareTo(hi) == 1 || n.compareTo(lo) == -1 ||
	    d.compareTo(hi) == 1 || d.compareTo(lo) == -1) {
	    BigDecimal res = (new BigDecimal(n,NUM_DIGITS)).divide(new BigDecimal(d,NUM_DIGITS),BigDecimal.ROUND_UP);
	    //System.out.println("res: "+res);
	    res = res.movePointRight(NUM_DIGITS);
	    //System.out.println("res2: "+res);
	    return res.intValue();
	}//if
	else return n.intValue();
	//return (n.multiply(new BigInteger(String.valueOf(f)))).intValue();
    }//end method getNumerator


    public int getDenominator() {
	//returns the denominator as an int
	//NOTE: this is only for the NestedList conversion
	//we have a loss of precision here!
	BigInteger hi = new BigInteger(String.valueOf(2147483647));
	BigInteger lo = new BigInteger(String.valueOf(-2147483648));
	if (d.compareTo(hi) == 1 || d.compareTo(lo) == -1 ||
	    n.compareTo(hi) == 1 || n.compareTo(lo) == -1) {
	    return 1;
	}//if
	else return d.intValue();
	//return (d.multiply(new BigInteger(String.valueOf(f)))).intValue();
    }//end method getDenominator
    

    public void assign(Rational r) {
	n = ((RationalBigInteger)r).n;
	d = ((RationalBigInteger)r).d;
    }//end method assign
    
    public void assign(int i) {
	n = new BigInteger(String.valueOf(i));
	d = new BigInteger(String.valueOf(1));
    }//end method assign
    
    public void assign(double d) {
	this.n = new BigInteger(String.valueOf((int) Math.round(d * 10000000)));
	this.d = new BigInteger(String.valueOf(10000000));
    }

    public Rational times (Rational r) {
	//this.cancel();
	//r.cancel();
	return new RationalBigInteger(n.multiply(((RationalBigInteger)r).n),
				      d.multiply(((RationalBigInteger)r).d));
    }//end method times
    
    public Rational times (int i) {
	//this.cancel();
	return new RationalBigInteger(n.multiply(new BigInteger(String.valueOf(i))),d);
    }//end method times

    public Rational times (Rational r, Rational in) {
	((RationalBigInteger)in).n = n.multiply(((RationalBigInteger)r).n);
	((RationalBigInteger)in).d = d.multiply(((RationalBigInteger)r).d);
	return in;
    }
    
    public Rational dividedby (Rational r) {
	//this.cancel();
	//r.cancel();
	//this.cancel();
	//r.cancel();
	return new RationalBigInteger(n.multiply(((RationalBigInteger)r).d),
				      d.multiply(((RationalBigInteger)r).n));
    }//end method dividedby
    
    public Rational dividedby (int i) {
	//this.cancel();
	return new RationalBigInteger(n,d.multiply(new BigInteger(String.valueOf(i))));
    }//end method dividedby
    
    public Rational dividedby (Rational r, Rational in) {
	((RationalBigInteger)in).n = n.multiply(((RationalBigInteger)r).d);
	((RationalBigInteger)in).d = d.multiply(((RationalBigInteger)r).n);
	return in;
    }

    public Rational plus (Rational r) {
	//this.cancel();
	//r.cancel();
	return new RationalBigInteger(n.multiply(((RationalBigInteger)r).d).add(d.multiply(((RationalBigInteger)r).n)),d.multiply(((RationalBigInteger)r).d));
	//return new Rational(n * r.d + d * r.n, d * r.d);
    }//end method plus
    
    public Rational plus (int i) {
	//this.cancel();
	return new RationalBigInteger(n.add(d.multiply(new BigInteger(String.valueOf(i)))),d);
	//return new Rational(n + d * i, d);
    }//end method plus

    public Rational plus (Rational r, Rational in) {
	((RationalBigInteger)in).n = n.multiply(((RationalBigInteger)r).d).add(d.multiply(((RationalBigInteger)r).n));
	((RationalBigInteger)in).d = d.multiply(((RationalBigInteger)r).d);
	return in;
    }
    
    public Rational minus (Rational r) {
	//this.cancel();
	//r.cancel();
	return new RationalBigInteger(n.multiply(((RationalBigInteger)r).d).subtract(d.multiply(((RationalBigInteger)r).n)),d.multiply(((RationalBigInteger)r).d));
	//return new Rational(n * r.d - d * r.n, d * r.d);
    }//end method minus
    
    public Rational minus (int i) {
	//this.cancel();
	return new RationalBigInteger(n.subtract(d.multiply(new BigInteger(String.valueOf(i)))),d);
	//return new Rational(n - d * i, d);
    }//end method minus
    
    public Rational minus (Rational r, Rational in) {
	((RationalBigInteger)in).n = n.multiply(((RationalBigInteger)r).d).subtract(d.multiply(((RationalBigInteger)r).n));
	((RationalBigInteger)in).d = d.multiply(((RationalBigInteger)r).d);
	return in;
    }
    
    public boolean less (Rational r) {
	//this.cancel();
	//r.cancel();
	return (this.n.multiply(((RationalBigInteger)r).d).compareTo(this.d.multiply(((RationalBigInteger)r).n)) == -1);
	//return (n * r.d < d * r.n);
    }//end method less
    
    public boolean less (int i) {
	//this.cancel();
	return (n.compareTo(new BigInteger(String.valueOf(i)).multiply(d)) == -1);
	//return (n < i * d);
    }//end method less
    
    public boolean equal (Rational r) {
	return (n.multiply(((RationalBigInteger)r).d).equals(d.multiply(((RationalBigInteger)r).n)));
    }//end method equal
    
    public boolean equal (int i) {
	//this.cancel();
	return (n.equals(d.multiply(new BigInteger(String.valueOf(i)))));
	//return (n == i * d);
    }//end method equal
    
    public boolean greater (Rational r) {
	//this.cancel();
	//r.cancel();
	return (this.n.multiply(((RationalBigInteger)r).d).compareTo(this.d.multiply(((RationalBigInteger)r).n)) == 1);
	//return (n * r.d > d * r.n);
    }//end method greater
    
    public boolean greater (int i) {
	//this.cancel();
	return (n.compareTo(new BigInteger(String.valueOf(i)).multiply(d)) == 1);
	//return (n > i * d);
    }//end method greater
    
    public byte comp (Rational r) {
	BigInteger res1 = n.multiply(((RationalBigInteger)r).d);
	//long res1 = n * r.d;
	BigInteger res2 = d.multiply(((RationalBigInteger)r).n);
	//long res2 = d * r.n;
	return (byte)res1.compareTo(res2);
	//if (res1 < res2) return -1;
	//if (res1 == res2) return 0;
	//else return 1; 
    }//end method comp
    
    public boolean lessOrEqual (Rational r) {
	//this.cancel();
	//r.cancel();
	return (this.less(r) || this.equal(r));
    }//end method lessOrEqual
    
    public boolean greaterOrEqual (Rational r) {
	//this.cancel();
	//r.cancel();
	return (this.greater(r) || this.equal(r));
    }//end method greaterOrEqual
    
    public int getInt() {
	int res = (int)Math.round(this.n.doubleValue() / this.d.doubleValue());
	return res;
    }//end method getInt
    //int getInt() { return (new Long(n/d)).intValue(); }
    
    public double getDouble() {
	double res = this.n.doubleValue() / this.d.doubleValue();
	return res;
    }//end method getDouble
	// double getDouble() {return (double) n/d;}


    public String toString() {
	if (d.equals(BigInteger.ONE)) return String.valueOf(n); 
	if (n.divide(d).equals(BigInteger.ZERO)) {
	    return String.valueOf(n)+"/"+String.valueOf(d);}
	else {
	    byte res = (byte)n.compareTo(BigInteger.ZERO);
	    if (res >= 0) 
		return (n.divide(d).toString()+" "+n.divideAndRemainder(d)[1].toString()+
			"/"+d.toString());
	    //return (String.valueOf(n/d)+" "+String.valueOf(n%d)+
	    //	"/"+String.valueOf(d));
	    else {
		RationalBigInteger cop = (RationalBigInteger)this.abs();
		return ("-"+cop.n.divide(cop.d).toString()+" "+cop.n.divideAndRemainder(cop.d)[1].toString()+"/"+cop.d.toString());
		//return ("-"+String.valueOf(cop.n/cop.d)+" "+String.valueOf(cop.n%cop.d)+
		//"/"+String.valueOf(cop.d));
	    }//else
	    
	}//else
    }//end method toString

    protected static BigInteger gcd(BigInteger a, BigInteger b) {
	//protected static long gcd(long a, long b) {
	//greatest common divider
	if (b.equals(BigInteger.ZERO)) return a;
	//if (b == 0) return a;
	else return gcd(b, a.divideAndRemainder(b)[1]);
	//else return gcd(b, a%b);
    }//end method gcd
    
    protected void cancel(){
	//cancels down this
	//System.out.println("R.cancel: n:"+this.n+", d:"+this.d);
	//if ((d == 0) && (n == 0)) { return; }
	//if (this.d == 0) { System.out.println("Error: Division by 0 in Rational.");
	//System.exit(0); }
	
	//d.cancel();
	BigInteger f = gcd(this.n,this.d);
	//long f = gcd(this.n,this.d);
	this.n = this.n.divide(f);
	//this.n = this.n/f;
	this.d = this.d.divide(f);
	//this.d = this.d/f;
	if (this.d.compareTo(BigInteger.ZERO) == -1) {
	    //if (this.d < 0) {
	    this.d = this.d.multiply(new BigInteger(String.valueOf(-1)));
	    //this.d = this.d * -1;
	    this.n = this.n.multiply(new BigInteger(String.valueOf(-1)));
	    //this.n = this.n * -1;
	}//if
    }//end method cancel
    
    public Rational copy() {return new RationalBigInteger(this); }
    
    public Rational abs() {
	Rational retVal = this.copy();
	if (this.less(0)) {
	    retVal = this.times(new RationalBigInteger(-1));
	}//if
	return retVal;
    }//end method abs

    public void round (int digits) {
	//rounds this to the number of digits behind the point
	//digits must be in a range from 0..9
	if (digits<0 || digits>9)
	    throw new WrongDigitValueException("RationalBigInteger: Wrong value for digits. Must lie in range (0..9)!");
	double val = this.getDouble();
	int times = (int)Math.pow(10,digits);
	int temp = (int)Math.round(val * times);
	this.n = new BigInteger(String.valueOf(temp));
	this.d = new BigInteger(String.valueOf(times));
    }//end method round
	
    public void setPrecision (Boolean precise) {
	PRECISE = precise.booleanValue();
    }//end method setPrecision
    
}//end class RationalBigInteger
