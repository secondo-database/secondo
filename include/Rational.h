/*
//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]

[1] Package for Rational Numbers

February, 2002 Victor Teixeira de Almeida

1 Overview

This package essentially defines the class ~Rational~ to handle rational numbers.
The coordinate system of the ROSE Algebra is defined using rational numbers.

2 Defines and includes

*/
#ifndef __RATIONAL_H__
#define __RATIONAL_H__

#include <iostream>
#include <math.h>
#include <assert.h>
#include <limits.h>

/*
3 The class ~Rational~

*/

class Rational
{
  public:
/*
3.1 Constructors and Destructor

There are four ways of constructing a rational number:

*/
    Rational( const long n = 0L ) :
      num( n ),
      den( 1L )
      {}
/*
The first constructor takes a ~long int~ argument ~n~ to create an integer
rational which is represented by ~n/1~.

*/
    Rational( const long n, const long d);
/*
The second constructor takes two ~long int~ arguments ~n~ and ~d~ to create
a rational which is represented by ~n/d~.

*/
    Rational( const double f );
/*
The third constructor takes a ~double~ arguments ~f~ to create
a rational which is an aproximated value.

*/
    Rational( const Rational& r) :
      num( r.Numerator() ),
      den( r.Denominator() )
      {}
/*
The fourth and the last constructor takes a ~Rational~ argument and make
a copy of it.

*/
    ~Rational()
      {}
/*
The destructor.

3.2 Assignment operators

*/
    const Rational& operator= ( const Rational& r );
    const Rational& operator= ( const long r );
    const Rational& operator= ( const double r );

    const Rational& operator+=( const Rational& r );
    const Rational& operator+=( const long r );
    const Rational& operator+=( const double r );

    const Rational& operator-=( const Rational& r );
    const Rational& operator-=( const long r );
    const Rational& operator-=( const double r );

    const Rational& operator/=( const Rational& r );
    const Rational& operator/=( const long r );
    const Rational& operator/=( const double r );

    const Rational& operator*=( const Rational& r );
    const Rational& operator*=( const long r );
    const Rational& operator*=( const double r );
/*
3.3 Mathematical binary operators

*/
    Rational operator+( const Rational& r ) const;
    Rational operator+( const long r ) const;
    Rational operator+( const double r ) const;

    Rational operator-( const Rational& r ) const;
    Rational operator-( const long r ) const;
    Rational operator-( const double r ) const;

    Rational operator/( const Rational& r ) const;
    Rational operator/( const long r ) const;
    Rational operator/( const double r ) const;

    Rational operator*( const Rational& r ) const;
    Rational operator*( const long r ) const;
    Rational operator*( const double r ) const;
/*
3.4 Relational operators

*/
    int operator< ( const Rational& r ) const;
    int operator< ( const long r ) const;
    int operator< ( const double r ) const;

    int operator<=( const Rational& r ) const;
    int operator<=( const long r ) const;
    int operator<=( const double r ) const;

    int operator> ( const Rational& r ) const;
    int operator> ( const long r ) const;
    int operator> ( const double r ) const;

    int operator>=( const Rational& r ) const;
    int operator>=( const long r ) const;
    int operator>=( const double r ) const;

    int operator==( const Rational& r ) const;
    int operator==( const long r ) const;
    int operator==( const double r ) const;

    int operator!=( const Rational& r ) const;
    int operator!=( const long r ) const;
    int operator!=( const double r ) const;
/*
3.5 Unary operators

*/
    const Rational& operator++();            // Prefix
    Rational operator++( int );              // Postfix
    const Rational& operator--();            // Prefix
    Rational operator--( int );              // Postfix
    const Rational& operator+() const;
    Rational operator-() const;
    int operator!() const;
/*
3.6 Member functions

*/
    double Value() const
      { return double( num ) / double( den ); }
/*
Returns the value of the rational in real number (~double~) format.

*/
    long Numerator() const
      { return num; }
/*
Returns the numerator.

*/
    long Denominator() const
      { return den; }
/*
Returns the denominator.

*/
    int IsInteger() const
      { return den == 1L; }
/*
Checks if the rational number is an integer number, i.e., can be directly
converted to an integer number.

*/
    long IntValue() const
      { assert( IsInteger() ); return num; }
/*
Returns the value of the rational in integer number (~long~) format.

*Precondition*: $IsInteger() == true$

*/
  private:
/*
3.7 Private member functions

*/
    void FixSigns();
/*
This function ensures that the denominator has a positive sign, i.e.,
~den $\geq$ 0~.

*/
    void Reduce();
/*
This function ensures that the rational number is in its lowest form.
It divides both the numerator (~num~) and the donominator (~den~) by
their greatest common divisor (~gcd~).

3.8 Attributes

*/
    long num;   
/* 
The numerator

*/
    long den;   
/*
The denominator

*/
};

