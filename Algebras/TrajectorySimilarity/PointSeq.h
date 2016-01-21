/*
----
This file is part of SECONDO.

Copyright (C) 2015, University in Hagen, Department of Computer Science,
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
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]

[1] Interface of the Point Sequence Classes

\tableofcontents

\noindent

1 Introduction

This file primarily declares the sequence classes ~PointSeq~ and ~TPointSeq~.
They provide the type constructors ~pointseq~ and ~tpointseq~ for point
sequences, respectively.

Common functionality of the two sequence classes ~PointSeq~ and ~TPointSeq~ is
provided by the class template ~Sequence$<$T$>$~ with the item type ~T~. The
classes ~Point~ and ~TPoint~ serve as item types of the sequence. A ~Point~
represents a 2-dimensional point, and a ~TPoint~ represents a tuple of an
instant and a 2-dimensional point.

The sequence types ~PointSeq~ and ~TPointSeq~ are derived of instantiations of
the class template ~Sequence$<$T$>$~ with item type ~Point~ and ~TPoint~,
respectively.


2 Includes, Constants, Forward and Using Declarations

*/

#ifndef __POINT_SEQ_H__
#define __POINT_SEQ_H__

#include "Attribute.h"
#include "DateTime.h"   // Instant
#include "../../Tools/Flob/DbArray.h"

#include <cmath>


class DLine;
class Geoid;
class Point;

namespace temporalalgebra {
  class MPoint;
};


namespace tsa {

class TPoint;
class TPointSeq;

template<class T>
class Segment;

using temporalalgebra::MPoint;


/*
3 Declaration of Class Template ~Sequence$<$T$>$~

This class template provides common functionality for the sequence classes
~PointSeq~ and ~TPointSeq~.

An object of the class ~Sequence$<$T$>$~ with item type ~T~ is either
~undefined~ or it consists of zero or more items of type ~T~. The sequence does
not contain undefined items. The class ~Sequence$<$T$>$~ derives from the class
~Attribute~ and uses ~DbArray$<$T$>$~ to maintain the storage of its items.

*/

template<class T>
class Sequence : public Attribute
{
public:
/*
3.1 Types

The point type of the sequence.

*/
  using point_t = T;

/*
3.2 Constructors, Destructor

The default constructor. It is used only the the Cast() function and must be
empty.

*/
  inline Sequence() { }

/*
Constructor to create either an undefined or a defined, but empty sequence.

*/
  Sequence(bool defined) : Attribute(defined), seq(0) { }

/*
Constructor to create a defined, empty sequence with an initial capacity as
specified by ~cap~. The capacity grows, if more items are added to the sequence.

*/
  Sequence(size_t cap) : Attribute(true), seq(cap) { }

/*
Constructor to create a sequence from an ~NList~. The list expression
$({\tt undefined})$ yields an undefined sequence. A list expression of the form
$((item_1)\ (item_2)\ ...\ (item_n))$ with valid list expressions $item_i$
matching type ~T~ yields a sequence with $n$ items. Any other list expression
results in a ~std::domain\_error~ exception.

*/
  Sequence(const NList& list) noexcept(false);

/*
Copy constructor.

*/
  Sequence(const Sequence& rhs);

