/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module Relation Algebra

[1] Separate part of persistent data representation

[1] Using Storage Manager Berkeley DB

June 1996 Claudia Freundorfer

May 2002 Frank Hoffmann port to C++

November 7, 2002 RHG Corrected the type mapping of ~tcount~.

November 30, 2002 RHG Introduced a function ~RelPersistValue~ instead of
~DefaultPersistValue~ which keeps relations that have been built in memory in a
small cache, so that they need not be rebuilt from then on.

[TOC]

*/
#ifdef RELALG_PERSISTENT  

using namespace std;

#include "RelationAlgebra.h"

int ccTuplesCreated = 0;
int ccTuplesDeleted = 0;
int ccRelsCreated = 0;
int ccRelsDeleted = 0;
int ccRelITsCreated = 0;
int ccRelITsDeleted = 0;
int ccTupleAttributesInfoCreated = 0;
int ccTupleAttributesInfoDeleted = 0;

ListExpr AttrTypeList = nl->TheEmptyList();

void CloseRecFile (CcRel* r)
{
  SmiRecordFile* myrecfile = r->GetRecFile();
  myrecfile->Close();
  //myrecfile->Drop();
  delete myrecfile;
  myrecfile = 0;
}

void CloseDeleteRecFile (CcRel* r)
{
  SmiRecordFile* myrecfile = r->GetRecFile();
  //myrecfile->Close();
  myrecfile->Drop();
  delete myrecfile;
  myrecfile = 0;
}

void CloseLobFile (CcRel* r)
{
  SmiRecordFile* myrecfile2 = r->GetLobFile();
  myrecfile2->Close();
  //myrecfile2->Drop();
  delete myrecfile2;
  myrecfile2 = 0;  
}

void CloseDeleteLobFile (CcRel* r)
{
  SmiRecordFile* myrecfile2 = r->GetLobFile();
  //myrecfile2->Close();
  myrecfile2->Drop();
  delete myrecfile2;
  myrecfile2 = 0;  
}

TupleAttributesInfo::TupleAttributesInfo (ListExpr typeInfo, ListExpr value)
{
  ListExpr attrlist, valuelist,first,firstvalue, errorInfo, lastElem;
  Word attr;
  int algebraId, typeId, noofattrs;
  attrTypes = new AttributeType[nl->ListLength(value)];
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  bool valueCorrect;

  //nl->WriteToFile("/dev/tty", typeInfo);
  //nl->WriteToFile("/dev/tty", value);

  attrlist = nl->Second(typeInfo);
  valuelist = value;
  noofattrs = 0;

  while (!nl->IsEmpty(attrlist))
  {
    first = nl->First(attrlist);
    attrlist = nl->Rest(attrlist);

    algebraId = nl->IntValue(nl->First(nl->Second(first)));
    typeId = nl->IntValue(nl->Second(nl->Second(first)));

    firstvalue = nl->First(valuelist);
    valuelist = nl->Rest(valuelist);
    attr = (algM->InObj(algebraId, typeId))(nl->Second(first),
              firstvalue, 0, errorInfo, valueCorrect);
    if (valueCorrect)
    {
      AttributeType attrtype = { algebraId, typeId, ((Attribute*)attr.addr)->Sizeof() };
      if (AttrTypeList == nl->TheEmptyList())
      {
        AttrTypeList = nl->Cons(nl->ThreeElemList(nl->IntAtom(algebraId),nl->IntAtom(typeId),
	  nl->IntAtom(((Attribute*)attr.addr)->Sizeof())), nl->TheEmptyList());
        lastElem = AttrTypeList;
      }
      else
        lastElem = nl->Append(lastElem, nl->ThreeElemList(nl->IntAtom(algebraId),
	  nl->IntAtom(typeId), nl->IntAtom(((Attribute*)attr.addr)->Sizeof())));

      //nl->WriteToFile("/dev/tty", AttrTypeList);
      attrTypes[noofattrs] = attrtype;
      noofattrs++;
    }
  }
  tupleType = new TupleAttributes(noofattrs, attrTypes);
  ccTupleAttributesInfoCreated++;
};

