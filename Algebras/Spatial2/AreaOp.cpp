/*
----
This file is part of SECONDO.

Copyright (C) 2016, University in Hagen,
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


*/

#include "AreaOp.h"
#include "Curve.h"

#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <iostream>

using namespace std;

namespace salr {

  Edge::Edge(Curve *c, int ctag) :
        curve(c), ctag(ctag), etag(AreaOp::ETAG_IGNORE) {
  }

  Edge::Edge(Curve *c, int ctag, int etag) : curve(c), ctag(ctag), etag(etag) {
  }

  Curve *Edge::getCurve() const {
    return curve;
  }

  int Edge::getCurveTag() {
    return ctag;
  }

  int Edge::getEdgeTag() {
    return etag;
  }

  void Edge::setEdgeTag(int etag) {
    this->etag = etag;
  }

  int Edge::getEquivalence() {
    return equivalence;
  }

  void Edge::setEquivalence(int eq) {
    equivalence = eq;
  }

  int Edge::compareTo(Edge *other, double *yrange) {
    if (other == lastEdge && yrange[0] < lastLimit) {
      if (yrange[1] > lastLimit) {
        yrange[1] = lastLimit;
      }
      return lastResult;
    }
    if (this == other->lastEdge && yrange[0] < other->lastLimit) {
      if (yrange[1] > other->lastLimit) {
        yrange[1] = other->lastLimit;
      }
      return 0 - other->lastResult;
    }
    int ret = curve->compareTo(other->curve, yrange);
    lastEdge = other;
    lastLimit = yrange[1];
    lastResult = ret;
    return ret;
  }

  void Edge::record(double yend, int etag) {
    this->activey = yend;
    this->etag = etag;
  }

  bool Edge::isActiveFor(double y, int etag) {
    return (this->etag == etag && this->activey >= y);
  }

  bool Edge::operator<(const Edge &str) const {
    Curve *c1 = getCurve();
    Curve *c2 = str.getCurve();
    double v1, v2;
    if ((v1 = c1->getYTop()) == (v2 = c2->getYTop())) {
      if ((v1 = c1->getXTop()) == (v2 = c2->getXTop())) {
        return 0;
      }
    }
    if (v1 < v2) {
      return -1;
    }
    return 1;
  }

  CurveLink::CurveLink(Curve *c, double ystart, double yend, int etag) :
                        curve(c), ytop(ystart), ybot(yend), etag(etag) {
    if (ytop < c->getYTop() || ybot > c->getYBot()) {
      stringstream sstm;
      sstm << "bad curvelink [" << ytop << "=>" << ybot << "] for " << &c;
      throw runtime_error(sstm.str());
    }
  }

  bool CurveLink::absorb(CurveLink *link) {
    return absorb(link->curve, link->ytop, link->ybot, link->etag);
  }

  bool CurveLink::absorb(Curve *curve, double ystart, double yend, int etag) {
    if (this->curve != curve || this->etag != etag ||
        ybot < ystart || ytop > yend) {
      return false;
    }
    if (ystart < curve->getYTop() || yend > curve->getYBot()) {
      stringstream sstm;
      sstm << "bad curvelink [" << ystart << "=>" << yend << "] for " << &curve;
      throw runtime_error(sstm.str());
    }
    ytop = min(ytop, ystart);
    ybot = max(ybot, yend);
    return true;
  }

  bool CurveLink::isEmpty() {
    return (ytop == ybot);
  }

  Curve *CurveLink::getCurve() {
    return curve;
  }

  Curve *CurveLink::getSubCurve() {
    if (ytop == curve->getYTop() && ybot == curve->getYBot()) {
      return curve->getWithDirection(etag);
    }
    return curve->getSubCurve(ytop, ybot, etag);
  }

  Curve *CurveLink::getMoveto() {
    return new MoveCurve(getXTop(), getYTop());
  }

  double CurveLink::getXTop() {
    return curve->XforY(ytop);
  }

  double CurveLink::getYTop() {
    return ytop;
  }

  double CurveLink::getXBot() {
    return curve->XforY(ybot);
  }

  double CurveLink::getYBot() {
    return ybot;
  }

  double CurveLink::getX() {
    return curve->XforY(ytop);
  }

  int CurveLink::getEdgeTag() {
    return etag;
  }

  void CurveLink::setNext(CurveLink *link) {
    this->next = link;
  }

  CurveLink *CurveLink::getNext() {
    return next;
  }

