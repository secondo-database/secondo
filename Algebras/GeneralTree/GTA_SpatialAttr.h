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

1.1 Headerfile "GTA[_]SpatialAttr.h"[4]

January-May 2008, Mirko Dibbert

This file implements the "hpoint"[4] (hyper point) and "hrect"[4] (hyper rectangle) type constructors. The methods of the resp. "HPoint"[4] and "HRect"[4] class could be accessed with help of the "hpoint"[4] or "hrect"[4] method.

*/
#ifndef __GTA_SPATIAL_ATTR_H__
#define __GTA_SPATIAL_ATTR_H__

#include "Attribute.h"
#include "GTA_Spatial.h"

namespace gta {

/********************************************************************
1.1.1 Class ~HPointAttr~

********************************************************************/
class HPointAttr
        : public Attribute
{
  public:
/*
Default constructor.

*/
    inline HPointAttr()
    {}

/*
Constructor (creates an undefined hyper point - the parameter is only needed to distinguish this constructor from the default constructor)

*/
    inline HPointAttr(size_t size)
        : m_coords(0), m_defined(false)
    {}

/*
Constructor (initialises the hyper point with the given values)

*/
    inline HPointAttr(int dim, GTA_SPATIAL_DOM *coords)
        : m_dim(dim),
          m_coords(vectorlen()),
          m_defined(true)
    { m_coords.Put(0, vectorlen(), coords); }

/*
Default copy constructor.

*/
    HPointAttr(const HPointAttr &p)
        : m_dim(p.m_dim),
          m_coords(p.m_coords.Size()),
          m_defined(p.m_defined)
    {
        if(IsDefined())
        {
            const char *buffer;
            p.m_coords.Get(0, &buffer);
            m_coords.Put(0, p.m_coords.Size(), buffer);
        }
    }

/*
Destructor.

*/
    inline ~HPointAttr()
    {}

/*
Sets the attribute values to the given values.

*/
    void set(bool defined, unsigned dim, GTA_SPATIAL_DOM *coords)
    {
        m_defined = defined;
        if(m_defined)
        {
            m_dim = dim;
            m_coords.Clean();
            m_coords.Resize(vectorlen());
            m_coords.Put(0, m_coords.Size(), coords);
        }
    }

/*
Sets the attribute values to the given values.

*/
    void set(bool defined, HPoint *p)
    {
        m_defined = defined;
        if(m_defined)
        {
            m_dim = p->dim();
            m_coords.Clean();
            m_coords.Resize(vectorlen());
            m_coords.Put(0, m_coords.Size(), p->coords());
        }
    }

/*
Returns the dimension of the hyper point.

*/
    inline unsigned dim() const
    { return m_dim; }

/*
Removes the disc representation of the coordinate vector FLOB.

*/
    inline void deleteFLOB()
    { m_coords.Destroy(); }

/*
Returns a new "HPoint"[4] object which represents "this"[4].

*/
    inline HPoint *hpoint() const
    {
        const char *buffer;
        m_coords.Get(0, &buffer);
        const GTA_SPATIAL_DOM *coords =
                reinterpret_cast<const GTA_SPATIAL_DOM*>(buffer);
        return new HPoint(m_dim, coords);
    }

/********************************************************************
Implementation of virtual methods from the Attribute class:

********************************************************************/
    inline virtual bool IsDefined() const
    { return m_defined; }

    inline virtual void SetDefined(bool defined)
    { m_defined = defined; }

    inline virtual size_t Sizeof() const
    { return sizeof(*this); }

    inline virtual bool Adjacent(const Attribute *attr) const
    { return false; }

    inline virtual Attribute *Clone() const
    { return new HPointAttr(*this); }

    inline virtual int NumOfFLOBs() const
    { return 1; }

    inline virtual FLOB *GetFLOB(const int i)
    { return &m_coords; }

    inline virtual int Compare(const Attribute *rhs) const
    { return 0; }

    virtual size_t HashValue() const
    { return 0; }

    virtual void CopyFrom(const Attribute *rhs)
    {
        const HPointAttr *p = static_cast<const HPointAttr*>(rhs);

        m_defined = p->IsDefined();
        if(IsDefined())
        {
            m_dim = p->m_dim;
            const char *buffer;

            m_coords.Clean();
            m_coords.Resize(vectorlen());
            p->m_coords.Get(0, &buffer);
            m_coords.Put(0, p->m_coords.Size(), buffer);
        }
    }

private:
/*
Returns the size of the coordinate vector in bytes.

*/
    inline unsigned vectorlen() const
    { return m_dim * sizeof(GTA_SPATIAL_DOM); }

    int m_dim;      // dimension of the hyper point
    FLOB m_coords;  // coordinate vector
    bool m_defined; // true, if the attribute is defined

}; // class HPointAttr



/********************************************************************
1.1.1 Class ~HRectAttr~

********************************************************************/
class HRectAttr
        : public Attribute
{
  public:
/*
Default constructor.

*/
    inline HRectAttr()
    {}

/*
Constructor (creates an undefined hyper rectangle - the parameter is only needed to distinguish this constructor from the default constructor).

*/
    inline HRectAttr(size_t size)
        : m_lbVect(0), m_ubVect(0), m_defined(false)
    {}

/*
Constructor (initialises the hyper rectangle with the given values)

*/
    inline HRectAttr(int dim, GTA_SPATIAL_DOM *lb,
                              GTA_SPATIAL_DOM *ub)
        : m_dim(dim),
          m_lbVect(vectorlen()),
          m_ubVect(vectorlen()),
          m_defined(true)
    {
        m_lbVect.Put(0, vectorlen(), lb);
        m_ubVect.Put(0, vectorlen(), ub);
    }

/*
Default copy constructor.

*/
    HRectAttr(const HRectAttr &r)
        : m_dim(r.m_dim),
          m_lbVect(r.m_lbVect.Size()),
          m_ubVect(r.m_ubVect.Size()),
          m_defined(r.m_defined)
    {
        if(IsDefined())
        {
            const char *buffer;

            r.m_lbVect.Get(0, &buffer);
            m_lbVect.Put(0, r.m_lbVect.Size(), buffer);

            r.m_ubVect.Get(0, &buffer);
            m_ubVect.Put(0, r.m_ubVect.Size(), buffer);
        }
    }

/*
Destructor.

*/
    inline ~HRectAttr()
    {}

/*
Sets the attribute values to the given values.

*/
    void set(
            bool defined, unsigned dim,
            GTA_SPATIAL_DOM *lb, GTA_SPATIAL_DOM *ub)
    {
        m_defined = defined;
        if(m_defined)
        {
            m_dim = dim;

            m_lbVect.Clean();
            m_lbVect.Resize(vectorlen());
            m_lbVect.Put(0, m_lbVect.Size(), lb);

            m_ubVect.Clean();
            m_ubVect.Resize(vectorlen());
            m_ubVect.Put(0, m_ubVect.Size(), ub);
        }
    }

/*
Sets the attribute values to the given values.

*/
    void set(bool defined, HRect *r)
    {
        m_defined = defined;
        if(m_defined)
        {
            m_dim = r->dim();

            // set  lower bounds vector
            m_lbVect.Clean();
            m_lbVect.Resize(vectorlen());
            m_lbVect.Put(0, m_lbVect.Size(), r->lb());

            // set upper bounds vector
            m_ubVect.Clean();
            m_ubVect.Resize(vectorlen());
            m_ubVect.Put(0, m_ubVect.Size(), r->ub());
        }
    }

/*
Returns the dimension of the hyper rectangle.

*/
    inline unsigned dim() const
    { return m_dim; }

/*
Removes the disc representation of the FLOBs.

*/
    inline void deleteFLOB()
    { m_lbVect.Destroy(); m_ubVect.Destroy(); }

/*
Returns a new "HRect"[4] object which represents "this"[4].

*/
    inline HRect *hrect() const
    {
        const char *buffer;
        m_lbVect.Get(0, &buffer);
        const GTA_SPATIAL_DOM *lb =
                reinterpret_cast<const GTA_SPATIAL_DOM*>(buffer);
        m_ubVect.Get(0, &buffer);
        const GTA_SPATIAL_DOM *ub =
                reinterpret_cast<const GTA_SPATIAL_DOM*>(buffer);
        return new HRect(m_dim, lb, ub);
    }

/********************************************************************
Implementation of virtual methods from the Attribute class:

********************************************************************/
    inline virtual bool IsDefined() const
    { return m_defined; }

    inline virtual void SetDefined(bool defined)
    { m_defined = defined; }

    inline virtual size_t Sizeof() const
    { return sizeof(*this); }

    inline virtual bool Adjacent(const Attribute *attr) const
    { return false; }

    inline virtual Attribute *Clone() const
    { return new HRectAttr(*this); }

    inline virtual int NumOfFLOBs() const
    { return 2; }

    inline virtual FLOB* GetFLOB(const int i)
    {
        if (i == 1)
            return &m_lbVect;
        else
            return &m_ubVect;
    }

    inline virtual int Compare(const Attribute *rhs) const
    { return 0; }

    virtual size_t HashValue() const
    { return 0; }

    virtual void CopyFrom(const Attribute *rhs)
    {
        const HRectAttr *r = static_cast<const HRectAttr*>(rhs);

        m_defined = r->IsDefined();
        if(IsDefined())
        {
            m_dim = r->m_dim;
            const char *buffer;

            // copy lower bounds vector
            m_lbVect.Clean();
            m_lbVect.Resize(vectorlen());
            r->m_lbVect.Get(0, &buffer);
            m_lbVect.Put(0, r->m_lbVect.Size(), buffer);

            // copy upper bounds vector
            m_ubVect.Clean();
            m_ubVect.Resize(vectorlen());
            r->m_ubVect.Get(0, &buffer);
            m_ubVect.Put(0, r->m_ubVect.Size(), buffer);
        }
    }

private:
/*
Returns the size of the coordinate vector in bytes.

*/
    inline unsigned vectorlen() const
    { return m_dim * sizeof(GTA_SPATIAL_DOM); }

    int m_dim;      // dimension of the hyper rectangle
    FLOB m_lbVect;  // lower bounds vector
    FLOB m_ubVect;  // upper bounds vector
    bool m_defined; // true, if the attribute is defined
}; // class HRectAttr


} // namespace gta
#endif // #ifndef __SPATIAL_ATTR_H__
