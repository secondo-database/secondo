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

This header file contains definitions of the Algebra constructor and some auxiliary functions.

2 Defines and includes

*/

#ifndef SECONDO_LOGGER_H
#define SECONDO_LOGGER_H

#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <unordered_map>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

namespace pregel {
#define FORCE_LOG BOOST_LOG_SCOPED_THREAD_TAG("Force", true)
#define DEBUG BOOST_LOG_SEV(logger, debug)
 enum severity_level {
  debug2,
  debug,
  info,
  warn,
  error
 };

// using Logger = src::severity_logger_mt<severity_level>;
 typedef src::severity_logger_mt<severity_level> Logger;

 class LoggerFactory {
 private:
//  static std::unordered_map<std::string,
// src::severity_logger_mt<severity_level>> loggers;
//
  static bool initialized;

  static bool initialize();

 public:
  static Logger build(severity_level level = info);

//  static Logger &getLogger(std::string type) {
//   if (loggers.find(type) != loggers.end()) {
//    return loggers.at(type);
//   }
//   auto logger = new src::severity_logger_mt<severity_level>();
//   loggers.insert(
//    std::pair<std::string, src::severity_logger_mt<severity_level>>(type,
//                                                                    *logger));
//   return *logger;
//  }
 };

 extern Logger logger;
}

#endif //SECONDO_LOGGER_H