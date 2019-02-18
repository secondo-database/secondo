/*
----
This file is part of SECONDO.

Copyright (C) 2019,
Faculty of Mathematics and Computer Science,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 Stl tools

*/

#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <ios>
#include <memory>
#include <cmath>
#include <vector>

namespace pointcloud2 {

/*
1.1 StlVector

Represents a vector (or a vertex) in an StL facet.

*/
struct StlVector {
    float _x = 0;
    float _y = 0;
    float _z = 0;

    StlVector() {}

    /* creates a vector with the given coordinates */
    StlVector(const float x, const float y, const float z);

    /* creates a copy of the given other vector */
    StlVector(const StlVector& other);

    /* creates a vector pointing from the given v1 to v2 */
    StlVector(const StlVector& v1, const StlVector& v2);

    /* creates a new vector, reading its coordinates from the given stream */
    StlVector(std::ifstream& in);

    ~StlVector() = default;

    /* sets the vector to the given coordinates */
    void set(const float x, const float y, const float z);

    /* sets the vector to the given coordinates which are converted to
     * float values */
    void set(const std::string x, const std::string y, const std::string z);

    /* returns the length of this vector */
    double getLength() const;

    /* returns the distance between two points */
    double getDistance(const StlVector& other) const;
};

/*
1.2 StlFacet

Represents a facet in an StL object, consisting of the normal vector and the
three vertices of a triangle in counter-clockwise order.

*/
struct StlFacet {
    /* the normal vector */
    StlVector _normal;
    /* vertex 1 */
    StlVector _v1;
    /* vector from vertex 1 to vertex 2 */
    StlVector _v1to2;
    /* vector from vertex 1 to vertex 3 */
    StlVector _v1to3;

    /* the "width" of the triangle's baseline, i.e. the length of _v1to2 */
    double _width;
    /* the "height" of the triangle */
    double _height;
    /* the area covered by the triangle */
    double _area;

    /* creates a new facet from the given normal vector and vertices */
    StlFacet(const StlVector& normal, const StlVector& v1,
            const StlVector& v2, const StlVector& v3);

    /* returns the area covered by this facet */
    double getArea() const { return _area; }

private:
    /* calculates the area covered by the triangle
     * in _v1, _v1to2, and _v1to3 */
    double calculateArea() const;
};

/*
1.3 StlObject

Represents an StereoLitography (StL) object which can be read from one or
several files and consists of a vector of facets (triangles).

*/
class StlObject {
    /* is set to true if an error occurs while reading from a file stream */
    bool _error = false;
    /* the facets read from one or several files */
    std::shared_ptr<std::vector<StlFacet>> _facets;

public:
    /* creates an empty object */
    StlObject() {
        _facets = std::make_shared<std::vector<StlFacet>>();
    }

    ~StlObject() = default;

    /* Adds the facets from the given file. This method may be called
     * several times to combine facets from various files. Returns true
     * if the import was successful, or false if an error occurred. */
    bool read(std::string fileName);

    /* Returns the facets that were imported from one or several files */
    std::shared_ptr<std::vector<StlFacet>> getFacets() { return _facets; }

private:
    /* Reads facet from the given fileStream in binary format */
    void readBinary(std::ifstream& fileStream, const size_t fileLength);

    /* Reads facet from the given fileStream in ascii format */
    void readAscii(std::ifstream& fileStream, const size_t fileLength);

};


} // end of namespace pointcloud2
