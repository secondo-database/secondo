/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] title: [{\Large \bf ]	[}]

[1] JNIExample Algebra -- RationalJNITest.java

Janary, 28th, 2003 Mirco G[ue]nster and Ismail Zerrad.

This class which represents a RationalJNITest number is used 
by the PointJNITest class. 

*/

class RationalJNITest {
    //Members

    int n;	//numerator
    int d;	//denominator

    //Constructors

    RationalJNITest(int n, int d) {
	if (d == 0) {
	    System.out.println("Error: Division by 0 in RationalJNITest.");
	    System.exit(0);
	}
	int f = gcd(n, d); 
	this.n = n/f; 
	this.d = d/f;
	if (this.d < 0) {
	    this.d = this.d * -1; 
	    this.n = this.n * -1;
	}
    }// end of constructor
    
    RationalJNITest(int n) {
	this(n, 1);
    }// end of constructor

    RationalJNITest(double f) {
	this((int) Math.round(f * 100), 100); // mal auf 1000 setzen.
    }// end of constructor
    //two fraction digits used, e.g. 17.25

    RationalJNITest(RationalJNITest r) {
	this.n = r.n; 
	this.d = r.d;
    }// end constructor

    //Methods

    void assign(RationalJNITest r) {
	n = r.n; 
	d = r.d;
    }// end method assign

    void assign(int i) {
	n = i; d = 1;
    }// end method assign

    RationalJNITest times (RationalJNITest r) {
	return new RationalJNITest(n * r.n, d * r.d);
    }// end method times

    RationalJNITest times (int i) {
	return new RationalJNITest(n * i, d);
    }// end method times

    RationalJNITest dividedby (RationalJNITest r) {
	return new RationalJNITest (n * r.d, d * r.n);
    }// end method dividedby

    RationalJNITest dividedby (int i) {
	return new RationalJNITest (n, d * i);
    }// end method dividedby
      
    RationalJNITest plus (RationalJNITest r) {
	return new RationalJNITest(n * r.d + d * r.n, d * r.d);
    }// end method plus

    RationalJNITest plus (int i) {
	return new RationalJNITest(n + d * i, d);
    }// end method plus

    RationalJNITest minus (RationalJNITest r) {
	return new RationalJNITest(n * r.d - d * r.n, d * r.d);
    }//end method minus
    
    RationalJNITest minus (int i) {
	return new RationalJNITest(n - d * i, d);
    }//end method minus

    boolean less (RationalJNITest r) {
	return (n * r.d < d * r.n);
    }// end method less

    boolean less (int i) {
	return (n < i * d);
    }// end method less
    
    boolean equal (RationalJNITest r) {
	return (n * r.d == d * r.n);
    }// end method equal

    boolean equal (int i) {
	return (n == i * d);
    }// end method equal
    
    boolean greater (RationalJNITest r) {
	return (n * r.d > d * r.n);
    }// end method greater

    boolean greater (int i) {
	return (n > i * d);
    }//end method greater
    
    boolean lessOrEqual (RationalJNITest r) {
	return (this.less(r) || this.equal(r)); 
    }//end method lessOrEqual

    boolean greaterOrEqual (RationalJNITest r) {
	return (this.greater(r) || this.equal(r)); 
    }//end method greaterOrEqual
    
    int getInt() {
	return (int) n/d;
    }//end method getInt

    double getDouble() {
	return (double) n/d;
    }//end method getDouble
    
    public String toString() {
	if (d == 1) return String.valueOf(n);
	if (n < d) {return String.valueOf(n)+"/"+String.valueOf(d);}
	else {
	    return (String.valueOf(n/d)+" "+String.valueOf(n%d)+
		    "/"+String.valueOf(d));
	}
    }//end method toString

    static int gcd(int a, int b) {
	if (b == 0) return a;
	else return gcd(b, a%b);
    }//end method gcd
    
    public RationalJNITest copy() {
	return new RationalJNITest(this.n,this.d); 
    }//end method copy
    
    public RationalJNITest abs() {
	RationalJNITest retVal = this.copy();
	if (this.less(0)) {
	    retVal = this.times(new RationalJNITest(-1));
	}//if
	return retVal;
    }//end method abs
}
