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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Header File for the classes ~Rational~ and ~SimpleRational~

[TOC]

1 Overview

This header file contains the classes ~Rational~ and ~SimpleRational~.

~Rational~ is a class to represent a rational number. Internally the ~Rational~
class uses an int (for the integer part) and two long long as numerator and 
denominator for the proper fraction part. 

~SimpleRational~ is only used inside the ~LineIntersection~ class to avoid the
performance penalty of the ~Rational~ normalization. If necessary 
a ~SimpleRational~ can be casted to a ~Rational~.

1 Defines and includes

*/

#pragma once

#include <stdexcept>
#include <limits>

namespace RobustPlaneSweep
{
/*

1 Class ~Rational~

*/
class Rational
{
/*

1.1 Member variables 

*/
private:
  int _integerPart;
  long long _numerator;
  long long _denominator;

/*

1.1 Private constructor

*/
  Rational(const int integerPart,
           const long long numerator,
           const long long denominator,
           const bool) :
      _integerPart(integerPart),
      _numerator(numerator),
      _denominator(denominator)
  {
  }

public:
/*

1.1 Constructors

*/
  explicit Rational(const int value) :
      _integerPart(value), _numerator(0), _denominator(1)
  {
  }

  Rational(const long long numerator, const long long denominator)
  {
    Init(0, numerator, denominator);
  }

  Rational(const int integerPart,
           const long long numerator,
           const long long denominator)
  {
    Init(integerPart, numerator, denominator);
  }

/*

1.1 Multiply operator

*/
  Rational operator *(const Rational& v) const
  {
    if (v._numerator == 0) {
      return *this * v._integerPart;
    } else if (_numerator == 0) {
      return v * _integerPart;
    } else {
      throw new std::invalid_argument("v2");
    }
  }

  Rational operator *(const int v) const
  {
    if (v == 0) {
      return Rational(0);
    } else if (v == 1) {
      return *this;
    }

    if (_numerator == 0) {
      return Rational(_integerPart * v);
    }

    bool negateResult = false;
    int factor = v;
    if (factor < 0) {
      factor = -factor;
      negateResult = true;
    }

    if (_numerator < std::numeric_limits<int>::max()
        && factor < std::numeric_limits<int>::max()) {
      long long m = _numerator * factor;
      long long newNum = m % _denominator;
      int intPart = (int)(m / _denominator);
      intPart += _integerPart * factor;

      Rational result(intPart, newNum, _denominator);

      if (negateResult) {
        result = -result;
      }
      return result;
    } else {
      int intAdd = 0;
      long long newNumerator = 0;

      long long a = _numerator;
      int d = factor;

      while (a > 0 && d > 0) {
        long long c = _denominator / a;
        long long cm = _denominator % a;
        if (cm != 0) {
          cm = a - cm;
          ++c;
        }

        long long dcm;
        int dc = (int)(d / c);
        dcm = d % c;

        intAdd += dc;
        newNumerator += dcm * a;
        if (newNumerator >= _denominator) {
          ++intAdd;
          newNumerator -= _denominator;
        }

        d = dc;
        a = cm;
      }

      Rational result((int)(_integerPart * factor) + intAdd,
                      newNumerator,
                      (newNumerator != 0 ? _denominator : 1));

      if (negateResult) {
        result = -result;
      }

      return result;
    }
  }

/*

1.1 Unary negation operator

*/
  Rational operator -() const
  {
    if (_numerator == 0) {
      return Rational(-_integerPart,
                      0,
                      1,
                      true);
    } else {
      return Rational(-_integerPart - 1,
                      _denominator - _numerator,
                      _denominator,
                      true);
    }
  }

/*

1.1 Subtraction operator

*/
  Rational operator -(const int v2) const
  {
    Rational result = Rational(_integerPart - v2,
                               _numerator,
                               _denominator,
                               true);
    return result;
  }

/*

1.1 Addition operator

*/
  Rational operator +(const int v2) const
  {
    Rational result = Rational(_integerPart + v2,
                               _numerator,
                               _denominator,
                               true);
    return result;
  }

/*

1.1 Greater than operator

*/
  bool operator >(const Rational& y) const
  {
    return Compare(*this, y) > 0;
  }

