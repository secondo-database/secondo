import java.io.*;

class Rational implements Serializable {
    //this is actually a double implementation!!!

 
    //Members
    double d;	//numerator
    
    //Constructors
    Rational(int n) { this.d = (double)n; }
    
    Rational(double f) { this.d = f; }
    
    Rational(Rational r) { this.d = r.d; }

    Rational(int nom, int denom) {
	this.d = (double)nom/(double)denom;
    }
    
    //Methods

    public int getNumerator() { 
	return (int)(d * 1000); 
    }

    public int getDenominator()  { 
	return 1000;
    }
    
    Rational times (Rational r) { return new Rational(this.d * r.d); }
    
    Rational times (int i) { return new Rational(this.d * i); }
    
    Rational dividedby (Rational r) { return new Rational(this.d / r.d); }
    
    Rational dividedby (int i) { return new Rational(this.d / i); }

    Rational plus (Rational r) { return new Rational(this.d + r.d); }

    Rational plus (int i) { return new Rational(this.d + i); }
    
    Rational minus (Rational r) { return new Rational(this.d - r.d); }
    
    Rational minus (int i) { return new Rational(this.d - i); }
    
    boolean less (Rational r) {
	if (this.equal(r)) return false;
	else return (this.d < r.d); }
    
    boolean less (int i) { 
	if (this.equal(i)) return false;
	return (this.d < i); }
    
    boolean equal (Rational r) {
	double erg = this.d - r.d;
	if ((-0.0000000001 < erg) &&
	    (erg < 0.0000000001)) return true;
	else return false; }
	//return (this.d == r.d); }
    
    boolean equal (int i) { 
	double erg = this.d - i;
	if ((-0.0000000001 < erg) &&
	    (erg < 0.0000000001)) return true;
	return (this.d == i); }
    
    boolean greater (Rational r) { 
	if (this.equal(r)) return false;
	else return (this.d > r.d); }
    
    boolean greater (int i) { 
	if (this.equal(i)) return false;
	return (this.d > i); }
    
    byte comp (Rational r) {
	if (this.equal(r)) return 0;
	if (this.d < r.d) return -1;
	else return 1; }
    
    boolean lessOrEqual (Rational r) {
	return (this.equal(r) || this.less(r));
    }//end method lessOrEqual
    
    boolean greaterOrEqual (Rational r) {
	return (this.equal(r) || this.greater(r));
    }//end method greaterOrEqual
    
    int getInt() { return (int)this.d; }
    double getDouble() { return this.d; }


    public String toString() { return String.valueOf(this.d); }
    
    public Rational copy() {return new Rational(this); }
    
    public Rational abs() {
	if (this.less(0)) { return this.times(-1); }
	else return this;
    }
}
