/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or(at your option) any later version.

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

#include "MultiObjectTriangleContainer.h"
#include "Spatial3D.h"
#include "AuxiliaryTypes.h"
#include "geometric_algorithm.h"
#include "geometric_algorithm_intersection_triangles.h"
#include "geometric_algorithm_intersection_line_plane.h"

#include<memory>
#include<limits>

using namespace spatial3d_geometric;

const bool debug_volume = false;
const bool debug_splitting = false;

ostream& operator<< (ostream& os, MultiObjectTriangleContainer::Edge3d edge) {
  return os << "(" << edge.startPoint << "->" << edge.endPoint << ")"; 
}

void MultiObjectTriangleContainer::test()
{
  printStats();
  
  auto_ptr<mmrtree::RtreeT<3, size_t>::iterator> it(triangles_tree.entries());
  
  size_t const * triangleIndex;
  while( (triangleIndex = it->next()) != 0)
  {
    const TriangleData& data = triangles[*triangleIndex];
    {
      cerr << data.object_membership << " " << getTriangle(data) << endl;
    }
  }
}

void MultiObjectTriangleContainer::printStats() const
{
  cerr << "OOOOOOOOOOXXXXXXX" << endl;
  cerr << "Size of points vec:    " << points.size() << endl;
  cerr << "Size of triangle vec:  " << triangles.size() << endl;
  cerr << "Size of unused tri:    " << unused_triangle_indices.size() << endl;
  cerr << "Size of object 0:      " << trianglesObject0.size() << endl;
  cerr << "Size of points tree:   " << points_tree.noObjects() << endl;
  cerr << "Size of triangle tree: " << triangles_tree.noObjects() << endl;
}

//////////////////////////////////////////////////////////////////////////
// helper functions //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MultiObjectTriangleContainer::object_t getObjectBitmask(int object)
{
  if (object < 0)
  {
    object = MultiObjectTriangleContainer::noExternalObjects - object;
  }
  return 1 << object;
}

TrianglePathDirection
operator!(TrianglePathDirection dir)
{
  if (dir == LEAVE)
    return ENTER;
  else
    return LEAVE;
}


//////////////////////////////////////////////////////////////////////////
// public interface //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MultiObjectTriangleContainer::MultiObjectTriangleContainer()
  : points(), points_tree(4, 8), triangles(), triangles_tree(4,8),
    unused_triangle_indices(), trianglesObject0()
  {  }

int MultiObjectTriangleContainer::noTriangles() const
{
  return triangles_tree.noObjects();
}
  
// Return true on success, false on error
bool MultiObjectTriangleContainer::addTriangle(const Triangle& triangle,
                                               int object,
                                               bool corrections)
{
  if (debug_splitting) cerr << "addTriangle: " << triangle << endl;
  assert(object > 0);
  assert(object <= noExternalObjects);
  
  size_t points[3];
  points[0] = addPoint(triangle.getA());
  points[1] = addPoint(triangle.getB());
  points[2] = addPoint(triangle.getC());
  
  try
  {
    bool result = addTriangle(points, object, corrections);
    return result;
  }
  catch (NumericFailure f)
  {
    removeObject(0);
    cerr << "Numeric problem, could not add triangle." << endl;
    return false;
  }
}

void MultiObjectTriangleContainer::removeObject(int object)
{
  object_t object_bits = getObjectBitmask(object);

  vector<size_t> trianglesToBeDeleted;
  
  if (object == 0)
  {
    trianglesToBeDeleted = trianglesObject0;
  }
  else
  {
   auto_ptr<mmrtree::RtreeT<3, size_t>::iterator> it(triangles_tree.entries());
  
    size_t const * triangleIndex;
    while( (triangleIndex = it->next()) != 0)
    {
      TriangleData& data = triangles[*triangleIndex];
      data.object_membership &= ~object_bits;
      if (data.object_membership == 0)
      {
        trianglesToBeDeleted.push_back(*triangleIndex);
      }
    }
  }
  for (vector<size_t>::iterator it = trianglesToBeDeleted.begin();
       it != trianglesToBeDeleted.end(); ++it)
  {
    removeTriangle(*it);
  }
}

void MultiObjectTriangleContainer::exportObject(int object,
                                                vector<Triangle>& out) const
{
  assert(object > 0);
  assert(object <= noExternalObjects);

  object_t object_bits = getObjectBitmask(object);
  
  auto_ptr<mmrtree::RtreeT<3, size_t>::iterator> it(triangles_tree.entries());
  
  size_t const * triangleIndex;
  while( (triangleIndex = it->next()) != 0)
  {
    const TriangleData& data = triangles[*triangleIndex];
    if ((data.object_membership & object_bits) != 0)
    {
      // Triangle is member of the object ...
      out.push_back(getTriangle(data, object));
    }
  }
}

// if corrections is enabled, will return true if we could repair the object
// if corrections is disabled, will return true
//    iff provided object is correct without repair
bool MultiObjectTriangleContainer::checkVolume(int object, bool corrections)
{
  if (debug_volume) cerr << "Entering checkVolume(object=" << object
                         << ", corrections=" << corrections << ")\n";
  
  std::vector<std::vector<size_t> > holes(0);
  bool found_holes = MultiObjectTriangleContainer::getHoles (object, holes);
  if (debug_volume) cerr << (found_holes?"found holes\n":"no holes\n");
  bool filled_holes = MultiObjectTriangleContainer::fillHoles (object, holes);
  if (debug_volume) cerr << (filled_holes ? "filled_holes OK\n"
                                          : "filled_holes NOK... Error\n");
  bool corrected_volume 
    = MultiObjectTriangleContainer::orientateAndCleanup (object);
  if (debug_volume) cerr << (corrected_volume ? "corrected_volume\n"
                                              : "no corrected_volume\n");
    
  bool old_check_vol=  checkVolume_closed(object, corrections);
  if (debug_volume) cerr << (old_check_vol ? "old_check_vol OK\n"
                                           : "NOK: old_check_vol\n");

  bool we_repaired_something = found_holes || corrected_volume;
  // corrections always "works", so just check if we had to repair anything
  bool result = (corrections || !we_repaired_something) ;
  result = result && filled_holes; // just learned: fill holes might fail...
  if (debug_volume) cerr << "Leaving checkVolume(): result=" << result 
                         << " we_repaired_something=" << we_repaired_something 
                         << " corrections=" << corrections << endl;
  
  return result;
}

bool MultiObjectTriangleContainer::orientateAndCleanup (int object) {
  // wir gehen mal davon aus, dass die meisten Dreiecke zum objekt gehören...
  // weil wir ja nur beim Import aufgerufen werden....
  
  // hilfstruktur für status der Dreiecke initialisieren
  // nicht alle dreiecke im triangles vector sind notwendig teil des object
  // also erst mal alle auf OTHER und dann per tree-iterator die richtigen
  // raussuchen
  if (debug_volume) cerr << "entering orientateAndCleanup\n";
  std::vector<TriangleStatus> tri_state (triangles.size(), OTHER);
   
  auto_ptr<mmrtree::RtreeT<3, size_t>::iterator> it(triangles_tree.entries());
  size_t const * triangleIndex;
  while( (triangleIndex = it->next()) != 0)
  {
    if (triangles[*triangleIndex].isMemberOfObject(object))
    {
      if (debug_volume) cerr << "U: " << *triangleIndex << endl;
      tri_state[*triangleIndex] = UNKNOWN; // part of our object
    } else {
      if (debug_volume) cerr << "O: " << *triangleIndex << endl;
    }
  }

  // now delete recursively all triangles with a lonely edge
  for(size_t index = 0; index < tri_state.size(); ++index ) {
    if (hasLonelyEdge(index, tri_state)) {
      deletePatch(index, tri_state);
    }
  }
   
  // get next correctly orientated "seed" triangle
  size_t first_unknown = 0;
  while (first_unknown < tri_state.size()) {
    if (tri_state[first_unknown] == UNKNOWN) {
      bool isOuter = false;
      size_t seed_triangle = getOrientatedOuterOrInnerTriangle(
        first_unknown, tri_state, object, isOuter);
      
        if (debug_volume) cerr << "isOuter"<< isOuter << endl;
      if (isOuter) {
        createVolume(seed_triangle, tri_state, object);
      } else {
        // we might assume that the enclosing component has already been built
        // so deleting everything connected to this triangle 
        // (except finally accepted ones) might also be OK.
        // But to be sure, just delete the patch.
        deletePatch(seed_triangle, tri_state);
      }
    } else {
      ++first_unknown; // we might have used a different seed before
    }
  }
  // solange noch unbearbeitete dreiecke da
  
  // äußeres dreieck ermitteln
  // körperschluss probieren
  // falls OK: Dreiecke als gut markieren & Nachbarn löschen
  // falls NOK: startDreieck löschen
  // Beim Donut: unterschiedliches Ergebnis abh. von start dreieck
   
  
  /*
    IntersectionPointResult intersection(const SimplePoint3d& p0,
                                       const Vector3d& segmentVector,
                                       const Triangle& triangle)
  */        
  
  //write out changes to container
 bool we_changed_the_object = false;
 for(size_t index = 0; index < tri_state.size(); ++index ) {
   if (debug_volume) cerr << "tri_state[" << index << "]=";
   switch (tri_state[index]) {
     case DELETE:
       if (debug_volume) cerr << "DELETE\n";
       we_changed_the_object = true;
       removeTriangleFromObject(object, index);
       break;
     case OK:
       if (debug_volume) cerr << "OK\n";
       if (triangles[index].getDirectionOfObject(object)) {
         we_changed_the_object = true;
         triangles[index].setDirectionOfObject(object, false);    
       }
       break;
     case FLIP:
       if (debug_volume) cerr << "FLIP\n";
       if (!triangles[index].getDirectionOfObject(object)) {
         we_changed_the_object = true;
         triangles[index].setDirectionOfObject(object, true);    
       }
       break;   
     case UNKNOWN:
       if (debug_volume) cerr << "UNKNOWN\n";
       break;
     case OTHER:
       if (debug_volume) cerr << "OTHER\n";
       break;  
     default:
       if (debug_volume) cerr << "DEFAULT("<< tri_state[index] <<")\n";
   }
 }
 
  
  if (debug_volume) cerr << "leaving orientateAndCleanup\n";
  return we_changed_the_object;
}

