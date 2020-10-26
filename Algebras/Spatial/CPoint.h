/*
4 class CPoint

This class implements the memory representation of the ~cpoint~ type
constructor. A cpoint represents a point in the Euclidean plane with a radius
which can be considered as a tolerance value.

The type ~cpoint~ is applied for the types ~cupoint~ and ~cmpoint~.

*/

#ifndef CPOINT_H
#define CPOINT_H

#include <sstream>
#include "Point.h"

class CPoint: public Point {
  public:
/*
4.1 Constructors and Destructor

This constructor should not be used:

*/
   inline CPoint() {};

/*
There are different ways of constructing a cpoint:

The first one receives a boolean value ~d~ indicating if the point is defined,
two coordinate ~x~ and ~y~ values, and a radius ~r~.

*/
    explicit CPoint(const bool d, const Coord x = 0, const Coord y = 0,
                    const double r = 0.0);
    
    
    CPoint(const Point& _p, const double _r);
/*
The second one receives a cpoint ~cp~ as argument and creates a cpoint that is a
copy of ~cp~.

*/
    CPoint(const CPoint& p);
/*
The destructor.

*/
    inline ~CPoint() {}
/*
4.2 Member functions

Returns the ~x~-coordinate. For geographic coordinates: the LONgitude

*/
    inline const Coord& GetX() const {
      return ((Point*)this)->GetX();
    }
/*

Returns the ~y~-coordinate. For geographic coordinates: the LATitude

*/
    inline const Coord& GetY() const {
      return ((Point*)this)->GetY();
    }
/*
Returns the point's bounding box which is a rectangle with (almost)
 no extension.

*/
    const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;

    void Clear() {} // for compatibility with other spatial objects

/*
Sets the value of the point object.

*/
    inline void Set(const bool _defined, const Coord& _x, const Coord _y,
                    const double _r) {
      ((Point*)this)->Set(_defined, _x, _y);
      radius = _r;
    }
    
    void Set(const Coord& x, const Coord& y, const double r);
    void Set(const Point& p, const double r);
    void Set(const CPoint& p);
/*
Operators redefinition.

*/
    CPoint& operator=(const CPoint& cp);
    inline bool operator<=(const CPoint& cp) const;
    bool operator<(const CPoint& cp) const;
    inline bool operator>=(const CPoint& cp) const;
    bool operator>(const CPoint& cp) const;
    inline CPoint operator+(const CPoint& cp) const;
    inline CPoint operator-(const CPoint& cp) const;
    inline CPoint operator*(const double d) const;
/*
4.3 Operations

4.3.1 Operation ~scale~

*Precondition:* ~u.IsDefined()~

*Semantics:* $factor * u$

*Complexity:* $O(1)$

*/
    inline void Scale(const Coord& factor) {
      assert(IsDefined());
      ((Point*)this)->Scale(factor);
      radius *= factor;
    }
/*
4.3.1 Operation $=$ (~equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u = v$

*Complexity:* $O(1)$

*/
    inline bool operator==(const CPoint& cp) const;

/*
4.3.2 Operation $\neq$ (~not equal~)

*Precondition:* ~u.IsDefined() $\&\&$ v.IsDefined()~

*Semantics:* $u \neq v$

*Complexity:* $O(1)$Algebras/RTree/RTree.examples

*/
    inline bool operator!=(const CPoint& cp) const;

/*
4.3.8 Operation ~inside~ (with ~rectangle~)

*Precondition:* ~u.IsDefined()~

*Semantics:* $u \in V$

*Complexity:* $O(1)$

*/
  bool Inside(const Rectangle<2>& r, const Geoid* geoid = 0) const;

/*
4.3.13 Operation ~distance~ (with ~cpoint~)

*Precondition:* ~u.IsDefined()~ and ~v.IsDefined()~ and (~geoid == NULL~ or
~geoid.IsDefined()~

*Semantics:* Minimum estimation of distances.

*Complexity:* $O(1)$

*/
    double Distance(const CPoint& cp, const Geoid* geoid = 0) const;

/*
4.3.13 Operation ~distance~ (with ~rect2~)

*Precondition:* ~u.IsDefined()~ and ~V.IsOrdered()~ and (~geoid == NULL~ or
~geoid.IsDefined()~

*/
    double Distance(const Rectangle<2>& r, const Geoid* geoid = 0) const;