TupleAttributesInfo::TupleAttributesInfo (ListExpr persInfo)
{
  ListExpr persInfoList,first;
  int algebraId, typeId, noofattrs, size;
  
  attrTypes = new AttributeType[nl->ListLength(persInfo)];

  //nl->WriteToFile("/dev/tty", persInfo);

  persInfoList = persInfo;
  noofattrs = 0;

  while (!nl->IsEmpty(persInfoList))
  {
    first = nl->First(persInfoList);
    persInfoList = nl->Rest(persInfoList);

    algebraId = nl->IntValue(nl->First(first));
    typeId = nl->IntValue(nl->Second(first));
    size = nl->IntValue(nl->Third(first));

    AttributeType attrtype = { algebraId, typeId, size };
    attrTypes[noofattrs] = attrtype;
    noofattrs++;
  }
  tupleType = new TupleAttributes(noofattrs, attrTypes);
  ccTupleAttributesInfoCreated++;
};

TupleAttributesInfo::TupleAttributesInfo (ListExpr value, int attrno)
{
  ListExpr first;
  string attrname;
  int algId, typeId;
  StandardAttribute* attr;
  Word createdObject;
  
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  SecondoCatalog* sc = SecondoSystem::GetCatalog( ExecutableLevel );

  attrTypes = new AttributeType[attrno];
  for (int i = 0; i < attrno; i++)
  {
    first = nl->First(value);
    value = nl->Rest(value);
    if (nl->IsAtom(nl->Second(first)))
    {
      attrname = nl->SymbolValue(nl->Second(first));
      sc->GetTypeId( attrname, algId, typeId );
    }
    createdObject = algM->CreateObj(algId, typeId)(0);
    attr = (StandardAttribute*)createdObject.addr;
    AttributeType attrtype = { algId, typeId, attr->Sizeof() };
    attrTypes[i] = attrtype;
  }
  tupleType = new TupleAttributes(attrno, attrTypes);
  ccTupleAttributesInfoCreated++;
};
  
TupleAttributesInfo::TupleAttributesInfo (TupleAttributes* ta, AttributeType* at)
{
  attrTypes = at;
  tupleType = ta;
  ccTupleAttributesInfoCreated++;
}

TupleAttributesInfo::~TupleAttributesInfo ()
{
  delete[] attrTypes;
  delete tupleType;
  ccTupleAttributesInfoDeleted++;
};

TupleAttributes* TupleAttributesInfo::GetTupleTypeInfo () { 
return tupleType;
};

AttributeType* TupleAttributesInfo::GetAttributesTypeInfo () {return attrTypes;};

AttributeType* CloneAttributesTypeInfo ( TupleAttributesInfo* tuai, int attrno )
{
  AttributeType* at = new AttributeType [attrno];
  for (int i = 0; i < attrno; i++)
  {
    at[i] = (tuai->GetAttributesTypeInfo())[i];
  }
  return at;
}

AttributeType* CloneAttributesType ( AttributeType* attrt, int attrno )
{
  AttributeType* at = new AttributeType [attrno];
  for (int i = 0; i < attrno; i++)
  {
    at[i] = attrt[i];
  }
  return at;
}

TupleAttributes* CloneTupleTypeInfo ( TupleAttributesInfo* tuai, int attrno )
{
  AttributeType* at = new AttributeType [attrno];
  for (int i = 0; i < attrno; i++)
  {
    at[i] = (tuai->GetAttributesTypeInfo())[i];
  }
  return new TupleAttributes(attrno, at);
}

TupleAttributes* CloneTupleType ( AttributeType* attrt, int attrno )
{
  AttributeType* at = new AttributeType [attrno];
  for (int i = 0; i < attrno; i++)
  {
    at[i] = attrt[i];
  }
  return new TupleAttributes(attrno, at);
}

ListExpr TupleProp ()
{
  return (nl->TwoElemList(nl->TwoElemList(nl->SymbolAtom("plus"),
          nl->TwoElemList(nl->SymbolAtom("ident"), nl->SymbolAtom("DATA"))),
          nl->SymbolAtom("TUPLE")));
}

CcTuple::CcTuple ()
{
  NoOfAttr = 0;
  AttrList = 0;
  attrTypes = 0;
  tupleType = 0;
  ccTuplesCreated++;
};

CcTuple::CcTuple ( TupleAttributes* attributes, AttributeType* at )
{
  NoOfAttr = 0;
  AttrList = new Tuple ( attributes );
  attrTypes = at;
  tupleType = attributes;
  ccTuplesCreated++;
};

CcTuple::CcTuple ( Tuple* t, int noattrs, AttributeType* at, TupleAttributes* tt )
{
  NoOfAttr = noattrs;
  AttrList = t;
  attrTypes = at;
  tupleType = tt;
  ccTuplesCreated++;
};

CcTuple::~CcTuple ()
{
  delete AttrList;
  AttrList = 0;
  ccTuplesDeleted++;
};

