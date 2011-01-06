/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[->] [$\rightarrow $]

{\Large \bf Anhang G: SpatialJoin-Algorithmus }

[1] Implementation of SpatialJoin-Algebra

December 2010, Jiamin Lu created *spatialjoin2* operator
by evaluating the *parajoin2* operator.

[TOC]

0 Overview

1 Defines and Includes

*/


using namespace std;

#include <string.h>
#include <vector>
#include <list>
#include <set>
#include <queue>

#include "SpatialJoinAlgebra.h"


extern NestedList* nl;
extern QueryProcessor* qp;

#define BBox Rectangle


/*
3 Operator ~spatjoin~

3.1 Overview

The new *spatialjoin2* operator performs the operation
by invoking a Secondo query composed by *parajoin2*,
*gridintersects*, *cellnumber* and other operators.

At present, the new version doesn't support join for 4D rectangles.

Basically, this operation uses partitioned based spatial-merge join
algorithm. This method partitions the input relations into
a cell gird, then execute the join operation within each cell.

The methods of partitioning tuples into cells, and joining tuples
inside cells are already implemented in some other Secondo operators,
but these operators need to set a lot parameters changed by different
inputs. Therefore, we implemented this operator to scan the inputs
and calculate these parameters automatically, to make the query as
easy as possible.
After setting these parameters, a fixed Secondo query is executed
inside this *spatialjoin2* operator, as a parameter function.

*/

/*
3.3 Type mapping function of operator ~spatialjoin2~

At first, we plan to invoke a new ~QueryProcessor~ in the valuemapping
function of the operator to perform the fixed *parajoin2* query.
But the results returned by the ~QueryProcessor~ are expressed
by nested-lists, and it's too expensive to resolve these nested-lists
into tuples. So we use another way to invoking the fixed Secondo query
by invoking the query as a parameter function.

In the typemapping function, we build the nested-list of the query,
and set this function as an appended element.
Then in the valuemapping function, we can scan the inputs,
set necessary parameters to this function, and get the result tuple
stream from the parameter function directly.

The signature of *spatialjoin2* is:

----
( (stream || rel (tuple ((x1 t1)...(xn tn))))
  (stream || rel (tuple ((y1 yt1)...(yn ytn))))
  rect||rect3||spatialtype
  rect||rect3||spatialtype )
  -> (stream (tuple ((x1 t1)...(xn tn)((y1 yt1)...(yn ytn)))))
----

*/

