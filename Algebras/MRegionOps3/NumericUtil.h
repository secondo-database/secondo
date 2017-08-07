/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
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
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]
//[x] [$\times $]
//[->] [$\rightarrow $]

[1] Definition and Implementation of the class ~NumericUtil~

April - November 2008, M. H[oe]ger for bachelor thesis.

[TOC]

1 Introduction

This file contains the definition and implementation of the class
~NumericUtil~ which provides several static methods to compare 
floating point values.

2 Defines and Includes

*/

#include <math.h>
#include <string>
#include <gmp.h>
#include <gmpxx.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

#ifndef NUMERICUTIL_H
#define NUMERICUTIL_H

namespace temporalalgebra {
  namespace mregionops3 {
/*
3 Class NumericUtil

*/
    class NumericUtil{
    public:
      static constexpr double eps  = 0.00000001;
      // for bounding
      static constexpr double eps2 = 0.0000001;
/*
3.1 nearlyEqual

Returns ~true~ if $-eps \le a-b \le eps$.

*/      
      static bool nearlyEqual(double a, double b);
      static bool nearlyEqual(const mpq_class& a, const mpq_class& b);
/*
3.2 lower

Returns ~true~ if $a < b-eps$.

*/      
      static bool lower(const mpq_class& a, const mpq_class& b);
      
      static bool lowerOrNearlyEqual(double a, double b);
      static bool lowerOrNearlyEqual(const mpq_class&  a, 
                                     const mpq_class&  b);
      
/*
3.3 greater

Returns ~true~ if $a > b+eps$.

*/ 
      static bool greater(const mpq_class& a, const mpq_class& b);
      
      static bool greaterOrNearlyEqual(double a, double b); 
      static bool greaterOrNearlyEqual(const mpq_class&  a, 
                                       const mpq_class&  b); 
      
      static bool between(double a, double x, double b);           
      static bool between(const mpq_class&  a, const mpq_class&  x, 
                          const mpq_class&  b); 
      
/*
3.4 minMax4

Returns the minimum and maximum value of $a$, $b$, $c$ and $d$.

*/      
      static std::pair<double, double> minMax4(double a, double b, 
                                               double c, double d);
    };
    
/*
4 Class NumericFailure

idea from http://stackoverflow.com/questions/348833 (11.03.2017)

*/    
    class NumericFailure : public std::runtime_error {
    public:
/*
4.1 Constructor

*/      
      NumericFailure(const std::string &message, const char *file, int line, 
                    const char *function);
/*
4.2 Destructor

*/      
      ~NumericFailure() throw();
/*
4.3 what

*/      
      const char *what() const throw();
    private:
      std::string message;
    
    };
 
#define NUM_FAIL(arg) throw NumericFailure(arg, __FILE__, __LINE__,__func__);

  } // end of namespace mregionops3
} // end of namespace temporalalgebra
#endif 
// NUMERICUTIL_H     