void CcTuple::PutTuple(Tuple* tuple) { AttrList = tuple; }

void CcTuple::PutAttrTypes(AttributeType* at) { attrTypes = at; }

void CcTuple::SetAttrType(int i, int j, AttributeType* at) { attrTypes[i] = at[j]; }

void CcTuple::PutTupleType(TupleAttributes* ta) { tupleType = ta; }

TupleAttributes* CcTuple::GetTupleAttributes () { return tupleType; };

AttributeType* CcTuple::GetAttributeType () { return attrTypes; };

Tuple* CcTuple::GetTuple () { return AttrList; };

Attribute* CcTuple::Get (int index) {return (Attribute*)AttrList->Get(index);};

void  CcTuple::Put (int index, Attribute* attr) {AttrList->Put(index, attr);};

void  CcTuple::SetNoAttrs (int noattr) {NoOfAttr = noattr;};

int   CcTuple::GetNoAttrs () {return NoOfAttr;};

bool CcTuple::IsFree() { return isFree; }

void CcTuple::SetFree(bool b) { isFree = b; }

SmiRecordId CcTuple::GetId()
{
  return id;
}

void CcTuple::SetId(SmiRecordId id)
{
  this->id = id;
}


CcTuple* CcTuple::Clone()
{
  CcTuple* result = new CcTuple();
  result->SetFree(true);
  result->SetNoAttrs(GetNoAttrs());
  //result->PutTupleType(GetTupleAttributes());
  result->PutTupleType( CloneTupleType(GetAttributeType(),GetNoAttrs()) );
  //result->PutAttrTypes(GetAttributeType()); 
  result->PutAttrTypes( CloneAttributesType(GetAttributeType(),GetNoAttrs()) );  
  result->PutTuple(new Tuple(GetTupleAttributes()));
  for(int i = 0; i < GetNoAttrs(); i++)
  {
    Attribute* attr = ((Attribute*)Get(i))->Clone();
    result->Put(i, attr);
  }
  return result;
}

CcTuple* CcTuple::CloneIfNecessary()
{
  //if(IsFree())
  //{
    return this;
  //}
  //else
  //{
    //return Clone();
  //}
}

void CcTuple::DeleteIfAllowed()
{
  //if(IsFree())
  //{
    //for(int i = 0; i < GetNoAttrs(); i++)
    //{
      //Attribute* attr = (Attribute*)Get(i);
      //delete attr;
    //}
    delete this;
  //}
}
/*

The next function supports writing objects of class CcTuple to standard
output. It is only needed for internal tests.

*/
ostream& operator<<(ostream& os, CcTuple t)
{
  TupleElement* attr;

  os << "(";
  for (int i=0; i < t.GetNoAttrs(); i++)
  {
    attr = (TupleElement*)t.Get(i);
    attr->Print(os);
    if (i < (t.GetNoAttrs() - 1)) os << ",";
  }
  os << ")";
  return os;
}
/*

The lexicographical order on CcTuple. To be used in conjunction with
STL algorithms.

*/
bool LexicographicalCcTupleCmp::operator()(const CcTuple* aConst, const CcTuple* bConst) const
{
  CcTuple* a = (CcTuple*)aConst;
  CcTuple* b = (CcTuple*)bConst;


  for(int i = 0; i < a->GetNoAttrs(); i++)
  {
    if(((Attribute*)a->Get(i))->Compare(((Attribute*)b->Get(i))) < 0)
    {
      return true;
    }
    else
    {
      if(((Attribute*)a->Get(i))->Compare(((Attribute*)b->Get(i))) > 0)
      {
        return false;
      }
    }
  }
  return false;
}

string
ReportTupleStatistics()
{
  ostringstream buf;
  buf << ccTuplesCreated << " tuples created, "
      << ccTuplesDeleted << " tuples deleted, difference is "
      << (ccTuplesCreated - ccTuplesDeleted) << "." << endl;

  ccTuplesCreated = 0;
  ccTuplesDeleted = 0;
  return buf.str();
}

string
ReportRelStatistics()
{
  ostringstream buf;
  buf << ccRelsCreated << " relations created, "
      << ccRelsDeleted << " relations deleted, difference is "
      << (ccRelsCreated - ccRelsDeleted) << "." << endl;

  ccRelsCreated = 0;
  ccRelsDeleted = 0;
  return buf.str();
}

string
ReportRelITStatistics()
{
  ostringstream buf;
  buf << ccRelITsCreated << " relationits created, "
      << ccRelITsDeleted << " relationits deleted, difference is "
      << (ccRelITsCreated - ccRelITsDeleted) << "." << endl;

  ccRelITsCreated = 0;
  ccRelITsDeleted = 0;
  return buf.str();
}