/*
1.1 ~TriangleGetZInOrder~

Gets the Z values of a triangle in descending order.
 
*/
void MultiObjectTriangleContainer::TriangleGetZInOrder(
  const Triangle& triangle,
  double& z1, double& z2, double& z3)
{
  double zTemp1 = triangle.getA().getZ();
  double zTemp2 = triangle.getB().getZ();
  double zTemp3 = triangle.getC().getZ();
  z1 =  max(zTemp1,max(zTemp2,zTemp3));
  if(z1 == zTemp1){
    z2 = max(zTemp2,zTemp3);
    z3 = min(zTemp2,zTemp3);
  }
  if(z1 == zTemp2){
    z2 = max(zTemp1,zTemp3);
    z3 = min(zTemp1,zTemp3);
  }
  if(z1 == zTemp3){
    z2 = max(zTemp1,zTemp2);
    z3 = min(zTemp1,zTemp2);
  }
}

  /*
  1.1 ~GetIndexOfHighestNotUsedTriangle~

  Gets the triangle with the highest Z values, that was not considert before.
  
  */
size_t MultiObjectTriangleContainer::GetIndexOfHighestNotUsedTriangle(
  std::vector<TriangleStatus>& tri_state){   
  double maxZ1;
  double maxZ2;
  double maxZ3;
  double z1;
  double z2;
  double z3;  
  size_t maxIndex = 0;

  auto_ptr<mmrtree::RtreeT<3, size_t>::iterator> it(triangles_tree.entries());
  size_t const * triangleIndex;
  while( (triangleIndex = it->next()) != 0)
  {
    if((tri_state[*triangleIndex] != DELETE)
      && (tri_state[*triangleIndex] != OTHER)
      &&(tri_state[*triangleIndex] != OK) 
      &&(tri_state[*triangleIndex] != FLIP)
      &&(tri_state[*triangleIndex] != TMP_OK)
      &&(tri_state[*triangleIndex] != TMP_FLIP)
      &&(getTriangle(triangles[*triangleIndex]).
      getNormalVector().getZ() != 0.0))
    {
      TriangleGetZInOrder(getTriangle(triangles[*triangleIndex]),z1,z2,z3);
      if( maxIndex == 0 || z1 > maxZ1){
        maxIndex = *triangleIndex;
        maxZ1 = z1;
        maxZ2 = z2;
        maxZ3 = z3;
      }
      if(maxZ1 == z1 && maxZ2 < z2){
        maxIndex = *triangleIndex;
        maxZ2 = z2;
        maxZ3 = z3;
      }
      if(maxZ1 == z1 && maxZ2 == z2 && maxZ3 < z3){
        maxIndex = *triangleIndex;
        maxZ3 = z3;
      }
    }
  }
  return maxIndex;
}


  /*
  1.1 ~IsOutside~

  Checks whether a triangle is not inside a volume.
  
  */
bool MultiObjectTriangleContainer::IsOutside(Triangle& triangle,
  std::vector<TriangleStatus>& tri_state){   
  size_t triangleIndex;
    
  bool found = false;
  int runNr = 1;
  
  while(!found && runNr < 100)
  {
    runNr++;
    int intersectionCount = 0;
    SimplePoint3d p( (1.0 - 1.0/runNr) * ((1.0/runNr) * triangle.getA().getX()  
      + (runNr - 1.0/runNr) * triangle.getB().getX()) 
      + (1.0/runNr) * triangle.getC().getX(),(1.0 - 1.0/runNr)
          * ((1.0/runNr) * triangle.getA().getY()  
      + (runNr - 1.0/runNr) * triangle.getB().getY()) 
      + (1.0/runNr) * triangle.getC().getY(),(1.0 - 1.0/runNr)
          * ((1.0/runNr) * triangle.getA().getZ()  
      + (runNr - 1.0/runNr) * triangle.getB().getZ()) 
      + (1.0/runNr) * triangle.getC().getZ());

    auto_ptr<mmrtree::RtreeT<3, size_t>::iterator> it(triangles_tree.entries());
    bool nextNr = false;
    while( (triangleIndex = *(it->next())) != 0 && !nextNr)
    {
      if((tri_state[triangleIndex] != DELETE)
        && (tri_state[triangleIndex] != OTHER)
        &&(tri_state[triangleIndex] != TMP_OK)
        &&(tri_state[triangleIndex] != TMP_FLIP))
      {
        
        Vector3d v(0.0,0.0,1.0);
        const Triangle& t = getTriangle(triangles[triangleIndex]);
        
        IntersectionPointResult  intersectionType = intersection(p, v, t);
        
        if (intersectionType.intersectionParameter > 0.00000001  && 
          intersectionType.resultType == IntersectionPointResult::INNER)
        {
          intersectionCount++;
        }else if(intersectionType.rayIntersects()  && 
          intersectionType.resultType == IntersectionPointResult::EDGE)
        {
            nextNr = true;
        }
      }
    }
    if(!nextNr){
        if(intersectionCount % 2 == 0){
          return true;
        }
      return false;
    }
  }
  cerr << "this should never happen !!!" << endl;
  return false;
}


  /*
  1.1 ~getOrientatedOuterOrInnerTriangle~

  Gets a triangle that was not cosidert before, 
  and checks whether the triangle is not inside a volume 
  and has the right orientation.
  
  */
size_t MultiObjectTriangleContainer::getOrientatedOuterOrInnerTriangle(
  size_t first_unknown,
  std::vector<TriangleStatus>& tri_state,
  int object,
  bool& isOuter){
  int first = GetIndexOfHighestNotUsedTriangle(tri_state);
  Triangle triangle = getTriangle(triangles[first]);
  if(triangle.getNormalVector().getZ() > 0.0){
    tri_state[first] = TMP_OK;
  }else{
    tri_state[first] = TMP_FLIP;
  }
  isOuter = IsOutside(triangle,tri_state);
  return first; 
}


// triangle[index] is considered to have the correct orientation 
// TODO: clarify if we are really completely outside
// ... we might just be on the surface and handle innner objects later on
// and to be outside of everything else.

void MultiObjectTriangleContainer::createVolume(
  size_t seed_triangle, std::vector<TriangleStatus>& tri_state, int object) {

  if (debug_volume) cerr << "cV: seed = " << seed_triangle << endl;
  
  bool success = createVolumeRecursively(seed_triangle, tri_state, object);
  if (success) {
    for(size_t index = 0; index < tri_state.size(); ++index ) {
      if (tri_state[index] == TMP_VISITED) {
        // we've seen those triangles, but they are not part of the surface
        // so they must be inside... Exterminate...
        deletePatchRecursively(index, tri_state);
      }
    }
  } else {
    // if the surface does not form a valid object 
    // delete all connected triangles
    // only deleting a patch may lead to different objects depending
    // on the order of insertion:
    // "donut with round piece of paper"
    deleteAllConnectedTriangles(seed_triangle, tri_state);
  }
  
  makePermanent(tri_state);
}

// seed_triangle is correctly orientated.
// for every edge get the volume maximizing triangle
// then check if it can be used and recursively continue
// otherwise break and get back...

bool MultiObjectTriangleContainer::createVolumeRecursively(
        size_t seed_triangle,
        std::vector<TriangleStatus>& tri_state,
        int object){
  
  if (debug_volume) cerr << "cVR: seed = " << seed_triangle << endl;
  
  TriangleData data = triangles[seed_triangle];
  if (tri_state[seed_triangle] == TMP_FLIP) {
    data = TriangleData(data.points[2], data.points[1], data.points[0]);
  }
  
  for (int c = 0; c < 3; ++c)
  {
    
    SortedEdge edge(data.points[c], data.points[(c + 1) % 3],
                    data.points[(c + 2) % 3], points);
    
    vector<size_t> neighbour_triangles = getTrianglesForEdge (
      data.points[c],
      data.points[(c+1) % 3],
      tri_state);
    
    if (debug_volume)
    {
      cerr << "p1=" << data.points[c] << " p2=" << data.points[(c+1) % 3]
           << " tFE.size=" << neighbour_triangles.size() << endl;
    }
    
    vector<SortedTriangle> sorted_triangles;
    
    for (vector<size_t>::iterator neighbourIndex
      = neighbour_triangles.begin();
    neighbourIndex != neighbour_triangles.end(); ++neighbourIndex)
    {
      const TriangleData neighbour = triangles[*neighbourIndex];
      sorted_triangles.push_back(SortedTriangle(edge, *neighbourIndex,
                                                neighbour, points));
      switch (tri_state[*neighbourIndex]) {
        case UNKNOWN:
          tri_state[*neighbourIndex] = TMP_VISITED;
          //fallthrough
        default:
          ;
      }
      if (debug_volume)
      {
        cerr << "NIdx=" << *neighbourIndex 
             << " ST.triangle=" << sorted_triangles.back().triangle
             << " tri_state=" << tri_state[*neighbourIndex]
             << endl;
      }
    }
    std::sort(sorted_triangles.begin(), sorted_triangles.end(),
              triangleSort);
    
    //now we have sorted triangles...
    //lets get the next one 
    vector<SortedTriangle>::iterator next_triangle;
    for(vector<SortedTriangle>::iterator sortedNeighbour
      = sorted_triangles.begin();
    sortedNeighbour != sorted_triangles.end(); ++sortedNeighbour)
    { 
      if (sortedNeighbour->triangle == seed_triangle) {
        if (debug_volume) cerr << "seed->";
        next_triangle = sortedNeighbour;
        ++next_triangle;
        if (next_triangle == sorted_triangles.end()) {
          next_triangle = sorted_triangles.begin();
        }  
      }
      if (debug_volume) cerr << "SN.triangle=" << sortedNeighbour->triangle 
                             << " phi=" << sortedNeighbour->phi <<endl;
    }
    size_t next_tri_idx = next_triangle->triangle;
    if (debug_volume) cerr << "NT=" << next_tri_idx << endl;
    
    //correctly orientated seed (container wise): TriangleData data
    //correctly orienatated edge in direction of data: edge
    
    //calculate target orientation for next_tri_idx
    // smth like 
    
    bool next_tri_in_container_has_correct_orientation
      = MultiObjectTriangleContainer::getTriangleEdgeDirection(
        triangles[next_tri_idx],
        edge.edgeP2, edge.edgeP1); // reverse points to have compatible edge
    
    TriangleStatus next_tri_status 
      = next_tri_in_container_has_correct_orientation?TMP_OK:TMP_FLIP;
    
    if (debug_volume) cerr << "tri_state[" << next_tri_idx << "]=" 
                           << tri_state[next_tri_idx] << " ";

    switch (tri_state[next_tri_idx]) {
      case UNKNOWN:
      case TMP_VISITED:
        if (debug_volume) cerr << "UNKNOWN, TMP_VISITED\n";
        // orientate and recursion
        tri_state[next_tri_idx] = next_tri_status;
        
        {
          if (debug_volume)
          {
            cerr << "recursion for next_tri_idx=" << next_tri_idx << endl;
          }
          bool our_success 
             = createVolumeRecursively(next_tri_idx, tri_state, object);
          if (debug_volume)
          {
            cerr << "recursion for next_tri_idx=" << next_tri_idx 
                 << " result=" << our_success << endl;
          }
          if (!our_success) { 
            return false;
          }
        }
        // bail out if that didn't work
        break;
      case TMP_OK:
      case TMP_FLIP:
        if (debug_volume) cerr << "TMP_OK, TMP_FLIP\n";
        if (tri_state[next_tri_idx] != next_tri_status) {
          if (debug_volume) cerr << "incompatible edge found!!!!\n";
          return false;
        }
        // check and bail out if incopatible
        break;
      default:
        if (debug_volume) cerr << "DEFAULT\n";
        // shouldn't get here TODO: check if that is really true...
    }   
  }
  
  return true;
}

