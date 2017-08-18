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

1 Defines and Includes

*/

#include "NumericUtil.h"

using namespace std;

namespace temporalalgebra {
  namespace mregionops3 {
/*
2 Class NumericUtil

*/    
    bool NumericUtil::nearlyEqual(double a, double b){
      return fabs(a - b) <= eps;  
    }// nearlyEqual
    
    bool NumericUtil::nearlyEqual(const mpq_class& a, 
                                  const mpq_class& b){
      return abs(a - b) <= eps;
    }// nearlyEqual
    
    bool NumericUtil::lower(double a, double b){
      return a < b - eps;  
    }// lower
        
    bool NumericUtil::lower(const mpq_class& a, const mpq_class& b){
      return a < b - eps;    
    }// lower
          
    bool NumericUtil::lowerOrNearlyEqual(double a, double b) {
      return a < b || nearlyEqual(a, b);
    }// lowerOrNearlyEqual
    
    bool NumericUtil::lowerOrNearlyEqual(const mpq_class&  a, 
                                         const mpq_class&  b) {
      return a < b || nearlyEqual(a, b);
    }// lowerOrNearlyEqual
   
    bool NumericUtil::greater(double a, double b){
      return a > b + eps;
    }// greater
 
    bool NumericUtil::greater(const mpq_class& a, const mpq_class& b){
      return a > b + eps;
    }// greater
    
    bool NumericUtil::greaterOrNearlyEqual(double a, double b){
      return a > b || nearlyEqual(a, b);
    }// greaterOrNearlyEqual
    
    bool NumericUtil::greaterOrNearlyEqual(const mpq_class& a, 
                                           const mpq_class& b){
      return a > b || nearlyEqual(a, b);
    }// greaterOrNearlyEqual
    
    bool NumericUtil::between(double a, double x, double b){
      return (lowerOrNearlyEqual(a, x) && 
              lowerOrNearlyEqual(x, b)) || 
             (lowerOrNearlyEqual(b, x) && 
              lowerOrNearlyEqual(x, a));
    }// between
    
    bool NumericUtil::between(const mpq_class& a, const mpq_class& x, 
                              const mpq_class& b){
      return (lowerOrNearlyEqual(a, x) && 
              lowerOrNearlyEqual(x, b)) || 
             (lowerOrNearlyEqual(b, x) && 
              lowerOrNearlyEqual(x, a));
    }// between
    
    std::pair<double, double> NumericUtil::minMax4(double a, double b,
                                                   double c, double d) {
      double min = a;
      double max = a;
      if (b < min) min = b;
      if (b > max) max = b;
      if (c < min) min = c;
      if (c > max) max = c;
      if (d < min) min = d;
      if (d > max) max = d;
      return std::pair<double, double>(min, max);
    }// minMax4
/*
2 Class NumericFailure

*/        
    NumericFailure::NumericFailure(const string &message, const char *file,
                                   int line, const char *function): 
        runtime_error(message){
      ostringstream out;
      out << "Numeric exception has occurred in '" << function << "', file '";
      out <<  file << "' at line " << line << ". " << message;
      this->message = out.str();
    }// Konstruktor
      
    NumericFailure::~NumericFailure() throw() {    
    }// Destruktor
    
    const char *NumericFailure::what() const throw() {
       return this->message.c_str();
    }// what
  
  } // end of namespace mregionops3
} // end of namespace temporalalgebra
   