string
ReportTupleAttributesInfoStatistics()
{
  ostringstream buf;
  buf << ccTupleAttributesInfoCreated << " tupleattributesinfos created, "
      << ccTupleAttributesInfoDeleted << " tupleattributesinfos deleted, difference is "
      << (ccTupleAttributesInfoCreated - ccTupleAttributesInfoDeleted) << "." << endl;

  ccTupleAttributesInfoCreated = 0;
  ccTupleAttributesInfoDeleted = 0;
  return buf.str();
}

void Concat (Word r, Word s, Word& t)
{
  int rnoattrs, snoattrs, tnoattrs;
  //Attribute* attr;
  AttributeType* attrTypes;
  TupleAttributes* tupleType;
  Tuple* tuple;

  rnoattrs = ((CcTuple*)r.addr)->GetNoAttrs();
  snoattrs = ((CcTuple*)s.addr)->GetNoAttrs();

  ((CcTuple*)t.addr)->SetNoAttrs(rnoattrs + snoattrs);
  
  //attrTypes = new AttributeType[ rnoattrs + snoattrs ];
  ((CcTuple*)t.addr)->PutAttrTypes( new AttributeType[ rnoattrs + snoattrs ] );
  
  for (int i = 1; i <= rnoattrs; i++)
  {
    ((CcTuple*)t.addr)->SetAttrType(i-1, i-1, ((CcTuple*)r.addr)->GetAttributeType());
  }
  
  for (int j = (rnoattrs+1); j <= (rnoattrs + snoattrs); j++)
  {
    //attrTypes[j-1] = (((CcTuple*)s.addr)->GetAttributeType())[j - rnoattrs - 1] ;
    ((CcTuple*)t.addr)->SetAttrType(j-1, j-rnoattrs-1, 
      ((CcTuple*)s.addr)->GetAttributeType());
  }
  
  //tupleType = new TupleAttributes(rnoattrs + snoattrs, attrTypes);
  ((CcTuple*)t.addr)->PutTupleType( new TupleAttributes(rnoattrs + snoattrs, 
    ((CcTuple*)t.addr)->GetAttributeType()) );
  
  //tuple = new Tuple ( tupleType );
  ((CcTuple*)t.addr)->PutTuple( new Tuple(((CcTuple*)t.addr)->GetTupleAttributes()) );
  
  for (int i = 1; i <= rnoattrs; i++)
  {
  
    //tuple->Put( (i-1), ((CcTuple*)r.addr)->Get(i-1) );
    (((CcTuple*)t.addr)->GetTuple())->Put( (i-1),
      (((CcTuple*)r.addr)->Get(i-1))->Clone() );
    
   // attr = ((CcTuple*)r.addr)->Get(i - 1);
   // ((CcTuple*)t.addr)->Put((i - 1), ((StandardAttribute*)attr)->Clone());
  }
  
  for (int j = (rnoattrs + 1); j <= (rnoattrs + snoattrs); j++)
  {
    //tuple->Put( (j-1), ((CcTuple*)s.addr)->Get(j - rnoattrs - 1) );
    (((CcTuple*)t.addr)->GetTuple())->Put( (j-1),
      (((CcTuple*)s.addr)->Get(j-rnoattrs-1))->Clone() );

    //attr = ((CcTuple*)s.addr)->Get(j - rnoattrs - 1);
    //((CcTuple*)t.addr)->Put((j - 1), ((StandardAttribute*)attr)->Clone());
  }
  
  //cout << *(((CcTuple*)t.addr)->GetTuple()) << endl;
  
  //((CcTuple*)t.addr)->PutTuple(tuple);
  //((CcTuple*)t.addr)->PutAttrTypes(attrTypes);
  //((CcTuple*)t.addr)->PutTupleType(tupleType);
  
  //t = SetWord ( new CcTuple( tuple, (rnoattrs + snoattrs), attrTypes, tupleType ) );
}

