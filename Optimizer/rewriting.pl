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

April 2006, Christian D[ue]ntgen. Initial Version

*/

 
/*

14 rewiteQuery - Query Rewriting

Rewriting is used as a preprocessing step during the optimization of a query. 
In the rewriting phase, a query posed by the user will be analysed and rewritten
on the level of the query language rather than on the executable language level.
The optimization on the executable langage level is done in the subsequent steps 
of the optimization process, and work on the rewritten query.

During the rewriting process, regarding the user's original conditions, 
inferred conditions are added, some may be removed and others be modified.


----     rewriteQuery(OriginalQuery, -Rewrittenquery)
----

rewrites the ~OriginalQuery~ performing several steps of rewriting and returns
the finally ~RewrittenQuery~.

*/

rewriteQuery(Query, RewrittenQuery) :-
  rewriteQueryForMacros(Query, RQuery1),
  rewriteQueryForInferenceOfPredicates(RQuery1, RQuery2),
  rewriteQueryForRedundancy(RQuery2, RQuery3),
  rewriteQueryForCSE(RQuery3, RewrittenQuery).

/*

14.1 Macros: with $\ldots$ in

In Queries, users are allowed to define macros. Macros can be used within the
query and are expanded automatically. A macro declaration is noted as 

----  with <macro-mext> as <macro-mnemo> in <SQL-Query>
----

*/


 
/*
---- rewriteQueryForMacros(+QueryIn,-QueryOut)
----

Rewrites a query by extracting macro definitions and expanding all
macros within ~QueryIn~ and unifies rewritten query with ~QueryOut~.

*/

rewriteQueryForMacros(QueryIn,QueryIn) :-
  not(optimizerOption(rewriteMacros)), !.

rewriteQueryForMacros(QueryIn,QueryOut) :-
  extractMacros(QueryIn, QueryTmp),
  expandMacros(QueryTmp, QueryOut),
  dm(rewriteMacros,['\nREWRITING: Macros\n\tIn:  ',QueryIn,'\n\tOut: ',
                    QueryOut,'\n\n']).

% dynamic tables for translating mnemos
:- dynamic(storedMacro/2),
   dynamic(storedFlatMacro/2).

% mopping up the translation tables
retractMacros :- 
  retractall(storedMacro(_,_)), 
  retractall(storedFlatMacro(_,_)).

/*
---- extractMacros(+QueryIn, -QueryOut)
----
will analyse ~QUeryIn~ for any makro definitions and stote them in a table
~storedMacro(Mnemo,Expansion)~. Then, the macro definitions are removed from
~QueryIn~ and the result is returned in ~QueryOut~.

*/

extractMacros(QueryIn, QueryOut) :-
  QueryIn = with(in(Macros,QueryOut)),
  retractMacros,
  makeList(Macros,MacroList),
  extractMacros1(MacroList),
  !.

extractMacros(with(X), _) :-
  write('\nERROR: Correct syntax for using macros in queries is \n'),
  write('           \'sql with <macro> as <mnemo> in <query>.\'\n'),
  write('           \'sql with [<macro> as <mnemo> {, <macro> as <mnemo>}] '),
  write('in <query>.\'.\n'), 
  !, 
  throw(sql_ERROR(rewriting_extractMacros(with(X), undefined))),
  fail.

extractMacros(Query, Query).

extractMacros1([]) :- !.

extractMacros1([Me|Others]) :-
  extractMacros1(Me),
  extractMacros1(Others), 
  !.

extractMacros1(Macro as Mnemo) :-
  isSubTerm(Macro,Mnemo),
  write('\nERROR: Left side of a macro declaration \'<macro> as <mnemo>\' '),
  write('must be an acyclic expression.'), 
  !,
  throw(sql_ERROR(rewriting_extractMacros1(Macro as Mnemo))),
  fail.
         
extractMacros1(X as Mnemo) :-
  not(atom(Mnemo)),
  write('\nERROR: Right side of a macro declaration \'<macro> as <mnemo>\' '),
  write('must be an identifier.'), 
  !, 
  throw(sql_ERROR(rewriting_extractMacros1(X as Mnemo))),
  fail.

extractMacros1(Macro as Mnemo) :-
  not(isUsedMnemo(Mnemo)),
  not(isCyclicMacro(Mnemo)),
  asserta(storedMacro(Mnemo, Macro)), % bring later macros to front
  !.

/*
---- isCyclicMacro(Mnemo)
----
Succeeds, if ~Mnemo~ appears as a subterm in the ~storedMacros/2~.

*/

isCyclicMacro(Mnemo) :-
  storedMacro(XMnemo, XMacro),
  isSubTerm(XMacro, Mnemo),
  write('\nERROR: Mnemo \''), write(Mnemo), 
  write('\' has already been used in macro declaration\n       \''),
  write(XMacro), write(' as '), write(XMnemo), 
  write('\'.\n'),
  write('       To avoid cyclic definitions, macros may not contain\n'),
  write('       the nmemos of other macros, that are declared later.\n').

/*
---- isUsedMnemo(Mnemo)
----
Succeeds, if ~Mnemo~ is already used in ~storedMacro/2~.

*/

isUsedMnemo(Mnemo) :-
  storedMacro(Mnemo,_),
  write('\nERROR: Mnemo \''), 
  write(Mnemo), 
  write('\' was used more than once.\n').


/*
---- expandMacros(+QueryIn, -QueryOut)
----
Used the table of ~storedMacro(Mnemo,Expansion)~ to expand any macros found
in ~QueryIn~ and returns the expanded query in ~QueryOut~.

A table ~storedFlatMacro(Mnemo,Expansion)~ is built, which holds macros that
have been flattened, i.e. all nested mnemos have been expanded with their
corresponding macros.

*/

expandMacros(QueryIn, QueryOut) :-
  flattenAllMacros,
  listAllMacros(Macros),
  expandSingleMacro(QueryIn,Macros,QueryOut).

expandSingleMacro(QueryIn, [], QueryIn).
expandSingleMacro(QueryIn, [Mnemo|Rest], QueryOut) :-
  storedFlatMacro(Mnemo, Macro),
  replace_term(QueryIn,Mnemo,Macro,TermOut),
  expandSingleMacro(TermOut, Rest, QueryOut).

listAllMacros(List) :-
  findall(Mnemo,storedMacro(Mnemo,_),ListR),
  reverse(ListR,List).

% extend a list of macros within MacroIn
flattenMacro(MacroIn,[],MacroIn).
flattenMacro(MacroIn,[N2|N2s],MacroOut) :-
  storedMacro(N2, M2),
  replace_term(MacroIn,N2,M2,MacroTemp),
  flattenMacro(MacroTemp,N2s,MacroOut).

flattenMacro(Mnemo) :-
  storedMacro(Mnemo, Macro),
  listAllMacros(Macros),
  flattenMacro(Macro,Macros,MacroFlat),
  assert(storedFlatMacro(Mnemo,MacroFlat)).

flattenMacros :-
  storedMacro(Mnemo,_),
  flattenMacro(Mnemo),
  fail.

flattenAllMacros :- not(flattenMacros).


/*

14.2 Inference of Predicates

*/

rewriteQueryForInferenceOfPredicates(Query, Query) :-
  not(optimizerOption(rewriteInference)), !.

rewriteQueryForInferenceOfPredicates(Query, RewrittenQuery) :-
  optimizerOption(rewriteInference),
  rewriteQueryForNonempty(Query, RQuery1),
  rewriteQueryForInferredPredicates(RQuery1, RewrittenQuery),
  dm(rewriteMacros,['\nREWRITING: Inference of Predicates\n\tIn:  ',
                    Query,'\n\tOut: ',RewrittenQuery,'\n\n']).


/*

14.2.1 Nonempty-Queries

Spatial data may contain `undefined' data and empty sets. Often, a user wants to
suppress empty results. To this end, the kewword `nonempty' within the select 
clause is introduced. A nonempty-query will automatically infer conditions
from the statements within the select clause, that guarantee, that only 
nonempty data will be reported, and rewrite the query: The additional 
predicates are added to the where clause of the query.

Rules to infer predicates. The code should be moved to file operators.pl

*/

rewritingNonemptyRule(val(X atinstant Y), [X present Y]).
rewritingNonemptyRule(X atperiods Y,      [X present Y]).
rewritingNonemptyRule(X at Y,             [X passes Y]).
rewritingNonemptyRule(X when P,           [X satisfies P]).
rewritingNonemptyRule(intersection(X, Y), [X intersects Y]).
rewritingNonemptyRule(minus(X, Y),        [Y inside X]).
rewritingNonemptyRule(X,                  [not(isempty(X))]) :- 
  X =.. [OP|_],
  isBBoxOperator(OP).


/*
---- inferNonemptyPredicates(-InferredPreds)
----
uses rules defined by ~inferNonemptyPredicate/2~ to detect expressions
within the term indexed by calling ~findCSEs/3~  that allow to infer 
nonempty-predicates. The latter are collected within list ~InferredPred~
and returned.

~inferSingleNonemptyPredicate(-Pred)~ searches the dynamic predicate
~storedExpressionLabel/4~ and the table ~inferNonemptyPredicate/2~ for 
any matches and returns the predictate inferred ~Pred~.

*/

inferNonemptyPredicates(NewPreds) :-
  findall(Pred, inferSingleNonemptyPredicate(Pred), NewPredsListList),
  flatten(NewPredsListList,NewPreds).

inferSingleNonemptyPredicate(Pred) :-
  storedExpressionLabel(Trigger, _, _, _),  
  rewritingNonemptyRule(Trigger, Pred).
   
/*
---- rewriteQueryForNonempty(+Query,-RewrittenQuery)
----

rewrites ~Query~ to contain conditions that force Secondo
to suppress empty data within the result of the query.

*/


% Skip, if optimizerOption(rewriteNonempty) undefined
rewriteQueryForNonempty(Query, Query) :-
  not(optimizerOption(rewriteNonempty)), !.

% Normal query (without nonempty)
rewriteQueryForNonempty(Query, Query) :-
  Query \= from(select(nonempty(_)),_),
  Query \= from(select(distinct(nonempty(_))),_),
  retractExpressionLabels,
  dm(rewrite,['\nREWRITING: Inference of Nonenpty predicates\n\tIn:  ',
              Query,'\n\tOut: ',Query,'\n\n']),
  !.

% Ordinary nonempty-queries with distinct
rewriteQueryForNonempty(Query, RewrittenQuery) :-
  Query 
     = from(select(distinct(nonempty(SelectClause))),where(Rels,WhereClause)),
  retractExpressionLabels,
  findCSEs(select(SelectClause),all,_),
  inferNonemptyPredicates(NonEmptyConditions),
  ( is_list(WhereClause) -> 
      append(WhereClause, NonEmptyConditions, RewrittenWhereClause)
    ; RewrittenWhereClause = [WhereClause|NonEmptyConditions]
  ),
  sort(RewrittenWhereClause,RewrittenWhereClause1),
  RewrittenQuery 
     = from(select(distinct(SelectClause)),where(Rels,RewrittenWhereClause1)),
  dm(rewrite,['\nREWRITING: Inference of Nonenpty predicates\n\tIn:  ',
              Query,'\n\tOut: ',RewrittenQuery,'\n\n']),
  !.
  
