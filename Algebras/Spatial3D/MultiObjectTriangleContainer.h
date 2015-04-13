/*
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
 
 01590 Fachpraktikum "Erweiterbare Datenbanksysteme" 
 WS 2014 / 2015

<our names here>

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of a Spatial3D algebra

[TOC]

1 Includes and Defines

*/

#ifndef _SPATIAL3DMULTIOBJECTTRIANGLECONTAINER_H
#define _SPATIAL3DMULTIOBJECTTRIANGLECONTAINER_H

#include "MMRTree.h"
#include "AuxiliaryTypes.h"
#include "geometric_algorithm_intersection_triangles.h"

enum TrianglePathDirection { LEAVE, ENTER };

// Class invariants:
// - objects 0 and negative are reserved for internal purposes
// - all triangles in objects != 0 are pairwise compatible
// - A TriangleData-instance is either for object 0 or for other objects

class MultiObjectTriangleContainer
{

public:
  
  typedef unsigned short int object_t;
  const static int noExternalObjects = 8;
  
  MultiObjectTriangleContainer();

  int noTriangles() const;
  
  // Return true on success, false on error.
  // If corrections == true, always returns true.
  bool addTriangle(const Triangle& triangle, int object, bool corrections);

  void removeObject(int object);

  void mergeObject(int fromObject, int toObject);

  void exportObject(int object, vector<Triangle>& out) const;

  // Return true if volume is valid or has been made valid.
  // If corrections == false, some corrections are tried, but might fail.
  bool checkVolume(int object, bool corrections);

  
  
  
  struct Edge3d {
    size_t startPoint;
    size_t endPoint;
  };
  
  // get the holes in a given objects
  // (cycles of edges which belong only to one triangle)
  // the result will be provided in result as a list of holes
  // which in turn are a list of point indices (path).
  // if the edge-triangles are correctly orientated, the resulting 
  // hole edges will be orientated the in the opposite direction.
  // (i.e. the way the patch for the hole should be orientated)
  // returns true for any holes (or other edges with odd triangle count)
  // the last point and the first point are considered to be 
  // also forming an edge
  bool getHoles (int object, std::vector<std::vector<size_t> >& result);
  
  
  // helpers for getHoles
  void getLonelyEdges (
    int object,
    std::vector<Edge3d>& edges,
    std::multimap<size_t,size_t>& edgeIndex);
  
  void formHolesFromEdges(int object, std::vector<Edge3d>& edges,
                          std::multimap<size_t,size_t>& edgeIndex,
                          std::vector<std::vector<size_t> >& result);
    
  // fill the holes in the provide object
  // holes are provided as received from getHoles
  // (i.e. vector of vector of point indices)
  // returns true if successfully
  bool fillHoles (int object, const std::vector<std::vector<size_t> >& holes);
  
  //helpers for fillHoles
  void getPatchForHoleRecursion(std::vector<size_t>::const_iterator first,
                                std::vector<size_t>::const_iterator last,
                                // points after last element
                                std::vector<std::vector<size_t> >& triangles);
  std::vector<std::vector<size_t> > getPatchForHole(
    int object,
    const std::vector<size_t>& holes);
  
  // make object valid by
  // - deleting excessive triangles
  // -> triangles inside volume
  // -> triangles with unsatisfied edges
  // - correcting the orientation
  // precondition: object is compatibalized
  // postcondition: object is valid (may mean you no longer have any triangles)
  // will return true on any correction done
  bool orientateAndCleanup (int object);
  
