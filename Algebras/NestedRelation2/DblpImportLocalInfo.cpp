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

#include "DblpImportLocalInfo.h"

using namespace nr2a;

DblpImportLocalInfo::DblpImportLocalInfo()
    : m_reader(NULL), m_documentsProcessed(0)
{

}

/*virtual*/ DblpImportLocalInfo::~DblpImportLocalInfo()
{

}

/*
This method overrides the default behaviour which is triggered by the estimator
if a tuple is processed. This is necessary, because the input is not organized
by tuples, but by bytes read.

*/
/*virtual*/ void DblpImportLocalInfo::UnitProcessed()
{
  m_documentsProcessed++;
  long bytesProcessed = xmlTextReaderByteConsumed(m_reader);
  SetUnitsProcessed(bytesProcessed);
  if ((m_documentsProcessed % cRefreshProgressAfterTuples) == 0)
  {
    // Calling CheckProgress would not be useful here, for it depends on
    // calls to Eval happening in the meantime.
    // Letting the operator write its progress of its own will not work if it
    // is combined with other operators.
    qp->UpdateProgress();
  }
}

/*
When getting triggered by the estimator this class asks the known reader for
its actual progress in reading the file. This method is used to set the
corresponding reader.

*/
void DblpImportLocalInfo::SetReader(xmlTextReaderPtr reader)
{
  m_reader = reader;
}
