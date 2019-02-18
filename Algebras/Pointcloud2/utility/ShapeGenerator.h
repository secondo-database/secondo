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

#pragma once

#include "MathUtils.h"
#include "PointBase.h"
#include "PcPoint.h"
#include "StlFacet.h"

#include "../tcPointcloud2.h"


namespace pointcloud2 {

enum ShapeGenOrder {
    byShape,
    random,
    byZYX
};

/* the order in which the points should be inserted into the Pointcloud */
constexpr ShapeGenOrder SHAPE_GEN_INSERT_ORDER = ShapeGenOrder::byZYX;

class ShapeGenerator {
public:
    enum Rotation {
        none, // no rotation at all
        paraxial, // rotation to a direction parallel to the x, y, or z axis
        arbitrary // rotation in any direction
    };

private:
    // settings

    /* whether and how planes should be rotated */
    static const Rotation ROTATE_PLANES = Rotation::arbitrary;
    /* whether and how spheres should be rotated (obviously, there is no
     * point in rotating an actual sphere, however, we are dealing with
     * a finite number of points in the shape of a sphere!) */
    static const Rotation ROTATE_SPHERES = Rotation::arbitrary;
    /* whether and how cylinders should be rotated */
    static const Rotation ROTATE_CYLINDERS = Rotation::paraxial;
    /* whether and how cones should be rotated */
    static const Rotation ROTATE_CONES = Rotation::paraxial;

    /* whether the location of the shapes should be reported to the
     * console */
    static const bool REPORT_TO_CONSOLE = true;

    /* --------------------------------------------------- */

    /* the Pointcloud2 into which the generated points should be inserted */
    Pointcloud2* _pc2;

    /* the number of planes to be generated */
    const unsigned _planeCount;
    /* the number of spheres to be generated */
    const unsigned _sphereCount;
    /* the number of cylinders to be generated */
    const unsigned _cylinderCount;
    /* the number of cones to be generated */
    const unsigned _coneCount;

    /* the regular distance between two points */
    const double _pointDist;
    /* the minimum size of a shape in any dimension */
    const double _shapeSizeMin;
    /* the maximum size of a shape in any dimension */
    const double _shapeSizeMax;
    /* the size of the available space in every dimension
     * (i.e. generated points are within [0, _spaceSize] for
     * each dimension) */
    const double _spaceSize;

    /* the maximum diffusion added to or subtracted from each coordinate
     * of a point */
    const double _diffusion;
    /* the number of noise points to be generated */
    const unsigned _noisePointCount;

    /* the seed for the random number generator, or 0 if the current
     * system time should be used as a seed. */
    const unsigned _rndSeed;

    /* if the center of a shape is in _padding distance from the border,
     * the shape is guaranteed to be within _spaceSize */
    double _padding;

    /* a vector of facets (i.e. triangles) imported from an StL file */
    std::shared_ptr<std::vector<StlFacet>> _facets;

    /* the number of points to be generated from the facets */
    size_t _facetPointCount = 0;

    /* whether to rotate facets */
    Rotation _facetsRotation = Rotation::none;

    /* the offset to which the current shape is moved */
    double _ox = 0.0, _oy = 0.0, _oz = 0.0;

    /* the rotation matrix used to rotate the current shape */
    double _r11 = 1.0, _r12 = 0.0, _r13 = 0.0;
    double _r21 = 0.0, _r22 = 1.0, _r23 = 0.0;
    double _r31 = 0.0, _r32 = 0.0, _r33 = 1.0;

    /* the points that were generated and, depending on MIX_POINTS,
     * need to be mixed or sorted before inserting them into the
     * Pointcloud2 */
    std::vector<PointBase<DIMENSIONS>> _points;

public:
    /* creates a ShapeGenerator with the given settings */
    ShapeGenerator(Pointcloud2* pc2,
            const unsigned planeCount,
            const unsigned sphereCount,
            const unsigned cylinderCount,
            const unsigned coneCount,
            const double pointDist,
            const double shapeSizeMin,
            const double shapeSizeMax,
            const double spaceSize,
            const double diffusion,
            const unsigned noisePointCount,
            const unsigned rndSeed);

    /* creates a ShapeGenerator with the given settings */
    ShapeGenerator(Pointcloud2* pc2,
            const std::shared_ptr<std::vector<StlFacet>> facets,
            const size_t pointCount,
            const Rotation rotation,
            const double diffusion,
            const unsigned rndSeed);

    ~ShapeGenerator() = default;

    /* generates shapes and adds them to the Pointcloud2 given in the
     * constructor */
    void generate();

private:
    /* generates a single plane */
    void generatePlane();

    /* generates a single sphere */
    void generateSphere();

    /* generates a single cylinder */
    void generateCylinder();

    /* generates a single cone */
    void generateCone();

    /* generates a single point of noise */
    void generateNoise();

    /* generates a single facet of an StL file */
    void generateFacet(const StlFacet& facet, const double totalFacetArea);

    /* sets the offset variables _o... and the rotation matrix _r...
     * for the next shape */
    void setOffsetAndRotation(const Rotation nrotation);

    /* returns the length of the given vector */
    double getLength(const double x, const double y, const double z) const;

    /* inserts the given point into the Pointcloud2 without moving it,
     * rotating it, or adding diffusion */
    void addPlain(const double x, const double y, const double z);

    /* moves the given point by the offset calculated in
     * setOffsetAndRotation(), adds diffusion
     * and inserts the resulting point into the Pointcloud2 */
    void addWithOffset(const double x, const double y, const double z);

    /* rotates the given point with the rotation matrix and moves it by
     * the offset calculated in setOffsetAndRotation(), adds diffusion,
     * then inserts the resulting point into the Pointcloud2 */
    void addTransformed(const double x, const double y, const double z);

    /* finalizes the add operations; if the vector _points was used to
     * store points, these points will now be mixed or sorted depending
     * on INSERT_ORDER and only then inserted to the Pointcloud2 */
    void finalize();
};

} /* namespace pointcloud2 */
