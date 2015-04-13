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

#include "Spatial3D.h"
#include "MMRTree.h"
#include "Stream.h"

namespace spatial3DOperatorComponents
{

  ListExpr ComponentsTypeMapping(ListExpr args)
  {
    if (!nl->HasLength(args, 1))
    {
      return listutils::typeError("wrong number of arguments");
    }
    if (Surface3d::checkType(nl->First(args)))
    {
      return nl->TwoElemList(listutils::basicSymbol<Stream<Surface3d> >(),
                             listutils::basicSymbol<Surface3d>());
    }
    if (Volume3d::checkType(nl->First(args)))
    {
      return nl->TwoElemList(listutils::basicSymbol<Stream<Volume3d> >(),
                             listutils::basicSymbol<Volume3d>());
    }
    return listutils::typeError("surface3d or volume3d expected");
  }

  template<class T> class ComponentsLI
  {

  public:

    ComponentsLI(T* arg)
      : components()
    {
      if (!arg->IsDefined()) {
        return;
      }
      mmrtree::RtreeT<3, size_t> triangles(4, 8);
      
      size_t arg_size = arg->size();
      for (int c = 0; c < arg_size; ++c) {
        triangles.insert(arg->get(c).BoundingBox(), c);
      }
      
      vector<bool> visited(arg_size);
      
      size_t first_unvisited_triangle_index = 0;
      
      for(;;) {
        // find a new component for each iteration
        
        // Start by finding first triangle in argument that is not yet visited
        // by search.
        
        while(first_unvisited_triangle_index < arg_size &&
              visited[first_unvisited_triangle_index]) {
          ++first_unvisited_triangle_index;
        }
        
        if (first_unvisited_triangle_index >= arg_size)
          break;
        
        stack<size_t> to_be_searched_for_neighbours;
        vector<size_t> all_neighbours_found;
        
        to_be_searched_for_neighbours.push(first_unvisited_triangle_index);
        visited[first_unvisited_triangle_index] = true;
        
        while(to_be_searched_for_neighbours.size() > 0) {
          size_t current_index = to_be_searched_for_neighbours.top();
          to_be_searched_for_neighbours.pop();
          Triangle current_triangle = arg->get(current_index);
          // find all undiscovered neighbours
          mmrtree::RtreeT<3, size_t>::iterator* it;
          Rectangle<3> bbox = current_triangle.BoundingBox();
          bbox.Extend(0.0001);
          it = triangles.find(bbox);
          size_t const * index;
          while((index = it->next()) != 0) {
            size_t potential_neighbour_index = *index;
            if (visited[potential_neighbour_index])
              continue;
            Triangle potential_neighbour_triangle
              = arg->get(potential_neighbour_index);
            if (doTrianglesShareEdge(current_triangle,
                                     potential_neighbour_triangle)) {
              to_be_searched_for_neighbours.push(potential_neighbour_index);
              visited[potential_neighbour_index] = true;
            }
          }
          delete it;
          all_neighbours_found.push_back(current_index);
        }
        
        T* next = new T(true);
        next->startBulkLoad();
        size_t s = all_neighbours_found.size();
        for (int c = 0; c < s; ++c) {
          next->add(arg->get(all_neighbours_found[c]));
        }
        next->endBulkLoad(NO_REPAIR);
        components.push(next);
      }
    }
    
    ~ComponentsLI()
    {
      while(components.size() > 0) {
        delete components.top();
        components.pop();
      }
    }
    
    T* getNext()
    {
      if (components.size() > 0) {
        T* result = components.top();
        components.pop();
        return result;
      }
      else {
        return NULL;
      }
    }
    
  private:

    stack<T*> components;
    
    bool doTrianglesShareEdge(const Triangle& t1, const Triangle& t2)
    {
      SimplePoint3d points1[3] = { t1.getA(), t1.getB(), t1.getC() };
      SimplePoint3d points2[3] = { t2.getA(), t2.getB(), t2.getC() };
      
      int common_points = 0;
      for (int c1 = 0; c1 < 3; ++c1) {
        for (int c2 = 0; c2 < 3; ++c2) {
          if (points1[c1] == points2[c2]) {
            ++common_points;
            continue;
          }
        }
      }
      return common_points >= 2;
    }
  };
  
  template<class T>
  int componentsVMT(Word* args, Word& result, int message,
                    Word& local, Supplier s)
  {
    ComponentsLI<T>* li = (ComponentsLI<T>*) local.addr;
    switch (message)
    {
      case OPEN:
        if (li)
        {
          delete li;
        }
        local.addr = new ComponentsLI<T>((T*) args[0].addr);
        return 0;
      case REQUEST:
        result.addr = li ? li->getNext() : 0;
        return result.addr ? YIELD : CANCEL;
      case CLOSE:
        if (li)
        {
          delete li;
          local.addr = 0;
        }
        return 0;
    }
    return 0;
  }
  
  ValueMapping componentsValueMapping[] = {
    componentsVMT<Surface3d>,
    componentsVMT<Volume3d>
  };

  int ComponentsSelect(ListExpr args)
  {
    if (Surface3d::checkType(nl->First(args)))
    {
      return 0;
    }
    if (Volume3d::checkType(nl->First(args)))
    {
      return 1;
    }
    return -1;
  }
  
  OperatorSpec ComponentsSpec(
    "surface3d -> stream(surface3d)",
    "_ components",
    "Divides a surface3d or a volume3d into its components",
    "query object components consume"
  );
  
  Operator* getComponentsPtr()
  {
    return new Operator(
      "components",
      ComponentsSpec.getStr(),
      2,
      componentsValueMapping,
      ComponentsSelect,
      ComponentsTypeMapping
    );
  }
}