/*
 Copyright (C) 2010 by The Regents of the University of California
 
 Redistribution of this file is permitted under
 the terms of the BSD license.
 
 Date: 11/01/2009
 Author: Sattam Alsubaiee <salsubai (at) ics.uci.edu>
         Shengyue Ji <shengyuj (at) ics.uci.edu>


*/

#ifndef _UTIL_H_
#define _UTIL_H_

#include <vector>

namespace rstartree {

class Point
{
public:
    double x;
    double y;

    bool operator==(const Point &p) const
    {
        return x == p.x && y == p.y;
    };
};

class Rectangle
{
public:
    // the mbr's lower value
    Point min;
    // the mbr's upper value
    Point max;

    // check whether two rectangles intersect or not
    bool intersects(const Rectangle &rect) const;
    // check whether two rectangles touch each other
    bool touches(const Rectangle &rect) const;
    // check whether the rectangle is contained in the other rectangle
    bool contains(const Rectangle &rect) const;
    // return the squared minimum distance between a point and a rectangle
    double minDist2(Point p) const;
    // return the rectangle's area
    double area() const;
    // return the enlarged area needed to include an object
    double enlargedArea(const Rectangle &rect) const;
    // enlarge an object into a rectangle
    void enlarge(const Rectangle &rect);
    // return the overlapped area between two rectangles
    double overlapedArea(const Rectangle &rect) const;
    // return the perimeter of a rectangle
    double margin() const;
    
    bool operator==(const Rectangle &r) const
    {
        return min == r.min && max == r.max;
    };
};

class Object
{
public:
    // the object/node id
    unsigned id;
    
    // the rectangle that represents the object/node
    Rectangle mbr;
    
    bool operator==(const Object &o) const
    {
        return id == o.id && mbr == o.mbr;
    };
};

class NodeMinDist2
{
public:
    unsigned id;
    double minDist2;
    const std::vector<char> *flags;

    bool operator<(const NodeMinDist2 &n2) const
    {
        return minDist2 > n2.minDist2;
    };
};

class CenterDistance
{
public:
    Object object;
    double distance;

    bool operator<(const CenterDistance &d) const
    {
        return distance > d.distance;
    };
};

class EntryValue
{
public:
    Object object;
    double value;
    unsigned index;

    bool operator<(const EntryValue &e) const
    {
        return value < e.value;
    };
};

}

#endif

