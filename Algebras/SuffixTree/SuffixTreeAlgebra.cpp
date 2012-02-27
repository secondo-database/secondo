/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2009, University in Hagen, Faculty of Mathematics and
 Computer Science, Database Systems for New Applications.

 SECONDO is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published
 by
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

 01590 Fachpraktikum "Erweiterbare Datenbanksysteme" 
 WS 2011 / 2012

 Svenja Fuhs
 Regine Karg
 Jan Kristof Nidzwetzki
 Michael Teutsch 
 C[ue]neyt Uysal


 //paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
 //paragraph [10] Footnote: [{\footnote{] [}}]
 //[TOC] [\tableofcontents]

 [TOC]

 0 Overview

 1 Includes and defines

*/
#include <cstdlib>
#include <queue>
#include <map>

#include <string>

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "ListUtils.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "Attribute.h"
#include "../../Tools/Flob/Flob.h"
#include "SuffixTreeAlgebra.h"
#include "FTextAlgebra.h"
#include "Stream.h"

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;


/*
~A Macro useful for debugging ~

*/

//#define __TRACE__ cout << __FILE__ << "@" << __LINE__ << endl;
#define __TRACE__



//namespace to avoid name conflicts
namespace sta
{
/*
 2.1 Constructor ~SuffixTree~

*/
SuffixTree::SuffixTree(SuffixTreeVertex* rootVertex) :
  Attribute(true), mSuffixTree(0), mSuffixIndex(0), mInput(0), mMemoryTree(0)
{
  SaveToPersistent(rootVertex);
}

SuffixTree::SuffixTree(bool def) :
  Attribute(def), mSuffixTree(0), mSuffixIndex(0), mInput(0), mMemoryTree(0)
{
}

/*
 2.2 Destructor ~SuffixTree~

*/
SuffixTree::~SuffixTree()
{
  if(mMemoryTree != NULL) {
    delete mMemoryTree;
    mMemoryTree = NULL;
  }
}

/*
 2.4 Destructor ~SuffixTree~

*/
SuffixTreeVertex* SuffixTree::GetInMemoryTree()
{
  if( mMemoryTree == NULL) {
    mMemoryTree = SuffixTree::LoadFromPersistent();
  }

  return mMemoryTree;
}

/*
 2.3 Equal Function
 To compare two SuffixTrees.
 The result is true if the text of two SuffixTrees is exactly the same.

*/
bool SuffixTree::Equal(Attribute* arg) const
{
  bool res = false;
  SuffixTree* argumentTree = static_cast<SuffixTree*> (arg);

  if (!IsDefined() || !argumentTree->IsDefined())
  {
    return false;
  }
  //get the flob of the suffixtree argument which has the text
  Flob* textFlobArgument = argumentTree->GetFLOB(2);
  size_t flobLengthArgument = textFlobArgument->getSize();
  //read the flob and put the characters into an char array
  char *charArrayArgument = (char *) malloc(flobLengthArgument);
  textFlobArgument->read(charArrayArgument, flobLengthArgument, 0);
  
  // current suffixtree
  size_t flobLengthCurrent = mInput.getSize();

  char *charArrayCurrent = (char *) malloc(flobLengthCurrent);
  mInput.read(charArrayCurrent, flobLengthCurrent, 0);

  //if both text legths are equal compare them by characters
  if (flobLengthArgument == flobLengthCurrent)
  {
    for (size_t pos = 0; pos < flobLengthArgument - 1; pos++)
    {
      if (*(charArrayArgument + pos) == *(charArrayCurrent + pos))
      {
        res = true;
      }
      else
      {
        res = false;
        break;
      }
    }
  }
  
  free(charArrayArgument);
  free(charArrayCurrent);

  return res;
}

/*
 3. Attribute functions

*/

/*
 3.1 ~HashValue~ function
 Using the Algorithm idea from Robert Sedgwicks.

*/
size_t SuffixTree::HashValue() const
{
  unsigned int b = 378551;
  unsigned int a = 63689;
  unsigned int hash = 0;

  if (!IsDefined())
  {
    return hash;
  }
  size_t textLength = mInput.getSize();
  char *charArray = (char *) malloc(textLength);
  mInput.read(charArray, textLength, 0);

  for (size_t pos = 0; pos < textLength - 1; pos++)
  {
    hash = hash * a + *(charArray + pos);
    a = a * b;
  }

  free(charArray);
  return hash;
}

/*
 3.2 Copy

*/
void SuffixTree::CopyFrom(const Attribute* arg)
{
  const SuffixTree *st = static_cast<const SuffixTree*> (arg);
  SetDefined(st->IsDefined());
  this->mInput.copyFrom(st->mInput);
  this->mSuffixTree.copyFrom(st->mSuffixTree);
  this->mSuffixIndex.copyFrom(st->mSuffixIndex);
}

/*
 3.3 ~Compare~ function
 Compares two SuffixTrees
 Defines an order on SuffixTrees: shorter text is smaller than longer text,
 if both texts have the same length, the text which is lexicographical first
 is the smaller one.

*/
int SuffixTree::Compare(const Attribute* arg) const
{
  int result = 0;
  const SuffixTree* constArgumentSuffixTree =
      static_cast<const SuffixTree*> (arg);
  //const_cast to be able to use GetFLOB() function
  SuffixTree* argumentSuffixTree =
      const_cast<SuffixTree*> (constArgumentSuffixTree);

  if (!IsDefined())
  {
    if (argumentSuffixTree->IsDefined())
    {
      result = -1;
    }
    else
    {
      result = 0;
    }
  }
  else if (argumentSuffixTree->IsDefined())
  {
    result = 1;
  }
  // both are defined
  //both are equal
  if (Equal(argumentSuffixTree))
  {
    result = 0;
  }
  else
  {
    //not equal, char Values equal
    //the shorter text is smaller

    //argument SuffixTree
    Flob* argumentTextFlob = argumentSuffixTree->GetFLOB(2);
    size_t argumentFlobLength = argumentTextFlob->getSize();
    char *charArrayArgument = (char *) malloc(argumentFlobLength);
    argumentTextFlob->read(charArrayArgument, argumentFlobLength, 0);

    // current SuffixTree
    size_t currentFlobLength = mInput.getSize();
    char *charArrayCurrent = (char *) malloc(currentFlobLength);
    mInput.read(charArrayCurrent, currentFlobLength, 0);
    //add all char Values as integers

    if (currentFlobLength < argumentFlobLength)
    {
      result = -1;
    }
    else if (currentFlobLength > argumentFlobLength)
    {
      result = 1;
    }
    else
    {
      //both text lengths are equal
      //lexicographical
      for (size_t pos = 0; pos < currentFlobLength; pos++)
      {
        if (*(charArrayCurrent + pos) != *(charArrayArgument + pos))
        {
          if (*(charArrayCurrent + pos) < *(charArrayArgument + pos))
          {
            result = -1;
          }
          else
          {
            result = 1;
          }
          break;
        }
      }
    }

    free(charArrayCurrent);
    free(charArrayArgument);
  }
  return result;
}

/*
 3.4 ~Adjacent~ function

*/
bool SuffixTree::Adjacent(const Attribute * arg) const
{
  return false;
}

/*
 3.5 ~Sizeof~ functions

*/
size_t SuffixTree::Sizeof() const
{
  return sizeof(*this);
}

int SuffixTree::SizeOfSuffixTree()
{
  return sizeof(SuffixTree);
}

/*
 3.6 ~Clone~ functions

 Returns a new created SuffixTree (clone) which is a
 copy of ~this~.

*/
SuffixTree* SuffixTree::Clone() const
{
  SuffixTree *newSuffixTree = new SuffixTree(true);
  newSuffixTree->CopyFrom(this);
  return newSuffixTree;
}

Word SuffixTree::CloneSuffixTree(const ListExpr typeInfo, const Word& w)
{
  return SetWord(((SuffixTree*) w.addr)->Clone());

}
/*
 3.7 ~Cast~ function

*/
void* SuffixTree::Cast(void* addr)
{
  return new (addr) SuffixTree;
}

/*
 3.8 ~NumOfFlobs~

*/
inline int SuffixTree::NumOfFLOBs() const
{
  return 3;
}

/*
 3.9 GetFlob

*/
inline Flob *SuffixTree::GetFLOB(const int i)
{
  assert( i >= 0 && i < NumOfFLOBs() );

  if (i == 0)
  {
    return &mSuffixTree;
  } else if(i == 1) {
    return &mSuffixIndex;
  }

  return &mInput;
}

/*
 4. Additional functions

*/

/*
 4.1 Print

*/
ostream& SuffixTree::Print(ostream &os) const
{
  os << "SuffixTree: ";
  if (IsDefined())
  {
    os << "DEFINED" << endl;
  }
  else
  {
    os << "UNDEFINED." << endl;
  }
  return os;
}

/*
 5. List Representation

*/

/*
 5.1 ~Out~-Function

*/
ListExpr SuffixTree::Out(ListExpr typeInfo, Word value)
{
  SuffixTree* suffixtree = static_cast<SuffixTree*> (value.addr);
  ListExpr result;
  if (suffixtree == NULL)
  {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  if (!suffixtree->IsDefined())
  {
    result = nl->SymbolAtom(Symbol::UNDEFINED());
  }
  else
  {
    SuffixTreeVertex *vertex = suffixtree -> LoadFromPersistent();
    ListExpr text = nl->TextAtom();

    // force complete loading of our tree
    vertex -> GetNumberOfLeaves();

    nl->AppendText(text, *(vertex -> GetInput()));
    ListExpr tree = SuffixTreeVertex::CreateListExprFromSuffixtree(vertex);
    result = nl->TwoElemList(tree, text);
    delete vertex;
  }
  return result;
}

/*
 5.2 ~In~-Function

*/
Word SuffixTree::In(const ListExpr typeInfo, const ListExpr instance,
    const int errorPos, ListExpr& errorInfo, bool& correct)
{
  Word w = SetWord(Address(0));
  SuffixTree* suffixtree = new SuffixTree(false);
  correct = false;
  if (nl->ListLength(instance) == 2)
  {
    ListExpr treeLE = nl->First(instance);
    ListExpr text = nl->Second(instance);
    Word t = ftext::InFText(typeInfo, text, errorPos, errorInfo, correct);
    FText* newFText = static_cast<FText*> (t.addr);
    if (newFText->IsDefined())
    {
      try
      {
        // Create our RootVertex
        SuffixTreeVertex* vertex =
            SuffixTreeVertex::CreateSuffixtreeFromListExpr(treeLE,
                new string(newFText->GetValue()));

        delete suffixtree;
        suffixtree = new SuffixTree(vertex);

        // Delete our im memory tree 
        delete vertex;
        vertex = NULL;

        correct = true;
      } catch (string& s)
      {
        cmsg.inFunError("The first element is not a suffixtree.");
      }
    }
    else
    {
      cmsg.inFunError("Undefined ftext");
    }

    // cleanup
    delete newFText;
    newFText = NULL;
  }
  else
  {
    cmsg.inFunError(
        "Expecting two elements! First a suffixtree and then a ftext");
  }
  w.addr = suffixtree;
  return w;
}

/*
 5.3 ~Create~-function

*/
Word SuffixTree::Create(const ListExpr typeInfo)
{
  return SetWord(new SuffixTree(false));
}

/*
 5.4 ~KindCheck~ Kind Checking Function

 This function checks whether the type constructor is applied
 correctly. Since type constructor ~SuffixTree~ does not have arguments,
 this is trivial.

*/
bool SuffixTree::KindCheck(ListExpr type, ListExpr& errorInfo)
{
  return (nl->IsEqual(type, SuffixTree::BasicType()));
}

/*
 5.5 ~Close~-function

*/
void SuffixTree::Close(const ListExpr typeInfo, Word& w)
{
  delete static_cast<SuffixTree*> (w.addr);
  w.addr = 0;
}

/*
 5.6 ~Delete~-function

*/
void SuffixTree::Delete(const ListExpr typeInfo, Word& w)
{
  delete static_cast<SuffixTree*> (w.addr);
  w.addr = 0;
}

/*
 5.7 ~Open~-function

*/
bool SuffixTree::Open( SmiRecord& valueRecord, size_t& offset,
     const ListExpr typeInfo, Word& value )
{
  SuffixTree *p = (SuffixTree*)Attribute::Open( valueRecord, offset, typeInfo );
  value.setAddr( p );
  return true;
}

/*
 5.5 Create a persistent representation of our
 transient SuffixTree

*/
void SuffixTree::SaveToPersistent(SuffixTreeVertex* vertex)
{
  // persist text
  mInput.clean();
  const char* newString = vertex -> GetInput() -> c_str();
  if (newString != NULL)
  {
    size_t sz = strlen(newString) + 1;
    if (sz > 0)
    {
      assert(newString[sz-1]==0);
      mInput.write(newString, sz);
    }
    else
    {
      char d = 0;
      mInput.write(&d, 1);
    }
  }

  // Persist vertex
  mSuffixTree.clean();
  mSuffixIndex.clean();

  // The suffixtree is processed by breadth-first-traversal. If a node has not
  // yet been processed, a pointer on this node is stored in vertexQueue.
  queue<SuffixTreeVertex*> vertexQueue;
  SuffixTreeVertex* currentVertex;
  SuffixTreeEdge* currentEdge;
  
  size_t vertexId = 0;
  size_t edgeCounter = 0;

  // calculate vertexIds
  vertexQueue.push(vertex);
  while (!vertexQueue.empty())
  {
    currentVertex = vertexQueue.front();
    vertexQueue.pop();

    currentVertex -> SetVertexId(vertexId);
    
    for (size_t edgeNo = 0; edgeNo < currentVertex->GetEdgeCount(); edgeNo++)
    {
      currentEdge = currentVertex->GetEdgeAt(edgeNo);

      if (currentEdge->GetChild() != NULL)
      {
        vertexQueue.push(currentEdge->GetChild());
      }
    }
  
    vertexId++;
  }

  // Persist data
  vertexQueue.push(vertex);
  while (!vertexQueue.empty())
  {
    currentVertex = vertexQueue.front();
    vertexQueue.pop();
    // For each node, first the number of outgoing edges is stored,
    // followed by the respective edges' start and end values.
    mSuffixTree.Append(currentVertex -> GetVertexId());
    mSuffixTree.Append(currentVertex->GetEdgeCount());

    for (size_t edgeNo = 0; edgeNo < currentVertex->GetEdgeCount(); edgeNo++)
    {
      currentEdge = currentVertex->GetEdgeAt(edgeNo);
      mSuffixTree.Append(currentEdge->GetStartIndex());
      mSuffixTree.Append(currentEdge->GetEndIndex());
      
      if (currentEdge->GetChild() != NULL)
      {
        // The currently processed edge has a subsequent node. A pointer
        // on this node is stored in vertexQueue.
        vertexQueue.push(currentEdge->GetChild());
        mSuffixTree.Append(currentEdge->GetChild()->GetVertexId());
      } else {
        // Leaf, so our vertex id is 0
        mSuffixTree.Append(0);
      }
      
      edgeCounter++;
    }
   
    // build index
    mSuffixIndex.Append(edgeCounter);
  }
}


/*
 5.6 Create a transient representation of our
 persistent SuffixTree

*/
SuffixTreeVertex* SuffixTree::LoadFromPersistent()
{
  string *OurText = new string();
  OurText -> resize(mInput.getSize() + 2, '\0');

  //delete[] s;
  //s = NULL;

  // Load only child of the root node
  // the next elements are loaded on demand through our 
  // SuffixTreeLoader
  SuffixTreeVertex *vertex = new SuffixTreeVertex(OurText);
  SuffixTreeLoader *loader = 
    new SuffixTreeLoader(&mSuffixTree, &mSuffixIndex, &mInput);
  vertex -> SetLoader(loader);
  
  // load persisted vertex
  SuffixTreeVertex* currentVertex = vertex;
  size_t offset = 0, vertexId = 0, edgeNo = 0;
  
  // offset stores the index on the next node to be processed. The
  // element in mSuffixtree[offset] stores the node's number of
  // outgoing edges.
    mSuffixTree.Get(offset++, vertexId);
    mSuffixTree.Get(offset++, edgeNo);
    
    currentVertex -> SetVertexId(vertexId);

    for (size_t edgeIndex = 0; edgeIndex < edgeNo; edgeIndex++)
    {
      size_t start, end, destVertexId;
      //mSuffixTree.Get((int) offset + (3 * edgeIndex) + 2, start);
      mSuffixTree.Get((int) offset++, start);
      mSuffixTree.Get((int) offset++, end);
      mSuffixTree.Get((int) offset++, destVertexId);
  
      loader -> LoadTextForIndex(vertex, start, start);

      SuffixTreeEdge* newEdge = new SuffixTreeEdge(start, end);
      currentVertex->InsertEdge(newEdge);
     
      // Inner edge?
      if (destVertexId != 0)
      {
        SuffixTreeVertex* newVertex = new SuffixTreeVertex(OurText);
        newVertex -> SetVertexId(destVertexId);
        newEdge->SetChild(newVertex);
      }
    }
    
    return vertex;
}
/*
 5.7 ~SuffixTreeInfo~ Type Description
 done by implementing a subclass of ~ConstructorInfo~.

*/
struct SuffixTreeInfo: ConstructorInfo
{

  SuffixTreeInfo()
  {
    //example babac$
    name = SuffixTree::BasicType();
    signature = "-> " + Kind::DATA();
    typeExample = SuffixTree::BasicType();
    listRep = "(<suffixtree><text>)";
    // example text is babac
    valueExample = "(((5 5 ())(1 1 (( 2 5 ())(4 5 ())))"
      "(0 1 (( 2 5 ())(4 5 ())))(4 5 ()))"
      "(4 5 ()))text)";
    remarks = "first value has to be a SuffixTree, second value"
      " from type Text.";
  }
};

/*
 5.8 Creation of the Type Constructor Instance

 template class
 ~ConstructorFunctions~ which will create many default implementations of
 functions used by a Secondo type.

*/
struct SuffixTreeFunctions: ConstructorFunctions<SuffixTree>
{

  SuffixTreeFunctions()
  {
    // re-assign some function pointers
    create = SuffixTree::Create;
    deletion = SuffixTree::Delete;
    open = SuffixTree::Open; 
    save = SaveAttribute<SuffixTree> ;
    in = SuffixTree::In;
    out = SuffixTree::Out;
    close = SuffixTree::Close;
    clone = SuffixTree::CloneSuffixTree;
    cast = SuffixTree::Cast;
    sizeOf = SuffixTree::SizeOfSuffixTree;
    kindCheck = SuffixTree::KindCheck;
    // the default implementations for open and save are only
    // suitable for a class which is derived from class ~Attribute~, hence
    // open and save functions must be overwritten here.
  }
};

SuffixTreeInfo suffInfo;
SuffixTreeFunctions suffFunctions;
TypeConstructor SuffixTreeTC(suffInfo, suffFunctions);

/*
 6 Creating Operators

 6.1 Type Mapping Functions

 A type mapping function checks whether the correct argument types are supplied
 for an operator; if so, it returns a list expression for the result type,
 otherwise the symbol ~typeerror~.

 6.1.1 ~createSuffixTreeTypeMap~ type mapping for create function

*/
ListExpr createSuffixTreeTypeMap(ListExpr args)
{
  ListExpr arg;
  string nlchrs;
  if (nl->ListLength(args) == 1)
    {
      arg = nl->First(args);
      if (nl->IsEqual(arg, typeName))
      {
        return NList(SuffixTree::BasicType()).listExpr();
      }
      else
      {
        return NList::typeError("Expecting a text from type FText");
      }
    }
    else
    {
      return NList::typeError("One argument expected.");
    }

}

/*
 6.1.2  ~patternOccursTypeMap~

*/
ListExpr patternOccursTypeMap(ListExpr args)
{
  NList type(args);
  const string errMsg = "Expecting a SuffixTree and a text from type FText ";

  //  SuffixTree x text  -> bool
  if (type == NList(SuffixTree::BasicType(), FText::BasicType()))
  {
    return NList(CcBool::BasicType()).listExpr();
  }
  return NList::typeError(errMsg);
}

/*
 6.1.3  ~patternPositionsTypeMap~

*/
ListExpr patternPositionsTypeMap(ListExpr args)
{
  NList type(args);
  const string errMsg = "Expecting a SuffixTree and a text from type FText ";

  //  SuffixTree x text  -> stream(int)
  if (type == NList(SuffixTree::BasicType(), FText::BasicType()))
  {
    return nl->TwoElemList(nl->SymbolAtom(Stream<CcInt>::BasicType()),
        nl->SymbolAtom(CcInt::BasicType()));
  }
  return NList::typeError(errMsg);
}

/*
 6.1.4  ~patternCountTypeMap~

*/
ListExpr patternCountTypeMap(ListExpr args)
{
  NList type(args);
  const string errMsg = "Expecting a SuffixTree and a text from type FText ";

  //  SuffixTree x text  -> int
  if (type == NList(SuffixTree::BasicType(), FText::BasicType()))
  {
    return NList(CcInt::BasicType()).listExpr();
  }
  return NList::typeError(errMsg);
}

/*

 6.1.5  ~longestRepeatedSubstringTypeMap~

*/
ListExpr suffixtree_textstreamTypeMap(ListExpr args)
{
  ListExpr resultType;
  if (nl->ListLength(args) == 1)
  {
    if (nl->IsEqual(nl->First(args), SuffixTree::BasicType()))
    {
      resultType = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
          nl->SymbolAtom(FText::BasicType()));
    }
    else
    {
      resultType = NList::typeError("Expecting a SuffixTree");
    }
  }
  else
  {
    resultType = NList::typeError("One argument expected.");
  }
  return resultType;
}



/*
 6.1.7  ~longestCommonSubstringTypeMap~

*/
ListExpr longestCommonSubstringTypeMap(ListExpr args)
{
  ListExpr resultType;
    if (nl->ListLength(args) == 2)
    {
      if (nl->IsEqual(nl->First(args), FText::BasicType()) &&
          nl->IsEqual(nl->Second(args), FText::BasicType()))
      {
        resultType = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
            nl->SymbolAtom(FText::BasicType()));
      }
      else
      {
        resultType = NList::typeError("two arguments of type FText expected.");
      }
    }
    else
    {
      resultType = NList::typeError("two arguments of type FText expected.");
    }
    return resultType;
}

