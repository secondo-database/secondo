/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2014, University in Hagen, Faculty of Mathematics and
 Computer Science, Database Systems for New Applications.

 SECONDO is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published
 by
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

 Jan Kristof Nidzwetzki

 //paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
 //paragraph [10] Footnote: [{\footnote{] [}}]
 //[TOC] [\tableofcontents]

 [TOC]

 0 Overview

 This is a single node load balancing policy for the
 cassandra cql driver. Normally the cql driver does
 a round robin load balancing between nodes. This
 policy sends all querys to the same node.
 
 1 Includes and defines

*/

#include <cql/cql.hpp>
#include <cql/cql_error.hpp>
#include <cql/cql_event.hpp>
#include <cql/cql_connection.hpp>
#include <cql/cql_session.hpp>
#include <cql/cql_cluster.hpp>
#include <cql/cql_builder.hpp>
#include <cql/cql_execute.hpp>
#include <cql/cql_result.hpp>
#include <cql/cql_set.hpp>
#include <cql/cql_metadata.hpp>
#include <cql/cql_host.hpp>

#include <cql/exceptions/cql_exception.hpp>
#include <cql/exceptions/cql_no_host_available_exception.hpp>

#include "CqlSingleNodeLoadbalancing.h"

 
using namespace std;

//namespace to avoid name conflicts
namespace cassandra {
 
void SingleNodeLoadBlancing::init(cql::cql_cluster_t* theCluster) {
  cluster = theCluster;
}

boost::shared_ptr<cql::cql_query_plan_t> SingleNodeLoadBlancing::new_query_plan(
    const boost::shared_ptr<cql::cql_query_t>&) {
  
    return boost::shared_ptr<cql::cql_query_plan_t>( 
           new SingleNodeQueryPlan( cluster, cassandraNode ) );
}

cql::cql_host_distance_enum 
   SingleNodeLoadBlancing::distance(const cql::cql_host_t& host) {
     
        return cql::CQL_HOST_DISTANCE_LOCAL;
}


SingleNodeQueryPlan::SingleNodeQueryPlan(const cql::cql_cluster_t* cluster, 
     string node) : cassandraNode(node) {
     
     // Get all nodes from current cluster configuration
     if(cluster != NULL) {
       if(cluster -> metadata().get() != NULL) {
         cluster->metadata()->get_hosts(allNodes);
       }
    }

}

boost::shared_ptr<cql::cql_host_t> SingleNodeQueryPlan::next_host_to_query() {
   for( size_t i = 0; i < allNodes.size(); ++i ) {
     if(allNodes[i].get() != NULL) {
        if( allNodes[i].get()->address().to_string() == cassandraNode ) {
          if (allNodes[i].get()->is_considerably_up()) {
            return allNodes[i];
          } else {
            
            // Our localnode is not up, break loop and
            // throw exception
            break; 
          }
        }
     }
   }
   
   // Host not found 
   throw cql::cql_no_host_available_exception(
     "no host is available according to load balancing policy.");
}


}