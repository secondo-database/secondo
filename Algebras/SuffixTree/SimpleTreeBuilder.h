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

[1] Implementation of a simple algoritm to construct a suffix tree

[TOC]

1 Includes and Defines

*/

#ifndef SIMPLETREEBUILDER_H_
#define SIMPLETREEBUILDER_H_

#include <string>
#include <queue>

#include "SuffixTree.h"


/*

1 Class ~SimpleTreeBuilder~

*/
class SimpleTreeBuilder
{

public:

  static void PrintTree (SuffixTreeVertex *st, std::string *t);
  static int PosAtEdge (int ESI, int EEI, int SSI, std::string *t);
  static SuffixTreeVertex* CreateSuffixTree(std::string *text);
};

#endif /* SIMPLETREEBUILDER_H_ */
