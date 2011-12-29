/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[->] [$\rightarrow $]

{\Large \bf Anhang G: SpatialJoin-Algorithmus }

[1] Implementation of SpatialJoin-Algebra

December 2010, Jiamin Lu created *spatialjoin2* operator
by evaluating the *parajoin2* operator.

July 2011, Jiamin Lu moved the *parajoin2* operator to
SpatialjoinAlgebra. The *spatialjoin2* operator is renamed
as *spatialjoin* operator.

September 2011, Jiamin Lu changed the internal join operator in spatialjoin
operator, from *symmjoin* to *realJoinMMRTree* + *filter*.
The cell size for 2D grid, is enlarged with 10 times,
according to the evaluation result that we got in parallel processing.

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
#include "Symbols.h"

extern NestedList* nl;
extern QueryProcessor* qp;

#define BBox Rectangle


/*
3 Operator ~spatialjoin2~

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
( (stream(tuple ((x1 t1)...(xn tn))))
  (stream(tuple ((y1 yt1)...(yn ytn))))
  rect||rect3||spatialtype
  rect||rect3||spatialtype )
  -> (stream (tuple ((x1 t1)...(xn tn)((y1 yt1)...(yn ytn)))))
----

*/

ListExpr spatialJoinTypeMap(ListExpr args)
{
  string lenErr = "4 parameters expected";
  string newErr = "stream(tuple) x stream(tuple) "
                  "x name1 x name2 expected";
  string mapErr = "It's not a map ";

  if (nl->ListLength(args) != 4)
    return listutils::typeError(lenErr);

  ListExpr stream1 = nl->First(args);
  ListExpr stream2 = nl->Second(args);
  ListExpr nameL1 = nl->Third(args);
  ListExpr nameL2 = nl->Fourth(args);

  if (!listutils::isTupleStream(stream1))
    return listutils::typeError(newErr);

  if (!listutils::isTupleStream(stream2))
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
  r.insert(Rectangle<2>::BasicType());
  r.insert(Rectangle<3>::BasicType());

  if(!listutils::isASymbolIn(type1,r) &&
     !listutils::isKind(type1,Kind::SPATIAL2D())){
    return listutils::typeError("attribute " + name1 +
                                " not supported by spatial join");
  }
  if(!listutils::isASymbolIn(type2,r) &&
     !listutils::isKind(type1,Kind::SPATIAL2D())){
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
  if (nl->SymbolValue(type1) == Rectangle<3>::BasicType())
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
  const string ptName4[] = {"stream2Lx", "stream2Rx"};
  const string ptName5 =  "streamelemMG";   //merged stream element

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
    << ptName[i]
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
             << "(<= (minD (attr " << ptName3[i]
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

  NList interJoinList(nl);
  interJoinList.append(NList("realJoinMMRTreeVec"));
  interJoinList.append(NList(ptName4[0]));
  interJoinList.append(NList(ptName4[1]));
  interJoinList.append(NList(eaName2[0]));
  interJoinList.append(NList(eaName2[1]));
  interJoinList.append(NList(10));
  interJoinList.append(NList(20));

  NList gisCheckList(nl);  //Prepare for gridintersects operator
  gisCheckList.append(NList("gridintersects"));
  gisCheckList.append(NList("xl"));
  gisCheckList.append(NList("yb"));
  if (is3D){
    gisCheckList.append(NList("zb"));
  }
  gisCheckList.append(NList("wx"));
  gisCheckList.append(NList("wy"));
  if (is3D){
    gisCheckList.append(NList("wz"));
  }
  gisCheckList.append(NList("nx"));
  if (is3D){
    gisCheckList.append(NList("ny"));
  }
  gisCheckList.append(
      NList(NList("attr"), NList(ptName5), NList(eaName2[0])));
  gisCheckList.append(
      NList(NList("attr"), NList(ptName5), NList(eaName2[1])));
  gisCheckList.append(
      NList(NList("attr"), NList(ptName5), NList(eaName[0])));

  NList inFunList(nl);
  inFunList.append(NList("fun"));
  inFunList.append(NList(NList(ptName4[0]), NList("ANY")));
  inFunList.append(NList(NList(ptName4[1]), NList("ANY2")));
  inFunList.append(
    NList(
      NList("remove"),
      NList(
          NList("filter"),
          interJoinList,
          NList(NList("fun"),
            NList(NList(ptName5), NList("STREAMELEM")),
              NList(
                NList("and"),
                NList(
                  NList("="),
                  NList(NList("attr"), NList(ptName5), NList(eaName[0])),
                  NList(NList("attr"), NList(ptName5), NList(eaName[1]))),
                gisCheckList ))),
      NList(
        NList(eaName[0])
        ,NList(eaName[1])
        ,NList(eaName2[0])
        ,NList(eaName2[1]) ))  );

  pjList.append( NList(inFunList) );
  funList.append( NList(pjList) );

  //remove the extended attributes,
  //to make sure the equality of the output tuple type


  // nl->WriteListExpr(funList.listExpr());

  return nl->ThreeElemList(
             nl->SymbolAtom(Symbol::APPEND()),
             nl->ThreeElemList(
                 nl->IntAtom(index1),
                 nl->IntAtom(index2),
                 funList.listExpr()
                 ),
             nl->TwoElemList(
                 nl->SymbolAtom(Symbol::STREAM()),
                 nl->TwoElemList(
                     nl->SymbolAtom(Tuple::BasicType()),
                     attrlist)));
}

/*
3.4 Selection function of operator ~spatialjoin~

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
  ListExpr errorInfo = nl->OneElemList ( nl->SymbolAtom (Symbol::ERRORS()));
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
  if ( (algMgr->CheckKind(Kind::SPATIAL2D(), attrTypeS, errorInfo)) ||
       ( nl->SymbolValue (attrTypeS) == Rectangle<2>::BasicType()) )
  return 0;  //two-dimensional objects to join
  else if ( nl->SymbolValue (attrTypeS) == Rectangle<3>::BasicType())
       return 1; //three-dimensiona objects to join
       else if ( nl->SymbolValue (attrTypeS) == Rectangle<4>::BasicType())
            return 2;  //four-dimensional objects to join
            else return -1; /* should not happen */
}



