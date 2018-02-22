/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

//paragraph    [10]    title:           [{\Large \bf ] [}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[TOC] [\tableofcontents]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]
//[<] [$<$]
//[>] [$>$]

[10] Implementation file of the Compiled Expressions Code Store

2016/2017 H.Brieschke created the new CompiledExpression Algebra


[TOC]


1 Overview 

This file is the central point in which the implemented types and operators of the code store
are integrated into the ~Compiled Expression Algebra~, more precisely the ~code generator~.
For this purpose, the header files created in the subdirectory ~ceCodeStore~ are included,
on the other hand, the associated algebra classes created in the header files are loaded.

2 Defines, includes, and constants

*/

#include "./CECodeGenerator.h"

/*
2.1 Include algebra header files

List here all implemented ~ceAlgebra[<]secondo algebra name[>].h~-headerfiles from the 
subdirectory ~ceCodeStore~.

*/
#include "ceCodeStore/ceAlgebraStandard.h"
#include "ceCodeStore/ceAlgebraRelation.h"
#include "ceCodeStore/ceAlgebraExtRelation.h"

/*
3 Load the implemented Algebras

In this function, you must create the corresponding object for each algebra created in the
subdirectory ~ceCodeStore~ and insert it into the datastructure ~classObjAlgebras~.

NOTE that this is the maximum possible superset of the types and operators supported by
the ~CECompiler~ respectively the ~CECodeGenerator~. At runtime, only the types and operators
that are active in the ~AlgebraManager~ of the ~Secondo~-kernel are activated.

*/
using namespace CompiledExpressions;
namespace CompiledExpressions {

/*
3.1 Function ~loadCECGImplAlgebras~

This function creates and loads all implemented algebra objects from the ~ceCodeStore~.
Create for all in the ~ceCodeStore~ implemented ~Algebras~ (NOTE that are not 
the ~Secondo~-algebras) an object and insert this in the data structure ~classObjAlgebras~
according to the following pattern:

----

classObjAlgebras.insert(new CECGImplSecondoAlgebra_<secondo algebra name>());

----

*/
  void
  CECodeGenerator::loadCECGImplAlgebras() {
    classObjAlgebras.insert(new CECGImplSecondoAlgebra_Standard());
    classObjAlgebras.insert(new CECGImplSecondoAlgebra_Relation());
    classObjAlgebras.insert(new CECGImplSecondoAlgebra_ExtRelation());
    
  }
  
} // end of namespace CompiledExpressions