  ~Sequence() { }

/*
3.3 Conversion

Create a ~DLine~ from the sequence.

*/
  void toDLine(DLine& dline) const;

/*
Discard the current content of the sequence and recreate it from an ~MPoint~.

*/
  void convertFrom(const MPoint& src) noexcept;

/*
Discard the current content of the sequence and recreate it from a ~TPointSeq~
with sampling.

*/
  void sample(
      const TPointSeq& src, const datetime::DateTime& duration,
      bool keep_end_point = false, bool exact_path = false);

/*
3.4 Access

The following methods provide access to the items of the sequence.

Get the number of items in the sequence. Returns 0, if the sequence is
~undefined~.

*/
  size_t GetNoComponents() const
  { return IsDefined() ? seq.Size() : 0; }

/*
Get (a copy of) the item at position ~pos~ from the sequence, where
$0 \le pos < size()$.

*/
  T get(const size_t pos) const;

/*
Copy ~item~ to position ~pos~ of the sequence, where $0 \le pos \le
GetNoComponents()$ and $pos = 0$ identifies the first position. If $pos =
GetNoComponents()$, the sequence grows by one item to $GetNoComponents() =
pos+1$. Otherwise an existing item is replaced and the size of the sequence
remains unchanged.

*/
  void set(const size_t pos, const T& item);

/*
Copy ~item~ as a new item to the end of the sequence. Same as
$set(GetNoComponents(),\ item)$.

*/
  void append(const T& item);

/*
3.5 Type Constructor Interface

*/
  static ListExpr Out(ListExpr typeInfo, Word value);

  static Word In(
      const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct);

/*
The implementation of the $Create()$ function provided by the class template
~ConstructorFunctions$<$T$>$~ cannot be used here, since it calls the default
constructor that does not initialize its member variables (as required for the
$Cast()$ function to work properly). This causes trouble on object destruction
(when $free()$ is called on uninitialized pointers). Therefore the present
implementation calls a constructor that correctly initializes the member
variables.

*/
  static Word Create(const ListExpr typeInfo)
  { return SetWord(new Sequence<T>(/*defined*/ false)); }

  static bool KindCheck(const ListExpr type, ListExpr& errorInfo)
  { return NList(type) == NList(Sequence<T>::BasicType()); }


/*
3.6 ~Attribute~ Interface

*/
  inline int NumOfFLOBs() const override
  { return 1; }

  inline Flob* GetFLOB(const int i) override
  {
    assert(i >= 0 && i < NumOfFLOBs());
    return &seq;
  }

  inline std::ostream& Print(std::ostream& os) const override
  {
    os << Sequence<T>::BasicType() << ": (";
    for (size_t i = 0; i < GetNoComponents(); ++i) {
      if (i != 0)
        os << ", ";
      os << get(i).toString();
    }
    os << ")";
    return os;
  }

  size_t Sizeof() const override
  { return sizeof(Sequence); }

  int Compare(const Attribute* rhs) const override;

/*
The coordinates of the classes ~Point~ and ~TPoint~ (see below) and the instant
of the class ~TPoint~ are considered continuous values, in spite their discrete
representations. Therefore two sequences of such items are never adjacent.

*/
  bool Adjacent(const Attribute* /*attrib*/) const override
  { return false; }

  Attribute *Clone() const override
  { return new Sequence<T>(*this); }

  size_t HashValue() const override;

  void CopyFrom(const Attribute* rhs) override;

  inline static const std::string BasicType()
  { return T::typeName() + "seq"; }

private:
  void destroy()
  { seq.Destroy(); }

/*
The ~DbArray$<$T$>$~ ~seq~ maintains the storage of the actual sequence of
items.

*/
protected:
  DbArray<T> seq;
};



/*
4 Declaration of Class ~Point~

An object of this class represents a 2-dimensional point with real-valued,
finite coordinates ~x~ and ~y~. A ~Point~ cannot be ~undefined~.

*/
class Point
{

/*
4.1 Constructors

The default constructor. In general, the class does not allow to create a point
with non-finite coordinates. But to allow ~Point~ to be used as the template
parameter of the class ~DbArray$<$T$>$~, the default constructor intentionally
does not define the data members. This allows to create points with undefined
coordinates. To prevent misuse, access to the default constructor is limited to
few classes. It is the responsibility of the default constructor's user to
prevent a point with undefined coordinates from leaving the user's scope.

*/
private:
  Point() = default;