// progress version only

/*
3.5 ~SpatialJoinLocalInfo~: Auxiliary Class for operator ~spatialjoin~

*/

template <unsigned dim>
SpatialJoin2LocalInfo<dim>::SpatialJoin2LocalInfo(
                       Word leftStreamWord, 
                       Word leftAttrIndexWord,
                       Word rightStreamWord,
                       Word rightAttrIndexWord,
                       Word funWord, Supplier s,
                       ProgressLocalInfo* p) : ProgressWrapper(p)
{
  isSet = false;

  int aiLeft = ((CcInt*)leftAttrIndexWord.addr)->GetValue() - 1;
  int aiRight = ((CcInt*)rightAttrIndexWord.addr)->GetValue() - 1;
  pf = funWord.addr;

  r[0].streamWord = leftStreamWord;
  r[1].streamWord = rightStreamWord;

  scanStream(aiLeft, leftStream, s); p->read = r[0].card;
	// cout << "left stream: p->read = " << p->read << endl;
  scanStream(aiRight, rightStream, s); p->total = r[1].card;
	// cout << "right stream: p->total = " << p->total << endl;
  joinBox =
      new Rectangle<dim>(r[0].MBR->Intersection(*(r[1].MBR)));

  ArgVectorPointer funargs = qp->Argument(pf);
  qp->SetupStreamArg(pf, 1, s);
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
  double xl, yb, zb, wx, wz = 0.0;
  double xr, yt, zt;
  size_t nx, ny, nz;

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

/*
According to the evaluation in parallel processing,
we enlarge the cell size for 2D space with 10 times.

*/
    wx *= 10;
    do
    {
      nx = size_t(ceil(length / wx));
      if ((nx > size_t(sqrt(INT_MAX))))
        wx *= 10;
      else
        break;
    }while(1);

    (*funargs)[2].setAddr(new CcReal(xl));
    (*funargs)[3].setAddr(new CcReal(yb));
    (*funargs)[4].setAddr(new CcReal(wx));
    (*funargs)[5].setAddr(new CcReal(wx));
    (*funargs)[6].setAddr(new CcInt(int(nx)));
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

    do
    {
      nx = size_t(ceil(length / wx));
      ny = size_t(ceil(width  / wx));
      nz = size_t(ceil(height / wz));
      size_t maxN = max(max(nx,ny),nz);
      if (maxN > size_t(pow(INT_MAX,1.0/3.0)))
      {
        wx *= 10;
        wz *= 10;
      }
      else
        break;
    }while(1);

    (*funargs)[2].setAddr(new CcReal(xl));
    (*funargs)[3].setAddr(new CcReal(yb));
    (*funargs)[4].setAddr(new CcReal(wx));
    (*funargs)[5].setAddr(new CcReal(wx));
    (*funargs)[6].setAddr(new CcInt(int(nx)));
    (*funargs)[7].setAddr(new CcReal(xr));
    (*funargs)[8].setAddr(new CcReal(yt));
    (*funargs)[9].setAddr(new CcReal(zb));
    (*funargs)[10].setAddr(new CcReal(wz));
    (*funargs)[11].setAddr(new CcInt(int(ny)));
    (*funargs)[12].setAddr(new CcReal(zt));

  }

  isSet = true;
}