  ChainEnd::ChainEnd(CurveLink* first, ChainEnd* partner) :
        head(first), tail(first), partner(partner), etag(first->getEdgeTag()) {
  }

  CurveLink* ChainEnd::getChain() {
    return head;
  }

  void ChainEnd::setOtherEnd(ChainEnd* partner) {
    this->partner = partner;
  }

  ChainEnd* ChainEnd::getPartner() {
    return partner;
  }

  CurveLink* ChainEnd::linkTo(ChainEnd* that) {
    if (etag == AreaOp::ETAG_IGNORE ||
        that->etag == AreaOp::ETAG_IGNORE)
    {
      throw runtime_error("ChainEnd linked more than once!");
    }
    if (etag == that->etag) {
      throw runtime_error("Linking chains of the same type!");
    }
    ChainEnd* enter;
    ChainEnd* exit;
    if (etag == AreaOp::ETAG_ENTER) {
      enter = this;
      exit = that;
    } else {
      enter = that;
      exit = this;
    }
    // Now make sure these ChainEnds are not linked to any others...
    etag = AreaOp::ETAG_IGNORE;
    that->etag = AreaOp::ETAG_IGNORE;
    // Now link everything up...
    enter->tail->setNext(exit->head);
    enter->tail = exit->tail;
    if (partner == that) {
      // Curve has closed on itself...
      return enter->head;
    }
    // Link this chain into one end of the chain formed by the partners
    ChainEnd* otherenter = exit->partner;
    ChainEnd* otherexit = enter->partner;
    otherenter->partner = otherexit;
    otherexit->partner = otherenter;
    if (enter->head->getYTop() < otherenter->head->getYTop()) {
      enter->tail->setNext(otherenter->head);
      otherenter->head = enter->head;
    } else {
      otherexit->tail->setNext(enter->head);
      otherexit->tail = enter->tail;
    }
    return 0;
  }

  void ChainEnd::addLink(CurveLink* newlink) {
    if (etag == AreaOp::ETAG_ENTER) {
      tail->setNext(newlink);
      tail = newlink;
    } else {
      newlink->setNext(head);
      head = newlink;
    }
  }

  double ChainEnd::getX() {
    if (etag == AreaOp::ETAG_ENTER) {
      return tail->getXBot();
    } else {
      return head->getXBot();
    }
  }

  void AreaOp::newRow() {
    count = 0;
  }

  int AreaOp::classify(Edge* e) {
    int newCount = count;
    int type = (newCount == 0 ? ETAG_ENTER : ETAG_IGNORE);
    newCount += e->getCurve()->getDirection();
    count = newCount;
    return (newCount == 0 ? ETAG_EXIT : type);
  }

  int AreaOp::getState() {
    return ((count == 0) ? RSTAG_OUTSIDE : RSTAG_INSIDE);
  }

  void AreaOp::finalizeSubCurves(vector<CurveLink*>* subcurves,
                                 vector<ChainEnd*>* chains) {
    int numchains = chains->size();
    if (numchains == 0) {
      return;
    }
    if ((numchains & 1) != 0) {
      throw runtime_error("Odd number of chains!");
    }
    ChainEnd* endlist[numchains];
    copy(chains->begin(), chains->end(), endlist);
    for (int i = 1; i < numchains; i += 2) {
      ChainEnd* open = endlist[i - 1];
      ChainEnd* close = endlist[i];
      CurveLink* subcurve = open->linkTo(close);
      if (subcurve != NULL) {
        subcurves->push_back(subcurve);
      }
    }
    chains->clear();
  }

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
  bool AreaOp::obstructs(double v1, double v2, int phase) {
    return (((phase & 1) == 0) ? (v1 <= v2) : (v1 < v2));
  }