void MultiObjectTriangleContainer::deleteAllConnectedTriangles(
  size_t index, std::vector<TriangleStatus>& tri_state) {
  if (debug_volume)
  {
    cerr << "dACT idx=" << index << " state=" << tri_state[index] << endl;
  }
  if (tri_state[index] == DELETE) { // shouldn't get here...
    if (debug_volume) cerr << "already deleted: " << index << endl;
    return;
  }
  if (tri_state[index] == OTHER) {
    if (debug_volume) cerr << "part of other object: " << index << endl;
    return;
  }  
  if (tri_state[index] == TMP_DELETE) { // needed to end recursion!
    // getTrianglesForEdge ignores DELETE triangles,
    //so we might run into problems there...
    if (debug_volume) cerr << "already tmp_deleted: " << index << endl;
    return;
  }
  
  // We might want to exclude the permanent states (OK, FLIP), too...
  
  tri_state[index] = TMP_DELETE;  
  
  TriangleData& data = triangles[index];
  
  vector<size_t> neighbors;
  
  for (int c = 0; c < 3; ++c)
  {
    vector<size_t> trisForEdge = getTrianglesForEdge (
      data.points[c],
      data.points[(c+1) % 3],
                                                      tri_state);
    if (debug_volume)
    {
      cerr << "p1=" << data.points[c] << " p2=" << data.points[(c+1) % 3]
           << " tFE.size=" << trisForEdge.size() << endl;
    }
    
    for (size_t neigh_index=0;
         neigh_index < trisForEdge.size();
         ++neigh_index) {
      deleteAllConnectedTriangles(trisForEdge[neigh_index], tri_state);
    }
  }
}

void MultiObjectTriangleContainer::deletePatch(
    size_t index, std::vector<TriangleStatus>& tri_state) {
  if (debug_volume) cerr << "deletePatch(" << index << ")\n";
  deletePatchRecursively(index, tri_state);
  makePermanent(tri_state);
}


void MultiObjectTriangleContainer::makePermanent(
  std::vector<TriangleStatus>& tri_state) {
  for(size_t index = 0; index < tri_state.size(); ++index ) {
    if (debug_volume) cerr << "mP: tri_state[" << index << "]=";
    switch (tri_state[index]) {
      case TMP_OK:
        if (debug_volume) cerr << "TMP_OK\n";
        tri_state[index] = OK;
        break;
      case TMP_FLIP:
        if (debug_volume) cerr << "TMP_FLIP\n";
        tri_state[index] = FLIP;
        break;
      case TMP_DELETE:
        if (debug_volume) cerr << "TMP_DELETE\n";
        tri_state[index] = DELETE;
        break;
      case TMP_VISITED:
        if (debug_volume) cerr << "TMP_VISITED\n";
        tri_state[index] = UNKNOWN;
        break;
      default:
        if (debug_volume) cerr << "DEFAULT("<< tri_state[index] <<")\n";
    }
  }
}

void MultiObjectTriangleContainer::deletePatchRecursively(
      size_t index, std::vector<TriangleStatus>& tri_state) {
  if (debug_volume)
  {
    cerr << "dP idx=" << index << " state=" << tri_state[index] << endl;
  }
  if (tri_state[index] == DELETE) { // shouldn't get here...
    if (debug_volume) cerr << "already deleted: " << index << endl;
    return;
  }
  if (tri_state[index] == OTHER) {
    if (debug_volume) cerr << "part of other object: " << index << endl;
    return;
  }  
  if (tri_state[index] == TMP_DELETE) { // needed to end recursion!
        // getTrianglesForEdge ignores DELETE triangles,
        //so we might run into problems there...
    if (debug_volume) cerr << "already tmp_deleted: " << index << endl;
    return;
  }
  
  // We might want to exclude the permanent states (OK, FLIP), too...
  
  tri_state[index] = TMP_DELETE;  
  
  TriangleData& data = triangles[index];
  
  vector<size_t> neighbors;
  
  for (int c = 0; c < 3; ++c)
  {
    vector<size_t> trisForEdge = getTrianglesForEdge (
      data.points[c],
      data.points[(c+1) % 3],
      tri_state);
    if (debug_volume)
    {
      cerr << "p1=" << data.points[c] << " p2=" << data.points[(c+1) % 3]
           << " tFE.size=" << trisForEdge.size() << endl;
    }
    if (trisForEdge.size() == 2) { // just us and one neighbor
      size_t neighbor = (trisForEdge[0]==index)?trisForEdge[1]:trisForEdge[0];
      if (debug_volume) cerr << "neigh= " << neighbor << endl;
      neighbors.push_back(neighbor);
    } 
  }
  
  for (size_t neigh_index=0; neigh_index < neighbors.size(); ++neigh_index) {
    deletePatchRecursively(neighbors[neigh_index], tri_state);
  }
}



bool MultiObjectTriangleContainer::hasLonelyEdge(
  size_t index, std::vector<TriangleStatus>& tri_state) {

  TriangleData& data = triangles[index];
  
  for (int c = 0; c < 3; ++c)
  {
    vector<size_t> trisForEdge = getTrianglesForEdge (
                                  data.points[c],
                                  data.points[(c+1) % 3],
                                  tri_state);
    if (debug_volume)
    {
      cerr << "p1=" << data.points[c] << " p2=" << data.points[(c+1) % 3] 
           << " tFE.size=" << trisForEdge.size() << endl;
    }
    if (trisForEdge.size() == 1) {
      if (debug_volume) cerr << "foundLonelyEdge" << endl;
      return true;
    }   
  }
  return false;
}


void MultiObjectTriangleContainer::removeTriangleFromObject(
  int object, size_t triangle)
{
  TriangleData& data = triangles[triangle];
  data.setMembershipOfObject(object, false);
}


// extend orignial method to accomodate for deleted triangles in tri_state
vector<size_t>
MultiObjectTriangleContainer::getTrianglesForEdge(
                              size_t point1,
                              size_t point2,
                              vector<TriangleStatus>& tri_state) {
  vector<size_t> result;
  
  vector<size_t> allTrianglesForEdge = getTrianglesForEdge(
    point1,
    point2);
  
  for (vector<size_t>::iterator it = allTrianglesForEdge.begin();
       it != allTrianglesForEdge.end();
       ++it) {
    TriangleStatus state = tri_state[*it];
    if (state == DELETE || state == OTHER ) continue;
    if (debug_volume) cerr << " " << *it << " ";
    result.push_back(*it);
  }
  return result;
}

// first and second must be != last
void MultiObjectTriangleContainer::getPatchForHoleRecursion(
              std::vector<size_t>::const_iterator first,
              std::vector<size_t>::const_iterator last, // one after last
              std::vector<std::vector<size_t> >& triangles) {
  assert(first != last);
  
  std::vector<size_t>::const_iterator second = first;
  ++second;
  assert(second != last);
  
  std::vector<size_t>::const_iterator third = second;
  ++third;
  
  while (third != last)  { //TODO: 
    //what if previous set of points was collinear when third becomes last??
    // if truly collinear we alread inserted an edge which is overlapping
    // these last points...
    if (!spatial3d_geometric::collinear(points[*first],
                points[*second], points[*third])) {
      
      std::vector<size_t> triangle;
      triangle.push_back(*first);
      triangle.push_back(*second);
      triangle.push_back(*third);
      
      if (debug_volume) cerr << "added: " << points[*first] << ", "
      << points[*second] << ", " << points[*third] << endl;
      triangles.push_back(triangle);
      
      std::vector<size_t>::const_iterator next_last = third;
      ++next_last;
      // if we have skipped some vertices, handle them now
      MultiObjectTriangleContainer::getPatchForHoleRecursion(second,
                                                    next_last, triangles);    
      second = third;
    } else {
      if (debug_volume)
      {
        cerr << "collinear: " << points[*first] << ", "
             << points[*second] << ", " << points[*third] << endl;
      }
    }
    ++third;
  }
}

// basically the 3 coins triangulation method adpated to 3D
// Idea: No need to consider self intersection of patching triangles.
// Just add to the container and let it do its magic.
// only thing to consider are colinear points, as they will not form a triangle
// if the first, second and third are colinear, we'll just advance third and 
std::vector<std::vector<size_t> >
MultiObjectTriangleContainer::getPatchForHole(
         int object, 
         const std::vector<size_t>& holes) {
          
  std::vector<std::vector<size_t> > triangles(0);
  if (debug_volume) cerr << "entering getPatchForHole()\n";
  // initialize vectors
  std::vector<size_t>::const_iterator last = holes.end();
  std::vector<size_t>::const_iterator first = holes.begin();
  if (first == last) {
    if (debug_volume) cerr << "empty hole vector!!\n";
    return triangles;
  }
  std::vector<size_t>::const_iterator second = first;
  ++second;
  if (second == last) {
    if (debug_volume) cerr << "only one point in hole vector!!\n";
    return triangles;
  }
  std::vector<size_t>::const_iterator third = second;
  ++third;
  if (third == last) {
    if (debug_volume) cerr << "only two points in hole vector!!\n";
    return triangles;
  }
  
 MultiObjectTriangleContainer::getPatchForHoleRecursion(first,last,triangles);
  
  if (debug_volume) cerr << "leaving getPatchForHole()\n";
  return triangles;
}



