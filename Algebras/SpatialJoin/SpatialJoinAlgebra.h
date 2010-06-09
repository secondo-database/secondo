/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

{\Large \bf Anhang F: Hauptspeicher R-Baum }

[1] Header-File of SpatialJoin-Algebra



[TOC]

1 Overview

2 Defines and Includes

*/


#include "stdarg.h"

#ifdef SECONDO_WIN32
#define Rectangle SecondoRectangle
#endif

#include <iostream>
#include <stack>
#include <vector>

using namespace std;

#include "SpatialAlgebra.h"
#include "RelationAlgebra.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "RectangleAlgebra.h"
#include "StandardTypes.h"


extern NestedList* nl;
extern QueryProcessor* qp;

#define BBox Rectangle
#define ArrayIndex long

#ifndef DOUBLE_MAX
#define DOUBLE_MAX (1.7E308)
#endif


