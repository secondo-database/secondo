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

#include "RelationAlgebra.h"
#include "TemporalUnitAlgebra.h"
#include "SecParser.h"
 
 using namespace std;
 
 namespace stj {
   
 enum SetRel {STANDARD, DISJOINT, SUPERSET, EQUAL, INTERSECT};
 enum DataType {LABEL, LABELS, PLACE, PLACES};
 
 struct NFAtransition {
  int oldState;
  int trigger;
  int newState;
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
  template<class T>
  static bool relationHolds(const set<T>& set1, const set<T>& set2, SetRel rel);
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
  static int getKey(const string& type);
  static string getDataType(const int key);
  static DataType getDataType(const string& type);
  static bool isSymbolicType(ListExpr typeList);
  static string extractVar(const string& input);
  static string extendDate(string input, const bool start);
  static bool checkSemanticDate(const string &text, const SecInterval &uIv,
                                const bool resultNeeded);
  static bool checkDaytime(const string& text, const SecInterval& uIv);
  static bool isInterval(const string& str);
  static void stringToInterval(const string& str, SecInterval& result);
  static bool timesMatch(const Interval<DateTime>& iv, const set<string>& ivs);
  static pair<QueryProcessor*, OpTree> processQueryStr(string query, int type);
  // static Word evaluate(string input);
  static void createTrajectory(int size, vector<string>& result);
  static void printNfa(vector<map<int, int> > &nfa, set<int> &finalStates);
  static void makeNFApersistent(vector<map<int, int> > &nfa,
     set<int> &finalStates, DbArray<NFAtransition> &trans, DbArray<int> &fs, 
     map<int, int> &final2Pat);
  static void createNFAfromPersistent(DbArray<NFAtransition> &trans, 
          DbArray<int> &fs, vector<map<int, int> > &nfa, set<int> &finalStates);
  static void printBinding(map<string, pair<unsigned int, unsigned int> > &b);
  static double distance(string& val1, string& val2);
  static double distance(pair<string, unsigned int>& val1, 
                         pair<string, unsigned int>& val2);
  static double distance(set<string>& values1, set<string>& values2);
  static double distance(set<pair<string, unsigned int> >& values1, 
                         set<pair<string, unsigned int> >& values2);
};

 }