bool MultiObjectTriangleContainer::fillHoles (
  int object, const std::vector<std::vector<size_t> >& holes) {
  
  if (debug_volume) cerr << "entering fillHoles()\n";
  for (std::vector<std::vector<size_t> >::const_iterator hole_it = 
         holes.begin();
       hole_it != holes.end();
       ++hole_it) {
    std::vector<std::vector<size_t> > patch = 
         MultiObjectTriangleContainer::getPatchForHole(object, *hole_it);
    if (debug_volume) cerr << "--- Patch start ---\n";
    if (patch.size() >0) {
      for (std::vector<std::vector<size_t> >::iterator tri_it = patch.begin();
           tri_it != patch.end();
           ++tri_it) {
        std::vector<size_t> triangle = *tri_it;
        if (debug_volume) {
          cerr << triangle[0] << " " << triangle[1] << " " << triangle[2]<<endl;
          cerr << points[triangle[0]] << " " << points[triangle[1]]
               << " "<< points[triangle[2]] << endl;
        }
        bool res = addTriangle(triangle.data(), object, true);
        if (!res) {return false;} // failed to add triangle... bail out
      }      
    }
    if (debug_volume) cerr << "--- Patch end ---\n";
  }
  if (debug_volume) cerr << "leaving fillHoles()\n";
  return true;
}

bool MultiObjectTriangleContainer::getHoles (
  int object, std::vector<std::vector<size_t> >& result) {
  
  assert(object > 0);
  assert(object <= noExternalObjects);
  
  std::multimap<size_t,size_t> edgeIndex;
  std::vector<Edge3d> edges;
  
  getLonelyEdges(object, edges, edgeIndex);
  
  if (debug_volume) {
    std::cerr << "found edges with one triangle:\n";
    for (std::multimap<size_t,size_t>::iterator it=edgeIndex.begin();
         it!=edgeIndex.end();
         ++it) {
      cerr << "point " << (*it).first << " => edge " << (*it).second << ": "
           << edges[(*it).second] << endl;
    }
  }
  
  formHolesFromEdges(object, edges, edgeIndex, result);
  
  for (std::vector<std::vector<size_t> >::iterator hole_it = result.begin();
       hole_it != result.end();
       ++hole_it) {
    if (debug_volume) {
      cerr << "----- hole start -----\n";
      for (std::vector<size_t>::iterator point_it = hole_it->begin();
           point_it != hole_it->end();
           ++point_it) {
        cerr << *point_it << endl;      
      }
      cerr << "------ hole end ------\n";
    }
  } 
  bool found_holes = (edges.size() > 0);
  
  return found_holes;
}

// helper to find the lonely edges inside a given object.
// edges are directed as in the containing triangle.
// returns the egdes as pair of indices to the start/end points
// and a multimap to quickly find the edges by the index of 
// a given start or end point

void MultiObjectTriangleContainer::getLonelyEdges (
  int object, 
  std::vector<Edge3d>& edges, 
  std::multimap<size_t,size_t>& edgeIndex) {
  
  auto_ptr<mmrtree::RtreeT<3, size_t>::iterator> it(triangles_tree.entries());
  size_t const * triangleIndex;
  while( (triangleIndex = it->next()) != 0)
  {
    const TriangleData& data = triangles[*triangleIndex];
    if (data.isMemberOfObject(object))
    {
      for (int c = 0; c < 3; ++c)
      {
        Edge3d edge;
        edge.startPoint = data.points[c];
        edge.endPoint = data.points[(c+1) % 3];
        
        vector<size_t> allTrianglesForEdge = getTrianglesForEdge(
              edge.startPoint,
              edge.endPoint);
        vector<size_t> trianglesForEdge;
        for (vector<size_t>::iterator index = allTrianglesForEdge.begin();
             index != allTrianglesForEdge.end();
             ++index)
             {
               if (triangles[*index].isMemberOfObject(object))
               {
                 trianglesForEdge.push_back(*index);
               }
             }
         //TODO: Check if edges with tri_count >2 need to be handled
         // In that case a test for duplicate edges inside the map is required
         if (trianglesForEdge.size() == 1)
             {
          size_t index = edges.size();
          edges.push_back(edge);
          edgeIndex.insert ( std::pair<size_t,size_t>(edge.startPoint,index) );
          edgeIndex.insert ( std::pair<size_t,size_t>(edge.endPoint,index) ); 
             }            
      }
    }
  } 
}

// helper to create/find cycles in the provided edges
// the found cycles are returned as vector containing the index to the vertices
// where vertex[n] and vertex[n % vertex.size()] form an edge of the cycle.
// (two successive vertexes form an edge, and also the last and first one)
// 
// This first (simple) implementation will only find cycles of edges whose pnts
// are only connected to exactly one other edge
// Or put another way: vertices of cycle A are disjoint from vertices of cycle B
// (for all cycles A, B)
//
// But in case we want to close loose ends, we need to navigate 
// the surface of the correct object

void MultiObjectTriangleContainer::formHolesFromEdges(
  int object,
  std::vector<Edge3d>& edges,
  std::multimap<size_t,size_t>& edgeIndex,
  std::vector<std::vector<size_t> >& result) {
  
  result.clear();
  
  if (edges.size() == 0) return;
  // try our best (so far): for every unvisited edge try to form a cycle...
  // TODO: check if we need more states if we allow other types of holes
  std::vector<bool> used(edges.size(), false);  
  size_t lowest_unused_edge = 0;
  while (true) {
    while (used[lowest_unused_edge]) {
      ++lowest_unused_edge;
      if (lowest_unused_edge == edges.size()) {
        return;
      }
    }
    
    std::vector<size_t> newCycle;  
    size_t current_edge_index = lowest_unused_edge;
    used[current_edge_index] = true;    
    Edge3d current_edge = edges[current_edge_index];
    
    // we start the (potential) cycle in opposite direction of given edge
    // that way (for correctly orietated egde triangles) the hole edges
    // will have the same orientation as the edges for the required patch
    //size_t current_point = current_edge.endPoint;  // wrong ordre
    size_t current_point = current_edge.startPoint;  
    newCycle.push_back(current_point);
    
    while(true) {    
      int edge_count_for_current_point = 0;
      size_t new_edge_index = 0;
      for (std::multimap<size_t,size_t>::iterator it
                 = edgeIndex.lower_bound(current_point);
           it != edgeIndex.upper_bound(current_point);
           ++it) {
        ++edge_count_for_current_point;      
      
        // we have found the incoming edge:
        if (it->second == current_edge_index) continue; 
        
        new_edge_index = it->second;
        used[new_edge_index] = true;
        size_t other_point = (edges[new_edge_index].startPoint == current_point)
          ? edges[new_edge_index].endPoint : edges[new_edge_index].startPoint;
      // push_back anyway, if we overwrite, we'll detect later (egde_count > 2)
        newCycle.push_back(other_point); 
      }
      
      if (edge_count_for_current_point !=2) { 
        // we only want a clean cycle! no "Abwzeigungen" allowed.
        if (debug_volume) {
          cerr << "detected " << edge_count_for_current_point 
               << " edge(s) for point " << current_point << endl;
        }
        break;
      }
      if (newCycle.back() == newCycle.front()) {
     // last = first element, front is valid, so we have a valid cycle
        newCycle.pop_back();
        result.push_back(newCycle);
        break;
      }
      
      current_edge_index = new_edge_index;
      current_point = newCycle.back();
    } 
  }  
}




// TODO: old method - used for reference only - check if can be removed
bool MultiObjectTriangleContainer::checkVolume_closed(int object,
                                                      bool corrections)
{
  assert(object > 0);
  assert(object <= noExternalObjects);

  auto_ptr<mmrtree::RtreeT<3, size_t>::iterator> it(triangles_tree.entries());

  size_t const * triangleIndex;
  while( (triangleIndex = it->next()) != 0)
  {
    const TriangleData& data = triangles[*triangleIndex];
    if (data.isMemberOfObject(object))
    {
      for (int c = 0; c < 3; ++c)
      {
        size_t edgePoint1 = data.points[c];
        size_t edgePoint2 = data.points[(c+1) % 3];
        vector<size_t> allTrianglesForEdge = getTrianglesForEdge(edgePoint1,
                                                                 edgePoint2);
        vector<size_t> trianglesForEdge;
        for (vector<size_t>::iterator index = allTrianglesForEdge.begin();
             index != allTrianglesForEdge.end(); ++index)
        {
          if (triangles[*index].isMemberOfObject(object))
          {
            trianglesForEdge.push_back(*index);
          }
        }
        // TODO: jede gerade Zahl erlauben und prüfen
        if (trianglesForEdge.size() != 2)
        {
          if (debug_volume) {
            cerr << "edgePoint1=" << edgePoint1
                 << " edgePoint2=" << edgePoint2
                 << " trianglesForEdge.size()=" << trianglesForEdge.size()
                 << endl;
          }
          return false;
        }
        else
        {
          bool edge1 = getTriangleEdgeDirection(triangles[trianglesForEdge[0]],
                                                edgePoint1,
                                                edgePoint2);
          bool edge2 = getTriangleEdgeDirection(triangles[trianglesForEdge[1]],
                                                edgePoint1,
                                                edgePoint2);
          if (edge1 == edge2)
          {
            if (debug_volume) {
              cerr << "same orientation for edgePoint1=" << edgePoint1 
                   << " edgePoint2=" << edgePoint2 
                   << " and triangles " << trianglesForEdge[0] 
                   << " " << trianglesForEdge[1] << endl;
            }
            return false;
          }
        }
      }
    }
  }
  return true;
}

// TODO: old method stub - not used anymore - remove
bool MultiObjectTriangleContainer::checkVolume_orientation(int object,
                                                      bool corrections)
{
  return true;
}

// TODO: old method stub - not used anymore - remove
bool MultiObjectTriangleContainer::checkVolume_unconnected_inner_components(
                                                              int object,
                                                              bool corrections)
{
  return true;
}

// TODO: old method stub - not used anymore - remove
bool MultiObjectTriangleContainer::checkVolume_connected_inner_components(
                                                              int object,
                                                              bool corrections)
{
  return true;
}


//////////////////////////////////////////////////////////////////////////
// private methods ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MultiObjectTriangleContainer::TriangleData::TriangleData(size_t p1,
                                                         size_t p2,
                                                         size_t p3)
{
  points[0] = p1;
  points[1] = p2;
  points[2] = p3;

  if (p1 == (size_t)-1)
  {
    cerr << "Invalid triangle XXX" << endl;
  }
  
  assert(p1 != p2);
  assert(p2 != p3);
  assert(p3 != p1);
  
  object_membership = 0;
  object_membership_direction = 0;
}