ListExpr spatialJoinTypeMap(ListExpr args)
{
  string lenErr = "4 parameters expected";
  string oldErr = "stream(tuple) x stream(tuple) "
                  "x name1 x name2 expected";
  string newErr = "{stream(tuple), rel(tuple)} "
                "x {stream(tuple), rel(tuple)} "
                "x name1 x name2 x isOld expected";
  string mapErr = "It's not a map ";

  if (nl->ListLength(args) != 4)
    return listutils::typeError(lenErr);

  ListExpr stream1 = nl->First(args);
  ListExpr stream2 = nl->Second(args);
  ListExpr nameL1 = nl->Third(args);
  ListExpr nameL2 = nl->Fourth(args);

  bool isRel[] = {false, false};
  if (listutils::isRelDescription(stream1))
    isRel[0] = true;
  else if (!listutils::isTupleStream(stream1))
    return listutils::typeError(newErr);
  if (listutils::isRelDescription(stream2))
    isRel[1] = true;
  else if (!listutils::isTupleStream(stream2))
    return listutils::typeError(newErr);

  if (!listutils::isSymbol(nameL1) ||
      !listutils::isSymbol(nameL2))
    return listutils::typeError(newErr);

  ListExpr al1 = nl->Second(nl->Second(stream1));
  ListExpr al2 = nl->Second(nl->Second(stream2));

  if(!listutils::disjointAttrNames(al1, al2)){
    return listutils::typeError("conflicting type names");
  }

  ListExpr type1;
  string name1 = nl->SymbolValue(nameL1);
  int index1 = listutils::findAttribute(al1,name1,type1);
  if(index1==0){
    return listutils::typeError("attribute " + name1 + "not found");
  }

  ListExpr type2;
  string name2 = nl->SymbolValue(nameL2);
  int index2 = listutils::findAttribute(al2,name2,type2);
  if(index2==0){
    return listutils::typeError("attribute " + name2 + "not found");
  }

  // check for rect, rect3, rect4
  set<string> r;
  r.insert("rect");
  r.insert("rect3");

  if(!listutils::isASymbolIn(type1,r) &&
     !listutils::isKind(type1,"SPATIAL2D")){
    return listutils::typeError("attribute " + name1 +
                                " not supported by spatial join");
  }
  if(!listutils::isASymbolIn(type2,r) &&
     !listutils::isKind(type1,"SPATIAL2D")){
    return listutils::typeError("attribute "
        + name2 + " not supported");
  }

  if(!listutils::isSymbol(type1) ||
     !listutils::isSymbol(type2)){
    return listutils::typeError("composite types not supported");
  }
  if(!nl->Equal(type1,type2)){
    return listutils::typeError("different types");
  }

  ListExpr attrlist = listutils::concat(al1, al2);

  NList funList(nl);
  //compose the nested list of the internal function
  bool is3D = false;
  if (nl->SymbolValue(type1) == "rect3")
    is3D = true;

  const ListExpr sType[] = {stream1, stream2};  //stream type
  const string aName[] = {name1, name2};  // join attribute name

  //extended attribute names
  const string eaName2[] = {"xxx_box_l", "xxx_box_r"};
  const string eaName[] = {"xxx_cell_l", "xxx_cell_r"};

  //parameter names
  const string ptName[] =  {"streamLx", "streamRx"};// stream name
  const string ptName1[] = {"tupleLx", "tupleRx"};
  const string ptName2[] = {"tupleL2x", "tupleR2x"};
  const string ptName3[] = {"streamelemLx", "streamelemRx"};

  funList.append(NList("fun"));
  funList.append(NList(NList(ptName[0]), NList(sType[0])));
  funList.append(NList(NList(ptName[1]), NList(sType[1])));

  funList.append(NList(NList("xl"), NList(Symbols::REAL())));
  funList.append(NList(NList("yb"), NList(Symbols::REAL())));
  funList.append(NList(NList("wx"), NList(Symbols::REAL())));
  funList.append(NList(NList("wy"), NList(Symbols::REAL())));
  funList.append(NList(NList("nx"), NList(Symbols::INT())));
  funList.append(NList(NList("xr"), NList(Symbols::REAL())));
  funList.append(NList(NList("yt"), NList(Symbols::REAL())));
  if (is3D)
  {
    funList.append(NList(NList("zb"), NList(Symbols::REAL())));
    funList.append(NList(NList("wz"), NList(Symbols::REAL())));
    funList.append(NList(NList("ny"), NList(Symbols::INT())));
    funList.append(NList(NList("zt"), NList(Symbols::REAL())));
  }

  NList pjList(nl);
  pjList.append(NList("parajoin2"));
  for(int i = 0; i < 2; i++)
  {
    ListExpr streamList;
    stringstream listStr;
    listStr
    << " (sortby (extendstream "
    << "(filter "
    << "(extend "
    << (isRel[i] ? "( feed " + ptName[i] + ")" : ptName[i])
    << "( ( " + eaName2[i]
    << "(fun (" << ptName2[i] << " TUPLE)"
    << "(bbox (attr " << ptName2[i] << " " << aName[i] << "))))))"
    << "(fun (" << ptName3[i] << " STREAMELEM)"
    << "(and (>= (maxD (attr " << ptName3[i]
                        << " " << eaName2[i] << ") 1) xl)"
      << "(and (>= (maxD (attr " << ptName3[i]
                          << " " << eaName2[i] << ") 2) yb)"
        << "(and (<= (minD (attr " << ptName3[i]
                            << " " << eaName2[i] << ") 1) xr)";

    if (!is3D)
      listStr
      << "(<= (minD (attr " << ptName3[i]
                     << " " << eaName2[i] << ") 2) yt)"
      << ")))))";
    else
      listStr
      << "(and (<= (minD (attr " << ptName3[i]
                          << " " << eaName2[i] << ") 2) yt)"
        << "(and (>= (maxD (attr " << ptName3[i]
                            << " " << eaName2[i] << ") 3) zb)"
             << "(<= (maxD (attr " << ptName3[i]
                            << " " << eaName2[i] << ") 3) zt)"
      << ")))))))";

    listStr << "( ( " << eaName[i]
            << "(fun ( " << ptName1[i] << " "
                << "TUPLE" << " ) "
            << "(cellnumber "
              << "(bbox (attr " << ptName1[i]
                         << " " << aName[i] << " ))"
                << " xl yb " << (is3D ? "zb" : "")
                << " wx wy " << (is3D ? "wz" : "")
                << " nx " << (is3D ? "ny" : "")
            << " ))))) (( " << eaName[i] << " asc))) ";
    nl->ReadFromString(listStr.str(), streamList);
    pjList.append(NList(streamList));
  }
  pjList.append(NList(eaName[0]));
  pjList.append(NList(eaName[1]));



  ListExpr inFunL;
  stringstream inFunStr;
  inFunStr << "( fun "
           << "(stream2Lx " << "ANY" << ") "
           << "(stream2Rx " << "ANY2" << ") "
           << "(symmjoin stream2Lx stream2Rx "
           << "(fun "
             << "(tupleL3x TUPLE ) "
             << "(tupleR3x TUPLE2 ) "
              << "(gridintersects xl yb " << (is3D ? "zb" : "")
                << " wx wy " << (is3D ? "wz" : "")
                << " nx " << (is3D ? "ny" : "")
                << " (bbox (attr tupleL3x " << aName[0]  << ")) "
                << " (bbox (attr tupleR3x " << aName[1] << ")) "
                << " (attr tupleL3x " << eaName[0] << "))"
                     << ")))";
  nl->ReadFromString(inFunStr.str(), inFunL);
  pjList.append(NList(inFunL));
  funList.append(NList(NList("remove"),
                pjList,
                NList(NList(eaName[0])
                      ,NList(eaName[1])
                      ,NList(eaName2[0])
                      ,NList(eaName2[1])
                      )));
  //remove the extended attributes,
  //to make sure the equality of the output tuple type


  return nl->ThreeElemList(
             nl->SymbolAtom("APPEND"),
             nl->SixElemList(
                 nl->IntAtom(index1),
                 nl->IntAtom(index2),
                 nl->StringAtom(nl->SymbolValue(type1)),
                 nl->BoolAtom(isRel[0]),
                 nl->BoolAtom(isRel[1]),
                 funList.listExpr()
                 ),
             nl->TwoElemList(
                 nl->SymbolAtom("stream"),
                 nl->TwoElemList(
                     nl->SymbolAtom("tuple"),
                     attrlist)));
}