  void AreaOp::resolveLinks(vector<CurveLink*> *subcurves,
                           vector<ChainEnd*> *chains,
                           vector<CurveLink*> *links)
  {
    int numlinks = links->size();
    if (numlinks == 0 && (numlinks & 1) != 0) {
      throw runtime_error("Odd number of new curves!");
    }
    vector<CurveLink*> linklist(*links);
    int numchains = chains->size();
    if (numlinks == 0 && (numchains & 1) != 0) {
      throw runtime_error("Odd number of chains!");
    }
    vector<ChainEnd*> endlist(*chains);
    int curchain = 0;
    int curlink = 0;
    chains->clear();
    ChainEnd* chain = endlist.at(0);
    ChainEnd* nextchain = endlist.at(1);
    CurveLink* link = linklist.at(0);
    CurveLink* nextlink = linklist.at(1);
    while (chain != NULL || link != NULL) {
      /*
       * Strategy 1:
       * Connect chains or links if they are the only things left...
       */
      bool connectchains = (link == NULL);
      bool connectlinks = (chain == NULL);

      if (!connectchains && !connectlinks) {
        // assert(link != null && chain != null);
        /*
         * Strategy 2:
         * Connect chains or links if they close off an open area...
         */
        connectchains = ((curchain & 1) == 0 &&
                         chain->getX() == nextchain->getX());
        connectlinks = ((curlink & 1) == 0 &&
                        link->getX() == nextlink->getX());

        if (!connectchains && !connectlinks) {
          /*
           * Strategy 3:
           * Connect chains or links if their successor is
           * between them and their potential connectee...
           */
          double cx = chain->getX();
          double lx = link->getX();
          connectchains =
            (nextchain != NULL && cx < lx &&
             obstructs(nextchain->getX(), lx, curchain));
          connectlinks =
            (nextlink != NULL && lx < cx &&
             obstructs(nextlink->getX(), cx, curlink));
        }
      }
      if (connectchains) {
        CurveLink* subcurve = chain->linkTo(nextchain);
        if (subcurve != NULL) {
          subcurves->push_back(subcurve);
        }
        curchain += 2;
        chain = endlist.at(curchain);
        nextchain = endlist.at(curchain+1);
      }
      if (connectlinks) {
        ChainEnd* openend = new ChainEnd(link, 0);
        ChainEnd* closeend = new ChainEnd(nextlink, openend);
        openend->setOtherEnd(closeend);
        chains->push_back(openend);
        chains->push_back(closeend);
        curlink += 2;
        link = linklist.at(curlink);
        nextlink = linklist.at(curlink+1);
      }
      if (!connectchains && !connectlinks) {
        chain->addLink(link);
        chains->push_back(chain);
        curchain++;
        chain = nextchain;
        nextchain = endlist.at(curchain+1);
        curlink++;
        link = nextlink;
        nextlink = linklist.at(curlink+1);
      }
    }
    if ((chains->size() & 1) != 0) {
      cout << "Odd number of chains!" << endl;
    }
  }

  void AreaOp::calculate(vector<Curve*>* left,
                                    vector<Curve*>* right,
                                    vector<Curve*>* result) {
    vector<Edge*> edges;
    addEdges(&edges, left, AreaOp::CTAG_LEFT);
    addEdges(&edges, right, AreaOp::CTAG_RIGHT);
    pruneEdges(&edges, result);
  }

  void AreaOp::addEdges(vector<Edge*>* edges,
                       vector<Curve*>* curves, int curvetag) {
    for(unsigned int i = 0; i < curves->size(); i++) {
      Curve* c = curves->at(i);
      if(c->getOrder() > 0) {
        edges->push_back(new Edge(c, curvetag));
      }
    }
  }

