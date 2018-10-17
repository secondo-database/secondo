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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header File of the class ~PregelAlgebra~

November 2018, J. Mende


[TOC]

1 Overview

This file contains definitions of the members of class LoggerFactory

*/

#include "LoggerFactory.h"
#include <boost/phoenix/bind.hpp>
//#include <boost/bind.hpp>

namespace pregel {
 BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", severity_level);
 BOOST_LOG_ATTRIBUTE_KEYWORD(loggerLevel, "LoggerLevel", severity_level);
 BOOST_LOG_ATTRIBUTE_KEYWORD(force_logging, "Force", bool)

 bool LoggerFactory::initialized = LoggerFactory::initialize();

 bool LoggerFactory::initialize() {
  return true;
 }

 Logger LoggerFactory::build(severity_level level) {
  Logger logger;

  return logger;
 }

 Logger LOG;
}