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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Header file of the AreaOp

September, 20017 Torsten Weidmann

[TOC]

1 Overview

This header file defines the class AreaOp, Edge, CurveLink and ChainEnd.

2 Defines and includes

*/

#ifndef SECONDO_AREAOP_H
#define SECONDO_AREAOP_H

#include "Curve.h"

namespace salr {

  class Edge {
  public:

    Edge(Curve* c, int ctag);

    Edge(Curve* c, int ctag, int etag);

    Curve* getCurve() const;

    int getCurveTag();

    int getEdgeTag();

    void setEdgeTag(int etag);

    int getEquivalence();

    void setEquivalence(int eq);

    int compareTo(Edge* other, double *yrange);

    void record(double yend, int etag);

    bool isActiveFor(double y, int etag);

    bool operator< (const Edge& str) const;

  private:
    static const int INIT_PARTS = 4;
    static const int GROW_PARTS = 10;

    Curve* curve;
    int ctag;
    int etag;
    double activey;
    int equivalence;

    Edge* lastEdge;
    int lastResult;
    double lastLimit;
  };

  class CurveLink {
  public:

    CurveLink(Curve* curve, double ystart, double yend, int etag);

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

    CurveLink* getNext();

  private:
    Curve* curve;
    double ytop;
    double ybot;
    int etag;

    CurveLink* next;
  };

  class ChainEnd {
  public:
    ChainEnd(CurveLink* first, ChainEnd* partner);

    CurveLink* getChain();

    void setOtherEnd(ChainEnd* partner);

    ChainEnd* getPartner();

    /*
     * Returns head of a complete chain to be added to subcurves
     * or null if the links did not complete such a chain.
     */
    CurveLink* linkTo(ChainEnd* that);

    void addLink(CurveLink* newlink);

    double getX();

  private:
    CurveLink* head;
    CurveLink* tail;
    ChainEnd* partner;
    int etag;
  };

  class AreaOp {
  public:

    /* Constants to tag the left and right curves in the edge list */
    static const int CTAG_LEFT = 0;
    static const int CTAG_RIGHT = 1;

    /* Constants to classify edges */
    static const int ETAG_IGNORE = 0;
    static const int ETAG_ENTER = 1;
    static const int ETAG_EXIT = -1;

    /* Constants used to classify result state */
    static const int RSTAG_INSIDE = 1;
    static const int RSTAG_OUTSIDE = -1;

    void newRow();

    int classify(Edge* e);

    int getState();

    static void finalizeSubCurves(std::vector<CurveLink*>* subcurves,
                                  std::vector<ChainEnd*>* chains);

    /*
     * Does the position of the next edge at v1 "obstruct" the
     * connectivity between current edge and the potential
     * partner edge which is positioned at v2?
     *
     * Phase tells us whether we are testing for a transition
     * into or out of the interior part of the resulting area.
     *
     * Require 4-connected continuity if this edge and the partner
     * edge are both "entering into" type edges
     * Allow 8-connected continuity for "exiting from" type edges
     */
    static bool obstructs(double v1, double v2, int phase);

    static void resolveLinks(std::vector<CurveLink*> *subcurves,
                             std::vector<ChainEnd*> *chains,
                             std::vector<CurveLink*> *links);

    void calculate(std::vector<Curve*>* left,
                   std::vector<Curve*>* right,
                   std::vector<Curve*>* result);

  private:
    int count;

    static void addEdges(std::vector<Edge*> *edges,
                         std::vector<Curve*> *curves, int curvetag);

   void pruneEdges(std::vector<Edge*>* edges, std::vector<Curve*>* result);

  };

}

#endif //SECONDO_AREAOP_H
