//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

abstract public class Rational {
    //This interface determines which methods every class
    //providing numeric values for the algebra has to implement.
    //It is used mainly by RationalFactory.

    //members
    //a variable named deriv should be defined. It determines a derivation
    //value, which may be needed for equality check. May be zero.

    //constructors
    //The following constructors MUST be implemented (if class name is RationalClass):
    //public RationalClass (int i);
    //public RationalClass (double d);
    //public RationalClass (Rational r);
    //public RationalClass (int i1, int i2);

    //methods
    abstract public int getNumerator();

    abstract public int getDenominator();

    abstract public void assign(Rational r);
    
    abstract public void assign(int i);

    abstract public Rational times(Rational r);

    abstract public Rational times(int i);

    abstract public Rational dividedby (Rational r);

    abstract public Rational dividedby (int i);

    abstract public Rational plus(Rational r);
    
    abstract public Rational plus(int i);

    abstract public Rational minus(Rational r);

    abstract public Rational minus(int i);

    abstract public boolean less(Rational r);

    abstract public boolean less(int i);

    abstract public boolean equal(Rational r);

    abstract public boolean equal(int i);

    abstract public boolean greater(Rational r);

    abstract public boolean greater(int i);

    abstract public byte comp(Rational r);

    abstract public boolean lessOrEqual(Rational r);

    abstract public boolean greaterOrEqual(Rational r);

    abstract public int getInt();

    abstract public double getDouble();

    abstract public String toString();

    abstract public Rational copy();

    abstract public Rational abs();

    abstract public void round(int i);
    
}//end class Rational
