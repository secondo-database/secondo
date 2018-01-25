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

6 AreaOp

6.1 Overview

This file defines the classes ~Edge~, ~CurveLink~, ~ChainEnd~ and ~AreaOp~.
Additionally it defines the child classes of ~AreaOp~:

  * NZWindOp
  * EOWindOp
  * CAGOp
  * UnionOp
  * MinusOp
  * IntersectsOp
  * XorOp

Defines and includes.

*/

#ifndef SECONDO_AREAOP_H
#define SECONDO_AREAOP_H

#include "Curve.h"

#include <vector>

namespace salr {

/*
6.2 Class ~Edge~

Used as wrapper class around a ~Curve~. Stores meta information like the
 which ~Region2~ it belongs to.

*/
  class Edge {
  public:

/*
Declaration of constructors.

*/
    Edge(Curve* c, int ctag);
    Edge(Curve* c, int ctag, int etag);

/*
Declaration of custom methods.

*/
    Curve* getCurve() const;
    int getCurveTag();
    int getEdgeTag();
    void setEdgeTag(int etag);
    int getEquivalence();
    void setEquivalence(int eq);
    int compareTo(Edge* other, double *yrange);
    void record(double yend, int etag);
    bool isActiveFor(double y, int etag);

  private:
/*
Fields used to add meta information to this ~Edge~.

*/
    Curve* curve;
    int ctag;
    int etag;
    double activey;
    int equivalence;

    Edge* lastEdge;
    int lastResult;
    double lastLimit;
  };

/*
6.3 Class ~CurveLink~

Linked list used to create a list of ~Curves~ representing a subcurve.

*/
  class CurveLink {
  public:

/*
Declaration of constructor.

*/
    CurveLink(Curve* curve, double ystart, double yend, int etag);

/*
Declaration of custom methods.

*/
    bool absorb(CurveLink* link);
    bool absorb(Curve* curve, double ystart, double yend, int etag);
    bool isEmpty();
    Curve* getCurve();
    Curve* getSubCurve();
    Curve* getMoveto();
    double getXTop();
    double getYTop();
    double getXBot();
    double getYBot();
    double getX();
    int getEdgeTag();
    void setNext(CurveLink* link);
    void resetCurve();
    CurveLink* getNext();

  private:
/*
Fields used to add meta information to this ~CurveLink~.

*/
    Curve* curve;
    double ytop;
    double ybot;
    int etag;
    bool hasNext;

    CurveLink* next;
  };

/*
6.4 Class ~ChainEnd~

Used as endpoints for subcurves created with ~CurveLink~.

*/
  class ChainEnd {
  public:

/*
Declaration of constructor.

*/
    ChainEnd(CurveLink* first, ChainEnd* partner);

/*
Declaration of custom methods.

*/
    CurveLink* getChain();
    void setOtherEnd(ChainEnd* partner);
    ChainEnd* getPartner();
    CurveLink* linkTo(ChainEnd* that);
    void addLink(CurveLink* newlink);
    double getX();

  private:
/*
Fields used to store connections modelling a subcurve:

  * head: Beginning of subcurve
  * tail: End of subcurve
  * partner: ~ChainEnd~ at other end of subcurve

*/
    CurveLink* head;
    CurveLink* tail;
    ChainEnd* partner;
    int etag;
  };

/*
6.5 Class ~AreaOp~

Abstract class containing methods used for geometric operations.

*/
  class AreaOp {
  public:
/*
Constants to tag the left and right curves in the edge list.

*/
    static const int CTAG_LEFT = 0;
    static const int CTAG_RIGHT = 1;

/*
Constants to classify edges .

*/
    static const int ETAG_IGNORE = 0;
    static const int ETAG_ENTER = 1;
    static const int ETAG_EXIT = -1;

/*
Constants used to classify result state .

*/
    static const int RSTAG_INSIDE = 1;
    static const int RSTAG_OUTSIDE = -1;

/*
Declaration of virtual methods.

*/
    virtual void newRow() = 0;
    virtual int classify(Edge* e) = 0;
    virtual int getState() = 0;

/*
Declaration of class methods.

*/
    static void finalizeSubCurves(std::vector<CurveLink*>* subcurves,
                                  std::vector<ChainEnd*>* chains);
    static bool obstructs(double v1, double v2, int phase);
    void resolveLinks(std::vector<CurveLink*> *subcurves,
                             std::vector<ChainEnd*> *chains,
                             std::vector<CurveLink*> *links);

    void calculate(std::vector<Curve*>* left, std::vector<Curve*>* right);

  protected:
    void addEdges(std::vector<Edge*> *edges,
                         std::vector<Curve*> *curves, int curvetag);

    void pruneEdges(std::vector<Edge*>* edges, std::vector<Curve*>* result);

  private:

    std::vector<CurveLink*> allCurveLinks;
    std::vector<ChainEnd*> allChainEnds;
    std::vector<Curve*> startCurves;

    void cleanupPointer();

  };

/*
6.5.1 Class ~NZWindOp~

Child of ~AreaOp~. Can be used during ~Region2~ construction.

*/
  class NZWindOp : public AreaOp {
  public:

/*
Declaration of class methods.

*/
    void newRow();
    int classify(Edge* e);
    int getState();

  private:
    int count;
  };

/*
6.5.2 Class ~EOWindOp~

Child of ~AreaOp~. Can be used during ~Region2~ construction.

*/
  class EOWindOp : public AreaOp {
  public:

/*
Declaration of class methods.

*/
    void newRow();
    int classify(Edge* e);
    int getState();

  private:
    bool inside;
  };

/*
6.5.3 Class ~CAGOp~

Abstract child class of ~AreaOp~. Child classes are used to implement
 criteria by which to classify an ~Edge~.

*/
  class CAGOp : public AreaOp {
  public:

/*
Declaration of class methods.

*/
    void newRow();
    int classify(Edge* e);
    int getState();


/*
Declaration of virtual method.

*/
    virtual bool newClassification(bool inLeft, bool inRight) = 0;

  private:
    bool inLeft;
    bool inRight;
    bool inResult;
  };

/*
6.5.3 Class ~UnionOp~

Child of ~CAGOp~. Used for union operation.

*/
  class UnionOp : public CAGOp {
  public:
    bool newClassification(bool inLeft, bool inRight);
  };

/*
6.5.4 Class ~MinusOp~

Child of ~CAGOp~. Used for minus operation.

*/
  class MinusOp : public CAGOp {
  public:
    bool newClassification(bool inLeft, bool inRight);
  };

/*
6.5.5 Class ~IntersectsOp~

Child of ~CAGOp~. Used for intersection operation.

*/
  class IntersectsOp : public CAGOp {
  public:
    bool newClassification(bool inLeft, bool inRight);
  };

/*
6.5.6 Class ~XorOp~

Child of ~CAGOp~. Used for xor operation.

*/
  class XorOp : public CAGOp {
  public:
    bool newClassification(bool inLeft, bool inRight);
  };

}

#endif //SECONDO_AREAOP_H
