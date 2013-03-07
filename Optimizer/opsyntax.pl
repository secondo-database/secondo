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
:- op(800, xf , getCategory).
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
:- op(800, xf , getHeight).
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
:- op(800, xf , getPictureDate).
:- op(800, xfx, present).
:- op(800, xf , rect2region).
:- op(800, xf , relcount).
:- op(800, xf , relcount2).
:- op(800, xfx, satisfies).
:- op(800, xfx, simpleequals).
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

  * postfix, N arguments: corresponds to \_ \# and \_ \_ \#, ...

  * postfixbrackets, N:  $N \geq 1$ arments before the functor, the rest following it
      within squared brackets, separated by commas:
        \_ \# [ \_ ], \_ \# [ \_ , \_ ], \_ \# [ \_ , \_ , \_ ] ...

  * prefix, N: $N \geq 0$ arguments following the functor in round parantheses,
      separated by commas:: \# (\_, \_)

  * infix, 2 arguments: \_ \# \_

  * special, N: all operators requiring a special ~plan\_to\_atom~ rule.

There are deprecated standard translations for operators, which should not be used anymore.
Define an explicit ~secondoOP~ fact instead!

  * binary operators: \_ \# \_  infix

  * N-ary operators, $N \neq 2$:  \# ( \_, \_, \_, ... ) prefix

----
Quick Reference:

 OperatorSyntax         |    Use: OperatorType, NoArgs
 -----------------------+------------------------------------------------
 # ()                   |    prefix, 0
 # ( _ )                |    prefix, 1
 # ( _, _ )             |    prefix, 2
 # (  _, _, _, ...)     |    prefix, n ( n>=1 )
 _ #                    |    postfix, 1
 _ # _                  |    infix, 2
 _ # [ _ ]              |    postfixbrackets, 1
 _ # [ _, _ ]           |    postfixbrackets, 1
 _ # [ _, _, ... ]      |    postfixbrackets, 1
 _ _ #                  |    postfix, 2
 _ _ # [ _ ]            |    postfixbrackets, 2
 _ _ # [ _, _ ]         |    postfixbrackets, 2
 _ _ # [ _, _, ... ]    |    postfixbrackets, 2
 _ _ _ #                |    postfix, 3
 _ _ _ # [ _ ]          |    postfixbrackets, 3
 _ _ _ # [ _, _ ]       |    postfixbrackets, 3
 _ _ _ # [ _, _, ... ]  |    postfixbrackets, 3
any using list          |    special, _
any using funlist       |    special, _
all others,             |    special, _

----

*/

:- dynamic(secondoOp/3).

