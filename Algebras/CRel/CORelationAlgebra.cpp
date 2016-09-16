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

*/

#include "CORelationAlgebra.h"

#include "AlgebraManager.h"
#include "CORelation.h"
#include "TupleBlock.h"
#include "QueryProcessor.h"

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

extern "C" Algebra *InitializeCORelationAlgebra(NestedList *nlRef,
                                                QueryProcessor *qpRef)
{
  return (new CORelationAlgebra());
}

ListExpr CORelationAlgebra::CRelProp()
{
  return nl->TwoElemList(
    nl->FourElemList(nl->StringAtom("Signature"), 
                      nl->StringAtom("Example Type List"), 
                      nl->StringAtom("List Rep"), 
                      nl->StringAtom("Example List")),
    nl->FourElemList(nl->StringAtom(""), 
                      nl->StringAtom(""), 
                      nl->TextAtom(""), 
                      nl->TextAtom("")));
}

ListExpr CORelationAlgebra::CRelOut(ListExpr typeInfo, Word value)
{
  return nl->Empty();
}

ListExpr CORelationAlgebra::SaveCRelToList(ListExpr typeInfo, Word value)
{
  return nl->Empty();
}

Word CORelationAlgebra::CreateCRel(const ListExpr typeInfo)
{
  return Word((Address)NULL);
}

Word CORelationAlgebra::CRelIn(ListExpr typeInfo, ListExpr value, int errorPos,
                               ListExpr &errorInfo, bool &correct)
{
  return Word((Address)NULL);
}

Word CORelationAlgebra::RestoreCRelFromList(ListExpr typeInfo, ListExpr value,
                                            int errorPos, ListExpr &errorInfo,
                                            bool &correct)
{
  return Word((Address)NULL);
}

void CORelationAlgebra::DeleteCRel(const ListExpr typeInfo, Word &w)
{
}

bool CORelationAlgebra::CheckCRel(ListExpr type, ListExpr &errorInfo)
{
  return false;
}

void *CORelationAlgebra::CastCRel(void *addr)
{
  return NULL;
}

void CORelationAlgebra::CloseCRel(const ListExpr typeInfo, Word &w)
{
}

bool CORelationAlgebra::OpenCRel(SmiRecord &valueRecord, size_t &offset,
                                 const ListExpr typeInfo, Word &value)
{
  return false;
}

bool CORelationAlgebra::SaveCRel(SmiRecord &valueRecord, size_t &offset,
                                 const ListExpr typeInfo, Word &value)
{
  return false;
}

int CORelationAlgebra::SizeOfCRel()
{
  return 0;
}

Word CORelationAlgebra::CloneCRel(const ListExpr typeInfo, const Word &w)
{
  return Word((Address)NULL);
}

ListExpr CORelationAlgebra::TBlockProp()
{
  return nl->TwoElemList(
    nl->FourElemList(nl->StringAtom("Signature"), 
                      nl->StringAtom("Example Type List"), 
                      nl->StringAtom("List Rep"), 
                      nl->StringAtom("Example List")),
    nl->FourElemList(nl->StringAtom(""), 
                      nl->StringAtom(""), 
                      nl->TextAtom(""), 
                      nl->TextAtom("")));
}

ListExpr CORelationAlgebra::TBlockOut(ListExpr typeInfo, Word value)
{
  return nl->Empty();
}

ListExpr CORelationAlgebra::SaveTBlockToList(ListExpr typeInfo, Word value)
{
  return nl->Empty();
}

Word CORelationAlgebra::CreateTBlock(const ListExpr typeInfo)
{
  return Word((Address)NULL);
}

Word CORelationAlgebra::TBlockIn(ListExpr typeInfo, ListExpr value,
                                 int errorPos, ListExpr &errorInfo, 
                                 bool &correct)
{
  return Word((Address)NULL);
}

Word CORelationAlgebra::RestoreTBlockFromList(ListExpr typeInfo, ListExpr value,
                                              int errorPos, ListExpr &errorInfo,
                                              bool &correct)
{
  return Word((Address)NULL);
}

void CORelationAlgebra::DeleteTBlock(const ListExpr typeInfo, Word &w)
{
}

bool CORelationAlgebra::CheckTBlock(ListExpr type, ListExpr &errorInfo)
{
  return false;
}

void *CORelationAlgebra::CastTBlock(void *addr)
{
  return NULL;
}

void CORelationAlgebra::CloseTBlock(const ListExpr typeInfo, Word &w)
{
}

bool CORelationAlgebra::OpenTBlock(SmiRecord &valueRecord, size_t &offset,
                                   const ListExpr typeInfo, Word &value)
{
  return false;
}

bool CORelationAlgebra::SaveTBlock(SmiRecord &valueRecord, size_t &offset,
                                   const ListExpr typeInfo, Word &value)
{
  return false;
}

int CORelationAlgebra::SizeOfTBlock()
{
  return 0;
}

Word CORelationAlgebra::CloneTBlock(const ListExpr typeInfo, const Word &w)
{
  return Word((Address)NULL);
}

CORelationAlgebra::CORelationAlgebra() : Algebra()
{
  AddTypeConstructor(new TypeConstructor(CORelation::GetBasicType(), CRelProp,
                                         CRelOut, CRelIn,
                                         SaveCRelToList, RestoreCRelFromList,
                                         CreateCRel, DeleteCRel,
                                         OpenCRel, SaveCRel, CloseCRel,
                                         CloneCRel, CastCRel,
                                         SizeOfCRel, CheckCRel), true);

  AddTypeConstructor(new TypeConstructor(TupleBlock::GetBasicType(), TBlockProp,
                                         TBlockOut, TBlockIn,
                                         SaveTBlockToList, 
                                         RestoreTBlockFromList,
                                         CreateTBlock, DeleteTBlock,
                                         OpenTBlock, SaveTBlock, CloseTBlock,
                                         CloneTBlock, CastTBlock,
                                         SizeOfTBlock, CheckTBlock), true);
}