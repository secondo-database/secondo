/* 
----
 * This file is part of libfmr
 * 
 * File:   TraversedAreaFactory.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 16, 2016, 10:48 AM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class UPoint

[TOC]

1 Overview

Header file with the function declaration of ~traversedArea~

2 Includes and definitions

*/

#ifndef FMR_TRAVERSEDAREAFACTORY_H
#define FMR_TRAVERSEDAREAFACTORY_H

#include "FMRegion.h"
#include "Curve.h"
#include "Trochoid.h"
#include "Ravdoid.h"
#include "SegT.h"
#include "ISSegCurve.h"
#include "ISTrochoids.h"
#include "ISRavdoids.h"
#include "ISTrocRavd.h"
#include "Region2.h"

#include <vector>
#include <algorithm>

namespace fmr {

/*
3 ~traversedArea~

Main function to calculate the traversed area of
the FMRegion ~fmregion~

*/
Region2 traversedArea(FMRegion& fmregion);

}

#endif  /* TRAVERSEDAREAFACTORY_H */