  friend class DbArray<Point>;
  friend class Segment<Point>;
  friend class Sequence<Point>;
  friend class TPoint;

public:

/*
Constructor to create a point with the given finite coordinates $x$ and $y$.
Specifying a non-finite value for any of the coordinates is an error and causes
program termination.

*/
  Point(double x, double y) : x(x), y(y) { assert(isValid()); }

/*
Constructor to create a point from an ~NList~. A list expression of the form
$(x\ y)$ with finite coordinates $x$ and $y$ yields a valid point.  Any other
list expression yields a ~std::domain\_error~ exception.

*/
  Point(const NList& list) noexcept(false);

/*
Constructor to create a point from a ~TPoint~ object. The coordinates are copied
and the instant is ignored.

*/
  Point(const TPoint& src);

/*
Constructor to create a point from a defined ~Point~ object of the
SpatialAlgebra.

*/
  Point(const ::Point& src);

/*
Constructor to create a point from an ~Instant~ and a defined ~Point~ object of
the SpatialAlgebra. The instant is ignored. The purpose of this constructor is
to provide the same interface for ~Point~ and ~TPoint~ to template functions
like ~Sequence$<$T$>$::convertFrom()~.

*/
  Point(const Instant& /*t*/, const Point& p) : Point(p)
  { assert(isValid()); }

/*
Copy constructor.

*/
  Point(const Point& rhs) = default;


/*
4.2 Conversion

Get the ~NList~ representation of the point.

*/
  NList toNList() const
  { return NList(NList(x), NList(y)); }

/*
Get a string representation for display purposes.

*/
  std::string toString() const;

/*
Get the point as a ~Point~ of the SpatialAlgebra.

*/
  ::Point toPoint() const;


/*
4.3 Access

Check if the point is valid. This is the case if both coordinates are finite.

*/
  inline bool isValid() const
  { return std::isfinite(x) && std::isfinite(y); }

/*
Get the ~x~ or ~y~ coordinate, respectively.

*/
  double getX() const { return x; }
  double getY() const { return y; }


/*
4.4 Comparison

Verify that the point ~[*]this~ may appear as the successor of the point ~prec~
(for ~preceding~) in a sequence of points. Throw a ~std::domain\_error~
exception otherwise. Used by ~Sequence$<$T$>$::in()~. Such restrictions apply
only to $TPoint$s, but not to $Point$s. Therefore this method is implemented
here only to provide a consistent interface to ~Sequence$<$T$>$~.

*/
  void validateSequenceOrder(const Point& prec) const noexcept(false) { }

/*
Compare the point ~[*]this~ to the point ~rhs~. Return -1 if ~[*]this~ is less
than ~rhs~, 0 if they are equal, and 1 if ~[*]this~ is greater than ~rhs~.

*/
  int compare(const Point& rhs) const;

/*
The usual comparison operators.

*/
  friend inline bool operator==(const Point& lhs, const Point& rhs)
  { return lhs.x == rhs.x && lhs.y == rhs.y; }

  friend inline bool operator!=(const Point& lhs, const Point& rhs)
  { return !(lhs == rhs); }

  friend inline bool operator<(const Point& lhs, const Point& rhs)
  { return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y); }

  friend inline bool operator>(const Point& lhs, const Point& rhs)
  { return rhs < lhs; }

  friend inline bool operator<=(const Point& lhs, const Point& rhs)
  { return !(lhs > rhs); }

