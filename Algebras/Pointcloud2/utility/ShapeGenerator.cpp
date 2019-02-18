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



1 ShapeGenerator

*/

#include "ShapeGenerator.h"

#include <memory>
#include <string>
#include <assert.h>
#include <cmath>
#include <math.h>
#include <iostream>
#include <algorithm>

using namespace pointcloud2;
using namespace std;

ShapeGenerator::ShapeGenerator(
        Pointcloud2* pc2,
        const unsigned planeCount, const unsigned sphereCount,
        const unsigned cylinderCount, const unsigned coneCount,
        const double pointDist, const double shapeSizeMin,
        const double shapeSizeMax, const double spaceSize,
        const double diffusion, const unsigned noisePointCount,
        const unsigned rndSeed) :
                _pc2(pc2),
                _planeCount(planeCount), _sphereCount(sphereCount),
                _cylinderCount(cylinderCount), _coneCount(coneCount),
                _pointDist(pointDist), _shapeSizeMin(shapeSizeMin),
                _shapeSizeMax(shapeSizeMax), _spaceSize(spaceSize),
                _diffusion(diffusion), _noisePointCount(noisePointCount),
                _rndSeed(rndSeed) {
    _padding = std::sqrt(DIMENSIONS * (shapeSizeMax / 2.0));
    _facets = shared_ptr<vector<StlFacet>>(nullptr);
    _facetPointCount = 0;
}

/* creates a ShapeGenerator with the given settings */
ShapeGenerator::ShapeGenerator(Pointcloud2* pc2,
        const std::shared_ptr<std::vector<StlFacet>> facets,
        const size_t pointCount,
        const Rotation rotation,
        const double diffusion,
        const unsigned rndSeed) :
            _pc2(pc2),
            _planeCount(0), _sphereCount(0),
            _cylinderCount(0), _coneCount(0),
            _pointDist(0.1), _shapeSizeMin(1.0),
            _shapeSizeMax(2.0), _spaceSize(10.01),
            _diffusion(diffusion), _noisePointCount(0),
            _rndSeed(rndSeed), _padding(5.0),
            _facets(facets), _facetPointCount(pointCount),
            _facetsRotation(rotation) {
}


void ShapeGenerator::generate() {
    assert (_pointDist > 0.0);
    assert (_shapeSizeMin > 2.0 * _pointDist);
    assert (_shapeSizeMax >= _shapeSizeMin);
    assert (_spaceSize >= 2.0 * _padding);

    // initialize the random number generator with the given seed
    MathUtils::initializeRnd(_rndSeed);

    // start bulk load
    _pc2->startInsert();

    // generate planes along the x and y axes
    for (unsigned i = 0; i < _planeCount; ++i)
        generatePlane();

    // generate spheres around the point of origin
    for (unsigned i = 0; i < _sphereCount; ++i)
        generateSphere();

    // generate cylinders, using the the z axis as the axis
    for (unsigned i = 0; i < _cylinderCount; ++i)
        generateCylinder();

    // generate cones, using the the z axis as the axis
    for (unsigned i = 0; i < _coneCount; ++i)
        generateCone();

    // generate noise
    for (unsigned i = 0; i < _noisePointCount; ++i)
        generateNoise();
    if (REPORT_TO_CONSOLE && _noisePointCount > 0) {
        std::cout << "+ noise (" << formatInt(_noisePointCount) << " points)"
                << std::endl;
    }

    if (_facets.get()) {
        double totalFacetArea = 0.0;
        for (StlFacet& facet : *_facets)
            totalFacetArea += facet.getArea();
        setOffsetAndRotation(_facetsRotation);
        for (StlFacet& facet : *_facets)
            generateFacet(facet, totalFacetArea);
    }

    // insert points to _pc2 (if not done already)
    finalize();

    // finalize bulk load
    _pc2->finalizeInsert();

    // if a specific seed was used above for the random number generator,
    // initialize it with the current system time, so subsequent operators
    // get "actual" random numbers
    if (_rndSeed > 0) {
        MathUtils::initializeRnd(0);
    }
}

