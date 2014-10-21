/* 
   poly.h
   This is a meta-includefile which includes the required file for
   the chosen implementation.

*/

#ifndef _POLY_H
#define _POLY_H

#ifdef POLYBOOST
# include "poly_boost.h"
#else
# include "poly_mathphys.h"
#endif

#endif