    bool Intersects(const Rectangle<2>& r, const Geoid* geoid = 0) const;


/*
4.1 Importz/Export to CSV files

*/

virtual std::string getCsvStr() const {
    if (!IsDefined()) {
      return "undef";    
    }
    std::stringstream ss;
    ss.precision(16);
    ss << "(" << ((Point*)this)->getCsvStr() << ", " << radius << ")";
    return ss.str();
 }

virtual void ReadFromString(std::string value);


/*
4.3.15 Operation ~translate~

This function moves the position of this CPoint object instance.

*/

    inline void Translate(const Coord& x, const Coord& y);
    inline void Translate(const Coord& x, const Coord& y, CPoint& result) const;

    inline void Scale(const Coord& sx, const Coord& sy, const double sr) {
      ((Point*)this)->Scale(sx, sy);
      radius *= sr;
    }
    inline void Scale(const Coord& sx, const Coord& sy, const double sr,
                      CPoint& result) const {
       result.Set(IsDefined(), x*sx, y*sy, radius*sr);
    }

   


/*
4.3.15 Operation ~rotate~
.
This function rotates this point around the cpoint defined by (x,y)
with a degree of alpha. The result is stored in res.

*/

   inline void Rotate(const Coord& x, const Coord& y, const double alpha,
                      CPoint& res) const;


/*
4.3.15 Operation ~add~

*Precondition:* ~cp1.IsDefined(), cp2.IsDefined()~

*Semantics:*  ~(cp1.x + cp2.x, cp1.y + cp2.y), max(cp1.radius, cp2.radius)~

*Complexity:* $O(1)$

*/
  inline CPoint Add(const CPoint& p, const Geoid* geoid = 0) const;

  inline bool IsEmpty() const {
    return ((Point*)this)->IsEmpty();
  }


/*
4.3.16 Operation ~toString~

Returns a textual representation of the cpoint. If a ~geoid~ is passed,
data is represented as geographic coordinates. Otherwise, a euclidean
coord-pair string is returned.

*/
  std::string toString(const Geoid* geoid = 0) const;

/*
4.3.16 Operation ~GeographicBBox~

Returns the MBR (bounding box) containing the shortest orthodrome path for THIS
Point and the ~other~ Point.

The Geographic MBR is different from the euclidean one, since the orthodrome may
exceed the two points extreme LATitude coordinates.

*/

  Rectangle<2> GeographicBBox(const CPoint &other, const Geoid &geoid) const;

/*
4.4 Functions needed to import the the ~cpoint~ data type to tuple

There are totally 8 functions which are defined as virtual functions. They need
to be defined here in order for the CPoint data type to be used in tuple 
definition as an attribute.

*/
    inline size_t Sizeof() const;

    inline size_t HashValue() const {
      return (size_t)(((Point*)this)->HashValue() * radius);
    }

    inline void CopyFrom(const Attribute* right) {
      const CPoint* cp = (const CPoint*)right;
      SetDefined(cp->IsDefined());
      if (IsDefined()) {
        Set(cp->x, cp->y, cp->radius);
      }
    }

    inline int Compare(const Attribute *arg) const {
      const CPoint* cp = (const CPoint*)arg;
      int pointCompare = ((Point*)this)->Compare((Point*)cp);
      if (pointCompare != 0) {
        return pointCompare;
      }
      if (radius == cp->radius) {
        return 0;
      }
      return radius > cp->radius ? 1 : -1;
    }

    inline int CompareAlmost(const Attribute *arg) const {
      const CPoint* cp = (const CPoint*)arg;
      int pointCompare = ((Point*)this)->CompareAlmost((Point*)cp);
      if (pointCompare != 0) {
        return pointCompare;
      }
      if (AlmostEqual(radius, cp->radius)) {
        return 0;
      }
      return radius > cp->radius ? 1 : -1;
    }

    inline bool Adjacent(const Attribute *arg) const {
      return ((Point*)this)->Adjacent(arg);
    }

    virtual inline CPoint* Clone() const {
      return new CPoint(*this);
    }

    std::ostream& Print(std::ostream &os) const;

    virtual bool hasBox() const {
       return ((Point*)this)->hasBox();
    }

    virtual double getMinX() const {
      return ((Point*)this)->getMinX();
    }
    
