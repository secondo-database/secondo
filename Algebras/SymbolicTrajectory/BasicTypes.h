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

#include "TemporalUnitAlgebra.h"
#include "StandardTypes.h"
#include "TemporalAlgebra.h"
#include "TemporalExtAlgebra.h"
#include "NestedList.h"
#include "ListUtils.h"

namespace stj {
  
template<class F, class S>
class NewPair {
 public:
  NewPair() {}
  NewPair(F f, S s) : first(f), second(s) {}
   
  F first;
  S second;
};
  
struct SymbolicUnit {
 public: 
  SymbolicUnit() {}
  SymbolicUnit(unsigned int p) : iv(true), pos(p) {}
  
  SecInterval iv;
  unsigned int pos;
};

struct ExtSymbolicUnit : public SymbolicUnit {
  ExtSymbolicUnit() {}
  ExtSymbolicUnit(unsigned int p) : SymbolicUnit(p), ref(0) {}
  
  unsigned int ref;
};

class Labels;

/*
\section{Class ~Label~}

*/
class Label : public Attribute {
 public:
  friend class Labels;
  
  typedef SymbolicUnit unitelem;
  typedef string base;
  typedef Labels coll;
   
  Label() {}
  Label(const string& text) : Attribute(true), value(0) {SetValue(text);}
  Label(const Label& rhs);
  explicit Label(const bool def) : Attribute(def), value(0) {}
  
  ~Label() {}
  
  void Clean() {value.clean();}
  void Destroy() {value.destroy();}
  void GetValue(string &text) const;
  static void buildValue(const string& text, const unitelem& unit,base& result);
  static ListExpr getList(const string& text) {return nl->TextAtom(text);}
  void Set(const bool def, const string &text) {SetDefined(def);SetValue(text);}
  void SetValue(const string &text);
  Label& operator=(const Label& lb) {CopyFrom(&lb); return *this;}
  bool operator==(const Label& lb) const;
  bool operator==(const string& text) const;

  static bool readValueFrom(ListExpr LE, string& text, unitelem& unit);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  ListExpr ToListExpr(ListExpr typeInfo);
  static bool     CheckKind(ListExpr type, ListExpr& errorInfo);
  static int      SizeOfObj() {return sizeof(Label);}
  static ListExpr Property();
  static const string BasicType() {return "label";}
  static const bool checkType(const ListExpr type);
  void            CopyFrom(const Attribute* right);
  int NumOfFLOBs() const {return 1;}
  Flob *GetFLOB(const int i) {assert(i == 0); return &value;}
  int             Compare(const Attribute* arg) const;
  size_t          Sizeof() const {return sizeof(*this);}
  bool            Adjacent(const Attribute*) const {return false;}
  Attribute*      Clone() const {return new Label(*this);}
  size_t          HashValue() const {return value.getSize();}
  ostream&        Print(ostream& os) const;
  
 protected:
  Flob value;
};

/*
\section{Class ~Labels~}

*/
class Labels : public Attribute {
 public:
  typedef SymbolicUnit unitelem;
  typedef unsigned int arrayelem;
  typedef string base;
  typedef Label single;
      
  Labels() {}
  explicit Labels(const bool defined) : Attribute(defined), values(8), pos(1) {}
  Labels(const Labels& src, const bool sort = false);
  
  ~Labels() {}

  Labels& operator=(const Labels& src);
  bool operator==(const Labels& src) const;
  void Append(const Label &lb);
  void Append(const string& text);
  void Append(const set<string>& values);
  void Destroy() {values.destroy(); pos.destroy();}
  int GetNoValues() const {return pos.Size();}
  size_t GetLength() const {return values.getSize();}
  void Get(int i, Label& lb) const;
  void GetValue(int i, string& text) const;
  void GetValues(set<string>& values) const;
  static void getRefToLastElem(const int size, unsigned int& result);
  static unsigned int getFlobPos(const arrayelem elem);
  static void valuesToListExpr(const set<string>& values, ListExpr& result);
  static void getString(const ListExpr& list, string& result);
  static void getElemFromList(const ListExpr& list, const unsigned int size, 
                              unsigned int& result);
  static void buildValue(const string& text, const unsigned int pos,
                         string& result);
  static void printArrayElem(const arrayelem e) {cout << "print " << e << endl;}
  const bool IsEmpty() const {return GetNoValues() == 0;}
  void Clean() {values.clean(); pos.clean();}
  bool Contains(const string& text) const;
  friend ostream& operator<<(ostream& os, const Labels& lbs);
  
