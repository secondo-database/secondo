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
        : Attribute(false), m_coords(0)
    {}

/*
Constructor (initialises the hyper point with the given values)

*/
    inline HPointAttr(int dim, GTA_SPATIAL_DOM *coords)
        : Attribute(true),
          m_dim(dim),
          m_coords(vectorlen())
    { m_coords.write((char*) coords, vectorlen(), 0);

     }

/*
Default copy constructor.

*/
    HPointAttr(const HPointAttr &p)
        : Attribute(p.IsDefined()),
          m_dim(p.m_dim),
          m_coords(p.m_coords.getSize())
    {
        if(IsDefined())
        {
           m_coords.copyFrom(p.m_coords);
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
        SetDefined(defined);
        if(defined)
        {
            m_dim = dim;
            m_coords.resize(vectorlen());
            m_coords.write((char*)coords, m_coords.getSize(), 0);
        }
    }

/*
Sets the attribute values to the given values.

*/
    void set(bool defined, HPoint *p)
    {
        SetDefined(defined);
        if(defined)
        {
            m_dim = p->dim();
            m_coords.resize(vectorlen());
            m_coords.write((char*)p->coords(), m_coords.getSize(), 0);
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
    { m_coords.destroy(); }

/*
Returns a new "HPoint"[4] object which represents "this"[4].

*/
    inline HPoint *hpoint() const
    {
        char * buffer= new char[vectorlen()];
        m_coords.read(buffer, vectorlen(), 0);
        const GTA_SPATIAL_DOM *coords =
                reinterpret_cast<const GTA_SPATIAL_DOM*>(buffer);
        HPoint* res = new HPoint(m_dim, coords);
        delete [] buffer;
        return res;
    }

/********************************************************************
Implementation of virtual methods from the Attribute class:

********************************************************************/

    inline virtual size_t Sizeof() const
    { return sizeof(*this); }

    inline virtual bool Adjacent(const Attribute *attr) const
    { return false; }

    inline virtual Attribute *Clone() const
    { return new HPointAttr(*this); }

    inline virtual int NumOfFLOBs() const
    { return 1; }

    inline virtual Flob *GetFLOB(const int i)
    { return &m_coords; }

    inline virtual int Compare(const Attribute *rhs) const
    { return 0; }

    virtual size_t HashValue() const
    { return 0; }

    virtual void CopyFrom(const Attribute *rhs)
    {
        const HPointAttr *p = static_cast<const HPointAttr*>(rhs);

        SetDefined( p->IsDefined());
        if(IsDefined())
        {
            m_dim = p->m_dim;
            m_coords.copyFrom(p->m_coords);
        }
    }

private:
/*
Returns the size of the coordinate vector in bytes.

*/
    inline unsigned vectorlen() const
    { return m_dim * sizeof(GTA_SPATIAL_DOM); }

    int m_dim;      // dimension of the hyper point
    Flob m_coords;  // coordinate vector

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
        : Attribute(false),  m_lbVect(0), m_ubVect(0)
    {}

/*
Constructor (initialises the hyper rectangle with the given values)

*/
    inline HRectAttr(int dim, GTA_SPATIAL_DOM *lb,
                              GTA_SPATIAL_DOM *ub)
        : Attribute(true),
          m_dim(dim),
          m_lbVect(vectorlen()),
          m_ubVect(vectorlen())
    {
        m_lbVect.write((char*)lb, vectorlen(), 0);
        m_ubVect.write((char*)ub,vectorlen(), 0);
    }

/*
Default copy constructor.

*/
    HRectAttr(const HRectAttr &r)
        : Attribute(r.IsDefined()),
          m_dim(r.m_dim),
          m_lbVect(r.m_lbVect.getSize()),
          m_ubVect(r.m_ubVect.getSize())
    {
        if(IsDefined())
        {
            m_lbVect.copyFrom(r.m_lbVect);
            m_ubVect.copyFrom(r.m_ubVect);
           
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
        SetDefined(defined);
        if(defined)
        {
            m_dim = dim;
            m_lbVect.resize(vectorlen());
            m_lbVect.write((char*)lb, m_lbVect.getSize(), 0);

            m_ubVect.resize(vectorlen());
            m_ubVect.write((char*)ub,m_ubVect.getSize(), 0);
        }
    }

/*
Sets the attribute values to the given values.

*/
    void set(bool defined, HRect *r)
    {
        SetDefined( defined);
        if(defined)
        {
            m_dim = r->dim();

            // set  lower bounds vector
            m_lbVect.resize(vectorlen());
            m_lbVect.write((char*)r->lb(), m_lbVect.getSize(), 0);

            // set upper bounds vector
            m_ubVect.resize(vectorlen());
            m_ubVect.write((char*)r->ub(), m_ubVect.getSize(), 0);
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
    { m_lbVect.destroy(); m_ubVect.destroy(); }

/*
Returns a new "HRect"[4] object which represents "this"[4].

*/
    inline HRect *hrect() const
    {
        char*  buffer1 = new char[vectorlen()];
        m_lbVect.read(buffer1, vectorlen(), 0);;

        char* buffer2 = new char[vectorlen()];
        m_ubVect.read(buffer2, vectorlen(), 0);
         
        GTA_SPATIAL_DOM *lb = reinterpret_cast< GTA_SPATIAL_DOM*>(buffer1);
        GTA_SPATIAL_DOM *ub = reinterpret_cast< GTA_SPATIAL_DOM*>(buffer2);

        HRect* res =  new HRect(m_dim, lb, ub);
        delete[] buffer1;
        delete[] buffer2;
        return res;
    }

/********************************************************************
Implementation of virtual methods from the Attribute class:

********************************************************************/

    inline virtual size_t Sizeof() const
    { return sizeof(*this); }

    inline virtual bool Adjacent(const Attribute *attr) const
    { return false; }

    inline virtual Attribute *Clone() const
    { return new HRectAttr(*this); }

    inline virtual int NumOfFLOBs() const
    { return 2; }

    inline virtual Flob* GetFLOB(const int i)
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

        SetDefined(r->IsDefined());
        if(IsDefined())
        {
            m_dim = r->m_dim;
            m_lbVect.copyFrom(r->m_lbVect);
            m_ubVect.copyFrom(r->m_ubVect);
        }
    }

private:
/*
Returns the size of the coordinate vector in bytes.

*/
    inline unsigned vectorlen() const
    { return m_dim * sizeof(GTA_SPATIAL_DOM); }

    int m_dim;      // dimension of the hyper rectangle
    Flob m_lbVect;  // lower bounds vector
    Flob m_ubVect;  // upper bounds vector
}; // class HRectAttr


} // namespace gta
#endif // #ifndef __SPATIAL_ATTR_H__
