/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

*/

#include "createHgtIndex.h"

using namespace std;

namespace raster2 {

ListExpr createHgtIndexTM(ListExpr args) {
  const std::string error_message = "expects an object of the type sint and "
                                    "an optional integer";
  if (!nl->HasLength(args, 1) && !nl->HasLength(args, 2)) {
    return listutils::typeError(error_message);
  }
  if (!sint::checkType(nl->First(args))) {
    return listutils::typeError(error_message);
  }
  if (nl->HasLength(args, 1)) {
    return nl->ThreeElemList(
             nl->SymbolAtom(Symbol::APPEND()),
             nl->OneElemList(nl->IntAtom(1)),
             nl->ThreeElemList(nl->SymbolAtom(Hash::BasicType()), nl->Empty(),
                               nl->SymbolAtom(CcInt::BasicType())));
  }
  else {
    if (!CcInt::checkType(nl->Second(args))) {
      return listutils::typeError(error_message);
    }
  }
  return nl->ThreeElemList(nl->SymbolAtom(Hash::BasicType()), nl->Empty(),
                           nl->SymbolAtom(CcInt::BasicType()));
}

int createHgtIndexVM(Word* args, Word& result, int message, Word& local, 
                     Supplier s) {
  result = qp->ResultStorage(s);
  sint *hgt = static_cast<sint*>(args[0].addr);
  int precision = (static_cast<CcInt*>(args[1].addr))->GetIntval();
  Hash *hash = static_cast<Hash*>(result.addr);
  hash->Truncate();
  sint::storage_type& rs = hgt->getStorage();
  for (raster2::sint::iter_type it = rs.begin(), e = rs.end(); it != e; ++it) {
    int value = ((int)((int)*it / precision)) * precision;
    hash->Append(value, it.getIndex()[0]);
    hash->Append(value, it.getIndex()[1]);
  }
  return 0;
}

}
