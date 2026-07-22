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

*/
#ifdef __cplusplus
extern "C"{
#endif
int registerSecondo();
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <string>
class SecondoInterface;

/*
Embed the optimizer into an already running SECONDO process (e.g. a forked
server) that owns ~serverSi~. Unlike ~registerSecondo~, this does NOT create a
second SecondoInterface/SMI environment; it reuses ~serverSi~ for the optimizer's
~secondo/2~ callbacks. The Prolog engine is started and the optimizer program
(~auxiliary~, ~calloptimizer~) is consulted with the working directory pointed
at ~SECONDO\_BUILD\_DIR/Optimizer~. Idempotent: a second call is a no-op.
Returns true on success.

*/
bool initEmbeddedOptimizer(SecondoInterface* serverSi);

/*
True once ~initEmbeddedOptimizer~ has successfully loaded the optimizer.

*/
bool embeddedOptimizerReady();

/*
Translate an SQL-dialect query into an executable plan using the embedded
optimizer's ~sqlToPlan~ predicate. On success returns true and sets ~plan~ (a
~query ...~ command) and ~costs~ (the optimizer's estimate for that plan, 0.0
when it has none); on failure returns false and sets ~errMsg~.

~errMsg~ is plain, ready-to-display text. The optimizer reports its failures as
~::ERROR::~ plus the message written back as a Prolog term (quoted, with ~\\n~
and ~\\'~ escapes); that encoding is undone here, so no client has to know it.

*/
/*
DDL note: for ~create ...~ and ~drop ...~ the optimizer's ~sqlToPlan~ performs
the operation itself and yields the sentinel atom ~done~ instead of a plan (see
~sqlToPlan2~ in Optimizer/optimizerNewProperties.pl). In that case this function
succeeds, sets ~plan~ to ~done~ and sets ~alreadyExecuted~ to true: the caller
must NOT execute ~plan~, there is nothing left to run. The sentinel is kept on
the wire because existing clients (e.g. the JDBC driver) already test for it.

*/
bool embeddedSqlToPlan(const std::string& sql, std::string& plan,
                       double& costs, std::string& errMsg,
                       bool& alreadyExecuted);

/*
Ensure the embedded optimizer has loaded the schema of database ~dbName~ (the
one currently open at the kernel). The optimizer only learns a database's schema
when it is opened through its own logic, so this drives that (close + reopen)
when needed. Returns true if the optimizer now tracks the database.

*/
bool embeddedOptimizerUseDatabase(const std::string& dbName);

/*
Run an arbitrary optimizer control directive (a Prolog goal given as text, e.g.
~showOptions~, ~setOption(subqueries)~, ~delOption(...)~, ~updateCatalog~,
~resetKnowledgeDB~) in the embedded optimizer. These directives do not produce a
result list; they ~write/1~ their feedback to the current output stream, so the
goal is run wrapped in ~with\_output\_to/2~ and whatever it prints is returned in
~output~. Returns true if the goal succeeded; on a Prolog exception or failure
~errMsg~ is set (the captured ~output~ is still returned, since a failing goal
may have written a useful message first).

*/
bool embeddedOptimizerRunGoal(const std::string& goal, std::string& output,
                              std::string& errMsg);

/*
True if executing ~sql~ changes the database catalog (~create table~/~index~,
~drop~, or a ~let X = select ...~ that creates a new object). The optimizer
caches the catalog, so after such a command it must be refreshed -- otherwise
subsequent optimizations run against a stale schema.

*/
bool embeddedOptimizerSqlChangesCatalog(const std::string& sql);

/*
Re-read the catalog of the open database into the optimizer (runs its
~updateCatalog~ goal). Call after a catalog-changing command, see
~embeddedOptimizerSqlChangesCatalog~.

*/
bool embeddedOptimizerRefreshCatalog(std::string& errMsg);
#endif