/*
 6.1.8  ~maximalUniqueMatchesTypeMap~

*/
ListExpr maximalUniqueMatchesTypeMap(ListExpr args)
{
  ListExpr resultType;
    if (nl->ListLength(args) == 2)
    {
      if (nl->IsEqual(nl->First(args), FText::BasicType()) &&
          nl->IsEqual(nl->Second(args), FText::BasicType()))
      {
        resultType = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
            nl->SymbolAtom(FText::BasicType()));
      }
      else
      {
        resultType = NList::typeError("two arguments of type FText expected.");
      }
    }
    else
    {
      resultType = NList::typeError("two arguments of type FText expected.");
    }
    return resultType;
}

/*
 6.1.9  ~circularStringLinearisationTypeMap~

*/
ListExpr circularStringLinearizationTypeMap(ListExpr args)
{
  ListExpr resultType;
    if (nl->ListLength(args) == 1)
    {
      if (nl->IsEqual(nl->First(args), FText::BasicType()))
      {


        // text -> stream(tuple(linstr: text, pos: int))
        return nl->TwoElemList(nl->SymbolAtom(Stream<Tuple>::BasicType()),
                   nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                   nl->TwoElemList(
                      nl->TwoElemList(nl->SymbolAtom("linstr"),
                           nl->SymbolAtom(FText::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("pos"),
                           nl->SymbolAtom(CcInt::BasicType()))
                   )));

      }
      else
      {
        resultType = NList::typeError("one argument of type FText expected.");
      }
    }
    else
    {
      resultType = NList::typeError("one argument of type FText expected.");
    }
    return resultType;
}