/*

1.3.2 ~Out~-function of type constructor ~tuple~

The ~out~-function of type constructor ~tuple~ takes as inputs a type
description (~typeInfo~) of the tuples attribute structure in nested list
format and a pointer to a tuple value, stored in main memory.
The function returns the tuple value from main memory storage
in nested list format.

*/
ListExpr OutTuple (ListExpr typeInfo, Word  value)
{
  int attrno, algebraId, typeId;
  ListExpr l, lastElem, attrlist, first, valuelist;
  CcTuple* tupleptr;

  tupleptr = (CcTuple*)value.addr;
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  attrlist = nl->Second(nl->First(typeInfo));
  attrno = 0;
  l = nl->TheEmptyList();
  while (!nl->IsEmpty(attrlist))
  {
    first = nl->First(attrlist);
    attrlist = nl->Rest(attrlist);
    algebraId = nl->IntValue(nl->First(nl->Second(first)));
    typeId = nl->IntValue(nl->Second(nl->Second(first)));
    valuelist = (algM->OutObj(algebraId, typeId))(nl->Rest(first),
                  SetWord(tupleptr->Get(attrno)));
    attrno++;
    if (l == nl->TheEmptyList())
    {
      l = nl->Cons(valuelist, nl->TheEmptyList());
      lastElem = l;
    }
    else
      lastElem = nl->Append(lastElem, valuelist);
  }
  return l;
}

void DeleteTuple(Word& w)
{
  CcTuple* tupleptr;
  int attrno;
  tupleptr = (CcTuple*)w.addr;
  attrno = tupleptr->GetNoAttrs();
  for (int i = 0; i <= (attrno - 1); i++)
  {
    delete (TupleElement*)tupleptr->Get(i);
  }
  delete tupleptr;
}

bool CheckTuple(ListExpr type, ListExpr& errorInfo)
{
  vector<string> attrnamelist;
  ListExpr attrlist, pair;
  string attrname;
  bool correct, ckd;
  int unique;
  vector<string>::iterator it;
  AlgebraManager* algMgr;

  if ((nl->ListLength(type) == 2) && (nl->IsEqual(nl->First(type), "tuple",
       true)))
  {
    attrlist = nl->Second(type);
    if (nl->IsEmpty(attrlist))
    {
      errorInfo = nl->Append(errorInfo,
        nl->ThreeElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
          nl->IntAtom(1)));
      return false;
    }
    if (nl->IsAtom(attrlist))
    {
      errorInfo = nl->Append(errorInfo,
        nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
          nl->IntAtom(2),
        attrlist));
      return false;
    }
    algMgr = SecondoSystem::GetAlgebraManager();
    attrnamelist.resize(MaxSizeOfAttr);
    it = attrnamelist.begin();
    unique = 0;
    correct = true;
    while (!nl->IsEmpty(attrlist))
    {
      pair = nl->First(attrlist);
      attrlist = nl->Rest(attrlist);
      if (nl->ListLength(pair) == 2)
      {
        if ((nl->IsAtom(nl->First(pair))) &&
          (nl->AtomType(nl->First(pair)) == SymbolType))
        {
          attrname = nl->SymbolValue(nl->First(pair));
          unique = std::count(attrnamelist.begin(), attrnamelist.end(),
                         attrname);
          if (unique > 0)
          {
            errorInfo = nl->Append(errorInfo,
             nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
               nl->IntAtom(3), nl->First(pair)));
            correct = false;
          }
          *it = attrname;
          ckd =  algMgr->CheckKind("DATA", nl->Second(pair), errorInfo);
          if (!ckd)
          {
            errorInfo = nl->Append(errorInfo,
              nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
                nl->IntAtom(6),nl->Second(pair)));
          }
          correct = correct && ckd;
        }
	else
        {
          errorInfo = nl->Append(errorInfo,
          nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
          nl->IntAtom(4),nl->First(pair)));
          correct = false;
        }
      }
      else
      {
        errorInfo = nl->Append(errorInfo,
          nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
          nl->IntAtom(5),pair ));
        correct = false;
      }
      it++;
    }
    return correct;
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("TUPLE"), type));
    return false;
  }
}

void* CastTuple(void* addr)
{
  return ( 0 );
}

Word CreateTuple(int Size)
{
  CcTuple* tup;
  tup = new CcTuple();
  return (SetWord(tup));
}

Word TupleInModel( ListExpr typeExpr, ListExpr list, int objNo )
{
  return (SetWord( Address( 0 ) ));
}

ListExpr TupleOutModel( ListExpr typeExpr, Word model )
{
  return (0);
}

Word TupleValueToModel( ListExpr typeExpr, Word value )
{
  return (SetWord( Address( 0 ) ));
}

Word TupleValueListToModel( const ListExpr typeExpr, const ListExpr valueList,
                       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = true;
  return (SetWord( Address( 0 ) ));
}

ListExpr RelProp ()
{
  return (nl->TwoElemList(nl->OneElemList(nl->SymbolAtom("TUPLE")),
          nl->SymbolAtom("REL")));
}

