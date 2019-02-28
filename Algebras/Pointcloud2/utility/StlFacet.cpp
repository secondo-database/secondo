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



1 StlFacet

*/

#include "StlFacet.h"
#include <boost/algorithm/string.hpp>

using namespace pointcloud2;
using namespace std;

StlVector::StlVector(const float x, const float y, const float z) :
    _x(x), _y(y), _z(z) {
}

StlVector::StlVector(const StlVector& other) {
    _x = other._x;
    _y = other._y;
    _z = other._z;
}

StlVector::StlVector(const StlVector& v1, const StlVector& v2) :
    _x(v2._x - v1._x),
    _y(v2._y - v1._y),
    _z(v2._z - v1._z) {
}

void StlVector::set(const float x, const float y, const float z) {
    _x = x;
    _y = y;
    _z = z;
}

void StlVector::set(const std::string x, const std::string y,
        const std::string z) {
    _x = std::stof(x);
    _y = std::stof( y);
    _z = std::stof(z);
}

StlVector::StlVector(std::ifstream& in) {
    in.read(reinterpret_cast<char*>(this), 3 * sizeof(float));
    // or, alternatively
    // in.read(reinterpret_cast<char*>(&_x), sizeof(float));
    // in.read(reinterpret_cast<char*>(&_y), sizeof(float));
    // in.read(reinterpret_cast<char*>(&_z), sizeof(float));
}

double StlVector::getLength() const {
    return std::sqrt(_x * _x + _y * _y + _z * _z);
}