  // helpers for orientateAndCleanup:
  enum TriangleStatus { UNKNOWN, 
    DELETE, // to be deleted
    OK, // absolutely part of volume (at the current processing state) 
    FLIP, // part of volume, but needs to be flipped
    TMP_OK, // if currently built component of object turns out to be valid,
            // this triangle will be part of it
    TMP_FLIP, // as above, but needs to be flipped
    TMP_VISITED, // this triangle has been seen, 
                 // but not sure if it will be part of the volume
    TMP_DELETE, // marker for triangle to be deleted
    OTHER // this triangle belongs to another object - ignore
  };
  bool hasLonelyEdge(size_t index, std::vector<TriangleStatus>& tri_state);
  void deletePatch(size_t index, std::vector<TriangleStatus>& tri_state);
  void makePermanent(std::vector<TriangleStatus>& tri_state);
  void deleteAllConnectedTriangles(size_t index,
                              std::vector<TriangleStatus>& tri_state);
  void deletePatchRecursively(size_t index,
                              std::vector<TriangleStatus>& tri_state);
  
  void createVolume(size_t seed_triangle,
                    std::vector<TriangleStatus>& tri_state,
                    int object);
  
  bool createVolumeRecursively(size_t seed_triangle,
                               std::vector<TriangleStatus>& tri_state,
                               int object);
  
  //true iff found maximizing triangle.
  bool getMaximizingTriangle(size_t seed_triangle,
                             std::vector<size_t>& trisForEdge,
                             std::vector<TriangleStatus>& tri_state,
                             int object,
                             size_t& result_triangle);
  
  size_t getOrientatedOuterOrInnerTriangle(
    size_t first_unknown,
    std::vector<TriangleStatus>& tri_state,
    int object, 
    bool& isOuter);
  
  vector<size_t> getTrianglesForEdge(size_t point1,
                                     size_t point2,
                                     vector<TriangleStatus>& tri_state);  
  
  // remove triangle from object. no checks done, be careful!
  void removeTriangleFromObject(int object, size_t triangle);
    
  bool checkVolume_closed(int object, bool corrections);
  bool checkVolume_orientation(int object, bool corrections);
  bool checkVolume_unconnected_inner_components(int object, bool corrections);
  bool checkVolume_connected_inner_components(int object, bool corrections);

  void prepareSetOperationSurface(int in_object1, int in_object2,
                                  int out_object_commonSurface,
                                  int out_object_only1,
                                  int out_object_only2);
  
  void prepareSetOperationVolume(int in_object1, int in_object2,
                                 int out_object_commonSame,
                                 int out_object_commonOpposite,
                                 int out_object_only1_outside2,
                                 int out_object_only2_outside1,
                                 int out_object_only1_inside2,
                                 int out_object_only2_inside1);

  // Preconditions:
  // - at least one triangle in fromObject
  // - no triangle in toObject
  // Postconditions:
  // - exactly one surface-connected component moved to toObject.
  void extractSurfaceComponent(int fromObject, int toObject);

  bool isPointInsideVolume(SimplePoint3d& point, int volume) const;

  void test();
  void printStats() const;

private:

  // Internal types
  
  struct TriangleData
  {
    TriangleData(size_t p1, size_t p2, size_t p3);
    
    // references member points
    size_t points[3];
    // if this triangle is part of object n, then bit 1 << n is set.
    object_t object_membership;
    // if this triangle is part of object n, but in opposite direction,
    // then bit 1 << n is set.
    object_t object_membership_direction;
    
    bool isMemberOfObject(int object) const;
    void setMembershipOfObject(int object, bool isMember);
    
    bool getDirectionOfObject(int object) const;
    void setDirectionOfObject(int object, bool oppositeDirection);
  };
  
  enum TriangleDataComparisonResult {
    SAME = 0, OPPOSITE = 1, SHARED_EDGE = 2, SHARED_CORNER = 3, DIFFERENT = 4 };
  enum TriangleSearchResult { NEW, KNOWN_SAME, KNOWN_OPPOSITE, INCOMPATIBLE };

  struct SortedEdge
  {
    size_t edgeP1, edgeP2;

    Plane3d projectionPlane;
    Projection2d projection;

    SortedEdge(size_t _edgeP1, size_t _edgeP2, size_t xPoint,
               const vector<SimplePoint3d>& points);