/*
 6.1.10  ~kMismatchTypeMap~

*/
ListExpr kMismatchTypeMap(ListExpr args)
{
  NList type(args);
  const string errMsg = "Expecting a SuffixTree, a text from type FText"
      "and an int";

  //  SuffixTree x text x int -> stream(text)
  if (type == NList(SuffixTree::BasicType(),FText::BasicType(),
      CcInt::BasicType()))
  {
    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
              nl->SymbolAtom(FText::BasicType()));
  }
  return NList::typeError(errMsg);
}

/*
 6.1.10  ~DeleteTerminalSymbolFromText~

*/
void DeleteTerminalSymbolFromText(string& text)
{
  size_t pos = 0;
  pos = text.find(terminationCharacter);
  while (pos != string::npos)
  {
    text.erase(pos, 1);
    pos = text.find(terminationCharacter, pos);
  }
}

/*
 6.2 Value Mapping Functions

 6.2.1 ~createSuffixTreeFunction~ value mapping for create function

 This function creates a suffixtree from a text with the Ukkonen algorithm
 in linear time.

*/
int createSuffixTreeFunction(Word* args, Word& result, int message,
    Word& local, Supplier s)
{
  result = qp->ResultStorage(s);

  FText* ftext = static_cast<FText*> (args[0].addr);
  string text = ftext->GetValue();
  DeleteTerminalSymbolFromText(text);
  text = text + terminationCharacter;
  
  SuffixTreeVertex* tempTree = 
    UkkonenTreeBuilder::CreateSuffixTree(new string(text));

  if (tempTree != NULL)
  {
    SuffixTree *res = static_cast<SuffixTree*> (result.addr);
    res -> SaveToPersistent(tempTree);
    res -> SetDefined(true);

    // Clean up
    delete tempTree;
    tempTree = NULL;
  }

  return 0;
}