% Ordinary nonempty-queries w/o distinct
rewriteQueryForNonempty(Query, RewrittenQuery) :-
  Query = from(select(nonempty(SelectClause)),where(Rels,WhereClause)),
  retractExpressionLabels,
  findCSEs(select(SelectClause),all,_),
  inferNonemptyPredicates(NonEmptyConditions),
  ( is_list(WhereClause) -> 
      append(WhereClause, NonEmptyConditions, RewrittenWhereClause)
    ; RewrittenWhereClause = [WhereClause|NonEmptyConditions]
  ),
  sort(RewrittenWhereClause,RewrittenWhereClause1),
  RewrittenQuery = from(select(SelectClause),where(Rels,RewrittenWhereClause1)),
  dm(rewrite,['\nREWRITING: Inference of Nonenpty predicates\n\tIn:  ',
              Query,'\n\tOut: ',RewrittenQuery,'\n\n']),
  !.

% Special cases: nonempty with empty where clause and distinct
rewriteQueryForNonempty(Query, RewrittenQuery) :-
  Query \= from(select(distinct(nonempty(_))),where(_,_)),
  Query  = from(select(distinct(nonempty(SelectClause))),Rels),
  retractExpressionLabels,
  findCSEs(select(SelectClause),all,_),
  inferNonemptyPredicates(NonEmptyConditions),
  sort(NonEmptyConditions,NonEmptyConditions1),
  ( NonEmptyConditions1 = [] -> 
      RewrittenQuery = from(select(distinct(SelectClause)),Rels)
    ; RewrittenQuery 
       = from(select(distinct(SelectClause)),where(Rels,NonEmptyConditions1))
  ),
  dm(rewrite,['\nREWRITING: Inference of Nonenpty predicates\n\tIn:  ',
              Query,'\n\tOut: ',RewrittenQuery,'\n\n']),
  !.
  
% Special cases: nonempty with empty where clause but w/o distinct
rewriteQueryForNonempty(Query, RewrittenQuery) :-
  Query \= from(select(nonempty(_)),where(_,_)),
  Query  = from(select(nonempty(SelectClause)),Rels),
  retractExpressionLabels,
  findCSEs(select(SelectClause),all,_),
  inferNonemptyPredicates(NonEmptyConditions),
  sort(NonEmptyConditions,NonEmptyConditions1),
  ( NonEmptyConditions1 = [] -> 
      RewrittenQuery = from(select(SelectClause),Rels)
    ; RewrittenQuery 
       = from(select(SelectClause),where(Rels,NonEmptyConditions1))
  ),
  dm(rewrite,['\nREWRITING: Inference of Nonenpty predicates\n\tIn:  ',
              Query,'\n\tOut: ',RewrittenQuery,'\n\n']),
  !.

% Special cases: ordering and grouping clauses can be 
%                ignored for nonempty-rewriting.
rewriteQueryForNonempty(Query, RewrittenQuery) :-
  Query = first(Query2, X),
  rewriteQueryForNonempty(Query2, RewrittenQuery2),
  RewrittenQuery = first(RewrittenQuery2,X),
  dm(rewriting,'6\n'),
  dm(rewrite,['\nREWRITING: Inference of Nonenpty predicates\n\tIn:  ',
              Query,'\n\tOut: ',RewrittenQuery,'\n\n']),
  !.

rewriteQueryForNonempty(Query, RewrittenQuery) :-
  Query = orderby(Query2, X),
  rewriteQueryForNonempty(Query2, RewrittenQuery2),
  RewrittenQuery = orderby(RewrittenQuery2,X),
  dm(rewrite,['\nREWRITING: Inference of Nonenpty predicates\n\tIn:  ',
              Query,'\n\tOut: ',RewrittenQuery,'\n\n']),
  !.

rewriteQueryForNonempty(Query, RewrittenQuery) :-
  Query = groupby(Query2, X),
  rewriteQueryForNonempty(Query2, RewrittenQuery2),
  RewrittenQuery = groupby(RewrittenQuery2,X),
  dm(rewrite,['\nREWRITING: Inference of Nonenpty predicates\n\tIn:  ',
              Query,'\n\tOut: ',RewrittenQuery,'\n\n']),
  !.

/*

14.2.2 Inference of Conditions

If the where clause contains certain sets of conditions, these conditions may be
extended by additional ones, e.g. in order to force the use of available indices.

*/

%:- [inference]. % XRIS: load module for inference machine

%rewriteQueryForInferredPredicates(Query,Query) :- % XRIS: for Testing Only
%  dm(rewrite,['\nREWRITING: Inferred predicates\n\tIn:  ',
%              Query,'\n\tOut: ',Query,'\n\n']),
%  !.

rewriteQueryForInferredPredicates(QIn, QIn) :-
  % case: no where clause
  QIn = from(select(_),WhereClause),
  WhereClause \= where(_,_),
  dm(rewrite,['\nREWRITING: Inferred predicates\n\tIn:  ',
              QIn,'\n\tOut: ',QIn,'\n\n']),
  !.

rewriteQueryForInferredPredicates(QIn, QOut) :-
  % case: non-empty where clause
  QIn = from(select(SelClause),where(Rels,WhereIn)),
  analyseConditions(WhereIn,WhereOut),
  QOut = from(select(SelClause),where(Rels,WhereOut)),
  dm(rewrite,['\nREWRITING: Inferred predicates\n\tIn:  ',
              QIn,'\n\tOut: ',QOut,'\n\n']),
  !.

% Special cases: ordering and grouping clauses can be ignored
rewriteQueryForInferredPredicates(Query, RewrittenQuery) :-
  Query = first(Query2, X),
  rewriteQueryForInferredPredicates(Query2, RewrittenQuery2),
  RewrittenQuery = first(RewrittenQuery2,X),
  dm(rewrite,['\nREWRITING: Inferred predicates\n\tIn:  ',
              Query,'\n\tOut: ',RewrittenQuery,'\n\n']),
  !.

rewriteQueryForInferredPredicates(Query, RewrittenQuery) :-
  Query = orderby(Query2, X),
  rewriteQueryForInferredPredicates(Query2, RewrittenQuery2),
  RewrittenQuery = orderby(RewrittenQuery2,X),
  dm(rewrite,['\nREWRITING: Inferred predicates\n\tIn:  ',
              Query,'\n\tOut: ',RewrittenQuery,'\n\n']),
  !.

rewriteQueryForInferredPredicates(Query, RewrittenQuery) :-
  Query = groupby(Query2, X),
  rewriteQueryForInferredPredicates(Query2, RewrittenQuery2),
  RewrittenQuery = groupby(RewrittenQuery2,X),
  dm(rewrite,['\nREWRITING: Inferred predicates\n\tIn:  ',
              Query,'\n\tOut: ',RewrittenQuery,'\n\n']),
  !.


/*
---- analyseConditions(+WhereIn,-WhereOut)
----

Analyze the predicates and infer additional conditions which are added to 
the where-clause, e.g. to inforce the use of indices etc.

*/

analyseConditions(WhereIn, WhereOut) :- 
  makeList(WhereIn, WhereInList),
  list_to_set(WhereInList, WhereInSet),
  findall(X, inferPredicate(WhereInSet, X), NewPredicates),
  flatten(NewPredicates, NewPredicatesFlat),
  append(WhereInSet, NewPredicatesFlat, WhereOutList), 
  list_to_set(WhereOutList, WhereOut), !.

% rules to infer additional predicates

/*

----
% this rule is an ad-hoc solution, until the bbox(moving point) is implemented
inferPredicate(Premises, [box3d(bbox(trajectory(X)),deftime(X)) intersects box3d(bbox(Z),Y)]) :-
  member(X present Y, Premises),
  member(X passes Z,  Premises),
  X \= Y, X \= Z, Y \= Z, !.
----

*/

% This is the better solution to replace the ad-hoc one
inferPredicate(Premises, [bbox(X) intersects box3d(bbox(Z),Y)]) :-
  member(X present Y, Premises),
  member(X passes Z,  Premises),
  X \= Y, X \= Z, Y \= Z.


/*
The following rules are deprecated, since generic rules for operators ~OP~ with
~isBBoxPredicate(OP)~ have been introduced to ~[=>]/2~ in file ``optimizer.pl''.
These rules should inforce the use of available indices.

----
inferPredicate(Premises, [X overlaps Y]) :-
  member(X touches Y, Premises),
  X \= Y.

inferPredicate(Premises, [X intersects Y]) :-
  member(X inside Y, Premises),
  X \= Y.

inferPredicate(Premises, [X intersects Y]) :-
  member(X overlaps Y, Premises),
  X \= Y.
----

*/


/*

14.3 Redundancy Elimination

Redundant clauses should be removed.

*/

/*
---- rewriteQueryForRedundancy(+Query,-RewrittenQuery).
----

rewrites ~Query~ by dropping redundant where clauses and optimizing
equivalent expressions. At the moment, only doublets within the list of where
conditions are dropped.

*/

rewriteQueryForRedundancy(Query, RewrittenQuery) :-
  Query = first(X),
  rewriteQueryForRedundancy(X, RX),
  RewrittenQuery = first(RX),
  dm(rewrite,['\nREWRITING: Remove redundant predicates\n\tIn:  ',
              Query,'\n\tOut: ',RewrittenQuery,'\n\n']),
  !.

rewriteQueryForRedundancy(Query, RewrittenQuery) :-
  Query = orderby(X,Y),
  rewriteQueryForRedundancy(X, RX),
  RewrittenQuery = orderby(RX,Y),
  dm(rewrite,['\nREWRITING: Remove redundant predicates\n\tIn:  ',
              Query,'\n\tOut: ',RewrittenQuery,'\n\n']),
  !.

rewriteQueryForRedundancy(Query, RewrittenQuery) :-
  Query = groupby(X,Y),
  rewriteQueryForRedundancy(X, RX),
  RewrittenQuery = groupby(RX,Y),
  dm(rewrite,['\nREWRITING: Remove redundant predicates\n\tIn:  ',
              Query,'\n\tOut: ',RewrittenQuery,'\n\n']),
  !.

rewriteQueryForRedundancy(Query, RewrittenQuery) :-
  Query = from(select(SelClause), where(Rels,WhereClause)),  
  list_to_set(WhereClause,RewrittenWhereClause),  % eliminate condition doublets  
  (RewrittenWhereClause = [] -> 
     (RewrittenQuery = from(select(SelClause), Rels))
   ; (RewrittenQuery 
       = from(select(SelClause), where(Rels,RewrittenWhereClause)))
  ),
  dm(rewrite,['\nREWRITING: Remove redundant predicates\n\tIn:  ',
              Query,'\n\tOut: ',RewrittenQuery,'\n\n']),
  !.

rewriteQueryForRedundancy(Query,Query) :-
  dm(rewrite,['\nREWRITING: Remove redundant predicates\n\tIn:  ',
              Query,'\n\tOut: ',Query,'\n\n']),
  !.