    size_t getThirdPoint(const size_t trianglePoints[]);
    TrianglePathDirection getDirection(const size_t trianglePoints[]);
  };

  struct SortedTriangle
  {
    SortedEdge edge;
    size_t triangle;
    TriangleData triangleData;
    SimplePoint3d thirdPoint;
    double phi;
    TrianglePathDirection direction;

    SortedTriangle(SortedEdge _edge, size_t _triangle,
                   const TriangleData _triangleData,
                   const vector<SimplePoint3d>& points);
  
    TrianglePathDirection getDirectionForObject(int object);
  
    bool isInObject(int object);
  };
  
  static inline bool triangleSort(const SortedTriangle& t1,
                                  const SortedTriangle& t2)
  {
    return t1.phi < t2.phi;
  }

  // Members

  // Iteration may only use the R-Trees, never the basic vectors, as the vectors
  // contain stale data.

  vector<SimplePoint3d> points;
  mmrtree::RtreeT<3, size_t> points_tree;

  vector<TriangleData> triangles;
  mmrtree::RtreeT<3, size_t> triangles_tree;
  stack<size_t> unused_triangle_indices;
  vector<size_t> trianglesObject0;

  // Methods

  // Container querys
  Triangle getTriangle(const TriangleData& data, int object) const;
  Triangle getTriangle(const TriangleData& data) const;

  // returns true if same direction, false if opposite direction
  bool getTriangleEdgeDirection(const TriangleData& data,
                                size_t point1, size_t point2) const;
  
  Rectangle<3> getSegmentBoundingBox(size_t point1, size_t point2) const;
  Rectangle<3> getTriangleBoundingBox(size_t triangle) const;
  
  TriangleSearchResult getAnIncompatibleTriangle(
            size_t triangle,
            size_t& out_incompatibleTriangle,
            spatial3d_geometric::TriangleIntersectionResult& out_intersection);

  set<size_t> getEdgesForPoint(size_t point) const;
  
  vector<size_t> getTrianglesForPoint(size_t point) const;

  vector<size_t> getTrianglesForEdge(size_t point1, size_t point2) const;

  TriangleDataComparisonResult compare(const TriangleData& t1,
                                       const TriangleData& t2) const;

  bool isEdgeOf(size_t point1, size_t point2, size_t triangle) const;
  
  // Container manipulation  
  bool addTriangle(size_t points[], int object, bool corrections);

  bool moveObject0(int toObject, bool corrections);
  
  // checks for redundancy: may return index of an existing point
  size_t addPoint(const SimplePoint3d& point);

  // Low-level access to the data structure: no checks.
  // Only call for valid triangles free of conflicts.
  size_t addTriangle(const TriangleData& triangle);
  void removeTriangle(size_t triangle);
  void addExistingTriangleToObject(TriangleData& triangleData,
                                   int object,
                                   bool oppositeDirection);

  // Triangle splitting primitives
  void splitEdge(size_t edgeP1, size_t edgeP2, size_t splitP,
                 vector<vector<size_t> >& tracked_partitions);
  void splitSurface(size_t triangle, size_t surfacePoint,
                    vector<vector<size_t> >& tracked_partitions);
  
  void cutTriangles2d(size_t t0, size_t t1,
                      vector<size_t>& o0, vector<size_t>& o1,
                      vector<size_t>& both);

  // Does some splitting of the triangle. Not always everything!
  // Precondition: segment of point1,point2 is in the plane of the triangle
  // and incompatible.
  bool splitTriangleForSegment(size_t triangle, size_t point1, size_t point2,
                               vector<vector<size_t> >& tracked_partitions);
  
  size_t GetIndexOfHighestNotUsedTriangle(
    std::vector<TriangleStatus>& tri_state);
  void TriangleGetZInOrder(const Triangle& triangle,
                             double& z1, double& z2, double& z3);
  bool IsOutside(Triangle& triangle,
    std::vector<TriangleStatus>& tri_state);
};

#endif