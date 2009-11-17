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

//paragraph [10] title: [{\Large \bf ]  [}]
//characters [1] formula:       [$]     [$]
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[**] [$**$]
//[star] [$*$]
//[->] [$\rightarrow$]
//[toc] [\tableofcontents]
//[=>] [\verb+=>+]
//[<] [$<$]
//[>] [$>$]
//[%] [\%]

//[newpage] [\newpage]

[10] Defining Operator Syntax

[File ~opsyntax.pl~]

[toc]

[newpage]


1 Query Language Operator Syntax


*/

:- op(800, xf , ++).
:- op(200, xfx, :).
:- op(800, xfx, =>).
:- op(800, xfx, ==).
:- op(800, xfx, <=).
:- op(800, xfx, <<).
:- op(800, xfx, <<==).
:- op(800, xfx, >>).
:- op(800, xfx, >>==).
:- op(800, xfx, #).
:- op(800, xfx, ##).
:- op(800, xfx, adjacent).
:- op(800, xfx, always).
:- op(800, xfx, and).
:- op(800, xfx, at).
:- op(800, xfx, atinstant).
:- op(800, xfx, atperiods).
:- op(800, xfx, attached).
:- op(800, xfx, before).
:- op(800, xf , category).
:- op(800, xfx, commonborder).
:- op(800, xfx, commonborderscan).
:- op(800, xfx, contains).
:- op(800, xfx, div).
:- op(800, xfx, eplus).
:- op(800, xfx, equalway).
:- op(800, xfx, ininterior).
:- op(800, xfx, inside).
:- op(800, xfx, insideold).
:- op(800, xfx, intersection).
:- op(800, xfx, intersects).
:- op(800, xfx, intersects_new).
:- op(800, xf , isgrayscale).
:- op(800, xf , isportrait).
:- op(800, xf , height).
:- op(800, xf , leapyear).
:- op(800, xf , line2region).
:- op(800, xfx, mod).
:- op(800, xfx, never).
:- op(800,  fx, not).
:- op(800, xfx, onborder).
:- op(800, xfx, overlaps).
:- op(800, xfx, or).
:- op(800, xfx, p_intersects).
:- op(800, xfx, passes).
:- op(800, xf , picturedate).
:- op(800, xfx, present).
:- op(800, xf , rect2region).
:- op(800, xf , relcount).
:- op(800, xf , relcount2).
:- op(800, xfx, satisfies).
:- op(800, xfx, simpleequals).
:- op(800, xfx, sometimes).
:- op(800, xfx, starts).
:- op(800, xfx, touchpoints).
:- op(800, xfx, union).
:- op(800, xfx, when).

% Section:Start:opSyntaxPreference_3_e
% Section:End:opSyntaxPreference_3_e

/*

2 Executable Language Operator Syntax


----	secondoOp(?Op, ?Syntax, ?NoArgs) :-
----

~Op~ is a Secondo operator written in ~Syntax~, with ~NoArgs~ arguments.
Currently implemented:

  * postfix, 1 or 2 arguments: corresponds to \_ \# and \_ \_ \#

  * postfixbrackets1, 2 or more arguments. All but the first one are put into
      the brackets: \_ \# [ \_ ], \_ \# [ \_ , \_ ], \_ \# [ \_ , \_ , \_ ] ...

  * postfixbrackets2: 3 or more arguments. All but the first two are put into
      the brackets: \_ \_ \# [\_ ], \_ \_ \# [\_ , \_ ] ...

  * prefix, 2 arguments: \# (\_, \_)

  * prefix, either 1 or 3 arguments, does not need a rule here, is
translated by default.

  * prefix, 0 argument: \# ( ), uses implicit argument(s), but no explicit one

  * infix, 2 arguments: does not need a rule, translated by default.

For all other forms, a plan\_to\_atom rule has to be programmed explicitly.
Otherwise, the standard schema is applied:

  * unary operators:   \# ( \_ )           prefix 1

  * binary operators: \_ \# \_              infix

  * N-ary operators:  \# ( \_, \_, \_, ... ) prefix, N [>] 2

----
Quick Reference:

 OperatorSyntax         |    Use: OperatorType, Arity ( '_' = don't care)
 -----------------------+------------------------------------------------
 # ()                   |    prefix, 0
 # ( _ )                |    (no explicit specification)
 # ( _, _ )             |    prefix, 2
 # (  _, _, _, ...)     |    (no explicit specification)
 _ #                    |    postfix, 1
 _ # _                  |    (no explicit specification)
 _ # [ _ ]              |    postfixbrackets, _
 _ # [ _, _ ]           |    postfixbrackets, _
 _ # [ _, _, ... ]      |    postfixbrackets, _
 _ _ #                  |    postfix, 2
 _ _ # [ _ ]            |    postfixbrackets2, _
 _ _ # [ _, _ ]         |    postfixbrackets2, _
 _ _ # [ _, _, ... ]    |    postfixbrackets2, _
 _ _ _ #                |    postfix, 3
 _ _ _ # [ _ ]          |    postfixbrackets3, _
 _ _ _ # [ _, _ ]       |    postfixbrackets3, _
 _ _ _ # [ _, _, ... ]  |    postfixbrackets3, _

----

*/

:- dynamic(secondoOp/3).

secondoOp( ++,                postfix, 1).
secondoOp( addcounter,        postfixbrackets1, 3).
secondoOp( addid,             postfix, 1).
secondoOp( addtupleid,        postfix, 1).
%secondoOp( aggregate,        postfixbrackets1, 3). % special syntax
%secondoOp( aggregateB,       postfixbrackets1, 3). % TODO: special syntax
%secondoOp( aggregateS,       postfixbrackets1, 3). % TODO: special syntax
secondoOp( approximate,       postfixbrackets1, 3).
secondoOp( attr,              prefix, 2).
secondoOp( attrsize,          postfixbrackets1, 2).
secondoOp( avg,               postfixbrackets1, 2).
secondoOp( bbox,              prefix, 1).
secondoOp( between,           postfixbrackets1, 3).
secondoOp( box3d,             prefix, 2).
secondoOp( breakpoints,       prefix, 2).
secondoOp( bulkloadtbtree,    postfixbrackets1, 4).
secondoOp( bulkloadrtree,     postfixbrackets1, 2).
secondoOp( cancel,            postfixbrackets1, 2).
secondoOp( category,          postfix, 1).
secondoOp( cluster_of,        prefix, 2).
secondoOp( clustername_of,    prefix, 2).
secondoOp( colordist,         postfixbrackets1, 2).
secondoOp( collect_line,      postfix, 1).
secondoOp( collect_set,       postfix, 1).
secondoOp( collect_sline,     postfix, 1).
secondoOp( collect_multiset,  postfix, 1).
secondoOp( collect_vector,    postfix, 1).
secondoOp( compare,           prefix, 2).
secondoOp( constgraph,        postfixbrackets1, 4).
secondoOp( constgraphpoints,  postfixbrackets1, 4).
secondoOp( consume,           postfix, 1).
secondoOp( concat,            postfix, 2).
secondoOp( concatS,           postfix, 1).
secondoOp( concatS2,          postfixbrackets1, 2).
secondoOp( count,             postfix, 1).
secondoOp( countboth,         postfix, 2).
secondoOp( create_duration,   prefix, 2). % also #(_)
secondoOp( create_instant,    prefix, 2). % also #(_)
secondoOp( createcluster,     prefix, 2).
secondoOp( createbtree,       postfixbrackets1, 2).
secondoOp( createdeleterel,   postfix, 1).
secondoOp( createhash,        postfixbrackets1, 2).
secondoOp( createinsertrel,   postfix, 1).
secondoOp( createupdaterel,   postfix, 1).
secondoOp( createpgroup,      prefix, 2). % also prefix 3+
secondoOp( createprioritypgroup,prefix, 2). % also prefix 3+
secondoOp( createtbtree,      postfixbrackets1, 3).
secondoOp( creatertree,       postfixbrackets1, 2).
secondoOp( createvalidpgroup, prefix, 2). % also prefix 3+
secondoOp( csvexport,         postfixbrackets1, 3). % also 4+5
secondoOp( csvimport,         postfixbrackets1, 4). % also 5
secondoOp( cumulate,          postfixbrackets1, 2).
secondoOp( cut,               postfixbrackets1, 5).
secondoOp( db3export,         postfixbrackets1, 2).
secondoOp( dbimport,          postfixbrackets1, 2).
secondoOp( deletebtree,       postfixbrackets2, 3).
secondoOp( deletedirect,      postfix, 2).
secondoOp( deletedirectsave,  postfix, 3).
secondoOp( deleteelem,        prefix, 2).
secondoOp( deletehash,        postfixbrackets2, 3).
secondoOp( deletesearch,      postfix, 2).
secondoOp( deletesearchsave,  postfix, 3).
secondoOp( difference,        prefix, 2).
secondoOp( direction,         prefix, 2).
secondoOp( display,           postfix, 1).
secondoOp( distance,          prefix, 2).
secondoOp( distancescan,      postfixbrackets2, 4).
secondoOp( distribute,        postfixbrackets1, 2).
secondoOp( dumpstream,        postfixbrackets1, 3).
secondoOp( echo,              postfixbrackets1, 3).
secondoOp( enlargeRect,       prefix, 2).
secondoOp( ensure,            postfixbrackets1, 2).
secondoOp( equals,            postfixbrackets2, 4).
secondoOp( evaluate,          prefix, 2).
secondoOp( everNearerThan,    prefix, 3).
secondoOp( exactmatch,        postfixbrackets2, 3).
secondoOp( exactmatchS,       postfixbrackets1, 2).
secondoOp( export,            postfix, 1).
secondoOp( extattrsize,       postfixbrackets1, 2).
secondoOp( extend,            postfixbrackets1, 2). % special syntax
secondoOp( extendstream,      postfixbrackets1, 2). % special syntax
secondoOp( extenddeftime,     prefix, 2).
secondoOp( extract,           postfixbrackets1, 2).
secondoOp( exttuplesize,      postfix, 1).
secondoOp( feed,              postfix, 1).
secondoOp( filename,          postfix, 1).
secondoOp( fileSize,          prefix, 2). % also prefix 1
secondoOp( filter,            postfixbrackets1, 2).
secondoOp( find,              prefix, 2).
secondoOp( flipleft,          postfixbrackets1, 2).
secondoOp( get,               prefix, 2).
secondoOp( getcatalog,        prefix, 2).
secondoOp( getDirectory,      prefix, 2). % also prefix 1
secondoOp( gettuples,         postfix, 2).
secondoOp( gettuples2,        postfixbrackets2, 3).
secondoOp( gettuplesdbl,      postfixbrackets2, 3).
secondoOp( getTypeNL,         postfix, 1).
secondoOp( getValueNL,        postfix, 1).
secondoOp( gps,               prefix, 2).
%secondoOp( groupby,           postfixbrackets1, 3). % special syntax
secondoOp( hashjoin,          postfixbrackets2, 5).
secondoOp( hashvalue,         prefix, 2).
secondoOp( head,              postfixbrackets1, 2).
secondoOp( height,            postfix, 1).
secondoOp( insert,            postfix, 2).
secondoOp( insertrtree,       postfixbrackets2, 3).
secondoOp( inserttuple,       postfixbrackets1, 2). % also postfixbrackets1, 3+
secondoOp( inserttuplesave,   postfixbrackets2, 3). % also postfixbrackets1, 4+
secondoOp( insertsave,        postfix, 3).
secondoOp( intersection_new,  prefix, 2).
secondoOp( intersection,      prefix, 2).
secondoOp( insertbtree,       postfixbrackets2, 3).
secondoOp( inserthash,        postfixbrackets2, 3).
secondoOp( invert,            prefix, 2).
secondoOp( isgrayscale,       postfix, 1).
secondoOp( isportrait,        postfix, 1).
secondoOp( kbiggest,          postfixbrackets1, 3).
secondoOp( keywords,          postfix, 1).
secondoOp( kinds,             postfix, 1).
secondoOp( krdup,             postfixbrackets1, 2). % also more than 2 args
secondoOp( ksmallest,         postfixbrackets1, 3).
secondoOp( ldistance,         prefix, 2).
secondoOp( leapyear,          postfix, 1).
secondoOp( leftrange,         postfixbrackets2, 3).
secondoOp( leftrangeS,        postfixbrackets1, 2).
secondoOp( like,              postfixbrackets1, 5).
secondoOp( line2region,       postfix, 1).
secondoOp( loop,              postfixbrackets1, 2).
secondoOp( loopa,             postfixbrackets2, 3).
secondoOp( loopb,             postfixbrackets2, 3).
secondoOp( loopjoin,          postfixbrackets1, 2).
secondoOp( loopsel,           postfixbrackets1, 2).
secondoOp( loopselect,        postfixbrackets1, 4).
secondoOp( loopselecta,       postfixbrackets2, 5).
secondoOp( loopselectb,       postfixbrackets2, 5).
secondoOp( loopswitch,        postfixbrackets1, 2).
secondoOp( loopswitcha,       postfixbrackets2, 3).
secondoOp( loopswitchb,       postfixbrackets2, 3).
secondoOp( makeline,          prefix, 2).
secondoOp( makesline,         prefix, 2).
secondoOp( makearray,         prefix, 2). % also prefix 1,...,n
secondoOp( makemvalue,        postfixbrackets1, 2).
secondoOp( max,               postfixbrackets1, 2).
secondoOp( maxD,              prefix, 2).
secondoOp( maxdegree,         prefix, 2).
secondoOp( maxDuration,       prefix , 0).
secondoOp( maxInstant,        prefix , 0).
secondoOp( memshuffle,        postfix, 1).
secondoOp( merge,             prefix, 2).
secondoOp( mergediff,         postfix, 2).
secondoOp( mergejoin,         postfixbrackets2, 4).
secondoOp( mergesec,          postfix, 2).
secondoOp( mergeunion,        postfix, 2).
secondoOp( min,               postfixbrackets1, 2).
secondoOp( minD,              prefix, 2).
secondoOp( mindegree,         prefix, 2).
secondoOp( minDuration,             prefix , 0).
secondoOp( minInstant,        prefix , 0).
secondoOp( minus_new,         prefix, 2).
secondoOp( mirror,            postfixbrackets1, 2).
secondoOp( move,              prefix, 2).
secondoOp( multiintersection, prefix, 2). % also prefix 3+
secondoOp( nanmedtransformstream,postfixbrackets1, 2).
secondoOp( now,               prefix, 0).
secondoOp( nnscan,            postfixbrackets2, 3).
secondoOp( nnsearch,          postfixbrackets2, 4).
secondoOp( partjoin,          postfixbrackets2, 3).
secondoOp( partjoinselect,    postfixbrackets2, 5).
secondoOp( partjoinswitch,    postfixbrackets2, 3).
secondoOp( pcreate,           postfixbrackets1, 2).
secondoOp( pdelete,           postfix, 1).
secondoOp( pfeed,             postfixbrackets1, 2).
secondoOp( picturedate,       postfix, 1).
secondoOp( pjoin2,            postfixbrackets2, 4).
secondoOp( polylines,         prefix, 2). % also prefix 3 (optional argument)
secondoOp( polylinesC,        prefix, 2). % also prefix 3 (optional argument)
%secondoOp( predcounts,        postfixbrackets1, 2). % special syntax
secondoOp( printstream,       postfix, 1).
secondoOp( product,           postfix, 2).
%secondoOp( project,            postfixbrackets1, 2). % special syntax
%secondoOp( projectextendstream,postfixbrackets1, 3). % special syntax
secondoOp( projecttransformstream,postfixbrackets1, 2).
secondoOp( pwdisjoint,        prefix, 2). % also prefix 3+
secondoOp( puse,              postfixbrackets1, 2).
secondoOp( randmax,             prefix , 0).
secondoOp( range,             postfixbrackets2, 4).
secondoOp( rangeS,            postfixbrackets1, 3).
secondoOp( rangesearch,       postfixbrackets2, 4).
secondoOp( rdup,              postfix, 1).
secondoOp( realm,             prefix, 2).
secondoOp( rect2region,       postfix, 1).
secondoOp( reduce,            postfixbrackets1, 3).
secondoOp( relax,             prefix, 2).
secondoOp( relcount,          postfix, 1).
secondoOp( relcount2,         postfix, 1).
secondoOp( remove,            postfixbrackets1, 2).
secondoOp( renamecluster,     postfixbrackets1, 2).
secondoOp( restrict,          prefix, 2). % also prefix 1
secondoOp( rightrange,        postfixbrackets2, 3).
secondoOp( rightrangeS,       postfixbrackets1, 2).
secondoOp( rng_binomial,      prefix, 2).
secondoOp( rng_flat,          prefix, 2).
secondoOp( rng_geometric,     prefix, 2).
secondoOp( rng_getMax,        prefix, 0).
secondoOp( rng_getMin,        prefix, 0).
secondoOp( rng_getSeed,       prefix, 0).
secondoOp( rng_getType,       prefix, 0).
secondoOp( rng_init,          prefix, 2).
secondoOp( rng_int,           prefix, 0).
secondoOp( rng_NoGenerators,  prefix, 0).
secondoOp( rng_real,          prefix, 0).
secondoOp( rng_realpos,       prefix, 0).
secondoOp( rootattrsize,      postfixbrackets1, 2).
secondoOp( roottuplesize,     postfix, 1).
secondoOp( rotate,            postfixbrackets1, 4).
secondoOp( round,             prefix, 2).
secondoOp( sample,            postfixbrackets1, 3). % also postfixbrackets1, 4
secondoOp( samplempoint,      prefix, 2). % also prefix3, prefix4
secondoOp( saveObject,        postfixbrackets1, 3).
secondoOp( scale,             postfixbrackets1, 2). % also postfixbrackets1 3
secondoOp( sentences,         postfix, 1).
secondoOp( seqnext,           prefix , 0).
secondoOp( setoption,         prefix, 2).
secondoOp( shpexport,         postfixbrackets1, 2). % also 3+4
secondoOp( shpimport,         postfixbrackets1, 2).
secondoOp( shuffle3,          postfix, 1).
secondoOp( sim_create_trip,   postfixbrackets1, 5).
secondoOp( sim_create_trip,   postfixbrackets1, 6).
secondoOp( sim_fillup_mpoint, postfixbrackets1, 6).
secondoOp( sim_print_params,  prefix, 0).
secondoOp( sim_set_rng,       prefix, 2).
secondoOp( sim_trips,         postfixbrackets1, 2).
secondoOp( sim_trips,         postfixbrackets1, 3).
secondoOp( simplify,          prefix, 2). % also prefix 3 (optional argument)
secondoOp( sizecounters,      postfixbrackets1, 2).
secondoOp( sort,              postfix, 1).
secondoOp( sortarray,         postfixbrackets1, 2).
secondoOp( sortby,            postfixbrackets1, 2).
secondoOp( sortmergejoin,     postfixbrackets2, 4).
secondoOp( sortmergejoin_r2,  postfixbrackets2, 4).
secondoOp( spatialjoin,       postfixbrackets2, 4).
secondoOp( speedup,           postfixbrackets1, 2).
secondoOp( stats,             postfixbrackets1, 3).
secondoOp( stdpgroup,         prefix , 0).
secondoOp( sum,               postfixbrackets1, 2).
secondoOp( summarize,         prefix, 1).
% secondoOp( symmjoin,          postfixbrackets2, 3). % special syntax
% secondoOp( symmproductextend, postfixbrackets2, 3). % special syntax
secondoOp( symmproduct,       postfix, 2).
secondoOp( tail,              postfixbrackets1, 2).
secondoOp( tconsume,          postfix, 1).
secondoOp( the_mvalue,        postfix, 1).
secondoOp( thevertex,         prefix, 2).
secondoOp( themonth,          prefix, 2).
secondoOp( theperiod,         prefix, 2).
secondoOp( tie,               postfixbrackets1, 2).
secondoOp( today,             prefix, 0).
secondoOp( toprel,            prefix, 2).
secondoOp( transformstream,   postfix, 1).
secondoOp( translate,         postfixbrackets1, 3).
secondoOp( translateappendS,  postfixbrackets1, 3).
secondoOp( transpose,         prefix, 2). % also prefix 1
secondoOp( tuplesize,         postfix, 1).
secondoOp( union_new,         prefix, 2).
secondoOp( updatebtree,       postfixbrackets2, 3).
secondoOp( updatebyid,        postfixbrackets2, 3).
secondoOp( updatehash,        postfixbrackets2, 3).
secondoOp( updatertree,       postfixbrackets2, 3).
secondoOp( updatesearch,      postfixbrackets2, 3).
secondoOp( updatedirectsave,  postfixbrackets3, 4).
secondoOp( updatesearchsave,  postfixbrackets3, 4).
secondoOp( use,               postfixbrackets1, 2).
secondoOp( use2,              postfixbrackets3, 3).
secondoOp( var,               postfixbrackets1, 2).
secondoOp( vec,               prefix, 2).
secondoOp( width,             postfix, 1).
secondoOp( windowclippingin,  prefix, 2).
secondoOp( windowclippingout, prefix, 2).
secondoOp( windowintersects,  postfixbrackets2, 3).
secondoOp( windowintersectsS, postfixbrackets1, 2).
secondoOp( within,            postfixbrackets1, 2).
secondoOp( within2,           postfixbrackets2, 3).
%secondoOp( whiledo,           postfixbrackets1, 4). % TODO: special syntax
secondoOp( writeFile,         prefix, 2). % also prefix 3
secondoOp( zero,              prefix , 0).
secondoOp( delay,             prefix, 2).
% Section:Start:secondoOp_3_e
secondoOp( randomdelay,       prefix, 2).
% Section:End:secondoOp_3_e

/*
End of file ~opsyntax.pl~

*/
