/*
----
This file is part of SECONDO.

Copyright (C) 2021, University in Hagen, Department of Computer Science,
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

[1] Association Analysis Algebra Implementation

January 2021 - April 2021, P. Fedorow for bachelor thesis.

*/

#include "Apriori.h"
#include "Common.h"
#include "Csv.h"
#include "Eclat.h"
#include "FPGrowth.h"
#include "GenRules.h"
#include "GenTransactions.h"

#include "Algebra.h"

namespace AssociationAnalysis {
class AssociationAnalysisAlgebra : public Algebra {
public:
  AssociationAnalysisAlgebra() : Algebra() {
    this->AddTypeConstructor(&fptreeTC);

    this->AddOperator(aprioriInfo(), mineVM<aprioriLI>, mineTM);
    this->AddOperator(createFpTreeInfo(), createFpTreeVM, createFpTreeTM);
    this->AddOperator(eclatInfo(), mineVM<eclatLI>, mineTM);
    this->AddOperator(fpGrowthInfo(), mineVM<fpGrowthLI>, mineTM);
    this->AddOperator(genStrongRulesInfo(), genStrongRulesVM, genStrongRulesTM);
    this->AddOperator(genTransactionsInfo(), genTransactionsVM,
                      genTransactionsTM);
    this->AddOperator(mineFpTreeInfo(), mineFpTreeVM, mineFpTreeTM);
    this->AddOperator(csvLoadTransactionsInfo(), csvLoadTransactionsVM,
                      csvLoadTransactionsTM);
    this->AddOperator(csvSaveTransactionsInfo(), csvSaveTransactionsVM,
                      csvSaveTransactionsTM);
  }
};
} // namespace AssociationAnalysis

extern "C" Algebra *InitializeAssociationAnalysisAlgebra(NestedList *,
                                                         QueryProcessor *) {
  return new AssociationAnalysis::AssociationAnalysisAlgebra();
}