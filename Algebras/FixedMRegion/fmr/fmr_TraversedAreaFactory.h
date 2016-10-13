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

#include "fmr_FMRegion.h"
#include "fmr_Curve.h"
#include "fmr_Trochoid.h"
#include "fmr_Ravdoid.h"
#include "fmr_SegT.h"
#include "fmr_ISSegCurve.h"
#include "fmr_ISTrochoids.h"
#include "fmr_ISRavdoids.h"
#include "fmr_ISTrocRavd.h"
#include "fmr_CRegion.h"

#include <vector>
#include <algorithm>

namespace fmr {

/*
3 ~traversedArea~

Main function to calculate the traversed area of
the FMRegion ~fmregion~

*/
CRegion traversedArea(FMRegion& fmregion);

}

#endif  /* TRAVERSEDAREAFACTORY_H */