bool MultiObjectTriangleContainer::TriangleData::isMemberOfObject(int object)
                                                                          const
{
  return (object_membership & getObjectBitmask(object)) != 0;
}

void MultiObjectTriangleContainer::TriangleData::setMembershipOfObject(
                                                   int object, bool isMember)
{
  if (isMember)
  {
    object_membership |= getObjectBitmask(object);
  }
  else
  {
    object_membership &= ~getObjectBitmask(object);
  }
}

bool MultiObjectTriangleContainer::TriangleData::getDirectionOfObject(
                                                              int object) const
{
  return (object_membership_direction & getObjectBitmask(object)) != 0;
}

void MultiObjectTriangleContainer::TriangleData::setDirectionOfObject(
                                                   int object, bool direction)
{
  if (direction)
  {
    object_membership_direction |= getObjectBitmask(object);
  }
  else
  {
    object_membership_direction &= ~getObjectBitmask(object);
  }
}

MultiObjectTriangleContainer::SortedEdge::SortedEdge(size_t _edgeP1,
                                                     size_t _edgeP2,
                                                     size_t xPoint,
                                       const vector<SimplePoint3d>& points)
      : edgeP1(_edgeP1), edgeP2(_edgeP2)
{
  Vector3d edgeVector(points[edgeP1], points[edgeP2]);
  projectionPlane = Plane3d(points[edgeP1],(1 / length(edgeVector)) 
                      * edgeVector);
  projection = Projection2d(projectionPlane, points[edgeP1], points[xPoint]);
}

size_t MultiObjectTriangleContainer::SortedEdge::getThirdPoint(
                                             const size_t trianglePoints[])
{
  for (int c = 0; c < 3; ++c)
  {
    if (trianglePoints[c] != edgeP1 && trianglePoints[c] != edgeP2)
    {
      return trianglePoints[c];
    }
  }
  assert(false);
  return 0;
}
  
TrianglePathDirection MultiObjectTriangleContainer::SortedEdge::getDirection(
                                                 const size_t trianglePoints[])
{
  for (int c = 0; c < 3; ++c)
  {
    if (trianglePoints[c] == edgeP1)
    {
      if (trianglePoints[(c + 1) % 3] == edgeP2)
        return LEAVE;
      else
        return ENTER;
    }
  }
  assert(false);
  return LEAVE;
}

MultiObjectTriangleContainer::SortedTriangle::SortedTriangle(
                               SortedEdge _edge, size_t _triangle,
                               const TriangleData _triangleData,
                               const vector<SimplePoint3d>& points)
      : edge(_edge), triangle(_triangle), triangleData(_triangleData)
{
  thirdPoint = points[edge.getThirdPoint(triangleData.points)];
  phi = getPolarAngle(edge.projection.project(thirdPoint));
  direction = edge.getDirection(triangleData.points);
}
  
TrianglePathDirection
MultiObjectTriangleContainer::SortedTriangle::getDirectionForObject(int object)
{
  object_t object_bits = getObjectBitmask(object);
  if ((triangleData.object_membership_direction & object_bits) == 0)
    return direction;
  else
    return !direction;
}
  
bool MultiObjectTriangleContainer::SortedTriangle::isInObject(int object)
{
  object_t object_bits = getObjectBitmask(object);
  return (triangleData.object_membership & object_bits) != 0;
}

bool MultiObjectTriangleContainer::addTriangle(size_t* points,
                                               int object,
                                               bool corrections)
{
  // Add to object 0
  TriangleData newTriangle(points[0], points[1], points[2]);
  newTriangle.object_membership = getObjectBitmask(0);
  
  addTriangle(newTriangle);
  
  return moveObject0(object, corrections);
}

MultiObjectTriangleContainer::TriangleSearchResult
MultiObjectTriangleContainer::getAnIncompatibleTriangle(size_t triangle,
                                 size_t& out_incompatibleTriangle,
                                 TriangleIntersectionResult& out_intersection)
{
  Rectangle<3> bbox = getTriangleBoundingBox(triangle);
  bbox.Extend(0.0001);
  auto_ptr<mmrtree::RtreeT<3, size_t>::iterator> it(triangles_tree.find(bbox));
  size_t const * existingTriangleIndex;
  while( (existingTriangleIndex = it->next()) != 0)
  {
    if (*existingTriangleIndex == triangle)
      continue;

    const TriangleData& existingTriangleData =triangles[*existingTriangleIndex];
    const TriangleData& newTriangle = triangles[triangle];
    
    TriangleDataComparisonResult comp = compare(newTriangle,
                                                existingTriangleData);
    
    if (comp == SAME || comp == OPPOSITE)
    {
      out_incompatibleTriangle = *existingTriangleIndex;
      return (comp == SAME) ? KNOWN_SAME : KNOWN_OPPOSITE;
    }
    TriangleIntersectionResult intersectionResult =
      intersection(getTriangle(newTriangle),
                   getTriangle(existingTriangleData));
    switch (intersectionResult.getIntersectionType())
    {
      case TriangleIntersectionResult::NO_INTERSECTION:
      case TriangleIntersectionResult::POINT:
        continue;
      case TriangleIntersectionResult::AREA:
        out_incompatibleTriangle = *existingTriangleIndex;
        out_intersection = intersectionResult;
        return INCOMPATIBLE;
      case TriangleIntersectionResult::SEGMENT:
        if (comp == SHARED_EDGE)
        {
          // This is legal!
          continue;
        }
        else
        {
          out_incompatibleTriangle = *existingTriangleIndex;
          out_intersection = intersectionResult;
          return INCOMPATIBLE;
        }
    }
  }
  return NEW;
}

size_t MultiObjectTriangleContainer::addPoint(const SimplePoint3d& point)
{
  /* First, compare coordinates with existing points. If there is a
   * matching point, just return its index. Otherwise, we create a new
   * point.
   */

  Rectangle<3> bbox = point.BoundingBox();
  bbox.Extend(0.0001);
  auto_ptr<mmrtree::RtreeT<3, size_t>::iterator> it(points_tree.find(bbox));
  size_t const * index;
  while( (index = it->next()) != 0)
  {
    SimplePoint3d p = points[*index];
    if (almostEqual(point, p))
    {
      return *index;
    }
  }
  int res = points.size();
  points.push_back(point);
  points_tree.insert(bbox, res);
  return res;
}

size_t MultiObjectTriangleContainer::addTriangle(const TriangleData& triangle)
{
  size_t newIndex;
  if (unused_triangle_indices.size() > 0)
  {
    newIndex = unused_triangle_indices.top();
    unused_triangle_indices.pop();
    triangles[newIndex] = triangle;
  }
  else
  {
    newIndex = triangles.size();
    triangles.push_back(triangle);
  }
  triangles_tree.insert(getTriangleBoundingBox(newIndex), newIndex);
  if ((triangle.object_membership & 1) == 1)
  {
    trianglesObject0.push_back(newIndex);
  }
  return newIndex;
}

void MultiObjectTriangleContainer::removeTriangle(size_t triangle)
{
  if ((triangles[triangle].object_membership & 1) == 1)
  {
    trianglesObject0.erase(std::remove(trianglesObject0.begin(),
                                       trianglesObject0.end(),
                                       triangle),
                           trianglesObject0.end());
  }
  triangles_tree.erase(getTriangleBoundingBox(triangle), triangle);
  unused_triangle_indices.push(triangle);
}

void MultiObjectTriangleContainer::addExistingTriangleToObject(
                                          TriangleData& triangleData,
                                          int object,
                                          bool oppositeDirection)
{
  object_t bit = 1 << object;
  triangleData.object_membership |= bit;
  if (oppositeDirection)
  {
    triangleData.object_membership_direction |= bit;
  }
  else
  {
    triangleData.object_membership_direction &= ~bit;
  }
}

Triangle MultiObjectTriangleContainer::getTriangle(const TriangleData& data,
                                                   int object) const
{
  object_t object_bits = getObjectBitmask(object);

  if ((data.object_membership_direction & object_bits) == 0)
  {
    // Direction of data
    return Triangle(points[data.points[0]],
                    points[data.points[1]],
                    points[data.points[2]]);
  }
  else
  {
    // Opposite direction
    return Triangle(points[data.points[2]],
                    points[data.points[1]],
                    points[data.points[0]]);
  }
}

Triangle MultiObjectTriangleContainer::getTriangle(const TriangleData& data)
                                                                          const
{
  return Triangle(points[data.points[0]],
                  points[data.points[1]],
                  points[data.points[2]]);
}

bool MultiObjectTriangleContainer::getTriangleEdgeDirection(
                                         const TriangleData& data,
                                         size_t point1, size_t point2) const
{
  for (int c = 0; c < 3; ++c)
  {
    if (data.points[c] == point1)
    {
      return data.points[(c + 1) % 3] == point2;
    }
  }
  assert(false);
}

Rectangle<3> MultiObjectTriangleContainer::getSegmentBoundingBox(size_t point1,
                                                                 size_t point2)
                                                                          const
{
  Rectangle<3> result = points[point1].BoundingBox();
  result.Extend(points[point2].BoundingBox());
  return result;
}

Rectangle<3> MultiObjectTriangleContainer::getTriangleBoundingBox(
                                                      size_t triangle) const
{
  Rectangle<3> result = points[triangles[triangle].points[0]].BoundingBox();
  result.Extend(points[triangles[triangle].points[1]].BoundingBox());
  result.Extend(points[triangles[triangle].points[2]].BoundingBox());
  return result;
}

// returns indices of points that share an edge with the argument
set<size_t>
MultiObjectTriangleContainer::getEdgesForPoint(size_t point) const
{
  set<size_t> result;

  Rectangle<3> bbox = points[point].BoundingBox();
  bbox.Extend(0.0001);
  auto_ptr<mmrtree::RtreeT<3, size_t>::iterator> it(triangles_tree.find(bbox));
  size_t const * existingTriangleIndex;
  while( (existingTriangleIndex = it->next()) != 0)
  {
    const TriangleData& existingTriangleData =triangles[*existingTriangleIndex];

    for(int c = 0; c < 3; ++c)
    {
      if (existingTriangleData.points[c] == point)
      {
        for (int d = 0; d < 3; ++d)
        {
          if (d != c)
          {
            result.insert(existingTriangleData.points[d]);
          }
        }
      }
    }
  }
  
  return result;
}