double StlVector::getDistance(const StlVector& other) const {
    double dx = other._x - _x;
    double dy = other._y - _y;
    double dz = other._z - _z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

StlFacet::StlFacet(const StlVector& normal, const StlVector& v1,
        const StlVector& v2, const StlVector& v3) :
            _normal(normal) {

    // let _v1to2 be the longest edge.
    double edgeLen12 = v1.getDistance(v2);
    double edgeLen23 = v2.getDistance(v3);
    double edgeLen31 = v3.getDistance(v1);
    if (edgeLen12 <= edgeLen23 && edgeLen12 <= edgeLen31) {
        _v1 = v1;
        _v1to2 = StlVector(_v1, v2);
        _v1to3 = StlVector(_v1, v3);
    } else if (edgeLen23 <= edgeLen31 && edgeLen23 <= edgeLen12) {
        _v1 = v2;
        _v1to2 = StlVector(_v1, v3);
        _v1to3 = StlVector(_v1, v1);
    } else { // len31 is the longest edge
        _v1 = v3;
        _v1to2 = StlVector(_v1, v1);
        _v1to3 = StlVector(_v1, v2);
    }
    _area = calculateArea();
    _width = _v1to2.getLength();
    _height = 2.0 * _area / _width;
}

double StlFacet::calculateArea() const {
    // calculate the area of this facet, see https://de.wikipedia.org/wiki/
    //   Dreiecksfl%C3%A4che#Im_dreidimensionalen_Raum
    double r1 = _v1to2._y * _v1to3._z - _v1to2._z * _v1to3._y;
    double r2 = _v1to2._z * _v1to3._x - _v1to2._x * _v1to3._z;
    double r3 = _v1to2._x * _v1to3._y - _v1to2._y * _v1to3._x;
    return std::sqrt(r1 * r1 + r2 * r2 + r3 * r3);
}

bool StlObject::read(std::string fileName) {
    // open the file
    std::ifstream fileStream(fileName, ios::binary);
    if (!fileStream.is_open()) {
        std::cout << "Error opening file." << endl;
        _error = true;
        return false;
    }
    try {
        // determine the file length
        fileStream.seekg(0);
        streamsize fileLength = fileStream.tellg();
        fileStream.seekg(0, std::ios::end);
        fileLength = (streamsize)fileStream.tellg() - fileLength;
        fileStream.seekg(0);

        // determine the file format
        char header[5];
        fileStream.seekg(0);
        fileStream.read(header, sizeof(header));
        bool hasAsciiFormat = ((header[0] == 's' && header[1] == 'o'
                && header[2] == 'l' && header[3] == 'i' && header[4] == 'd'));
        if (hasAsciiFormat && fileLength > 160) {
            // further tests are required, since a binary file may start with
            // the text "solid", too
            fileStream.seekg(80);
            char sample[80];
            fileStream.read(sample, sizeof(sample));
            for (char i : sample) {
                if (i < 0 || i > 127) {
                    hasAsciiFormat = false;
                    break;
                }
            }
        }

        // read the facets
        fileStream.seekg(0);
        if (hasAsciiFormat)
            readAscii(fileStream, fileLength);
        else
            readBinary(fileStream, fileLength);
    } catch (std::invalid_argument &){
        _error = true;
    }
    fileStream.close();
    return !_error;
}

void StlObject::readBinary(ifstream& fileStream, const size_t fileLength) {
    // see http://www.fabbers.com/tech/STL_Format#Sct_binary

    constexpr streamsize HEADER_SIZE = 80;
    constexpr streamsize FACET_SIZE = 12 * 4 + 2; // 12 floats, 1 uint16_t

    // skip header of 80 bytes
    char header[HEADER_SIZE];
    fileStream.read(header, HEADER_SIZE);

    // read number of facets
    uint32_t facetCount;
    fileStream.read(reinterpret_cast<char*>(&facetCount), sizeof(uint32_t));

    // ensure the file is long enough
    streamsize bytesLeft = fileLength - HEADER_SIZE - sizeof(uint32_t);
    if (bytesLeft < FACET_SIZE * facetCount) {
        // correct facetCount - it appears that some StL file creators
        // do not bother to set it correctly
        uint32_t wrongFacetCount = facetCount;
        facetCount = bytesLeft / FACET_SIZE;
        cout << "facet count " << wrongFacetCount << " corrected to "
                << facetCount << endl;
        // no need to throw std::invalid_argument("invalid facet count");
    }

    // read facets. Despite correcting facetCount above, we use .eof()
    // to determine the end of file since facets may contain extra bytes
    // for attributes, and sometimes, facetCount seems to be 32767
    // irrespective of the actual entry count
    _facets->reserve(_facets->size() + facetCount);
    while (!fileStream.eof()) { //  && facets->size() < facetCount
        StlVector normal(fileStream);
        StlVector v1(fileStream);
        StlVector v2(fileStream);
        StlVector v3(fileStream);

        constexpr size_t attrByteCountSize = 2;
        uint16_t attrByteCount = 0;
        fileStream.read(reinterpret_cast<char*>(&attrByteCount),
                attrByteCountSize);
        if (attrByteCount > 0) {
            // ignore this. Reading (attrByteCount) bytes does not seem to be
            // appropriate.
        }
        _facets->push_back( { normal, v1, v2, v3 } );
    }
}

void StlObject::readAscii(ifstream& fileStream, const size_t fileLength) {
    // see http://www.fabbers.com/tech/STL_Format#Sct_ASCII
    std::string line;
    StlVector vectors[4];
    int vectorsRead = 0;
    unsigned errorLineCount = 0;
    while (std::getline(fileStream, line)){
        // trim line
        size_t first = line.find_first_not_of(' ');
        if (string::npos != first) {
            size_t last = line.find_last_not_of(" \r\n");
            line = line.substr(first, (last - first + 1));
        }

        // split line at spaces
        std::vector<std::string> tokens;
        boost::algorithm::split(tokens, line, boost::is_any_of(" "));
        if (tokens.size() < 1)
            continue;

        if (tokens[0].compare("facet") == 0) {
            // start of facet found
            if (tokens.size() < 5) {
                ++errorLineCount;
                continue;
            }
            if (tokens[1].compare("normal") != 0) {
                ++errorLineCount;
                continue;
            }
            vectors[0].set(tokens[2], tokens[3], tokens[4]);
            vectorsRead = 1;
        } else if (tokens[0].compare("vertex") == 0) {
            // next vertex found
            if (vectorsRead >= 1 && vectorsRead < 4) {
                vectors[vectorsRead].set(tokens[1], tokens[2], tokens[3]);
                ++vectorsRead;
            } else {
                ++errorLineCount;
            }
        } else if (tokens[0].compare("endfacet") == 0) {
            // end of facet found
            if (vectorsRead == 4) {
                StlFacet facet { vectors[0], vectors[1], vectors[2],
                    vectors[3] };
                _facets->push_back(facet);
            } else {
                ++errorLineCount;
            }
        }
    }
    if (errorLineCount > 0) {
        cout << "encountered " << errorLineCount << " unexpected lines."
                << endl;
    }
}
