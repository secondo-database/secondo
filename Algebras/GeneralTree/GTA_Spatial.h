/*
\newpage

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

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

1.1 Headerfile "GTA[_]Spatial.h"[4]

January-May 2008, Mirko Dibbert

This file contains the folllowing classes:

  * "Spatial"[4] (base for "HPoint"[4] and "HRect"[4])

  * "HPoint"[4] (models hyper points)

  * "HRect"[4] (models hyper rects)

  * "SpatialDistfuns"[4] (distance functions for hpoint and hrect)

*/
#ifndef __GTA_SPATIAL_H__
#define __GTA_SPATIAL_H__

#include "GTA_Config.h"
#include "NestedList.h"
#include "ListUtils.h"
#include <assert.h>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <string>


namespace gta
{

// forward declarations:
class HRect;

/********************************************************************
1.1.1 Class ~Spatial~

********************************************************************/
class Spatial
{
  public:
/*
Default constructor (used from the 'read' constructors).

*/
    inline Spatial()
    {}

/*
Constructor.

*/
    inline Spatial(unsigned dim)
        : m_dim(dim),
          m_vectorlen(m_dim * sizeof(GTA_SPATIAL_DOM))
    {}

/*
Copy constructor.

*/
    inline Spatial(const Spatial &e)
        : m_dim(e.dim()),
          m_vectorlen(e.vectorlen())
    {}

/*
Destructor.

*/
    inline ~Spatial()
    {}

/*
Returns the dimension of the spatial object.

*/
    inline unsigned dim() const
    { return m_dim; }

/*
Returns the lengths of the coordinate vector arrays in bytes.

*/
    inline unsigned vectorlen() const
    { return m_vectorlen; }

  protected:
/*
Writes the spatial object to "buffer"[4] and increases "offset"[4].

*/
    void inline write(char *buffer, int &offset) const
    {
        memcpy(buffer+offset, &m_dim, sizeof(unsigned));
        offset += sizeof(unsigned);
    }

/*
Reads the spatial object from "buffer"[4] and increases "offset"[4].

*/
    void inline read(const char *buffer, int &offset)
    {
        memcpy(&m_dim, buffer+offset, sizeof(unsigned));
        offset += sizeof(unsigned);

        m_vectorlen = m_dim * sizeof(GTA_SPATIAL_DOM);
    }

/*
Returns the size of the spatial object on disc in bytes.

*/
    inline size_t size() const
    { return sizeof(unsigned); }