vector<size_t>
MultiObjectTriangleContainer::getTrianglesForPoint(size_t point) const
{
  vector<size_t> result;

  Rectangle<3> bbox = points[point].BoundingBox();
  bbox.Extend(0.0001);
  auto_ptr<mmrtree::RtreeT<3, size_t>::iterator> it(triangles_tree.find(bbox));
  size_t const * existingTriangleIndex;
  while( (existingTriangleIndex = it->next()) != 0)
  {
    const TriangleData& existingTriangleData =triangles[*existingTriangleIndex];

    for(int c = 0; c < 3; ++c)
    {
      if (existingTriangleData.points[c] == point)
      {
        result.push_back(*existingTriangleIndex);
        break;
      }
    }
  }
  
  return result;
}

vector<size_t>
MultiObjectTriangleContainer::getTrianglesForEdge(size_t point1,
                                                  size_t point2) const
{
  vector<size_t> result;

  Rectangle<3> bbox = getSegmentBoundingBox(point1, point2);
  bbox.Extend(0.0001);
  auto_ptr<mmrtree::RtreeT<3, size_t>::iterator> it(triangles_tree.find(bbox));
  size_t const * existingTriangleIndex;
  while( (existingTriangleIndex = it->next()) != 0)
  {
    const TriangleData& existingTriangleData =
                            triangles[*existingTriangleIndex];

    int number_of_corners = 0;
    for(int c = 0; c < 3; ++c)
    {
      size_t p = existingTriangleData.points[c];
      if (p == point1 || p == point2)
      {
        ++ number_of_corners;
      }
    }
    if (number_of_corners == 2)
    {
      result.push_back(*existingTriangleIndex);
    }
  }
  
  return result;
}

MultiObjectTriangleContainer::TriangleDataComparisonResult
MultiObjectTriangleContainer::compare(const TriangleData& t1,
                                      const TriangleData& t2) const
{
  size_t index_in_t2[3];
  int number_of_corners = 0;
  for (int c1 = 0; c1 < 3; ++c1)
  {
    for (int c2 = 0; c2 < 3; ++c2)
    {
      if (t1.points[c1] == t2.points[c2])
      {
        index_in_t2[c1] = c2;
        ++ number_of_corners;
      }
    }
  }
  switch(number_of_corners)
  {
    case 0:
      return DIFFERENT;
    case 1:
      return SHARED_CORNER;
    case 2:
      return SHARED_EDGE;
    case 3:
      if ( index_in_t2[1] == (index_in_t2[0] + 1) % 3 )
        return SAME;
      else
        return OPPOSITE;
    default:
      assert(false);
      return DIFFERENT;
  }
}

void MultiObjectTriangleContainer::splitEdge(size_t edgeP1, size_t edgeP2,
                size_t splitP,
                vector<vector<size_t> >& tracked_partitions)
{
  vector<size_t> trianglesToBeSplit = getTrianglesForEdge(edgeP1, edgeP2);

  for(size_t c = 0; c < trianglesToBeSplit.size(); ++c)
  {
    size_t triangleToBeSplitNow = trianglesToBeSplit[c];
    TriangleData oldTriangleData = triangles[triangleToBeSplitNow];

    removeTriangle(triangleToBeSplitNow);
    
    vector<size_t> partitions_containing_this_triangle;
    
    // Find out if this triangle belongs to a tracked partition
    for (size_t p = 0; p < tracked_partitions.size(); ++p)
    {
      for (size_t t = 0; t < tracked_partitions[p].size(); ++t)
      {
        if (tracked_partitions[p][t] == triangleToBeSplitNow)
        {
          tracked_partitions[p].erase(tracked_partitions[p].begin() + t);
          partitions_containing_this_triangle.push_back(p);
          break;
        }
      }
    }

    // One point of the old triangle will still be in both of the new
    // triangles. Find it.
    int point_index_of_old_common_point = -1;
    for (int d = 0; d < 3; ++d)
    {
      if (oldTriangleData.points[d] != edgeP1 &&
          oldTriangleData.points[d] != edgeP2)
      {
        point_index_of_old_common_point = d;
      }
    }
    // Now make two copies of the old triangle data, then replace one point
    // with our new point. Do not replace the common point.
    
    for (int n = 1; n <= 2; ++n)
    {
      TriangleData newTriangleData = oldTriangleData;
      int index_to_replace = (point_index_of_old_common_point + n) % 3;
      newTriangleData.points[index_to_replace] = splitP;

      size_t newIndex = addTriangle(newTriangleData);
      
      for (size_t p = 0; p < partitions_containing_this_triangle.size(); ++p)
      {
        tracked_partitions[partitions_containing_this_triangle[p]]
                      .push_back(newIndex);
      }
    }
  }
}

void MultiObjectTriangleContainer::splitSurface(size_t triangle,
                                  size_t surfacePoint,
                                  vector<vector<size_t> >& tracked_partitions)
{
  TriangleData oldTriangleData = triangles[triangle];
  removeTriangle(triangle);

  vector<size_t> partitions_containing_this_triangle;
    
  // Find out if this triangle belongs to a tracked partition
  for (size_t p = 0; p < tracked_partitions.size(); ++p)
  {
    for (size_t t = 0; t < tracked_partitions[p].size(); ++t)
    {
      if (tracked_partitions[p][t] == triangle)
      {
        tracked_partitions[p].erase(tracked_partitions[p].begin() + t);
        partitions_containing_this_triangle.push_back(p);
        break;
      }
    }
  }
  
  for (int c = 0; c < 3; ++c)
  {
    TriangleData newTriangleData = oldTriangleData;
    newTriangleData.points[c] = surfacePoint;

    size_t newIndex = addTriangle(newTriangleData);

    for (size_t p = 0; p < partitions_containing_this_triangle.size(); ++p)
    {
      tracked_partitions[partitions_containing_this_triangle[p]]
                    .push_back(newIndex);
    }
  }
}

// Output vector both: orientation of t0
void MultiObjectTriangleContainer::cutTriangles2d(size_t t0, size_t t1,
                    vector<size_t>& o0, vector<size_t>& o1,
                    vector<size_t>& both)
{
  vector<vector<size_t> > parts;
  parts.push_back(vector<size_t>());
  parts.push_back(vector<size_t>());

  parts[0].push_back(t0);
  parts[1].push_back(t1);
  
  TriangleData originalTriangleData[2] = { triangles[t0], triangles[t1] };
  
  // Iterate over triangles t whose edges we use to split the other triangle.
  // Iterate over the edges e of that triangle.
  for (size_t t = 0; t < 2; ++t) for (int e = 0; e < 3; ++e)
  {
    const size_t edgePoint1 = originalTriangleData[t].points[e];
    const size_t edgePoint2 = originalTriangleData[t].points[(e + 1) % 3];
    size_t parts_to_be_split = 1 - t;
    
    for(;;)
    {
      bool split = false;
      for (size_t c = 0; c < parts[parts_to_be_split].size(); ++c )
      {
        size_t triangle = parts[parts_to_be_split][c];
        // Wir haben nun ein Dreieck sowie eine Strecke und müssen die
        // Verträglichkeit beurteilen.
        split = splitTriangleForSegment(triangle, edgePoint1, edgePoint2,
                                        parts);
        if (split)
          break;
      }
      if (!split)
      {
        break;
      }
    }
  }

  if (debug_splitting)
  {
    cerr << "Parts 0: ";
    for (size_t c = 0; c < parts[0].size(); ++c)
    {
      cerr << getTriangle(triangles[parts[0][c]]) << " ";
    }
    cerr << endl;
    cerr << "Parts 1: ";
    for (size_t c = 0; c < parts[1].size(); ++c)
    {
      cerr << getTriangle(triangles[parts[1][c]]) << " ";
    }
    cerr << endl;
  }
  
  // Nun sind alle Teile mit den Kanten des anderen Ursprungsdreiecks
  // verträglich. Die Teile liegen nun komplett innerhalb oder komplett
  // außerhalb des anderen Ursprungsdreiecks.
  for (size_t c = 0; c < parts[0].size(); ++c)
  {
    if (isCompletelyInside(getTriangle(triangles[parts[0][c]]),
                           getTriangle(originalTriangleData[1])))
    {
      both.push_back(parts[0][c]);
    }
    else
    {
      o0.push_back(parts[0][c]);
    }
  }
  for (size_t c = 0; c < parts[1].size(); ++c)
  {
    if (isCompletelyInside(getTriangle(triangles[parts[1][c]]),
                           getTriangle(originalTriangleData[0])))
    {
      // not needed (common parts are already found)
    }
    else
    {
      o1.push_back(parts[1][c]);
    }
  }
}