    virtual double getMaxX() const {
      return ((Point*)this)->getMaxX();
    }
    
    virtual double getMinY() const {
      return ((Point*)this)->getMinY();
    }
    
    virtual double getMaxY() const {
      return ((Point*)this)->getMaxY();
    }
    
    virtual double getRadius() const {
      return radius;
    }

    static const std::string BasicType() {
       return "cpoint";
    }
    static const bool checkType(const ListExpr type) {
      return listutils::isSymbol(type, BasicType());
    }

    static void* Cast(void* addr) {
      return (new (addr)CPoint());
    }

  protected:

/*
4.5 Attributes

*/
    double radius;
/*
The radius.

*/
};

/*
4.6 Auxiliary functions

*/
std::ostream& operator<<(std::ostream& o, const CPoint& p);

inline bool AlmostEqual(const CPoint& cp1, const CPoint& cp2) {
  return AlmostEqual(*((Point*)&cp1), *((Point*)&cp2)) &&
         AlmostEqual(cp1.getRadius(), cp2.getRadius());
}

/*
11 The inline functions

11.1 Class ~CPoint~

*/
inline CPoint::CPoint(const bool d, const Coord _x, const Coord _y, 
                      const double _r) :
  Point(d, _x, _y), radius(_r) {}

inline CPoint::CPoint(const Point& _p, const double _r) : 
  Point(_p), radius(_r) {}

inline CPoint::CPoint(const CPoint& cp) :
  Point(*((Point*)&cp)), radius(cp.getRadius()) {}


inline void CPoint::Set(const Coord& x, const Coord& y, const double r) {
  ((Point*)this)->Set(x, y);
  radius = r;
}

inline void CPoint::Set(const Point& p, const double r) {
  ((Point*)this)->Set(p);
  radius = r;
}

inline void CPoint::Set(const CPoint& cp) {
  ((Point*)this)->Set(*((Point*)&cp));
  radius = cp.getRadius();
}

inline CPoint CPoint::Add(const CPoint& cp, const Geoid* geoid) const {
  assert(IsDefined() && cp.IsDefined());
  if (!IsDefined() || !cp.IsDefined() || (geoid && !geoid->IsDefined())) {
    return CPoint(false, 0.0, 0.0, 0.0);
  }
  return CPoint(true, x + cp.x, y + cp.y, std::max(radius, cp.radius));
}

inline CPoint& CPoint::operator=(const CPoint& cp) {
  *((Point*)this) = (*((Point*)&cp));
  radius = cp.getRadius();
  return *this;
}

inline bool CPoint::operator==(const CPoint& cp) const {
  if (*((Point*)this) == (*((Point*)&cp))) {
    if (!IsDefined() && !cp.IsDefined()){
      return true;
    }
    return AlmostEqual(radius, cp.getRadius());
  }
  return false;
}

inline bool CPoint::operator!=(const CPoint& cp) const {
  return !(*this == cp);
}

inline bool CPoint::operator<=(const CPoint& cp) const {
  return !(*this > cp);
}

inline bool CPoint::operator<(const CPoint& cp) const {
  return *((Point*)this) < (*((Point*)&cp));
}

inline bool CPoint::operator>=(const CPoint& cp) const {
  return !(*this < cp);
}

inline bool CPoint::operator>(const CPoint& cp) const {
  return *((Point*)this) > (*((Point*)&cp));
}




inline CPoint CPoint::operator+(const CPoint& cp) const {
  return CPoint((IsDefined() && cp.IsDefined()), x + cp.x, y + cp.y,
                std::max(radius, cp.getRadius()));
}

inline CPoint CPoint::operator-(const CPoint& cp) const {
  return CPoint((IsDefined() && cp.IsDefined()), x - cp.x, y - cp.y,
                std::min(radius, cp.getRadius()));
}

inline CPoint CPoint::operator*(const double d) const {
  return CPoint(IsDefined(), x * d, y * d , radius * d);
}

inline size_t CPoint::Sizeof() const {
  return sizeof(*this);
}

inline void CPoint::Translate(const Coord& x, const Coord& y) {
  ((Point*)this)->Translate(x, y);
}

inline void CPoint::Translate(const Coord& tx, const Coord& ty, CPoint& res)
                                                                         const {
  res.Set(IsDefined(), x + tx, y + ty, radius);
}

#endif