/* generates a single plane */
void ShapeGenerator::generatePlane() {
    size_t pointCount = 0;
    setOffsetAndRotation(ROTATE_PLANES);
    double extX = MathUtils::getRnd(_shapeSizeMin, _shapeSizeMax) / 2.0;
    double extY = MathUtils::getRnd(_shapeSizeMin, _shapeSizeMax) / 2.0;
    for (double y = -extY; y < extY; y += _pointDist) {
        for (double x = -extX; x < extX; x += _pointDist) {
            addTransformed(x, y, 0.0);
            ++pointCount;
        }
    }

    if (REPORT_TO_CONSOLE) {
        // the plane's normal vector is the transformed z axis:
        double n1 = _r13;
        double n2 = _r23;
        double n3 = _r33;
        double d = _ox * n1 + _oy * n2 + _oz * n3;
        std::cout << "+ plane defined by 'ax + by + cz = d' with "
                << "a = " << n1 << ", b = " << n2 << ", c = " << n3
                << ", d = " << d << ", closest point from origin "
                << "(" << d * n1 << ", " << d * n2 << ", " << d * n3 << ")"
                << " (" << formatInt(pointCount) << " points)" << std::endl;
    }
}

/* generates a single sphere */
void ShapeGenerator::generateSphere() {
    size_t pointCount = 0;
    setOffsetAndRotation(ROTATE_SPHERES);
    double r = MathUtils::getRnd(_shapeSizeMin, _shapeSizeMax) / 2.0;
    double angle1Step = std::asin(_pointDist / r);
    for (double a1 = 0.0; a1 < M_PI; a1 += angle1Step) {
        double z = r * std::cos(a1);
        double r2 = r * std::sin(a1);
        double angle2Step = std::asin(_pointDist / r2);
        for (double a2 = -M_PI; a2 < M_PI; a2 += angle2Step) {
            double x = r2 * std::cos(a2);
            double y = r2 * std::sin(a2);
            addTransformed(x, y, z);
            ++pointCount;
        }
    }

    if (REPORT_TO_CONSOLE) {
        std::cout << "+ sphere at "
                << "center = (" << _ox << ", " << _oy << ", " << _oz
                << "), radius = " << r
                << " (" << formatInt(pointCount) << " points)" << std::endl;
    }
}

/* generates a single cylinder */
void ShapeGenerator::generateCylinder() {
    size_t pointCount = 0;
    setOffsetAndRotation(ROTATE_CYLINDERS);
    double h = MathUtils::getRnd(_shapeSizeMin, _shapeSizeMax) / 2.0;
    double r = MathUtils::getRnd(_shapeSizeMin, _shapeSizeMax) / 2.0;
    double angleStep = std::asin(_pointDist / r);
    for (double z = -h; z < h; z += _pointDist) {
        for (double a = -M_PI; a < M_PI; a += angleStep) {
            double x = r * std::cos(a);
            double y = r * std::sin(a);
            addTransformed(x, y, z);
            ++pointCount;
        }
    }

    if (REPORT_TO_CONSOLE) {
        // the direction of the cylinder's axis is the transformed z axis:
        double n1 = _r13;
        double n2 = _r23;
        double n3 = _r33;
        // The plane which is orthogonal to n1/n2/n3 and contains the
        // origin (0, 0, 0) is n1*x + n2*y + n3*z = 0. We determine the
        // point p where the line x = _o + s * n crosses this plane:
        double s = -(n1 * _ox + n2 * _oy + n3 * _oz)
                / (n1 * n1 + n2 * n2 + n3 * n3);
        double px = _ox + s * n1;
        double py = _oy + s * n2;
        double pz = _oz + s * n3;
        // the axis of the cylinder is now defined by 'p + a * _n':
        std::cout << "+ cylinder with axis = "
                << "(" << px << ", " << py << ", " << pz << ") + a * "
                << "("  << n1 << ", " << n2 << ", " << n3 << ")"
                << ", radius = " << r
                << " (" << formatInt(pointCount) << " points)" << std::endl;
    }
}