  friend inline bool operator>=(const Point& lhs, const Point& rhs)
  { return !(lhs < rhs); }

/*
4.5 Operators

Arithmetic operations.

*/
  Point& operator+=(const Point& rhs) { x += rhs.x; y += rhs.y; return *this; }
  Point& operator-=(const Point& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
  Point& operator*=(const double v) { x *= v; y *= v; return *this; }
  Point& operator/=(const double v) { x /= v; y /= v; return *this; }

  friend Point operator+(Point lhs, const Point& rhs)
  { lhs += rhs; return lhs; }
  friend Point operator-(Point lhs, const Point& rhs)
  { lhs -= rhs; return lhs; }
  friend Point operator*(Point lhs, const double v)
  { lhs *= v; return lhs; }
  friend Point operator/(Point lhs, const double v)
  { lhs /= v; return lhs; }

/*
Euclidean ($L^2$) distance between two points.

*/
  friend inline double euclideanDistance(const Point& lhs, const Point& rhs)
  { return sqrt(pow(lhs.x - rhs.x, 2) + pow(lhs.y - rhs.y, 2)); }

/*
Euclidean distance between two points under consideration of a geoid. If ~geoid~
is ~nullptr~, this yields the same result as the preceding overload
$euclideanDistance(lhs, rhs)$, but with a bit of run-time overhead.

*/
  friend double euclideanDistance(
      const Point& lhs, const Point& rhs, const Geoid* geoid);

/*
Squared Euclidean distance between two points.

*/
  friend inline double sqrEuclideanDistance(const Point& lhs, const Point& rhs)
  { return pow(lhs.x - rhs.x, 2) + pow(lhs.y - rhs.y, 2); }

/*
Squared Euclidean distance between two points under consideration of a geoid. If
~geoid~ is ~nullptr~, this yields the same result as the preceding overload
$sqrEuclideanDistance(lhs, rhs)$, but with a bit of run-time overhead.

*/
  friend double sqrEuclideanDistance(
      const Point& lhs, const Point& rhs, const Geoid* geoid);

/*
$L^1$ distance between two points, also known as the taxicab distance or
Manhattan distance.

*/
  friend inline double l1Distance(const Point& lhs, const Point& rhs)
  { return std::min(fabs(lhs.x - rhs.x), fabs(lhs.y - rhs.y)); }


/*
4.6 ~Sequence~ Interface

These methods are used by the class ~Sequence$<$Point$>$~.

Get a hash value for the point.

*/
  size_t hash() const
  { return std::hash<double>()(x) ^ std::hash<double>()(y); }

/*
Get the name of the type.

*/
  inline static const std::string typeName()
  { return "point"; }


/*
4.7 Data Members

*/
private:
  double x;
  double y;
};



/*
5 Declaration of Class ~PointSeq~

An object of the class ~PointSeq~ is either ~undefined~ or it consists of zero
or more items of type ~Point~. ~PointSeq~ derives from ~Sequence$<$Point$>$~.

*/
class PointSeq : public Sequence<Point>
{
public:

/*
5.1 Conversion

Make ~covertFrom()~ of the base class visible for overload resolution.

*/
  using Sequence<Point>::convertFrom;

/*
Discard the current content of the sequence and recreate it from a ~TPointSeq~.

*/
  void convertFrom(const TPointSeq& src) noexcept;

};



/*
6 Declaration of Class ~TPoint~

An object of this class represents a tuple of a defined ~Instant~ and a
2-dimensional point with real-valued, finite coordinates ~x~ and ~y~. A ~TPoint~
cannot be ~undefined~.

*/
class TPoint
{

/*
6.1 Constructors

The default constructor. In general, the class does not allow to create an
object with an undefined instant or non-finite coordinates. But to allow
~TPoint~ to be used as the template parameter of the class ~DbArray$<$T$>$~, the
default constructor intentionally does not define the data members. This allows
to create objects with undefined instant and coordinates. To prevent misuse,
access to the default constructor is limited to few classes. It is the
responsibility of the default constructor's user to prevent an object with
undefined instant or coordinates from leaving the user's scope.

*/
private:
  TPoint() = default;