TupleAttributesInfo* CcRel::globreltai = 0;

CcRel::CcRel () 
{
  NoOfTuples = 0; 
  reltai = 0;
  TupleList = new CTable<CcTuple*>(100);
  recFile = new SmiRecordFile(false);
  recFile->Create("TEMPRECFILE");
  recFileId = recFile->GetFileId();
  //bool openrecfile = recFile->Open(recFileId, "RECFILE");
  
  lobFile = new SmiRecordFile(false);
  //bool openlobfile = lobFile->Open("LOBFILE");
  lobFile->Create("TEMPLOBFILE");
  lobFileId = lobFile->GetFileId();
  ccRelsCreated++;
};

CcRel::CcRel ( ListExpr ti, ListExpr v ) 
{
  NoOfTuples = 0; 
  reltai = 0;
  globreltai = new TupleAttributesInfo(ti, v);
  TupleList = new CTable<CcTuple*>(100);
  recFile = new SmiRecordFile(false);
  recFile->Create("RECFILE");
  recFileId = recFile->GetFileId();
  //bool openrecfile = recFile->Open(recFileId, "RECFILE");
  
  lobFile = new SmiRecordFile(false);
  //bool openlobfile = lobFile->Open("LOBFILE");
  lobFile->Create("LOBFILE");
  lobFileId = lobFile->GetFileId();
  ccRelsCreated++;
};

CcRel::CcRel ( int rfi, int lfi, TupleAttributesInfo* t, int noTuples )
{
  NoOfTuples = noTuples; 
  reltai = t;
  //TupleList = new CTable<CcTuple*>(100);
  recFile = new SmiRecordFile(false);
  recFile->Open(rfi, "RECFILE");
  recFileId = rfi;
  lobFile = new SmiRecordFile(false);
  lobFile->Open(lfi, "LOBFILE");
  lobFileId = lfi;
  ccRelsCreated++;
};

CcRel::~CcRel () 
{ 
  //delete TupleList;
  //recFile->Close();
  //delete recFile;
  //lobFile->Close();
  //delete lobFile;

  //delete recFile;
  //delete lobFile;
  delete reltai;
  ccRelsDeleted++; 
};

TupleAttributesInfo* CcRel::GetTupleAttributesInfo ()
{
  return reltai;
}

void CcRel::SetRelTupleAttributesInfo ( TupleAttributesInfo* ta )
{
  reltai = ta;
}

bool CcRel::OpenRecFile ()
{
  return recFile->Open ( lobFileId, "RECFILE" );
}

bool CcRel::OpenLobFile ()
{
  return lobFile->Open ( recFileId, "LOBFILE");
}

void CcRel::CloseRecFile ()
{
  recFile->Close();
}

void CcRel::CloseLobFile ()
{
  lobFile->Close();
}

void CcRel::AppendTuple (CcTuple* t)
{
  //TupleList->Add(t);
  (t->GetTuple())->SaveTo(recFile, lobFile);
  //delete t;
  //t = 0;
  NoOfTuples++;
};

SmiRecordFile* CcRel::GetRecFile() { return recFile; };
int CcRel::GetRecFileId() { return recFileId; };
SmiRecordFile* CcRel::GetLobFile() { return lobFile; };
int CcRel::GetLobFileId() { return lobFileId; };

CcRelIT* CcRel::MakeNewScan()
{ 
  CcRelIT* result; 
  result = new CcRelIT();

  SmiRecordFileIterator* it = new SmiRecordFileIterator();
  (this->GetRecFile())->SelectAll(*it, SmiFile::ReadOnly);
  it->Next(result->actualrecid, result->actualrec);
  result->r = this;
  result->rs = it;
  return result;
}

PrefetchingRelIterator* CcRel::MakeNewPrefetchedScan()
{
  return new PrefetchingRelIterator(this);
}

CcTuple* CcRel::GetTupleById(SmiRecordId id)
{
  return (*TupleList)[id];
}

void CcRel::SetNoTuples (int notuples) 
{
  NoOfTuples = notuples;
};

int CcRel::GetNoTuples () 
{
  return NoOfTuples;
};



//CcRelIT::CcRelIT (CTable<CcTuple*>::Iterator rs, CcRel* r)
//{
  //this->rs = rs;
  //this->r = r;
//}
CcRelIT::CcRelIT ()
{
  this->rs = 0;
  this->r = 0;
  this->actualrecid = 0;
  ccRelITsCreated++;
}