/*

14.4 Handle Common SubExpressions (CSEs)

The repeated evaluation of common subexpressions (CSEs) should be avoided, 
especially if complex funtions are used or big objects are created. Intermediary
results can be stored by extending the tuples by an addiditonal attribute using
the CSE as a function to calculate the attribute values. The extend should be 
called immeadiatly before the CSE occurs for the first time; and it should be 
removed immeadetely after its last occurence (in sequence of evaluation). 
If the CSE is named using the keyword ``as'' in the select-clause, no remove 
is needed and the final rename can be omitted. The same holds for queries, where 
the remove would be inserted directly in front of a final consume(project[...]).

When replacing occurences of CSEs, one starts at the bottom of the operator tree
and inserts the extend beneath the first occurence. Then, the path to the root
is followed upward, replacing all further occurences of the CSE with the 
extended attribute.

*/

/*
---- rewriteQueryForCSE(+Query,-RewrittenQuery)
----

Prepares ~Query~ such that the optimizer will avoid repetitive evaluation of 
~expensive~ common subexpressions (CSEs). A CSE is expensive, if it contains at 
least one operator labeled by ~rewritingCSEExpensiveOP/1~. The definitions of 
~rewritingCSEExpensiveOP/1~ should be moved to file operators.pl later.

The processing scheme for CSEs will optimize the unchanged query, but replace
CSEs within the plan later on by virtual attributes. To continue the use of
virtual attributes, the non-conjunctive parts of the query must still be
processed separately.

To this end, the first (left) argument of functor ~from/2~ is modified to 
allow the (re-) use of virtual attributes within the select-clause.

*/

rewriteQueryForCSE(Query, Query) :-
  not(optimizerOption(rewriteCSE)), !.

rewriteQueryForCSE(QueryIn, QueryIn) :-
  optimizerOption(rewriteCSE),
  retractExpressionLabels,          % delete old data
  dc(rewriteCSE,showExpressionLabels),   % output debugging info
  findCSEs(QueryIn, expensive, _),  % find all expensive subexpressions
  dc(rewriteCSE,showExpressionLabels),   % output debugging info
  retractNonCSEs,                   % delete expressions used less than twice
  dc(rewriteCSE,showExpressionLabels),   % output debugging info
  replaceAllCSEs,                   % nest CSEs but also save flat CSE expression
  dc(rewriteCSE,showExpressionLabels),   % output debugging info
%  compactCSEs(QueryIn, QueryOut, _), % replace CSEs by virtual attributes
%  dm(rewriteCSE,['\nREWRITING: Substitute Common Subexpressions\n\tIn:  ',
%              QueryIn,'\n\tOut: ',QueryOut,'\n\n']),
  !.

  
/*

14.4.1 Auxiliary Predicates to ~rewriteQueryForCSE/2~

---- isSubTerm(+Term,+SubTerm)
     isSubTermNotEqual(+Term,+SubTerm)
----

succeeds, iff ~SubTerm~ is a (not equal) subterm of ~Term~.

*/

isSubTermNotEqual(Term,SubTerm) :-
  Term \= SubTerm,
  isSubTerm(Term,SubTerm).

isSubTerm(Term,Term).
isSubTerm(Term, Subterm) :-
  compound(Term),
  Term =.. [_|Args],
  isSubTerm1(Args, Subterm).

isSubTerm1([],_) :- fail.
isSubTerm1([Me|Others],Subterm) :-
    isSubTerm(Me,Subterm)
  ; isSubTerm1(Others,Subterm).


/*
---- replace_term(+Term, +SubExpr, +Replacement, -Result)
----

Replace any occurences of ground term ~SubExpr~ in expression ~Term~ with 
ground term ~Replacement~ and return the ~Result~.

*/

replace_termL([], _, _, []).
replace_termL([Me|Others], SubExpr, Repl, [MeRepl|OthersRepl]) :-
  replace_term(Me, SubExpr, Repl, MeRepl),
  replace_termL(Others, SubExpr, Repl, OthersRepl).

replace_term(Term, Term, Replacement, Replacement).

replace_term(Term, SubExpr, Replacement, Result) :-
  compound(Term),
  Term \= SubExpr,
  Term =.. [Me|Args],
  replace_termL(Args, SubExpr, Replacement, ArgsResult),
  Result =.. [Me|ArgsResult].

replace_term(Term, SubExpr, _, Term) :-
  atomic(Term),
  Term \= SubExpr.

/*
---- findCSEs(+Node, +Mode, -Expensive)
----

Creates a dynamic table ~storedExpressionLabel(FlatExpr,Label,NoOccurences,CompactExp)~ by
parsing the operator tree for term ~node~. 

Depending on ~Mode~, either all subexpressions are idexed (Mode = all), or only 
expensive subexpressions (Mode \= all) are indexed.

Each sub-expression ~FlatExpr~ is labeled with an unique identifier ~label~ and the
number of encountered occurrences ~NoOccurences~ of ~FlatExpr~.

If a term contains an expensive operator (as indicated by a defined fact 
~rewritingCSEExpensiveOP(OP)~), ~Expensive~ will be 1, 0 
otherwise.

If a complete CSE has an alias (like `CSE as Alias'), its label is Alias rather
than the canonical label cse\_N.

*/

:- dynamic(storedExpressionLabel/4),
   reset_gensym(cse_). % reset unique identifier generator

% delete table of label-term associations
retractExpressionLabels :-
  retractall(storedExpressionLabel(_, _, _, _)),
  retractall(storedFlatCSE(_)),
  reset_gensym(cse_). % reset unique identifier generator

% print a table of all stored term-label associations
showExpressionLabel :-
  storedExpressionLabel(FlatExpr,Label,NoOcc,CompactExpr),
  write(' '), write(Label), write('   '), write(NoOcc), 
  write('\t'), write(FlatExpr), write('   '), write(CompactExpr), nl,
  fail.

showExpressionLabels :-
  write('\nLabeled Expressions:\n'),
  findall(_,showExpressionLabel,_),
  nl.
  
% return the label associated with a known Node, or create a new association 
% and return that new label 
% if called with Label being a bound variable or term, the latter is used as
% the label. If a label has already been assigned to an identical term, it
% is overwritten with Label.
getExpressionLabel(Node, Label) :-
  var(Label),
  storedExpressionLabel(Node, Label, NoOcc, X), 
  retractall(storedExpressionLabel(_, Label, _, _)),
  NoOcc1 is NoOcc + 1,
  !,
  assert(storedExpressionLabel(Node, Label, NoOcc1, X)).

getExpressionLabel(Node, Label) :-
  var(Label),
  not(storedExpressionLabel(Node, _, _, _)),
  gensym(cse_, Label),
  !,
  assert(storedExpressionLabel(Node, Label, 1, *)).

getExpressionLabel(Node, Label) :- % case: Label unknown, Node unknown
  nonvar(Label),
  not(storedExpressionLabel(_, Label, _, _)), 
  not(storedExpressionLabel(Node, _, _, _)),
  !,
  assert(storedExpressionLabel(Node, Label, 1, *)).

getExpressionLabel(Node, Label) :- 
% case: Label unknown, Node known - Change Label if OldLabel is canonical
  nonvar(Label),
  not(storedExpressionLabel(_, Label, _, _)), 
  storedExpressionLabel(Node, OldLabel, NoOcc, X),
  sub_atom(OldLabel, 0, _, _, cse_),
  NoOcc1 is NoOcc + 1,
  !,
  retractall(storedExpressionLabel(_, OldLabel, _, _)),
  assert(storedExpressionLabel(Node, Label, NoOcc1, X)).


getExpressionLabel(Node, Label) :- 
% case: Label unknown, Node known - Forget the new label, but increase the old 
%       counter
% This can happen if a column is multiplied in the output 
% (e.g. select[Expr as n1, Expr as n2]...)
  nonvar(Label),
  not(storedExpressionLabel(_, Label, _, _)), 
  storedExpressionLabel(Node, OldLabel, NoOcc, X),
  not(sub_atom(OldLabel, 0, _, _, cse_)),
  NoOcc1 is NoOcc + 1,
  !,
  retractall(storedExpressionLabel(Node, OldLabel, NoOcc, X)),
  assert(storedExpressionLabel(Node, OldLabel, NoOcc1, X)).

getExpressionLabel(Node, Label) :- % case: Label known, Node matches - Warning
  nonvar(Label),
  storedExpressionLabel(Node, Label, NoOcc, X), 
  retractall(storedExpressionLabel(_, Label, _, _)),
  NoOcc1 is NoOcc + 1,
  !,
  assert(storedExpressionLabel(Node, Label, NoOcc1, X)),
  write('WARNING (getExpressionLabel/2): Identical alias used severalfold:\n'),
  write('\tstoredExpressionLabel('), write(Node), write(','), write(Label), 
  write(','), write(NoOcc), write(','), write(X), write(').\n\n').

getExpressionLabel(Node, Label) :- % case: Label known, Node conflicts - Error
  nonvar(Label),
  storedExpressionLabel(NodeOld, Label, _, _),
  Node \= NodeOld,
  write('Error in getExpressionLabel: Conflicting expressions for Alias \''), 
  write(Label), write('\': \n'),
  write('\tOld Expression: '), write(NodeOld), write('.\n'),
  write('\tNew Expression: '), write(Node), write('.\n\n'),
  throw(sql_ERROR(rewriting_getExpressionLabel(rewriting_getExpressionLabel(Node, Label)))),
  fail, !.


% find all CSEs
findCSEs1([], _, 0).      % nothing to do
findCSEs1([Me|Others],Mode,Expense) :-
  findCSEs1(Others, Mode, OthersExpense),
  findCSEs(Me, Mode, MyExpense),
  Expense is MyExpense \/ OthersExpense, !.

findCSEs(Node, _, 0) :-   % Node is a leaf. No label is assigned to leaves.
  ( atomic(Node) ; Node = :(_,_) ), !.

findCSEs(NodeList, Mode, Expense) :-
  is_list(NodeList),
  findCSEs1(NodeList, Mode, Expense), !.

findCSEs(Node, Mode, 1) :- % Don't recurse into term, if term is already known
  not(optimizerOption(rewriteCSEall)),
  Mode \= all,
  storedExpressionLabel(Node, _, _, _),
  getExpressionLabel(Node, _), !.

findCSEs(Node, Mode, Expense) :-
  compound(Node),                      % Node is an inner node.
  not(is_list(Node)),
  Node =.. [as, Arg, Alias],           % special case: renaming
  findCSEs_alias_case(Arg, Mode, Alias, Expense), % use specialized predicate
  ((Mode = all ; Expense = 1)          % check if Node should be indexed
    -> getExpressionLabel(Node,_)      % handle only expensive CSEs
     ; true
  ), !.

findCSEs(Node, Mode, Expense) :-
  compound(Node),             % Node is an inner node.
  not(is_list(Node)),
  Node =.. [Me|MyArgs],       % decompose node
  findCSEs1(MyArgs, Mode, ArgsExpense),
  (rewritingCSEExpensiveOP(Me) 
   -> Expense is 1
    ; Expense is ArgsExpense
  ),
  ((Mode = all ; Expense = 1)  % check if Node should be indexed
    -> getExpressionLabel(Node,_)   % handle only expensive CSEs
     ; true
  ), !.