/*
3.4 Selection function of operator ~spatjoin~

*/

int
spatialJoinSelection (ListExpr args)
{
  /* find out type of key; similar to typemapping function */
  /* Split argument in four parts */
  ListExpr relDescriptionS = nl->First(args);//outerRelation
  ListExpr attrNameS_LE = nl->Third(args);   //attrName of outerRel
  ListExpr tupleDescriptionS = nl->Second(relDescriptionS);
  ListExpr attrListS = nl->Second(tupleDescriptionS);

  /* handle attrName of outerRelation */
  int attrIndexS;
  ListExpr attrTypeS;
  string attrNameS = nl->SymbolValue(attrNameS_LE);
  attrIndexS = FindAttribute(attrListS, attrNameS, attrTypeS);

  /* selection function */
  ListExpr errorInfo = nl->OneElemList ( nl->SymbolAtom ("ERRORS"));
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
  if ( (algMgr->CheckKind("SPATIAL2D", attrTypeS, errorInfo)) ||
       ( nl->SymbolValue (attrTypeS) == "rect") )
  return 0;  //two-dimensional objects to join
  else if ( nl->SymbolValue (attrTypeS) == "rect3")
       return 1; //three-dimensiona objects to join
       else if ( nl->SymbolValue (attrTypeS) == "rect4")
            return 2;  //four-dimensional objects to join
            else return -1; /* should not happen */
}

/*
3.5 ~SpatialJoinLocalInfo~: Auxiliary Class for operator ~spatialjoin~

*/