  bool operator >(const int y) const
  {
    if (y == 1) {
      return (_integerPart == 1 && _numerator > 0) || _integerPart > 1;
    } else {
      return Compare(*this, y) > 0;
    }
  }

/*

1.1 Greater/Equal than operator

*/
  bool operator >=(const Rational& y) const
  {
    return Compare(*this, y) >= 0;
  }

  bool operator >=(const int y) const
  {
    return Compare(*this, y) >= 0;
  }

/*

1.1 Less than operator

*/
  bool operator <(const Rational& y) const
  {
    return Compare(*this, y) < 0;
  }

  bool operator <(const int y) const
  {
    if (y == 0) {
      return (_integerPart < 0);
    } else {
      return Compare(*this, y) < 0;
    }
  }

/*

1.1 Less/Equal than operator

*/
  bool operator <=(const Rational& y) const
  {
    return Compare(*this, y) <= 0;
  }

  bool operator <=(const int y) const
  {
    return Compare(*this, y) <= 0;
  }

/*

1.1 Equality operator

*/
  bool operator ==(const Rational& y) const
  {
    return Compare(*this, y) == 0;
  }

  bool operator ==(const int y) const
  {
    return _integerPart == y && _numerator == 0;
  }

/*

1.1 Not-Equality operator

*/
  bool operator !=(const Rational& y) const
  {
    return !(*this == y);
  }

  bool operator !=(const int y) const
  {
    return _integerPart != y || _numerator != 0;
  }

/*

1.1 ~GetIntegerPart~

*/
  int GetIntegerPart() const
  {
    return _integerPart;
  }

/*

1.1 ~Compare~

*/
  static int Compare(const Rational& x, const int y)
  {
    if (x._integerPart < y) {
      return -1;
    } else if (x._integerPart > y || x._numerator > 0) {
      return 1;
    } else {
      return 0;
    }
  }