% like findCSEs(Node, Mode, Expense), but use Alias as potential label
findCSEs_alias_case(Node, Mode, Alias, Expense) :-
  compound(Node),             % Node is an inner node.
  not(is_list(Node)),
  Node =.. [Me|MyArgs],       % decompose node
  findCSEs1(MyArgs, Mode, ArgsExpense),
  (rewritingCSEExpensiveOP(Me) 
   -> Expense is 1
    ; Expense is ArgsExpense
  ),
  ((Mode = all ; Expense = 1)  % check if Node should be indexed
    -> getExpressionLabel(Node,Alias)   % handle only expensive CSEs
     ; true
  ), !.

findCSEs_alias_case(Node, Mode, _, Expense) :-
  findCSEs(Node, Mode, Expense), !.

/*
---- compactCSEs(+Node, -NodeMarked, -Used)
----
For term ~Node~, return an equivalent term ~NodeMarked~, where all CSEs have 
been replaced by their labels (according to table storedExpressionLabel/4) 
and also return a list ~Used~ of all applied CSE-labels.

*/

% Starting rules to avoid replacement of X by X
compactCSEs(Node, NodeMarked, Used) :-
  compound(Node),            
  not(is_list(Node)),
  Node =.. [Me|MyArgs], 
  compactCSEs_1(MyArgs, MyArgsMarked, MyArgsUsed),
  NodeMarked =.. [Me|MyArgsMarked],
  Used = MyArgsUsed, !.

compactCSEs(NodeList, MarkedNodeList, Used) :-
  is_list(NodeList),
  compactCSEs_1(NodeList, MarkedNodeList, Used), !.

compactCSEs(Node, Node, []).

% Normal rules
compactCSEs_1([],[],[]).           % nothing to do
compactCSEs_1([Me|Others],[MeMarked|OthersMarked],Used) :-
  compactCSEs_1(Others, OthersMarked, OthersUsed),
  compactCSEs_(Me, MeMarked, MyUsed),
  merge_set(MyUsed, OthersUsed, Used), !.

compactCSEs_(Node, Node, []) :- 
  atomic(Node);
  Node = :(_,_), !.

compactCSEs_(NodeList, MarkedNodeList, Used) :-
  is_list(NodeList),
  compactCSEs_1(NodeList, MarkedNodeList, Used), !.

compactCSEs_(Node, NodeMarked, Used) :- % Node is a CSE
  compound(Node),            
  not(is_list(Node)),
  storedExpressionLabel(Node, MyLabel, _, _),
  NodeMarked = MyLabel,
  Used = [MyLabel], !.

compactCSEs_(Node, NodeMarked, Used) :- % Node is not a CSE
  compound(Node), 
  not(is_list(Node)),
  Node =.. [Me|MyArgs], 
  compactCSEs_1(MyArgs, MyArgsMarked, MyArgsUsed),
  ( (Me = as, MyArgsMarked = [Alias,Alias]) 
     % Special case: renaming CSE = Alias, where CSE-label = Alias
     %               The rename will then already be done during optimization
      -> NodeMarked = Alias
       ; NodeMarked =.. [Me|MyArgsMarked]
  ),
  Used = MyArgsUsed, !.

/*
---- replaceAllCSEs/0
----
Update the table of stored expressions by replacing all occurences of CSEs by 
the according label cse\_N and save that compacted expression to the forth 
(CompactExpr) argument of ~storedExpressionLabel/4~.

*/

replaceAllCSEs :- 
  findall(X, storedExpressionLabel(_,X,_, _),XList),
  replaceSingleCSEList(XList), !.

replaceSingleCSEList([]).
replaceSingleCSEList([Me|Others]) :-
  storedExpressionLabel(FlatExpr, Me, NoOcc, _),
  compactCSEs(FlatExpr,CompactExpr,_),
  retractall(storedExpressionLabel(_,Me,_,_)),
  assert(storedExpressionLabel(FlatExpr, Me, NoOcc, CompactExpr)),
  replaceSingleCSEList(Others), !.
  

/*
---- retractNonCSE/0
----
Remove non-CSE entries from the expression table.

*/

retractNonCSE :-
  storedExpressionLabel(_,Label,NoOcc, _),
  NoOcc < 2,
  retractall(storedExpressionLabel(_,Label,_,_)),
  fail.

retractNonCSEs :- not(retractNonCSE).

/*
---- replaceAllFlatCSEs/0
----

Replace CSEs within CSEs recursively with according labels cse\_N.

*/

% succeeds, if Expr contains no CSE
:- dynamic(storedFlatCSE/1).

isFlatCSE(Expr) :- storedFlatCSE(Expr), !.

isFlatCSE(Expr) :-
  storedExpressionLabel(Expr, _, _, _),
  not(isUnflatCSE(Expr)),
  assert(storedFlatCSE(Expr)), !.
  
isUnflatCSE(Expr) :-
  storedExpressionLabel(Expr, _, _, _),
  storedExpressionLabel(CSE, _, _, _),
  isSubTermNotEqual(Expr,CSE), !.


% replaces all flat CSEs within the table
replaceAllFlatCSEs :-
  findall(X,isFlatCSE(X),FlatCSEs),
  findall(Y,isUnflatCSE(Y),UnflatCSEs),
  replace_term_list(UnflatCSEs,FlatCSEs).

replace_term_list([],_) :- !.
replace_term_list(_,[]) :- !.

replace_term_list([First|Rest],FlatCSEs) :-
  storedExpressionLabel(First, FirstLabel, FirstList),
  replace_term_list1(First,FlatCSEs,Result),
  retractall(storedExpressionLabel(First, FirstLabel, FirstList)),
  asserta(storedExpressionLabel(Result, FirstLabel, FirstList)),
  replace_term_list(Rest,FlatCSEs), !.

replace_term_list1(X, [], X) :- !.
replace_term_list1(Expr, [CSE|Rest], Result) :-
  storedExpressionLabel(CSE, CSELabel, _),
  replace_term(Expr, CSE, CSELabel, Result1),
  replace_term_list1(Result1, Rest, Result), !.


% testing the predicates
analyseCSE(TermIn,TermOut) :-
  retractExpressionLabels, % delete old data
  showExpressionLabels,

  findCSEs(TermIn, expensive, _), % find all expensive subexpressions
  showExpressionLabels,

  retractNonCSEs, % delete all subexpressions used less than twice
  showExpressionLabels,

  replaceAllCSEs, % nest CSEs but also save flat CSE expression
  showExpressionLabels,

  compactCSEs_(TermIn, TermOut, _).

% examples for expensive operators
rewritingCSEExpensiveOP(*). % XRIS: for testing only
rewritingCSEExpensiveOP(intersection).
rewritingCSEExpensiveOP(minus).
rewritingCSEExpensiveOP(atinstant).
rewritingCSEExpensiveOP(atperiod). % ?
rewritingCSEExpensiveOP(at).       % ?
rewritingCSEExpensiveOP(crossings).
rewritingCSEExpensiveOP(center).
rewritingCSEExpensiveOP(touchpoints).
rewritingCSEExpensiveOP(commonborder).
rewritingCSEExpensiveOP(trajectory).
rewritingCSEExpensiveOP(distance).
rewritingCSEExpensiveOP(attached).
rewritingCSEExpensiveOP(commonborderscan).

/*
15 rewritePlan - Rewriting the Query Plan

After the plan has been generated by ~plan/2~, it can be further optimized, e.g. by
replacing CSEs with virtual attributes and removing unused attributes from the plan
as early as possible.
*/


/*
15.1 Substituting CSE by Virtual Attributes

Methods to find common subexpressions (CSEs) within the rewritten query have been described in
section 14.4. In this section, the generated plan is searched for already identified CSEs and
is modified, such that CSEs are replaced by ~virtual attributes~ whenever this is possible.

Virtual attributes are attributes that are inserted into the tuples and whose values are 
calculated from already available attributes by using a CSE. The according extend operators
are inserted into the plan as late as possible, but as early as needed.

While attributes not used at all within a query are removed by initial project operators directly
after the feed, attributes (both from base relations and virtual attributes) might thrash the memory.
To avoid this, in a subsequent step, they can be removed from the streams as early as possible
(when ~optimizerOption(rewriteRemove)~ is defined). In all cases, virtual attributes are removed
within the translation of the select clause, either by the standard procedure, or by dedicated
clauses added to ~selectClause/4~ in file ``optimizer.pl'' in case of ``select [star]''-queries.

The substitution of CSEs is done in a bottom-up trace through the operator-tree represented by the 
query plan. For each stream, a set of contained attributes is propagated upwards. Attribute sets 
are initialised using the ~variable~ and ~usedAttr~ facts stored in the lookup-step of the query 
preprocessing. If an extend or remove operator is encountered, the attribute set is updated, for 
each join, the sets of attributes from both argument streams get merged.

All predicates within the plan are analyzed, whether they contain CSEs identified in section 14.4. 
If a CSE is detected, the set of available attributes is checked to determine, whether the 
corresponding virtual attributes is available. If it is available, the CSE is replaced directly with the 
virtual attribute. Otherwise, it is tried to extended as many virtual attributes occuring in the 
compacted CSE as possible to the tuple (last including the virtual attribute for the CSE itself).
The set of available attributes to be passed upward is adjusted to the extends.

The select-clause of the query must be considered for its own.

*/

/*
15.1.0 Integrating Plan Rewriting for CSE Substitution with the Optimizer

---- rewritePlanforCSE(+PlanIn, -PlanOut, +SelectIn, -SelectOut)
----
Carries out Plan Rewriting for plan ~PanIn~ and select clause ~SelectIn~, 
returning the rewritten plan ~PlanOut~ and the rewritten select clause 
~SelectOut~.

The steps actually carried out by the predicate depend on the settings of
~optimizerOption/1~.

The set of all available attributes available at the stream resulting from 
~PlanOut~ is stored in a dynamic predicate ~storedAvailStreamAttributes/1~
for further processing steps (e.g. removal of unused attributes within a 
starquery).

Called by predicate ~translate~.

*/

:- dynamic(storedAvailStreamAttributes/1).
:- dynamic(rewritePlanInsertedAttribute/1). % Table of attributes extended to the streams

rewritePlanforCSE(PlanIn, PlanOut, SelectIn, SelectOut) :-
  optimizerOption(rewriteCSE),
  retractall(rewritePlanInsertedAttribute(_)),
  registerCSEs,
  insertExtend(PlanIn, Plan2, [], AvailAttrs),
  dm(rewritePlan,['\n\nrewritePlanforCSE: Attrs avail after conj query: ',
                   AvailAttrs,'.\n']),
  lookupCSESelect(SelectIn,SelectOut,AvailAttrs,ExtensionSequence,SelectAttrs), 
  dm(rewritePlan,['\nrewritePlanforCSE: \n   Original Select: ',SelectIn,
                  '\n   Rewritten Select:',SelectOut,'.\n']),
  dm(rewritePlan,['rewritePlanforCSE: Attrs needed in Select clause: ',
                  SelectAttrs,'.\n']),
  dm(rewritePlan,['rewritePlanforCSE: ExtensionSequence', ExtensionSequence, '\n']),
  removeUnusedAttrs(Plan2,Plan3,SelectAttrs), 
  removeDublets(ExtensionSequence,AvailAttrs,ExtensionSequence2),
  dm(rewritePlan,['rewritePlanforCSE: Extend with virt attrs ',
                  ExtensionSequence2,'.\n']),
  extendPhrase(Plan3, ExtensionSequence2, PlanOut), 
  union(ExtensionSequence2,SelectAttrs,AllAvailAttrs),
  dm(rewritePlan,['rewritePlanforCSE: Available attrs for Select: ',
                  AllAvailAttrs,'.\n']),
  retractall(storedAvailStreamAttributes(_)),
  assert(storedAvailStreamAttributes(AllAvailAttrs)),
  dc(rewritePlan,(write('rewritePlanforCSE: Rewritten plan:\n\n'),
                  wp(PlanOut),nl,nl)),
  !.