/*
 6.2.1 ~createSuffixTreeFunction~ value mapping for create function
         with quadratic running time

*/
int createSuffixTreeFunction_quadratic(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);

  FText* ftext = static_cast<FText*> (args[0].addr);
  string text = ftext->GetValue();
  DeleteTerminalSymbolFromText(text);
  text = text + terminationCharacter;
  SuffixTreeVertex* tempTree =
      SimpleTreeBuilder::CreateSuffixTree(new string(text));

  if (tempTree != NULL)
  {
    SuffixTree *res = static_cast<SuffixTree*> (result.addr);
    res -> SaveToPersistent(tempTree);
    res -> SetDefined(true);

    // Clean up
    delete tempTree;
    tempTree = NULL;
  }

  return 0;
}

/*
 6.2.2 ~patternOccursFunction~
 Returns true if the pattern p occurs in text s, else returns false.
 The function decides the result in O(|p|).

*/
int patternOccursFunction(Word* args, Word& result, int message, Word& local,
    Supplier s)
{
  bool defined = true;
  bool occurs = false;

  SuffixTree* tree = static_cast<SuffixTree*>( args[0].addr );

  FText *text = static_cast<FText*>( args[1].addr );
  string searchPattern = text -> GetValue();

  if( ! tree->IsDefined() || ! text->IsDefined() )
  {
    defined = false;
  }
  else
  {
    if(searchPattern.length() > 0)
    {
      const SuffixTreeEdge *ourEdge = NULL;
      int offset = -1;

      bool patternFound = tree -> GetInMemoryTree() -> FindEdgeForSearchPattern
          (searchPattern, &ourEdge, &offset);

      if( patternFound )
      {
        occurs = true;
      }
    }
  }

  result = qp->ResultStorage(s);
   //query processor has provided
   //a CcBool instance for the result

   CcBool* b = static_cast<CcBool*> (result.addr);

   b->Set(defined, occurs); //the first argument says the boolean
   //value is defined, the second is the
   //real boolean value)

   return 0;

}

/*
 6.2.3 ~patternPositionsFunction~

 Find the positions for a given search pattern in our suffixtree

  1. Search the given pattern in our SuffixTree. 
  2. Every vertex below this position is a position of our pattern

*/
int patternPositionsFunction(Word* args, Word& result, int message,
    Word& local, Supplier s)
{

   switch( message )

  {
    case OPEN: {
      list<int> *positions = new list<int> ();
      
      map<SuffixTreeVertex*, int> vertexDepthMap;

      SuffixTree* p = static_cast<SuffixTree*>( args[0].addr );
      FText *text = static_cast<FText*>( args[1].addr );
      string searchPattern = text -> GetValue();

      if( p->IsDefined() && text->IsDefined() ) 
      {
        if(searchPattern.length() > 0) 
        {
          // search position in suffix tree
          const SuffixTreeEdge *ourEdge = NULL;
          int offset = -1;
         
          bool patternFound = p -> GetInMemoryTree() -> FindEdgeForSearchPattern
            (searchPattern, &ourEdge, &offset);

          if( patternFound ) 
          {
            // Find all edges below our position
            queue<SuffixTreeVertex*> vertices;
           
            // Is searchPattern not unique? 
            if( ourEdge -> HasVertex() )
            {
              // Calculate depth for ChildVertex
              int vertexDepth = searchPattern.length() 
                + (ourEdge -> GetLength() - offset);

              vertices.push( ourEdge -> GetChild() );
              
              // Save vertex depth
              vertexDepthMap[ourEdge -> GetChild()] = vertexDepth;
            } 
            else 
            {
              // Add our edge to result
              positions -> push_front(ourEdge -> GetStartIndex() 
                           + offset - searchPattern.length());
            }

            // Process all vertices and caclculate depth of the vertex
            while(! vertices.empty()) 
            {
              SuffixTreeVertex *ourVertex = vertices.front();
              vertices.pop();

              // Read depth of our vertex
              int depthOfOurVertex = vertexDepthMap[ourVertex];

              size_t edges = ourVertex -> GetEdgeCount();
              
              for(size_t i = 0; i < edges; i++) 
              {
                SuffixTreeEdge *aEdge = ourVertex -> GetEdgeAt(i);
                
                // Append position to result set
                positions -> push_front(aEdge -> GetStartIndex()
                    - depthOfOurVertex);
                
                if(aEdge -> HasVertex()) 
                {
                  // Save depth for child vertices
                  vertexDepthMap[ aEdge -> GetChild() ] 
                    = depthOfOurVertex + aEdge -> GetLength();
                  
                  vertices.push( aEdge -> GetChild() );
                }
              }
            }
          }
        }
      }

      positions -> sort();
      positions -> unique();
      
      local.setAddr(positions);
      return 0;
    }
    case REQUEST: {
      
      list<int> *positions = static_cast< list<int>* >(local.addr);
      
      // Are elements in our list?
      if(! positions->empty()) 
      {
        // Return next position
        int firstValue = positions -> front();
        positions -> pop_front();
     
        CcInt* elem = new CcInt(true, firstValue);
        result.addr = elem;
        return YIELD;
      }

      // End of list reached
      result.addr = 0;
      return CANCEL;
    }

    case CLOSE: {
      
      // Clean up
      if(local.addr != NULL) 
      {
        list<int> *positions = static_cast< list<int>* >(local.addr);
        delete positions;
        local.setAddr(NULL);
      }

      return 0;
    }
    default: {
      /* should not happen */
      return -1;
    }
  }
}

/*
 6.2.4 ~patternCountFunction~

  Count the appearing of a given pattern in our Suffixtree:

  * search this pattern in our suffixtree
  * count all leafes below this position. Every leaf is a 
    appearing

*/
int patternCountFunction(Word* args, Word& result, int message, Word& local,
    Supplier s)
{

  bool defined = true;
  int count = 0;

  SuffixTree* p = static_cast<SuffixTree*> (args[0].addr);
  FText *text = static_cast<FText*> (args[1].addr);
  string searchPattern = text -> GetValue();

  if (!p->IsDefined() || !text->IsDefined())
  {
    defined = false;
  }
  else
  {
    if (searchPattern.length() > 0)
    {
      // search position in suffix tree
      const SuffixTreeEdge *ourEdge = NULL;
      int offset = -1;

      // Count leaves
      bool patternFound = p -> GetInMemoryTree() -> FindEdgeForSearchPattern(
        searchPattern, &ourEdge, &offset);

      if (patternFound)
      {
        // Result is unique
        if (!ourEdge -> HasVertex())
        {
          count = 1;
        }
        else
        {
          count = (ourEdge -> GetChild()) -> GetNumberOfLeaves();
        }
      }
    }
  }

  // Write result
  result = qp->ResultStorage(s);
  CcInt* res = static_cast<CcInt*> (result.addr);

  res->Set(defined, count);
  return 0;
}