/* generates a single cone */
void ShapeGenerator::generateCone() {
    size_t pointCount = 0;
    setOffsetAndRotation(ROTATE_CONES);
    double h = MathUtils::getRnd(_shapeSizeMin, _shapeSizeMax) / 2.0;
    double rmax = MathUtils::getRnd(_shapeSizeMin, _shapeSizeMax) / 2.0;
    // iterate from tip to bottom
    for (double z = h; z > -h; z -= _pointDist) {
        double r = rmax * (h - z) / (2.0 * h);
        double angleStep = std::asin(_pointDist / r);
        // apart from the very tip, add at least 4 points
        if (4.0 * r > _pointDist)
            angleStep = MIN(angleStep, M_PI / 2.001);
        for (double a = -M_PI; a < M_PI; a += angleStep) {
            double x = r * std::cos(a);
            double y = r * std::sin(a);
            addTransformed(x, y, z);
            ++pointCount;
        }
    }

    if (REPORT_TO_CONSOLE) {
        // the direction of the cone's axis is the transformed -z axis
        double n1 = -_r13;
        double n2 = -_r23;
        double n3 = -_r33;
        // the apex is the transformed point (0, 0, h):
        double px = _ox + h * _r13;
        double py = _oy + h * _r23;
        double pz = _oz + h * _r33;
        std::cout << "+ cone with apex = "
                << "(" << px << ", " << py << ", " << pz << ") + a * "
                << "("  << n1 << ", " << n2 << ", " << n3 << ")"
                << ", tan(angle) = " << (rmax / h)
                << " (" << formatInt(pointCount) << " points)" << std::endl;
    }
}

void ShapeGenerator::generateNoise() {
    double x = MathUtils::getRnd(0.0, _spaceSize);
    double y = MathUtils::getRnd(0.0, _spaceSize);
    double z = MathUtils::getRnd(0.0, _spaceSize);
    addPlain(x, y, z);
}

void ShapeGenerator::generateFacet(const StlFacet& facet,
        const double totalFacetArea) {
    // get the number of points to be created for this facet
    double pointCountDbl = _facetPointCount
            * (facet.getArea() / totalFacetArea);
    size_t pointCount = static_cast<size_t>(std::floor(pointCountDbl));

    // if the number is not an int value (esp. if it is below 1.0), use
    // a random number to determine whether an extra point should be created
    double rest = pointCountDbl - std::floor(pointCountDbl);
    if (MathUtils::getRnd() < rest)
        ++pointCount;

    // may omit this facet
    if (pointCount == 0)
        return;

    // get point 1 and the vectors pointing from point 1 to point 2 and 3
    const StlVector& v1 = facet._v1;
    const StlVector& v1to2 = facet._v1to2;
    const StlVector& v1to3 = facet._v1to3;

    // get the "width" and "height" of the triangle
    double width = facet._width;
    double height = facet._height;

    // determine the step needed to create approx. (pointCount) points
    double step = std::sqrt((width * height)
            / static_cast<double>(2 * pointCount));
    // scale the step so fac3 and fac2 can run from 0.0 to 1.0
    double step2 = step / width;
    double step3 = step / height;

    // create the points for the triangle; avoid the edges by starting with
    // step3 / 2.0 etc.
    for (double fac3 = step3 / 2.0; fac3 <= 1.0; fac3 += step3) {
        double startFac2 = step2 / 2.0;
        if (startFac2 + fac3 > 1.0) {
            // at the tip of the triangle, use random numbers to determine
            // whether to create points (otherwise whole parts of the object
            // could be missing, or points get too dense)
            if (MathUtils::getRnd() < fac3)
                continue;
            startFac2 = (1.0 - fac3) / 2.0;
        }
        for (double fac2 = startFac2; fac2 + fac3 <= 1.0; fac2 += step2){
            double x = v1._x + fac2 * v1to2._x + fac3 * v1to3._x;
            double y = v1._y + fac2 * v1to2._y + fac3 * v1to3._y;
            double z = v1._z + fac2 * v1to2._z + fac3 * v1to3._z;
            addTransformed(x, y, z);
        }
    }

    // alternatively, create random points
    /*
    for (size_t i = 0; i < pointCount; ++i) {
        double fac2 = MathUtils::getRnd();
        double fac3 = MathUtils::getRnd();
        if (fac2 + fac3 > 1.0) {
            fac2 = 1.0 - fac2;
            fac3 = 1.0 - fac3;
        }
        double x = v1._x + fac2 * v1to2._x + fac3 * v1to3._x;
        double y = v1._y + fac2 * v1to2._y + fac3 * v1to3._y;
        double z = v1._z + fac2 * v1to2._z + fac3 * v1to3._z;
        addTransformed(x, y, z);
    }
    */
}