secondoOp((+),                infix, 2).
secondoOp( ++,                postfix, 1).
secondoOp((-),                infix, 2).
secondoOp((*),                infix, 2).
secondoOp((/),                infix, 2).
secondoOp((#),                infix, 2).
secondoOp((##),               infix, 2).
secondoOp((=),                infix, 2).
secondoOp((==),               infix, 2).
secondoOp((<),                infix, 2).
secondoOp((<=),               infix, 2).
secondoOp((<<),               infix, 2).
secondoOp((<<==),             infix, 2).
secondoOp((>),                infix, 2).
secondoOp((>=),               infix, 2).
secondoOp((>>),               infix, 2).
secondoOp((>>==),             infix, 2).
secondoOp( abs,               prefix, 1).
secondoOp( addcounter,        postfixbrackets, 1).
secondoOp( addid,             postfix, 1).
secondoOp( addtupleid,        postfix, 1).
secondoOp( aggregate,         special, 1). % special syntax
secondoOp( aggregateB,        special, 1). % TODO: special syntax
secondoOp( aggregateS,        special, 1). % TODO: special syntax
secondoOp((adjacent),         infix, 2).
secondoOp((always),           infix, 2).
secondoOp((and),              infix, 2).
secondoOp( approximate,       postfixbrackets, 1).
secondoOp( arccos,            prefix, 1).
secondoOp( arcsin,            prefix, 1).
secondoOp( arctan,            prefix, 1).
secondoOp( arctan2,           prefix, 2).
secondoOp((at),               infix, 2).
secondoOp((atinstant),        infix, 2).
secondoOp((atperiods),        infix, 2).
secondoOp((attached),         infix, 2).
secondoOp( attr,              prefix, 2).
secondoOp( attrsize,          postfixbrackets, 1).
secondoOp( attr2text,         prefix, 1).
secondoOp( avg,               postfixbrackets, 1).
secondoOp( bbox,              prefix, 1).
secondoOp((before),           infix, 2).
secondoOp( between,           postfixbrackets, 1).
secondoOp( box2d,             prefix, 1).
secondoOp( box3d,             prefix, 2).
secondoOp( breakpoints,       prefix, 2).
secondoOp( bulkloadtbtree,    postfixbrackets, 1).
secondoOp( bulkloadrtree,     postfixbrackets, 1).
secondoOp( cancel,            postfixbrackets, 1).
secondoOp( getCategory,       postfix, 1).
secondoOp( charToText,        prefix, 1).
secondoOp( cluster_of,        prefix, 2).
secondoOp( clustername_of,    prefix, 2).
secondoOp( colordist,         postfixbrackets, 1).
secondoOp( collect_line,      postfix, 1).
secondoOp( collect_set,       postfix, 1).
secondoOp( collect_sline,     postfix, 1).
secondoOp( collect_multiset,  postfix, 1).
secondoOp( collect_vector,    postfix, 1).
secondoOp((commonborder),     infix, 2).
secondoOp((commonborderscan), infix, 2).
secondoOp( compare,           prefix, 2).
secondoOp( constgraph,        postfixbrackets, 1).
secondoOp( constgraphpoints,  postfixbrackets, 1).
secondoOp( consume,           postfix, 1).
secondoOp( concat,            postfix, 2).
secondoOp( concatS,           postfix, 1).
secondoOp( concatS2,          postfixbrackets, 1).
secondoOp( const_e,           prefix, 0).
secondoOp( const_pi,          prefix, 0).
secondoOp((contains),         infix, 2).
secondoOp( cos,               prefix, 1).
secondoOp( count,             postfix, 1).
secondoOp( countboth,         postfix, 2).
secondoOp( create_duration,   prefix, 1).
secondoOp( create_instant,    prefix, 1).
secondoOp( createcluster,     prefix, 2).
secondoOp( createbtree,       postfixbrackets, 1).
secondoOp( createdeleterel,   postfix, 1).
secondoOp( createhash,        postfixbrackets, 1).
secondoOp( createinsertrel,   postfix, 1).
secondoOp( createupdaterel,   postfix, 1).
secondoOp( createpgroup,      prefix, 2).
secondoOp( createprioritypgroup,prefix, 2).
secondoOp( createtbtree,      postfixbrackets, 1).
secondoOp( creatertree,       postfixbrackets, 1).
secondoOp( createvalidpgroup, prefix, 2).
secondoOp( csvexport,         postfixbrackets, 1).
secondoOp( csvimport,         postfixbrackets, 1).
secondoOp( cumulate,          postfixbrackets, 1).
secondoOp( cut,               postfixbrackets, 1).
secondoOp( db3export,         postfixbrackets, 1).
secondoOp( dbimport,          postfixbrackets, 1).
secondoOp( deftime,           prefix, 1).
secondoOp( deg2rad,           prefix, 1).
secondoOp( delay,             prefix, 2).
secondoOp( deletebtree,       postfixbrackets, 2).
secondoOp( deletedirect,      postfix, 2).
secondoOp( deletedirectsave,  postfix, 3).
secondoOp( deleteelem,        prefix, 2).
secondoOp( deletehash,        postfixbrackets, 2).
secondoOp( deletesearch,      postfix, 2).
secondoOp( deletesearchsave,  postfix, 3).
secondoOp( difference,        prefix, 2).
secondoOp( direction,         prefix, 2).
secondoOp( display,           postfix, 1).
secondoOp( distance,          prefix, 2).
secondoOp( distancescan,      postfixbrackets, 2).
secondoOp( distribute,        postfixbrackets, 1).
secondoOp((div),              infix, 2).
secondoOp( dms2deg,           prefix, 2).
secondoOp( dumpstream,        postfixbrackets, 1).
secondoOp( echo,              postfixbrackets, 1).
secondoOp( end,               prefix, 1).
secondoOp( enlargeRect,       prefix, 2).
secondoOp( ensure,            postfixbrackets, 1).
secondoOp((eplus),            infix, 2).
secondoOp((equalway),         infix, 2).
secondoOp( equals,            postfixbrackets, 2).
secondoOp( evaluate,          prefix, 2).
secondoOp( everNearerThan,    prefix, 3).
secondoOp( exactmatch,        postfixbrackets, 2).
secondoOp( exactmatchS,       postfixbrackets, 1).
secondoOp( export,            postfix, 1).
secondoOp( extattrsize,       postfixbrackets, 1).
secondoOp( extend,            postfixbrackets, 1).
secondoOp( extendstream,      special, 1).
secondoOp( extenddeftime,     prefix, 2).
secondoOp( extract,           postfixbrackets, 1).
secondoOp( exttuplesize,      postfix, 1).
secondoOp( feed,              postfix, 1).
secondoOp( fileSize,          prefix,  1).
secondoOp( filter,            postfixbrackets, 1).
secondoOp( find,              prefix, 2).
secondoOp( flipleft,          postfixbrackets, 1).
secondoOp( get,               prefix, 2).
secondoOp( getcatalog,        prefix, 2).
secondoOp( getDirectory,      prefix, 1).
secondoOp( getFilename,       postfix, 1).
secondoOp( getHeight,         postfix, 1).
secondoOp( gettuples,         postfix, 2).
secondoOp( gettuples2,        postfixbrackets, 2).
secondoOp( gettuplesdbl,      postfixbrackets, 2).
secondoOp( getMaxVal,         prefix, 1).
secondoOp( getMinVal,         prefix, 1).
secondoOp( getTypeNL,         postfix, 1).
secondoOp( getValueNL,        postfix, 1).
secondoOp( gps,               prefix, 2).
secondoOp( groupby,           special, 1).
secondoOp( hashjoin,          postfixbrackets, 2).
secondoOp( hashvalue,         prefix, 2).
secondoOp( head,              postfixbrackets, 1).
secondoOp((ininterior),       infix, 2).
secondoOp( insert,            postfix, 2).
secondoOp( insertrtree,       postfixbrackets, 2).
secondoOp( inserttuple,       postfixbrackets, 1).
secondoOp( inserttuplesave,   postfixbrackets, 2).
secondoOp( insertsave,        postfix, 3).
secondoOp((inside),           infix, 2).
secondoOp((insideold),        infix, 2).
secondoOp( insertbtree,       postfixbrackets, 2).
secondoOp( inserthash,        postfixbrackets, 2).
secondoOp( intersection_new,  prefix, 2).
secondoOp( intersection,      prefix, 2).
secondoOp((intersects),       infix, 2).
secondoOp((intersects_new),   infix, 2).
secondoOp( invert,            prefix, 2).
secondoOp( isempty,           prefix, 1).
secondoOp( isgrayscale,       postfix, 1).
secondoOp( isportrait,        postfix, 1).
secondoOp( kbiggest,          postfixbrackets, 1).
secondoOp( keywords,          postfix, 1).
secondoOp( kinds,             postfix, 1).
secondoOp( krdup,             postfixbrackets, 1).
secondoOp( ksmallest,         postfixbrackets, 1).
secondoOp( ldistance,         prefix, 2).
secondoOp( leapyear,          postfix, 1).
secondoOp( leftrange,         postfixbrackets, 2).
secondoOp( leftrangeS,        postfixbrackets, 1).
secondoOp( length,            prefix, 1).
secondoOp( like,              postfixbrackets, 1).
secondoOp( line2region,       postfix, 1).
secondoOp( log,               prefix, 1).
secondoOp( logB,              prefix, 2).
secondoOp( loop,              postfixbrackets, 1).
secondoOp( loopa,             postfixbrackets, 2).
secondoOp( loopb,             postfixbrackets, 2).
secondoOp( loopjoin,          postfixbrackets, 1).
secondoOp( loopsel,           postfixbrackets, 1).
secondoOp( loopselect,        postfixbrackets, 1).
secondoOp( loopselecta,       postfixbrackets, 2).
secondoOp( loopselectb,       postfixbrackets, 2).
secondoOp( loopswitch,        postfixbrackets, 1).
secondoOp( loopswitcha,       postfixbrackets, 2).
secondoOp( loopswitchb,       postfixbrackets, 2).
secondoOp( makeline,          prefix, 2).
secondoOp( makesline,         prefix, 2).
secondoOp( makearray,         prefix, 1).
secondoOp( makemvalue,        postfixbrackets, 1).
secondoOp( matches,           infix, 2).
secondoOp( max,               postfixbrackets, 1).
secondoOp( maxD,              prefix, 2).
secondoOp( maxdegree,         prefix, 2).
secondoOp( maxDuration,       prefix , 0).
secondoOp( maxInstant,        prefix , 0).
secondoOp( memshuffle,        postfix, 1).
secondoOp( merge,             prefix, 2).
secondoOp( mergediff,         postfix, 2).
secondoOp( mergejoin,         postfixbrackets, 2).
secondoOp( mergesec,          postfix, 2).
secondoOp( mergeunion,        postfix, 2).
secondoOp( min,               postfixbrackets, 1).
secondoOp( minD,              prefix, 2).
secondoOp( mindegree,         prefix, 2).
secondoOp( minDuration,       prefix , 0).
secondoOp( minInstant,        prefix , 0).
secondoOp( minus_new,         prefix, 2).
secondoOp( mirror,            postfixbrackets, 1).
secondoOp((mod),              infix, 2).
secondoOp( move,              prefix, 2).
secondoOp( multiintersection, prefix, 2).
secondoOp( nanmedtransformstream,postfixbrackets, 1).
secondoOp((never),            infix, 2).
secondoOp( not,               prefix, 1).
secondoOp( now,               prefix, 0).
secondoOp( nnscan,            postfixbrackets, 2).
secondoOp( nnsearch,          postfixbrackets, 2).
secondoOp((onborder),         infix, 2).
secondoOp((overlaps),         infix, 2).
secondoOp((or),               infix, 2).
secondoOp((p_intersects),     infix, 2).
secondoOp((passes),           infix, 2).
secondoOp( passmbool,         prefix, 1).
secondoOp( partjoin,          postfixbrackets, 2).
secondoOp( partjoinselect,    postfixbrackets, 2).
secondoOp( partjoinswitch,    postfixbrackets, 2).
secondoOp( pcreate,           postfixbrackets, 1).
secondoOp( pdelete,           postfix, 1).
secondoOp( pfeed,             postfixbrackets, 1).
secondoOp( getPictureDate,    postfix, 1).
secondoOp( pjoin2,            postfixbrackets, 2).
secondoOp( polylines,         prefix, 2).
secondoOp( polylinesC,        prefix, 2).
secondoOp( pow,               prefix, 2).
secondoOp( predcounts,        special, 1).
secondoOp((present),          infix, 2).
secondoOp( printstream,       postfix, 1).
secondoOp( product,           postfix, 2).
secondoOp( project,           special, 1).
secondoOp( projectextendstream,special, 1).
secondoOp( projecttransformstream,postfixbrackets, 1).
secondoOp( pwdisjoint,        prefix, 2).
secondoOp( puse,              postfixbrackets, 1).
secondoOp( rad2deg,           prefix, 1).
secondoOp( randmax,           prefix, 0).
secondoOp( randombool,        prefix, 1).
secondoOp( randomdelay,       prefix, 2).
secondoOp( range,             postfixbrackets, 2).
secondoOp( rangeS,            postfixbrackets, 1).
secondoOp( rangesearch,       postfixbrackets, 2).
secondoOp( rdup,              postfix, 1).
secondoOp( realm,             prefix, 2).
secondoOp( rect2region,       postfix, 1).
secondoOp( reduce,            postfixbrackets, 1).
secondoOp( relax,             prefix, 2).
secondoOp( relcount,          postfix, 1).
secondoOp( relcount2,         postfix, 1).
secondoOp( remove,            postfixbrackets, 1).
secondoOp( renamecluster,     postfixbrackets, 1).
secondoOp( restrict,          prefix, 1).
secondoOp( rightrange,        postfixbrackets, 2).
secondoOp( rightrangeS,       postfixbrackets, 1).
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
secondoOp( rootattrsize,      postfixbrackets, 1).
secondoOp( roottuplesize,     postfix, 1).
secondoOp( rotate,            postfixbrackets, 1).
secondoOp( round,             prefix, 2).
secondoOp( sample,            postfixbrackets, 1).
secondoOp( samplempoint,      prefix, 2).
secondoOp((satisfies),        infix, 2).
secondoOp( saveObject,        postfixbrackets, 1).
secondoOp( scale,             postfixbrackets, 1).
secondoOp( sendtextstreamTCP, postfixbrackets, 1).
secondoOp( sentences,         postfix, 1).
secondoOp( seqnext,           prefix , 0).
secondoOp( setoption,         prefix, 2).
secondoOp( shpexport,         postfixbrackets, 1).
secondoOp( shpimport,         postfixbrackets, 1).
secondoOp( shuffle3,          postfix, 1).
secondoOp( sim_create_trip,   postfixbrackets, 1).
secondoOp( sim_create_trip,   postfixbrackets, 1).
secondoOp( sim_fillup_mpoint, postfixbrackets, 1).
secondoOp( sim_print_params,  prefix, 0).
secondoOp( sim_set_rng,       prefix, 2).
secondoOp( sim_trips,         postfixbrackets, 1).
secondoOp( sim_trips,         postfixbrackets, 1).
secondoOp((simpleequals),     infix, 2).
secondoOp( simplify,          prefix, 2).
secondoOp( sin,               prefix, 1).
secondoOp( size,              prefix, 1).
secondoOp( sizecounters,      postfixbrackets, 1).
secondoOp((sometimes),        prefix, 1).
secondoOp( sort,              postfix, 1).
secondoOp( sortarray,         postfixbrackets, 1).
secondoOp( sortby,            postfixbrackets, 1).
secondoOp( sortmergejoin,     postfixbrackets, 2).
secondoOp( sortmergejoin_r2,  postfixbrackets, 2).
secondoOp( spatialjoin,       postfixbrackets, 2).
secondoOp( speedup,           postfixbrackets, 1).
secondoOp( start,             prefix, 1).
secondoOp((starts),           infix, 2).
secondoOp( stats,             postfixbrackets, 1).
secondoOp( stconstraint,      special, 1). % faked operator
secondoOp( stpattern,         special, 1).
secondoOp( stpattern2,        special, 1).
secondoOp( stpatternex,       special, 1).
secondoOp( stpatternex2,      special, 1).
secondoOp( stpatternextend,   special, 1).
secondoOp( stpatternextend2,  special, 1).
secondoOp( stpatternexextend, special, 1).
secondoOp( stpatternexextend2,special, 1).
secondoOp( stpatternextendstream,special, 1).
secondoOp( stpatternextendstream2,special, 1).
secondoOp( stpatternexextendstream,special, 1).
secondoOp( stpatternexextendstream2,special, 1).
secondoOp( stdpgroup,         prefix , 0).
secondoOp( strequal,          prefix, 2).
secondoOp( sum,               postfixbrackets, 1).
secondoOp( summarize,         prefix, 1).
secondoOp( symmjoin,          special, 2).
secondoOp( symmproduct,       postfix, 2).
secondoOp( symmproductextend, special, 2).
secondoOp( tail,              postfixbrackets, 1).
secondoOp( tan,               prefix, 1).
secondoOp( tconsume,          postfix, 1).
secondoOp( the_mvalue,        postfix, 1).
secondoOp( thevertex,         prefix, 2).
secondoOp( themonth,          prefix, 2).
secondoOp( theperiod,         prefix, 2).
secondoOp( tie,               postfixbrackets, 1).
secondoOp( timeout,           postfixbrackets, 1).
secondoOp( today,             prefix, 0).
secondoOp( tokenize,          prefix, 2).
secondoOp( toprel,            prefix, 2).
secondoOp((touchpoints),      infix, 2).
secondoOp( trajectory,        prefix, 1).
secondoOp( transformstream,   postfix, 1).
secondoOp( translate,         postfixbrackets, 1).
secondoOp( translateappendS,  postfixbrackets, 1).
secondoOp( transpose,         prefix, 1).
secondoOp( tuplesize,         postfix, 1).
secondoOp((union),            infix, 2).
secondoOp( union_new,         prefix, 2).
secondoOp( updatebtree,       postfixbrackets, 2).
secondoOp( updatebyid,        postfixbrackets, 2).
secondoOp( updatehash,        postfixbrackets, 2).
secondoOp( updatertree,       postfixbrackets, 2).
secondoOp( updatesearch,      postfixbrackets, 2).
secondoOp( updatedirectsave,  postfixbrackets, 3).
secondoOp( updatesearchsave,  postfixbrackets, 3).
secondoOp( use,               postfixbrackets, 1).
secondoOp( use2,              postfixbrackets, 2).
secondoOp( val,               prefix, 1).
secondoOp( var,               postfixbrackets, 1).
secondoOp( vec,               prefix, 1).
secondoOp( getWidth,          postfix, 1).
secondoOp((when),             infix, 2).
secondoOp( windowclippingin,  prefix, 2).
secondoOp( windowclippingout, prefix, 2).
secondoOp( windowintersects,  postfixbrackets, 2).
secondoOp( windowintersectsS, postfixbrackets, 1).
secondoOp( within,            postfixbrackets, 1).
secondoOp( within2,           postfixbrackets, 2).
secondoOp( whiledo,           special, 1). % TODO: special syntax
secondoOp( writeFile,         prefix, 2).
secondoOp( zero,              prefix , 0).
secondoOp( theInstant, prefix, 1).
secondoOp( randomdelay,       prefix, 2).
secondoOp( speed,       prefix, 1).
% Section:Start:secondoOp_3_e
% Section:End:secondoOp_3_e

/*
End of file ~opsyntax.pl~

*/