/*
 This function searches for the longest repeated substrings in vertex and
 write it in curStringList.

*/
void SearchLongestRepeatedSubstring(SuffixTreeVertex* vertex,
    list<string>* curStringList, string curString)
{
  vertex -> TriggerLoadNextData();
  for (size_t edgeNo = 0; edgeNo < vertex->GetEdgeCount(); edgeNo++)
  {
    SuffixTreeEdge* edge = vertex->GetEdgeAt(edgeNo);
    if (edge->HasVertex())
    {
       edge ->GetChild() -> TriggerLoadNextData();
      // there is another node which lies lower in the tree
      SearchLongestRepeatedSubstring(
          edge->GetChild(),
          curStringList,
          curString + vertex->GetInput()->substr(edge->GetStartIndex(),
              edge->GetLength()));
    }
    else
    {
      // vertex is relating to edge the 'final' inner node before the leaf node
      if (curString.length() == curStringList->front().length())
      {
        // curStringVector contains a string of the length curString.length().
        // If curString is not yet included in the vector, it will be added
        // to the vector.
        if (curStringList->back()!=curString)
        {
          curStringList->push_back(curString);
        }
      }
      if (curString.length() > curStringList->front().length())
      {
        // curString is a new 'longest repeated substring'. The previously
        // identified shorter repeated substrings will be deleted.
        curStringList -> clear();
        curStringList->push_front(curString);
      }
    }
  }
}

/*
 6.2.5.1 ~longestRepeatedSubstringFunction~
 The function returns a stream of the longest repeated substrings.
 They will be find by searching the deepest inner node before the leaf
 with the greatest distance from the root.

*/
int longestRepeatedSubstringFunction(Word* args, Word& result, int message,
    Word& local, Supplier s)
{
  list<string>* repeatedStringList = static_cast<list<string>*> (local.addr);

  switch (message)
  {
  case OPEN:
  { // initialize the local storage
    SuffixTree* suffixtree = static_cast<SuffixTree*> (args[0].addr);
    list<string>* curStringList = new list<string> ();
    curStringList->push_front("");
    string curString = "";
    SearchLongestRepeatedSubstring(suffixtree -> GetInMemoryTree(), 
      curStringList, curString);

    local.addr = curStringList;
    return 0;
  }
  case REQUEST:
  { // return the next stream element

    if (!repeatedStringList->empty() && repeatedStringList->front().length()
        > 0)
    {
      FText* elem = new FText(true, repeatedStringList->front());
      repeatedStringList->pop_front();
      result.addr = elem;
      return YIELD;
    }
    else
    {
      result.addr = 0;
      return CANCEL;
    }
  }
  case CLOSE:
  { // free the local storage

    if (repeatedStringList != 0)
    {
      delete repeatedStringList;
      local.addr = 0;
    }

    return 0;
  }
  default:
  {
    /* should never happen */
    return -1;
  }
  }
}


void SearchShortestUniqueSubstring(SuffixTreeVertex* vertex,
    list<string>* curStringList, string curString)
{
  for (size_t edgeNo = 0; edgeNo < vertex->GetEdgeCount(); edgeNo++)
  {
    SuffixTreeEdge* edge = vertex->GetEdgeAt(edgeNo);
    if (edge->HasVertex())
    {
      // There is another node which lies lower in the tree.
      SearchShortestUniqueSubstring(
          edge->GetChild(),
          curStringList,
          curString + vertex->GetInput()->substr(edge->GetStartIndex(),
              edge->GetLength()));
    }
    else
    {
      // vertex is relating to edge, the 'final' inner node before the
      // leaf node
      if (edge->GetStartIndex() < vertex->GetInput()->length() )
      {
        if((vertex->GetInput()->at(edge->GetStartIndex())) != '\0')
        {
        // curString is the prefix of the shortest unique substring, since
        // the edge is labeled not only with the index for the guide.
        string newUniqueString = curString + vertex->GetInput()->at(
            edge->GetStartIndex());
        
        if (newUniqueString.length() == curStringList->front().length())
        {
          if (curStringList->back()!=newUniqueString){
             curStringList->push_back(newUniqueString);
          }
        }
        if (newUniqueString.length() < curStringList->front().length())
        {
          curStringList-> clear();
            curStringList->push_front(newUniqueString);
        }
        }
      }
    }
  }
}

/*
 6.2.6 ~shortestUniqueSubstringFunction~
 The function returns a stream of the shortes unique substrings.
 They will be find by looking for the last inner node v before a leaf
 fullfilling two conditions:
 a) v has minimal stringlength
 b) the outgoing edge is not only labled with the termination character.
 The string spelled until v concatenated with the first character on the
 outgoing edge is a unique substring.

*/
int shortestUniqueSubstringFunction(Word* args, Word& result, int message,
    Word& local, Supplier s)
{
  list<string>* repeatedStringList = static_cast<list<string>*> (local.addr);

  switch (message)
  {
  case OPEN:
  { // initialize the local storage
    SuffixTree* suffixtree = static_cast<SuffixTree*> (args[0].addr);
    list<string>* curStringList = new list<string> ();
    
    string data = *(suffixtree -> GetInMemoryTree() ->GetInput());

    if(data[0] != '\0')
    {
      curStringList->push_front(data);
      string curString = "";
    
      SearchShortestUniqueSubstring(suffixtree -> GetInMemoryTree(), 
       curStringList, curString);
    }

    local.addr = curStringList;
    return 0;
  }
  case REQUEST:
  { // return the next stream element
    
    if (!repeatedStringList->empty() && repeatedStringList->front().length()
        > 0)
    {
      FText* elem = new FText(true, repeatedStringList->front());
      repeatedStringList->pop_front();
      result.addr = elem;
      return YIELD;
    }
    else
    {
      result.addr = 0;
      return CANCEL;
    }
  }
  case CLOSE:
  { // free the local storage

    if (repeatedStringList != 0)
    {
      delete repeatedStringList;
      local.addr = 0;
    }

    return 0;
  }
  default:
  {
    /* should never happen */
    return -1;
  }
  }
}

/*
 6.2.7 ~longestCommonSubstringFunction~
This function delivers the longest common substring of two strings.
Therefore it first construct a generalized suffix tree with additional
informations in the vertices. The additional informations are the current
string depth and whether the the node covers suffixes of string one or two
or both function AlterToGeneralizedST.
Then the algorithm search the node with the greatest string depth that is
marked with the information that it covers suffixes from both strings.

*/
void AlterToGeneralizedST(SuffixTreeVertex *st,size_t endS1)
{
    for (size_t edge=0; edge<=st->GetEdgeCount()-1; edge++)
    {

        SuffixTreeEdge *e = st->GetEdgeAt(edge);
        size_t ESI=e->GetStartIndex();
        size_t EEI=e->GetEndIndex();

        if (e->GetChild() != NULL)
        {
            SuffixTreeVertex *child = e->GetChild();
            child->SetSDepth(st->GetSDepth()+e->GetLength());


            AlterToGeneralizedST( child, endS1 );

            SuffixTreeVertex *parent = e->GetParent();
            parent->SetCov1(child->GetCov1() || parent->GetCov1());
            parent->SetCov2(child->GetCov2() || parent->GetCov2());
        }
        else //edge to a leaf
        {
           // reduce 2nd edge index on leaf edges
            if (ESI<=endS1 && EEI>endS1) e->SetEndIndex(endS1);
            if (e->GetEndIndex()<=endS1) e->GetParent()->SetCov1(true);
            if (e->GetEndIndex()>endS1)  e->GetParent()->SetCov2(true);
        }
    }
}


void lcs(SuffixTreeVertex *st,queue<string>*LCSqueue,string curPathLabel="")
{
    for (size_t edge=0; edge<=st->GetEdgeCount()-1; edge++)
    {
        SuffixTreeEdge *e = st->GetEdgeAt(edge);
        if (e->GetChild()!=NULL && e->GetChild()->GetCov1()==true &&
            e->GetChild()->GetCov2()==true)
            lcs(e->GetChild(), LCSqueue,
                curPathLabel+
                st->GetInput()->substr(e->GetStartIndex(),e->GetLength()));
        else
        {
            string last = "";
            if (!LCSqueue->empty())
                last = LCSqueue->back();

            if (last.length()<st->GetSDepth())
            {
                while (!LCSqueue->empty()) LCSqueue->pop();
                LCSqueue->push(curPathLabel);
            }

            if (last.length() == st->GetSDepth() && last!=curPathLabel)
                LCSqueue->push(curPathLabel);

        }
    }
}


