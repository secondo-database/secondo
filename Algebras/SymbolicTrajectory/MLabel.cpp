
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

Started March 2012, Fabio Vald\'{e}s

[TOC]

\section{Overview}
This is the implementation of the Symbolic Trajectory Algebra.

\section{Defines and Includes}

*/

#include "Algorithms.h"

using namespace temporalalgebra;
using namespace std;


namespace stj{


/*
\section{Implementation of class ~MLabels~}

\subsection{Function ~createML~}

*/
void MLabel::createML(const int size, const int number, vector<string>& labels){
  if (size > 0) {
    ULabel ul(1);
    Instant start(1.0);
    start.Set(2015, 1, 1);
    time_t t;
    time(&t);
    srand((unsigned int)t);
    Instant end(start);
    SecInterval iv(true);
    int labelStartPos = size * number;
    for (int i = 0; i < size; i++) {
      datetime::DateTime dur(0, rand() % 86400000 + 3600000, 
                             datetime::durationtype);
      end.Add(&dur);
      ul.constValue.Set(true, labels[labelStartPos + i]);
      iv.Set(start, end, true, false);
      ul.timeInterval = iv;
      Add(ul);
      start = end;
    }
    units.TrimToSize();
  }
  else {
    cout << "Invalid parameters for creation." << endl;
    SetDefined(false);
  }
}

/*
\subsection{Function ~convertFromMString~}

*/
void MLabel::convertFromMString(const MString& source) {
  Clear();
  if (!IsDefined()) {
    return;
  }
  SetDefined(true);
  UString us(true);
  for (int i = 0; i < source.GetNoComponents(); i++) {
    source.Get(i, us);
    ULabel ul(us.timeInterval, us.constValue.GetValue());
    Add(ul);
  }
  units.TrimToSize();
}


}