rewritePlanforCSE(Plan, Plan, Select, Select).

/*
15.1.1 Register Virtual Attributes for CSE Substitution

---- registerCSEs/0
----

Registers virtual attributes for all CSEs with the datastructures for CSE
substitution.

Virtual attributes to replace CSEs are stored in a table
~virt\_attr(Name, CompactExpr, FlatSimpleExpr, UsedAttrs, UsedRels)~

  * ~Name~ is the name of the virtual attribute (the field ~label~ from ~storedExpressionLabel/4~)
 
  * ~CompactExpr~ is the compact form of the CSE propably using other virtual attributes.
    The expression is formatted in a way, such that it can be directly used in plans (using
    ~attr/3~).

  * ~FlatSimpleExpr~ is a simle form of the flat CSE not using ~attr/3~, but simple attribute names and
    alias:attributename instead. It is used to find CSEs within plans.

  * ~UsedAttrs~ is the list of all simply formatted virtual and base attributes directly occuring 
    in ~CompactExpr~. It is used to check, whether a virtual attribute can be extended to
    a stream or not.

  * ~UsedRels~ is a list of relations used in the CSE in sequence of occurency.
 
*/

:- dynamic(virt_attr/5).

registerCSEs :-
  retractall(virt_attr(_,_,_,_,_)),
  findall(Label,storedExpressionLabel(_,Label,_,_),CSE_list),
  registerCSE(CSE_list,_), !.

% Working version - registerCSE(+InList,-DoneList)
registerCSE([],[]).

registerCSE([Label|Others],Done) :-  
  storedExpressionLabel(ExprFlat, Label, _, ExprCompact), 
  ( not(virt_attr(Label, _, _, _, _)) 
    -> ( registerCSE1(ExprFlat, ExprFlattened, [], UsedRels),
         directAttributes(ExprCompact, BaseAttrs, VirtAttrsComp),
         registerCSE(VirtAttrsComp, Done1), % first register needed CSEs
         registerCSE1(ExprCompact, ExprCompact2, [], _),
         simpleExpr(ExprFlattened,ExprFlatSimple),
         union(BaseAttrs,VirtAttrsComp,AttrsUsed),
         assert(virt_attr(Label, ExprCompact2, ExprFlatSimple, 
                          AttrsUsed, UsedRels)),
         !,
         union([Label], Done1, Done2), 
         subtract(Others,Done2,Others2), % update CSE waiting list
         registerCSE(Others2, Done3),    % call remaining CSEs from waiting list
         union(Done2, Done3, Done)
       )
    ;  ( registerCSE(Others, Done1),
         union([Label],Done1,Done) 
       )
  ), !.

registerCSE1(VirtualAttr, attr(VirtualAttr, 1, u), RelsBefore, RelsAfter) :-
  virt_attr(VirtualAttr, _, _, _, Base_relations), % already stored virt attr
  mergeRelations(RelsBefore, Base_relations, RelsAfter, 0, _), !.

registerCSE1(Var:Attr, attr(Var:Attr2, Index, Case), RelsBefore, RelsAfter) :-
  variable(Var, Rel2), !, Rel2 = rel(Rel, _, _),
  spelled(Rel:Attr, attr(Attr2, _, Case)),
  ( memberchk(Rel2, RelsBefore) 
      -> RelsAfter = RelsBefore
       ; append(RelsBefore, [Rel2], RelsAfter)
  ),
  nth1(Index,RelsAfter,Rel2), !.

registerCSE1(Attr, attr(Attr2, Index, Case), RelsBefore, RelsAfter) :-
  isAttribute(Attr, Rel), !,
  spelled(Rel:Attr, attr(Attr2, _, Case)),
  queryRel(Rel, Rel2),
  ( memberchk(Rel2, RelsBefore)
      -> RelsAfter = RelsBefore
       ; append(RelsBefore, [Rel2], RelsAfter)
  ),
  nth1(Index,RelsAfter,Rel2), !.

registerCSE1(Term, Term2, RelsBefore, RelsAfter) :-
  compound(Term),
  Term =.. [Op|Args],
  registerCSE2(Args, Args2, RelsBefore, RelsAfter),
  Term2 =.. [Op|Args2], !.  

registerCSE1(Term, Term, Rels, Rels) :-
  atom(Term),
  not(is_list(Term)),
  write('Symbol \''), write(Term), 
  write('\' not recognized, supposed to be a Secondo object.'), nl, !.

registerCSE1(Term, Term, Rels, Rels).
 
registerCSE2([], [], RelsBefore, RelsBefore).

registerCSE2([Me|Others], [Me2|Others2], RelsBefore, RelsAfter) :-
  registerCSE1(Me,     Me2,     RelsBefore,  RelsAfterMe),
  registerCSE2(Others, Others2, RelsAfterMe, RelsAfter), !.

/*
---- directAttributes(+Term,-BaseAttrs,-VirtAttrs)
----
Collects all base and virtual attributes directly occurring within ~Term~,
without resolving the dependencies of virtual attributes recursively.

The predicate uses only the simple form of terms (without ~attr/3~).

May be called only after lookupRels!

*/

directAttributes([],[],[]) :- !.

directAttributes([A|B],Base,Virt) :-
  directAttributes(A,BaseA,VirtA),
  directAttributes(B,BaseB,VirtB),
  union(BaseA, BaseB, Base),
  union(VirtA,VirtB,Virt), !.

directAttributes(VirtAttr,[],[VirtAttr]) :-
  storedExpressionLabel(_,VirtAttr,_,_), !.
  
directAttributes(Var:Attr,[Var:Attr],[]) :- !.

directAttributes(Term,BaseAttrs,VirtAttrs) :-
  compound(Term),
  not(is_list(Term)),
  Term =.. [_|Args],
  directAttributes(Args,BaseAttrs,VirtAttrs), !.

directAttributes(Attr,[Attr],[]) :- 
  atomic(Attr), 
  isAttribute(Attr, _),
  !.

directAttributes(X,[],[]) :-
  atomic(X), !.

/*
---- mergeRelations(+RelsBefore,+RelsAdd,-RelsResult,+IndexIn,-IndexOut)
----
Auxiliary predicate. Given a list of relations ~RelsBefore~, 
and a second list of relations ~RelsAdd~, both lists will be merged, maintaining
the given ordering, but avoiding dublets. Furthermore, it will return the binary
encoded index of ~RelsAdd~ with respect to ~RelsResult~. ~IndexIn~ should be
initialized to 0.

*/

mergeRelations(Before, [], Before, IndexIn, IndexIn) :- !.
mergeRelations(Before, [First|Rest], After, IndexIn, IndexOut) :-
  ( not(memberchk(First,Before))
      -> append(Before,[First],Intermediate)
       ; Intermediate = Before
  ),
  nth1(Index,Intermediate,First),
  IndexIntermediate is IndexIn + 2**Index,
  mergeRelations(Intermediate, Rest, After, IndexIntermediate, IndexOut), 
  !.


/*
15.1.2 Rewriting the Plan for the Where-Clause

---- insertExtend(+PlanIn, -PlanOut, +AvailableAttrsIn, -AvailableAttrsOut).
----
Insert all neccessary extend operators to substitute CSEs as efficient as 
possible. ~PlanIn~ is the original plan, ~PlanOut~ the modified plan. 
~AvailableAttrsIn~ is a set of attributes available at an input stream to the 
lowest level of the plan, if no relation is used. This is only meaningful for 
incomplete plans, like that generated to represent the select-clause. The stream
should be marked by the term ``incomplete\_subplan'' within ~PlanIn~.

While ~AvailableAttrsIn~ is handed down, ~AvailableAttrsOut~ is handed up 
during the evaluation of the predicate. Both represent sets of available 
attributes. 

*/