  static int Compare(const Rational& x, const Rational& y)
  {
    if (x._integerPart < y._integerPart) {
      return -1;
    } else if (x._integerPart > y._integerPart) {
      return 1;
    } else {
      if (x._denominator == y._denominator) {
        if (x._numerator == y._numerator) {
          return 0;
        } else if (x._numerator < y._numerator) {
          return -1;
        } else {
          return 1;
        }
      } else if (x._numerator == 0 && y._numerator > 0) {
        return -1;
      } else if (x._numerator > 0 && y._numerator == 0) {
        return 1;
      } else {
        long long n1 = x._denominator;
        long long d1 = x._numerator;
        long long n2 = y._denominator;
        long long d2 = y._numerator;
        int result = 1;

        if (n1 < std::numeric_limits<int>::max()
            && d1 < std::numeric_limits<int>::max()
            && n2 < std::numeric_limits<int>::max()
            && d2 < std::numeric_limits<int>::max()) {
          long long t1 = (d1 * n2);
          long long t2 = (d2 * n1);
          return (t1 < t2 ? -1 : (t1 > t2 ? 1 : 0));
        }

        for (;;) {
          long long div1 = n1 / d1, rem1 = n1 % d1;
          long long div2 = n2 / d2, rem2 = n2 % d2;
          if (div1 < div2) {
            return result;
          } else if (div1 > div2) {
            return -result;
          }

          if (rem1 == 0 && rem2 > 0) {
            return result;
          } else if (rem1 > 0 && rem2 == 0) {
            return -result;
          } else if (rem1 == 0 && rem2 == 0) {
            return 0;
          }

          n1 = d1;
          d1 = rem1;
          n2 = d2;
          d2 = rem2;
          result = -result;
        }
      }
    }
  }

/*

1.1 ~Max~

*/
  static const Rational Max(const Rational& x, const Rational& y)
  {
    if (x > y) {
      return x;
    } else {
      return y;
    }
  }

/*

1.1 ~Min~

*/
  static const Rational Min(const Rational& x, const Rational& y)
  {
    if (x < y) {
      return x;
    } else {
      return y;
    }
  }

/*

1.1 ~Round~

*/
  static int Round(const Rational& number, const int step)
  {
    if (step == 1) {
      if (number._numerator == 0) {
        return number._integerPart;
      }

      long long d = number._denominator - number._numerator;
      if (d < number._numerator
          || (number._integerPart >= 0 && d == number._numerator)) {
        return number._integerPart + 1;
      } else {
        return number._integerPart;
      }
    } else {
      int intPart = number._integerPart;
      int mod = intPart % step;
      intPart = intPart - mod + (mod * 2 >= step ? step : 0);
      return intPart;
    }
  }

private:
/*

1.1 ~Init~

*/
  void Init(const int integerPart,
            const long long numerator,
            const long long denominator)
  {
    if (denominator == 0) {
      throw new std::invalid_argument("denominator");
    }

    _integerPart = integerPart;

    if (numerator == 0) {
      _numerator = 0;
      _denominator = 1;
    } else {
      long long num = numerator;
      long long den = denominator;

      if (den < 0) {
        den = -den;
        num = -num;
      }

      bool numeratorNegative;
      if (num < 0) {
        num = -num;
        numeratorNegative = true;
      } else {
        numeratorNegative = false;
      }

      if (num >= den) {
        int temp = (int)(num / den);
        _numerator = num % den;
        _integerPart =
            (numeratorNegative ? _integerPart - temp : _integerPart + temp);
        _denominator = den;
      } else {
        _denominator = den;
        _numerator = num;
      }

      if (numeratorNegative && _numerator > 0) {
        _numerator = _denominator - _numerator;
        --_integerPart;
      }

      if (_numerator == 0) {
        _denominator = 1;
      }
    }
  }
};

/*

1 Class ~SimpleRational~

*/
class SimpleRational
{
/*

1.1 Member variables

*/
private:
  long long _numerator;
  long long _denominator;

public:
/*

1.1 Constructor

*/
  SimpleRational(const long long numerator, const long long denominator)
  {
    if (denominator == 0) {
      throw new std::invalid_argument("denominator");
    }

    if (denominator < 0) {
      _numerator = -numerator;
      _denominator = -denominator;
    } else {
      _numerator = numerator;
      _denominator = denominator;
    }
  }

/*

1.1 Greater than operator

*/
  bool operator>(const SimpleRational &y)
  {
    if (_denominator == y._denominator) {
      return _numerator > y._numerator;
    } else {
      throw new std::invalid_argument("y");
    }
  }

  bool operator>(int y)
  {
    if (y == 1) {
      return _numerator > _denominator;
    } else {
      throw new std::invalid_argument("y");
    }
  }
/*

1.1 Less than operator

*/
  bool operator<(const SimpleRational &y)
  {
    if (_denominator == y._denominator) {
      return _numerator < y._numerator;
    } else {
      throw new std::invalid_argument("y");
    }
  }

  bool operator<(int y)
  {
    if (y == 0) {
      return _numerator < 0;
    } else {
      throw new std::invalid_argument("y");
    }
  }

/*

1.1 Equality operator

*/
  bool operator==(int y)
  {
    if (y == 0) {
      return _numerator == 0;
    } else if (y == 1) {
      return _numerator == _denominator;
    } else {
      throw new std::invalid_argument("y");
    }
  }

  bool operator==(const SimpleRational &y)
  {
    if (_denominator == y._denominator) {
      return _numerator == y._numerator;
    } else if (_numerator == 0) {
      return y._numerator == 0;
    } else if (y._numerator == 0) {
      return _numerator == 0;
    } else if (_numerator == _denominator) {
      return y._numerator == y._denominator;
    } else if (y._numerator == y._denominator) {
      return _numerator == _denominator;
    } else {
      throw new std::invalid_argument("y");
    }
  }

/*

1.1 Cast to ~Rational~ operator

*/
  operator Rational()
  {
    return Rational(_numerator, _denominator);
  }
};
}
