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

Started July 2014, Fabio Vald\'{e}s

*/


#ifndef SYMB_TOOLS_H
#define SYMB_TOOLS_H


#include "RelationAlgebra.h"
#include "TemporalUnitAlgebra.h"
#include "MovingRegionAlgebra.h"
#include "SecParser.h"
#include "RTreeAlgebra.h"
#include "FTextAlgebra.h"
#include "BasicTypes.h"
#include "InvertedFile.h"
#include "LongInt.h"
#include "Geoid.h"
 
 
 namespace stj {
   
 enum SetRel {STANDARD, DISJOINT, SUPERSET, EQUAL, INTERSECT};
 enum DataType {MLABEL, MLABELS, MPLACE, MPLACES};
 
 struct NFAtransition {
  int oldState;
  int trigger;
  int newState;
};

struct ivCmp {
  bool operator() (const temporalalgebra::Interval<CcReal>& iv1, 
                   const temporalalgebra::Interval<CcReal>& iv2)
                   const {
    if (iv1.start == iv2.start) {
      if (iv1.lc == iv2.lc) {
        if (iv1.end == iv2.end) {
          return iv1.rc > iv2.rc;
        }
        return iv1.end.GetValue() < iv2.end.GetValue();
      }
      return iv1.lc > iv2.lc;
    }
    return iv1.start.GetValue() < iv2.start.GetValue();
  }
};
 
class Tools {
 public:
  static void intersect(const std::vector<std::set<TupleId> >& tidsets, 
                        std::set<TupleId>& result);
  static void intersectPairs(
               std::vector<std::set<std::pair<TupleId, int> > >& posVec, 
               std::set<std::pair<TupleId, int> >*& result);
  static void uniteLast(unsigned int size, 
                        std::vector<std::set<TupleId> >& tidsets);
  static void uniteLastPairs(unsigned int size, 
              std::vector<std::set<std::pair<TupleId, int> > >& posVec);
  static void filterPairs(std::set<std::pair<TupleId, int> >* pairs,
                    const std::set<TupleId>& pos, 
                    std::set<std::pair<TupleId, int> >*& result);
  static std::string int2String(int i);
  static int str2Int(std::string const &text);
  static void deleteSpaces(std::string& text);
  static std::string setToString(const std::set<std::string>& input);
  static int prefixCount(std::string str, std::set<std::string> strings);
  static void splitPattern(std::string& input, 
                           std::vector<std::string>& result);
  static char* convert(std::string arg);
  static void eraseQM(std::string& arg); // QM = quotation marks
  static void addQM(std::string& arg);
  static void simplifyRegEx(std::string &regEx);
  static std::set<unsigned int>** createSetMatrix(unsigned int dim1, 
                                             unsigned int dim2);
  static void deleteSetMatrix(std::set<unsigned int>** &victim, 
                              unsigned int dim1);
  static int getKey(const std::string& type, Tuple *tuple = 0, 
                    ListExpr tupleType = 0);
  static std::string getDataType(const int key);
  static DataType getDataType(const std::string& type);
  static DataType getDataType(TupleType *ttype, const int attrno);
  static std::string getTypeName(TupleType *ttype, const int attrno);
  static int getNoComponents(Relation *rel, const TupleId tid, 
                             const std::string &type, const int attrno);
  static bool isSymbolicType(ListExpr typeList);
  static std::string extractVar(const std::string& input);
  static std::string extendDate(std::string input, const bool start);
  static bool checkSemanticDate(const std::string &text, 
                                const temporalalgebra::SecInterval &uIv,
                                const bool resultNeeded);
  static bool checkDaytime(const std::string& text, 
                           const temporalalgebra::SecInterval& uIv);
  static bool isInterval(const std::string& str);
  static void stringToInterval(const std::string& str, 
                               temporalalgebra::SecInterval& result);
  static bool orderCheckInsert(temporalalgebra::Range<CcReal> *range,
                               const temporalalgebra::Interval<CcReal> &iv);
  static bool parseInterval(const std::string& input, bool &isEmpty, int &pos,
                            int &endpos, Word &value);
  static bool isSetRel(const std::string& input, int &pos, int &endpos, 
                       SetRel &setrel);
  static bool parseBoolorObj(const std::string& input, bool &isEmpty, int &pos, 
                             int &endpos, Word &value);
  static bool checkAttrType(const std::string& typeName, const Word &value);
  static bool isRelevantAttr(const std::string& name);
  static bool isMovingAttr(const ListExpr ttype, const int attrno);
  static std::vector<std::pair<int, std::string> > getRelevantAttrs(
                       TupleType *ttype, 
                       const int majorAttrNo, int& majorValueNo);
  static void deleteValue(Word &value, const std::string &type);
  static void queryTrie(InvertedFile *inv, std::string str, 
                        std::vector<std::set<int> > &result);
  static void queryTrie(InvertedFile *inv, std::pair<std::string, 
                        unsigned int> place, 
                        std::vector<std::set<int> > &result);
  static void queryBtree(BTree_t<LongInt> *btree, 
                         temporalalgebra::Interval<CcReal> &iv,
                         std::vector<std::set<int> > &result);
  static void queryRtree1(RTree1TLLI *rtree, 
                          temporalalgebra::Interval<CcReal> &iv,
                          std::vector<std::set<int> > &result);
  static void queryRtree2(RTree2TLLI *rtree, Rectangle<2> &box,
                          std::vector<std::set<int> > &result);
  static bool timesMatch(
                   const temporalalgebra::Interval<datetime::DateTime>& iv, 
                   const std::set<std::string>& ivs);
  static std::pair<QueryProcessor*, OpTree> processQueryStr(
                           std::string query, int type);
  // static Word evaluate(string input);
  static bool createTransitions(const bool dortmund,
                                std::map<std::string, 
                                std::set<std::string> >& transitions);
  static bool createLabelSequence(const int size, const int number,
          const bool dortmund, std::map<std::string, 
          std::set<std::string> >& transitions,
          std::vector<std::string>& result);
  static void printNfa(std::vector<std::map<int, int> > &nfa, 
                       std::set<int> &finalStates);
  static void makeNFApersistent(std::vector<std::map<int, int> > &nfa,
     std::set<int> &finalStates, DbArray<NFAtransition> &trans, 
     DbArray<int> &fs, 
     std::map<int, int> &final2Pat);
  static void createNFAfromPersistent(DbArray<NFAtransition> &trans, 
          DbArray<int> &fs, std::vector<std::map<int, int> > &nfa, 
          std::set<int> &finalStates);
  static double distance(const std::string& str1, const std::string& str2, 
                         const int fun);
  static double distance(const std::pair<std::string, unsigned int>& val1, 
                       const std::pair<std::string, unsigned int>& val2, 
                       const int fun);
  static double distance(const std::set<std::string>& values1, 
                         const std::set<std::string>& values2,
                         const int fun, const int labelFun);
  static double distance(std::set<std::pair<std::string, 
                         unsigned int> >& values1, 
                         std::set<std::pair<std::string, 
                         unsigned int> >& values2,
                         const int fun, const int labelFun);
  
  template<class T>
  static bool relationHolds(const std::set<T>& s1, const std::set<T>& s2,
                            const SetRel rel) {
    std::set<T> temp;
    switch (rel) {
      case STANDARD: {
        set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(), 
                      std::inserter(temp, temp.begin()));
        return temp.empty();
      }
      case DISJOINT: {
        set_union(s1.begin(), s1.end(), s2.begin(), s2.end(),
                  std::inserter(temp, temp.begin()));
        return (temp.size() == s1.size() + s2.size());
      }
      case SUPERSET: {
        set_difference(s2.begin(), s2.end(), s1.begin(), s1.end(), 
                      std::inserter(temp, temp.begin()));
        return temp.empty();
      }
      case EQUAL: {
        return s1 == s2;
      }
      case INTERSECT: {
        set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), 
                  std::inserter(temp, temp.begin()));
        return (temp.size() < s1.size() + s2.size());
      }
      default: // cannot occur
        return false;
    }
  }
  
  template<class T>
  static bool relationHolds(const temporalalgebra::Range<T>& r1, 
                            const temporalalgebra::Range<T>& r2, 
                            const SetRel rel) {
    switch (rel) {
      case STANDARD: {
        return r1.Inside(r2);
      }
      case DISJOINT: {
        return !r1.Intersects(r2);
      }
      case SUPERSET: {
        return r2.Inside(r1);
      }
      case EQUAL: {
        return r1 == r2;
      }
      case INTERSECT: {
        return r1.Intersects(r2);
      }
      default: { // cannot occur
        return false;
      }
    }
  }
  
  static bool relationHolds(const temporalalgebra::Range<CcReal>& range, 
                           const std::set<int>& intSet,
                            const SetRel rel) {
    std::set<int>::iterator it;
    switch (rel) {
      case STANDARD: {
        for (it = intSet.begin(); it != intSet.end(); it++) {
          CcReal ccreal((double)*it);
          if (!range.Contains(ccreal)) {
            return false;
          }
        }
        return true;
      }
      case DISJOINT: {
        for (it = intSet.begin(); it != intSet.end(); it++) {
          CcReal ccreal((double)*it);
          if (range.Contains(ccreal)) {
            return false;
          }
        }
        return true;
      }
      case SUPERSET: {
        return false; // meaningless
      }
      case EQUAL: {
        return false; // meaningless
      }
      case INTERSECT: {
        for (it = intSet.begin(); it != intSet.end(); it++) {
          CcReal ccreal((double)*it);
          if (range.Contains(ccreal)) {
            return true;
          }
        }
        return false;
      }
      default: { // cannot occur
        return false;
      }
    }
  }
  
  static bool relationHolds(const temporalalgebra::MPoint &mp, 
                            const Region &reg, 
                            const SetRel rel) {
    switch (rel) {
      case STANDARD: {
        if (reg.Distance(mp.BoundingBoxSpatial()) == 0.0) {
          temporalalgebra::MPoint mpAtReg(true);
          mp.AtRegion(&reg, mpAtReg);
          return !mpAtReg.IsEmpty();
        }
        return false;
      }
      case DISJOINT: {
        if (reg.Distance(mp.BoundingBoxSpatial()) == 0.0) {
          return !mp.Passes(reg);
        }
        return true;
      }
      case SUPERSET: { // meaningless
        return false;
      }
      case EQUAL: { // meaningless
        return false;
      }
      case INTERSECT: {
        if (reg.Distance(mp.BoundingBoxSpatial()) == 0.0) {
          return mp.Passes(reg);
        }
        return false;
      }
      default: { // cannot occur
        return false;
      }
    }
  }
  
    static bool relationHolds(const temporalalgebra::MRegion &mreg, 
                             const Region &reg, 
                              const SetRel rel) {
    // TODO: a lot. use MRegion2 ?
    Rectangle<3> bb3 = mreg.BoundingBox();
    Rectangle<2> bbox(true, bb3.MinD(0), bb3.MaxD(0), bb3.MinD(1), bb3.MaxD(1));
    temporalalgebra::URegionEmb ur(true);
    switch (rel) {
      case STANDARD: {
        if (reg.Intersects(bbox)) {
          for (int i = 1; i < mreg.GetNoComponents(); i++) {
            mreg.Get(i, ur);
          }
        }
        return false;
      }
      case DISJOINT: {
        return false;
      }
      case SUPERSET: {
        return false;
      }
      case EQUAL: {
        return false;
      }
      case INTERSECT: {
        return false;
      }
      default: { // cannot occur
        return false;
      }
    }
  }
  
  static bool relationHolds(const std::set<bool> &boolSet, const bool b, 
                            const SetRel rel) {
    switch (rel) {
      case STANDARD: {
        return (boolSet.find(b) != boolSet.end()) && (boolSet.size() == 1);
      }
      case DISJOINT: {
        return boolSet.find(b) == boolSet.end();
      }
      case SUPERSET: {
        return boolSet.find(b) != boolSet.end();
      }
      case EQUAL: {
        return (boolSet.find(b) != boolSet.end()) && (boolSet.size() == 1);
      }
      case INTERSECT: {
        return boolSet.find(b) != boolSet.end();
      }
      default: { // cannot occur
        return false;
      }
    }
  }
};

 }

#endif