bool MultiObjectTriangleContainer::moveObject0(int toObject, bool corrections)
{
  // In each iteration of this loop, one action is being made.
  // Either integrating the top element of the object zero stack into
  // the object, deleting it as a correction if it is not necessary,
  // splitting the new object (increasing the number of objects on the stack),
  // splitting existing triangles or both.
  // Integrating a single triangle may need several iterations of this loop,
  // but progress is guaranteed each time.

  object_t toObject_bits = getObjectBitmask(toObject);
  
  while(trianglesObject0.size() > 0)
  {
    if (debug_splitting)
    {
      cerr << "Next step." << endl;
      // cerr << "Current triangles are: " << endl;
      //test();
    }

    size_t newTriangleIndex = trianglesObject0.back();

    size_t incompatibleTriangleIndex;
    TriangleIntersectionResult intersectionResult;

    TriangleSearchResult searchResult = getAnIncompatibleTriangle(
                                                  newTriangleIndex,
                                                  incompatibleTriangleIndex,
                                                  intersectionResult);

    TriangleData& newTriangleData = triangles[newTriangleIndex];
    Triangle newTriangle = getTriangle(newTriangleData);

    if (debug_splitting) cerr << "new Triangle: "
                              << getTriangle(newTriangleData) << endl;
    
    switch(searchResult)
    {
      case NEW:
      {
        if (debug_splitting) cerr << "case NEW" << endl;
        removeTriangle(newTriangleIndex);
        newTriangleData.object_membership = getObjectBitmask(toObject);
        newTriangleData.object_membership_direction = 0;
        addTriangle(newTriangleData);
        continue;
      }
      case KNOWN_SAME:
      case KNOWN_OPPOSITE:
      {
        if (debug_splitting) cerr << "case KNOWN" << endl;
        TriangleData& incompatibleData = triangles[incompatibleTriangleIndex];
        bool twice_in_same_object =
           (toObject_bits & incompatibleData.object_membership) != 0;
        if (twice_in_same_object)
        {
          if (corrections)
          {
            // Just drop it
            removeTriangle(newTriangleIndex);
            continue;
          }
          else
          {
            // That is not allowed without corrections.
            removeObject(0);
            return false;
          }
        }
        else
        {
          removeTriangle(newTriangleIndex);
          bool direction = searchResult == KNOWN_OPPOSITE;
          addExistingTriangleToObject(incompatibleData, toObject, direction);
          continue;
        }
      }
      case INCOMPATIBLE:
      {
        if (debug_splitting) cerr << "case INCOMPATIBLE" << endl;
        TriangleData& incompatibleData = triangles[incompatibleTriangleIndex];
        Triangle incompatibleTriangle = getTriangle(incompatibleData);

        if (debug_splitting) cerr << "incompatible: "
                                  << getTriangle(incompatibleData) << endl;
        
        if (debug_splitting) cerr << "intersection Type: "
                                  << intersectionResult.getIntersectionType()
                                  << endl;

        if (!corrections)
        {
          if (debug_splitting) cerr << "Abort add: no corrections" << endl;
          removeObject(0);
          return false;
        }
        // Decide on action
        if (intersectionResult.getIntersectionType() ==
                                     TriangleIntersectionResult::AREA)
        {
          if (debug_splitting) cerr << "area intersection" << endl;
          // Prüfen, ob gleiche oder gegensätzliche Orientierung
          bool sameDirection = newTriangle.getNormalVector()
                               * incompatibleTriangle.getNormalVector() > 0;
          
          vector<size_t> onlyNew, onlyExisting, both;
          cutTriangles2d(incompatibleTriangleIndex, newTriangleIndex,
                         onlyExisting, onlyNew, both);
          
          if (both.size() == 0)
          {
            cerr << "size only existing: " << onlyExisting.size() << endl
                 << "size only new: " << onlyNew.size() << endl
                 << "size both: " << both.size() << endl;
            cerr << "---" << endl;
            cerr << "triangles only existing: ";
            for (int c = 0; c < onlyExisting.size(); ++c)
            {
              cerr << getTriangle(triangles[onlyExisting[c]]) << " ";
            }
            cerr << endl;
            cerr << "triangles only new: ";
            for (int c = 0; c < onlyNew.size(); ++c)
            {
              cerr << getTriangle(triangles[onlyNew[c]]) << " ";
            }
            cerr << endl;
            cerr << "triangles both: ";
            for (int c = 0; c < both.size(); ++c)
            {
              cerr << getTriangle(triangles[both[c]]) << " ";
            }
            cerr << endl;
            numeric_fail();
          }
          
          // Both sind jetzt Dreiecke des bestehenden Objekts.
          // Das kopieren wir in Objekt 0 und setzen die Orientierung
          // entsprechend.
          // Da Dreiecke in Objekt 0 in der nativen Orientierung erwartet
          // werden, drehen wir sie bei Bedarf um.
          
          for (int c = 0; c < both.size(); ++c)
          {
            if (sameDirection)
            {
              TriangleData copyForObject0(triangles[both[c]].points[0],
                                          triangles[both[c]].points[1],
                                          triangles[both[c]].points[2]);
              copyForObject0.object_membership = getObjectBitmask(0);
              addTriangle(copyForObject0);
            }
            else
            {
              TriangleData copyForObject0(triangles[both[c]].points[2],
                                          triangles[both[c]].points[1],
                                          triangles[both[c]].points[0]);
              copyForObject0.object_membership = getObjectBitmask(0);
              addTriangle(copyForObject0);
            }
          }
        }
        if (intersectionResult.getIntersectionType() ==
                                      TriangleIntersectionResult::SEGMENT)
        {
          if (debug_splitting) cerr << "segment intersection" << endl;
          vector<SimplePoint3d>& intersectionPoints
                                   = intersectionResult.getIntersectionPoints();
          size_t point1 = addPoint(intersectionPoints[0]);
          size_t point2 = addPoint(intersectionPoints[1]);

          // Try to split new triangle first

          if (!isEdgeOf(point1, point2, newTriangleIndex))
          {
            vector<vector<size_t> > parts;
            bool success = splitTriangleForSegment(newTriangleIndex,
                                                   point1, point2, parts);
            if (!success)
            {
              cerr << "Fehler: 1" << endl
                   << "new: " << newTriangle << "(normal: "
                   << newTriangle.getPlane().getNormalVector() << ")" << endl
                   << "incompatible: " << incompatibleTriangle << "(normal: "
                   << incompatibleTriangle.getPlane().getNormalVector() << ")"
                   << endl
                   << "Segment: " << intersectionPoints[0] << "<->"
                                  << intersectionPoints[1] << endl;
              numeric_fail();
            }
            continue;
          }
          else
          {
            vector<vector<size_t> > parts;
            bool success = splitTriangleForSegment(incompatibleTriangleIndex,
                                                   point1, point2,
                                                   parts);
            if (!success)
            {
              cerr << "Fehler: 2" << endl
                   << "new: " << newTriangle << "(normal: "
                   << newTriangle.getPlane().getNormalVector() << ")" << endl
                   << "incompatible: " << incompatibleTriangle << "(normal: "
                   << incompatibleTriangle.getPlane().getNormalVector() << ")"
                   << endl
                   << "Segment: " << intersectionPoints[0] << "<->"
                                  << intersectionPoints[1] << endl;
              numeric_fail();
            }
            continue;
          }
        }
      }
    }
  }
  // Stack is empty if we left the loop!
  return true;
}

bool MultiObjectTriangleContainer::isEdgeOf(size_t point1, size_t point2,
                                            size_t triangle) const
{
  bool point1Found = false;
  bool point2Found = false;
  for (int c = 0; c < 3; ++c)
  {
    if (triangles[triangle].points[c] == point1)
    {
      point1Found = true;
    }
    if (triangles[triangle].points[c] == point2)
    {
      point2Found = true;
    }
  }
  return point1Found && point2Found;
}

bool MultiObjectTriangleContainer::splitTriangleForSegment(size_t triangleIndex,
                                                           size_t point1Index,
                                                           size_t point2Index,
                                    vector<vector<size_t> >& tracked_partitions)
{
  TriangleData& triangleData = triangles[triangleIndex];
  Triangle triangle = getTriangle(triangleData);

  // First job to do: reduce the segment to the subset of the segment that is
  // inside the triangle. Done in 2D here, so transform everything to 2D.
  
  const Transformation2d t(triangle.getPlane());
  
  const SimplePoint2d p1_2d = t.transform(points[point1Index]);
  const SimplePoint2d p2_2d = t.transform(points[point2Index]);

  SimplePoint2d triangle2d[3] = { t.transform(triangle.getA()),
                                  t.transform(triangle.getB()),
                                  t.transform(triangle.getC()) };

  if (intersection(p1_2d, p2_2d, triangle2d) != SEGMENT)
  {
    return false;
  }

  const SimplePoint2d p1_2d_in = firstPointInsideTriangle(p1_2d, p2_2d,
                                                          triangle2d[0],
                                                          triangle2d[1],
                                                          triangle2d[2]);
  const SimplePoint2d p2_2d_in = firstPointInsideTriangle(p2_2d, p1_2d,
                                                          triangle2d[0],
                                                          triangle2d[1],
                                                          triangle2d[2]);

  // Now, p1_2d_in and p2_2d_in are the part of the segment inside the triangle.
  // Transform the results back into 3D space.
  
  const SimplePoint3d p1_in = t.transform(p1_2d_in);
  const SimplePoint3d p2_in = t.transform(p2_2d_in);
  
  // If either point is inside the triangle (not on an edge), then do
  // a surface split.
  
  InsideResult insideResult1 = pointInsideTriangle(p1_in, triangle);
  if (insideResult1 == INSIDE)
  {
    // Split surface for this point
    splitSurface(triangleIndex, addPoint(p1_in), tracked_partitions);
    return true;
  }

  InsideResult insideResult2 = pointInsideTriangle(p2_in, triangle);
  if (insideResult2 == INSIDE)
  {
    // Split surface for this point
    splitSurface(triangleIndex, addPoint(p2_in), tracked_partitions);
    return true;
  }

  // No point is on the inside, so find a point of the segment that is on an
  // edge. Split that edge.

  size_t split_point_index;
  SimplePoint2d const * splitPoint2d;
  
  if (insideResult1 == EDGE)
  {
    split_point_index = addPoint(p1_in);
    splitPoint2d = &p1_2d_in;
  }
  else if (insideResult2 == EDGE)
  {
    split_point_index = addPoint(p2_in);
    splitPoint2d = &p2_2d_in;
  }
  else
  {
    return false;
  }
  
  // Split the edge of the triangle.
  // First, find out which edge.
  for (int c = 0; c < 3; ++c)
  {
    InsideResult r = pointInsideSegment(*splitPoint2d, triangle2d[c],
                                                       triangle2d[(c + 1) % 3]);
    if (r == INSIDE)
    {
      // This is the edge we want to split.
      splitEdge(triangleData.points[c],
                triangleData.points[(c + 1) % 3],
                split_point_index,
                tracked_partitions);
      return true;
    }
  }
  
  // If we reach this code, the point we found on an edge turned out not to
  // be on an edge. Should not happen.
  return false;
}

void MultiObjectTriangleContainer::prepareSetOperationSurface(int in_object1,
                                                              int in_object2,
                                                 int out_object_commonSurface,
                                                 int out_object_only1,
                                                 int out_object_only2)
{
  object_t in1 = getObjectBitmask(in_object1);
  object_t in2 = getObjectBitmask(in_object2);
  object_t in_common = in1 | in2;
  object_t out_common = getObjectBitmask(out_object_commonSurface);
  object_t out1 = getObjectBitmask(out_object_only1);
  object_t out2 = getObjectBitmask(out_object_only2);
  
  auto_ptr<mmrtree::RtreeT<3, size_t>::iterator> it(triangles_tree.entries());
  
  size_t const * triangleIndex;
  while( (triangleIndex = it->next()) != 0)
  {
    TriangleData& data = triangles[*triangleIndex];
    if ((data.object_membership & in_common) == in_common)
    {
      data.object_membership |= out_common;
    }
    else if ((data.object_membership & in1) != 0)
    {
      data.object_membership |= out1;
    }
    else if ((data.object_membership & in2) != 0)
    {
      data.object_membership |= out2;
    }
  }
}