/*
4 Implementation of class ~Rational~

4.1 Auxiliary functions

This function ~Gcd1~ calculates the greatest common divisor
between two integer numbers ~n~ and ~m~.
*Precondition*  $m > 0$

*/
long Gcd1( const long n, const long m )
{
  assert( m > 0L );

  if( n % m == 0L )
    return m;
  else
    return Gcd1( m, n % m );
}
/*
This function ~Gcd~ is a generic function to calculate the greatest 
common divisor between two integer numbers ~n~ and ~m~.

*/
long Gcd( const long m, const long n )
{
  if( m > 0L )
    return Gcd1( n, m );
  else
    return Gcd1( n, -m );
}

/*
4.2 Constructors and Destructor

*/
Rational::Rational( const long n, const long d):
num( n ),
den( d )
{
  assert( d != 0L );
  FixSigns();
  Reduce();
}

Rational::Rational( const double f )
{
  if( f == (long)f )
    // if ~f~ is an integer.
  {
    num = (long)f;
    den = 1L;
  }
  else if( LONG_MAX < f )
    // We use the biggest representable rational.
  {
    num = LONG_MAX;
    den = 1L;
  }
  else if( LONG_MIN > f )
    // We use the smallest (more negative) representable rational.
  {
    num = LONG_MIN;
    den = 1L;
  }
  else
    // General cases: tries to get the better approximation.
  {
    double absValue;

    if( f >= 0L )
      absValue = f;
    else
      absValue = -f;

    if( absValue > 1L)
    {
      long factor = (long)(LONG_MAX / absValue);
      assert( factor >= 1L );

      num = (long)( f * factor + (double)0.5 );
      den = factor;
      Reduce();
    }
    else
    {
      num = (long)( f * LONG_MAX );
      den = LONG_MAX;
      Reduce();
    }
  }
}

/*
4.3 Assignment operators

*/
const Rational& Rational::operator=( const Rational& r )
{
  if( this != &r )
  {
    num = r.Numerator();
    den = r.Denominator();
  }
  return *this;
}

const Rational& Rational::operator=( const long r )
{
  Rational result( r );
  *this = result;

  return *this;
}

const Rational& Rational::operator=( const double r )
{
  Rational result( r );
  *this = result;

  return *this;
}

const Rational& Rational::operator+=( const Rational& r )
{
  num = num * r.Denominator() + r.Numerator() * den;
  den = den * r.Denominator();
  Reduce();

  return *this;
}

const Rational& Rational::operator+=( const long r )
{
  Rational result( r );
  *this += result;

  return *this;
}

const Rational& Rational::operator+=( const double r )
{
  Rational result( r );
  *this += result;

  return *this;
}

const Rational& Rational::operator-=( const Rational& r )
{
  num = num * r.Denominator() - r.Numerator() * den;
  den = den * r.Denominator();
  Reduce();

  return *this;
}

const Rational& Rational::operator-=( const long r )
{
  Rational result( r );
  *this -= result;

  return *this;
}

const Rational& Rational::operator-=( const double r )
{
  Rational result( r );
  *this -= result;

  return *this;
}

const Rational& Rational::operator*=( const Rational& r )
{
  num *= r.Numerator();
  den *= r.Denominator();
  Reduce();

  return *this;
}

const Rational& Rational::operator*=( const long r )
{
  Rational result( r );
  *this *= result;

  return *this;
}

const Rational& Rational::operator*=( const double r )
{
  Rational result( r );
  *this *= result;

  return *this;
}

const Rational &Rational::operator/=( const Rational& r )
{
  num *= r.Denominator();
  den *= r.Numerator();

  FixSigns();
  Reduce();

  return *this;
}

const Rational &Rational::operator/=( const long r )
{
  Rational result( r );
  *this /= result;

  return *this;
}

