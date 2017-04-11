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

1.1 Rename

This operator postfixes each attribut name of a tuple stream with an
underscore and a given postfix. This is done for subrelations too.

*/

#ifndef ALGEBRAS_NESTEDRELATION2_OPERATORS_RENAME_H_
#define ALGEBRAS_NESTEDRELATION2_OPERATORS_RENAME_H_

#include "Algebras/Stream/Stream.h"

using namespace std;
namespace nr2a {

class Rename
{
  public:
    struct Info : OperatorInfo
    {

        Info()
        {
          name = "rename";
          signature = "stream(tuple(X)) -> stream(tuple(Y))";
          syntax = "_ rename[ _ ]";
          meaning = "Renames all attribute names by adding"
              " them with the postfix passed as parameter. "
              "It can also be used by its short form.";
          example = "query publishers feed {s} consume";
        }
    };
    virtual ~Rename();

    static ListExpr MapType(ListExpr args);
    static ValueMapping functions[];
    static int SelectFunction(ListExpr args);
    static int RenameValue(Word* args, Word& result, int message,
        Word& local, Supplier s);

    static CreateCostEstimation costEstimators[];

  protected:

  private:
    Rename();
    // Declared, but not defined => Linker error on usage
    static bool RenameTypes(const ListExpr type, const string & postfix,
        ListExpr & outType, string & errorMsg, const bool sub=false);
};

} /* namespace nr2a*/

#endif /* ALGEBRAS_NESTEDRELATION2_OPERATORS_RENAME_H_*/
