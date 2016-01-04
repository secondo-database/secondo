/*
----
This file is part of SECONDO.

Copyright (C) 2015, University in Hagen, Department of Computer Science,
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
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]

[1] Implementation of the Point Sequence Classes

\tableofcontents

\noindent

1 Introduction

This file implements and registers the type constructors ~pointseq~ and
~tpointseq~ and basic operators for these type constructors.


1.1 Type Contructors

1.1.1 ~pointseq~

A ~pointseq~ is either ~undefined~ or it is a sequence of zero or more
2-dimensional points $(x,\ y)$ with finite, real-valued coordinates $x$ and $y$.

1.1.2 ~tpointseq~

A ~tpointseq~ is either ~undefined~ or it is a sequence of zero or more
tuples $(inst,\ (x,\ y))$, each with a defined instant $inst$ and a
2-dimensional point $(x,\ y)$ with finite, real-valued coordinates $x$ and $y$.


1.2 Operators

1.2.1 ~isempty~

The operator $isempty : (pointseq|tpointseq) \rightarrow bool$ yields ~TRUE~ if
the sequence is ~undefined~ or contains no items and ~FALSE~ otherweise.

1.2.2 ~no\_components~

The operator $no\_components : (pointseq|tpointseq) \rightarrow int$ determines
the number of items in the given sequence, if it is defined, and 0 otherwise.

1.2.3 ~to\_pointseq~

The operator ~to\_pointseq~ extracts a ~pointseq~ from some input object.

  * The overload $to\_tpointseq : mpoint \rightarrow pointseq$ is equivalent to
    $to\_pointseq(to\_tpointseq(m))$ (see below) for an ~mpoint m~. It extracts
    a ~pointseq~ from a moving point object as

    * the start point of the first unit of the moving point object followed by

    * the end point of each unit of the moving point object whose interval
      contains not just a single instant.

    If there is a temporal or spatial gap between two successive units or a unit
    makes a spatial move in no time, the ~pointseq~ is ~undefined~.

  * The overload $to\_pointseq : tpointseq \rightarrow pointseq$ computes the
    spatial projection of the ~tpointseq~.

1.2.3 ~to\_tpointseq~

The operator $to\_tpointseq : mpoint \rightarrow tpointseq$ extracts a
~tpointseq~ from a moving point object as

  * the start instant and point of the first unit of the moving point object
    followed by

  * the end instant and point of each unit of the moving point object whose
    interval contains not just a single instant.

If there is a temporal or spatial gap between two successive units or a unit
makes a spatial move in no time, the ~tpointseq~ is ~undefined~.


2 Includes

*/

#include "PointSeq.h"

#include "TrajectorySimilarity.h"

#include "DLine.h"
#include "Point.h"
#include "Stream.h"
#include "TemporalAlgebra.h"   // MPoint

#include "ConstructorTemplates.h"
#include "ListUtils.h"
#include "NList.h"
#include "TypeConstructor.h"
#include "TypeMapUtils.h"

#include <climits>