void ShapeGenerator::setOffsetAndRotation(const Rotation rotation) {
    // set random offset
    double offsetMin = _padding;
    double offsetMax = _spaceSize - _padding;
    _ox = MathUtils::getRnd(offsetMin, offsetMax);
    _oy = MathUtils::getRnd(offsetMin, offsetMax);
    _oz = MathUtils::getRnd(offsetMin, offsetMax);

    switch (rotation) {
    case none: {
        // set identity matrix
        _r11 = 1.0; _r12 = 0.0; _r13 = 0.0;
        _r21 = 0.0; _r22 = 1.0; _r23 = 0.0;
        _r31 = 0.0; _r32 = 0.0; _r33 = 1.0;
        break;
    }

    case paraxial: {
        size_t axis = MathUtils::getRndInt(6);
        switch(axis) {
        case 0: {
            _r11 = 1.0; _r12 = 0.0; _r13 = 0.0;
            _r21 = 0.0; _r22 = 1.0; _r23 = 0.0;
            _r31 = 0.0; _r32 = 0.0; _r33 = 1.0;
            break;
        }
        case 1: {
            _r11 = 0.0; _r12 = 0.0; _r13 = 1.0;
            _r21 = 1.0; _r22 = 0.0; _r23 = 0.0;
            _r31 = 0.0; _r32 = 1.0; _r33 = 0.0;
            break;
        }
        case 2: {
            _r11 = 0.0; _r12 = 1.0; _r13 = 0.0;
            _r21 = 0.0; _r22 = 0.0; _r23 = 1.0;
            _r31 = 1.0; _r32 = 0.0; _r33 = 0.0;
            break;
        }
        case 3: {
            _r11 = 0.0; _r12 = 0.0; _r13 =-1.0;
            _r21 = 0.0; _r22 = 1.0; _r23 = 0.0;
            _r31 = 1.0; _r32 = 0.0; _r33 = 0.0;
            break;
        }
        case 4: {
            _r11 = 1.0; _r12 = 0.0; _r13 = 0.0;
            _r21 = 0.0; _r22 = 0.0; _r23 = -1.0;
            _r31 = 0.0; _r32 = 1.0; _r33 = 0.0;
            break;
        }
        case 5: {
            _r11 = 1.0; _r12 = 0.0; _r13 = 0.0;
            _r21 = 0.0; _r22 = -1.0; _r23 = 0.0;
            _r31 = 0.0; _r32 = 0.0; _r33 = -1.0;
            break;
        }
        default: {
            assert (false); // unexpected case
            break;
        }
        } // end of switch(axis)
        break;
    }

    case arbitrary: {
        // set random unit vector
        const double eps = 0.00001;
        double n1;
        double n2;
        double n3;
        do {
            n1 = MathUtils::getRnd(-1.0, 1.0);
            n2 = MathUtils::getRnd(-1.0, 1.0);
            n3 = MathUtils::getRnd(-1.0, 1.0);
        } while (std::abs(n1) < eps && std::abs(n2) < eps
                && std::abs(n3) < eps);
        double length = getLength(n1, n2, n3);
        n1 /= length;
        n2 /= length;
        n3 /= length;

        // set random angle
        double a = MathUtils::getRnd(0, 2 * M_PI);
        double cos = std::cos(a);
        double cos1 = 1.0 - cos;
        double sin = std::sin(a);

        // set rotation matrix // see https://de.wikipedia.org/wiki/Drehmatrix
        _r11 = n1 * n1 * cos1 + cos;
        _r12 = n1 * n2 * cos1 - n3 * sin;
        _r13 = n1 * n3 * cos1 + n2 * sin;

        _r21 = n2 * n1 * cos1 + n3 * sin;
        _r22 = n2 * n2 * cos1 + cos;
        _r23 = n2 * n3 * cos1 - n1 * sin;

        _r31 = n3 * n1 * cos1 - n2 * sin;
        _r32 = n3 * n2 * cos1 + n1 * sin;
        _r33 = n3 * n3 * cos1 + cos;
        break;
    }
    default: {
        assert (false); // unexpected case
        break;
    }
    } // end of switch(rotation)
}