% Case: remove (Can be ignored, as (so far) only used as extend-join-remove
% Case: extend (Can be ignored, as (so far) only used as extend-join-remove

% Case: special rule for handling input of incomplete subplans
% Simply pass up the set of attributes that has been declared available at label 
% ''incomplete_subplan''
insertExtend(incomplete_subplan, incomplete_subplan, AttrsIn, AttrsIn).

% Case: feed(rel(Name,Alias,Case))
insertExtend(feed(rel(Name,Alias,Case)), feed(rel(Name,Alias,Case)), 
             _, AttrsOut) :-
  relation(Name,AttrsOutL),
  list_to_set(AttrsOutL,AttrsOut),
  dm(insertExtend,['insertExtend - avail attrs: feed(rel)) = ',AttrsOut,'\n']),
  !.

% Case: rename
insertExtend(rename(Arg,Var),rename(ArgE,Var),AttrsIn,AttrsOut) :-
  insertExtend(Arg,ArgE,AttrsIn,AttrsOutE),
  findall( X,
         ( member(Y,AttrsOutE),
           X = Var:Y
         ),
         AttrsOutL),
  list_to_set(AttrsOutL,AttrsOut),   
  dm(insertExtend,['insertExtend - avail attrs: rename = ',AttrsOut,'\n']),
  !.

insertExtend(exactmatch(Index, Rel, X), 
             exactmatch(Index, Rel, X),
             AttrsIn,
             AttrsOut) :-
  insertExtend(feed(Rel),_,AttrsIn,AttrsOut), 
  dm(insertExtend,['insertExtend - avail attrs: exactmatch = ',AttrsOut,'\n']),
  !.

insertExtend(leftrange(Index, Rel, X), 
             leftrange(Index, Rel, X),
             AttrsIn,
             AttrsOut) :-
  insertExtend(feed(Rel),_,AttrsIn,AttrsOut), 
  dm(insertExtend,['insertExtend - avail attrs: leftrange = ',AttrsOut,'\n']),
  !.

insertExtend(rightrange(Index, Rel, X), 
             rightrange(Index, Rel, X),
             AttrsIn,
             AttrsOut) :-
  insertExtend(feed(Rel),_,AttrsIn,AttrsOut), 
  dm(insertExtend,['insertExtend - avail attrs: rightrange = ',AttrsOut,'\n']),
  !.

insertExtend(windowintersects(Index, Rel, X), 
             windowintersects(Index, Rel, X),
             AttrsIn,
             AttrsOut) :-
  insertExtend(feed(Rel),_,AttrsIn,AttrsOut), 
  dm(insertExtend,['insertExtend - avail attrs: windowintersects = ',AttrsOut,'\n']),
  !.

insertExtend(windowintersectsS(Index, X), 
             windowintersectsS(Index, X),
             _,
             AttrsOut) :-
  AttrsOut = [id], % produces a stream of tuple identifiers (attr name = 'id') 
  dm(insertExtend,['insertExtend - avail attrs: windowintersectsS = ',AttrsOut,'\n']),
  !.

insertExtend(gettuples(X, Rel), 
             gettuples(X2, Rel),
             AttrsIn,
             AttrsOut) :-
  % expects a stream of tuple identifiers (attr name 'id')
  % returns tuples from relation Rel with all of its base attrs
  insertExtend(X, X2, AttrsIn, _), 
  insertExtend(feed(Rel),_,AttrsIn,AttrsOut), 
  dm(insertExtend,['insertExtend - avail attrs: gettuples = ',AttrsOut,'\n']),
  !.

% Case: project (Recurse for argument and return set of projection attributes)
insertExtend(project(Stream, AttrNames), 
             project(Stream2, AttrNames), 
             AttrsIn, 
             AttrsOut) :-
  dm(rewrite,['--->',insertExtend(project(Stream, AttrNames)),'\n\t']),
  findall( FlatAttr,
           ( member(X,AttrNames), 
             X = attrname(attr(Attr,_,_)), 
             ( atomic(Attr) 
               -> downcase_atom(Attr,FlatAttr) 
               ;  ( Attr = Alias:Attr2,
                    downcase_atom(Alias,AliasF),
                    downcase_atom(Attr2,Attr2F),
                    FlatAttr = AliasF:Attr2F
                  )
             )
           ),
           AttrsOutL
         ), 
  list_to_set(AttrsOutL,AttrsOut),
  insertExtend(Stream,Stream2,AttrsIn,AttrsStream), 
  dm(insertExtend,['insertExtend - avail attrs: ',project(AttrsStream,AttrsOut),
                  ' = ', AttrsOut,'\n']),
  !.


% Case: symmjoin. This is the only join operator that allows for generic
%       predicates, while all other joins are equijoins that accept only
%       attributenames instead of a predicate.
insertExtend(PlanIn, PlanOut, AttrsIn, AttrsOut) :-
  compound(PlanIn),
  PlanIn =.. [symmjoin,ArgS1,ArgS2,Pred],
  insertExtend(ArgS1, ArgS1E, AttrsIn, AttrsOut1), !,
  insertExtend(ArgS2, ArgS2E, AttrsIn, AttrsOut2), !,
 
  modifyPredicate(Pred, PredE, 
                  AttrsOut1, AttrsOut2, 
                  ExtLeft, ExtRight, ExtResult),

  extendPhrase(ArgS1E, ExtLeft,  ArgS1R),
  extendPhrase(ArgS2E, ExtRight, ArgS2R),
  PlanTmp =.. [symmjoin, ArgS1R, ArgS2R, PredE],
  extendPhrase(PlanTmp, ExtResult, PlanOut),

  union(ExtLeft, ExtRight, ExtArgs),
  union(AttrsOut1, AttrsOut2, AttrArgs),
  union(ExtArgs, AttrArgs, TmpAttrs),
  union(TmpAttrs,ExtResult, AttrsOut),
  dm(insertExtend,['\ninsertExtend: symmjoin-predicate \n\tOld: ',
                  Pred,'\n\tNew: ',PredE,'\n']),
  dm(insertExtend,['insertExtend - avail attrs: ',symmjoin(AttrsOut1,AttrsOut2),
                  ' = ',AttrsOut,'\n']),
  !.

% Case: other join operators (no extends, just merge attribute sets)
insertExtend(PlanIn, PlanOut, AttrsIn, AttrsOut) :-
  compound(PlanIn),
  PlanIn =.. [OP,ArgS1,ArgS2|OtherArgs],
  isJoinOP(OP),
  insertExtend(ArgS1, ArgS1E, AttrsIn, AttrsOut1), !,
  insertExtend(ArgS2, ArgS2E, AttrsIn, AttrsOut2), !,
  PlanOut =.. [OP,ArgS1E,ArgS2E|OtherArgs],
  union(AttrsOut1, AttrsOut2, AttrsOut), 
  dm(insertExtend,['insertExtend - avail attrs: ',OP,'(',
                  AttrsOut1,AttrsOut2,') = ',AttrsOut,'\n']),
  !.  

% case: filter (possibly extend argument stream)
insertExtend(PlanIn, PlanOut, AttrsIn, AttrsOut) :-
  compound(PlanIn),
  PlanIn =.. [filter,Arg,Pred],
  insertExtend(Arg, ArgS, AttrsIn, AttrArg), !,
  modifyPredicate(Pred, PredE, AttrArg, [], ExtArg, _, _),
  extendPhrase(ArgS, ExtArg, ArgsSE),
  PlanOut =.. [filter,ArgsSE,PredE],
  union(AttrArg, ExtArg, AttrsOut), 
  dm(insertExtend,['\ninsertExtend: filter-predicate \n\tOld: ',
                  Pred,'\n\tNew: ',PredE,'\n']),
  dm(insertExtend,['insertExtend - avail attrs: ',filter(AttrArg),' = ',
                  AttrsOut,'\n']),
  !.  

% Case: Operator that does not modify attribute sets 
% (should be the last clause! Do nothing)
insertExtend(PlanIn, PlanOut, AttrsIn, AttrsOut) :-
  compound(PlanIn),
  PlanIn =.. [OP,StreamArg|OtherArgs],
  not(isJoinOP(OP)),
  insertExtend(StreamArg, StreamArg2, AttrsIn, AttrsOut), 
  PlanOut =.. [OP,StreamArg2|OtherArgs], 
  dm(insertExtend,['insertExtend - avail attrs: ',OP,'(',
                  AttrsOut,') = ',AttrsOut,'\n']),
  !.

  

/*
Axiliary predicates to insertExtend

*/

/*
---- modifyPredicate(+PredIn,       -PredOut, 
                     +AttrsAvail1,  +AttrsAvail2, 
                     -AttrsExtend1, -AttrsExtend2, -AttrsExtendResult),
----
Modifies predicate ~PredIn~ by replacing CSEs with already extended virtual 
attributes or with virtual attributes that can be extended to the both argument
streams. ~AttrsAvail1~ and ~AttrsAvail2~ are sets of all attributes already 
available in argument streams 1 resp. 2. The ~Arg~-field of ~attr/3~ functors is 
adjusted to reflect the correct argument numbers of all attributes within ~PredOut~.

~AttrsExtend1~ and ~AttrsExtend2~ are sequences of virtual attributes, that 
~must~ be extended to both argument streams ~before~ the streames are merged. 
~AttrsExtendResult~ is a sequence of virtual attributes, that ~might~ be 
extended to the result stream, ~after~ the tuples have being merged.

If ~PredIn~ is a selection predicate, the available attributes should be passed
in ~AttrsAvail1~, passing an empty list for ~AttrsAvail2~. The resulting extension
sequence will then be returned in ~AttrsExtend1~, ~AttrsExtend2~ and 
~AttrsExtendResult~ should be empty lists.

*/


modifyPredicate(PredIn,PredOut,Avail1,Avail2,Left,Right,Result) :-
  modifyPredicate2(PredIn,PredOut,Avail1,Avail2,LeftI,RightI,ResultI),
  removeDublets(LeftI,Avail1,Left),
  removeDublets(RightI,Avail2,Right),
  union(Left,Right,ExtArgs),
  union(Avail1,Avail2,Avail),
  union(ExtArgs,Avail,AvailPreRes),
  removeDublets(ResultI,AvailPreRes,Result).

modifyPredicate2([], [], _, _, [], [], []).

modifyPredicate2([Me|Others], [Me2|Others2], Arg1, Arg2, Left, Right, Result) :-
  modifyPredicate2(Me, Me2, Arg1, Arg2, LeftM, RightM, ResultM),
  modifyPredicate2(Others, Others2, Arg1, Arg2, LeftO, RightO, ResultO), 
  append(LeftM,   LeftO,   Left  ),
  append(RightM,  RightO,  Right ),
  append(ResultM, ResultO, Result), !.

modifyPredicate2(Term, Term, _, _, [], [], []) :-
  atomic(Term).

modifyPredicate2(attr(X,Y,Z), attr(X,Y,Z), _, _, [], [], []).

modifyPredicate2(Term, Term2, Arg1, Arg2, Left, Right, Result) :-
  testOnCSE(Term, Label),
  (
            ( Arg1 \= [],
              extendable([Label],Arg1, Left), 
              Right = [], Result = [], 
              Term2 = attr(Label,1,u)
            )
          ; ( Arg2 \= [],
              extendable([Label],Arg2, Right), 
              Left = [], Result = [], 
              Term2 = attr(Label,2,u)
            )
          ; ( union(Arg1, Arg2, Res),
              Res \= [],
              extendable([Label],Res,Result), 
              Left = [], Right = [], 
              Term2 = Term
            )
  ), !.

modifyPredicate2(Term, Term2, Arg1, Arg2, Left, Right, Result) :-
  compound(Term),
  not(is_list(Term)),
  Term =.. [OP|Args],
  modifyPredicate2(Args, ArgsM, Arg1, Arg2, Left, Right, Result),
  Term2 =.. [OP|ArgsM], !.


/*
---- extendable(+LabelList, +Attrs, -ExtendSequence)
----
Succeeds, iff all CSEs from List ~LabelList~ are extendable to a stream having 
attributes ~Attrs~. ~ExtendSequence~ is the sequence of labels that must
be extended in reversed extension order (the first should be extended last etc.) and 
includes ~LabelList~.

*/
extendable(LabelList, AvailAttrs, ExtendSequence) :-
  extendable1(LabelList, AvailAttrs, ExtendSequence1),
  removeDublets(ExtendSequence1, AvailAttrs, ExtendSequence).

extendable1([], _, []) :- !.
extendable1([Label|Rest], AvailAttrs, [Label|SubLabels]) :-
  virt_attr(Label, _, _, UsedAttrs, _),
  subtract(UsedAttrs,AvailAttrs,MissedAttrs), 
  extendable1(MissedAttrs, AvailAttrs, SubLabels1), 
  extendable1(Rest, AvailAttrs, SubLabels2), 
  append(SubLabels1,SubLabels2,SubLabels), !.


/*
---- removeDublets(+List, +NegativeList, -Result)
----
Remove all elements from ~List~, that are dublets or occur in ~NegativeList~,
but keep the ordering. Return the resulting list in ~Result~.

*/

removeDublets(List, Negative, Result) :-
  removeDublets(List,Negative, [], Result), !.

removeDublets([], _, Interim, Result) :-
  reverse(Interim,Result), !.
removeDublets([First|Next], NegativeList, Interim, Result) :-
  ( (  memberchk(First,Interim)
     ; memberchk(First,NegativeList)
    )
      -> removeDublets(Next,NegativeList,Interim,Result)
       ; removeDublets(Next,NegativeList,[First|Interim],Result)
  ), !.

/*
---- testOnCSE(+Expr,-Label)
----
Succeeds, iff ~Expr~ is a CSE stored within ~virt\_attr/5~. If so,
~Label~ is the label used for this CSE.

*/

testOnCSE(Expr, Label) :-
  simpleExpr(Expr,Simple),
  virt_attr(Label,_,Simple,_,_), !.


% transcribe expression into simple format (with all attributes having either 
% format 'Name' or 'Alias:Name')
simpleExpr([],[]) :- !.
simpleExpr([E1|E2],[E1R|E2R]) :-
  simpleExpr(E1,E1R),
  simpleExpr(E2,E2R), !.
simpleExpr(attr(Name,_,_), Name) :- !.
simpleExpr(Expr,Expr) :-
  atomic(Expr).
simpleExpr(Expr,Result) :-
  compound(Expr),
  not(is_list(Expr)),
  Expr =.. [OP|Args],
  simpleExpr(Args, ResultList),
  Result =.. [OP|ResultList], !.


/*
---- extendPhrase(+ArgS, +NeededVirtAttrs, -ArgSE)
----
Get stream ~ArgS~ extended with all necessary virtual attributes from 
~NeededVirtAttrs~ in the correct ordering, as given by that list (the
first element being extended as the outermost). The result ~ArgSE~ is 
the extended stream expression.

*/

extendPhrase(ArgS, [], ArgS) :- !.
extendPhrase(ArgS, [VA1|InnerVAs], ArgsSE) :-
  virt_attr(VA1, CompExpr, _, _, _),
  extendPhrase(ArgS, InnerVAs, ArgsSE1),
  ArgsSE = extend(ArgsSE1,[newattr(attrname(attr(VA1,1,u)), CompExpr)]),
  assert(rewritePlanInsertedAttribute(attr(VA1,1,u))), !.

extendPhrase(ArgS, List, _) :-
  write('ERROR in extendPhrase('), write(ArgS), write(','), write(List), 
  write(', Result).\n\tAttribute list contains a non-virtual attribute.\n'),
  throw(sql_ERROR(rewriting_extendPhrase(ArgS, List, undefined))), fail, !.
  

/*
15.1.3 Rewriting the Plan for the Select-Clause

---- lookupCSESelect(+Term, -Term2, +AttrsIn, -ExtensionSequence, -UsedAttrs) :-
----
~Term~ should have be unified with the argument of the ~select~ functor and
should have been processed by ~lookupAttrs~ before. ~Term~ may be a list or
an arbitrary expression. ~AttrsIn~ must be the set of attributes available 
after the conjunctive part of the query has finished. All elements of the set
must be in flat format, either ``alias:attrname'' or ``attrname''.

~Term2~ will return a modified version of ~Term~, where CSEs have been 
substituted by virtual attributes. ~ExtensionSequence~ will be unified with a 
sequence of virtual attributes to extend the result stream from the conjunctive 
query with, before the select clause can be translated.

After the extend operators have been prepended to the result of the optimized
conjunctive query, ~Term2~ can simply be processed instead of the regular 
select-clause to get a complete plan. This is, because all neccessary attributes 
have already been marked as ~usedAttr/2~ or ~queryAttr/1~ by ~lookupAttrs/2~.

~UsedAttrs~ is the set of all attributes used within the select clause in the 
simple format (including those used to extend the demanded virtual attributes).
If the select clause contains ``[star]'' or ``count([star])'', all base 
attributes will be preserved.

*/

% Functors first, orderby, groupby, select, distinct, , 'count(X)', etc. 
% are handled implicitly by the last clause. Only special cases are handled
% explicitly:

% Process a list of Expressions
lookupCSESelect([],[],_,[], []).
lookupCSESelect([Arg|Args],[ArgM|ArgsM],AttrsIn,AllExt,AllUsed) :-
  lookupCSESelect(Arg, ArgM, AttrsIn,AttrsArgExt, AttrUsed),
  lookupCSESelect(Args,ArgsM,AttrsIn,AttrsArgsExt, AttrsUsed),
  append(AttrsArgExt,AttrsArgsExt,AllExt),
  list_to_set(AllExt,AllExtSet),
  union(AttrUsed,AttrsUsed,AllUsed1),
  union(AllExtSet,AllUsed1,AllUsed), !.

% Handle '*': all base attributes shall be kept
lookupCSESelect(*,*,_,[],AllBaseAttrs) :-
  findall(X,   % get a set of all attributes from all unrenamed relations
          ( usedAttr(rel(_,*,_), attr(Attr2, _, _)),
            downcase_atom(Attr2,X)
          ),
          UnrenamedAttrs),
  findall(Y,   % get a set of all attributes from all renamed relations
          ( usedAttr(rel(_,Alias,_), attr(Attr2, _, _)),
            Alias \= *,
            downcase_atom(Attr2,Attr3),
            Y = Alias:Attr3
          ),
          RenamedAttrs),
  list_to_set(UnrenamedAttrs,UnrenamedAttrsS),
  list_to_set(RenamedAttrs,RenamedAttrsS),
  union(UnrenamedAttrsS,RenamedAttrsS,AllBaseAttrs), !.  

% Handle atomic expressions
lookupCSESelect(Term,Term,_,[],[]) :-
  atomic(Term).

% Handle ordinary select-attributes
lookupCSESelect(attr(X,Y,Z),attr(X,Y,Z),_,[],[X]) :-
  dm(rewritePlan,['\n  lookupCSESelect 1(',attr(X,Y,Z),',',
                  attr(X,Y,Z),',...): ',[], [X]]),
  !.

% cut off from- and where-clauses
lookupCSESelect(Term from X, Term2 from X, AttrsIn, AttrsExt, AttrsUsed) :-
  lookupCSESelect(Term, Term2, AttrsIn, AttrsExt, AttrsUsed).

% Extension attribute already available as virtual attribute
lookupCSESelect(Expr as attr(Name,X,Y), attr(Name,X,Y), AttrsIn, [], [Name]) :-
  member(Name,AttrsIn), 
  dm(rewritePlan,['\n  lookupCSESelect 2(',Expr as attr(Name,X,Y),','
                  ,attr(Name,X,Y),',...): ',[], [Name]]),
  !.

% Extension attribute that is not also a virtual attribute
lookupCSESelect(Expr as attr(Name,X,Y), ExprE as attr(Name,X,Y), 
                AttrsIn, AttrsExt, AttrsUsed) :-
  not(virt_attr(Name,_,_,_,_)),
  lookupCSESelect(Expr, ExprE, AttrsIn, AttrsExt, AttrsUsedE), 
  union([Name],AttrsUsedE,AttrsUsed), 
  dm(rewritePlan,['\n  lookupCSESelect 3(',Expr as attr(Name,X,Y),','
                  ,ExprE as attr(Name,X,Y),',...): ',AttrsExt, AttrsUsed]),
  !.

% Extension attribute that also is a virtual attribute, but has not yet been
% extended
lookupCSESelect(Expr as attr(Name,X,Y), attr(Name,X,Y), 
                AttrsIn, AttrsExt, AttrsUsed) :-
  virt_attr(Name,_,_,_,_),
  not(member(Name,AttrsIn)),
  extendable([Name], AttrsIn, ExtendSequence),
  removeDublets(ExtendSequence,AttrsIn,AttrsExt),
  findall( X,            % get all needed attributes to extend
           ( member(Y,AttrsExt), 
             virt_attr(Y, _, _, X, _)
           ),
           AttrsUsed1),
  flatten(AttrsUsed1,AttrsUsed2),
  list_to_set([Name|AttrsUsed2],AttrsUsed), 
  dm(rewritePlan,['\n  lookupCSESelect 4(',Expr as attr(Name,X,Y),',',
                  attr(Name,X,Y),',...): ',AttrsExt, AttrsUsed]),
  !.

% Replace a CSE within an expression
lookupCSESelect(Term, attr(Label,0,u), AttrsIn, AttrsExt, AttrsUsed) :-
  simpleExpr(Term,TermS),
  virt_attr(Label, _, TermS, _, _),
  ( member(Label,AttrsIn)
    -> ( AttrsUsed = [Label],
         AttrsExt = []
       )
     ; ( extendable([Label], AttrsIn, ExtendSequence),
         removeDublets(ExtendSequence,AttrsIn,AttrsExt),
         findall( X,            % get all needed attributes to extend
                  ( member(Y,AttrsExt), 
                    virt_attr(Y, _, _, X, _)
                  ),
                  AttrsUsed1),
         flatten([AttrsUsed1|AttrsExt],AttrsUsed2),
         list_to_set(AttrsUsed2,AttrsUsed)
       )
  ),
  dm(rewritePlan,['\n  lookupCSESelect 5(',Term,',',attr(Label,0,u),'...): ',
                  AttrsExt, AttrsUsed]),
  !.

% Handle all other compound expressions
lookupCSESelect(Term, Term2, AttrsIn, AttrsExt, AttrsUsed) :-
  compound(Term),
  not(is_list(Term)),
  Term =.. [OP|Args],
  lookupCSESelect(Args,Args2,AttrsIn,AttrsExt,AttrsUsed),
  Term2 =.. [OP|Args2],
  dm(rewritePlan,['\n  lookupCSESelect 6(',Term,',',Term2,'...): ',
                  AttrsExt, AttrsUsed]),
  !.


/*
15.1.4 Remove Virtual Attributes from Result

When CSE substitution is enabled, virtual attributes may be added into the streams and will
also be propagated into the result stream. If the query is a star-query 
(having ``select [star]''), the virtual attributes will occur within the result, when no
final projection is inserted into the plan (as it is in all other cases). For 
``select count([star])'', no removes are needed at all, but if ``distinct'' occurs, it should. 

The removes are carried out in file ``optimizer.pl'', predicate ~selectClause/4~,
which calls ~rewritePlanRemoveInsertedAttributes/2~.

Attributes, that have been removed earlier (e.g. because ~optimizerOption(rewriteRemove)~ switched on),
may not be removed. The according fact ~rewritePlanInsertedAttribute/1~ must have been retracted!

This functionality will be automatically added to the optimzer, when ~optimizerOption(rewriteCSE)~
is defined.

*/

rewritePlanRemoveInsertedAttributes(StreamIn, StreamOut) :-
  findall(X, 
          ( rewritePlanInsertedAttribute(X),
            X = attr(Name, _, Case),
            not(queryAttr(attr(Name, _, Case)))
          ), 
          AttrsL),
  list_to_set(AttrsL,AttrsS),
  attrnames(AttrsS,AttrNames),
  ( AttrNames = [] 
    -> StreamOut = StreamIn
    ;  StreamOut = remove(StreamIn,AttrNames)
  ), !.
  
/*

15.2 Early Removal of Unused Attributes

Early Removal of Unused Attributes can be applied together with CSE-Subsitution, but can also
be useful without prior CSE-Substitution to free memory from unused data and avoid IO-overhead
for buffered tuples in external phases of operator execution.

This step is carried out in a top-down fashion with regard to the operator tree represented by
the query plan. First, the set of attributes needed within the select-clause is needed, let us call
it ~S~.

While descending into the operator-tree, we maintain ~S~ and add every attribute encountered within
a predicate or extend operator, parallely branching into all argument streams.

If an attribute ~A~ is encountered for the first time, it is added to ~S~. Also, we wrap the actual
subplan ~P~ into a ~remove(P, A)~ operator, which will remove the attribute from the tuplestream.

~Early Removal of Unused Attributes~ is an optional optimizer expansion that will only be used 
when ~optimizerOption(rewriteCSE)~ AND ~optimizerOption(rewriteRemove)~ are defined.

*/

/*
---- removeUnusedAttrs(+PlanIn,-PlanOut,+ExceptionList)
----
Remove all attributes occurring in ~PlanIn~ as early as possible, but keep all
attributes passed in ~ExceptionList~. Pass the result as ~PlanOut~.

*/

removeUnusedAttrs(PlanIn,PlanIn, _) :-
  not(optimizerOption(rewriteRemove)), !.

removeUnusedAttrs(PlanIn,PlanIn, _) :-
  atomic(PlanIn), !.

% Case: remove. Remove nothing, but add UsedAttrs to SeenAttrs
removeUnusedAttrs(remove(Arg,AttrList), remove(ArgE,AttrList), SeenAttrs) :-
  usedAttributes(remove(Arg,AttrList),UsedAttrs),
  union(UsedAttrs,SeenAttrs,NewSeenAttrs),
  removeUnusedAttrs(Arg,ArgE,NewSeenAttrs), !.

% Case: project. Replace SeenAttrs by UsedAttrs. Remove nothing.
removeUnusedAttrs(project(Arg,AttrList),project(ArgE,AttrList),_) :-
  usedAttributes(project(Arg,AttrList),UsedAttrs),
  removeUnusedAttrs(Arg, ArgE, UsedAttrs), !.

% Case: gettuples. Remove nothing, but replace SeenAttrs by id 
removeUnusedAttrs(gettuples(Arg,Rel), gettuples(ArgE,Rel), _) :-
  removeUnusedAttrs(Arg,ArgE,[id]).

% Case: (generic linear operator). Add UsedAttrs to SeenAttrs, Remove New Attrs
removeUnusedAttrs(PlanIn,PlanOut,SeenAttrs) :-
  compound(PlanIn),
  not(is_list(PlanIn)),
  PlanIn =.. [OP,Arg1|Args], % assuming one single stream argument only
  ( not(isJoinOP(OP)), OP \= symmjoin ),
  usedAttributes(PlanIn,UsedAttrs),
  union(UsedAttrs,SeenAttrs,NewSeenAttrs),
  removeUnusedAttrs(Arg1,Arg1E,NewSeenAttrs),
  Plan2 =..[OP,Arg1E|Args],
  subtract(UsedAttrs,SeenAttrs,RemoveAttrs),
  removePhrase(Plan2, PlanOut, RemoveAttrs), !.

% Case: (generic join operator). Add UsedAttrs to SeenAttrs, Remove New Attrs
removeUnusedAttrs(PlanIn,PlanOut,SeenAttrs) :-
  compound(PlanIn), 
  not(is_list(PlanIn)),
  PlanIn =.. [OP,Arg1,Arg2|Args], % assuming two argument streams
  ( isJoinOP(OP) ; OP = symmjoin ),
  usedAttributes(PlanIn,UsedAttrs),
  union(UsedAttrs,SeenAttrs,NewSeenAttrs),
  removeUnusedAttrs(Arg1,Arg1E,NewSeenAttrs),
  removeUnusedAttrs(Arg2,Arg2E,NewSeenAttrs),
  Plan2 =..[OP,Arg1E,Arg2E|Args],
  subtract(UsedAttrs,SeenAttrs,RemoveAttrs),
  removePhrase(Plan2, PlanOut, RemoveAttrs),
  !.

% Case: Default case. Do nothing
removeUnusedAttrs(PlanIn,PlanIn,SeenAttrs) :- 
  dm(rewritePlan,['\nremoveUnusedAttrs(',PlanIn,',',SeenAttrs,
                  ',?) WARNING: default case (Should not occur).\n']),
  !.

/*
---- removePhrase(+ArgS, -ArgSE, +RemoveAttrs)
----
Remove attributes ~RemoveAttrs~ from stream ~ArgS~ by prepending a remove-operator
to ~ArgS~. The reulting plan is ~ArgSE~. Elements of ~RemoveAttrs~ are in simple
format, either ``attributename'' or ``alias:attributename''.

When processing the conjunctive subplan, ~RemoveAttrs~ can be initialized to the 
set of all attributes used within the select clause.

Also, for each removed attribute, the fact ~rewritePlanInsertedAttribute(Attr)~ is
retracted.

*/


removePhrase(Term, Term, []) :- !.
removePhrase(Term, Term2, RemoveList) :-
  makeRemoveList(RemoveList,RemoveList2),
  Term2 = remove(Term, RemoveList2), !.

makeRemoveList([],[]).
makeRemoveList([A|As],[attrname(Attr)|Bs]) :-
  ( ( usedAttr(_,Attr), 
      Attr = attr(A,_,_)
    )
   ;( A = Alias:Name,
      usedAttr(rel(_,Alias,_),attr(Name,X,Y)),
      Attr = attr(Alias:Name, X, Y)
    )
   ;( virt_attr(A,_,_,_,_),
      Attr = attr(A, 1, u)
    )
  ), !,
  retractall(rewritePlanInsertedAttribute(Attr)),
  makeRemoveList(As,Bs).


/*
---- usedAttributes(+Operator, -Attrs)
----
Returns the set of attributes ~Attrs~ that are directly used in Operator
~Operator~.

*/

usedAttributes([],[]).
usedAttributes([A|As],Attrs) :-
  usedAttributes(A,B),
  usedAttributes(As, Bs),
  union(B,Bs,Attrs), !.
usedAttributes(X,[]) :-
  atomic(X), !.

usedAttributes(attr(A:N,_,_),[A1:N1]) :- 
  downcase_atom(A,A1), 
  downcase_atom(N,N1), !.
usedAttributes(attr(A,_,_),[A1]) :- downcase_atom(A,A1), !.

usedAttributes(attr2(A:N,_,_),[A1:N1]) :- 
  downcase_atom(A,A1), 
  downcase_atom(N,N1), !.
usedAttributes(attr2(A,_,_),[A1]) :- downcase_atom(A,A1), !.

usedAttributes(a(A:N,_,_),[A1:N1]) :- 
  downcase_atom(A,A1), 
  downcase_atom(N,N1), !.
usedAttributes(a(A,_,_),[A1]) :- downcase_atom(A,A1), !.

usedAttributes(remove(_,AttrList),Attrs) :-
  usedAttributes(AttrList,Attrs), !.
usedAttributes(extend(_,AttrList),Attrs) :-
  usedAttributes(AttrList,Attrs), !.
usedAttributes(project(_,AttrList),Attrs) :-
  usedAttributes(AttrList,Attrs), !.
usedAttributes(rename(_,_),[]) :- !.
usedAttributes(counter(_,_),[]) :- !.
usedAttributes(groupby(_,X,Y),Attrs) :- 
  usedAttributes([X,Y],Attrs), !.
usedAttributes(sortby(_,X),Attrs) :- 
  usedAttributes(X,Attrs), !.
usedAttributes(equals(_,_,A,B),Attrs) :- 
  usedAttributes([A,B],Attrs), !.
usedAttributes(like(_,_,_,A,B),Attrs) :- 
  usedAttributes([A,B],Attrs), !.
usedAttributes(exactmatchfun(_,_,Attr),Attrs) :- 
  usedAttributes(Attr,Attrs), !.
usedAttributes(filter(_,Pred),Attrs) :-
  usedAttributes(Pred,Attrs), !.
usedAttributes(symmjoin(_,_,Pred),Attrs) :-
  usedAttributes(Pred,Attrs), !.
usedAttributes(product(_,_),[]).
usedAttributes(leftrange(_, _, Pred),Attrs) :-
  usedAttributes(Pred,Attrs), !.
usedAttributes(rightrange(_, _, Pred),Attrs) :-
  usedAttributes(Pred,Attrs), !.
usedAttributes(exactmatch(_, _, Pred), Attrs) :-
  usedAttributes(Pred,Attrs), !.
usedAttributes(windowintersects(_, _, Pred), Attrs) :-
  usedAttributes(Pred,Attrs), !.
usedAttributes(windowintersectsS(_, Pred), Attrs) :-
  usedAttributes(Pred,Attrs), !.
usedAttributes(filter(_,Pred),Attrs) :-
  usedAttributes(Pred,Attrs), !.
usedAttributes(gettuples(_,_), [id]) :- !.

usedAttributes(sort(_),[]) :- !. % XRIS: Perhaps, to be handeled separately?
usedAttributes(rdup(_),[]) :- !.

usedAttributes(Term,Attrs) :-
  compound(Term),
  not(is_list(Term)),
  Term =.. [OP,_,_|Args],
  isJoinOP(OP),
  usedAttributes(Args,Attrs), !.

usedAttributes(Term,Attrs) :-
  compound(Term),
  not(is_list(Term)),
  Term =.. [_|Args],
  usedAttributes(Args,Attrs), !.



/*
15.3 Testing

Comment out this complete section for standard behavior.

*/


% predicates for testing CSE substitution:

xristest :- delOption(rewriteCSE), 
            setOption(rewriteRemove), 
            open 'database opt'. % XRIS: testing only!

:- [autotest].          % XRIS: testing only!

testquery1 :- sql select[no*1 as no1, (no*1)*(no*1) as no2] 
                  from ten 
                  where (no*1)*(no*1) > 1.

testquery2 :- sql select[no*1 as no1, (no*1)*(no*1) as no2]
                  from[ten, ten as ten2] 
                  where[no*1* (no*1)<ten2:no].

testquery3 :- sql select[no*1 as no1, (no*1)*(no*1)+ten2:no as no2]
                  from[ten, ten as ten2] 
                  where[no*1* (no*1)>ten2:no].

testquery4 :- sql select[no*1+ten2:no as no1, 
                         (no*1+ten2:no)*(no*1+ten2:no)+ten2:no as no2]
                  from[ten, ten as ten2] 
                  where[(no*1+ten2:no)* (no*1+ten2:no)>ten2:no].

testquery5 :- sql select[no*1 as no1, (no*1)*(no*1) as no2] 
                  from ten 
                  where[(no*1)*(no*1) > 1, (no*1)*(no*1)+1 <20].

testquery6 :- sql select[no*1+ten2:no as no1, 
                         (no*1+ten2:no)*(no*1+ten2:no)+ten2:no as no2]
                  from[ten, ten as ten2] 
                  where[(no*1+ten2:no)* (no*1+ten2:no)>ten2:no, 
                        (no*1)*(no*1) > 1, 
                        (no*1)*(no*1)+1 <20].

testquery7 :- sql select[no*1 as no1, (no*1)*(no*1) as no2] 
                  from ten 
                  where [(no*1)*(no*1) > 1] 
                  first 3.

testquery8 :- sql select[no*1+ten2:no as no1, 
                         (no*1+ten2:no)*(no*1+ten2:no)+ten2:no as no2]
                  from[ten, ten as ten2] 
                  where[(no*1+ten2:no)* (no*1+ten2:no)>ten2:no, 
                        (no*1)*(no*1) > 1, 
                        (no*1)*(no*1)+1 <20] 
                  first 3.

testquery9 :- sql select sname 
                  from staedte 
                  where[sname starts "B", 
                        bev >= 1000000, 
                        plz<90000, 
                        kennzeichen starts "B"].