int longestCommonSubstringFunction(Word* args, Word& result, int message,
    Word& local, Supplier s)
{
  queue<string> *LCSqueue = static_cast<queue<string>*> (local.addr);

  switch (message)
  {
  case OPEN:
  {
    FText *t1 = static_cast<FText*> (args[0].addr);
    FText *t2 = static_cast<FText*> (args[1].addr);
    queue<string> *q = new queue<string> ();

    if (t1->IsDefined() && t2->IsDefined())
    {
      string s1 = t1->GetValue();
      string s2 = t2->GetValue();
      string mergedText = s1+textSeparator+s2+terminationCharacter;

      SuffixTreeVertex *root =
        SimpleTreeBuilder::CreateSuffixTree(new string(mergedText));
      AlterToGeneralizedST(root,s1.length());

      lcs (root, q);
      delete root;
    }
    local.addr = q;

    return 0;
  }
  case REQUEST:
  {
     // return next stream elemt
    if (!LCSqueue->empty())
    {
      FText *elem = new FText(true, LCSqueue->front());
      LCSqueue->pop();
      result.addr = elem;
      return YIELD;
    }
    else
    {
      result.addr = 0;
      return CANCEL;
    }
  }
  case CLOSE:
  {
    if (LCSqueue != NULL)
    {
      delete LCSqueue;
      LCSqueue = NULL;
      local.addr = 0;
    }
    return 0;
  }
  default:
  {
    // should never happen
    return -1;
  }
  }
}


void SearchMaximalUniqueMatches(SuffixTreeVertex* vertex,
    vector<string>* curStringVector, string curString, size_t posSeparator)
{
  if (vertex->GetEdgeCount()==2 && !vertex->GetEdgeAt(0)->HasVertex()
      && !vertex->GetEdgeAt(1)->HasVertex() && curString.length()>0)
  {
    // inner node
    SuffixTreeEdge* edge1 = vertex->GetEdgeAt(0);
    SuffixTreeEdge* edge2 = vertex->GetEdgeAt(1);
    bool charExistsLeftText = true;
    bool charExistsRightText = true;
    size_t index1 = 0;  // index of the char before curString in the left Text
    size_t index2 = 0;  // index of the char before curString in the right text
    // If exact one of the two startIndexes of the two edges is lower than
    // the index of the separator, we have a MUM and curString may not
    // be extended at the right end.
    if (edge1->GetStartIndex()<=posSeparator &&
        edge2->GetStartIndex()>posSeparator){
      // If there is another char left of curString in the left text and a
      // char left of curString in the right text, we compare these chars. If
      // they are different we have another MUM. If there is no other char
      // left of curString in the right text or in the left text, curString is
      // a MUM too.
      if (edge1->GetStartIndex()>curString.length()){
        index1 = edge1->GetStartIndex() - curString.length() -1;
      } else {
        charExistsLeftText = false;
      }
      if (( edge2->GetStartIndex() - curString.length() - 1  ) > posSeparator){
        index2 = edge2->GetStartIndex() - curString.length() -1;
      } else {
        charExistsRightText = false;
      }
      if (charExistsLeftText && charExistsRightText){
        if ( vertex->GetInput()->at(index1) != vertex->GetInput()->at(index2)){
          curStringVector->push_back(curString);
        }
      } else {
        curStringVector->push_back(curString);
      }
    }
    if (edge2->GetStartIndex()<=posSeparator &&
        edge1->GetStartIndex()>posSeparator){
      // see the last comment
      if (edge2->GetStartIndex()>curString.length()){
        index2 = edge2->GetStartIndex() - curString.length() -1;
      } else {
        charExistsLeftText = false;
      }
      if (( edge1->GetStartIndex() - curString.length() - 1  ) > posSeparator){
        index1 = edge1->GetStartIndex() - curString.length() -1;
      } else {
        charExistsRightText = false;
      }
      if (charExistsLeftText && charExistsRightText){
        if ( vertex->GetInput()->at(index1) != vertex->GetInput()->at(index2)){
          curStringVector->push_back(curString);
        }
      } else {
          curStringVector->push_back(curString);
      }
    }
  }
  else {
    for (size_t edgeNo = 0; edgeNo < vertex->GetEdgeCount(); edgeNo++)
    {
        SuffixTreeEdge* edge = vertex->GetEdgeAt(edgeNo);
        if (edge->HasVertex())
        {
          // There is another node which lies lower in the tree.
          SearchMaximalUniqueMatches(
              edge->GetChild(),
              curStringVector,
              curString + vertex->GetInput()->substr(edge->GetStartIndex(),
                  edge->GetLength()),
              posSeparator);
        }
    }
  }
}