CcRelIT::CcRelIT (SmiRecordFileIterator* rs, CcRel* r, SmiRecordId recid,
                    SmiRecord rec)
{
  //(r->GetRecFile())->SelectAll(*rs, SmiFile::ReadOnly);
  this->rs = rs;
  this->r = r;
  this->actualrec = rec;
  this->actualrecid = recid;
  ccRelITsCreated++;
}

CcRelIT::~CcRelIT () 
{ 
  if (rs) { delete rs; rs = 0; };
  ccRelITsDeleted++;
  //if (r) { delete r; r = 0; };
};

CcRel* CcRelIT::GetRel()
{
  return r;
}

CcTuple* CcRelIT::GetTuple() 
{
  SmiRecord rec;
  Tuple* tuple;
  
  if ( rs->EndOfScan() ) return 0;
  
  else
  {
    tuple = new Tuple (r->GetRecFile(), this->actualrecid, r->GetLobFile(),
      ((r->GetTupleAttributesInfo())->GetTupleTypeInfo()), SmiFile::ReadOnly);
      //tai->GetTupleTypeInfo(), SmiFile::ReadOnly);

      
    return ( new CcTuple( tuple, tuple->GetAttrNum(), 
      //(r->GetTupleAttributesInfo())->GetAttributesTypeInfo(),
      CloneAttributesTypeInfo( r->GetTupleAttributesInfo(), tuple->GetAttrNum()),
      CloneTupleTypeInfo( r->GetTupleAttributesInfo(), tuple->GetAttrNum()))  );
      //(r->GetTupleAttributesInfo())->GetTupleTypeInfo()) );
      //tai->GetAttributesTypeInfo()) );
  }
};

void CcRelIT::Next() 
{ 
  if ( rs->Next(actualrecid, actualrec) )
  {
  }  
};

//bool CcRelIT::EndOfScan() { return ( rs == (r->TupleList)->End() ); };
bool CcRelIT::EndOfScan() 
{
  return ( rs->EndOfScan() ); 
};

CcRelIT& CcRelIT::operator=(CcRelIT& right)
{
  rs = right.rs;
  r = right.r;
  return (*this);

};

CcTuple* CcRelIT::GetNextTuple()
{
  //SmiRecord rec;
  //SmiRecordId recId;
  Tuple* tuple;
  
  if ( rs->EndOfScan() ) return 0;
  
  //if ( rs->Next(recId, rec) )
  else
  {
    tuple = new Tuple (r->GetRecFile(), this->actualrecid, r->GetLobFile(),
      (r->GetTupleAttributesInfo())->GetTupleTypeInfo(), SmiFile::ReadOnly);
    //tuple = new Tuple (r->GetRecFile(), recId, r->GetLobFile(),
      //tai->GetTupleAttributesInfo(), SmiFile::ReadOnly);

    //cout << " Contents of GetNextTuple() " << *tuple << endl;
    rs->Next(this->actualrecid, this->actualrec);
    /*if ( rs->Next(recId, rec) )
    {
      this->actualrec = rec;
      this->actualrecid = recId;
    }*/
    return ( new CcTuple( tuple, tuple->GetAttrNum(), 
      //(r->GetTupleAttributesInfo())->GetAttributesTypeInfo(),
      CloneAttributesTypeInfo( r->GetTupleAttributesInfo(), tuple->GetAttrNum()),
      CloneTupleTypeInfo( r->GetTupleAttributesInfo(), tuple->GetAttrNum()))  );
      //(r->GetTupleAttributesInfo())->GetTupleTypeInfo()) );
  }
  //else return 0;
}

PrefetchingRelIterator::PrefetchingRelIterator(CcRel* r)
{
  SmiRecordFile* file;
  
  this->r = r;
  file = r->GetRecFile();
  iter = file->SelectAllPrefetched();
};

PrefetchingRelIterator::~PrefetchingRelIterator()
{
  delete iter;
};
  
CcRel* PrefetchingRelIterator::GetRel()
{
  return r;
};

CcTuple* PrefetchingRelIterator::GetCurrentTuple()
{
  Tuple* tuple;
  
  TupleAttributesInfo* tai = r->GetTupleAttributesInfo();
  TupleAttributes* attributes = tai->GetTupleTypeInfo();
  
  SmiRecordFile* recfile = r->GetRecFile();
  SmiRecordFile* lobfile = r->GetLobFile();
  
  tuple = new Tuple(recfile, this->iter, lobfile, attributes);
  
  return 
    new CcTuple(tuple, tuple->GetAttrNum(), 
      CloneAttributesTypeInfo( r->GetTupleAttributesInfo(), tuple->GetAttrNum()),
      CloneTupleTypeInfo( r->GetTupleAttributesInfo(), tuple->GetAttrNum()));
};

