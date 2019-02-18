/*
----
This file is part of SECONDO.

Copyright (C) 2019,
Faculty of Mathematics and Computer Science,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



0 LAS Utils

*/
#pragma once
#include "StandardTypes.h"

#include "Algebras/Pointcloud/lasreader/laspoint.h"

namespace pointcloud2 {

template<class T> ListExpr getAttr(std::string name);

ListExpr lasFormatAttrList(int pointType);
void extendAttrList(ListExpr pc2Format, int pointType);
CcInt* getAttr(int i);
CcInt* getAttr(uint8_t i);
CcInt* getAttr(uint16_t i);
CcInt* getAttr(uint32_t i);
CcInt* getAttr(uint64_t i);
CcInt* getAttr(char c);
CcBool* getAttr(bool b);
CcReal* getAttr(double d);

template<typename T> void extendTuple(Tuple* t, T v, size_t& offset);
void fillTuple0(Tuple* t, lasPoint0* p, size_t& offset);
void fillTuple1(Tuple* t, lasPoint1* p, size_t& offset);
void fillTuple2(Tuple* t, lasPoint2* p, size_t& offset);
void fillTuple3(Tuple* t, lasPoint3* p, size_t& offset);
void fillTuple4(Tuple* t, lasPoint4* p, size_t& offset);
void fillTuple5(Tuple* t, lasPoint5* p, size_t& offset);

} // end of namespace pointcloud2