namespace tsa {

/*
3 Implementation of Class Template ~Sequence$<$T$>$~

Constructor to create a sequence from an ~NList~.

*/

template<class T>
Sequence<T>::Sequence(const NList& list)
  : Attribute(/*defined*/ false), seq(0)
{
/*
The sequence may be ~undefined~. The base class ~Attribute~ and the
~DbArray$<$T$>$~ ~seq~ have been initialized accordingly.

*/
  if (listutils::isSymbolUndefined(list.listExpr()))
    return;

  if (list.isAtom())
    throw std::domain_error("List is an atom.");

  SetDefined(true);
  seq.resize(list.length());

/*
Iterate the list elements.

*/
  T prec_item;
  for (size_t i = 1; i <= list.length(); ++i) {
    const NList& elem = list.elem(i);
    try {
/*
Try to create an item of type ~T~ from the element. If this fails, a
~std::domain\_error~ exception is caught.

*/
      const T item(elem);
/*
If this is not the first item in the list, verify that it is valid as a
successor of the preceding item. If not, a ~std::domain\_error~ exception is
caught.

*/
      if (i > 1)
        item.validateSequenceOrder(prec_item);
/*
Append the new item to the sequence and remember it as preceding item for the
next iteration. No exceptions happen here.

*/
      append(item);
      prec_item = item;
    }
    catch (const std::domain_error& ex) {
      std::ostringstream os;
      os << "Processing list element " << i << " failed: " << ex.what();
      throw std::domain_error(os.str());
    }
  }
}


/*
Copy constructor.

*/
template<class T>
Sequence<T>::Sequence(const Sequence<T>& rhs)
  : Attribute(rhs.IsDefined()),
    seq(rhs.size())
{
  seq.copyFrom(rhs.seq);
}


/*
Helper for ~Sequence$<$T$>$::toDLine(DLine\&)~.

*/
template<class T>
static inline SimpleSegment makeSimpleSegment(const T& start, const T end)
{
  return SimpleSegment(start.getX(), start.getY(), end.getX(), end.getY());
}

/*
Create a ~DLine~ from the sequence.

*/
template<class T>
void Sequence<T>::toDLine(DLine& dline) const
{
  if(!IsDefined()){
    dline.SetDefined(false);
    return;
  }

  dline.clear();

/*
If the sequence is empty, create an empty ~DLine~.

*/
  if (size() == 0)
    return;

/*
If the sequence has only one item, create a ~DLine~ with a single segment whose
start and end point are equal.

*/
  if (size() == 1) {
    const T& item = get(0);
    const SimpleSegment seg = makeSimpleSegment(item, item);
    dline.append(seg);
    return;
  }

/*
Otherwise iterate the pairs of successive items and create a segment for each of
them.

*/
  T prev_item = get(0);
  for(size_t i = 1; i < size(); ++i) {
    const T& item = get(i);
    const SimpleSegment seg = makeSimpleSegment(prev_item, item);
    dline.append(seg);
    prev_item = item;
  }
}


/*
Recreate the sequence from an ~MPoint~.

*/
template<class T>
void Sequence<T>::convertFrom(const MPoint& src) noexcept
{
  SetDefined(false);
  seq.clean();

  if (!src.IsDefined())
    return;

  const size_t size = src.GetNoComponents();
  seq.resize(size + 1);

/*
Iterare UPoints of MPoint.

*/
  temporalalgebra::Interval<Instant> prec_ival =
      temporalalgebra::Interval<Instant>(/*dummy*/ false);
  ::Point prec_p1(/*defined*/ false);

  for (size_t i = 0; i < size; ++i) {
    temporalalgebra::UPoint upoint;
    src.Get(i, upoint);
    const temporalalgebra::Interval<Instant>& ival = upoint.getTimeInterval();
    const ::Point& p0 = upoint.p0;
    const ::Point& p1 = upoint.p1;

    if (i == 0) {
/*
Copy the start point of the first upoint.

*/
      append(T(ival.start, Point(p0)));
    }
    else {
/*
Bail out if there is a temporal or spatial gap between upoints.

*/
      if (!prec_ival.R_Adjacent(ival) || !AlmostEqual(prec_p1, p0)) {
        seq.clean();
        return;
      }
    }

    if (ival.start == ival.end) {
/*
The upoint contains just a single instant. Bail out if the upoint makes a
spatial move in no time. Otherwise silently ignore the end point.

*/
      if (!AlmostEqual(p0, p1)) {
        seq.clean();
        return;
      }
    }
    else {
/*
The upoint's interval is not just a single instant. Therefore also copy its end
point to the sequence.

*/
      append(T(ival.end, Point(p1)));
    }

    prec_ival = ival;
    prec_p1 = p1;
  }

  SetDefined(true);
}


/*
Get the item at position ~pos~.

*/
template<class T>
T Sequence<T>::get(const size_t pos) const
{
  assert(static_cast<int>(pos) < seq.Size());
  T item;
  const bool success = seq.Get(pos, item);
  assert(success);
  return item;
}


/*
Set the item at position ~pos~.

*/
template<class T>
void Sequence<T>::set(const size_t pos, const T& item)
{
  assert(static_cast<int>(pos) <= seq.Size());
  const bool success = seq.Put(pos, item);
  assert(success);
}


/*
Append the item to the end of the sequence.

*/
template<class T>
void Sequence<T>::append(const T& item)
{
  const bool success = seq.Append(item);
  assert(success);
}


/*
Create a list expression from a sequence.

*/
template<class T>
ListExpr Sequence<T>::Out(ListExpr typeInfo, Word value)
{
  Sequence<T>& ps = *static_cast<Sequence<T>*>(value.addr);
  if (!ps.IsDefined())
    return nl->SymbolAtom(Symbol::UNDEFINED());

  NList list;
  for (size_t i = 0; i < ps.size(); ++i) {
    const T& item = ps.get(i);
    list.append(item.toNList());
  }

  return list.listExpr();
}


/*
Create a sequence from a list expression.

*/
template<class T>
Word Sequence<T>::In(
    const ListExpr typeInfo, const ListExpr instance,
    const int errorPos, ListExpr& errorInfo, bool& correct)
{
  const NList list(instance);
  Sequence<T>* ps = nullptr;

  try {
/*
Let the constructor ~Sequence(const NList\& list)~ do the actual work. If it
fails, a ~std::domain\_error~ exception is caught.

*/
    ps = new Sequence<T>(list);
  }
  catch (const std::domain_error& ex) {
    correct = false;
    cmsg.error() << ex.what() << std::endl;
    cmsg.send();
    return SetWord(Address(0));
  }

  correct = true;
  return SetWord(ps);
}


/*
Compare two sequences as specified by ~Attribute::Compare()~.

*/
template<class T>
int Sequence<T>::Compare(const Attribute* rhs) const
{
  const Sequence<T>& rps = *static_cast<const Sequence<T>*>(rhs);

/*
Handle undefined sequences. An undefined sequence is considered ~less than~ a
defined sequence. Two undefined sequences are considered equal.

*/
  if (!IsDefined()) {
    if (rps.IsDefined())
      return -1;
    return 0;
  }
  if (!rps.IsDefined())
    return 1;

/*
Compare defined sequences. First compare them item by item until the end of the
shorter sequence. The first pair of unequal items decides, if any.

*/
  size_t min_size = std::min(size(), rps.size());
  for (size_t i = 0; i < min_size; ++i) {
    const int cmp = get(i).compare(rps.get(i));
    if (cmp != 0)
      return cmp;
  }

/*
Otherwise the sequences are equal, if they have the same length.

*/
  if (size() == rps.size())
    return 0;

/*
Otherwise the shorter sequence is considered ~less than~ the longer sequence.

*/
  if (rps.size() > min_size)
    return -1;

  return 1;
}


/*
Calculate a hash value of the sequence.

*/
template<class T>
size_t Sequence<T>::HashValue() const
{
  if (!IsDefined())
    return 0;

  size_t hash = 0;
/*
Iterate the sequence, get the hash for each item, and XOR it with the previous
hash value rotated by one bit.

*/
  for (size_t i = 0; i < size(); ++i)
    hash = get(i).hash() ^ (hash << 1 | hash >> ((sizeof(size_t)*CHAR_BIT)-1));

  return hash;
}


/*
Copy the content of ~rhs~ into ~this~, assuming that both are of the same type.

*/
template<class T>
void Sequence<T>::CopyFrom(const Attribute* rhs)
{
  const Sequence<T>& rps = *static_cast<const Sequence<T>*>(rhs);

  if (!rps.IsDefined()) {
    SetDefined(false);
    seq.clean();
    return;
  }

  SetDefined(true);
  seq.copyFrom(rps.seq);
}

/*
4 Implementation of Class ~Point~

Constructor to create an object from an ~NList~.

*/
Point::Point(const NList& list)
{
  if (list.isAtom()) {
    throw std::domain_error(
        "Element is an atom, but a list of 2 elements is required.");
  }

  if (list.length() != 2) {
    std::ostringstream os;
    os << "Element has " << list.length() << " elements, but 2 are required.";
    throw std::domain_error(os.str());
  }

  const NList& elem_x = list.first();
  const NList& elem_y = list.second();

  if (!elem_x.isReal()) {
    std::ostringstream os;
    os << "First element has non-real value '" << elem_x << "'.";
    throw std::domain_error(os.str());
  }

  if (!elem_y.isReal()) {
    std::ostringstream os;
    os << "Second element has non-real value '" << elem_y << "'.";
    throw std::domain_error(os.str());
  }

  x = elem_x.realval();
  y = elem_y.realval();
/*
The list parser does not support non-finite real values like ~not a number~ and
~infinity~. Therefore at this point both values are finite, and the ~Point~
object is valid.

*/
}


/*
Constructor to create a point from a ~TPoint~ object. The coordinates are copied
and the instant is ignored.

*/
Point::Point(const TPoint& src)
  : x(src.getX()),
    y(src.getY())
{ }


/*
Constructor to create a point from a defined ~::Point~ object.

*/
Point::Point(const ::Point& src)
  : x(src.GetX()),
    y(src.GetY())
{
  assert(src.IsDefined() && isValid());
}


/*
Get a string representation for display purposes.

*/
std::string Point::toString() const
{
  std::ostringstream os;
  os << "(" << x << ", " << y << ")";
  return os.str();
}


/*
Compare ~[*]this~ to ~rhs~.

*/
int Point::compare(const Point& rhs) const
{
  if (*this < rhs)
    return -1;

  if (*this == rhs)
    return 0;

  return 1;
}


/*
5 Implementation of Class ~PointSeq~

Recreate the sequence from a ~TPointSeq~.

*/

void PointSeq::convertFrom(const TPointSeq& src) noexcept
{
  SetDefined(false);
  seq.clean();

  if (!src.IsDefined())
    return;

  SetDefined(true);
  seq.resize(src.size());
  for (size_t i = 0; i < src.size(); ++i)
    append(Point(src.get(i)));
}



/*
6 Implementation of Class ~TPoint~

Constructor to create an object from an ~NList~.

*/
TPoint::TPoint(const NList& list)
  : instant(datetime::instanttype)
{
  if (list.isAtom())
    throw std::domain_error(
        "Element is an atom, but a list of 2 elements is required.");

  if (list.length() != 2) {
    std::ostringstream os;
    os << "Element has " << list.length() << " elements, but 2 are required.";
    throw std::domain_error(os.str());
  }

  const NList& elem_instant = list.first();
  const NList& elem_point = list.second();

  const bool success =
      instant.ReadFrom(elem_instant.listExpr(), /*typeincluded*/ false);
  if (!success) {
    std::ostringstream os;
    os << "First element '" << elem_instant << "' is no instant.";
    throw std::domain_error(os.str());
  }

  if (!instant.IsDefined())
    throw std::domain_error("Instant is undefined.");

  try {
    point = Point(elem_point);
  }
  catch (const std::domain_error& ex) {
    std::ostringstream os;
    os << "Second element is no point: " << ex.what();
    throw std::domain_error(os.str());
  }
/*
The ~instant~ has been checked explicitly for being defined, and the constructor
~Point(const NList\& list)~ does not support the creation of invalid (or
undefined) points. Therefore at this point the ~TPoint~ object is valid.

*/
}


/*
Verify that ~[*]this~ may appear as the successor of ~prec~ in a sequence of
~TPoint~s.

*/
void TPoint::validateSequenceOrder(const TPoint& prec) const
{
  if (instant <= prec.instant)
    throw std::domain_error(
        "Instant of element is not greater than instant of preceding element.");
}


/*
Get a string representation for display purposes.

*/
std::string TPoint::toString() const
{
  std::ostringstream os;
  os << "(" << instant.ToString() << ", " << point.toString() << ")";
  return os.str();
}


/*
Compare ~[*]this~ to ~rhs~.

*/
int TPoint::compare(const TPoint& rhs) const
{
  if (*this < rhs)
    return -1;

  if (*this == rhs)
    return 0;

  return 1;
}


/*
7 Registration of Type Constructors

7.1 ~pointseq~

*/

struct PointSeqInfo : ConstructorInfo
{
  PointSeqInfo()
  {
    name         = PointSeq::BasicType();
    signature    = "-> " + Kind::DATA();
    typeExample  = PointSeq::BasicType();
    listRep      =  "((x1 y1) (x2 y2) ... (xn yn))";
    valueExample = "((0.0 0.0) (1.0 1.0) (1.0 0.0))";
    remarks      = "Each element of the sequence is a 2-dimensional point "
                   "with real coordinates.";
  }
};

struct PointSeqFunctions : ConstructorFunctions<PointSeq> {

