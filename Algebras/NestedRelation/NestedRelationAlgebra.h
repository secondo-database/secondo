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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header file of the Nested Relation Algebra

August 2009 Klaus Teufel

[TOC]


1 Overview

The Nested Relation Algebra implements two type constructors,
namely ~nrel~ and ~arel~. nrel implements a nested relation, i.e.
a relation that can have subrelations as attributes. arel implements
an attribute relation, i.e. a relation that can be the attribute
of a nested relation. arel can have attributes of type arel again, so 
that the Nested Relation Algebra allows for several levels of nesting. 

Both nrel and arel rely heavily on the types and functions implemented 
by the Relation Algebra module.

As an example, a nested relation ~publisher~ with two levels of 
nesting could be described as

\begin{displaymath}
{\underline{\smash{\mathit{nrel}}}}
  ({\underline{\smash{\mathit{tuple}}}}
    (
      (\textrm{publisher}, {\underline{\smash{\mathit{string}}}}),
      (\textrm{publications}, {\underline{\smash{\mathit{arel}}}} 
      ({\underline{\smash{\mathit{tuple}}}}
        (
          (\textrm{title}, {\underline{\smash{\mathit{string}}}}),
\end{displaymath}
\begin{displaymath}
          (\textrm{authors}, {\underline{\smash{\mathit{arel}}}} 
          ({\underline{\smash{\mathit{tuple}}}}
            (
              (\textrm{name}, {\underline{\smash{\mathit{string}}}})
            )
          ))
        )
      ))      
    )
  )
\end{displaymath}

This file will contain an interface of classes for these two type 
constructors, namely ~AttributeRelation~ and ~NestedRelation~.

2 Defines, includes, and constants

*/

#include "Algebra.h"
#include "NestedList.h"
#include "NList.h" 
#include "QueryProcessor.h"
#include "SecondoSystem.h"
#include "ConstructorTemplates.h" 
#include "StandardTypes.h"
#include "QueryProcessor.h"
#include "DBArray.h"
#include "Attribute.h"
#include "RelationAlgebra.h"

#include <vector>

extern NestedList* nl;
extern QueryProcessor *qp;

/*
3 Types arel and nrel

3.1 Class AttributeRelation

This class implements the representation of the type
constructor ~arel~. An attribute relation is a relation,
that can be used as an attribute within a nested relation.

*/

class NestedRelation;
/*
Forward declaration of class NestedRelation.

*/

