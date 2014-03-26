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

#ifndef CQL_SINGLENODEBALANCING
#define CQL_SINGLENODEBALANCING

#include <cql/policies/cql_load_balancing_policy.hpp>



using namespace std;

//namespace to avoid name conflicts
namespace cassandra {

/*
2.1 The single node policy

*/
class SingleNodeLoadBlancing : public cql::cql_load_balancing_policy_t {
public:

                SingleNodeLoadBlancing( string node ) 
                   : cassandraNode ( node ) {} 

/*
2.1.0 Init the class

*/
                virtual void init(cql::cql_cluster_t* theCluster);
                   
/*
2.1.1 Create a new query plan of type SingleNodeQueryPlan

*/
                virtual boost::shared_ptr<cql::cql_query_plan_t> 
                  new_query_plan( const boost::shared_ptr<cql::cql_query_t>& );

/*
2.1.2 calculate the distance of the host

*/                     
                virtual cql::cql_host_distance_enum distance(
                    const cql::cql_host_t& host);

private:
                
                // Node to query
                string cassandraNode;
                
                // Pointer to our cluster configuration
                cql::cql_cluster_t*  cluster;
};


/*
2.2 The query plans created by the policy

*/
class SingleNodeQueryPlan : public cql::cql_query_plan_t {
public:

                SingleNodeQueryPlan(const cql::cql_cluster_t* cluster, 
                                    string node );

                
/*
2.2.1 Returns the host to query for this query plan

*/
                virtual boost::shared_ptr<cql::cql_host_t> 
                    next_host_to_query();
                
private:
                // Node to query
                string cassandraNode;
                
                // List of all cassandra nodes
                vector<boost::shared_ptr <cql::cql_host_t> > allNodes;
};

} // Namespace
#endif