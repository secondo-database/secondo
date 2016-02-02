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

1.1 Implementation file "GTA[_]Spatial.cpp"[4]

January-May 2008, Mirko Dibbert

*/
#include "GTA_Spatial.h"

using namespace gta;

/*
Method ~HPoint::hrect~:

*/
HRect *HPoint::bbox() const
{ return new HRect(dim(), coords(), coords()); }

/*
Method ~euclDist~

*/
double SpatialDistfuns::euclDist(HPoint *p1, HPoint *p2)
{
#ifdef __GTA_DEBUG
    assert(p1->dim() == p2->dim());
#endif

    double result = 0.0;
    for (unsigned i = 0; i < p1->dim(); ++i)
        result += std::pow(std::abs (p1->coord(i) - p2->coord(i)), 2);
    return std::sqrt(result);
}



/*
Method ~euclDist2~

*/
double SpatialDistfuns::euclDist2(HPoint *p1, HPoint *p2)
{
#ifdef __GTA_DEBUG
    assert(p1->dim() == p2->dim());
#endif

    double result = 0.0;
    for (unsigned i = 0; i < p1->dim(); ++i)
        result += std::pow(std::abs (p1->coord(i) - p2->coord(i)), 2);
    return result;
}



/*
Method ~minDist~

*/
double SpatialDistfuns::minDist(HPoint *p, HRect *r)
{
#ifdef __GTA_DEBUG
    assert(p->dim() == r->dim());
#endif

    double result = 0.0;
    for (unsigned i = 0; i < p->dim(); ++i)
    {
        if (p->coord(i) < r->lb(i))
            result += std::pow(p->coord(i) - r->lb(i), 2);
        else if (p->coord(i) > r->ub(i))
            result += std::pow(p->coord(i) - r->ub(i), 2);
        else
            result += std::pow(p->coord(i) - p->coord(i), 2);
    }
    return result;
}

/*
Method ~minMaxDist~

*/
double SpatialDistfuns::minMaxDist(HPoint *p, HRect *r)
{
#ifdef __GTA_DEBUG
    assert(p->dim() == r->dim());
#endif

    // compute center vector
    GTA_SPATIAL_DOM c[r->dim()];
    for (unsigned i = 0; i < p->dim(); ++i)
        c[i] = r->center(i);

    // compute S (dist from p to furthest vertex of r)
    double S = 0.0;
    for (unsigned i = 0; i < p->dim(); ++i)
    {
        if (p->coord(i) >= c[i])
            S += std::pow(p->coord(i) - r->lb(i), 2);
        else
            S += std::pow(p->coord(i) - r->ub(i), 2);
    }

    double min = S;
    if (p->coord(0) > c[0])
    {
        min -= std::pow(p->coord(0) - r->lb(0), 2);
        min += std::pow(p->coord(0) - r->ub(0), 2);
    }
    else if (p->coord(0) < c[0])
    {
        min -= std::pow(p->coord(0) - r->ub(0), 2);
        min += std::pow(p->coord(0) - r->lb(0), 2);
    }

    for (unsigned i = 1; i < p->dim(); ++i)
    {
        double sum = S;
        if (p->coord(i) > c[i])
        {
            sum -= std::pow(p->coord(i) - r->lb(i), 2);
            sum += std::pow(p->coord(i) - r->ub(i), 2);
        }
        else if (p->coord(i) < c[i])
        {
            sum -= std::pow(p->coord(i) - r->ub(i), 2);
            sum += std::pow(p->coord(i) - r->lb(i), 2);
        }

        if (sum < min)
            min = sum;
    }
    return min;
} // minMaxSum