template<unsigned dim>
void
SpatialJoin2LocalInfo<dim>::scanStream( int attrIndex,
                                       streamType loc, Supplier s )
{
  Rectangle<dim> *MBR = 0;
  r[loc].streamBuffer = 0;
  int tupleNo = 0;
  double size[dim];
  memset(size, 0, sizeof(double)*dim);
  string undefErr = "One bounding box is not defined, "
      "check the data set please.";

  {
    //Scan the stream tuple by tuple
    Word streamTupleWord;
    //  qp->Open(r[loc].streamWord.addr);  
    // (has been moved into the OPEN branch of the value mapping of spatialjoin
    qp->Request(r[loc].streamWord.addr, streamTupleWord);
    r[loc].streamBuffer =
        new TupleBuffer2((qp->GetMemorySize(s) * 1024 * 1024));
    while (qp->Received(r[loc].streamWord.addr))
    {
      Tuple *nextTup = (Tuple*)streamTupleWord.addr;
      r[loc].streamBuffer->AppendTuple(nextTup);
      Rectangle<dim> tBox =
          ((StandardSpatialAttribute<dim>*)nextTup
              ->GetAttribute(attrIndex))->BoundingBox();
      if (!tBox.IsEmpty())
      {
        for(unsigned i = 0; i < dim; i++)
          size[i] += tBox.MaxD(i) - tBox.MinD(i);
        if (!MBR)
          MBR = new Rectangle<dim>(tBox);
        else
          *MBR = tBox.Union(*MBR);
      }
      else
        cerr << undefErr << endl;

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
  r[loc].tb2Iter = r[loc].streamBuffer->MakeScan();
}

template<unsigned dim>
Tuple*
SpatialJoin2LocalInfo<dim>::getNextInputTuple(streamType loc)
{
  return r[loc].tb2Iter->GetNextTuple();
}

template<unsigned dim>
void
SpatialJoin2LocalInfo<dim>::closeInputStream(streamType loc)
{
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
3.5 Value mapping function of operator ~spatialjoin~

*/
template<int D>
int
spatialJoinValueMapping(Word* args, Word& result, int message,
                         Word& local, Supplier s)
{
  typedef LocalInfo< SpatialJoin2LocalInfo<D> > LocalType;
  LocalType* li = static_cast<LocalType*>( local.addr );

  SpatialJoin2LocalInfo<D> *localInfo = 0;

	// cout << "message = " << message << endl;

  switch(message)
  {
    case OPEN:
    {
      if ( li ) {
        delete li;
      }
      li = new LocalType();
      li->ptr = 0;
      local.setAddr(li);

      qp-> Open(args[0].addr);
      qp-> Open(args[1].addr);
 
      return 0;
    }

    case REQUEST:
    {
      if ( li->ptr == 0 )      // first request
      {
        li->ptr = new SpatialJoin2LocalInfo<D>(
            args[0], args[4],
            args[1], args[5],
            args[6], s, li);
        localInfo = li->ptr;
        localInfo->OpenFunction();
      }

      localInfo = li->ptr;

      result.setAddr(localInfo->NextResultTuple());  
      return result.addr != 0 ? YIELD : CANCEL;
    }

    case (1*FUNMSG)+OPEN:{
      localInfo = li->ptr;
      localInfo->openInputStream(leftStream);
      return 0;
    }

    case (2*FUNMSG)+OPEN:{
      localInfo = li->ptr;
      localInfo->openInputStream(rightStream);
      return 0;
    }

    case (1*FUNMSG)+REQUEST:{
      localInfo = li->ptr;
      result.setAddr(localInfo->getNextInputTuple(leftStream));
      if (result.addr) {
        li->readFirst++;
        return YIELD;
      }
      else
        return CANCEL;
    }

    case (2*FUNMSG)+REQUEST:{
      localInfo = li->ptr;
      result.setAddr(localInfo->getNextInputTuple(rightStream));
      if (result.addr) {
        li->readSecond++;
        return YIELD;
      }
      else
        return CANCEL;
    }

    case (1*FUNMSG)+CLOSE:{
      localInfo = li->ptr;
      if ( localInfo != 0 )
      {
        localInfo->closeInputStream(leftStream);
      }
      return 0;
    }

    case (2*FUNMSG)+CLOSE:{
      localInfo = li->ptr;
      if ( localInfo != 0 )
      {
        localInfo->closeInputStream(rightStream);
      }
      return 0;
    }

    // We abuse the fields "read" and "total" in the ProgressLocalInfo to keep 
    // the observed total number of tuples in the first and second input stream,
    // respectively, of the spatialjoin operator. The fields "readFirst" and 
    // "readSecond" are used to maintain the number of tuples already delivered
    // to the parameter function for the first and second stream, respectively.

    case (1*FUNMSG)+REQUESTPROGRESS:{

		// cout << "first parameter stream called" << endl;

      localInfo = li->ptr;
      if ( localInfo != 0 )
      {
        ProgressInfo q1;
        ProgressInfo *q1Res;

        const double wSpatialJoin = 0.014; // msecs per tuple
    
        // This is based on the cost of the query "Roads feed count" which takes
        // 3.15 seconds for 735683 tuples (using machine factor 3.35).
        //
        // (3150 / 735683) * 3.35 = 0.0143438138

        q1Res = (ProgressInfo*) result.addr;

        if ( qp->RequestProgress(args[0].addr, &q1) )
        {
          q1Res->CopySizes(q1);
          q1Res->Card = li->read;
          q1Res->Time = (li->read + 1) * wSpatialJoin;  // must be greater 0
          q1Res->Progress = (li->readFirst + 1) * wSpatialJoin / q1Res->Time;

		// cout << endl << "li->readFirst = " << li->readFirst << endl;

          q1Res->BTime = 0.01;
          q1Res->BProgress = 1.0;
          return YIELD;         
        }
        else return CANCEL;
      }
      else return CANCEL;
    }


    case (2*FUNMSG)+REQUESTPROGRESS:{
		// cout << "second parameter stream called" << endl;
      localInfo = li->ptr;
      if ( localInfo != 0 )
      {
        ProgressInfo q2;
        ProgressInfo *q2Res;

        const double wSpatialJoin = 0.014; // msecs per tuple (see above)

        q2Res = (ProgressInfo*) result.addr;

        if ( qp->RequestProgress(args[1].addr, &q2) )
        {
          q2Res->CopySizes(q2);
          q2Res->Card = li->total;
          q2Res->Time = (li->total + 1) * wSpatialJoin;  // must be greater 0
          q2Res->Progress = (li->readSecond + 1) * wSpatialJoin / q2Res->Time;

		// cout << "li->readSecond = " << li->readSecond << endl;

          q2Res->BTime = 0.01;
          q2Res->BProgress = 1.0;
          return YIELD;          
        }
        else return CANCEL;
      }
      else return CANCEL;
    }


    case CLOSE:
    {
      localInfo = li->ptr;
      if ( localInfo != 0 )
      {
        delete localInfo;
        local.setAddr(0);
      }
      return 0;
    }

    case CLOSEPROGRESS:
      if ( li )
      {
         delete li;
         local.setAddr(0);
      }
      return 0;


    case REQUESTPROGRESS:
    {
      ProgressInfo p1, p2, pFun;
      ProgressInfo *pRes;

      const double uSpatialJoin = 0.27; // msecs per tuple

      // based on a rough calculation 120000 / (2 * 735683) =  0.0815568662
      // as the spatial join on the Roads relation with cardinality
      // 735683 takes roughly two minutes (using machine factor 3.35)

      const double vSpatialJoin = 0.09; // msecs per tuple

      // based on a rough calculation 40000 / (2 * 735683) = 0.0271856221
      // as the sorting phase before parajoin takes roughly 40 seconds
      // on the Roads relation with 735683 tuples (using machine factor 3.35)

       
      pRes = (ProgressInfo*) result.addr;

      if (!li) {
         return CANCEL;
      }

      if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2))
      {
        // Sizes
        li->SetJoinSizes(p1, p2);
        pRes->CopySizes(li);

        if ( li->ptr == 0)
        {
          // Cardinality
          if ( qp->GetSelectivity(s) == 0.1 )
            pRes->Card = max(p1.Card, p2.Card);
          else pRes->Card = qp->GetSelectivity(s) * (p1.Card * p2.Card);

          // Time
          pRes->Time = p1.Time + p2.Time + (p1.Card + p2.Card) * uSpatialJoin;

          // Progress
          pRes->Progress = 
            (p1.Time * p1.Progress +
            p2.Time * p2.Progress)
          / pRes->Time;

          // Blocking Time
          pRes->BTime = p1.Time + p2.Time + (p1.Card + p2.Card) * vSpatialJoin;

          // Blocking Progress
          pRes->BProgress = 
            ( p1.BTime * p1.BProgress + p2.BTime * p2.BProgress)
          / pRes->BTime;
 
          return YIELD;
        }
        else
        {
          if ( qp->RequestProgress(args[6].addr, &pFun) )
          {
            pRes->Card = pFun.Card;
            pRes->Time = p1.Time + p2.Time + pFun.Time;

            pRes->Progress =
              ( p1.Time + p2.Time + pFun.Time * pFun.Progress ) / pRes->Time;

            pRes->BTime = p1.BTime + p2.BTime + pFun.BTime;

            pRes->BProgress =
              ( p1.BTime * p1.BProgress + 
              p2.BTime * p2.BProgress + 
              pFun.BTime * pFun.Progress)
            / pRes->BTime;

            return YIELD;
          }
          else return CANCEL;
        }
      }
      else return CANCEL;
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
3.8 Specification of operator ~spatialjoin~

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
         "spatialjoin",          // name
         spatialJoinSpec,         // specification
         3,                       // number of overloaded functions
         spatialJoinMap,          // value mapping
         spatialJoinSelection,    // trivial selection function
         spatialJoinTypeMap       // type mapping
);


/*
6 Parajoin2

This is a modified version of *parajoin* operator.
The main difference of the new operator is that,
it accepts two separated sorted tuple stream,
and collect tuples have a same key attribute value,
then use the parameter join function to process them.

*/

// struct paraJoin2Info : OperatorInfo
// {
//   paraJoin2Info()
//   {
//     name = "parajoin2";
//     signature =
//        "( (stream(tuple(T1))) x (stream(tuple(T2)))"
//        "x (map (stream(T1)) (stream(T2)) "
//        "(stream(T1 T2))) ) -> stream(tuple(T1 T2))";
//     syntax = "_ _ parajoin2 [ _, _ ; fun]";
//     meaning = "use parameter join function to merge join two "
//              "input sorted streams according to key values.";
//   }
// };


/*
Take another two sorted stream,
then use the parameter function to execute merge-join operation.

----
   ( (stream(tuple((a1 t1) (a2 t2) ... (ai ti) ... (am tm) )))
   x (stream(tuple((b1 p1) (b2 p2) ... (bj tj) ... (bn pn) )))
   x ai x bj
   x ((map (stream((a1 t1) (a2 t2) ... (am tm) ))
           (stream((b1 p1) (b2 p2) ... (bn pn) ))
           (stream((a1 t1) (a2 t2) ... (am tm)
                   (b1 p1) (b2 p2) ... (bn pn)))))  )
   -> stream(tuple((a1 t1) (a2 t2) ... (am tm)
                   (b1 p1) (b2 p2) ... (bn pn)))
----

*/

ListExpr paraJoin2TypeMap(ListExpr args)
{
  if(nl->ListLength(args) == 5)
    {
      NList l(args);
      NList streamA = l.first();
      NList streamB = l.second();
      NList keyA = l.third();
      NList keyB = l.fourth();
      NList mapList = l.fifth();

      string err = "parajoin2 expects "
          "(stream(tuple(T1)) x stream(tuple(T2)) "
          "x string x string "
          "x (map (stream(T1)) (stream(T2)) (stream(T1 T2))))";
      string err1 = "parajoin2 can't found key attribute : ";

      NList attrA;
      if (!streamA.checkStreamTuple(attrA))
        return l.typeError(err);

      NList attrB;
      if (!streamB.checkStreamTuple(attrB))
        return l.typeError(err);

      ListExpr keyAType, keyBType;
      int keyAIndex = listutils::findAttribute(
                                 attrA.listExpr(),
                                 keyA.convertToString(),
                                 keyAType);
      if ( keyAIndex <= 0 )
        return l.typeError(err1 + keyA.convertToString());

      int keyBIndex = listutils::findAttribute(
                                 attrB.listExpr(),
                                 keyB.convertToString(),
                                 keyBType);
      if ( keyBIndex <= 0 )
        return l.typeError(err1 + keyB.convertToString());

      if (!nl->Equal(keyAType, keyBType))
        return l.typeError(
            "parajoin2 expects two key attributes with same type.");

      NList attrResult;
      if (mapList.first().isSymbol(Symbol::MAP())
          && mapList.second().first().isSymbol(Symbol::STREAM())
          && mapList.second().
             second().first().isSymbol(Tuple::BasicType())
          && mapList.third().first().isSymbol(Symbol::STREAM())
          && mapList.third().
             second().first().isSymbol(Tuple::BasicType())
          && mapList.fourth().checkStreamTuple(attrResult)  )
      {
        NList resultStream =
            NList(NList(Symbol::STREAM(),
                        NList(NList(Tuple::BasicType()),
                              attrResult)));

        return NList(NList(Symbol::APPEND()),
                     NList(NList(keyAIndex), NList(keyBIndex)),
                     resultStream).listExpr();
      }
      else
        return l.typeError(err);
    }
    else
    {
      ErrorReporter::ReportError(
        "Operator parajoin expect a list of five arguments");
      return nl->TypeError();
    }
}

#ifndef USE_PROGRESS

// standard version

int paraJoin2ValueMap(Word* args, Word& result,
                int message, Word& local, Supplier s)
{
  pj2LocalInfo* li=0;

  switch(message)
  {
    case OPEN:{
      qp->Open(args[0].addr);
      qp->Open(args[1].addr);

      if (li)
        delete li;
      li = new pj2LocalInfo(args[0], args[1],
                            args[5], args[6],
                            args[4], s);
      local.setAddr(li);
      return 0;
    }
    case REQUEST:{
      if (0 == local.addr)
        return CANCEL;
      li = (pj2LocalInfo*)local.addr;

      result.setAddr(li->getNextTuple());
      if (result.addr)
        return YIELD;
      else
        return CANCEL;
    }
    case (1*FUNMSG)+OPEN:{
      return 0;
    }
    case (2*FUNMSG)+OPEN:{
      return 0;
    }
    case (1*FUNMSG)+REQUEST:{
      if (0 == local.addr)
        return CANCEL;
      li = (pj2LocalInfo*)local.addr;

      result.setAddr(li->getNextInputTuple(tupBufferA));
      if (result.addr)
        return YIELD;
      else
        return CANCEL;
    }
    case (2*FUNMSG)+REQUEST:{
      if (0 == local.addr)
        return CANCEL;
      li = (pj2LocalInfo*)local.addr;

      result.setAddr(li->getNextInputTuple(tupBufferB));
      if (result.addr)
        return YIELD;
      else
        return CANCEL;
    }
    case (1*FUNMSG)+CLOSE:{
      return 0;
    }
    case (2*FUNMSG)+CLOSE:{
      return 0;
    }
    case CLOSE:{
      if (0 == local.addr)
        return CANCEL;
      li = (pj2LocalInfo*)local.addr;

      delete li;
      qp->Close(args[0].addr);
      qp->Close(args[1].addr);
      return 0;
    }
  }

  //should never be here
  return 0;
}

bool pj2LocalInfo::LoadTuples()
{
  bool loaded = false;

  //Clear the buffer
  if (ita){
    delete ita; ita = 0;
  }
  if (tba){
    delete tba; tba = 0;
  }

  if (itb){
    delete itb; itb = 0;
  }
  if (tbb){
    delete tbb; tbb = 0;
   }

  if (moreInputTuples)
  {
    if (cta == 0){
      cta.setTuple(NextTuple(streamA));
    }
    if (ctb == 0){
      
      ctb.setTuple(NextTuple(streamB));
    }
  }
  if ( cta == 0 || ctb == 0)
  {
    //one of the stream is exhausted
    endOfStream = true;
    moreInputTuples = false;
    return loaded;
  }

  int cmp = CompareTuples(cta.tuple, keyAIndex,
                          ctb.tuple, keyBIndex);

  // Assume both streams are ordered by asc
  while(0 != cmp)
  {
    if (cmp < 0)
    {
      //a < b, get more a until a >= b
      while (cmp < 0)
      {
        cta.setTuple(NextTuple(streamA));
        if ( cta == 0 )
        {
          endOfStream = true;
          return loaded;
        }
        cmp = CompareTuples(cta.tuple, keyAIndex,
                            ctb.tuple, keyBIndex);
      }
    }
    else if (cmp > 0)
    {
      //a > b, get more b until a <= b
      while (cmp > 0)
      {
        ctb.setTuple(NextTuple(streamB));
        if ( ctb == 0 )
          {
            endOfStream = true;
            return loaded;
          }
        cmp = CompareTuples(cta.tuple, keyAIndex,
                            ctb.tuple, keyBIndex);
      }
    }
  }

  //Take all tuples from streamA, until the next tuple is bigger
  //than the current one.
  tba = new TupleBuffer(maxMem);
  int cmpa = 0;
  RTuple lta;
  while ( (cta != 0) && (0 == cmpa) )
  {
    lta = cta;
    tba->AppendTuple(lta.tuple);
    cta.setTuple(NextTuple(streamA));
    if ( cta != 0 )
      cmpa = CompareTuples(lta.tuple, keyAIndex,
                           cta.tuple, keyAIndex);
  }

  tbb = new TupleBuffer(maxMem);
  int cmpb = 0;
  RTuple ltb;
  while ( (ctb != 0) && (0 == cmpb) )
  {
    ltb = ctb;
    tbb->AppendTuple(ltb.tuple);
    ctb.setTuple(NextTuple(streamB));
    if ( ctb != 0 )
      cmpb = CompareTuples(ltb.tuple, keyBIndex,
                           ctb.tuple, keyBIndex);
  }
  if ((cta == 0) || (ctb == 0)){
    moreInputTuples = false;
  }

  if ((0 == tba->GetNoTuples()) || (0 == tbb->GetNoTuples()))
  {
    endOfStream = true;
    return loaded;
  }

  ita = tba->MakeScan();
  itb = tbb->MakeScan();
  loaded = true;

  return loaded;
}

int pj2LocalInfo::CompareTuples(Tuple* ta, int kai,
                                Tuple* tb, int kbi)
{
  Attribute* a = static_cast<Attribute*>(ta->GetAttribute(kai));
  Attribute* b = static_cast<Attribute*>(tb->GetAttribute(kbi));

  if (!a->IsDefined() || !b->IsDefined()){
    cerr << "Undefined Tuples are contained." << endl;
    return -1;
  }

  int cmp = a->Compare(b);
  return cmp;
}

Tuple* pj2LocalInfo::getNextTuple()
{
  Word funResult(Address(0));

  while(!endOfStream)
  {
    qp->Request(pf, funResult);
    if (funResult.addr)
      return (Tuple*)funResult.addr;
    else if (endOfStream)
    {
      qp->Close(pf);
      return 0;
    }
    else
    {
      qp->Close(pf);
      if (LoadTuples())
        qp->Open(pf);
    }
  }

  return 0;
}

#else

// with support for progress queries


int paraJoin2ValueMap(Word* args, Word& result,
                int message, Word& local, Supplier s)
{
  pj2LocalInfo* li;
  li = (pj2LocalInfo*)local.addr;

  switch(message)
  {
    case OPEN:{
      qp->Open(args[0].addr);
      qp->Open(args[1].addr);

      if (li)
        delete li;
      li = new pj2LocalInfo(args[0], args[1],
                            args[5], args[6],
                            args[4], s);
      local.setAddr(li);
      return 0;
    }

    case REQUEST:{
      if (0 == local.addr)
        return CANCEL;

      result.setAddr(li->getNextTuple());
      if (result.addr)
      {
        li->returned++;
        return YIELD;
      }
      else
        return CANCEL;
    }

    case (1*FUNMSG)+OPEN:{
      return 0;
    }

    case (2*FUNMSG)+OPEN:{
      return 0;
    }

    case (1*FUNMSG)+REQUEST:{
      if (0 == local.addr)
        return CANCEL;

      result.setAddr(li->getNextInputTuple(tupBufferA));
      if (result.addr)
        return YIELD;
      else
        return CANCEL;
    }

    case (2*FUNMSG)+REQUEST:{
      if (0 == local.addr)
        return CANCEL;

      result.setAddr(li->getNextInputTuple(tupBufferB));
      if (result.addr)
        return YIELD;
      else
        return CANCEL;
    }

    case (1*FUNMSG)+CLOSE:{
      return 0;
    }

    case (2*FUNMSG)+CLOSE:{
      return 0;
    }

    case CLOSE:{
      if (0 == local.addr)
        return CANCEL;

      qp->Close(args[0].addr);
      qp->Close(args[1].addr);
      return 0;
    }


    case CLOSEPROGRESS:
      if ( li )
      {
         delete li;
         local.setAddr(0);
      }
      return 0;


    case REQUESTPROGRESS:
    {
      ProgressInfo p1, p2;
      ProgressInfo *pRes;
      const double uParajoin = 0.1742;  // msecs per input tuple
        // assuming that the parameter function is 
        // parajoin2[Cell_r1, Cell_r2
        // ; . .. realJoinMMRTreeVec[Box_r1, Box_r2, 10, 20]
        //   filter[.osm_id_r1 < .osm_id_r2]
        //   filter[gridintersects(5.5, 50.0, 0.2, 0.2, 50, 
        //     .Box_r1, .Box_r2, .Cell_r1)] ] 
        // taken from the nrw Roads relation.

      // see determination of progress constants in file ProgressConstants.txt
       

      pRes = (ProgressInfo*) result.addr;

      if (!li) {
         return CANCEL;
      }

      if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2))
      {
        // Sizes
        li->SetJoinSizes(p1, p2);
        pRes->CopySizes(li);

	// Cardinality
        if ( li->returned > enoughSuccessesJoin )
          pRes->Card = ((double) li->returned + 1) * p1.Card
            /  ((double) li->readFirst + 1);   
        else
          if ( qp->GetSelectivity(s) == 0.1 )
            pRes->Card = max(p1.Card, p2.Card);
          else pRes->Card = qp->GetSelectivity(s) * (p1.Card * p2.Card);

        // Time
        pRes->Time = p1.Time + p2.Time + 
          p1.Card * uParajoin +
          p2.Card * uParajoin;

        // Progress
        pRes->Progress = 
          ( p1.Time * p1.Progress +
          p2.Time * p2.Progress +
          li->readFirst * uParajoin +
          li->readSecond * uParajoin )
          / pRes->Time;
          
        // Blocking Progress
        pRes->CopyBlocking(p1, p2);  //non-blocking operator;

        return YIELD;
      } else {
        return CANCEL;
      }
    }


  }
  //should never be here
  return 0;
}

bool pj2LocalInfo::LoadTuples()
{
  bool loaded = false;

  //Clear the buffer
  if (ita){
    delete ita; ita = 0;
  }
  if (tba){
    delete tba; tba = 0;
  }

  if (itb){
    delete itb; itb = 0;
  }
  if (tbb){
    delete tbb; tbb = 0;
   }

  if (moreInputTuples)
  {
    if (cta == 0){
      cta.setTuple(NextTuple(streamA));
      if ( cta != 0 ) readFirst++;
    }
    if (ctb == 0){
      ctb.setTuple(NextTuple(streamB));
      if ( ctb != 0 ) readSecond++;
    }
  }
  if ( cta == 0 || ctb == 0)
  {
    //one of the streams is exhausted
    endOfStream = true;
    moreInputTuples = false;
    return loaded;
  }

  int cmp = CompareTuples(cta.tuple, keyAIndex,
                          ctb.tuple, keyBIndex);

  // Assume both streams are ordered by asc
  while(0 != cmp)
  {
    if (cmp < 0)
    {
      //a < b, get more a until a >= b
      while (cmp < 0)
      {
        cta.setTuple(NextTuple(streamA));
        if ( cta != 0 ) readFirst++;
        else // ( cta == 0 )
        {
          endOfStream = true;
          return loaded;
        }
        cmp = CompareTuples(cta.tuple, keyAIndex,
                            ctb.tuple, keyBIndex);
      }
    }
    else if (cmp > 0)
    {
      //a > b, get more b until a <= b
      while (cmp > 0)
      {
        ctb.setTuple(NextTuple(streamB));
        if ( ctb != 0 ) readSecond++;
        else // ( ctb == 0 )
          {
            endOfStream = true;
            return loaded;
          }
        cmp = CompareTuples(cta.tuple, keyAIndex,
                            ctb.tuple, keyBIndex);
      }
    }
  }

  //Take all tuples from streamA, until the next tuple is bigger
  //than the current one.
  tba = new TupleBuffer(maxMem);
  int cmpa = 0;
  RTuple lta;
  while ( (cta != 0) && (0 == cmpa) )
  {
    lta = cta;
    tba->AppendTuple(lta.tuple);
    cta.setTuple(NextTuple(streamA));
    if ( cta != 0 ) 
    {
      readFirst++;
      cmpa = CompareTuples(lta.tuple, keyAIndex,
                           cta.tuple, keyAIndex);
    }
  }

  tbb = new TupleBuffer(maxMem);
  int cmpb = 0;
  RTuple ltb;
  while ( (ctb != 0) && (0 == cmpb) )
  {
    ltb = ctb;
    tbb->AppendTuple(ltb.tuple);
    ctb.setTuple(NextTuple(streamB));
    if ( ctb != 0 )
    {
      readSecond++;
      cmpb = CompareTuples(ltb.tuple, keyBIndex,
                           ctb.tuple, keyBIndex);
    }
  }
  if ((cta == 0) || (ctb == 0)){
    moreInputTuples = false;
  }

  if ((0 == tba->GetNoTuples()) || (0 == tbb->GetNoTuples()))
  {
    endOfStream = true;
    return loaded;
  }

  ita = tba->MakeScan();
  itb = tbb->MakeScan();
  loaded = true;

  return loaded;
}

int pj2LocalInfo::CompareTuples(Tuple* ta, int kai,
                                Tuple* tb, int kbi)
{
  Attribute* a = static_cast<Attribute*>(ta->GetAttribute(kai));
  Attribute* b = static_cast<Attribute*>(tb->GetAttribute(kbi));

  if (!a->IsDefined() || !b->IsDefined()){
    cerr << "Undefined Tuples are contained." << endl;
    return -1;
  }

  int cmp = a->Compare(b);
  return cmp;
}

Tuple* pj2LocalInfo::getNextTuple()
{
  Word funResult(Address(0));

  while(!endOfStream)
  {
    qp->Request(pf, funResult);
    if (funResult.addr)
      return (Tuple*)funResult.addr;
    else if (endOfStream)
    {
      qp->Close(pf);
      return 0;
    }
    else
    {
      qp->Close(pf);
      if (LoadTuples())
        qp->Open(pf);
    }
  }

  return 0;
}



#endif

/*
2.19.3 Specification of operator ~parajoin2~

*/
const string parajoin2Spec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "

  "( <text>( (stream(tuple(T1))) x (stream(tuple(T2)))"
  "x attrT1 x attrT2 x "
  "( stream(tuple(T1))) x (stream(tuple(T2))) -> "
  "stream(tuple(T1 T2)) ) -> stream(tuple(T1 T2))</text--->"

  "<text>_ _ parajoin2 [_, _; fun ]</text--->"

  "<text>use parameter join function to merge join two "
  "input sorted streams according to key values.</text--->"

  "<text>query plz feed sortby[Ort] {p1} "
  " plz feed sortby[Ort] {p2} "
  " parajoin2[Ort_p1, Ort_p2; . .. product] count</text--->"

  ") )";

/*
2.19.4 Definition of operator ~parajoin2~

*/
Operator parajoin2 (
         "parajoin2",                // name
         parajoin2Spec,              // specification
         paraJoin2ValueMap,          // value mapping
         Operator::SimpleSelect,     // trivial selection function
         paraJoin2TypeMap            // type mapping
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
      spatialjoin2.SetUsesMemory();
    AddOperator(&parajoin2);
      parajoin2.SetUsesMemory();

    // AddOperator(paraJoin2Info(),
    //    paraJoin2ValueMap, paraJoin2TypeMap);

#ifdef USE_PROGRESS
// support for progress queries
    spatialjoin2.EnableProgress();
    parajoin2.EnableProgress();
#endif

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

