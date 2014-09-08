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

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

2 Source file "Optics.cpp"[4]

March-October 2014, Marius Haug

2.1 Overview

This file contains the "Optics"[4] class definition.

2.2 Includes

*/
#include "Optics.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include <vector>

namespace clusteropticsalg
{

/*
2.3 Implementation of the class ~Optics~

*/
/*
Default constructor ~Optics::Optics~

*/
  Optics::Optics()
  {
    mtree = NULL;
    return;
  }
/*
Method ~Optics::order~

*/
  void Optics::order(TupleBuffer* objs, int eps, unsigned int minPts
    ,TupleBuffer* order)
  {
//printf("--------------- order\n");
    GenericRelationIterator *relIter = objs->MakeScan();
    Tuple* obj;

    while((obj = relIter->GetNextTuple()))
    {
      if(!((CcBool*)obj->GetAttribute(PRC))->GetValue())
      {
        expandClusterOrder(objs, obj, eps, minPts, order);
      }
    }
  }
/*
Method ~Optics::expandClusterOrder~

*/
  void Optics::expandClusterOrder(TupleBuffer* objs, Tuple* obj
    ,int eps, unsigned int minPts, TupleBuffer* order)
  {
//printf("--------------- expandClusterOrder\n");
    std::list<Tuple*> orderedSeeds;
    
    std::list<Tuple*> neighbors = getNeighbors(objs, obj, eps);

    CcBool processed(true, true);
    obj->PutAttribute(PRC, ((Attribute*) &processed)->Clone());
    
    CcReal rDist(UNDEFINED);
    obj->PutAttribute(REA, ((Attribute*) &rDist)->Clone());

    setCoreDistance(neighbors, obj, eps, minPts);
    //orderedOut add the Point obj
    order->AppendTuple(obj);    

    //(obj->distCore != UNDEFINED)
    if(((CcReal*)obj->GetAttribute(COR))->GetValue() != UNDEFINED) 
    {
      update(neighbors, obj, orderedSeeds);

      std::list<Tuple*>::iterator it;
      for (it = orderedSeeds.begin(); it != orderedSeeds.end(); it++)
      {
        Tuple* currentObject = *it;

        std::list<Tuple*> neighbors = getNeighbors(objs, currentObject, eps);
        
        CcBool processed(true, true);
        currentObject->PutAttribute(PRC, ((Attribute*) &processed)->Clone());

        setCoreDistance(neighbors, currentObject, eps, minPts);
        //orderedOut add the Point currentObject
        order->AppendTuple(currentObject);
        
        //(currentObject->distCore != UNDEFINED)
        if(((CcReal*)currentObject->GetAttribute(COR))->GetValue()
          != UNDEFINED) 
        {
          update(neighbors, currentObject, orderedSeeds);
        }
      }
    }
  }
/*
Method ~Optics::getNeighbors~

*/    
  std::list<Tuple*> Optics::getNeighbors(TupleBuffer* objs, Tuple* obj
    ,int eps)
  {
//printf("--------------- getNeigbors\n");
    std::list<Tuple*> near;

    if(mtree != NULL)
    {
//printf("--------------- MTREE USED\n");
      list<TupleId> ids;
      mtree->rangeSearch((Attribute*)obj->GetAttribute(PNT), eps, &ids);
      
      std::list<TupleId>::iterator it = ids.begin();
      for (it = ids.begin(); it != ids.end(); it++)
      {
        TupleId id = *it;
        Tuple* point;
        point = objs->GetTuple(id, true);
        near.push_back(point);
      }
      
//printf("--------------- near.size =  %d\n", near.size());
    }
    else
    {
      GenericRelationIterator *relIter = objs->MakeScan();
      Tuple* point;

      while((point = relIter->GetNextTuple()))
      {
        if(((Point*)obj->GetAttribute(PNT))->GetX()
        != ((Point*)point->GetAttribute(PNT))->GetX()
        || ((Point*)obj->GetAttribute(PNT))->GetY()
        != ((Point*)point->GetAttribute(PNT))->GetY())
        {
          if(((((Point*)obj->GetAttribute(PNT))->GetX()
             - ((Point*)point->GetAttribute(PNT))->GetX()) 
            * (((Point*)obj->GetAttribute(PNT))->GetX()
             - ((Point*)point->GetAttribute(PNT))->GetX()))
           + ((((Point*)obj->GetAttribute(PNT))->GetY()
             - ((Point*)point->GetAttribute(PNT))->GetY()) 
            * (((Point*)obj->GetAttribute(PNT))->GetY() 
             - ((Point*)point->GetAttribute(PNT))->GetY())) < eps*eps) 
          {
            near.push_back(point);
          }
        }
      }
    }

    return near;
  }
/*
Method ~Optics::setCoreDistance~

*/
  void Optics::setCoreDistance(std::list<Tuple*>& neighbors, Tuple* obj
    ,int eps, unsigned int minPts)
  {
//printf("--------------- setCoreDistance\n");
    double curDist  = UNDEFINED;
    double lastDist = UNDEFINED;
    double coreDist = UNDEFINED;
    unsigned int count   = 0;
    unsigned int biggest = 0;
    Tuple* nearest[minPts];

    if(neighbors.size() < minPts)
    {
      CcReal cDist(UNDEFINED);
      obj->PutAttribute(COR, ((Attribute*) &cDist)->Clone());
      return;
    }

    std::list<Tuple*>::iterator it;
    for (it = neighbors.begin(); it != neighbors.end(); it++)
    {
      Tuple* neighbor = *it;

      if(count < minPts)
      {
        nearest[count++] = neighbor;

        curDist = sqrt(((((Point*)obj->GetAttribute(PNT))->GetX()
                       - ((Point*)neighbor->GetAttribute(PNT))->GetX())
                      * (((Point*)obj->GetAttribute(PNT))->GetX()
                       - ((Point*)neighbor->GetAttribute(PNT))->GetX()))
                     + ((((Point*)obj->GetAttribute(PNT))->GetY()
                       - ((Point*)neighbor->GetAttribute(PNT))->GetY()) 
                      * (((Point*)obj->GetAttribute(PNT))->GetY()
                       - ((Point*)neighbor->GetAttribute(PNT))->GetY())));

        if(curDist > lastDist)
        {
          biggest = count-1;
          coreDist = curDist;
        }

        lastDist = curDist;
      }
      else
      {
        curDist = sqrt(((((Point*)obj->GetAttribute(PNT))->GetX()
                       - ((Point*)neighbor->GetAttribute(PNT))->GetX()) 
                      * (((Point*)obj->GetAttribute(PNT))->GetX()
                       - ((Point*)neighbor->GetAttribute(PNT))->GetX()))
                     + ((((Point*)obj->GetAttribute(PNT))->GetY()
                       - ((Point*)neighbor->GetAttribute(PNT))->GetY()) 
                      * (((Point*)obj->GetAttribute(PNT))->GetY()
                       - ((Point*)neighbor->GetAttribute(PNT))->GetY())));

        for (unsigned int i = 0; i < count; i++)
        {
          Tuple* curNear = nearest[i];

          double cDst = sqrt(((((Point*)obj->GetAttribute(PNT))->GetX()
                             - ((Point*)curNear->GetAttribute(PNT))->GetX())
                            * (((Point*)obj->GetAttribute(PNT))->GetX()
                             - ((Point*)curNear->GetAttribute(PNT))->GetX()))
                           + ((((Point*)obj->GetAttribute(PNT))->GetY()
                             - ((Point*)curNear->GetAttribute(PNT))->GetY())
                            * (((Point*)obj->GetAttribute(PNT))->GetY()
                             - ((Point*)curNear->GetAttribute(PNT))->GetY())));

          if(cDst > lastDist)
          {
            biggest = i;
            coreDist = cDst;
          }

          lastDist = cDst;
        }

        Tuple* curNear = nearest[biggest];
        
        double dstnc = sqrt(((((Point*)obj->GetAttribute(PNT))->GetX()
                            - ((Point*)curNear->GetAttribute(PNT))->GetX()) 
                           * (((Point*)obj->GetAttribute(PNT))->GetX()
                            - ((Point*)curNear->GetAttribute(PNT))->GetX())) 
                          + ((((Point*)obj->GetAttribute(PNT))->GetY()
                            - ((Point*)curNear->GetAttribute(PNT))->GetY()) 
                           * (((Point*)obj->GetAttribute(PNT))->GetY()
                            - ((Point*)curNear->GetAttribute(PNT))->GetY())));

        if(dstnc > curDist)
        {
          nearest[biggest] = neighbor;
        }
      }
    }

    CcReal cDist(coreDist);
    obj->PutAttribute(COR, ((Attribute*) &cDist)->Clone());
  }
/*
Method ~Optics::update~

*/
  void Optics::update(std::list<Tuple*>& neighbors, Tuple* center
    ,std::list<Tuple*>& orderedSeeds)
  {
//printf("--------------- update\n");
    //c_dist := CenterObject.core_distance;
    //double coreDist = ((CcReal*)center->GetAttribute(COR))->GetValue();

    //FORALL Object FROM neighbors DO
    std::list<Tuple*>::iterator it = neighbors.begin();
    for (it = neighbors.begin(); it != neighbors.end(); it++)
    {
      Tuple* obj = *it;

      //IF NOT Object.Processed THEN      
      if(!((CcBool*)obj->GetAttribute(PRC))->GetValue()) //(!obj->processed)
      {
        //new_r_dist:=max(c_dist,CenterObject.dist(Object));
        double newReachDist = getReachableDist(center, obj);

        //IF Object.reachability_distance=UNDEFINED THEN
        if(((CcReal*)obj->GetAttribute(REA))->GetValue() == UNDEFINED) 
        {
        
          //Object.reachability_distance := new_r_dist;
          CcReal rDist(newReachDist);
          obj->PutAttribute(REA, ((Attribute*) &rDist)->Clone());
          insert(orderedSeeds, obj);
        }
        //ELSE IF new_r_dist<Object.reachability_distance THEN
        // Object already in OrderSeeds
        else if(newReachDist < ((CcReal*)obj->GetAttribute(REA))->GetValue())
        {
          //Object.reachability_distance := new_r_dist;
          CcReal rDist(newReachDist);
          obj->PutAttribute(REA, ((Attribute*) &rDist)->Clone());
          decrease(orderedSeeds, obj);
        }
      }
    }
  }
/*
Method ~Optics::insert~

*/
  void Optics::insert(std::list<Tuple*>& orderedSeeds, Tuple* obj)
  {
//printf("--------------- insert\n");
    std::list<Tuple*>::iterator it;

    for (it = orderedSeeds.begin(); it != orderedSeeds.end(); it++)
    {
      Tuple* seed = *it;
      if(((CcReal*)seed->GetAttribute(REA))->GetValue()
       > ((CcReal*)obj->GetAttribute(REA))->GetValue())
      {
        orderedSeeds.insert(it, obj);
        return;
      }
    }
    
    orderedSeeds.push_back(obj);
  }
/*
Method ~Optics::decrease~

*/
  void Optics::decrease(std::list<Tuple*>& orderedSeeds, Tuple* obj)
  {
//printf("--------------- decrease\n");
    std::list<Tuple*>::iterator it;
    std::list<Tuple*>::iterator itObj;

    for (itObj = orderedSeeds.begin(); itObj != orderedSeeds.end(); itObj++)
    {
      if(obj == ((Tuple*) *itObj))
      {
        for (it = itObj; it != orderedSeeds.begin(); --it)
        {
          Tuple* seed = *it;
          if(((CcReal*)seed->GetAttribute(REA))->GetValue() 
           > ((CcReal*)obj->GetAttribute(REA))->GetValue())
          {
            orderedSeeds.splice(itObj, orderedSeeds, it);
            return;
          }
        }
      }
    }
  }

  double Optics::getReachableDist(Tuple* obj, Tuple* neighbor)
  {
//printf("--------------- getReachableDist\n");
    double distance = sqrt(((((Point*)obj->GetAttribute(PNT))->GetX() 
                           - ((Point*)neighbor->GetAttribute(PNT))->GetX()) 
                          * (((Point*)obj->GetAttribute(PNT))->GetX()
                           - ((Point*)neighbor->GetAttribute(PNT))->GetX()))
                         + ((((Point*)obj->GetAttribute(PNT))->GetY()
                           - ((Point*)neighbor->GetAttribute(PNT))->GetY()) 
                          * (((Point*)obj->GetAttribute(PNT))->GetY() 
                           - ((Point*)neighbor->GetAttribute(PNT))->GetY())));

    //new_r_dist:=max(c_dist,CenterObject.dist(Object));        
    return ((CcReal*)obj->GetAttribute(COR))->GetValue() > distance 
      ? ((CcReal*)obj->GetAttribute(COR))->GetValue() 
      : distance;
  }  
}