/*
 6.2.8 ~maximalUniqueMatchesFunction~
 The function returns a stream of the substring of 2 texts which occur in each
 text exactly once and have maximal length.
 They will be find by searching the deepest inner node with exactly 2 edges.
 One of the edges has to be a leaf of the first text and the other has to be a
 leaf of the second text. Thats ensure the maximality on the right side of
 the substring. The maximality on the left side will be proofed by comparing
 the character before the substring.

*/
int maximalUniqueMatchesFunction(Word* args, Word& result, int message,
    Word& local, Supplier s)
{
  vector<string>* MUMVector = static_cast<vector<string>*> (local.addr);

    switch (message)
    {
    case OPEN:
    { // initialize the local storage
      FText *ftext1 = static_cast<FText*> (args[0].addr);
      string firstText = ftext1 -> GetValue();

      FText *ftext2 = static_cast<FText*> (args[1].addr);
      string secondText = ftext2 -> GetValue();
      
      vector<string>* curStringVector = new vector<string> ();
      
      if(ftext1 -> IsDefined() && ftext2 -> IsDefined())
      {
        string mergedText = firstText + textSeparator + secondText
            + terminationCharacter;

        SuffixTreeVertex* root = 
          UkkonenTreeBuilder::CreateSuffixTree(new string(mergedText));
      
        string curString = "";
        SearchMaximalUniqueMatches(root, curStringVector, curString,
          firstText.length());
        
        delete root;
      }

      local.addr = curStringVector;
      return 0;
    }
    case REQUEST:
    { // return the next stream element

      if (!MUMVector->empty())
      {
        FText* elem = new FText(true, MUMVector->front());
        MUMVector->erase(MUMVector->begin());
        result.addr = elem;
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE:
    { // free the local storage

      if (MUMVector != NULL)
      {
        delete MUMVector;
        local.addr = 0;
      }

      return 0;
    }
    default:
    {
      /* should never happen */
      return -1;
    }
    }
}



/*
 6.2.9 ~circularStringLinearisationFunction~
 This function return a stream of tuple with the lexically smallest linear
 string representation of a circular string with the position(s) to cut the
 circular string.
 Given a string T of length n, it constructs a suffix tree off SS and
 then traverse the tree with the rule that at every node the traversal
 follows the edge whose first character is the lexically smallest over all
 first characters(except the termination character) until the path has at
 least a string depth of n. Any leaf in the subtree at that point
 delivers a cut-position.

*/


void FindLeaves (SuffixTreeEdge *e, queue<int> *LeafQueue, int aktSDepth,
    int lengthString)
{

    if (e->GetChild()==NULL) // start position of suffix is cut position
    {
        if (lengthString-aktSDepth!=(lengthString-1)/2)
          LeafQueue->push(lengthString-aktSDepth);
    }
    else
    {
        SuffixTreeVertex *st = e->GetChild();
        for (size_t edge=0; edge<=st->GetEdgeCount()-1; edge++)
        {
            e=st->GetEdgeAt(edge);
            FindLeaves(e, LeafQueue, aktSDepth+e->GetLength(), lengthString);
        }
    }

}


void CSL(SuffixTreeVertex *st, queue<int> *pos, string curPathLabel="",
    size_t aktSDepth=0)
{
    size_t lengthString = st->GetInput()->length();
    SuffixTreeEdge *e=NULL;

    // traverse the tree until string-depth n is achieved

    while (aktSDepth<(lengthString-1) / 2)
    {
        e=st->GetEdgeAt(0);
        if (e->GetStartIndex()==lengthString-1)
        {
          e=st->GetEdgeAt(1);
        }
        aktSDepth+=e->GetLength();
        curPathLabel = curPathLabel+st->GetInput()->substr(e->GetStartIndex(),
                       e->GetLength());
        if (e->GetChild()!=NULL && aktSDepth<lengthString)
        {
          st=e->GetChild();
        }

    }
    // string-depth n is achieved find all leaves
    FindLeaves(e, pos, aktSDepth,lengthString);
}

int circularStringLinearizationFunction(Word* args, Word& result, int message,
    Word& local, Supplier s)
{
    switch (message)
  {
  case OPEN:
  {
    
    ListExpr resultType = GetTupleResultType(s);
    TupleType *resultTupleType = new TupleType(nl->Second(resultType));
   
    FText *t =  static_cast<FText*> (args[0].addr);
    queue<int> *q = new queue<int> ();


    if (t->IsDefined())
    {
      string s = t->GetValue();
      string mergedText = s+s+terminationCharacter;
       SuffixTreeVertex *root =
           UkkonenTreeBuilder::CreateSuffixTree(new string(mergedText));
      if (mergedText.length()>1)
      {
        CSL(root,q);
      }
       delete root;
    }
    // store local data
    CslLocalData *cslLocalData = new CslLocalData();
    cslLocalData->posQueue = q; 
    cslLocalData->tupleType = resultTupleType;

    local.addr = cslLocalData;

    return 0;
  }
  case REQUEST:
  {
    CslLocalData *cslLocalData = static_cast<CslLocalData*> (local.addr);
    queue<int> *posQueue = cslLocalData->posQueue;
    TupleType *resultTupleType = cslLocalData->tupleType;

    if (!posQueue->empty())
    {
      size_t nextElem = posQueue->front(); posQueue->pop();
      CcInt *elem = new CcInt(true,nextElem);

      string erg=static_cast<FText*> (args[0].addr)->GetValue();
      if (nextElem>0 && nextElem<erg.length())
      {
        erg=erg.substr(nextElem)+erg.substr(0,nextElem);
      }
      FText *linstr= new FText(true,erg);


      Tuple *newTuple = new Tuple (resultTupleType);
      newTuple->PutAttribute(0,linstr);
      newTuple->PutAttribute(1,elem);

      result.setAddr(newTuple);
      return YIELD;
    }
    else
    {
      result.addr = 0;
      return CANCEL;
    }
  }
  case CLOSE:
  {
    if (local.addr != NULL)
    {
      CslLocalData *cslLocalData = static_cast<CslLocalData*> (local.addr);
      queue<int> *posQueue = cslLocalData->posQueue;
      TupleType *resultTupleType = cslLocalData->tupleType;

      delete posQueue;
      resultTupleType->DeleteIfAllowed();
      delete cslLocalData;

      local.addr = 0;
    }
    return 0;
  }
  default:
  {
    return -1;
  }
  }
}

/*
 Creates a list of all patterns |p| of a text s,
 which have at most k mismatches.

*/
void createkmismatchList(SuffixTreeVertex* vertex, list<string>* curStringList,
    int numReadChars, int numAllowedMismatches, int curFoundMismatches,
    string curString, string pattern)
{

  //number of edges from this vertex
  size_t numEdgesOfVertex = vertex->GetEdgeCount();

  //for all suffixtreeEdges in suffixtreeVertex
  for (size_t curNumEdges = 0; curNumEdges < numEdgesOfVertex; curNumEdges++)
  {
    //SuffixtreeEdge on position curNumEdge
    SuffixTreeEdge* edge = vertex->GetEdgeAt(curNumEdges, false);

    //actually found mismatches until now
    int newFoundMismatches = curFoundMismatches;

    //number characters on edge
    size_t numCharsOnEdge = edge->GetLength();

    string currentStringOnEdge = string(curString);
    int readCharsTotal = numReadChars;

    //for characters on edge
    for(size_t curCharNumOnEdge = 0; curCharNumOnEdge<= numCharsOnEdge;
        curCharNumOnEdge++)
    {

      //no more chars on edge start recursion
      if(curCharNumOnEdge  >= edge->GetLength())
      {
        if(edge->HasVertex()){
        createkmismatchList(edge->GetChild(), curStringList, readCharsTotal,
            numAllowedMismatches, newFoundMismatches,
               currentStringOnEdge, pattern);
        }
      }else{

        size_t offset = edge->GetStartIndex()+curCharNumOnEdge;

        //lese ein zeichen auf kante
        vertex -> LoadTextForIndex(offset, offset);

        const string *input = vertex -> GetInput();
        char curChar = (*input)[offset];

        
        if(curChar == '\0') {
          continue;
        }

        if(curChar != pattern[readCharsTotal]) 
        {
          newFoundMismatches++;
        }

        currentStringOnEdge.push_back(curChar);
        readCharsTotal++;
      }

      if(newFoundMismatches > numAllowedMismatches)
      {
        break;
      }

      if(currentStringOnEdge.length() == pattern.length())
      {
       curStringList->push_front(currentStringOnEdge);
       // curStringList->push_back(currentStringOnEdge);

        break;
      }

    }//end for char on edge

  }//end for num edges

  curStringList->unique();
  //curStringList->reverse();

   }//end function

/*
 6.2.10 ~kMismatchFunction~
 Returns an integer stream containing all patterns with at most k mismatches.

*/
int kMismatchFunction(Word* args, Word& result, int message, Word& local,
    Supplier s)
{

  list<string>* mismatchList = static_cast<list<string>*> (local.addr);

    switch (message)
    {
    case OPEN:
    {
        //suffixtree
        SuffixTree* suffixtree = static_cast<SuffixTree*> (args[0].addr);
      
      //textpattern
      FText* pattern = static_cast<FText*> (args[1].addr);
      string patternString = pattern->GetValue();

      //number of allowed mismatches
      CcInt* numOfMis = static_cast<CcInt*> (args[2].addr);
      int numOfMisInt = numOfMis->GetValue();
      size_t num = numOfMisInt;

      //list for output
      list<string>* curStringList = new list<string> ();
      curStringList->push_front("");
      string curString = "";

      //if all parameters are defined call createkmismatchList function
      //to fill the curStringList
      if(suffixtree->IsDefined() && pattern->IsDefined() &&
          numOfMis->IsDefined()){
      //rootvertex of suffixtree
        SuffixTreeVertex* root = suffixtree->GetInMemoryTree();

        int numofReadChars = 0;
        int curFoundMismatches = 0;

        //pattern mustn`t be longer than the text into the suffixtree
        //and number of mismatches mustn`t be longer than pattern length
        if((patternString.length() <= root->GetInput()->length())&&
            (num <= patternString.length())){

      createkmismatchList(root, curStringList, numofReadChars, numOfMisInt,
          curFoundMismatches, curString, patternString);
        }
      }

      local.addr = curStringList;
      return 0;
    }
    case REQUEST:
    { // return the next stream element

      if (!mismatchList->empty() && mismatchList->front().length()
          > 0)
      {
        FText* elem = new FText(true, mismatchList->front());

        //removes first element
        mismatchList->pop_front();
        result.addr = elem;

        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE:
    { // free the local storage

      if (mismatchList != 0)
      {
        delete mismatchList;
        local.addr = 0;
      }

      return 0;
    }
    default:
    {
      /* should never happen */
      return -1;
    }
    }
}

/*
 6.3 Operator Descriptions

 Similar to the ~property~ function of a type constructor, an operator needs to
 be described, e.g. for the ~list operators~ command.  This is now done by
 creating a subclass of class ~OperatorInfo~.

 6.3.1 ~createSuffixTreeInfo~ operator info for create function

*/
struct createSuffixTreeInfo: OperatorInfo
{

  createSuffixTreeInfo() :
    OperatorInfo()
  {
    name = "createsuffixtree";
    signature = FText::BasicType() + " -> " + SuffixTree::BasicType();
    syntax = "createsuffixtree (_)";
    meaning = "Creates a SuffixTree from a text in O(n) time.";
  }

};

/*
 6.3.1 ~createSuffixTreeQuadraticInfo~ operator info for create function
  with quadratic running time

*/
struct createSuffixTreeQuadraticInfo: OperatorInfo
{

 createSuffixTreeQuadraticInfo() :
   OperatorInfo()
 {
   name = "createsuffixtree_quadratic";
   signature = FText::BasicType() + " -> " + SuffixTree::BasicType();
   syntax = "createsuffixtree_quadratic (_)";
   meaning = "Creates a SuffixTree from a text with quadratic running time.";
 }

};

/*
 6.3.2 ~patternOccursInfo~

*/
struct patternOccursInfo: OperatorInfo
{

  patternOccursInfo() :
    OperatorInfo()
  {
    name = "patternoccurs";

    signature = SuffixTree::BasicType() + " x " + FText::BasicType() 
      + " -> " + CcBool::BasicType();
    syntax = "_ patternoccurs _";
    meaning = "Decides if a pattern is into the given text.";
  }

};

/*
 6.3.3 ~patternPositionInfo~

*/
struct patternPositionsInfo: OperatorInfo
{

  patternPositionsInfo() :
    OperatorInfo()
  {
    name = "patternpositions";
    signature = SuffixTree::BasicType() + " x " + FText::BasicType()
        + " -> stream(int)";
    syntax = "_ patternpositions _";
    meaning = "Return the positions of a given pattern";
  }

};

/*
 6.3.4 ~patternCountInfo~

*/
struct patternCountInfo: OperatorInfo
{

  patternCountInfo() :
    OperatorInfo()
  {
    name = "patterncount";
    signature = SuffixTree::BasicType() + FText::BasicType() + " -> "
        + CcInt::BasicType();
    syntax = "_ patterncount _";
    meaning = "Count a the given pattern in our SuffixTree";
  }

};

/*
 6.3.5 ~longestRepeatedSubstringInfo~

*/
struct longestRepeatedSubstringInfo: OperatorInfo
{

  longestRepeatedSubstringInfo() :
    OperatorInfo()
  {
    name = "longestrepeatedsubstring";
    signature = SuffixTree::BasicType() + " -> (stream text)";
    syntax = "longestrepeatedsubstring (_)";
    meaning = "Returns the longest string which occurs more than once"
      " into the text.";
  }

};

/*
 6.3.6 ~shortestUniqueSubstringInfo~

*/
struct shortestUniqueSubstringInfo: OperatorInfo
{

  shortestUniqueSubstringInfo() :
    OperatorInfo()
  {
    name = "shortestuniquesubstring";
    signature = SuffixTree::BasicType() + " -> stream(text)";
    syntax = "shortestuniquesubstring _";
    meaning = "returns the shortest not empty string which occurs"
      " only once.";
  }

};

/*
 6.3.7 ~longestCommonSubstringInfo~

*/
struct longestCommonSubstringInfo: OperatorInfo
{

  longestCommonSubstringInfo() :
    OperatorInfo()
  {
    name = "longestcommonsubstring";
    signature = FText::BasicType() +" x " + FText::BasicType() + " -> "
        + "(stream (text)";
    // stream(text)
    syntax = "longestcommonsubstring _ _";
    meaning = "returns the longest string which occurs in both texts.";
  }

};

/*
 6.3.8 ~maximalUniqueMatchesInfo~

*/
struct maximalUniqueMatchesInfo: OperatorInfo
{

  maximalUniqueMatchesInfo() :
    OperatorInfo()
  {
    name = "maximaluniquematches";
    signature = FText::BasicType() + FText::BasicType() + " -> stream(text)";
    syntax = "maximaluniquematches (_, _)";
    meaning = "returns all max unique matches of two strings.";
  }

};

/*
 6.3.9 ~circularStringLinearisationInfo~

*/
struct circularStringLinearizationInfo: OperatorInfo
{

  circularStringLinearizationInfo() :
    OperatorInfo()
  {
    name = "circularstringlinearization";
    signature = FText::BasicType() + " -> "
        "stream(tuple(linstr: text, pos: int))";
    syntax = "circularstringlinearization _ ";

    meaning = "returns the lexicographical smallest of all strings and"
      "their start positions.";
  }

};

/*
 6.3.10 ~kMismatchInfo~

*/
struct kMismatchInfo: OperatorInfo
{

  kMismatchInfo() :
    OperatorInfo()
  {
    name = "kmismatch";
    signature = SuffixTree::BasicType() + FText::BasicType()
    + CcInt::BasicType() +  " -> (stream text)";
    syntax = "kmismatch (_ _ _)";
    meaning = "returns all patterns of length p into a text s "
      "which have at most k errors in comperisson to pattern p .";
  }

};

/*
 6.4 Additional Operators

 6.4.1 Equals
 compares two SuffixTrees using the text

*/
ListExpr equalSuffixTreeTypeMap(ListExpr args)
{
  NList type(args);
  const string errMsg = "Expecting two SuffixTrees";

  // two SuffixTrees
  if (type == NList(SuffixTree::BasicType(), SuffixTree::BasicType()))
  {
    return NList(CcBool::BasicType()).listExpr();
  }
  return NList::typeError(errMsg);
}


int equalSuffixTreeFun(Word* args, Word& result, int message, Word& local,
    Supplier s)
{

  SuffixTree* tree1 = static_cast<SuffixTree*> (args[0].addr);
  SuffixTree* tree2 = static_cast<SuffixTree*> (args[1].addr);

  result = qp->ResultStorage(s);
  //query processor has provided
  //a CcBool instance for the result

  CcBool* b = static_cast<CcBool*> (result.addr);

  bool res = tree1->Equal(tree2);

  b->Set(true, res); //the first argument says the boolean
  //value is defined, the second is the
  //real boolean value)

  return 0;
}

struct equalSuffixTreeInfo: OperatorInfo
{

  equalSuffixTreeInfo()
  {
    name = "=";

    signature = SuffixTree::BasicType() + " x " + SuffixTree::BasicType()
        + " -> " + CcBool::BasicType();
    syntax = "_ = _";
    meaning = "Equal predicate.";
  }
};

/*
 7 Creating the Algebra

*/
class SuffixTreeAlgebra: public Algebra
{
public:
  SuffixTreeAlgebra() :
    Algebra()
  {
    /*
     7.1 Registration of Types

     */

    AddTypeConstructor(&SuffixTreeTC);

    //the lines below define that SuffixTree
    //can be used in places where types of kind DATA are expected
    SuffixTreeTC.AssociateKind(Kind::DATA());

    /*
     7.2 Registration of Operators

     */
    //addOperator
    AddOperator(createSuffixTreeInfo(), createSuffixTreeFunction,
        createSuffixTreeTypeMap);

    AddOperator(createSuffixTreeQuadraticInfo(),
        createSuffixTreeFunction_quadratic, createSuffixTreeTypeMap);

    AddOperator(patternOccursInfo(), patternOccursFunction,
        patternOccursTypeMap);

    AddOperator(patternPositionsInfo(), patternPositionsFunction,
        patternPositionsTypeMap);

    AddOperator(patternCountInfo(), patternCountFunction, patternCountTypeMap);
    //addOperator
    AddOperator(longestRepeatedSubstringInfo(),
        longestRepeatedSubstringFunction, suffixtree_textstreamTypeMap);

    AddOperator(shortestUniqueSubstringInfo(), shortestUniqueSubstringFunction,
        suffixtree_textstreamTypeMap);

    AddOperator(longestCommonSubstringInfo(), longestCommonSubstringFunction,
        longestCommonSubstringTypeMap);

    AddOperator(maximalUniqueMatchesInfo(), maximalUniqueMatchesFunction,
        maximalUniqueMatchesTypeMap);
    //addOperator
    AddOperator(circularStringLinearizationInfo(),
      circularStringLinearizationFunction, circularStringLinearizationTypeMap);

    AddOperator(kMismatchInfo(), kMismatchFunction, kMismatchTypeMap);

    AddOperator(equalSuffixTreeInfo(), equalSuffixTreeFun,
        equalSuffixTreeTypeMap);
  }
  ~SuffixTreeAlgebra()
  {
  }
  ;
};

/*
 8 Initialization

 Each algebra module needs an initialization function. The algebra
 manager has a reference to this function if this algebra is
 included in the list of required algebras, thus forcing the linker
 to include this module.

 The algebra manager invokes this function to get a reference to the
 instance of the algebra class and to provide references to the
 global nested list container (used to store constructor, type,
 operator and object information) and to the query processor.

 The function has a C interface to make it possible to load the
 algebra dynamically at runtime.

*/

} // end of namespace ~sta~

extern "C" Algebra*
InitializeSuffixTreeAlgebra(NestedList* nlRef, QueryProcessor* qpRef,
    AlgebraManager* amRef)
{
  nl = nlRef;
  qp = qpRef;
  am = amRef;

  return (new sta::SuffixTreeAlgebra());

}