  PointSeqFunctions()
  {
    in = PointSeq::In;
    out = PointSeq::Out;
    create = PointSeq::Create;
    kindCheck = PointSeq::KindCheck;
  }
};

PointSeqInfo psi;
PointSeqFunctions psf;
TypeConstructor pointseq(psi, psf);

void TrajectorySimilarityAlgebra::addPointSeqTC()
{
  AddTypeConstructor(&pointseq);
  pointseq.AssociateKind(Kind::DATA());
}


/*
7.2 ~tpointseq~

*/
struct TPointSeqInfo : ConstructorInfo
{
  TPointSeqInfo()
  {
    name         = TPointSeq::BasicType();
    signature    = "-> " + Kind::DATA();
    typeExample  = TPointSeq::BasicType();
    listRep      =  "((i1 (x1 y1)) (i2 (x2 y2)) ... (in (xn yn)))";
    valueExample = "((\"2015-12-29-00:00:00\" (0.0 0.0)) "
                   "(\"2015-12-29-01:00:00\" (1.0 1.0)) "
                   "(\"2015-12-29-02:00:00\" (1.0 0.0)))";
    remarks      = "Each element of the sequence is a tuple of an instant and "
                   "a 2-dimensional point with real coordinates. The instants "
                   "are in strictly increasing order.";
  }
};

struct TPointSeqFunctions : ConstructorFunctions<TPointSeq> {