template <unsigned dim>
SpatialJoin2LocalInfo<dim>::SpatialJoin2LocalInfo(
                       Word leftStreamWord, bool _isLR,
                       Word leftAttrIndexWord,
                       Word rightStreamWord, bool _isRR,
                       Word rightAttrIndexWord,
                       Word funWord, Supplier s)
{
  isSet = false;

  int aiLeft = ((CcInt*)leftAttrIndexWord.addr)->GetValue() - 1;
  int aiRight = ((CcInt*)rightAttrIndexWord.addr)->GetValue() - 1;
  pf = funWord.addr;

  r[0].isRel = _isLR;
  r[0].streamWord = leftStreamWord;
  r[1].isRel = _isRR;
  r[1].streamWord = rightStreamWord;

  scanStream(aiLeft, leftStream);
  scanStream(aiRight, rightStream);
  joinBox =
      new Rectangle<dim>(r[0].MBR->Intersection(*(r[1].MBR)));

  ArgVectorPointer funargs = qp->Argument(pf);
  if (r[0].isRel)
    (*funargs)[0].setAddr((GenericRelation*)r[0].streamWord.addr);
  else
    qp->SetupStreamArg(pf, 1, s);
  if (r[1].isRel)
    (*funargs)[1].setAddr((GenericRelation*)r[1].streamWord.addr);
  else
    qp->SetupStreamArg(pf, 2, s);

/*
Revision RHG 28.12.2010

(a) 2d case

We let the number of cells k in the grid correspond to the cardinality of the smaller
argument divided by 4. This means that roughly 4 elements of this set would cover
a grid cell and about 9 of them would intersect a grid cell (based on a calculation
with squares). Also each element would be represented 9/4 times.

Hence k = cardMin / 4.

Let B be the area of the enclosing box. As we have k grid cells, the area of each
cell is B/k, and the width of a square grid cell is wx = sqrt(B/k).

Let the width of the box be BX. Then the number of cells in a row is nx = BX/wx.


(b) 3d case

Using as similar rule, we let the number of cells k correspond to the cardinality
of the smaller set divided by 8. This means that 3\verb+^+3 = 27 are expected to intersect
a grid cell; hence the overhead is 27/8.

Hence k = cardMin / 8

The width of a grid cell is wx = (B/k)\verb+^+(1/3).

nx = BX / wx; ny = BY / wx;

Revision Jiamin 05.01.2011

By now, we didn't find a generic formula to decide the optimal cell number,
for achieving an acceptable reduplication coefficient
and a minimal overhead.

At present, we simply define the size of cells in the grid
is 5 times the objects' average size.
In 3D case, considering the 3rd edge's length of the data set's bounding box
is always much shorter than the other two sizes,
cells' height is set alone.

*/

  double length, width, height;
  double xl, yb, zb, wx, wz;
  double xr, yt, zt;
  int nx, ny;

  double size[dim];
  for(unsigned i = 0; i < dim; i++)
  {
    size[i] = ((r[0].avgSize[i] * r[0].card)
          + (r[1].avgSize[i] * r[1].card) )
        / double(r[0].card + r[1].card);
  }
  wx = ((size[0] < size[1]) ? size[0] : size[1]) * 5;
  if (3 == dim)
    wz = size[2] * 5;

  if (2 == dim)
  {
    //2D space grid
    length = joinBox->MaxD(0) - joinBox->MinD(0);
    width = joinBox->MaxD(1) - joinBox->MinD(1);
    xl = joinBox->MinD(0);
    yb = joinBox->MinD(1);
    xr = joinBox->MaxD(0);
    yt = joinBox->MaxD(1);

    nx = ceil(length / wx);

    (*funargs)[2].setAddr(new CcReal(xl));
    (*funargs)[3].setAddr(new CcReal(yb));
    (*funargs)[4].setAddr(new CcReal(wx));
    (*funargs)[5].setAddr(new CcReal(wx));
    (*funargs)[6].setAddr(new CcInt(nx));
    (*funargs)[7].setAddr(new CcReal(xr));
    (*funargs)[8].setAddr(new CcReal(yt));

  }
  else
  {
    //3D space grid
    length = joinBox->MaxD(0) - joinBox->MinD(0);
    width = joinBox->MaxD(1) - joinBox->MinD(1);
    height = joinBox->MaxD(2) - joinBox->MinD(2);
    xl = joinBox->MinD(0);
    yb = joinBox->MinD(1);
    zb = joinBox->MinD(2);
    xr = joinBox->MaxD(0);
    yt = joinBox->MaxD(1);
    zt = joinBox->MaxD(2);

    nx = ceil(length / wx);
    ny = ceil(width / wx);

    (*funargs)[2].setAddr(new CcReal(xl));
    (*funargs)[3].setAddr(new CcReal(yb));
    (*funargs)[4].setAddr(new CcReal(wx));
    (*funargs)[5].setAddr(new CcReal(wx));
    (*funargs)[6].setAddr(new CcInt(nx));
    (*funargs)[7].setAddr(new CcReal(xr));
    (*funargs)[8].setAddr(new CcReal(yt));
    (*funargs)[9].setAddr(new CcReal(zb));
    (*funargs)[10].setAddr(new CcReal(wz));
    (*funargs)[11].setAddr(new CcInt(ny));
    (*funargs)[12].setAddr(new CcReal(zt));

  }

  isSet = true;
}