  friend class DbArray<TPoint>;
  friend class Segment<TPoint>;
  friend class Sequence<TPoint>;

public:

/*
Constructor to create an object with the given defined instant $i$ and finite
coordinates $x$ and $y$. Specifying an undefined instant or a non-finite value
for any of the coordinates is an error and causes program termination.

*/
  TPoint(const Instant& t, double x, double y) : instant(t), point(x, y)
  { assert(isValid()); }

/*
Constructor to create an object with the given defined instant $i$ and the valid
point $p$. Specifying an undefined instant or an invalid point is an error and
causes program termination.

*/
  TPoint(const Instant& t, const Point& p) : instant(t), point(p)
  { assert(isValid()); }

/*
Constructor to create an object from an ~NList~. A list expression of the form
$(inst\ (x\ y))$ with defined instant $inst$ and finite coordinates $x$ and $y$
yields a valid point. Any other list expression yields a ~std::domain\_error~
exception.

*/
  TPoint(const NList& list) noexcept(false);

/*
Copy constructor.

*/
  TPoint(const TPoint& rhs) = default;


/*
6.2 Conversion

Get the ~NList~ representation of the object.

*/
  NList toNList() const
  {
    return NList(
        NList(instant.ToListExpr(/*typeincluded*/ false)), point.toNList());
  }

/*
Get a string representation for display purposes.

*/
  std::string toString() const;

/*
Get the spatial projection of the point as a ~Point~ of the SpatialAlgebra.

*/
  ::Point toPoint() const;


/*
6.3 Access

Check if the object is valid. This is the case if the instant is defined and
both coordinates are finite.

*/
  inline bool isValid() const
  { return instant.IsDefined() && point.isValid(); }

/*
Get the instant, the point, or the ~x~ or ~y~ coordinate, respectively.

*/
  const Instant& getInstant() const { return instant; }
  const Point& getPoint() const { return point; }
  double getX() const { return point.getX(); }
  double getY() const { return point.getY(); }


/*
6.4 Comparison

Verify that the object ~[*]this~ may appear as the successor of the object
~prec~ (for ~preceding~) in a sequence of ~TPoint~s. This is the case if the
instant of ~[*]this~ is greater than the instant of ~prec~. Throw a
~std::domain\_error~ exception otherwise. Used by ~Sequence$<$T$>$::in()~.

*/
  void validateSequenceOrder(const TPoint& prec) const noexcept(false);

/*
Compare the object ~[*]this~ to the object object ~rhs~. Return -1 if ~[*]this~
is less than ~rhs~, 0 if they are equal, and 1 if ~[*]this~ is greater than
~rhs~.

*/
  int compare(const TPoint& rhs) const;

/*
The usual comparison operators.

*/
  friend inline bool operator==(const TPoint& lhs, const TPoint& rhs)
  { return lhs.instant == rhs.instant && lhs.point == rhs.point; }

  friend inline bool operator!=(const TPoint& lhs, const TPoint& rhs)
  { return !(lhs == rhs); }

  friend inline bool operator<(const TPoint& lhs, const TPoint& rhs)
  {
    return lhs.instant < rhs.instant
       || (lhs.instant == rhs.instant && lhs.point < rhs.point);
  }

  friend inline bool operator>(const TPoint& lhs, const TPoint& rhs)
  { return rhs < lhs; }

  friend inline bool operator<=(const TPoint& lhs, const TPoint& rhs)
  { return !(lhs > rhs); }

  friend inline bool operator>=(const TPoint& lhs, const TPoint& rhs)
  { return !(lhs < rhs); }


/*
6.5 ~Sequence~ Interface

These methods are used by the class ~Sequence$<$TPoint$>$~.

Get a hash value for the point.

*/
  size_t hash() const
  { return instant.HashValue() ^ point.hash(); }

/*
Get the name of the type.

*/
  inline static const std::string typeName()
  { return "tpoint"; }


/*
6.6 Data Members

*/
private:
  Instant instant;
  Point point;
};



/*
7 Declaration of Class ~TPointSeq~

An object of the class ~TPointSeq~ is either ~undefined~ or it consists of zero
or more items of type ~TPoint~ with instants in strictly increasing order.
~TPointSeq~ instantiates ~Sequence$<$T$>$~ with the item type $T=TPoint$.

*/
class TPointSeq : public Sequence<TPoint> { };

} //-- namespace tsa

#endif  //-- __POINT_SEQ_H__