  int NumOfFLOBs() const {return 2;}
  Flob *GetFLOB(const int i);
  int Compare(const Attribute*) const;
  bool Adjacent(const Attribute*) const {return false;}
  Attribute *Clone() const {return new Labels(*this);}
  size_t Sizeof() const {return sizeof(*this);}
  ostream& Print(ostream& os) const {return (os << *this);}
  static ListExpr Property();
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  static int SizeOfObj() {return sizeof(Labels);}
  static const string BasicType() {return Label::BasicType() + "s";}
  void CopyFrom(const Attribute* right) {*this = *((Labels*)right);}
  size_t HashValue() const;

 protected:
  Flob values;
  DbArray<unsigned int> pos;
};

class Places;

/*
\section{Class ~Place~}

*/
class Place : public Label {
 public: 
  friend class Places;
  
  typedef ExtSymbolicUnit unitelem;
  typedef pair<string, unsigned int> base;
  typedef Places coll;
  
  Place() : Label() {}
  Place(const pair<string, unsigned int>& v) : Label(v.first), ref(v.second) {}
  Place(const Place& rhs) : Label(rhs.IsDefined()) {CopyFrom(&rhs);}
  explicit Place(const bool def) : Label(def), ref(0) {}

  ~Place() {}

  void Set(const bool defined, const pair<string, unsigned int>& value);
  void SetValue(const pair<string, unsigned int>& value);
  void SetRef(const unsigned int value) {ref = value;}
  void GetValue(pair<string, unsigned int>& value) const;
  static void buildValue(const string& text, const unitelem& unit,base& result);
  static ListExpr getList(const base& value);
  unsigned int GetRef() const {return ref;}
  Place& operator=(const Place& p);
  bool operator==(const Place& p) const;
  bool operator==(const pair<string, unsigned int>& value) const;

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(Place);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return "place";}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return Label::NumOfFLOBs();}
  Flob* GetFLOB(const int i) {return Label::GetFLOB(i);}
  size_t Sizeof() const {return sizeof(*this);}
  int Compare(const Attribute* arg) const;
  bool Adjacent(const Attribute *arg) const {return false;}
  Attribute *Clone() const {return new Place(*this);}
  size_t HashValue() const {return Label::HashValue() * ref;}
  virtual void CopyFrom(const Attribute* right) {*this = *((Place*)right);}
  ListExpr ToListExpr(ListExpr typeInfo);
  static bool readValueFrom(ListExpr LE, string& text, unitelem& unit);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);

 protected:
  unsigned int ref;
};

/*
\section{Class ~Places~}

*/
class Places : public Attribute {
 public:
  typedef ExtSymbolicUnit unitelem;
  typedef NewPair<unsigned int, unsigned int> arrayelem;
  typedef pair<string, unsigned int> base;
  typedef Place single;
   
  Places() {}
  Places(const int n) : Attribute(n > 0), values(8), posref(n) {}
  Places(const Places& rhs, const bool sort = false);
  explicit Places(const bool def) : Attribute(def), values(8), posref(1) {}

  ~Places() {}

  void Append(const base& value);
  void Append(const Place& pl);
  void Append(const set<base>& values);
  void Destroy() {values.destroy(); posref.destroy();}
  int GetNoValues() const {return posref.Size();}
  size_t GetLength() const {return values.getSize();}
  void Get(const int i, Place& pl) const;
  void GetValue(int i, base& val) const;
  void GetValues(set<base>& values) const;
  bool IsEmpty() const {return (GetNoValues() == 0);}
  bool Contains(const base& val) const;
  void Clean() {values.clean(); posref.clean();}
  static void getRefToLastElem(const int size, arrayelem& result);
  static unsigned int getFlobPos(const arrayelem elem);
  static void valuesToListExpr(const set<base>& values, ListExpr& result);
  static void getString(const ListExpr& list, string& result);
  static void getElemFromList(const ListExpr& list, const unsigned int size, 
                              arrayelem& result);
  static void buildValue(const string& text, const arrayelem pos, base& result);
  static void printArrayElem(const arrayelem e) {cout << e.first << " " 
                                                      << e.second;}
  void operator=(const Places& p);
  bool operator==(const Places& p) const;

  static ListExpr Property();
  static int SizeOfObj() {return sizeof(Places);}
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  static const string BasicType() {return Place::BasicType() + "s";}
  static bool checkType(ListExpr t) {return listutils::isSymbol(t,BasicType());}
  int NumOfFLOBs() const {return 2;}
  Flob* GetFLOB(const int i);
  size_t Sizeof() const {return sizeof(*this);}
  int Compare(const Attribute* arg) const;
  bool Adjacent(const Attribute *arg) const {return false;}
  Attribute *Clone() const {return new Places(*this);}
  size_t HashValue() const;
  virtual void CopyFrom(const Attribute* right) {*this = *((Places*)right);}
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr LE, ListExpr typeInfo);
  
 protected:
  Flob values;
  DbArray<NewPair<unsigned int, unsigned int> > posref;
};

}