// To prepare the six disjoint sets:
//
// The triangles in both sets are easy to find. For the other triangles,
// we have to find out whether these are on the inside of the other volume or
// on the outside. We do this by finding edges that are shared by both objects,
// which are the locations the surfaces intersect. For all triangles on these
// edges, we can follow a close path around the shared edge that intersects
// each triangle in turn. By keeping track which volume we are inside of, we
// know the result for these triangles. For triangles that do not share such
// an edge, we find out by their connection to a triangle we know about (which
// is just Tarjan's algorithm to find connected components in a graph).

// A working set for each output object other than the common objects is
// created. This set contains all triangles that have known output groups,
// but may have neighbours not visited yet.

// A triangle is marked as visited as soon as it is assigned its output group.
// Unless it is a member of both objects, it is added to the working set the
// same time. Once we look for its neighbours, it is removed from the set again.

void MultiObjectTriangleContainer::prepareSetOperationVolume(int in_object1,
                                                             int in_object2,
                                                 int out_object_commonSame,
                                                 int out_object_commonOpposite,
                                                 int out_object_only1_outside2,
                                                 int out_object_only2_outside1,
                                                 int out_object_only1_inside2,
                                                 int out_object_only2_inside1)
{
  stack<size_t> working_set_out1_outside;
  stack<size_t> working_set_out2_outside;
  stack<size_t> working_set_out1_inside;
  stack<size_t> working_set_out2_inside;

  stack<size_t>* working_sets[4] = { &working_set_out1_outside,
                                     &working_set_out2_outside,
                                     &working_set_out1_inside,
                                     &working_set_out2_inside };
  int working_set_in_objects[4] = { in_object1, in_object2,
                                    in_object1, in_object2 };
  int working_set_out_objects[4] = { out_object_only1_outside2,
                                     out_object_only2_outside1,
                                     out_object_only1_inside2,
                                     out_object_only2_inside1 };
  
  vector<bool> visited(triangles.size(), false);

  // Iterator over all triangles of object 1
  auto_ptr<mmrtree::RtreeT<3, size_t>::iterator> it(triangles_tree.entries());
  
  size_t const * triangleIndex;
  while( (triangleIndex = it->next()) != 0)
  {
    TriangleData& data = triangles[*triangleIndex];
    if (data.isMemberOfObject(in_object1))
    {
      if (data.isMemberOfObject(in_object2))
      {
        // Triangle is part of both volumes.
        bool direction_in1 = data.getDirectionOfObject(in_object1);
        bool direction_in2 = data.getDirectionOfObject(in_object2);
        if (direction_in1 == direction_in2)
        {
          data.setMembershipOfObject(out_object_commonSame, true);
          data.setDirectionOfObject(out_object_commonSame, direction_in1);
        }
        else
        {
          data.setMembershipOfObject(out_object_commonOpposite, true);
          data.setDirectionOfObject(out_object_commonOpposite, direction_in1);
        }
        visited[*triangleIndex] = true;
      }
      else
      {
        // Triangle is part of only in1. We have to find edges that are common
        // to both objects, so search for neighbours of object in2.
        for(int edge_n = 0; edge_n < 3; ++edge_n)
        {
          vector<size_t> neighbour_triangles
                            = getTrianglesForEdge(data.points[edge_n],
                                                  data.points[(edge_n+ 1) % 3]);
          SortedEdge edge(data.points[edge_n], data.points[(edge_n + 1) % 3],
                          data.points[(edge_n + 2) % 3], points);
          vector<SortedTriangle> sorted_triangles;
          bool we_have_neighbour_of_object2 = false;
          for (vector<size_t>::iterator neighbourIndex
                                          = neighbour_triangles.begin();
               neighbourIndex != neighbour_triangles.end(); ++neighbourIndex)
          {
            const TriangleData neighbour = triangles[*neighbourIndex];
            if (neighbour.isMemberOfObject(in_object1) ||
                neighbour.isMemberOfObject(in_object2))
            {
              sorted_triangles.push_back(SortedTriangle(edge, *neighbourIndex,
                                                        neighbour, points));
            }
            if (neighbour.isMemberOfObject(in_object2))
            {
              we_have_neighbour_of_object2 = true;
            }
          }
          if (!we_have_neighbour_of_object2)
            continue;
          // We now know we have an edge shared by both objects.
          std::sort(sorted_triangles.begin(), sorted_triangles.end(),
                    triangleSort);
          bool inside1, inside2;
          // Walk around the edge to see whether we are inside or outside
          // the objects for the first triangle.
          for(vector<SortedTriangle>::iterator sortedNeighbour
                                         = sorted_triangles.begin();
              sortedNeighbour != sorted_triangles.end(); ++sortedNeighbour)
          {
            bool isObj1 = sortedNeighbour->isInObject(in_object1);
            bool isObj2 = sortedNeighbour->isInObject(in_object2);
            if (isObj1)
            {
              // For the next triangle, we know whether we are inside or
              // outside of object 1.
              TrianglePathDirection direction
                         = sortedNeighbour->getDirectionForObject(in_object1);
              switch (direction)
              {
                case ENTER:
                  inside1 = true;
                  break;
                case LEAVE:
                  inside1 = false;
                  break;
              }
            }
            if (isObj2)
            {
              // For the next triangle, we know whether we are inside or
              // outside of object 2.
              TrianglePathDirection direction
                         = sortedNeighbour->getDirectionForObject(in_object2);
              switch (direction)
              {
                case ENTER:
                  inside2 = true;
                  break;
                case LEAVE:
                  inside2 = false;
                  break;
              }
            }
          }
          // As we had a triangle of both objects on this edge, inside1 and
          // inside2 now have the right values for the starting position.
          // Walk around the edge a second time to put the triangles into
          // the right working sets.
          for(vector<SortedTriangle>::iterator sortedNeighbour
                                         = sorted_triangles.begin();
              sortedNeighbour != sorted_triangles.end(); ++sortedNeighbour)
          {
            // First, put the triangle into the right working set if
            // - it only belongs to one of the objects and
            // - it is not yet marked as visited
            bool isObj1 = sortedNeighbour->isInObject(in_object1);
            bool isObj2 = sortedNeighbour->isInObject(in_object2);
            if (!visited[sortedNeighbour->triangle])
            {
              size_t triangleIndex = sortedNeighbour->triangle;
              TriangleData& triangleData = triangles[triangleIndex];
              object_t out_object = 0;
              bool out_direction;
              if (isObj1 && !isObj2)
              {
                out_direction = triangleData.getDirectionOfObject(in_object1);
                if (inside2)
                {
                  working_set_out1_inside.push(triangleIndex);
                  out_object = out_object_only1_inside2;
                }
                else
                {
                  working_set_out1_outside.push(triangleIndex);
                  out_object = out_object_only1_outside2;
                }
              }
              if (isObj2 && !isObj1)
              {
                out_direction = triangleData.getDirectionOfObject(in_object2);
                if (inside1)
                {
                  working_set_out2_inside.push(triangleIndex);
                  out_object = out_object_only2_inside1;
                }
                else
                {
                  working_set_out2_outside.push(triangleIndex);
                  out_object = out_object_only2_outside1;
                }
              }
              if (out_object != 0)
              {
                triangleData.setMembershipOfObject(out_object, true);
                triangleData.setDirectionOfObject(out_object, out_direction);
                visited[triangleIndex] = true;
              }
            }
            if (isObj1)
            {
              // For the next triangle, we know whether we are inside or
              // outside of object 1.
              TrianglePathDirection direction
                         = sortedNeighbour->getDirectionForObject(in_object1);
              switch (direction)
              {
                case ENTER:
                  inside1 = true;
                  break;
                case LEAVE:
                  inside1 = false;
                  break;
              }
            }
            if (isObj2)
            {
              // For the next triangle, we know whether we are inside or
              // outside of object 2.
              TrianglePathDirection direction
                         = sortedNeighbour->getDirectionForObject(in_object2);
              switch (direction)
              {
                case ENTER:
                  inside2 = true;
                  break;
                case LEAVE:
                  inside2 = false;
                  break;
              }
            }
          }
        }
      }
    }
  }
  // Now, all triangles that are a member of both volumes are already in their
  // destination object. All triangles that share an edge with a triangle of
  // the other object have been classified into the right working set. From
  // here, we have to transitively follow all their neighbours of the same
  // object and put them into the same working set.
  
  for(int c = 0; c < 4; ++c)
  {
    stack<size_t>& working_set = *working_sets[c];
    int working_set_in_object = working_set_in_objects[c];
    int working_set_out_object = working_set_out_objects[c];
    
    while(!working_set.empty())
    {
      size_t currentIndex = working_set.top();
      working_set.pop();

      const TriangleData& currentData = triangles[currentIndex];
      
      // Find all neighbours of the current triangle
      for (int edge = 0; edge < 3; ++edge)
      {
        vector<size_t> neighbour_triangles
                    = getTrianglesForEdge(currentData.points[edge],
                                          currentData.points[(edge + 1) % 3]);
        for (vector<size_t>::iterator neighbourIndex
                                        = neighbour_triangles.begin();
             neighbourIndex != neighbour_triangles.end(); ++neighbourIndex)
        {
          if (visited[*neighbourIndex])
            continue;
          TriangleData& neighbourData = triangles[*neighbourIndex];
          if (neighbourData.isMemberOfObject(working_set_in_object))
          {
            // Neighbour triangle is not visited and part of the same object,
            // so it is also a member of the same result set. Put it into
            // the current working set.
            working_set.push(*neighbourIndex);
            neighbourData.setMembershipOfObject(working_set_out_object, true);
            neighbourData.setDirectionOfObject(working_set_out_object,
                    neighbourData.getDirectionOfObject(working_set_in_object));
            visited[*neighbourIndex] = true;
          }
        }
      }
    }
  }
  
  // Now, there are triangles left that are not visited.

  auto_ptr<mmrtree::RtreeT<3, size_t>::iterator> it2(triangles_tree.entries());
  
  while( (triangleIndex = it2->next()) != 0)
  {
    if (visited[*triangleIndex])
      continue;
    TriangleData& data = triangles[*triangleIndex];
    if (data.isMemberOfObject(in_object1))
    {
      data.setMembershipOfObject(out_object_only1_outside2, true);
      data.setDirectionOfObject(out_object_only1_outside2,
                                data.getDirectionOfObject(in_object1));
      visited[*triangleIndex] = true;
    }
    if (data.isMemberOfObject(in_object2))
    {
      data.setMembershipOfObject(out_object_only2_outside1, true);
      data.setDirectionOfObject(out_object_only2_outside1,
                                data.getDirectionOfObject(in_object2));
      visited[*triangleIndex] = true;
    }
  }
}