double ShapeGenerator::getLength(const double x, const double y,
        const double z) const {
    return std::sqrt(x * x + y * y + z * z);
}

void ShapeGenerator::addWithOffset(
        const double x, const double y, const double z) {
   addPlain(
       _ox + x + MathUtils::getRnd(-_diffusion, _diffusion),
       _oy + y + MathUtils::getRnd(-_diffusion, _diffusion),
       _oz + z + MathUtils::getRnd(-_diffusion, _diffusion)
   );
}

void ShapeGenerator::addTransformed(
        const double x, const double y, const double z) {
   addPlain(
       _ox + _r11 * x + _r12 * y + _r13 * z +
       MathUtils::getRnd(-_diffusion, _diffusion),
       _oy + _r21 * x + _r22 * y + _r23 * z +
       MathUtils::getRnd(-_diffusion, _diffusion),
       _oz + _r31 * x + _r32 * y + _r33 * z +
       MathUtils::getRnd(-_diffusion, _diffusion)
   );
}

void ShapeGenerator::addPlain(
        const double x, const double y, const double z)  {
    if (SHAPE_GEN_INSERT_ORDER == ShapeGenOrder::byShape) {
        // points are immediately inserted into the Pointcloud2
        _pc2->insert({ x, y, z });
    } else {
        // points are stored in a vector and only inserted into the
        // Pointcloud2 at finalize()
        _points.push_back({ x, y, z });
    }
}

void ShapeGenerator::finalize() {
    switch(SHAPE_GEN_INSERT_ORDER) {
    case byShape:
        return; // _pc2->insert already called in addPlain()

    case byZYX:
        if (REPORT_TO_CONSOLE) {
            cout << "sorting " << formatInt(_points.size()) << " points"
                    << endl;
        }
        std::sort(_points.begin(), _points.end());
        if (REPORT_TO_CONSOLE)
            cout << "inserting points into Pointcloud2" << endl;
        for (const PointBase<DIMENSIONS>& point : _points) {
            PcPoint pcPoint {point._coords[0], point._coords[1],
                point._coords[2]};
            _pc2->insert(pcPoint);
        }
        return;

    case random:
        size_t size = _points.size();
        if (REPORT_TO_CONSOLE) {
            cout << "mixing " << formatInt(size) << " points" << endl;
        }
        vector<size_t> indices;
        indices.reserve(size);
        for (size_t i = 0; i < size; ++i)
            indices.push_back(i);
        for (size_t i = 0; i < size; ++i) {
            size_t j = MathUtils::MathUtils::getRndInt(size);
            size_t swap = indices[i];
            indices[i] = indices[j];
            indices[j] = swap;
        }
        if (REPORT_TO_CONSOLE)
            cout << "inserting points into Pointcloud2" << endl;
        for (size_t i = 0; i < size; ++i) {
            const PointBase<DIMENSIONS>& point = _points[indices[i]];
            PcPoint pcPoint {point._coords[0], point._coords[1],
                point._coords[2]};
            _pc2->insert(pcPoint);
        }
    }
}