bool PrefetchingRelIterator::Next()
{
  if(iter == 0)
  {
    return false;
  }
  else
  {
    return iter->Next();
  };
};

ListExpr OutRel(ListExpr typeInfo, Word  value)
{
  CcTuple* t;
  ListExpr l, lastElem, tlist, TupleTypeInfo;
  
  cout << "OutRel " << endl;
  
  CcRel* r = (CcRel*)(value.addr);

  CcRelIT* rit = r->MakeNewScan();
  l = nl->TheEmptyList();

  while ( (t = rit->GetNextTuple()) != 0 )
  {
    TupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
	  nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));
    tlist = OutTuple(TupleTypeInfo, SetWord(t));
    
    #ifdef RELALG_PERSISTENT
    
    delete t;
    
    #endif
    
    if (l == nl->TheEmptyList())
    {
      l = nl->Cons(tlist, nl->TheEmptyList());
      lastElem = l;
    }
    else
      lastElem = nl->Append(lastElem, tlist);
    //nl->WriteToFile("/dev/tty", l);
  }
  
  //SmiRecordFile* srf = r->GetRecFile();
  //cout << srf->GetContext() << endl;
  if ( ((r->GetRecFile())->GetContext()) == "TEMPRECFILE" )
  {    
    CloseDeleteRecFile(r);
    CloseDeleteLobFile(r);
    cout << "RelsCreatedOutRel : " << ccRelsCreated << endl;
    cout << "RelsDeletedOutRel : " << ccRelsDeleted << endl;
    //delete r;

  }
  else
  {
    CloseRecFile(r);
    CloseLobFile(r);
    cout << "RelsCreatedOutRel : " << ccRelsCreated << endl;
    cout << "RelsDeletedOutRel : " << ccRelsDeleted << endl;
    delete r;
  }
    
  //if (r) { delete r; r = 0; }
  delete rit;
  //delete tai;
  //tai = 0;
  //l = nl->OneElemList(nl->FiveElemList(nl->StringAtom("Berlin"),
    //nl->IntAtom(1859000),nl->IntAtom(1000),nl->StringAtom("030"),nl->StringAtom("B")));
  //delete r;
  //nl->WriteToFile("/dev/tty",l);
  return l;
}

Word CreateRel(int Size)
{
  cout << "CreateRel " << endl;
  CcRel* rel = new CcRel();
  return (SetWord(rel));
}

/*void DeleteRel(Word& w)
{
  cout << "DeleteRel " << endl;
  if(w.addr == 0)
  {
    return;
  }
  
  #ifndef RELALG_PERSISTENT

  CcTuple* t;
  CcRel* r;
  Word v;

  r = (CcRel*)w.addr;
  cout << "DeleteRel " << endl;
  CcRelIT* rit = r->MakeNewScan();
  while ( (t = rit->GetNextTuple()) != 0 )
  {
    v = SetWord(t);
    DeleteTuple(v);
  }
  delete rit;
  delete r;
  
  #else
  
  CcRel* r;

  r = (CcRel*)w.addr;
  SmiRecordFile* rf = r->GetRecFile();
  rf->Close();
  delete rf;
  SmiRecordFile* lf = r->GetLobFile();
  lf->Close();
  delete lf;

  delete r;
  
  #endif  
}*/

bool CheckRel(ListExpr type, ListExpr& errorInfo)
{
  AlgebraManager* algMgr;

  if ((nl->ListLength(type) == 2) && nl->IsEqual(nl->First(type), "rel"))
  {
    algMgr = SecondoSystem::GetAlgebraManager();
    return (algMgr->CheckKind("TUPLE", nl->Second(type), errorInfo));
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("REL"), type));
    return false;
  }
}

void* CastRel(void* addr)
{
  return ( 0 );
}

Word RelInModel( ListExpr typeExpr, ListExpr list, int objNo )
{
  return (SetWord( Address( 0 ) ));
}

ListExpr RelOutModel( ListExpr typeExpr, Word model )
{
  return (0);
}

Word RelValueToModel( ListExpr typeExpr, Word value )
{
  return (SetWord( Address( 0 ) ));
}

Word RelValueListToModel( const ListExpr typeExpr, const ListExpr valueList,
                       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = true;
  return (SetWord( Address( 0 ) ));
}

#endif

