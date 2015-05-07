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
 
 using namespace std;
 
 namespace stj {
   
 enum SetRel {STANDARD, DISJOINT, SUPERSET, EQUAL, INTERSECT};
 enum DataType {MLABEL, MLABELS, MPLACE, MPLACES};

 struct NFAtransition {
  int oldState;
  int trigger;
  int newState;
};

struct ivCmp {
  bool operator() (const Interval<CcReal>& iv1, const Interval<CcReal>& iv2)
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
  static void intersect(const vector<set<TupleId> >& tidsets, 
                        set<TupleId>& result);
  static void intersectPairs(vector<set<pair<TupleId, int> > >& posVec, 
                             set<pair<TupleId, int> >*& result);
  static void uniteLast(unsigned int size, vector<set<TupleId> >& tidsets);
  static void uniteLastPairs(unsigned int size, 
                             vector<set<pair<TupleId, int> > >& posVec);
  static void filterPairs(set<pair<TupleId, int> >* pairs,
                    const set<TupleId>& pos, set<pair<TupleId, int> >*& result);
  static string int2String(int i);
  static int str2Int(string const &text);
  static void deleteSpaces(string& text);
  static string setToString(const set<string>& input);
  static int prefixCount(string str, set<string> strings);
  static void splitPattern(string& input, vector<string>& result);
  static char* convert(string arg);
  static void eraseQM(string& arg); // QM = quotation marks
  static void addQM(string& arg);
  static void simplifyRegEx(string &regEx);
  static set<unsigned int>** createSetMatrix(unsigned int dim1, 
                                             unsigned int dim2);
  static void deleteSetMatrix(set<unsigned int>** &victim, unsigned int dim1);
  static int getKey(const string& type, Tuple *tuple = 0);
  static string getDataType(const int key);
  static DataType getDataType(const string& type);
  static DataType getDataType(TupleType *ttype, const int attrno);
  static bool isSymbolicType(ListExpr typeList);
  static string extractVar(const string& input);
  static string extendDate(string input, const bool start);
  static bool checkSemanticDate(const string &text, const SecInterval &uIv,
                                const bool resultNeeded);
  static bool checkDaytime(const string& text, const SecInterval& uIv);
  static bool isInterval(const string& str);
  static void stringToInterval(const string& str, SecInterval& result);
  static bool orderCheckInsert(Range<CcReal> *range,const Interval<CcReal> &iv);
  static bool parseInterval(const string& input, bool &isEmpty, int &pos,
                            int &endpos, Word &value);
  static bool isSetRel(const string& input, int &pos, int &endpos, 
                       SetRel &setrel);
  static bool parseBoolorObj(const string& input, bool &isEmpty, int &pos, 
                             int &endpos, Word &value);
  static bool checkAttrType(const string& typeName, const Word &value);
  static bool isRelevantAttr(const string& name);
  static vector<pair<int, string> > getRelevantAttrs(TupleType *ttype, 
                                      const int majorAttrNo, int& majorValueNo);
  static void deleteValue(Word &value, const string &type);
  static bool timesMatch(const Interval<DateTime>& iv, const set<string>& ivs);
  static pair<QueryProcessor*, OpTree> processQueryStr(string query, int type);
  // static Word evaluate(string input);
  static bool createTransitions(const bool dortmund,
                                map<string, set<string> >& transitions);
  static bool createLabelSequence(const int size, const int number,
                    const bool dortmund, map<string, set<string> >& transitions,
                    vector<string>& result);
  static void printNfa(vector<map<int, int> > &nfa, set<int> &finalStates);
  static void makeNFApersistent(vector<map<int, int> > &nfa,
     set<int> &finalStates, DbArray<NFAtransition> &trans, DbArray<int> &fs, 
     map<int, int> &final2Pat);
  static void createNFAfromPersistent(DbArray<NFAtransition> &trans, 
          DbArray<int> &fs, vector<map<int, int> > &nfa, set<int> &finalStates);
  static void printBinding(const map<string, pair<int, int> > &b);
  static double distance(const string& str1, const string& str2, const int fun);
  static double distance(const pair<string, unsigned int>& val1, 
                       const pair<string, unsigned int>& val2, const int fun);
  static double distance(const set<string>& values1, const set<string>& values2,
                         const int fun, const int labelFun);
  static double distance(set<pair<string, unsigned int> >& values1, 
                         set<pair<string, unsigned int> >& values2,
                         const int fun, const int labelFun);
  
  template<class T>
  static bool relationHolds(const set<T>& s1, const set<T>& s2,
                            const SetRel rel) {
    set<T> temp;
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
  static bool relationHolds(const Range<T>& r1, const Range<T>& r2, 
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
  
  static bool relationHolds(const Range<CcReal>& range, const set<int>& intSet,
                            const SetRel rel) {
    set<int>::iterator it;
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
  
  static bool relationHolds(const MPoint &mp, const Region &reg, 
                            const SetRel rel) {
    switch (rel) {
      case STANDARD: {
        if (reg.Intersects(mp.BoundingBoxSpatial())) {
          MPoint mpAtReg(true);
          mp.AtRegion(&reg, mpAtReg);
          return !mpAtReg.IsEmpty();
        }
        return false;
      }
      case DISJOINT: {
        if (reg.Intersects(mp.BoundingBoxSpatial())) {
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
        if (reg.Intersects(mp.BoundingBoxSpatial())) {
          return mp.Passes(reg);
        }
        return false;
      }
      default: { // cannot occur
        return false;
      }
    }
  }
  
    static bool relationHolds(const MRegion &mreg, const Region &reg, 
                              const SetRel rel) {
    // TODO: a lot. use MRegion2 ?
    Rectangle<3> bb3 = mreg.BoundingBox();
    Rectangle<2> bbox(true, bb3.MinD(0), bb3.MaxD(0), bb3.MinD(1), bb3.MaxD(1));
    URegionEmb ur(true);
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
  
  static bool relationHolds(const set<bool> &boolSet, const bool b, 
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