  TPointSeqFunctions()
  {
    in = TPointSeq::In;
    out = TPointSeq::Out;
    create = TPointSeq::Create;
    kindCheck = TPointSeq::KindCheck;
  }
};

TPointSeqInfo tpsi;
TPointSeqFunctions tpsf;
TypeConstructor tpointseq(tpsi, tpsf);

void TrajectorySimilarityAlgebra::addTPointSeqTC()
{
  AddTypeConstructor(&tpointseq);
  tpointseq.AssociateKind(Kind::DATA());
}


/*
8 Registration of Operators

8.1 ~isempty~

*/
const std::string is_empty_maps[2][2] = {
  /*0*/ {PointSeq::BasicType(),  /* -> */ CcBool::BasicType()},
  /*1*/ {TPointSeq::BasicType(), /* -> */ CcBool::BasicType()}
};

ListExpr IsEmptyTypeMap(ListExpr args)
{ return mappings::SimpleMaps<2, 2>(is_empty_maps, args); }

int IsEmptySelect(ListExpr args)
{ return mappings::SimpleSelect<2, 2>(is_empty_maps, args); }

template<class SEQ>
int IsEmpty(
    Word* args, Word& result, int /*message*/, Word& /*local*/, Supplier s)
{
  const SEQ& seq = *static_cast<SEQ*>(args[0].addr);
  result = qp->ResultStorage(s);    // CcBool
  CcBool& is_empty = *static_cast<CcBool*>(result.addr);
  is_empty.Set(/*defined*/ true, seq.size() == 0);
  return 0;
}

ValueMapping is_empty_functions[] = {
  IsEmpty<PointSeq>,
  IsEmpty<TPointSeq>,
  nullptr
};

struct IsEmptyInfo : OperatorInfo
{
  IsEmptyInfo() : OperatorInfo()
  {
    name      = "isempty";
    signature = PointSeq::BasicType() + " -> " + CcBool::BasicType();
    appendSignature(
                TPointSeq::BasicType() + " -> " + CcBool::BasicType());
    syntax    = "isempty(_)";
    meaning   = "Yields TRUE if the sequence is undefined or contains no items "
                "and FALSE otherwise. The time complexity is O(1).";
  }
};

void TrajectorySimilarityAlgebra::addIsEmptyOp()
{
  AddOperator(
      IsEmptyInfo(), is_empty_functions,
      IsEmptySelect, IsEmptyTypeMap);
}


/*
8.2 ~no\_components~

*/
const std::string no_components_maps[2][2] = {
  /*0*/ {PointSeq::BasicType(),  /* -> */ CcInt::BasicType()},
  /*1*/ {TPointSeq::BasicType(), /* -> */ CcInt::BasicType()}
};

ListExpr NoComponentsTypeMap(ListExpr args)
{ return mappings::SimpleMaps<2, 2>(no_components_maps, args); }

int NoComponentsSelect(ListExpr args)
{ return mappings::SimpleSelect<2, 2>(no_components_maps, args); }

template<class SEQ>
int NoComponents(
    Word* args, Word& result, int /*message*/, Word& /*local*/, Supplier s)
{
  const SEQ& seq = *static_cast<SEQ*>(args[0].addr);
  result = qp->ResultStorage(s);    // CcInt
  CcInt& no_components = *static_cast<CcInt*>(result.addr);
  no_components.Set(/*defined*/ true, seq.size());
  return 0;
}

ValueMapping no_components_functions[] = {
  NoComponents<PointSeq>,
  NoComponents<TPointSeq>,
  nullptr
};

struct NoComponentsInfo : OperatorInfo
{
  NoComponentsInfo() : OperatorInfo()
  {
    name      = "no_components";
    signature = PointSeq::BasicType() + " -> " + CcInt::BasicType();
    appendSignature(
                TPointSeq::BasicType() + " -> " + CcInt::BasicType());
    syntax    = "no_components(_)";
    meaning   = "Number of items in the sequence or 0, if the sequence is "
                "undefined. The time complexity is O(1).";
  }
};

void TrajectorySimilarityAlgebra::addNoComponentsOp()
{
  AddOperator(
      NoComponentsInfo(), no_components_functions,
      NoComponentsSelect, NoComponentsTypeMap);
}


/*
8.3 ~to\_dline~

*/
const std::string to_dline_maps[2][2] = {
  /*0*/ {PointSeq::BasicType(), /* -> */ DLine::BasicType()},
  /*1*/ {TPointSeq::BasicType(), /* -> */ DLine::BasicType()}
};

ListExpr ToDLineTypeMap(ListExpr args)
{ return mappings::SimpleMaps<2, 2>(to_dline_maps, args); }

int ToDLineSelect(ListExpr args)
{ return mappings::SimpleSelect<2, 2>(to_dline_maps, args); }

template<class T>
int ToDLine(
    Word* args, Word& result, int /*message*/, Word& /*local*/, Supplier s)
{
  const T& src = *static_cast<T*>(args[0].addr);
  result = qp->ResultStorage(s);    // DLine
  DLine& dl = *static_cast<DLine*>(result.addr);
  src.toDLine(dl);
  return 0;
}

ValueMapping to_dline_functions[] = {
  ToDLine<PointSeq>,
  ToDLine<TPointSeq>,
  nullptr
};

struct ToDLineInfo : OperatorInfo
{
  ToDLineInfo() : OperatorInfo()
  {
    name      = "to_dline";
    signature = PointSeq::BasicType() + " -> " + DLine::BasicType();
    appendSignature(
                TPointSeq::BasicType() + " -> " + DLine::BasicType());
    syntax    = "to_dline(_)";
    meaning   = "Creates a dline (for display purposes) from a point sequence. "
                "Each pair of successive points yields a segment of the dline. "
                "If the sequence contains just one point, the dline consists "
                "of single segment whose start and end are the same point.\n"
                "The time complexity is O(n).";
  }
};

void TrajectorySimilarityAlgebra::addToDLineOp()
{
  AddOperator(
      ToDLineInfo(), to_dline_functions,
      ToDLineSelect, ToDLineTypeMap);
}


/*
8.4 ~to\_pointseq~

*/
const std::string to_pointseq_maps[2][2] = {
  /*0*/ {MPoint::BasicType(), /* -> */ PointSeq::BasicType()},
  /*1*/ {TPointSeq::BasicType(), /* -> */ PointSeq::BasicType()}
};

ListExpr ToPointSeqTypeMap(ListExpr args)
{ return mappings::SimpleMaps<2, 2>(to_pointseq_maps, args); }

int ToPointSeqSelect(ListExpr args)
{ return mappings::SimpleSelect<2, 2>(to_pointseq_maps, args); }

template<class T>
int ToPointSeq(
    Word* args, Word& result, int /*message*/, Word& /*local*/, Supplier s)
{
  const T& src = *static_cast<T*>(args[0].addr);
  result = qp->ResultStorage(s);    // PointSeq
  PointSeq& ps = *static_cast<PointSeq*>(result.addr);
  ps.convertFrom(src);
  return 0;
}

ValueMapping to_pointseq_functions[] = {
  ToPointSeq<MPoint>,
  ToPointSeq<TPointSeq>,
  nullptr
};

struct ToPointSeqInfo : OperatorInfo
{
  ToPointSeqInfo() : OperatorInfo()
  {
    name      = "to_pointseq";
    signature = MPoint::BasicType() + " -> " + PointSeq::BasicType();
    appendSignature(
                TPointSeq::BasicType() + " -> " + PointSeq::BasicType());
    syntax    = "to_pointseq(_)";
    meaning   = "For an mpoint: Extracts the start point of the first unit of "
                "the moving point object followed by the end point of each "
                "unit of the moving point object whose interval contains "
                "not just a single instant. If there is a temporal or spatial "
                "gap between two successive units or a unit makes a spatial "
                "move in no time, the pointseq if undefined. This is "
                "equivalent to to_tpointseq(to_pointseq(m)) with an mpoint m.\n"
                "For a tpointseq: Spatial projection of the tpointseq.\n"
                "The time complexity is O(n).";
  }
};

void TrajectorySimilarityAlgebra::addToPointSeqOp()
{
  AddOperator(
      ToPointSeqInfo(), to_pointseq_functions,
      ToPointSeqSelect, ToPointSeqTypeMap);
}


/*
8.5 ~to\_tpointseq~

*/
const std::string to_tpointseq_maps[1][2] = {
  /*0*/ {MPoint::BasicType(), /* -> */ TPointSeq::BasicType()}
};

ListExpr ToTPointSeqTypeMap(ListExpr args)
{ return mappings::SimpleMaps<1, 2>(to_tpointseq_maps, args); }

int ToTPointSeqSelect(ListExpr args)
{ return mappings::SimpleSelect<1, 2>(to_tpointseq_maps, args); }

template<class T>
int ToTPointSeq(
    Word* args, Word& result, int /*message*/, Word& /*local*/, Supplier s)
{
  const T& src = *static_cast<T*>(args[0].addr);
  result = qp->ResultStorage(s);    // TPointSeq
  TPointSeq& ps = *static_cast<TPointSeq*>(result.addr);
  ps.convertFrom(src);
  return 0;
}

ValueMapping to_tpointseq_functions[] = {
  ToTPointSeq<MPoint>,
  nullptr
};

struct ToTPointSeqInfo : OperatorInfo
{
  ToTPointSeqInfo() : OperatorInfo()
  {
    name      = "to_tpointseq";
    signature = MPoint::BasicType() + " -> " + TPointSeq::BasicType();
    syntax    = "to_tpointseq(_)";
    meaning   = "Extracts the start instant and point of the first unit of the "
                "moving point object followed by the end instant and point of "
                "each unit of the moving point object whose interval contains "
                "not just a single instant. If there is a temporal or spatial "
                "gap between two successive units or a unit makes a spatial "
                "move in no time, the tpointseq if undefined.\n"
                "The time complexity is O(n).";
  }
};

void TrajectorySimilarityAlgebra::addToTPointSeqOp()
{
  AddOperator(
      ToTPointSeqInfo(), to_tpointseq_functions,
      ToTPointSeqSelect, ToTPointSeqTypeMap);
}

} //-- namespace tsa