  void AreaOp::pruneEdges(vector<Edge*>* edges, vector<Curve*>* result) {
    int numedges = edges->size();
    if (numedges < 2) {
      return;
    }
    Edge* edgelist[numedges];
    copy(edges->begin(), edges->end(), edgelist);
    sort(edgelist, edgelist + numedges);
    Edge* e;
    int left = 0;
    int right = 0;
    int cur = 0;
    int next = 0;
    double yrange[2];
    vector<CurveLink*> subcurves;
    vector<ChainEnd*> chains;
    vector<CurveLink*> links;
    // Active edges are between left (inclusive) and right (exclusive)
    while (left < numedges) {
      double y = yrange[0];
      // Prune active edges that fall off the top of the active y range
      for (cur = next = right - 1; cur >= left; cur--) {
        e = edgelist[cur];
        if (e->getCurve()->getYBot() > y) {
          if (next > cur) {
            edgelist[next] = e;
          }
          next--;
        }
      }
      left = next + 1;
      // Grab a new "top of Y range" if the active edges are empty
      if (left >= right) {
        if (right >= numedges) {
          break;
        }
        y = edgelist[right]->getCurve()->getYTop();
        if (y > yrange[0]) {
          finalizeSubCurves(&subcurves, &chains);
        }
        yrange[0] = y;
      }
      // Incorporate new active edges that enter the active y range
      while (right < numedges) {
        e = edgelist[right];
        if (e->getCurve()->getYTop() > y) {
          break;
        }
        right++;
      }
      // Sort the current active edges by their X values and
      // determine the maximum valid Y range where the X ordering
      // is correct
      yrange[1] = edgelist[left]->getCurve()->getYBot();
      if (right < numedges) {
        y = edgelist[right]->getCurve()->getYTop();
        if (yrange[1] > y) {
          yrange[1] = y;
        }
      }
      // Note: We could start at left+1, but we need to make
      // sure that edgelist[left] has its equivalence set to 0.
      int nexteq = 1;
      for (cur = left; cur < right; cur++) {
        e = edgelist[cur];
        e->setEquivalence(0);
        for (next = cur; next > left; next--) {
          Edge* prevedge = edgelist[next-1];
          int ordering = e->compareTo(prevedge, yrange);
          if (yrange[1] <= yrange[0]) {
            stringstream sstm;
            sstm << "backstepping to " << yrange[1] << " from " << yrange[0];
            throw runtime_error(sstm.str());
          }
          if (ordering >= 0) {
            if (ordering == 0) {
              // If the curves are equal, mark them to be
              // deleted later if they cancel each other
              // out so that we avoid having extraneous
              // curve segments.
              int eq = prevedge->getEquivalence();
              if (eq == 0) {
                eq = nexteq++;
                prevedge->setEquivalence(eq);
              }
              e->setEquivalence(eq);
            }
            break;
          }
          edgelist[next] = prevedge;
        }
        edgelist[next] = e;
      }
      // Now prune the active edge list.
      // For each edge in the list, determine its classification
      // (entering shape, exiting shape, ignore - no change) and
      // record the current Y range and its classification in the
      // Edge object for use later in constructing the new outline.
      newRow();
      double ystart = yrange[0];
      double yend = yrange[1];
      for (cur = left; cur < right; cur++) {
        e = edgelist[cur];
        int etag;
        int eq = e->getEquivalence();
        if (eq != 0) {
          // Find one of the segments in the "equal" range
          // with the right transition state and prefer an
          // edge that was either active up until ystart
          // or the edge that extends the furthest downward
          // (i.e. has the most potential for continuation)
          int origstate = getState();
          etag = (origstate == AreaOp::RSTAG_INSIDE
                  ? AreaOp::ETAG_EXIT
                  : AreaOp::ETAG_ENTER);
          Edge* activematch;
          Edge* longestmatch = e;
          double furthesty = yend;
          do {
            // Note: classify() must be called
            // on every edge we consume here.
            classify(e);
            if (activematch == NULL &&
                e->isActiveFor(ystart, etag))
            {
              activematch = e;
            }
            y = e->getCurve()->getYBot();
            if (y > furthesty) {
              longestmatch = e;
              furthesty = y;
            }
          } while (++cur < right &&
                   (e = edgelist[cur])->getEquivalence() == eq);
          --cur;
          if (getState() == origstate) {
            etag = AreaOp::ETAG_IGNORE;
          } else {
            e = (activematch != NULL ? activematch : longestmatch);
          }
        } else {
          etag = classify(e);
        }
        if (etag != AreaOp::ETAG_IGNORE) {
          e->record(yend, etag);
          links.push_back(new CurveLink(e->getCurve(), ystart, yend, etag));
        }
      }
      if (getState() != AreaOp::RSTAG_OUTSIDE) {
        cout << "Still inside at end of active edge list!" << endl;
        cout << "num curves = " << right-left << endl;
        cout << "num links = " << links.size() << endl;
        cout << "y top = " << yrange[0] << endl;
        if (right < numedges) {
          cout << "y top of next curve = " <<
               edgelist[right]->getCurve()->getYTop() << endl;
        } else {
          cout << "no more curves" << endl;
        }
        for (cur = left; cur < right; cur++) {
          e = edgelist[cur];
          cout << &e << endl;
          int eq = e->getEquivalence();
          if (eq != 0) {
            cout << "  was equal to " << eq << "..." << endl;
          }
        }
      }
      resolveLinks(&subcurves, &chains, &links);
      links.clear();
      // Finally capture the bottom of the valid Y range as the top
      // of the next Y range.
      yrange[0] = yend;
    }
    finalizeSubCurves(&subcurves, &chains);
    result->clear();
    for(unsigned int i = 0; i < subcurves.size(); i++) {
      CurveLink* link = subcurves.at(i);
      result->push_back(link->getMoveto());
      CurveLink* nextlink = link;
      while((nextlink = nextlink->getNext()) != NULL) {
        if (!link->absorb(nextlink)) {
          result->push_back(link->getSubCurve());
          link = nextlink;
        }
      }
      result->push_back(link->getSubCurve());
    }
  }

}