    unsigned m_dim; // dimension of the spatal object
    unsigned m_vectorlen; // length of a coordinate vector in bytes
}; // class Spatial



/********************************************************************
1.1.1 Class ~HPoint~

********************************************************************/
class HPoint
        : public Spatial
{
  public:
/*
Constructor.

*/
    inline HPoint(unsigned dim, const GTA_SPATIAL_DOM *coords)
        : Spatial(dim),
          m_coords(new GTA_SPATIAL_DOM[dim]) { 
         memcpy(m_coords, coords, vectorlen());
     }

/*
Default Copy constructor.

*/
    inline HPoint(const HPoint &e)
        : Spatial(e),
          m_coords(new GTA_SPATIAL_DOM[dim()])
    { memcpy(m_coords, e.m_coords, vectorlen()); }

/*
Constructor (reads the given hyper point from buffer and increases offset)

*/
    inline HPoint(const char *buffer, int &offset)
    { read(buffer, offset); }

/*
Destructor.

*/
    inline ~HPoint() { 
      delete [] m_coords; 
    }

/*
Assignment operator.

*/
    inline HPoint & operator=(const HPoint &rhs)
    {
        m_dim = rhs.dim();
        m_vectorlen = m_dim * sizeof(GTA_SPATIAL_DOM);
        delete[] m_coords;
        m_coords = new GTA_SPATIAL_DOM[dim()];
        memcpy(m_coords, rhs.m_coords, vectorlen());
        return *this;
    }

/*
Returns the coordinate vector of the hyper point.

*/
    inline GTA_SPATIAL_DOM *coords() const
    { return m_coords; }

/*
Returns the i-th coordinate of the hyper point.

*/
    inline GTA_SPATIAL_DOM coord(unsigned i) const
    {
        #ifdef __GTA_DEBUG
        assert(i < dim());
        #endif

        return m_coords[i];
    }

/*
Returns the eucledean distance between "this"[4] and "p"[4].

*/
    inline double eucl_dist(HPoint *p) const
    {
        double result = 1.0;
        for (unsigned i = 0; i < dim(); ++i)
            result  += std::pow(coord(i) - p->coord(i), 2);
        result = std::sqrt(result);
        return result;
    }

/*
Returns the square of eucledean distance between "this"[4] and "p"[4].

*/
    inline double eucl_dist2(HPoint *p) const
    {
        double result = 1.0;
        for (unsigned i = 0; i < dim(); ++i)
            result  += std::pow(coord(i) - p->coord(i), 2);
        return result;
    }

/*
Writes the hyper point to "buffer"[4] and increases "offset"[4].

*/
    void inline write(char *buffer, int &offset) const
    {
        Spatial::write(buffer, offset);

        memcpy(buffer+offset, m_coords, vectorlen());
        offset += vectorlen();
    }

/*
Reads the hyper point from "buffer"[4] and increases "offset"[4].

*/
    void inline read(const char *buffer, int &offset)
    {
        Spatial::read(buffer, offset);

        m_coords = new GTA_SPATIAL_DOM[dim()];
        memcpy(m_coords, buffer+offset, vectorlen());
        offset += vectorlen();
    }

/*
Returns the size of the hyper point on disc in bytes.

*/
    inline size_t size() const
    { return Spatial::size()  + vectorlen(); }

/*
Returns the bounding box of "this"[4].

*/
    HRect *bbox() const;

    static const std::string BasicType() { return "hpoint"; }
    static const bool checkType(const ListExpr type){
       return listutils::isSymbol(type, BasicType());
    }

  private:
    GTA_SPATIAL_DOM *m_coords; // coordinate vector
}; // class HPoint



/********************************************************************
1.1.1 Class ~HRect~

********************************************************************/
class HRect
        : public Spatial
{
  public:
/*
Constructor.

*/
    HRect(unsigned dim, const GTA_SPATIAL_DOM *lb,
                        const GTA_SPATIAL_DOM *ub)
        : Spatial(dim),
          m_lbVect(new GTA_SPATIAL_DOM[dim]),
          m_ubVect(new GTA_SPATIAL_DOM[dim])
    {
        memcpy(m_lbVect, lb, vectorlen());
        memcpy(m_ubVect, ub, vectorlen());
    }

/*
Default copy constructor.

*/
    inline HRect(const HRect &e)
        : Spatial(e),
          m_lbVect(new GTA_SPATIAL_DOM[e.dim()]),
          m_ubVect(new GTA_SPATIAL_DOM[e.dim()])
    {
        memcpy(m_lbVect, e.m_lbVect, vectorlen());
        memcpy(m_ubVect, e.m_ubVect, vectorlen());
    }

/*
Constructor (reads the given hyper rectangle from buffer and increases offset)

*/
    inline HRect(const char *buffer, int &offset)
    { read(buffer, offset); }

/*
Destructor.

*/
    inline ~HRect() { 
      delete [] m_lbVect; 
      delete [] m_ubVect; 
    }

/*
Assignment operator.

*/
    inline HRect &operator=(const HRect &rhs)
    {
        m_dim = rhs.dim();
        m_vectorlen = m_dim * sizeof(GTA_SPATIAL_DOM);
        delete [] m_lbVect;
        delete [] m_ubVect;
        m_lbVect = new GTA_SPATIAL_DOM[dim()];
        m_ubVect = new GTA_SPATIAL_DOM[dim()];
        memcpy(m_lbVect, rhs.m_lbVect, vectorlen());
        memcpy(m_ubVect, rhs.m_ubVect, vectorlen());
        return *this;
    }

/*
Returns the lower bounds vector of the hyper rectangle.

*/
    inline const GTA_SPATIAL_DOM *lb() const
    { return m_lbVect; }

/*
Returns the upper bounds vector of the hyper rectangle.

*/
    inline const GTA_SPATIAL_DOM *ub() const
    { return m_ubVect; }

/*
Returns the i-th coordinate of the lower bounds vector of the hyper rectangle.

*/
    inline GTA_SPATIAL_DOM lb(unsigned i) const
    {
        #ifdef __GTA_DEBUG
        assert(i < dim());
        #endif

        return m_lbVect[i];
    }

/*
Returns the i-th coordinate of the upper bounds vector of the hyper rectangle.

*/
    inline GTA_SPATIAL_DOM ub(unsigned i) const
    {
        #ifdef __GTA_DEBUG
        assert(i < dim());
        #endif

        return m_ubVect[i];
    }

/*
Returns the i-th coordinate of the center vector of the hyper rectangle.

*/
    inline GTA_SPATIAL_DOM center(unsigned i) const
    {
        #ifdef __GTA_DEBUG
        assert(i < dim());
        assert(m_ubVect[i] >= m_ubVect[i]);
        #endif

        return (m_ubVect[i] + m_lbVect[i]) / 2;
    }

/*
Returns the hpoint of which represents the center of the hyper rectangle.

*/
    inline HPoint center() const
    {
        GTA_SPATIAL_DOM  c[dim()];
        for (unsigned i = 0; i < dim(); ++i)
            c[i] = center(i);
        return HPoint(dim(), c);
    }

/*
Returns the area of the hyper rectangle.

*/
    inline double area() const
    {
        double result = 1.0;
        for (unsigned i = 0; i < dim(); ++i)
        {
            result *= (ub(i) - lb(i));
            if ((ub(i) - lb(i)) == 0.0)
                return 0.0;
        }
        return result;
    }

/*
Returns the margin of the hyper rectangle.

*/
    inline double margin() const
    {
        double result = 0.0;
        for (unsigned i = 0; i < dim(); ++i)
            result += (ub(i) - lb(i));
        result *= std::pow(2, (double)(dim()-1));
        return result;
    }

/*
Returns the partial margin of the hyper rectangle (each side is only added once instead of $2^(dim-1)$ times - this method should be used, if the margin is only used to order a set of hyper rectangles, since it fulfills the same inequalities (e.g. r1.margin() < r2.margin() iff. r1.margin2() < r2.margin2()).

*/
    inline double margin2() const
    {
        double result = 0.0;
        for (unsigned i = 0; i < dim(); ++i)
            result += (ub(i) - lb(i));
        return result;
    }

/*
Returns the overlap between "this"[4] and "r"[4].

*/
    inline double overlap(HRect *r) const
    {
        if (!intersects(r))
            return 0.0;
        else
            return Intersection(r).area();
    }

/*
Returns "true"[4] if the current hyper rectangle intersects "r"[4].

*/
    bool intersects(HRect *r) const
    {
        for (unsigned i = 0; i < dim(); ++i)
            if ((r->lb(i) > ub(i)) || (r->ub(i) < lb(i)))
                 return false;

        return true;
    }

/*
Returns "true"[4] if the current hyper rectangle contains "p"[4].

*/
    bool contains(HPoint *p) const
    {
        for (unsigned i = 0; i < dim(); ++i)
            if ((p->coord(i) < lb(i)) || (p->coord(i) > ub(i)))
                 return false;

        return true;
    }

/*
Returns "true"[4] if the current hyper rectangle contains "r"[4].

*/
    bool contains(HRect *r) const
    {
        for (unsigned i = 0; i < dim(); ++i)
            if ((r->lb(i) < lb(i)) || (r->ub(i) > ub(i)))
                 return false;

        return true;
    }

/*
Returns "true"[4] if the specified hyper rectangle is equal to the current one.

*/
    inline bool operator== (const HRect &r) const
    {
        for(unsigned i = 0; i < dim(); ++i)
            if(lb(i) != r.lb(i) || ub(i) != r.ub(i) )
                return false;

        return true;
}

/*
Returns "true"[4] if the specified hyper rectangle is not equal to the current one.

*/
    inline bool operator!= (const HRect &r) const
    { return !(*this == r); }

/*
Returns the intersection between "this"[4] and "r"[4].

*/
    inline const HRect Intersection(HRect *r) const
    {
        GTA_SPATIAL_DOM lbVect[dim()], ubVect[dim()];
        for(unsigned i = 0; i < dim(); ++i)
        {
            lbVect[i] = std::max(lb(i), r->lb(i));
            ubVect[i] = std::min(ub(i), r->ub(i));
        }
        return HRect(dim(), lbVect, ubVect);
    }

/*
Returns the union of "this"[4] and "r"[4].

*/
    inline const HRect Union(HRect *r) const
    {
        GTA_SPATIAL_DOM lbVect[dim()], ubVect[dim()];
        for(unsigned i = 0; i < dim(); ++i)
        {
            lbVect[i] = std::min(lb(i), r->lb(i));
            ubVect[i] = std::max(ub(i), r->ub(i));
        }
        return HRect(dim(), lbVect, ubVect);
    }

/*
Replaces "this"[4] with the union of "this"[4] and "r"[4].

*/
    void unite(HRect *r)
    {
        for (unsigned i=0; i < dim(); ++i)
        {
            m_lbVect[i] = std::min(lb(i), r->lb(i));
            m_ubVect[i] = std::max(ub(i), r->ub(i));
        }
    }

/*
Returns "true"[4], if "this"[4] represents the empty set (could e.g. happen, if the hyper rectangle was created from the "Intersection"[4]  method for two non intersecting bounding boxes).

*/
    inline bool isEmptySet()
    {
        for(unsigned i=0; i<dim(); ++i)
            if(lb(i) > ub(i))
                return true;

        return false;
    }

/*
Writes the hyper rectangle to "buffer"[4] and increases "offset"[4].

*/
    void inline write(char *buffer, int &offset) const
    {
        Spatial::write(buffer, offset);

        memcpy(buffer+offset, m_lbVect, vectorlen());
        offset += vectorlen();
        memcpy(buffer+offset, m_ubVect, vectorlen());
        offset += vectorlen();
    }

/*
Reads the hyper rectangle from "buffer"[4] and increases "offset"[4].

*/
    void inline read(const char *buffer, int &offset)
    {
        Spatial::read(buffer, offset);

        m_lbVect = new GTA_SPATIAL_DOM[dim()];
        memcpy(m_lbVect, buffer+offset, vectorlen());
        offset += vectorlen();

        m_ubVect = new GTA_SPATIAL_DOM[dim()];
        memcpy(m_ubVect, buffer+offset, vectorlen());
        offset += vectorlen();
    }


/*
Returns the size of the hyper point on disc in bytes.

*/
    inline size_t size() const
    { return Spatial::size()  + 2*vectorlen(); }

/*
Returns the bounding box of "this"[4].

*/
    inline HRect *bbox() const
    { return new HRect(*this); }

      static const std::string BasicType() { return "hrect"; }
      static const bool checkType(const ListExpr type){
        return listutils::isSymbol(type, BasicType());
      }


  private:
    GTA_SPATIAL_DOM *m_lbVect, *m_ubVect; // lower/upper bounds vect.
}; // class HRect




/********************************************************************
1.1.1 Class ~SpatialDistfuns~

********************************************************************/
class SpatialDistfuns
{
  public:
/*
Returns the Euclidean distance between "p1"[4] and "p2"[4].

*/
    static double euclDist(HPoint *p1, HPoint *p2);

/*
Returns the square of the Euclidean distance between "p1"[4] and "p2"[4] (avoids computing the square root of result).

*/
    static double euclDist2(HPoint *p1, HPoint *p2);

/*
Returns 0 if "r"[4] contains "p"[4] and the square of the Euclidean distance between "p"[4] and the edge of "r"[4] otherwhise.

*/
    static double minDist(HPoint *p, HRect *r);

/*
This method computes the square of the Euclidean distance between "p"[4] and the farthest point of each edge of "r"[4] and returns the minimum of all theese distances (needed for the RKV nearast-neighbour search algorithm, e.g. in the XTreeAlgebra).

*/
    static double minMaxDist(HPoint *p, HRect *r);
}; // class SpatialDistfuns



} // namespace gta
#endif // #ifndef __GTA_SPATIAL_H__