template<unsigned dim>
void
SpatialJoin2LocalInfo<dim>::scanStream( int attrIndex,
                                       streamType loc)
{
  Rectangle<dim> *MBR = 0;
  r[loc].streamBuffer = 0;
  int tupleNo = 0;
  double size[dim];
  memset(size, 0, sizeof(double)*dim);

  if (r[loc].isRel)
  {
    //Scan the relation tuple by tuple
    GenericRelation* rel =
        (GenericRelation*)r[loc].streamWord.addr;
    GenericRelationIterator *iter = rel->MakeScan();
    Tuple* nextTup = iter->GetNextTuple();
    while(!iter->EndOfScan())
    {
      Rectangle<dim>
          tBox = ((StandardSpatialAttribute<dim>*) nextTup
              ->GetAttribute(attrIndex))->BoundingBox();
      for(unsigned i = 0; i < dim; i++)
        size[i] += tBox.MaxD(i) - tBox.MinD(i);
      if (!MBR)
        MBR = new Rectangle<dim> (tBox);
      else
        *MBR = tBox.Union(*MBR);
      nextTup->DeleteIfAllowed();
      tupleNo++;
      nextTup = iter->GetNextTuple();
    }
    delete iter;
  }
  else
  {
    //Scan the stream tuple by tuple
    Word streamTupleWord;
    qp->Open(r[loc].streamWord.addr);
    qp->Request(r[loc].streamWord.addr, streamTupleWord);
    r[loc].streamBuffer =
        new TupleBuffer2(qp->MemoryAvailableForOperator());
    while (qp->Received(r[loc].streamWord.addr))
    {
      Tuple *nextTup = (Tuple*)streamTupleWord.addr;
      r[loc].streamBuffer->AppendTuple(nextTup);
      Rectangle<dim> tBox =
          ((StandardSpatialAttribute<dim>*)nextTup
              ->GetAttribute(attrIndex))->BoundingBox();
      for(unsigned i = 0; i < dim; i++)
        size[i] += tBox.MaxD(i) - tBox.MinD(i);
      if (!MBR)
        MBR = new Rectangle<dim>(tBox);
      else
        *MBR = tBox.Union(*MBR);

      nextTup->DeleteIfAllowed();
      qp->Request(r[loc].streamWord.addr, streamTupleWord);
    }
    qp->Close(r[loc].streamWord.addr);
    tupleNo = r[loc].streamBuffer->GetNoTuples();
  }

  r[loc].card = tupleNo;
  r[loc].MBR = MBR;
  for(unsigned i = 0; i < dim; i++)
    r[loc].avgSize[i] = size[i] / (double)tupleNo;
}

template<unsigned dim>
void
SpatialJoin2LocalInfo<dim>::openInputStream(streamType loc)
{
  assert(!r[loc].isRel);
  r[loc].tb2Iter = r[loc].streamBuffer->MakeScan();
}

template<unsigned dim>
Tuple*
SpatialJoin2LocalInfo<dim>::getNextInputTuple(streamType loc)
{
  assert(!r[loc].isRel);
  return r[loc].tb2Iter->GetNextTuple();
}

template<unsigned dim>
void
SpatialJoin2LocalInfo<dim>::closeInputStream(streamType loc)
{
  assert(!r[loc].isRel);
  delete r[loc].tb2Iter;
  r[loc].tb2Iter = 0;
}

template<unsigned dim>
Tuple*
SpatialJoin2LocalInfo<dim>::NextResultTuple()
{
  Word funResult(Address(0));

  qp->Request(pf, funResult);
  if (funResult.addr)
    return (Tuple*)funResult.addr;
  else
  {
    qp->Close(pf);
    return 0;
  }
}

