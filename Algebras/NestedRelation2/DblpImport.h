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

 1.1 DBLP-Import

 It is possible to download the DBLP from the university of Trier as an XML
 file. This file can be imported into a nested relation in SECONDO using this
 operator.

*/

#ifndef ALGEBRAS_NESTEDRELATION2_DBLPIMPORT_H_
#define ALGEBRAS_NESTEDRELATION2_DBLPIMPORT_H_

//#include "Include.h"


namespace nr2a {

class DblpImportLocalInfo;


class DblpImport
{
  public:
    struct Info : OperatorInfo
    {
      Info();
    };
    virtual ~DblpImport();

    static ListExpr MapType(ListExpr args);
    static ValueMapping functions[];
    static int SelectFunction(ListExpr args);
    static int DblpImportValue(Word* args, Word& result, int message,
        Word& local, Supplier s);

    static CreateCostEstimation costEstimators[];

  protected:

  private:
    DblpImport(); // Declared, but not defined => Linker error on usage
    static void ReadStopwords(const string & stopwordsFilename,
        std::set<std::string> *stopwords);
    static std::ifstream::pos_type GetFilesize(const char* filename);
};

} /* namespace nr2a*/

#endif /* ALGEBRAS_NESTEDRELATION2_DBLPIMPORT_H_*/