const Rational &Rational::operator/=( const double r )
{
  Rational result( r );
  *this /= result;

  return *this;
}
/*
4.4 Mathematical binary operators

*/

Rational Rational::operator+( const long r ) const
{
  Rational result( r );

  return *this + r;
}

Rational Rational::operator+( const double r ) const
{
  Rational result( r );

  return *this + r;
}

Rational Rational::operator-( const Rational& r ) const
{
  Rational result( *this );
  result -= r;

  return result;
}

Rational Rational::operator-( const long r ) const
{
  Rational result( r );

  return *this - r;
}

Rational Rational::operator-( const double r ) const
{
  Rational result( r );

  return *this - r;
}

Rational Rational::operator*( const Rational& r ) const
{
  Rational result( *this );
  result *= r;
  return result;
}

Rational Rational::operator*( const long r ) const
{
  Rational result( r );

  return *this * r;
}

Rational Rational::operator*( const double r ) const
{
  Rational result( r );

  return *this * r;
}

Rational Rational::operator/( const Rational& r ) const
{
  Rational result( *this );
  result /= r;
  return result;
}

Rational Rational::operator/( const long r ) const
{
  Rational result( r );

  return *this / r;
}

Rational Rational::operator/( const double r ) const
{
  Rational result( r );

  return *this / r;
}

/*
4.5 Relational operators

*/
int Rational::operator==( const Rational& r ) const
{
  return num * r.Denominator() == den * r.Numerator();
}

int Rational::operator==( const long r ) const
{
  Rational result( r );

  return *this == r;
}

int Rational::operator==( const double r ) const
{
  Rational result( r );

  return *this == r;
}

int Rational::operator!=( const Rational& r ) const
{
  return num * r.Denominator() != den * r.Numerator();
}

int Rational::operator!=( const long r ) const
{
  Rational result( r );

  return *this != r;
}

int Rational::operator!=( const double r ) const
{
  Rational result( r );

  return *this != r;
}

int Rational::operator<=( const Rational& r ) const
{
  return num * r.Denominator() <= den * r.Numerator();
}

int Rational::operator<=( const long r ) const
{
  Rational result( r );

  return *this <= r;
}

int Rational::operator<=( const double r ) const
{
  Rational result( r );

  return *this <= r;
}

int Rational::operator<( const Rational& r ) const
{
  return num * r.Denominator() < den * r.Numerator();
}

int Rational::operator<( const long r ) const
{
  Rational result( r );

  return *this < r;
}

int Rational::operator<( const double r ) const
{
  Rational result( r );

  return *this < r;
}

int Rational::operator>=( const Rational& r ) const
{
  return num * r.Denominator() >= den * r.Numerator();
}

int Rational::operator>=( const long r ) const
{
  Rational result( r );

  return *this >= r;
}

int Rational::operator>=( const double r ) const
{
  Rational result( r );

  return *this >= r;
}

int Rational::operator>( const Rational& r ) const
{
  return num * r.Denominator() > den * r.Numerator();
}

int Rational::operator>( const long r ) const
{
  Rational result( r );

  return *this > r;
}

int Rational::operator>( const double r ) const
{
  Rational result( r );

  return *this > r;
}

/*
4.6 Unary operators

*/
const Rational& Rational::operator++()
{
  num += den;

  return 


*this;
}

Rational Rational::operator++( int )
{
  Rational result = *this;
  num += den;

  return result;
}

const Rational& Rational::operator--()
{
  num -= den;

  return *this;
}

Rational Rational::operator--( int )
{
  Rational result = *this;
  num -= den;
  return result;
}

int Rational::operator!() const
{
  return !num;
}

const Rational& Rational::operator+() const
{
  return *this;
}

Rational Rational::operator-() const
{
  return Rational( -num, den );
}

ostream& operator<<( ostream& o, const Rational& r )
{
 o << r.Numerator();
 if( r.Denominator() != 1L )
   o << "/" << r.Denominator();

  return o;
}

/*
4.8 Private member functions

*/
void Rational::FixSigns()
{
  if( den < 0L )
  {
    den = -den;
    num = -num;
  }

}

void Rational::Reduce()
{
  long d = 1L;

  if( den != 0L && num != 0L )
    d = Gcd( num, den );

  if( d > 1L )
  {
    num /= d;
    den /= d;
  }
}

#endif // __RATIONAL_H__