/*
3.5 Value mapping function of operator ~spatjoin~

*/
template<int D>
int
spatialJoinValueMapping(Word* args, Word& result, int message,
                         Word& local, Supplier s)
{
  SpatialJoin2LocalInfo<D> *localInfo = 0;

  switch(message)
  {
    case OPEN:
    {
      if (localInfo)
        delete localInfo;
      localInfo =
          new SpatialJoin2LocalInfo<D>(
            args[0], ((CcBool*)args[7].addr)->GetValue(), args[4],
            args[1], ((CcBool*)args[8].addr)->GetValue(), args[5],
            args[9], s);

      local.setAddr(localInfo);
      localInfo->OpenFunction();
      return 0;
    }
    case REQUEST:
    {
      result.setAddr(Address(0));
      if (0 != local.addr)
      {
        localInfo = (SpatialJoin2LocalInfo<D>*)local.addr;
        result.setAddr(localInfo->NextResultTuple());
      }
      return result.addr != 0 ? YIELD : CANCEL;
    }
    case (1*FUNMSG)+OPEN:{
      if (0 != local.addr)
      {
        localInfo = (SpatialJoin2LocalInfo<D>*)local.addr;
        localInfo->openInputStream(leftStream);
      }
      return 0;
    }
    case (2*FUNMSG)+OPEN:{
      if (0 != local.addr)
      {
        localInfo = (SpatialJoin2LocalInfo<D>*)local.addr;
        localInfo->openInputStream(rightStream);
      }
      return 0;
    }
    case (1*FUNMSG)+REQUEST:{
      if (0 == local.addr)
      {
        return CANCEL;
      }

      localInfo = (SpatialJoin2LocalInfo<D>*)local.addr;
      result.setAddr(localInfo->getNextInputTuple(leftStream));
      if (result.addr)
        return YIELD;
      else
        return CANCEL;
    }
    case (2*FUNMSG)+REQUEST:{
      if (0 == local.addr)
      {
        return CANCEL;
      }
      localInfo = (SpatialJoin2LocalInfo<D>*)local.addr;
      result.setAddr(localInfo->getNextInputTuple(rightStream));
      if (result.addr)
        return YIELD;
      else
        return CANCEL;
    }
    case (1*FUNMSG)+CLOSE:{
      if (0 != local.addr) {
        localInfo = (SpatialJoin2LocalInfo<D>*)local.addr;
        localInfo->closeInputStream(leftStream);
      }
      return 0;
    }
    case (2*FUNMSG)+CLOSE:{
      if (0 != local.addr) {
        localInfo = (SpatialJoin2LocalInfo<D>*)local.addr;
        localInfo->closeInputStream(rightStream);
      }
      return 0;
    }
    case CLOSE:
    {
      if (local.addr)
      {
        localInfo = (SpatialJoin2LocalInfo<D>*)local.addr;
        delete localInfo;
        local.setAddr(0);
      }
      return 0;
    }
  }
  return 0;

};

/*
3.7 Definition of value mapping vectors

*/
ValueMapping spatialJoinMap [] = {spatialJoinValueMapping<2>,
    spatialJoinValueMapping<3>,
    spatialJoinValueMapping<4> };

/*
3.8 Specification of operator ~spatjoin~

*/
const string spatialJoinSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\""
    " \"Example\" )"
    "( <text>( (stream || rel (tuple ((x1 t1)...(xn tn)))) "
    "(stream || rel (tuple ((y1 yt1)...(yn ytn))))"
    " rect||rect3||SpatialType rect||rect3||SpatialType) -> "
    "(stream (tuple ((x1 t1)...(xn tn) ((y1 yt1)...(yn ytn)))))"
    "</text--->"
    "<text> stream1||rel1 stream2||rel2 spatjoin2 [Attr1, Attr2]"
    "</text--->"
    "<text> Perform the spatialjoin operation by invoking the "
    "parajoin2 operator automatically, "
    "avoid setting a lot of parameters. </text--->"
    "<text></text--->"
    ") )";

/*
3.9 Definition of operator ~spatjoin~

*/

Operator spatialjoin2 (
         "spatialjoin2",          // name
         spatialJoinSpec,         // specification
         3,                       // number of overloaded functions
         spatialJoinMap,          // value mapping
         spatialJoinSelection,    // trivial selection function
         spatialJoinTypeMap       // type mapping
);
/*
4 Definition and initialization of ~SpatialJoin~ algebra

*/
class SpatialJoinAlgebra : public Algebra
{
 public:
  SpatialJoinAlgebra() : Algebra()
  {
    AddOperator(&spatialjoin2);
  }
  ~SpatialJoinAlgebra() {};
};


extern "C"
Algebra*
InitializeSpatialJoinAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new SpatialJoinAlgebra());
}

