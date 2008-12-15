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

//[<] [$<$]
//[>] [$>$]
//[%] [\%]
//[->] [$\rightarrow$]


1 Database Dependent Information

[File ~database.pl~]

The Secondo optimizer module needs database dependent information to
compute the best query plan. In particular information about the
cardinality and the schema of all relations is needed by the optimizer.
Furthermore the spelling of relation and attribute names must be
known to send a Secondo query or command. Finally the optimizer has
to be informed, if an index exists for the pair (~relationname~,
~attributename~). All this information look up is provided by this
module.

Also, predicates for creating and deleting secondary database objects, as
indexes, samples, and small objects are provided by this module. All these
objects are created using the ~derive~ command rather than the ~let~ command.
This policy ensures, that these database objects are not included in the dump
file created by ~save database to~, but are restored, when calling
~restore database from~ with such a database dump file.

Only primary database objects - e.g. those you might possibly update -
(as relations and atomic objects) should be created using the ~let~ command.

As several information, e.g. on specialized indexes cannot
be derived from the catalog's type descriptions, we are required to assume a
naming convention to infer the presence of (specialized) indexes for relations
stored in a database. Therefore, you should obey to the ~naming conventions~
explained in the according chapter of file ~optimizer.pl~.



1.1 Spelling of Identifiers within the Optimizer

As Prolog recognizes any atoms starting with an uppercase character as a
variable, identifiers starting with a upper case letter cannot be typed
when using the optimizer. (Also operator names may not start upper-cased.)

Instead, relations, indexes, attributes and any other database objects are
always referenced using totally down-cased versions of their original
identifiers when communicating with the optimizer. The optimizer will translate
this so-called ~DC-Spelling~ into its own ~InternalSpelling~ and convert it to
the full-spelled ~ExternalSpelling~, when it creates executable plans, which
are meant to be passed to the Secondo kernel.

Therefore, we have three kinds of spelling within the optimizer:

  * ~ExternalSpelling~: The original spelling of identifiers, as they are known within the Secondo Catalog.

  * ~InternalSpelling~: A representation used to store the ExternalSpelling within Prolog facts without confusing the Prolog system. It is ~only~ used to translate between ~DownCasedSpelling~ and ~ExternalSpelling~.

  * ~DownCasedSpelling~ or ~DC-Spelling~: The fully down-cased version of identifiers in ~ExternalSpelling~. Each upper-case character is substituted by its lower-case counterpart. This spelling is used within the optimizer, e.g. to store and query meta using facts and predicates. The user has to formulate all queries using this spelling for attributes, relations, indexes, etc.

As the optimizer only allows the use of identifiers in DC-Spelling, we need to
translate between that notation and the ~ExternalSpelling~ (the real names known to the Secondo kernel).

We use dynamic predicate storedSpell(Internal, External) to store the
translation of attribute names on disk. Due to this, we must avoid identifiers
starting with an underscore or a uppercase character. As we proscribe the use
of underscores within relation and attribute identifiers, we just need to keep
care of identifiers starting with an uppercase letter.

We do so by introducing an ~InternalSpelling~ schema, which is illustrated
by the following table:

----
          External | Internal  | DCspelled
          ---------+-----------+---------
           Mouse   | mouse     |  mouse
           mouse   | lc(mouse) |  mouse
           mOUSe   | lc(mOUSe) |  mouse
           MOUSe   | mOUSe     |  mouse
----

Database object names are not maintained by storedSpell/3 facts. Instead, their
spelling can be looked up on the catalog entries secondoCatalogInfo/4 directly.

---- storedSpell(DCspelledObjName, ExternallySpelledObjName, Type, TypeExpr)
----

Attribute identifiers are stored using facts

---- storedSpell(+DB, +DCspelledRel:+DCspelledAttr, +InternalAttr)
----


~DB~ is the name of the database, which is always in DC-spelling (the Secondo
kernel does not consider spelling for database names at all).

Three predicates are used to translate between the different spelling schemas:

----
          dcName2internalName(?DC,?Intern)
          dcName2externalName(?DC,?External)
          internalName2externalName(?Intern,?Extern)
----

They can be used for all database objects and for attributes.

When using these predicates, at least one of the two arguments must be
instantiated with a ~ground term~, i.e. a term containing no unbound variable.

Otherwise, or if the translation fails due to some other circumstances, the
translation predicate will throw an exception. This is to prevent the propagation
of errors through the optimization process, which easily leads to unpredictable
behaviour of the optimizer.

So, if you get an exception from these predicates, carefully inspect the error
message. It is likely, that you just miss-typed an identifier.

*/

/*
1.1.1 Translating Between Down Cased and Internal Spelling

---- dcName2internalName(?DC,?Intern)
----
Translate between a DownCasedSpelling identifier and the according
InternalSpelling within the current database.

*/

% InternObj -> DCobj  ALWAYS SUCCEEDS FOR CORRECT INTERNAL SPELLINGS
dcName2internalName(DC,Intern) :-
  ground(Intern),                % second arg instantiated
  atomic(Intern),
  downcase_atom(Intern,DC),
  !.
dcName2internalName(DC,lc(Intern)) :-
  ground(Intern),                % second arg instantiated
  atomic(Intern),
  downcase_atom(Intern,DC),
  !.
% DCobj -> InternObj  RELIES ON STORED TRANSLATION!
dcName2internalName(DC,Intern) :-
  ground(DC),                    % first arg instantiated
  atomic(DC),                    % is DB object identifier
  secondoCatalogInfo(DC,Extern), % get stored external name from catalog
  internalName2externalName(Intern,Extern), % translate extern -> intern
  !.
% DCrel:DCattr -> InternAttr
dcName2internalName(DC,Intern) :-
  ground(DC),                    % first arg instantiated
  DC = DCrel : DCattr,           % is attribute identifier
  databaseName(DB),              % get current DB name
  storedSpell(DB,DCrel:DCattr,Intern), % get stored internal attribute spelling
  !.
dcName2internalName(DC,Intern) :-
  write('ERROR:\tdcName2internalName('),write(DC),write(','),write(Intern),
  write(') failed!'), nl,
  ( ground(DC)
    -> write('--->\tProbably wrong spelling of 1st argument?!')
    ;  ( write('--->\tProbably missing storedSpell/3 or '),
         write('secondoCatalogInfo/4 for 2nd argument?!')
       )
  ), !,
  throw(error_SQL(database_dcName2internalName(DC,Intern):cannotTranslate)),
  nl, fail, !.

/*
1.1.2 Translating Between Down Cased and External Spelling

---- dcName2externalName(?DC, ?External)
----

Translate between a DC-spelled identifier and the according external
spelling within the current database.

*/

% DCobj -> ExternalObj  RELIES ON STORED TRANSLATION!
dcName2externalName(DC,External) :-
  ground(DC),                    % first arg instantiated
  atomic(DC),                    % is DB object identifier
  secondoCatalogInfo(DC,External,_,_), % get external spelling from catalog
  !.
% DCrel:DCattr -> ExternalAttr  RELIES ON STORED TRANSLATION!
dcName2externalName(DC,External) :-
  ground(DC),                    % first arg instantiated
  DC = _ : _,                    % is attribute identifier
  databaseName(DB),              % get current DB name
  storedSpell(DB, DC, Internal), % get stored internal object spelling
  internalName2externalName(Internal,External), % covert to external
  !.
% ExternalObj -> DCobj  ALWAYS SUCCEEDS!
dcName2externalName(DC,External) :-
  ground(External),              % second arg instantiated
  atomic(External),              % is DB object identifier
  downcase_atom(External, DC),
  !.
dcName2externalName(DC,External) :-
  write('ERROR:\tdcName2externalName('),write(DC),write(','),write(External),
  write(') failed!'), nl,
  ( ground(DC)
    -> ( write('--->\tProbably missing storedSpell/3 or secondoCatalogInfo/4 '),
         write('for 1st argument?!')
       )
    ;  write('--->\tProbably 2nd argument is not atomic?!')
  ), !,
  nl,
  throw(error_SQL(database_dcName2externalName(DC,External):cannotTranslate)),
  fail, !.

/*
1.1.3 Translating Internal and External Spelling

---- internalName2externalName(?Intern,?Extern)
----

Translate between the internal and external spelling of an identifier within the
current database.

*/

% Intern -> Extern  ALWAYS SUCCEEDS!
internalName2externalName(lc(Intern),Extern) :-
  ground(Intern),
  Extern = Intern,
  !.
internalName2externalName(Intern,Extern) :-
  ground(Intern),
  atomic(Intern),
  sub_atom(Intern,0,1,_,Prefix),
  sub_atom(Intern,1,_,0,Suffix),
  upcase_atom(Prefix,PrefixUC),
  concat_atom([PrefixUC,Suffix],'',Extern),
  !.

% Extern -> Intern  ALWAYS SUCCEEDS!
internalName2externalName(Intern,Extern) :-
  ground(Extern),
  sub_atom(Extern,0,1,_,ExternPrefix),
  char_type(ExternPrefix,lower),
  Intern = lc(Extern),
  !.
internalName2externalName(Intern,Extern) :-
  ground(Extern),
  sub_atom(Extern,0,1,_,Prefix),
  char_type(Prefix,upper),
  sub_atom(Extern,1,_,0,Suffix),
  downcase_atom(Prefix,PrefixDC),
  concat_atom([PrefixDC,Suffix],'',InternDC),
  Intern = InternDC,
  !.
internalName2externalName(Intern,Extern) :-
  write('ERROR:\tinternalName2externalName('),write(Intern),write(','),
  write(Extern),write(') failed!'), nl,
  ( ground(Intern)
    -> write('--->\tProbably 1st argument has unvalid internal spelling?!')
    ;  write('--->\tProbably both arguments unbound? This should not happen!')
  ), !,
  nl,
  throw(error_SQL(database_internalName2externalName(Intern,Extern)
                 :cannotTranslate)),
  fail, !.

/*
2 Retrieving and Updating Relation Schemas


---- updateRelationSchema(+DCrel)
----

For a relation ~DCrel~ in down-cased spelling, the predicate retrieves the
external spelling of the relation and its attributes, converts them to the
internal spelling and stores these in dynamic facts ~storedSpell/3~, which
is stored on disk. As we read this information during the optimizer's startup
by importing the fats, we need to use the internal spelling schema within these
facts.

Further more, it retrieves the ~types~ for all attributes and the according
attribute sizes. This information is stored as facts
~storedAttrSize(DB, DCRel, DCAttr, Type, CoreTupleSize, InFlobSize,
ExtFlobSize)~. To determine ~InFlobSize~, a query should be send to
Secondo, but that operator is still not implemented.

Operators to determine attribute sizes in Secondo:

  * ~attrsize~
   ( stream | rel(tuple X) x ident [->] real ),
   Return the size of the attribute within a stream or a
   relation taking into account the FLOBs.

  * ~exattrsize~
   ( stream(tuple X) | rel(tuple X) x identifier [->] real ),
   Return the size of the attribute within a stream or a
   relation taking into account the small FLOBs.

  * ~rootattrsize~
   ( stream(tuple X) | rel(tuple X) x identifier [->]  int ),
   Return the size of the attributes root part within a
   stream or a relation (without small FLOBs).

  * ~tuplesize~
   ( stream | rel (tuple x) [->] real ),
   Return the average size of the tuples within a stream or a
   relation taking into account the FLOBs.

  * ~exttuplesize~
   ( stream(tuple X) | rel(tuple X) [->] real ),
   Return the average size of the tuples within a stream
   or a relation taking into account the small FLOBs.

  * ~roottuplesize~
   ( stream(tuple X) | rel(tuple X) [->]  int ),
   Return the size of the attributes root part within a
   stream or a relation (without small FLOBs).

Simple datatypes ~X~, that have no FLOBs and for which ~noFlobType(X)~
in file ``operators.pl'' is defined, will be assumed to have no FLOBs.
Instead of querying Secondo for the attribute sizes, the rootsizes stored
in file ``storedTypeSizes.pl'' for each Secondo datatype are used.

Prior to asserting the ~storedSpell/3~, ~storedAttrSize/7~,  ~storedTupleSize/3~,
~storedCard/3~, ~storedOrderings/3~ facts, all such facts regarding the relation
and its attributes are retracted.

*/

updateRelationSchema(DCrel) :-
  dm(dbhandling,['\nTry: updateRelationSchema(',DCrel,').']),
  ( databaseName(DB)
    -> true
    ;  ( write_list(['\nERROR:\tCannot update relation schema for \'',DCrel,
                     '\': No database open.']), nl,
         throw(error_SQL(database_updateRelationSchema(DCrel):noDatabaseOpen)),
         fail, !
       )
  ),
  retractStoredInformation(DCrel),
  ( ( secondoCatalogInfo(DCrel,ExtRel, _, TypeExpr),
      (   TypeExpr = [[rel, [tuple, ExtAttrList]]]
        ; TypeExpr = [[trel, [tuple, ExtAttrList]]]
      ),
      internalName2externalName(IntRel,ExtRel)
    )
    -> true
    ;  ( write_list(['ERROR:\tCannot retrieve information on relation \'',
                     DCrel,
                     '\':\n','--->\tNo matching relation found in catalog.']),
         nl,
         throw(error_SQL(database_updateRelationSchema(DCrel):lostObject)),
         fail, !
       )
  ),
  assert(storedSpell(DB,DCrel,IntRel)),           % XRIS: could be omitted!
  ( retrieveAttributes(DB, ExtRel, DCrel, ExtAttrList, DCattrList)
    -> true
    ; (  write_list(['\nERROR:\tFailure retrieving attribute information on ',
                     'relation \'',ExtRel,'\': .']), nl,
         throw(error_SQL(database_updateRelationSchema(DCrel)
                        :cannotRetrieveAttribute)),
         fail, !
      )
  ),
  assert(storedRel(DB,DCrel,DCattrList)),
  ( card(DCrel, _)                                % get cardinality
    -> true
    ;  ( write_list(['\nERROR:\tFailure retrieving cardinality for \'',ExtRel,
                     '\'.']), nl,
         throw(error_SQL(database_updateRelationSchema(DCrel)
                        :cannotRetrieveCardinality)),
         fail, !
       )
  ),
  ( tuplesize(DCrel, _)                           % get tuple size
    -> true
    ;  ( write_list(['\nERROR:\tFailure retrieving tuplesize for \'',ExtRel,
                     '\'.']), nl,
         throw(error_SQL(database_updateRelationSchema(DCrel)
                        :cannotRetrieveTuplesize)),
         fail, !
       )
  ),
  write_list(['\nINFO:\tUpdated information on relation \'',ExtRel,'\'.']), nl,
  !.

updateRelationSchema(DCrel) :-
  write_list(['ERROR:\tupdateRelationSchema failed for \'',DCrel,'\'.']), nl,
  throw(error_SQL(database_updateRelationSchema(DCrel)
                 :cannotLookupRelationschema)).


/*
2.1 Processing Relation Schemas

---- retrieveAttributes(+DB, +ExtRel, +DCrel, +ExtAttrList, -DCattrList)
----

Recursively creates and asserts spelling information for a attribute list
~ExtAttrList~ containing attribute names in external spelling.
Returns a list ~DCattrList~ with all attribute names in down-cased spelling.

If the cardinality of ~ExtRel~ is 0, and the attribute has a type of variable
size, Secondo will return UNDEF for attrsize, rootattrsize and extattrsize.
To avoid problems in other parts of the optimizer, we will assert
storedAttrSize(, ..., 1,0,0) in that case.

*/
retrieveAttributes(_, _, _, [], []).
retrieveAttributes(DB,ExtRel,DCrel,[[ExtAttr,Type]|Rest],[DCattr|DCattrList]) :-
  dcName2externalName(DCattr,ExtAttr),
  internalName2externalName(IntAttr,ExtAttr),
  ( noFlobType(Type)
    *-> ( secDatatype(Type, CoreAttrSize),
          InFlobSize   is 0,
          ExtFlobSize  is 0
        )
      ; (
          getTotalAttrSize(ExtRel, ExtAttr, AttrSize1),
          getExtAttrSize(ExtRel, ExtAttr, ExtAttrSize1),
          getRootAttrSize(ExtRel, ExtAttr, RootAttrSize1),
          ( (AttrSize1 = undef ; ExtAttrSize1 = undef ; RootAttrSize1 = undef)
            -> ( AttrSize is 1,
                 ExtAttrSize is 1,
                 RootAttrSize is 1
               )
            ;  ( AttrSize is AttrSize1,
                 ExtAttrSize is ExtAttrSize1,
                 RootAttrSize is RootAttrSize1
               )
          ),
          CoreAttrSize is RootAttrSize,
          InFlobSize   is ExtAttrSize - RootAttrSize,
          ExtFlobSize  is AttrSize - ExtAttrSize
        )
  ),
  assert(storedSpell(DB, DCrel:DCattr, IntAttr)),
  assert(storedAttrSize(DB, DCrel, DCattr, Type,
                        CoreAttrSize, InFlobSize, ExtFlobSize)),
  !,
  retrieveAttributes(DB, ExtRel, DCrel, Rest, DCattrList).


% query Secondo for root attr size of an attribute
totalAttrSizeQuery(RelE, AttrE, QueryAtom) :-
  secondoCatalogInfo(RelDC,RelE,_,_),
  systemTable(RelDC,_),
  concat_atom(['query ', RelE, ' feed attrsize[ ', AttrE, ' ]'],QueryAtom), !.

totalAttrSizeQuery(RelE, AttrE, QueryAtom) :-
  concat_atom(['query ', RelE, ' attrsize[ ', AttrE, ' ]'],QueryAtom), !.

getTotalAttrSize(RelE, AttrE, AttrSize) :-
  totalAttrSizeQuery(RelE, AttrE, QueryAtom),
  write(QueryAtom), nl,
  secondo(QueryAtom, [real, AttrSize]), !.

getTotalAttrSize(RelE, AttrE, _) :-
  write('\nERROR:\tSomething\'s wrong in getTotalAttrSize('), write(RelE),
  write(','), write(AttrE), write(').'),nl,                                   %'
  fail, !.


% query Secondo for root attr size of an attribute
rootAttrSizeQuery(RelE, AttrE, QueryAtom) :-
  secondoCatalogInfo(RelDC,RelE,_,_),
  systemTable(RelDC,_),
  concat_atom(['query ', RelE, ' feed rootattrsize[ ', AttrE, ' ]'],QueryAtom),
  !.

rootAttrSizeQuery(RelE, AttrE, QueryAtom) :-
  concat_atom(['query ', RelE, ' rootattrsize[ ', AttrE, ' ]'],QueryAtom),
  !.

getRootAttrSize(RelE, AttrE, RootAttrSize) :-
  rootAttrSizeQuery(RelE, AttrE, QueryAtom),
  secondo(QueryAtom, [int, RootAttrSize]), !.

getRootAttrSize(RelE, AttrE, _) :-
  write('\nERROR:\tSomething\'s wrong in getRootAttrSize('), write(RelE),
  write(','), write(AttrE), write(').'),nl,                                   %'
  fail, !.


% query Secondo for the extattrsize of an attribute
extAttrSizeQuery(RelE, AttrE, QueryAtom) :-
  secondoCatalogInfo(RelDC,RelE,_,_),
  systemTable(RelDC,_),
  concat_atom(['query ', RelE, ' feed extattrsize[ ', AttrE, ' ]'],QueryAtom),
  !.

extAttrSizeQuery(RelE, AttrE, QueryAtom) :-
  concat_atom(['query ', RelE, ' extattrsize[ ', AttrE, ' ]'],QueryAtom), !.

getExtAttrSize(RelE, AttrE, ExtAttrSize) :-
  extAttrSizeQuery(RelE, AttrE, QueryAtom),
  secondo(QueryAtom, [real, ExtAttrSize]), !.

getExtAttrSize(RelE, AttrE, _) :-
  write('\nERROR:\tSomething\'s wrong in getExtAttrSize('), write(RelE),
  write(','), write(AttrE), write(').'),nl,                                   %'
  fail, !.

/*
3 Maintaining Catalog Information

The catalog contains information on the names and types of database objects.
If a database is open, the optimizer can query for this information and store
it in its internal metadata. The catalog information may be changed between two
optimizer sessions. Therefore, the catalog information is not written to a
file.

The catalog information is maintained in dynamic facts

---- secondoCatalogInfo(ObjectNameDC,ObjectNameExtern,TypeList,TypeExprList)
----

~ObjectNameDC~ is the object's name in ~downcased spelling~. ~ObjectNameExtern~
is the object's name in ~external spelling~, ~TypeList~ is the nested list
representing the type information (which is actually not used at the moment),
~TypeExprList~ is a nested list providing the object's actual type.

The facts are also used to translate between different spelling schemas for
database objects.

The facts are ordered by the lexicographical ordering of object identifiers.
That means, that index objects respecting the naming convention should come
after their according relation objects.

3.1 Maintaining the Internal Catalog of Database Objects

---- refreshSecondoCatalogInfo
----

Retracts all ~secondoCatalogInfo/4~ facts. Then retrieves the catalog info from
the kernel and stores ~secondoCatalogInfo/4~ for each database object.

*/

:- dynamic(secondoCatalogInfo/4).

% recursively assert secondoCatalogInfo/4 facts for the passed object list
assertAllSecondoCatalogInfo([]) :- !.
assertAllSecondoCatalogInfo([Object|MoreObjects]) :-
  ( Object = ['OBJECT', ObjNameExt, TypeList, TypeExprList]
    -> (
          dcName2externalName(ObjNameDC,ObjNameExt),
          assert(secondoCatalogInfo(ObjNameDC,ObjNameExt,TypeList,TypeExprList))
       )
  ), !,
  assertAllSecondoCatalogInfo(MoreObjects).


refreshSecondoCatalogInfo :-
  dm(dbhandling,['\nTry: refreshSecondoCatalogInfo.']),
  retractall(secondoCatalogInfo(_,_,_,_)),
  ( databaseName(_)
    -> ( true )
    ;  (
         write('ERROR:\trefreshSecondoCatalogInfo failed!'), nl,
         write('--->\tNo database open!'),
         throw(error_SQL(database_refreshSecondoCatalogInfo:noDatabaseOpen)),
         !, fail
       )
  ), !,
  ( getSecondoList(ObjList)
    -> ( true )
    ;  (
         write('ERROR:\trefreshSecondoCatalogInfo failed!'), nl,
         write('--->\tRetrieving Catalog failed!'),
         throw(error_SQL(database_refreshSecondoCatalogInfo
                        :cannotRetrieveCatalog)),
         !, fail
       )
  ), !,
  sort(ObjList,ObjListSorted),   % sort lexicographically and remove duplicates
  assertAllSecondoCatalogInfo(ObjListSorted),
  !.

/*
---- checkObjectNamingConvention/0
----

Check whether any objects within the ~secondoCatalogInfo~ facts violates the
unique downcased naming convention.

If so, an exception is thrown. Otherwise the predicate succeeds.

*/

checkObjectNamingConvention :-
  dm(dbhandling,['\nTry: checkObjectNamingConvention.']),
  refreshSecondoCatalogInfo,
  findall(Clashes,(
              secondoCatalogInfo(ObjNameDC,_,_,_),
              findall( ObjectNameExt1,
                       secondoCatalogInfo(ObjNameDC,ObjectNameExt1,_,_),
                       ObjsHavingSameName
                     ),
              length(ObjsHavingSameName,NoObjsHavingSameName),
              ( NoObjsHavingSameName > 1
                -> Clashes = ObjsHavingSameName
                ;  Clashes = []
              )
            ),
          FoundClashes),
  flatten(FoundClashes,FlatClashes),
  sort(FlatClashes,SortedClashes),
  length(SortedClashes,NoInstances),
  ( NoInstances > 0
    -> (
         write_list(
         ['\nERROR:\tThe database contains objects violating the unique',
         '\n--->\tdowncased naming convention.',
         '\n--->\tPlease rename the following objects such that no two of\n',
         '\n--->\ttheir names are the same, when using lower case \n',
         '\n--->\tletters only:\n',
         '\n--->\t\t',SortedClashes]),
         nl, nl,
         throw(error_SQL(database_checkObjectNamingConvention(SortedClashes)
                        :namingConventionVioled))
       )
    ; true
  ),
  !.

/*
3.2 Identifying Added and Lost Catalog Entries

The following predicates are used to find the names (in ~down cases spelling~)
of database objects, that have been added or removed in comparison with the
persistent meta data.

*/

% identify relation objects, that are known, but not present in the catalog
findLostRelations(LostRelations) :-
  dm(dbhandling,['\nTry: findLostRelations(',LostRelations,').']),
  databaseName(DB),
  findall( DCRel,
           (
              storedRel(DB, DCRel, _),
              not(secondoCatalogInfo(DCRel,_,_,_))
           ),
           LostRelations
         ), !.

% identify  unknown relation objects within the catalog
findNewRelations(NewRelations) :-
  dm(dbhandling,['\nTry: findNewRelations(',NewRelations,').']),
  databaseName(DB),
  findall( DCRel,
           (
              secondoCatalogInfo(DCRel,_,_,[[rel, _]]),
              not(storedRel(DB, DCRel, _))
           ),
           NewRelations
         ), !.

% identify index objects, that are known, but not present in the catalog
findLostIndexes(LostIndexes) :-
  dm(dbhandling,['\nTry: findLostIndexes(',LostIndexes,').']),
  databaseName(DB),
  findall( DCindex,
           (
              storedIndex(DB,_,_,_,DCindex),
              not(secondoCatalogInfo(DCindex,_,_,_))
           ),
           LostIndexes
         ), !.

% identify  unknown index objects
findNewIndexes(NewIndexes) :-
  dm(dbhandling,['\nTry: findNewIndexes(',NewIndexes,').']),
  databaseName(DB),
  findall( DCindex,
           (
              secondoCatalogInfo(DCindex,ExtIndex,_,_),
              splitIndexObjectName(ExtIndex,ExtRel,ExtAttr,IndexCode,_,no),
              logicalIndexType(IndexCode,LogicalType,_,_,_,_,_,_),
              dcName2externalName(DCrel,ExtRel),
              dcName2externalName(DCAttr,ExtAttr),
              not(storedIndex(DB, DCrel, DCAttr, LogicalType, DCindex))
           ),
           NewIndexes
         ), !.

/*
3.3 Keeping Catalog and Meta Data Consistent

To keep the information base of the optimizer consistent with the kernel's
catalog of database objects, we need to handle Catalog Entries corresponding to
newly added or deleted database objects.

We only keep care of changes to relation and index objects, as information on
other kinds of objects is not maintained in the optimizer's persistent
information base.

---- updateCatalog
----

The Catalog is retrieved from the database kernel. It is examined and meta data
is added to the internal tables. The consistency of the database and meta data
is checked.

  1 First, a table with information on available data types and their sizes is
    created by calling ~readSecondoTypeSizes~

  2 Second, by comparing the stored metadata with the secondoCatalogInfo, we
    check, whether some objects (relations, indexes) have been lost.
    If so, we retract the according facts from the metadata. We then also remove
    the according ~small~ and ~sample~ objects and all information linked to
    them.
    For each already known index, we check whether the base relation exists.
    If not, we delete the index (and the small index) and retract according metadata.

  3 Then, by comparing the stored metadata with the secondoCatalogInfo, we
    check for new objects in the database. For each new relation object, we
    retrieve metadata (relation schema, cardinality, attribute sizes, orderings).
    We possibly create sample relations and the small relation.
    For each new index, we check whether the base relation exists. If not,
    we delete the index (and the small index) and retract according metadata.
    If so, we check wheather we need to create a small index.

This sequence guarantees, that small relations (and samples) are created before
small indexes are possibly created (which rely on the existence of small relations).

Also, if a relation is removed, this will be detected when searching for lost
indexes and a list of obsolete indexes will be printed.

~updateCatalog~ must be called whenever a database object has been added or
deleted (e.g. after each 'let' or 'delete' command. You do not need to call
it after an 'update' command, as this will neither change the object identifier,
nor the object type (though it might change cardinalities, tuplesizes,
attribute sizes, etc.).

*/
:-assert(helpLine(updateCatalog,0,[],
   'Re-read the current DB´s catalog and update the metadata.')).

updateCatalog :-
  dm(dbhandling,['\nTry: updateCatalog.']),
  readSecondoTypeSizes,
  refreshSecondoCatalogInfo,
  findLostRelations(LostRelations),
  findall(_,(member(LR,LostRelations),handleLostRelation(LR)),_),
  findLostIndexes(LostIndexes),
  findall(_,(member(LI,LostIndexes),handleLostIndex(LI)),_),
  findNewRelations(NewRelations),
  findall(_,(member(NR,NewRelations),handleNewRelation(NR)),_),
  findNewIndexes(NewIndexes),
  findall(_,(member(NI,NewIndexes),handleNewIndex(NI)),_),
  !.

updateCatalog :-
  write('ERROR:\tupdateCatalog/0 failed! That schould not happen!'), nl,
  !, fail.

% Do not handle small relations:
handleNewRelation(DCrel) :-
  dm(dbhandling,['\nTry: handleNewRelation(',DCrel,').']),
  sub_atom(DCrel,_,_,0,'_small'), !.

% Handle non-small relations:
handleNewRelation(DCrel) :-
  dm(dbhandling,['\nTry: handleNewRelation(',DCrel,').']),
  ( databaseName(_)
    -> true
   ;  (  write('ERROR:\tNo database open. Cannot handle new relation!'),nl,
         throw(error_SQL(database_handleNewRelation(DCrel):noDatabaseOpen)),
         !, fail
      )
  ),
  ( ( secondoCatalogInfo(DCrel,ExtRel,_,Type),
      Type = [[rel, [tuple, _]]]
    )
    -> true
    ;  (  write_list(['\nERROR:\tObject \'',DCrel,
                      '\' unknown or not a relation.']),nl,
          throw(error_SQL(database_handleNewRelation(DCrel):typeError)), !, fail
       )
  ),
  write_list(['\nINFO:\tRelation \'',ExtRel,
              '\' has been added to the database.\n',
              '--->\tNow retrieving related metadata...']),nl,
  updateRelationSchema(DCrel),   % get spellings, schema, attr names and sizes,
                                 % tuple size, cardinality,
                                 % also retract ordering information
  ( ( not(sub_atom(DCrel, _, _, 0, '_small')),
      not(sub_atom(DCrel, _, _, 1, '_sample_')),
      not(systemTable(DCrel,_))
    )
    -> ( createSampleRelationObjectsForRel(DCrel),
         createSmallRelationObjectForRel(DCrel)
       )
    ;  true
  ),
  !.

handleNewRelation(DCrel) :-
  write_list(['\nERROR:\tSomething went wrong during updating relation \'',
              DCrel,'\'.']),nl,                                           %'
  throw(error_SQL(database_handleNewRelation(DCrel):unknownError)), !,
  fail.

% Do not handle small indexes:
handleNewIndex(DCindex) :-
  dm(dbhandling,['\nTry: handleNewIndex(',DCindex,').']),
  sub_atom(DCindex,_,_,0,'_small'), !.

% Handle non-small indexes:
handleNewIndex(DCindex) :-
  dm(dbhandling,['\nTry: handleNewIndex(',DCindex,').']),
  ( databaseName(DB)
    -> true
    ;  (  write('ERROR:\tNo database open. Cannot handle new index!'),nl,
          throw(error_SQL(database_handleNewIndex(DCindex):noDatabaseOpen)),
          !, fail
       )
  ),
  secondoCatalogInfo(DCindex,ExtIndex,_,Type),
  (
    ( indexType(PhysIndexType,Type),
      splitIndexObjectName(ExtIndex,ExtRel,ExtAttr,IndexCode,_,no),
                                              % ignore small indexes
      logicalIndexType(IndexCode, LogicalIndexType, PhysIndexType,
                       SupportedAttributeTypeList,
                       _, _, _, _),
      dcName2externalName(DCrel,ExtRel),
      dcName2externalName(DCattr,ExtAttr),
      storedRel(DB,DCrel,AttrList),
      member(DCattr,AttrList),
      storedAttrSize(DB,DCrel,DCattr,AttrType,_,_,_),
      member(AttrType,SupportedAttributeTypeList)
    )
    -> ( write_list(['\nINFO:\tIndex \'',ExtIndex,
                     '\' has been added to the database.',
                     '\n--->\tThe index is for ',ExtRel,':',ExtAttr,
                     '\n--->\tThe index type is ',LogicalIndexType,'.']),
         nl,
         ( optimizerOption(entropy)
           -> ( ( write('\nINFO:\tNow trying to create the small index.'), nl,
                  createSmallIndex(DCrel,DCattr,LogicalIndexType)
                )
                -> ( write('--->\tSmall index available.'), nl )
                ;  ( write('WARNING:\tCreating small index failed!'), nl )
              )
           ; ( write('\nINFO:\tCreation of the small index not required.'),
               nl
             )
         ),
         retractall(storedNoIndex(DB,DCrel,DCattr)),
         assert(storedIndex(DB,DCrel,DCattr,LogicalIndexType,DCindex))
       )
    ;  ( write_list(['\nWARNING:\tDatabase object \'',ExtIndex,
                     '\' seems to be an unsupported index.',
                     '\n--->\tPossibly it has been named in a wrong way?',
                     '\n--->\tPossibly the according relation does not exist?',
                     '\n--->\tPossibly unsupported attribute or index type?']),
         nl
       )
  ), !.

handleLostRelation(DCrel) :-
  dm(dbhandling,['\nTry: handleLostRelation(',DCrel,').']),
  ( databaseName(_)
    -> true
   ;  (  write('ERROR:\tNo database open. Cannot handle lost relation!'),nl,
          throw(error_SQL(database_handleLostRelation(DCrel):noDatabaseOpen)),
          !, fail
      )
  ),
  write_list(['\nWARNING:\tRelation \'',DCrel,
              '\' has been removed from the database.',
              '\nINFO:\tNow removing related metadata...']),nl,
  % possibly delete small relation
  ( ( atom_concat(DCrel,'_small',DCsmallRel),
      secondoCatalogInfo(DCsmallRel,ExtSmallRel,_,_)
    )
    -> ( write('--->\tDeleting small relation: '),
         (( deleteObject(ExtSmallRel),
            retractAllStoredInfo(DCsmallRel)
          )
          -> write('succeeded.\n')
          ;  write('FAILED!\n')
         )
       )
    ;  true
  ),
  % possibly delete sample_s relation
  ( ( atom_concat(DCrel,'_sample_s',DCsamplesRel),
      secondoCatalogInfo(DCsamplesRel,ExtSamplesRel,_,_)
    )
    -> ( write('--->\tDeleting _sample_s relation: '),
         (( deleteObject(ExtSamplesRel),
            retractAllStoredInfo(DCsamplesRel)
          )
          -> write('succeeded.\n')
          ;  write('FAILED!\n')
         )
       )
    ;  true
  ),
  % possibly delete sample_j relation
  ( ( atom_concat(DCrel,'_sample_j',DCsamplejRel),
      secondoCatalogInfo(DCsamplejRel,ExtSamplejRel,_,_)
    )
    -> ( write('--->\tDeleting _sample_j relation: '),
         (( deleteObject(ExtSamplejRel),
            retractAllStoredInfo(DCsamplejRel)
          )
          -> write('succeeded.\n')
          ;  write('FAILED!\n')
         )
       )
    ;  true
  ),
  % prompt obsolete indexes
  findall( DCindex,
           ( storedIndex(DB,DCrel,_,_,DCindex),
             secondoCatalogInfo(DCindex,ExtIndex,_,_),
             write_list(['\n--->\tIndex \'',ExtIndex,'\' is obsolete.']), nl
           ),
           ObsoleteIndexes),
  ( ObsoleteIndexes = []
    -> true
    ;  ( write('\n--->\tObsolete indexes are not removed automatically!'),nl )
  ),
  % possibly delete small indexes
  findall( _,
           ( storedIndex(DB,DCrel,_,_,DCindex),
             atom_concat(DCindex,'_small',DCindexSmall),
             secondoCatalogInfo(DCindexSmall,ExtIndexSmall,_,_),
             write_list(['--->\tDeleting small index \'',ExtIndexSmall,'\': ']),
             ( deleteObject(ExtIndexSmall)
               -> ( write('succeeded.\n'),
                    retractall(storedIndex(DB,DCrel,_,_,DCindexSmall))
                  )
               ;  write('FAILED!\n')
             )
           ),
           _),
  retractAllStoredInfo(DCrel), % retract all stored meta data
  updateCatalog,
  !.


handleLostIndex(DCindex) :-
  dm(dbhandling,['\nTry: handleLostIndex(',DCindex,').']),
  ( databaseName(DB)
    -> true
   ;  (  write('ERROR:\tNo database open. Cannot handle lost index!'),nl,
          throw(error_SQL(database_handleLostIndex(DCindex):noDatabaseOpen)),
          !, fail
      )
  ),
  not(sub_atom(DCindex, _, _, 0, '_small')),  % ignore small index objects
                                              % (they should not be appera here
                                              % anyway)
  write_list(['\nWARNING:\tIndex \'',DCindex,
              '\' has been removed from the database.',
              '\n--->\tNow removing related metadata...']),nl,
  storedIndex(DB, DCrel, DCattr, IndexType, DCindex),
  % retract storedIndex
  retractall(storedIndex(DB, DCrel, DCattr, IndexType, DCindex)), !,
  % possibly update storedNoIndex/3
  ( storedIndex(DB, DCrel, DCattr, _, _)
    -> true
    ;  assert(storedNoIndex(DB, DCrel, DCattr))
  ),
  % possibly delete small index
  findall( _,
           ( atom_concat(DCindex,'_small',DCindexSmall),
             secondoCatalogInfo(DCindexSmall,ExtIndexSmall,_,_),
             write_list(['--->\tDeleting small index \'',ExtIndexSmall,'\': ']),
             ( deleteObject(ExtIndexSmall)
               -> ( write('succeeded.\n'),
                    retractall(storedIndex(DB,DCrel,_,_,DCindexSmall))
                  )
               ;  write('FAILED!\n')
             )
           ),
           _),
  write('--->\t... finished removing metadata.'),
  !.


/*
----
     ensureSamplesExist
     ensureSmallObjectsExist
----

These predicates check, whether for all relation (and index) objects within
the current database, also samples (small objects) exist, when running without
~optimizerOption(dynamicSample)~ (with ~optimizerOption(entropy)~).
If such an object is found missing, it will be created.

*/

ensureSamplesExist :-       % nothing to do, when using dynamic sampling
  dm(dbhandling,['\nTry: ensureSamplesExist.']),
  optimizerOption(dynamicSample), !.

ensureSamplesExist :-
  dm(dbhandling,['\nTry: ensureSamplesExist.']),
  not(optimizerOption(dynamicSample)),
  write_list(['\nINFO:\tEnsuring, that sample relations exist...']), nl,
  databaseName(DB),
  findall(_,
          ( storedRel(DB,DCrel,_),
            not(sub_atom(DCrel,_,_,0,'_small')),
            not(sub_atom(DCrel,_,_,1,'_sample_')),
            secondoCatalogInfo(DCrel,_,_,[[rel, [tuple, _]]]),
            createSampleRelationObjectsForRel(DCrel)
          ),
          _),
  updateCatalog,
  write_list(['\nINFO:\tFinished ensuring, that sample relations exist.']), nl,
  !.

ensureSmallObjectsExist :-  % nothing to do, when not using entropy approach
  dm(dbhandling,['\nTry: ensureSmallObjectsExist.']),
  not(optimizerOption(entropy)), !.

ensureSmallObjectsExist :-
  dm(dbhandling,['\nTry: ensureSmallObjectsExist.']),
  optimizerOption(entropy),
  ensureSmallRelationsExist,
  ensureSmallIndexesExist,
  !.

ensureSmallObjectsExist :-
  write('\nERROR:\tPredicate ensureSmallObjectsExist failed!'), nl,
  throw(error_SQL(database_ensureSmallObjectsExist:unknownError)),
  fail, !.


ensureSmallRelationsExist :-
  dm(dbhandling,['\nTry: ensureSmallRelationsExist.']),
  write_list(['\nINFO:\tEnsuring, that small relations exist...']), nl,
  databaseName(DB),
  findall(_,
          ( storedRel(DB,DCrel,_),
            not(sub_atom(DCrel,_,_,0,'_small')),
            not(sub_atom(DCrel,_,_,1,'_sample_')),
            secondoCatalogInfo(DCrel,_,_,[[rel, [tuple, _]]]),
            atom_concat(DCrel,'_small',DCsmallRel),
            not(secondoCatalogInfo(DCsmallRel,_,_,[[rel, [tuple, _]]])),
            createSmallRelationObjectForRel(DCrel)
          ),
          _),
  updateCatalog,
  write_list(['\nINFO:\tFinished ensuring, that small relations exist.']), nl,
  !.

ensureSmallRelationsExist :-
  dm(dbhandling,['\nTry: ensureSmallRelationsExist.']),
  write('\nERROR:\tPredicate ensureSmallRelationsExist failed!'), nl,
  throw(error_SQL(database_ensureSmallRelationsExist:unknownError)),
  fail, !.


ensureSmallIndexesExist :-
  dm(dbhandling,['\nTry: ensureSmallIndexesExist.']),
  write_list(['\nINFO:\tEnsuring, that small indexes exist...']), nl,
  databaseName(DB),
  findall(_,
          ( storedIndex(DB,DCrel,DCattr,LogicalIndexType,DCindex),
            not(sub_atom(DCindex,_,_,0,'_small')),
            secondoCatalogInfo(DCindex,_,_,_),
            atom_concat(DCindex,'_small',DCsmallIndex),
            not(secondoCatalogInfo(DCsmallIndex,_,_,_)),
            createSmallIndex(DCrel,DCattr,LogicalIndexType)
          ),
          _),
  updateCatalog,
  write_list(['\nINFO:\tFinished ensuring, that small indexes exist.']), nl,
  !.

ensureSmallIndexesExist :-
  write('\nERROR:\tPredicate ensureSmallIndexesExist failed!'), nl,
  throw(error_SQL(database_ensureSmallIndexesExist:unknownError)),
  fail, !.


/*
3.4 Auxiliary Predicates

3.4.1 Deleting Objects

---- deleteObject(+ExtObjName)
----

Deletes the database object named ~ExtObjName~ (external spelling).
Fails, if deletion does not succeed.

*/


deleteObject(ExtObjName) :-
  dm(dbhandling,['\nTry: deleteObject(',ExtObjName,').']),
  concat_atom(['delete ', ExtObjName], '', QueryAtom),
  secondo_direct(QueryAtom).


/*
3.4.2 Retracting Relation-Related Meta Data

---- retractAllStoredInfo(+DCrel)
----

Retracts all stored meta data on relation ~DCrel~ (given in down-cased spelling).

*/
retractAllStoredInfo(DCrel) :-
  dm(dbhandling,['\nTry: retractAllStoredInfo(',DCrel,').']),
  databaseName(DB),
  retractSels(Rel),
  retractPETs(Rel),
  retractall(storedTupleSize(DB, DCrel, _)),
  retractall(storedCard(DB, DCrel, _)),
  retractall(storedOrderings(DB, DCrel, _)),
  retractall(storedIndex(DB, DCrel, _, _, _)),
  retractall(storedNoIndex(DB, DCrel, _)),
  retractall(storedAttrSize(DB, DCrel, _, _, _, _, _)),
  retractall(storedRel(DB, DCrel, _)),
  retractall(storedSpell(DB, DCrel:_, _)),
  !.


/*
4.4.3 Creating Sample Relations

Sample Relations are used to determine the selectivity of predicates occuring
in the WHERE clause of a conjunctice database query.

In the standard mode of the optimizer, two different samples are created once
(one smaller for join, one large for selection predicates) and stored within
the database.

*/

% do not create static sample relations, if dynamic sampling is activated
createSampleRelationObjectsForRel(X) :-
  dm(dbhandling,['\nTry: createSampleRelationObjectsForRel(',X,').']),
  optimizerOption(dynamicSample), !.

createSampleRelationObjectsForRel(DCrel) :-
  dm(dbhandling,['\nTry: createSampleRelationObjectsForRel(',DCrel,').']),
  not(optimizerOption(dynamicSample)),
  createSampleJ(DCrel),
  createSampleS(DCrel),
  !.


/*
4.4.5 Creating Small Relations

Create small relations for use with the entropy optimizer. Relations are classified into three groups called ~small~, ~middle~, and ~large~ by the ~classify~ predicate. For each group, sample sizes can be set differently. Currently ~small~ sizes are determined as follows:

  * small = less than 1000 tuples: full size

  * middle = between 1000 and 100000 tuples: 10 [%], but at least 1000 tuples

  * large = more than 100000 tuples, 1 [%], but at least 10000 tuples

This schema is chosen to have sample sizes grow monotonically.

In addition, an attribute is added for uniquely identifying tuples in the small relations with name ~xxxID[<]relname[>]~ (~[<]relname[>]~ starting with a lower case letter). This is needed for selfjoin correction in the entropy optimizer.

---- createSmallRelationObjectForRel(+DCrel)
----

This predicate creates a small relation with a canonical name and automatically determined cardinality.
~DCrel~ is the name of the original relation ind ~down cased spelling~.

The small object is only created, when optimizerOption(entropy) is set.

*/


/*
Classify the relation ~DCrel~ to one of three categories

*/
classifyRel(DCrel, SizeClass) :-
  dm(dbhandling,['\nTry: classifyRel(',DCrel,',',SizeClass,').']),
  card(DCrel, Size),
  member([LowerSizeBound, SizeClass],[[100001,large],[1000,medium],[0,small]]),
  Size >= LowerSizeBound, !.

/*
Create the canonocal small object name for an arbitrary name
(in dc- or external spelling) by appending '\_small' to it.

*/
getSmallName(Name, NameSmall) :-
  dm(dbhandling,['\nTry: getSmallName(',Name,',',NameSmall,').']),
  ( atomic(Name)
    -> atom_concat(Name,'_small',NameSmall)
    ; ( write('ERROR:\tName must be atomic!'), nl,
        throw(error_SQL(database_getSmallName(Name, NameSmall)
                       :wrongInstantiationPattern)),
        fail
      )
  ), !.

% case: optimizerOption(entropy) not set
createSmallRelationObjectForRel(X) :-
  dm(dbhandling,['\nTry: createSmallRelationObjectForRel(',X,').']),
  not(optimizerOption(entropy)), !.

% case: small relation already present
createSmallRelationObjectForRel(DCrel) :-
  dm(dbhandling,['\nTry: createSmallRelationObjectForRel(',DCrel,').']),
  optimizerOption(entropy),
  getSmallName(DCrel,DCrelSmall),
  secondoCatalogInfo(DCrelSmall,_,_,_).

% case: need to create small relation
createSmallRelationObjectForRel(DCrel)  :-
  dm(dbhandling,['\nTry: createSmallRelationObjectForRel(',DCrel,').']),
  optimizerOption(entropy),
  classifyRel(DCrel, SizeClass),
  member([SizeClass, MinSize, Percent], % lookup sampling parameters in list:
         [ [small, 0, 1], [medium, 1000, 0.1], [large, 10000, 0.01] ]),
  dcName2externalName(DCrel,ExtRel),
  getSmallName(ExtRel,ExtRelSmall),
  buildSmallRelation(ExtRel, ExtRelSmall, MinSize, Percent), !.

/*
---- buildSmallRelation(+ExtRel, +ExtRelSmall, +MinSize, +Percent) :-
----

Build a small relation for relation ~ExtRel~ (in external spelling), named
~ExtRelSmall~ (in external spelling). ~MinSize~ is the desired minimal size,
and ~Percent~ is the desired minimal percentage of tuples for use in the sample
operator. Add a unique identifier attribute ~xxxID[<]relname[>]~ by numbering
tuples sequentially. If ~MinSize~ = 0, no sampling is needed.

*/
buildSmallRelation(ExtRel, ExtRelSmall, 0, X) :-
  dm(dbhandling,['\nTry: buildSmallRelation(',ExtRel,',',
                 ExtRelSmall,',0,',X,').']),
  dcName2externalName(DCrel,ExtRel),
  dcName2externalName(DCrelSmall,ExtRelSmall),
  atom_concat('xxxID', DCrel, IDAttr),
  % just add tuple counter...
  concat_atom(['derive ', ExtRelSmall, ' = ', ExtRel,
    ' feed extend[', IDAttr, ': seqnext()] consume'], '', QueryAtom),
  tryCreate(QueryAtom),
  card(DCrel, SmallCard),
  databaseName(DB),
  assert(storedCard(DB, DCrelSmall, SmallCard)),
  !.

buildSmallRelation(ExtRel, ExtRelSmall, MinSize, Percent) :-
  dm(dbhandling,['\nTry: buildSmallRelation(',ExtRel,',',
                 ExtRelSmall,',',MinSize,',',Percent,').']),
  dcName2externalName(DCrel,ExtRel),
  dcName2externalName(DCrelSmall,ExtRelSmall),
  atom_concat('xxxID', DCrel, IDAttr),
  concat_atom(['derive ', ExtRelSmall, ' = ', ExtRel,
    ' sample[', MinSize, ', ', Percent, '] extend[', IDAttr,
    ': seqnext()] consume'], '',
    QueryAtom),
  tryCreate(QueryAtom),
  card(DCrel, Card),
  SmallCard is truncate(min(Card, max(MinSize, Card * Percent))),
  databaseName(DB),
  assert(storedCard(DB, DCrelSmall, SmallCard)),
  !.

buildSmallRelation(ExtRel, ExtRelSmall, MinSize, Percent) :-
  write('ERROR:\tbuildSmallRelation/4 somehow failed.'), nl,
  throw(error_SQL(database_buildSmallRelation(ExtRel, ExtRelSmall,
                                                MinSize, Percent)
                                                :unknownError)).

/*
---- createSmall(+DCRel, +Size) :-
----

Create small relation manually, for non-standard databases.

*/

createSmall(DCrel, Size)  :-
  dm(dbhandling,['\nTry: createSmall(',DCrel,',',Size,').']),
  ( ( ground(DCrel),
      ground(Size),
      number(Size),
      dcName2externalName(DCrel2,DCrel),
      DCrel2 = DCrel
    )
    -> true
    ;  ( write('ERROR:\tIn createSmall(Rel, Size), Rel must be typed in lower'),
         write('\n\tcase letters, and Size must be a number.'), nl,
         throw(error_SQL(database_createSmall(DCrel, Size):wrongSpelling)),
         fail
       )
  ),
  getSmallName(ExtRel,ExtRelSmall),
  ( secondoCatalogInfo(DCrel,ExtRel,_,[[rel, [tuple, _]]])
    -> buildSmallRelation(ExtRel, ExtRelSmall, Size, 0.000001)
    ;  ( write('ERROR:\tThat relation does not exist, or no database is open.'),
         nl,
         throw(error_SQL(database_createSmall(DCrel, Size):lostObject)),
         fail
       )
  ),
  !.


/*
4.4.6 Creating Sample Relations

Sample relations are created automatically with predefined cardinalities, unless
a defined threshold (total in-memory size) is reached.

In these cases, the user is prompted to create sample relations on his own,
specifying the wanted cardinality.

----
  getSampleSname(+Name, -SampleSname)
  getSampleJname(+Name, -SampleJname)
----

Creating sample object names. The names for relation samples are:

  * [<]relname[>]\_sample\_s for a selection sample on relation <relname>

  * [<]relname[>]\_sample\_j for a join sample on relation <relname>

~Name~ may be in dc- or external spelling.

Thepredicates use the following constants defined in ~operators.pl~:

----
secOptConstant(sampleScalingFactor, 0.00001).  % scaling factor for samples
secOptConstant(sampleSelMaxDiskSize, 2097152). % maximum byte size for samples
secOptConstant(sampleSelMinCard, 100).         % minimum cardinality for samples
secOptConstant(sampleSelMaxCard, 2000).        % maximum cardinality for samples
secOptConstant(sampleJoinMaxDiskSize, 2097152).% maximum byte size for samples
secOptConstant(sampleJoinMinCard, 50).         % minimum cardinality for samples
secOptConstant(sampleJoinMaxCard, 500).        % maximum cardinality for samples
----

*/

getSampleSname(Name, SampleSname) :-
  dm(dbhandling,['\nTry: getSampleSname(',Name,',',SampleSname,').']),
  ( atomic(Name)
    -> atom_concat(Name,'_sample_s',SampleSname)
    ; ( write('ERROR:\tName must be atomic!'), nl,
        throw(error_SQL(database_getSampleSname(Name, SampleSname)
                       :wrongInstantiationPattern)),
        fail
      )
  ), !.

getSampleJname(Name, SampleJname) :-
  dm(dbhandling,['\nTry: getSampleJname(',Name,',',SampleJname,').']),
  ( atomic(Name)
    -> atom_concat(Name,'_sample_j',SampleJname)
    ; ( write('ERROR:\tName must be atomic!'), nl,
        throw(error_SQL(database_getSampleSname(Name, SampleJname)
                       :wrongInstantiationPattern)),
        fail
      )
  ), !.

/*

----
 getStandardSampleCard(+DCRel, +CardMin, +CardMax, +SF, +MaxMem,
                       -CardStd, -MemStd, -CardRec, -MemRec)
 getSampleJsize(+DCrel, -CardStd, -MemStd, -CardRec, -MemRec)
 getSampleSsize(+DCrel, -CardStd, -MemStd, -CardRec, -MemRec)
----

Calculate the cardinality and size (in KB), that a standard join/selection
sample would take. The predicates are used to recommend sample sizes.

~DCRel~ is the relation name, ~CardMin~/~CardMax~ is the minumum/maximum cardinality for the sample,
~SF~ is the sample scaling factor, and ~MaxMem~ is the maximum memory consumption threshold (in KB).

~CardStd~ is the calculated standard cardinality, and ~MemStd~ the according memory size (in KB).
~CardRec~ is the recommended cardinality for the sample, and ~MemRec~ is its size (in KB).

*/

getStandardSampleCard(DCRel, CardMin, CardMax, SF, MaxMem,
                      CardStd, MemStd, CardRec, MemRec) :-
  dm(dbhandling,['\nTry: getStandardSampleCard(',DCRel,',',CardMin,',',
                 CardMax,',',SF,',',MaxMem,',',CardStd,',',MemStd,',',
                 CardRec,',',MemRec,').']),
  card(DCRel, Card),
  tuplesize(DCRel, TupleSize),
  dm(dbhandling,['\ngetStandardSampleCard(',DCRel,',...): Card=',Card]),
  dm(dbhandling,['\ngetStandardSampleCard(',DCRel,',...): TupleSize=',
                 TupleSize]),
  ( TupleSize =< 0
    -> ( write_list(['ERROR:\tTuplesize for relation ',DCRel,' < 1!']), nl,
         throw(error_SQL(database_getStandardSampleCard(DCRel, CardMin, CardMax,
                      SF, MaxMem,CardStd, MemStd, CardRec, MemRec)
                      :invalidTupleSize)),
         fail
       )
    ;  true
  ),
  CardStd     is min(CardMax,max(truncate(SF*Card),Card)),
  MemStd      is CardStd*TupleSize/1024,                     % in KB
  MaximumCard is truncate(min(min(CardMax, MaxMem*1024/TupleSize),Card)),
  secondoCatalogInfo(DCRel,ExtRel,_,_),
  ( (MemStd > MaxMem)            % standard sample is too large (both in KB)
    -> ( CardRec is MaximumCard, % recommend MaximumCard
         MemRec is CardRec*TupleSize/1024,               % in KB
         write_list(['WARNING:\tRelation \'',ExtRel,'\':',
                     '\n--->\tStandard sample card=', CardStd,' (',
                     MemStd,' KB) is above the\n','--->\tmaximum memory size=',
                     MaxMem, ' KB\n','--->\tTherefore, reduced sample card=',
                     CardRec, ' (',MemRec,' KB) is recommended.']), nl
       )
    ;  ( CardRec is CardStd,                % recommend SandardCard
         MemRec is CardRec*TupleSize/1024   % (recommended StandardSize in KB)
       )
  ),
  ( (CardRec < CardMin)
    -> ( write_list(['WARNING:\tRelation \'',ExtRel,'\':',
                     '\n--->\tRecommended sample card=',CardRec,' (',
                     MemRec,' KB) is below the\n','--->\trecommended minimum ',
                     'sample card=',CardMin,'.']), nl
       )
    ; true
  ),
  write_list(['CardStd=', CardStd, '\nMemStd=',MemStd, '\nCardRec=',CardRec,
              '\nMemRec=', MemRec]),nl,
  !.

getSampleJsize(DCRel, CardStd, MemStd, CardRec, MemRec) :-
  dm(dbhandling,['\nTry: getSampleJsize(',DCRel,',',CardStd,',',
                 MemStd,',',CardRec,',',MemRec,').']),
  secOptConstant(sampleJoinMinCard, CardMin),
  secOptConstant(sampleJoinMaxCard, CardMax),
  secOptConstant(sampleJoinMaxDiskSize, MaxMem),
  secOptConstant(sampleScalingFactor, SF),
  getStandardSampleCard(DCRel, CardMin, CardMax, SF, MaxMem,
                        CardStd, MemStd, CardRec, MemRec).

getSampleSsize(DCRel, CardStd, MemStd, CardRec, MemRec) :-
  dm(dbhandling,['\nTry: getSampleSsize(',DCRel,',',CardStd,',',
                 MemStd,',',CardRec,',',MemRec,').']),
  secOptConstant(sampleSelMinCard, CardMin),
  secOptConstant(sampleSelMaxCard, CardMax),
  secOptConstant(sampleSelMaxDiskSize, MaxMem),
  secOptConstant(sampleScalingFactor, SF),
  getStandardSampleCard(DCRel, CardMin, CardMax, SF, MaxMem,
                        CardStd, MemStd, CardRec, MemRec).


/*

----
  createSample(+ExtSample, +ExtRel, +SampleSize, -SampleCard)
  createSampleJ(+DCRel)
  createSampleS(+DCRel)
----

Create a random order sample ~ExtSample~ for relation ~ExtRel~ (both in external spelling)
with desired sample size ~SampleSize~. The actual sample size is returned as ~SampleCard~.

The predicates use some constants defined in file ~operators.pl~ to detremine the
correct size for sample relations.

----
secOptConstant(maximumSampleSize, X).
secOptConstant(minimumSelSampleCard, X).
secOptConstant(maximumSelSampleCard, X).
secOptConstant(minimumJoinSampleCard, X).
secOptConstant(maximumJoinSampleCard, X).
----

*/


createSample(ExtSample, ExtRel, RequestedCard, ActualCard) :-
  dm(dbhandling,['\nTry: createSample(',ExtSample,',',ExtRel,',',
                 RequestedCard,',',ActualCard,').']),
  sampleQuery(ExtSample, ExtRel, RequestedCard, QueryAtom),
  tryCreate(QueryAtom),
  dcName2externalName(DCrel,ExtRel),
  card(DCrel, Card),
  % the following line implements the calculation done by the sample-operator:
  secOptConstant(sampleScalingFactor, SF),
  ActualCard is truncate(min(Card, max(RequestedCard, Card*SF))).

createSampleJ(DCRel) :-
  dm(dbhandling,['\nTry: createSampleJ(',DCRel,').']),
  dcName2externalName(DCRel,ExtRel),
  sampleNameJ(ExtRel, Sample),
  ( secondoCatalogInfo(_,Sample,_,_)
    -> (write_list(['\nINFO:\tJoin-Sample for \'',ExtRel,'\' already exists.']),
        nl
       )
    ;  ( getSampleJsize(DCRel, CardStd, MemStd, CardRec, MemRec),
         write_list(['\nINFO:\tCreating Join-Sample for \'',ExtRel,'\'.']),nl,
         ( CardStd =< CardRec
           -> ( % sample size is ok
                SampleCard = CardStd,
                SampleSize = MemStd
              )
           ;  ( % sample size is too big
                optimizerOption(autoSamples)
                -> ( % automatically set sample size and force creation
                    SampleCard = CardRec,
                    SampleSize = MemRec,
                    write_list(['\tJoin sample size was automatically ',
                                'restricted\n','\tfrom ',(CardStd),'(',MemStd,
                                ' KB) to ',CardRec,'(',MemRec,' KB).']),nl
                   )
                ;  ( % leave sample creation to the user
                     write_list(['ERROR\tJoin sample is to large: ',CardStd,
                                 ' (=',MemStd,' KB).\n','Maximum size is ',
                                 CardRec,' (=',MemRec,' KB).']),nl,nl,
                     write_list(['\tPlease specify the concrete sample ',
                                 'cardinality\n','\tmanually and create the ',
                                 'sample e.g. by calling\n','\tcreateSamples(',
                                 DCRel,', ',CardRec,', ',CardRec,').']),nl,
                     throw(error_SQL(database_createSampleJ(DCRel)
                                    :requestUserInteraction)),
                     fail
                   )
              )
         ),
         createSample(Sample, ExtRel, SampleCard, ActualSampleCard),
         write_list(['\tSample cardinality=',ActualSampleCard]),nl,
         databaseName(DB),
         dcName2externalName(DCSample,Sample),
         assert(storedCard(DB, DCSample, SampleCard))
       )
  ), !.

createSampleS(DCRel) :-
  dm(dbhandling,['\nTry: createSampleS(',DCRel,').']),
  dcName2externalName(DCRel,ExtRel),
  sampleNameS(ExtRel, Sample),
  ( secondoCatalogInfo(_,Sample,_,_)
    -> ( write_list(['\nINFO:\tSelection-Sample for \'',ExtRel,
                     '\' already exists.']),
         nl
       )
    ;  ( getSampleSsize(DCRel, CardStd, MemStd, CardRec, MemRec),
         write_list(['\nINFO:\tCreating Selection-Sample for \'',ExtRel,'\'.']),
         nl,
         ( CardRec =< CardStd
           -> ( % sample size is ok
                SampleCard = CardStd,
                SampleSize = MemStd
              )
           ;  ( % sample size is too big
                optimizerOption(autoSamples)
                -> ( % automatically set sample size and force creation
                    SampleCard = CardRec,
                    SampleSize = MemRec,
                    write_list(['\tSelection sample size was automatically ',
                                'restricted\n','\tfrom ',(CardStd),'(',MemStd,
                                ' KB) to ',CardRec,'(',MemRec,' KB).']),nl
                   )
                ;    ( % leave sample creation to the user
                       write_list(['ERROR\tSelection sample is to large: ',
                                   CardStd,
                                   ' (=',MemStd,' KB).\n','Maximum size is ',
                                   CardRec,' (=',MemRec,' KB).']),nl,nl,
                       write_list(['\tPlease specify the concrete sample ',
                                   'cardinality\n','\tmanually and create the ',
                                   'sample e.g. by calling\n',
                                   '\tcreateSamples(',
                                   DCRel,', ',CardRec,', ',CardRec,').']),nl,
                       throw(error_SQL(database_createSampleS(DCRel)
                                      :requestUserInteraction)),
                       fail
                     )
              )
         ),
         createSample(Sample, ExtRel, SampleCard, ActualSampleCard),
         write_list(['\tSample cardinality=',ActualSampleCard]),nl,
         databaseName(DB),
         dcName2externalName(DCSample,Sample),
         assert(storedCard(DB, DCSample, SampleCard))
       )
  ), !.

% special case: trel (used for system tables)
sampleQuery(ExtSample, ExtRel, SampleSize, QueryAtom) :-
  dm(dbhandling,['\nTry: sampleQuery(',ExtSample,',',ExtRel,',',SampleSize,',',
                 QueryAtom,').']),
  sub_atom(ExtRel, _, _, _, 'SEC2'),
  concat_atom(['derive ', ExtSample, ' = ', ExtRel,
    ' feed head[', SampleSize, ']
      extend[xxxNo: randint(20000)] sortby[xxxNo asc] remove[xxxNo]
      consume'], '', QueryAtom).

% standard case: rel
sampleQuery(ExtSample, ExtRel, SampleSize, QueryAtom) :-
  dm(dbhandling,['\nTry: sampleQuery(',ExtSample,',',ExtRel,',',SampleSize,',',
                 QueryAtom,').']),
  secOptConstant(sampleScalingFactor, SF),
  concat_atom(['derive ', ExtSample, ' = ', ExtRel,
    ' sample[', SampleSize, ',', SF, ']
      extend[xxxNo: randint(20000)] sortby[xxxNo asc] remove[xxxNo]
      consume'], '', QueryAtom).


/*

----	createSamples(+DCRel, +SelectionCard, +JoinCard) :-
----

Create samples for ~DCRel~ (in down cased spellinf) manually, speciying the
size (cardinality) of selection and join samples.

*/

:- assert(helpLine(resizeSamples,3,
    [[+,'DCrel','The relation for which samples shall be created.'],
     [+,'SelectionCard','Cardinality for the selection sample.'],
     [+,'JoinCard','Cardinality for the (smaller) join sample.']],
    'Create (but do not replace) samples for a relation.')).

createSamples(DCrel, SelectionCard, JoinCard) :-
  dm(dbhandling,['\nTry: createSamples(',DCrel,',',SelectionCard,',',
                 JoinCard,').']),
  ( ( ground(DCrel), number(SelectionCard), number(JoinCard) )
    -> ( dcName2externalName(DCrel,ExtRel),
         sampleNameS(ExtRel, SampleS),
         createSample(SampleS, ExtRel, SelectionCard, _),
         sampleNameJ(ExtRel, SampleJ),
         createSample(SampleJ, ExtRel, JoinCard, _),
         updateCatalog                                % FIXME: added to force
                                                      % rescan and continue
                                                      % database setuo
       )
    ;  ( write('You must instantiate all 3 arguments of createSamples/3!'),
         nl,
         fail
       )
  ), !.

/*
----  resizeSamples(+DCRel, +SelectionCard, +JoinCard) :-
----

User-level command.
Resize (or possibly create) samples for ~DCRel~ (in down cased spellinf)
manually, speciying the size (cardinality) of selection and join samples.

Update the stored metadata on the relation samples.

*/

:- assert(helpLine(resizeSamples,3,
    [[+,'DCrel','The relation for which samples shall be created.'],
     [+,'SelectionCard','Cardinality for the selection sample.'],
     [+,'JoinCard','Cardinality for the (smaller) join sample.']],
    'Create (and replace existing) samples for a relation.')).

resizeSamples(DCrel, SelectionCard, JoinCard) :-
  dm(dbhandling,['\nTry: resizeSamples(',DCrel,',',SelectionCard,',',
                 JoinCard,').']),
  ( ( ground(DCrel), number(SelectionCard), number(JoinCard) )
    -> ( dcName2externalName(DCrel,ExtRel),
         sampleNameS(ExtRel, SampleS),
         ( secondoCatalogInfo(_,SampleS,_,_)
          -> resizeSample(SampleS, ExtRel, SelectionCard, _)
          ;  createSample(SampleS, ExtRel, SelectionCard, _)
         ),
         sampleNameJ(ExtRel, SampleJ),
         ( secondoCatalogInfo(_,SampleJ,_,_)
          -> resizeSample(SampleJ, ExtRel, JoinCard, _)
          ;  createSample(SampleJ, ExtRel, JoinCard, _)
         )
       )
    ;  ( write('You must instantiate all 3 arguments of createSamples/3!'),
         nl,
         fail
       )
  ), !.

resizeSample(ExtSample, ExtRel, RequestedCard, ActualCard) :-
  dm(dbhandling,['\nTry: resizeSample(',ExtSample,',',ExtRel,',',
                 RequestedCard,',',ActualCard,').']),
  deleteObject(ExtSample),
  dcName2externalName(DCsample,ExtSample),
  retractStoredInformation(DCsample),
  sampleQuery(ExtSample, ExtRel, RequestedCard, QueryAtom),
  tryCreate(QueryAtom),
  card(DCsample, ActualCard),
  updateCatalog.

/*
4 Querying, Storing and Loading Relation Schemas

*/

relation(Rel, AttrList) :-
  dm(dbhandling,['\nTry: relation(',Rel,',',AttrList,').']),
  databaseName(DB),
  storedRel(DB, Rel, AttrList), !.

readStoredRels :-
  retractall(storedRel(_, _, _)),
  [storedRels].

writeStoredRels :-
  open('storedRels.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredRel(FD), _),
  close(FD).

writeStoredRel(Stream) :-
  storedRel(DB, X, Y),
  write(Stream, storedRel(DB, X, Y)),
  write(Stream, '.\n').

showStoredRel :-
  storedRel(N, X, Y),
  write(N), write('.'), write(X), write(':\t'), write(Y), nl.

showStoredRels :-
  nl, write('Stored relation schemas:\n'),
  findall(_, showStoredRel, _).

:-
  dynamic(storedRel/3),
  at_halt(writeStoredRels),
  readStoredRels.


/*
5 Printing the complete Database Schema

By now, only stored meta information is handled by the optimizer and printed
when using the show- or write- predicates.

In contrast, ~showDatabaseSchema/0~ will print the schemas of all, not only
the actually used relations.

*/

showRelationAttrs([]).
showRelationAttrs([[AttrD, Type] | Rest]) :-
  % prints a list of [attributes,datatype]-pairs
  write(' '), write(AttrD), write(':'), write(Type), write(' '),
  showRelationAttrs(Rest), !.

showRelationSchemas([]).
  % filters all relation objects from the database schema
showRelationSchemas([Obj | ObjList]) :-
  Obj = ['OBJECT',Rel,_ | [[[_ | [[_ | [AttrList2]]]]]]],
  write('  '), write(Rel), write('  ['),
  showRelationAttrs(AttrList2),
  write(']\n'),
  showRelationSchemas(ObjList),
  !.
showRelationSchemas([_ | ObjList]) :-
  showRelationSchemas(ObjList),
  !.

showDatabaseSchema :-
  databaseName(DB),
  getSecondoList(ObjList),
  write('\nAll relation-schemas of database \''), write(DB), write('\':\n'),
  showRelationSchemas(ObjList),
  nl,
  write('(Type \'showDatabase.\' to see meta data
         collected by the optimizer.)\n'),
  !.


getSchema(Rel, Objs, AttrList) :-
  Member = ['OBJECT',Rel,_ | [[[rel | [[tuple | [AttrList]]]]]]],
  member(Member, Objs).


getSchemas(Rels):-
  getSecondoList(Objs),
  findall([Rel, AttrList], getSchema(Rel, Objs, AttrList), Rels).

disjointListPair(L, A, B) :-
  member(A, L),
  member(B, L),
  A \= B.

checkMember(O, L, R):-
  member(A, L),
  R = ['OBJECT', A | _],
  member(R, O).

restrictObjList(L, Objs):-
  getSecondoList(Otmp),
  findall( R, checkMember(Otmp, L, R), Objs).

getPairs(Rels, Pairs) :-
  findall([A, B], disjointListPair(Rels, A, B), Pairs).





/*
6 Spelling of Attribute Names

Due to the facts, that PROLOG interprets words beginning with a capital
letter as varibales and that Secondo allows arbitrary writing of
relation and attribute names, we have to find a convention. So, for
Secondo attribute names beginning with a small letter, the internal PROLOG
notation will be lc(name), which means, leaver the first letter as it is. If
the first letter of a Secondo name is written in upper case, then it is set to
lower case.

The PROLOG notation for ~pLz~ is ~lc(pLz)~ and for ~EMPLOYEE~ it'll be ~eMPLOYEE~.

6.1 Auxiliary Rules


---- getSecondoList(-ObjList)
----

Returns a list of Secondo objects, if available in the knowledge
base, otherwise a Secondo command is issued to get the list.

*/

getSecondoList(ObjList) :-
  secondo('list objects',[_, [_, [_ | ObjList]]]), !.

/*
6.2 Spelling of Attribute Names

---- spelling(Rel:Attr, Spelled) :-
----

Returns the internal spelling ~Spelled~ of dc-spelled attribute name ~Attr~.

~Spelled~ is available via the dynamic predicate ~storedSpell/3~.
Otherwise, the catalog info is searched and the spelling is stored for further
use.

*/

% return the internal spelling for a attribute name given in dc-spelling
%
getIntSpellingFromDCattrList(_, [], _) :- !, fail.
getIntSpellingFromDCattrList(DCattr,[[ExtAttr, _] | _], ExtAttr) :-
  dcName2externalName(DCattr,ExtAttr), !.
getIntSpellingFromDCattrList(DCattr,[[_, _] | Rest], ExtAttr) :-
    getIntSpellingFromDCattrList(DCattr,Rest,ExtAttr),
    !.

spelling(Rel:Attr, Spelled) :-
  dm(dbhandling,['\nTry: spelling(',Rel,':',Attr,',',Spelled,').']),
  databaseName(DB),
  ( storedSpell(DB, Rel:Attr, Spelled)
    ; (
        secondoCatalogInfo(Rel,_,_,TypeList),
        TypeList = [[rel, [tuple, AttrList]]],
        getIntSpellingFromDCattrList(Attr,AttrList,Spelled), !,
        assert(storedSpell(DB, Rel:Attr, Spelled))
      )
  ),
  !.

spelling(DCR:DCA, Rext) :- !,
  write_list(['\tERROR:\tCannot translate attribute \'',DCR,':',DCA,'\'.']),nl,
  throw(error_SQL(database_spelling(DCR:DCA, Rext):cannotTranslate)),
  fail.
/*
6.3 Spelling of Relation Names (DEPRECATED)

---- spelling(Rel, Spelled) :-
----

Returns the internal spelling of relation name ~Rel~,

~Spelled~ is available via the dynamic predicate ~storedSpell/3~.

This predicate is deprecated, since the spelling of database objects is
available from the ~secondoCatalogInfo/4~ facts.

*/
spelling(Rel, Spelled) :-
  dm(dbhandling,['\nTry: spelling(',Rel,',',Spelled,').']),
  databaseName(DB),
  ( storedSpell(DB, Rel, Spelled)
    ; (
        secondoCatalogInfo(Rel, ExtRel, _, _),
        internalName2externalName(IntRel,ExtRel),
        assert(storedSpell(DB, Rel, IntRel))
      )
  ),
  !.

spelling(Rel, RelExt) :- !,
  write_list(['\tERROR:\tCannot translate relation \'',Rel,'\'.']),nl,
  throw(error_SQL(database_spelling(Rel, RelExt):cannotTranslate)),
  fail.

/*
6.5 Storing And Loading of Spelling Information

*/
readStoredSpells :-
  retractall(storedSpell(_, _, _)),
  [storedSpells].

writeStoredSpells :-
  open('storedSpells.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredSpell(FD), _),
  close(FD).

writeStoredSpell(Stream) :-
  storedSpell(DB, X, Y),
  write(Stream, storedSpell(DB, X, Y)),
  write(Stream, '.\n').

:-
  dynamic(storedSpell/3),
  dynamic(elem_is/3),
  at_halt(writeStoredSpells),
  readStoredSpells.

/*
7  Cardinalities of Relations

---- card(Rel, Size) :-
----

The cardinality of relation ~Rel~ is ~Size~.

7.1 Get Cardinalities

If ~card~ is called, it tries to look up the cardinality via the
dynamic predicate ~storedCard/3~ (automatically stored).

If this fails, a Secondo query is issued, which determines the
cardinality. This cardinality is then stored in local memory.

*/
card(DCrel, Size) :-
  dm(dbhandling,['\nTry: card(',DCrel,',',Size,').']),
  databaseName(DB),
  storedCard(DB, DCrel, Size),
  !.

card(DCrel, Size) :-
  dm(dbhandling,['\nTry: card(',DCrel,',',Size,').']),
  secondoCatalogInfo(DCrel,_,_,_),
  Query = (count(rel(DCrel, _))),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  secondo(QueryAtom, ResultList),
  ( ResultList = [int, Size]
    -> true
    ;  ( write_list(['\nERROR:\tUnexpected result list format during ',
                     'cardinality query:\n',
                     'Expected \'[int, <intvalue>]\' but got \'',
                     ResultList, '\'.']), nl,
         throw(error_SQL(database_card(DCrel, Size)
                         :unexpectedListType)),
         fail
       )
  ),
  databaseName(DB),
  assert(storedCard(DB, DCrel, Size)),
  !.

card(DCrel, X) :-
  write('\ERROR:\tCardinality for relation \''),write(DCrel), %'
  write('\' cannot be retrieved.'),nl,                        %'
  throw(error_SQL(database_card(DCrel, X):cannotRetrieveCardinality)),
  fail.

/*
7.2 Storing And Loading Cardinalities

*/
readStoredCards :-
  retractall(storedCard(_, _, _)),
  [storedCards].

writeStoredCards :-
  open('storedCards.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredCard(FD), _),
  close(FD).

writeStoredCard(Stream) :-
  storedCard(DB, X, Y),
  write(Stream, storedCard(DB, X, Y)),
  write(Stream, '.\n').

:-
  dynamic(storedCard/3),
  at_halt(writeStoredCards),
  readStoredCards.

/*
8 Managing Indexes and Information on Available Indexes

8.1 Looking Up For Existing Indexes

---- hasIndex(rel(+Rel, _),attr(+Attr, _, _), ?IndexName, ?IndexType)
----

If it exists, the index name for relation ~Rel~ and attribute ~Attr~
is ~IndexName~. The type of the index is ~IndexType~.

~IndexType~ is a logical index type.

If the use of indexes is switched off, the predicate will always fail.

If the predicate fails, this means, that there is no such index.

*/

hasIndex(Rel, Attr, IndexName, IndexType) :-
  ( ( ground(Rel), ground(Attr) )
    -> ( dm(dbhandling,['\nTry: hasIndex(',Rel,',',Attr,',',IndexName,',',
                 IndexType,').']),
         not(optimizerOption(noIndex)),
         hasIndex2(Rel, Attr, IndexName, IndexType)
      )
    ; (
        write_list(['\ERROR:\tUninitialized argument in hasIndex(',Rel,',',
                    Attr,',', IndexName, ',', IndexType, ').']),
        throw(error_SQL(database_hasIndex(Rel, Attr, IndexName, IndexType)
                        :cannotTranslate)),
        fail
      )
  ).


/*
---- hasIndex2(+Rel,+Attr, ?IndexName, ?IndexType)
----
Gets the index name ~IndexName~ for relation ~Rel~ and attribute ~Attr~
via dynamic predicate ~storedIndex/5~.

~Rel~ and ~Attr~ are exressed using relation resp. attribute descriptors
as terms rel(LFRel,\_) and attr(LFAttr,\_,\_).

*/

% simplify attribute descriptor
hasIndex2(rel(Rel, _), attr(_:Attr, _, _), DCindexName, Type) :-
  atomic(Rel),
  atomic(Attr),
  downcase_atom(Rel,DCrel),
  downcase_atom(Attr,DCattr),
  hasIndex2(rel(DCrel, _), attr(DCattr, _, _), DCindexName, Type).

% fail, if absence of that index is known.
hasIndex2(rel(Rel, _), attr(Attr, _, _), _, _) :-
  atomic(Rel),
  atomic(Attr),
  downcase_atom(Rel,DCrel),
  downcase_atom(Attr,DCattr),
  databaseName(DB),
  storedNoIndex(DB, DCrel, DCattr),
  !,
  fail.

% check for known presence of index
hasIndex2(rel(Rel, _), attr(Attr, _, _), DCindex, Type) :-
  atomic(Rel),
  atomic(Attr),
  downcase_atom(Rel,DCrel),
  downcase_atom(Attr,DCattr),
  databaseName(DB),
  storedIndex(DB, DCrel, DCattr, Type, DCindex).

/*
8.2 Storing And Loading About Existing Indexes

Storing and reading of  the two dynamic predicates ~storedIndex/5~ and
~storedNoIndex/3~ in the file ~storedIndexes~.

*/
readStoredIndexes :-
  retractall(storedIndex(_, _, _, _, _)),
  retractall(storedNoIndex(_, _, _)),
  [storedIndexes].

writeStoredIndexes :-
  open('storedIndexes.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredIndex(FD), _),
  findall(_, writeStoredNoIndex(FD), _),
  close(FD).

writeStoredIndex(Stream) :-
  storedIndex(DB, U, V, W, X),
  write(Stream, storedIndex(DB, U, V, W, X)),
  write(Stream, '.\n').

writeStoredNoIndex(Stream) :-
  storedNoIndex(DB, U, V),
  write(Stream, storedNoIndex(DB, U, V)),
  write(Stream, '.\n').

:-
  dynamic(storedIndex/5),
  dynamic(storedNoIndex/3),
  at_halt(writeStoredIndexes),
  readStoredIndexes.


/*
8.3 Index Types

8.3.1 Physical Index Types

By the term ~physical index type~, we refer to index structures, that are
directly implemented by some algebra linked to the Secondo kernel.

The following facts define Secondo object types, that are used for indices,
and give a pattern that matches all according type expressions for this
physical index type:

----indexType(+PhysIndesType, +TypeExpressionPattern)
----

While ~PhysIndesType~ must be a ground term, ~TypeExpressionPattern~ may contain
variables, if this is required to matcj the index type expression.

If you want to add a physical index type, you just need to add an according fact
here. After this, you may define logical index types using the added physical
index type and define optimization/translation rules using the logical index
type(s).

You may also need to define syntax (predicates secondoOp/3 in file opsyntx.pl)
and translation rules (predicate plan\_to\_atom/2 in file optimizer.pl) for
special index operators.

*/

indexType( btree,  [[btree|_]]  ).
indexType( hash,   [[hash|_]]   ).
indexType( rtree,  [[rtree|_]]  ).
indexType( rtree3, [[rtree3|_]] ).
indexType( rtree4, [[rtree4|_]] ).
indexType( rtree8, [[rtree8|_]] ).
indexType( mtree,  [mtree]      ).
indexType( xtree,  [xtree]      ).


/*
----extractPhysicalIndexType(+CatalogObject, ?IndexName, ?PhysicalIndexType)
----

Checks, if the catalog entry ~CatalogObject~ represents a physical index type
~PhysicalIndexType~ and returns its name ~IndexName~ (in external spelling).

If ~CatalogObject~ has not a pysical index type (as defined by indexType/1),
the predicate fails.

*/

extractPhysicalIndexType(CatalogObject, IndexName, PhysicalIndexType) :-
  CatalogObject =   ['OBJECT', IndexName , _ , TypeList ],
  indexType(PhysicalIndexType, TypeList),
  !.

/*
8.3.2 Logical Index Types

Within the optimization rules, the optimizer only refers to ~logical index
types~ rather than to ~physical~ ones (which are directly supported by Secondo).

Logical index types are mapped to physical index types. This mapping can be
native, or complex, using some kind of transformations to generate the key
values from the indexed attribute, e.g. using projections. Therefore, we need
some definitions to allow the optimizer maintaning the index.

We define ~logical index types~ using facts

----
     logicalIndexType(LogicalIndexTypeCode, LogicalTypeExpr, PhysicalIndexType,
                      SupportedAttributeTypeList,
                      CreateIndexPattern,
                      InsertIntoIndexPattern,
                      DeleteFromIndexPattern,
                      UpdateIndexPattern)

----

~LogicalIndexTypeCode~ is a substring used in the index object's name to mark
the object to represent a certain logical index, that is referenced by type
~LogicalTypeExpr~ within the optimizer. Indexes of that logical type are
implemented using the physical index type ~PhysicalIndexType~,
which must have been registered as ~indexType(PhysicalIndexType,TypeListPattern)~
above.

~SupportedAttributeTypeList~ provides a list all of attribute type expressions,
that are supported by the defined logical index type.

~CreateIndexPattern~, ~InsertIntoIndexPattern~, ~DeleteFromIndexPattern~, and
~UpdateIndexPattern~ define patterns, that are used to create executable
Secondo queries performing the creation of a new index, and three types of
update operations, namely insertions, deletions, and updates.

Patterns are given as lists, that will be concatenated to the creation query.
You can refer to the indexed relation by a listelement ~'\_\_rel\_\_'~ and to
the indexed attribute by ~'\_\_attr\_\_'~. When creating the index, these
elements will be replaced by the actual identifiers.

Wherever a field is not applicable (e.g. because updates are not supported by
the index), ~undefined~ is used within the definition.

Therefore, using the ~LogicalIndexTypeCode~ 'undefined' is prohibited.

Predicate ~showIndexTypes~ will list information on all available logical index
types.

*/

:-assert(helpLine(showIndexTypes,0,[],'List available index types.')).

showSingleIndexType :-
  logicalIndexType(LogicalIndexTypeCode, LogicalTypeExpr, _,
                   SupportedAttributeTypeList,_,_,_,_),
  format('~2|~p~34|~p~44|~p~79|~n',
         [LogicalTypeExpr,LogicalIndexTypeCode,SupportedAttributeTypeList]).

showIndexTypes :-
  write('\nAvailable index types:\n'),
  format('~2|~p~34|~p~44|~p~79|~n',
         ['Index Type','Type Code','Supported Key Types']),
  write_list(['  --------------------------------------------------',
              '----------------------------']),nl,
  findall(_,showSingleIndexType,_),
  nl,!.

% standard indexes - just wrapping physical index types
logicalIndexType(btree, btree, btree,
    [int, real, string, text, point],
    ['__REL__', ' createbtree[', '__ATTR__', ']'],
    undefined,
    undefined,
    undefined).

logicalIndexType(hash, hash, hash,
    [int, real, string, text],
    ['__REL__', ' createhash[', '__ATTR__', ']'],
    undefined,
    undefined,
    undefined).

logicalIndexType(rtree, rtree, rtree,
    [point, points, line, sline, region, rect],
    ['__REL__', ' creatertree[', '__ATTR__', ']'],
    undefined,
    undefined,
    undefined).

logicalIndexType(rtree3, rtree3, rtree3,
    [rect3],
    ['__REL__', ' creatertree3[', '__ATTR__', ']'],
    undefined,
    undefined,
    undefined).

logicalIndexType(rtree4, rtree4, rtree4,
    [rect4],
    ['__REL__', ' creatertree4[', '__ATTR__', ']'],
    undefined,
    undefined,
    undefined).

logicalIndexType(rtree8, rtree8, rtree8,
    [rect8],
    ['__REL__', ' creatertree8[', '__ATTR__', ']'],
    undefined,
    undefined,
    undefined).

logicalIndexType(mtree, mtree, mtree,
    [int, real],
    ['__REL__', ' createmtree[', '__ATTR__', ']'],
    undefined,
    undefined,
    undefined).

logicalIndexType(xtree, xtree, xtree,
    [point, points, line, sline, rect],
    ['__REL__', ' creatextree[', '__ATTR__', ']'],
    undefined,
    undefined,
    undefined).

% keyword indexes for text and string attributes
logicalIndexType(keywdb, keyword(btree), btree,
    [string, text],
    [ '__REL__', ' feed addid extendstream[KeyWd: .',
      '__ATTR__', ' keywords] createbtree[ KeyWd ]'],
    undefined,
    undefined,
    undefined).

logicalIndexType(keywdh, keyword(hash), hash,
    [string, text],
    [ '__REL__', ' feed addid extendstream[KeyWd: .',
      '__ATTR__', ' keywords] createhash[ KeyWd ]'],
    undefined,
    undefined,
    undefined).


% index types on spatio-temporal data
logicalIndexType(tmpobj, temporal(rtree, object), rtree,
    [upoint,uregion,mpoint,mregion],
    [ '__REL__', ' feed addid extend[ p: point2d( deftime(.',
      '__ATTR__', ') ) ] creatertree[ p ]' ],
    undefined,
    undefined,
    undefined).

logicalIndexType(tmpuni, temporal(rtree, unit), rtree,
    [mpoint,mregion],
    [ '__REL__', ' feed projectextend[ ', '__ATTR__',
      ' ; TID: tupleid(.)] projectextendstream[TID; MBR: units(.',
      '__ATTR__', ') use[fun(U: upoint) point2d(deftime(U)) ]] ',
      'sortby[MBR asc] bulkloadrtree[MBR]' ],
    undefined,
    undefined,
    undefined).

logicalIndexType(sptobj, spatial(rtree, object), rtree,
    [upoint,uregion,mpoint,mregion],
    [ '__REL__', ' feed addid extend[ t: trajectory(.',
      '__ATTR__', ') ] creatertree[ t ]' ],
    undefined,
    undefined,
    undefined).

logicalIndexType(sptuni, spatial(rtree, unit), rtree,
    [mpoint,mregion],
    [ '__REL__', ' feed projectextend[ ', '__ATTR__',
      ' ; TID: tupleid(.)] projectextendstream[TID; MBR: units(.','__ATTR__',
      ') use[fun(U: upoint) bbox2d(U) ]] sortby[MBR asc] bulkloadrtree[MBR]' ],
    undefined,
    undefined,
    undefined).
%%% XRIS: CREATERTREE  ?????
logicalIndexType(sptmpobj, spatiotemporal(rtree3, object), rtree3,
    [upoint,uregion,mpoint,mregion],
    [ '__REL__', ' feed addid extend[ b: box3d( bbox( trajectory(.',
      '__ATTR__', ') ), deftime( .',
      '__ATTR__', ') ) ] creatertree[ b ]' ],
    undefined,
    undefined,
    undefined).

logicalIndexType(sptmpuni, spatiotemporal(rtree3, unit), rtree3,
    [mpoint,mregion],
    [ '__REL__', ' feed projectextend[ ',
      '__ATTR__', ' ; TID: tupleid(.)] projectextendstream[TID; MBR: units(.',
      '__ATTR__', ') use[fun(U: upoint) point2d(deftime(U)) ]] ',
      'sortby[MBR asc] bulkloadrtree[MBR]' ],
    undefined,
    undefined,
    undefined).



/*
8.4 Creating, Deleting, Updating Indexes

---- replaceInPattern(+Rel,+Attr,+ListIn,-ListOut)
----

Replaces all occurences of '__ATTR__' by Attr and of '__REL__' by Rel.
The predicate is used to instantiate a pattern defined in logicalIndexType/8.

*/
replaceInPattern(_, _, [], []).
replaceInPattern(Rel, Attr, ['__ATTR__'|ListIn], [Attr|ListOut]) :-
  replaceInPattern(Rel, Attr, ListIn, ListOut), !.
replaceInPattern(Rel, Attr, ['__REL__'|ListIn], [Rel|ListOut]) :-
  replaceInPattern(Rel, Attr, ListIn, ListOut), !.
replaceInPattern(Rel, Attr, [Elem|ListIn], [Elem|ListOut]) :-
  replaceInPattern(Rel, Attr, ListIn, ListOut), !.


/*
---- getCreateIndexExpression(+Rel, +Attr,
                              +LogicalIndexType,
                              +CreateSmall,
                              -IndexName,
                              -Query)
----
Return the right-hand expression ~Query~ of an executable Secondo query which
will create an index structure named ~IndexName~ (external spelling) of
logical index type ~LogicalIndexType~ for attribute ~Attr~ in Relation ~Rel~.

The canonical name ~IndexName~ is generated in accordance with the naming
conventions and will be returned.

If ~CreateSmall~ = yes holds, a ~\_small~-index is created, i.e. both the
relation name and the index name are modified by appending '\_small'.

Uses the ~CreateIndexPattern~ defined within ~logicalIndexType/8~.

~Rel~ and ~Attr~ must respect external spelling conventions (i.e. must be
spelled as within the Secondo Catalog).

*/

getCreateIndexExpression(Rel, Attr,
                         LogicalIndexType,
                         CreateSmall,
                         IndexName, Query) :-
  ( CreateSmall = yes
    -> Small = '_small'
    ;  Small = ''
  ),
  logicalIndexType(LogicalIndexTypeCode, LogicalIndexType, _, _,
                   CreateIndexPattern, _, _, _),
  concat_atom([Rel, Small] ,'', RelName),
  replaceInPattern(RelName,Attr,CreateIndexPattern,
                   CreatePatternInstantiated),
  concat_atom(CreatePatternInstantiated, '', Query),
  concat_atom([Rel,Attr,LogicalIndexTypeCode],'_', IndexName2),
  concat_atom([IndexName2,Small], '', IndexName),
  !.


/*
----
     databaseObjectExists(+ObjectName)
     databaseObjectExistsDC(+ObjectName)
----

Checks, whether the given ~ObjectName~ (in external spelling) is already used
within the current database. The predicate fails, if no such object exists

Variant ~databaseObjectExists~ checks, if there exists a object with that
name, taking into account for spelling variants. Here, ~ObjectName~ may be
either in down-case or external spelling.

*/

databaseObjectExists(ObjectName) :-
  secondoCatalogInfo(_, ObjectName, _, _),
  !.

databaseObjectExistsDC(ObjectName) :-
  dcName2externalName(DCobjectName,ObjectName),
  secondoCatalogInfo(DCobjectName, _, _, _),
  !.

/*
---- keyAttributeTypeMatchesIndexType(+LFRel,+LFAttr,?LogicalIndexType)
----

Checks, whether attribute ~Attr~ of relation ~Rel~ matches a key type for
the logical index type ~LogicalIndexType~. If called with unbound
~LogicalIndexType~, all available logical index types will be enumerated.

~LFRel~ and ~LFAttr~ must respect internal naming conventions.

*/

keyAttributeTypeMatchesIndexType(LFRel, LFAttr, LogicalIndexType) :-
  databaseName(DB),
  storedAttrSize(DB, LFRel, LFAttr, Type, _, _, _),
  !,
  logicalIndexType(_, LogicalIndexType, _, TypeList, _,_,_,_),
  member(Type,TypeList).


/*
----createIndex(+LFRel, +LFAttr, +LogicalIndexType, +CreateSmall, -IndexName)
----

Creates an index for ~LFRel~:~LFAttr~ of type ~LogicalIndexType~.
If ~CreateSmall~ = ~yes~ holds, a '\_small'-index is created (otherwise
a normal object). I.e. ~IndexName~ will be extended with '~\_small~' and the
index will be built using relation ~rel\_small~ instead of ~rel~.

The name of the index object is unified with ~IndexName~.

Dynamic predicate ~storedIndex/5~ will be updated.

The predicate will succeed,

  * if the index has been created successfully.

The predicate fails and throws an exception,

  * if no database is currently open

  * if the relation does not have that attribute

  * if there already exists an according index (no index will be created).

  * if the automatically determined indexname is already used.

  * if the relation ~Rel~ or attribute ~attr~ does not exist;

  * if the choosen index type does not match the attribute;

  * if the ~LogicalIndexType~ is unknown;


~LFRel~ and ~LFAttr~ are in ~down cased spelling~,
~LogicalIndexType~ is a term,
~CreateSmall~ should be either ~yes~ or ~no~.
The resulting ~IndexName~ is in ~external spelling~.

*/

createIndex(LFRel, LFAttr, LogicalIndexType, CreateSmall, IndexName) :-
  dm(dbhandling,['\nTry: createIndex(',LFRel,',',LFAttr,',',
                 LogicalIndexType,',',CreateSmall,',',IndexName,').']),
  % Check if a database is opened
  ( databaseName(DB)
    -> true
    ; ( write('ERROR:\tNo database open. Cannot create index.'),nl, !,
        throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
           CreateSmall, IndexName):noDatabaseOpen)),
        fail
      )
  ),
  % Prepare small-suffix
  ( CreateSmall = small
    -> Small = '_small'
    ;  Small = ''
  ),
  % Get names in external spelling
  ( ( dcName2externalName(LFRel, Rel),
      dcName2externalName(LFRel:LFAttr, Attr)
    )
    -> true
    ;  ( write_list(['ERROR:\tCannot create index on unknown relation or ',
                     'attribute \'',LFRel,':',LFAttr,'\'.']),nl, !,
         throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
               CreateSmall, IndexName):unknownRelationOrAttribute)),
         fail
       )
  ),
  % Check, whether the relation has an according attribute
  ( storedRel(DB, LFRel, AttrList)
    -> true
    ;  ( write('ERROR:\tCannot create index. Relation \''),
         write(LFRel),write('\' unknown.'),nl, !,
         throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
            CreateSmall, IndexName):unknownRelation)),
         fail
       )
  ),
  ( member(LFAttr,AttrList)
    -> true
    ;  ( write('ERROR:\tCannot create index because relation \''),
         write(LFRel),write('\' does not have an attribute \''),
         write(LFAttr), write('\'. Available attributes are:'),
         write(AttrList), nl, !,
         throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
            CreateSmall, IndexName):attributeNameMismatch)),
         fail
       )
  ),
  % Check whether an index of that type already exists for that attribute
  ( ( storedIndex(DB,LFRel,LFAttr,LogicalIndexType,DColdIndexName),
      dcName2externalName(DColdIndexName,ExtOldIndexName),
      CreateSmall \= yes                         % do not check for small index
    )
    -> ( write('ERROR:\tThere already exists an index of type '),
         write(LogicalIndexType), write(' for '),
         write(LFRel),write(':'),write(LFAttr),
         write(' under the name \''),write(ExtOldIndexName),write('\'.'),nl, !,
         throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
            CreateSmall, IndexName):indexAlreadyExists)),
         fail
       )
    ;  true
  ),
  % Check, whether the index type matches the attribute type
  ( ( getCreateIndexExpression(Rel, Attr, LogicalIndexType, CreateSmall,
                               IndexName, Query),
      keyAttributeTypeMatchesIndexType(LFRel, LFAttr, LogicalIndexType)
    )
    -> true
    ;  ( storedAttrSize(DB, LFRel, LFAttr, Type, _, _, _),
         write('ERROR:\tIndex Type '),write(LogicalIndexType),
         write(' unknown or not matching the type of '),
         write(LFRel),write(':'),write(LFAttr),
         write(' which is \''),write(Type),write('\'.'), nl,
         findall(X,
                 keyAttributeTypeMatchesIndexType(LFRel,LFAttr,X),
                 AvailIndTypes),
         write('\tYou may create the following index types for this key:\n\t'),
         write(AvailIndTypes),nl, !,
         throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
             CreateSmall, IndexName):unknownIndexType)),
         fail
       )
  ),
  % Check, whether a DB-object with the proper name already exists
  ( (   databaseObjectExists(IndexName)
      ; databaseObjectExistsDC(IndexName)
    )
    -> ( write_list(['ERROR:\tThe index object of type ',LogicalIndexType,
         ' must be named \'',IndexName,'\',\n--->\tbut there already exists a',
         ' database object with that name!']),
         nl, !,
         throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
             CreateSmall, IndexName):fileAlreadyExists)),
         fail
       )
    ;  true
  ),
  % Check whether the according base relation exists
  concat_atom([Rel,Small],'',RelName),  % concat _small suffix, if specified
  ( databaseObjectExists(RelName)
    -> true
    ;  ( write('ERROR:\tThe base relation \''), write(RelName),
         write('\' does not exist. Cannot create the index.'),nl,!,
         throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
            CreateSmall, IndexName):lostObject)),
         fail
       )
  ),
  % All preconditions checked and OK. Build the index...
  write('NOTE:\tTrying to create the requested index...'),nl,
  concat_atom(['derive ', IndexName, ' = ', Query],'',CreationQuery), !,
  write('CreationQuery = '),write(CreationQuery),nl,
  ( secondo_direct(CreationQuery)
    -> true
    ;  ( write('ERROR:\tCould not create index. The creation query failed.'),
         nl,!,
         throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
             CreateSmall, IndexName):creationQueryFailed)),
         fail
       )
  ),
  dcName2externalName(DCindexName,IndexName),
  ( (CreateSmall \= yes)
    -> ( retractall(storedNoIndex(DB,LFRel,LFAttr)),
         assert(storedIndex(DB,LFRel,LFAttr,LogicalIndexType,DCindexName))
       )
    ;  true                                      % store no info in small index
  ),
  write('\tIndex \''),write(IndexName),write('\' has been created.'),nl,
  !.

createIndex(LFRel, LFAttr, LogicalIndexType, CreateSmall, IndexName) :-
  write('ERROR:\tSome error occured when trying to create an index:'),nl,!,
  throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
         CreateSmall, IndexName):unknownError)),
  fail.


/*
---- createIndex(+DCrel,+DCattr,+LogicalIndexType)
----

This is the user-level predicate to create an index of a given ~LogicalIndexType~
for a given Relation ~DCrel~ and attribute ~DCattr~.
After creating the index, updateCatalog is called.

Relation and attribute are passed in downcased spelling.

*/

:- assert(helpLine(createIndex,3,
    [[+,'DCRel','The relation the index is for.'],
     [+,'DCAttr','The key attribute.'],
     [?,'LogicalIndexType','The logical index type.']],
    'Create an index of a specified type over a given attribute for a relation.')).

createIndex(DCrel,DCattr,LogicalIndexType) :-
  dm(dbhandling,['\nTry: createIndex(',DCrel,',',DCattr,',',LogicalIndexType,
                 ').']),
  ( isDatabaseOpen
    -> true
    ;  ( throw(error_SQL(database_createIndex(DCrel,DCattr,LogicalIndexType)
                                      :noDatabaseOpen)),
         fail
       )
  ),
  ( ground(DCrel)
    -> true
    ;  ( write_list(['\nERROR:\tCannot create index.\n',
                     '--->\tYou are required to pass a relation name.']), nl,
         !,
         throw(error_SQL(database_createIndex(DCrel,DCattr,LogicalIndexType)
                                      :unspecifiedRelation)),
         fail
       )
  ),
  ( ground(DCattr)
    -> true
    ;  ( write_list(['\nERROR:\tCannot create index.\n',
                     '--->\tYou are required to pass an attribute name.']), nl,
         !,
         throw(error_SQL(database_createIndex(DCrel,DCattr,LogicalIndexType)
                                      :unspecifiedAttribute)),
         fail
       )
  ),
  ( ground(LogicalIndexType)
    -> true
    ;  ( databaseName(DB),
         storedAttrSize(DB,DCrel,DCattr,AttrType,_,_,_),
         findall( PossIndType,
                  ( logicalIndexType(_, PossIndType, _,
                                     SupportedAttributeTypeList,_,_,_,_),
                    member(AttrType,SupportedAttributeTypeList)
                  ),
                  AvailTypes),
         dcName2externalName(DCrel,ExtRel),
         dcName2externalName(DCrel:DCattr,ExtAttr),
         write_list(['\nERROR:\tCannot create index.\n',
                     '--->\tYou are required to pass a logical index type.\n',
                     '--->\tAvailable logical index types for \'',ExtRel,
                     ':',ExtAttr,'\' are:\n','--->\t',AvailTypes]), nl,
         !,
          throw(error_SQL(database_createIndex(DCrel,DCattr,LogicalIndexType)
                                      :unspecifiedIndexType)),
         fail
       )
  ),
  createIndex(DCrel,DCattr,LogicalIndexType, no, _), !,
  updateCatalog.

createIndex(DCrel,DCattr,LogicalIndexType) :-
  write_list(['\nERROR:\tCannot create index.\n',
                     '--->\tPerhaps the given relation or attribute does ',
                     'not exist?.']), nl,
  !,
  throw(error_SQL(database_createIndex(DCrel,DCattr,LogicalIndexType)
                                      :unknownError)),
  fail.

/*
----
     splitIndexObjectName(+ObjectName,-RelName,-AttrName,
                          -IndexCode,-Disambiguator,-IsSmall)
----

Split a Secondo object name ~ObjectName~ into its compartments RelationName ~RelName~,
AttributeName ~AttrName~, the logical index code ~IndexCode~, a disambiguating
appendix ~Disambiguator~ and a flags ~IsSmall~ (yes/no), indicating whether this object is
a small object.

If some compartment is not relevant or present, ~undefined~ is returned for the according part.

All names will have external spelling (i.e. as in the Secondo catalog)!

*/
splitIndexObjectName(ObjectName, RelName,AttrName,
                   IndexCode, Disambiguator, IsSmall) :-
  ( sub_atom(ObjectName, _, _, 0, '_small')
    -> (
         IsSmall = yes,
         sub_atom(ObjectName, 0, _, 6, ObjectName2)
       )
    ;  (
         IsSmall = no,
         ObjectName2 = ObjectName
       )
  ),
  concat_atom(FragList, '_', ObjectName2),
  ( ( FragList = [RelName,AttrName],IndexCode = undefined,
                                    Disambiguator = undefined)
   ;( FragList = [RelName,AttrName, IndexCode],Disambiguator = undefined )
   ;( FragList = [RelName,AttrName, IndexCode,Disambiguator] )
  ),
  !.


/*
---- createIndexName(+RelName,+AttrName,
                     +LogicalIndexType,+Disambiguator,+IsSmall,?ObjectName)
----

Create the canonical indexName ~ObjectName~ for an index of type
~LogicalIndexType~ on ~RelName~. If ~IsSmall~ = yes, the name for a
\_small-index is returened. The ~Disambiguator~ will be used, if
~Disambiguator~ \\= undefined holds.

All names will have external spelling (i.e. as in the Secondo catalog)!

*/

createIndexName(ExtRelName, ExtAttrName, LogicalIndexType,
                ExtDisambiguator, IsSmall, ExtObjectName) :-
  logicalIndexType(TypeCode, LogicalIndexType, _,_,_,_,_,_),
  ( IsSmall = yes
    -> SmallSuffix = '_small'
    ;  SmallSuffix = ''
  ),
  ( ExtDisambiguator = undefined
    -> DisambiguatorSuffix = ''
    ;  concat_atom(['_',ExtDisambiguator],'',DisambiguatorSuffix)
  ),
  concat_atom([ExtRelName,ExtAttrName,TypeCode],'_',ObjectName2),
  concat_atom([ObjectName2,DisambiguatorSuffix,SmallSuffix],'',ExtObjectName),
  !.

/*
----createSmallIndex(+LFRel,+LFAttr,+LogicalIndexType)
----

Creates a '\_small'-index for ~DCRel~:~DCAttr~ having type ~LogicalIndexType~.

~DCRel~ and ~DCAttr~ are atomic terms respecting down-cased spelling rules
~LogicalIndexType~ is a term.

*/

createSmallIndex(DCRel,DCAttr,LogicalIndexType) :-
  dm(dbhandling,['\nTry: createSmallIndex(',DCRel,',',DCAttr,',',
                 LogicalIndexType,').']),
  (optimizerOption(entropy)
   -> catch( createIndex(DCRel, DCAttr, LogicalIndexType, yes, _),
            error_SQL(database_createIndex(DCRel, DCAttr,
                    LogicalIndexType,IndexName):Reason),
            ( member(Reason,[indexAlreadyExists,fileAlreadyExists])
              -> ( write_list(['\nINFO:\tSmall index already exists.']), nl )
              ;  throw(error_SQL(database_createIndex(DCRel, DCAttr,
                    LogicalIndexType,IndexName):Reason))
            )
          )
   ;  true
  ),
  !.


/*
----
dropIndex(+IndexName)
dropIndex(+DCRel,+DCAttr,+LogicalIndexType)
----

These predicates will delete the referenced index and update the optimizer's
index information (storedIndex/5, storedNoIndex/3).

If a '\_small'-index exists, it will be removed.

The predicate fails, if no such index (and no '\_small'-index) exists.
Otherwise, it will succeed.

~IndexName~ must obey the external spelling schema.

~DCRel~ and ~DCAttr~ must be conform with the down-cased spelling convention.

*/

:- assert(helpLine(dropIndex,3,
    [[+,'DCRel','The relation the index is for.'],
     [+,'DCAttr','The key attribute.'],
     [+,'LogicalIndexType','The logical index type.']],
    'Drop a specified index from the current DB.')).


dropIndex(ExtIndexName) :-
  dm(dbhandling,['\nTry: dropIndex(',ExtIndexName,').']),
  % Check if a database is opened
  ( databaseName(DB)
    -> true
    ; ( write('ERROR:\tNo database open. Cannot drop index.'),nl, !,
        throw(error_SQL(database_dropIndex(ExtIndexName):noDatabaseOpen)),
        fail
      )
  ),
  splitIndexObjectName(ExtIndexName,RelName,AttrName,_,_,IsSmall),
  % check if index is not a small-index
  ( IsSmall = yes
    -> ( write('ERROR:\tCannot drop \'_small\'- index manually.'),nl,!,
         fail
       )
    ; true
  ),
  % check if index exists
  ( databaseObjectExists(ExtIndexName)
    -> true
    ;  ( write('ERROR:\tThe index \''), write(ExtIndexName),
         write('\' does not exist. Cannot drop the index.'),nl,!,
         throw(error_SQL(dropIndex(ExtIndexName):lostObject)),
         fail
       )
  ),
  % remove the index object
  concat_atom(['delete ', ExtIndexName],'',DeleteIndexAtom),
  secondo_direct(DeleteIndexAtom),
  % update storedIndex/5
  dcName2externalName(DCRel,RelName),            % get DCReal
  % dcName2externalName(DCRel:DCAttr, AttrName), % original code
  dcName2externalName(DCAttr, AttrName),         % modified code
  dcName2externalName(DCindexName,ExtIndexName),
  retractall(storedIndex(DB,DCRel,DCAttr,_,DCindexName)),
  % update storedNoIndex/3
  ( storedIndex(DB,DCRel,DCAttr, _, _)
    -> true
    ;  assert(storedNoIndex(DB,DCRel,DCAttr))
  ),
  % possibly remove small-index
  concat_atom([ExtIndexName,'_small'],'',ExtSmallIndexName),
  ( databaseObjectExists(ExtSmallIndexName)
    -> ( write('NOTE:\tAlso dropping the _small-index \''),         %'
         write(ExtSmallIndexName), write('\'.'),nl,                 %'
         concat_atom(['delete ', ExtSmallIndexName],'',DeleteSmallIndexAtom),
         secondo_direct(DeleteSmallIndexAtom)
       )
    ;  true
  ),
  write('NOTE:\tIndex \''), write(ExtIndexName), write('\' has been dropped.'),
  nl,
  !.

dropIndex(ExtIndexName) :-
  write('ERROR:\tSome error occured while trying to drop an index:'),nl,!,
  throw(error_SQL(database_dropIndex(ExtIndexName):unknownError)),
  fail.

dropIndex(DCRel,DCAttr,LogicalIndexType) :-
  dm(dbhandling,['\nTry: dropIndex(',DCRel,',',DCAttr,',',
                 LogicalIndexType,').']),
  % create index name
  ( not( (ground(DCRel), ground(DCAttr), ground(LogicalIndexType),
          atomic(DCRel), atomic(DCAttr) )
       )
    -> ( write('ERROR:\tYou need to specify all 3 arguments of dropIndex/3.'),
         nl,
         throw(error_SQL(dropIndex(DCRel,DCAttr,LogicalIndexType)
                 :unsufficientParameters)),
         fail
       )
    ;  ( true )
  ),
  dcName2externalName(DCRel,ExtRelName),
  dcName2externalName(DCRel:DCAttr, ExtAttrName),
  createIndexName(ExtRelName,ExtAttrName,LogicalIndexType,undefined,no,
                  ExtIndexName),!,
  dropIndex(ExtIndexName),!.

dropIndex(LFRel,LFAttr,LogicalIndexType) :-
  write('ERROR:\tSome error occured while trying to drop an index:'),nl,!,
  throw(error_SQL(database_dropIndex(LFRel,LFAttr,LogicalIndexType)
                 :unknownError)),
  fail.

/*
---- showIndexes
----

This predicate shows a list of all registered indexes present within the currently
opened database.

*/
:-assert(helpLine(helpMe,0,[],
         'List information on available indexes within the current DB.')).


showIndexes :-
  (databaseName(DB)
   -> ( write_list(['\n\nOverview on all Indexes available in database \'',
                    DB,'\':']),nl,
        format('~p~35|~p~69|~p~n',
               ['Index Name', 'Relation:Attribute', 'Logical Index Type']),
        write_list(['---------------------------------------------------',
                    '------------------------------------']),
        nl,
        findall(_,
                ( storedIndex(DB,DCrel,DCattr,IndexType,DCindexName),
                  dcName2externalName(DCrel,ExtRelName),
                  dcName2externalName(DCrel:DCattr,ExtAttrName),
                  dcName2externalName(DCindexName,ExtIndexName),
                  format('~p~35|~p~p~p~69|~p~n',
                         [ExtIndexName, ExtRelName, ':',ExtAttrName,IndexType])
                ),
                _)
      )
    ; (write_list(['\nWARNING:\tCannot list indexes. No database open.']), nl)
  ),
  nl, nl, !.

/*
9 Update Indexes And Relations

The next two predicates provide an update about known indexes and
an update for informations about relations, which are stored in local
memory.

9.1 Update Relations, Update Databases

----
updateRel(+DCRel)
updateDB(+DCDB)
----

All information stored in local memory about relation ~Rel~ will
be deleted. Catalog information and some basic statsitics (relation schema,
attribute and tuple sizes, cardinalities, availability of indexes) will be
updated immediately.

Further meta data (selectivities, PETs) will be purged and retrieved on
explicit request only (e.g. when that information is needed during optimization).

The relation is written according to ~down cased spelling~, so that the predicate
can be used on the user-level.

~updateDB~ will retract all information on the specified database
(which may not be opened), but ~not~ delete any samples, small objects or indexes.

*/

:- assert(helpLine(updateRel,1,
    [[+,'DCRelName','The name of the relation to handle in DC-spelling.']],
    'Retract and update all metadata and samples for a given relation.')).

:- assert(helpLine(updateDB,1,
    [[+,'DCDBName','The name of the database to handle.']],
    'Retract all metadata for a given database (which may not be opened).')).

% Some auxiliary predicates to retract selectivity and PET information facts
getRelAttrName(Rel, Arg) :-
  Arg = Rel:_.

getRelAttrName(Rel, Term) :-
  functor(Term, _, 1),
  arg(1, Term, Arg1),
  getRelAttrName(Rel, Arg1).

getRelAttrName(Rel, Term) :-
  functor(Term, _, 2),
  arg(1, Term, Arg1),
  getRelAttrName(Rel, Arg1).

getRelAttrName(Rel, Term) :-
  functor(Term, _, 2),
  arg(2, Term, Arg2),
  getRelAttrName(Rel, Arg2).

retractSels(Rel) :-
  databaseName(DB),
  storedSel(DB, Term, _),
  arg(1, Term, Arg1),
  getRelAttrName(Rel, Arg1),
  retract(storedSel(DB, Term, _)),
  retractSels(Rel).

retractSels(Rel) :-
  databaseName(DB),
  storedSel(DB, Term, _),
  arg(2, Term, Arg2),
  getRelAttrName(Rel, Arg2),
  retract(storedSel(DB, Term, _)),
  retractSels(Rel).

retractSels(_).

retractPETs(Rel) :-
  databaseName(DB),
  storedPET(DB, Term, _, _),
  arg(1, Term, Arg1),
  getRelAttrName(Rel, Arg1),
  retract(storedPET(DB, Term, _, _)),
  retractPETs(Rel).

retractPETs(Rel) :-
  databaseName(DB),
  storedPET(DB, Term, _, _),
  arg(2, Term, Arg2),
  getRelAttrName(Rel, Arg2),
  retract(storedPET(DB, Term, _, _)),
  retractPETs(Rel).

retractPETs(_).

retractStoredInformation(DCrel) :-
  dm(dbhandling,['\nTry: retractStoredInformation(',DCrel,').']),
  databaseName(DB),
  getSampleSname(DCrel, SampleS),
  getSampleJname(DCrel, SampleJ),
  getSmallName(DCrel,Small),
  retractSels(DCrel),
  retractPETs(DCrel),
  retractall(storedOrderings(DB, DCrel, _)),
  retractall(storedCard(DB, DCrel, _)),
  retractall(storedCard(DB, SampleS, _)),
  retractall(storedCard(DB, SampleJ, _)),
  retractall(storedCard(DB, Small, _)),
  retractall(storedAttrSize(DB, DCrel, _, _, _, _, _)),
  retractall(storedTupleSize(DB, DCrel, _)),
  retractall(storedSpell(DB, DCrel, _)),
  retractall(storedSpell(DB, DCrel:_, _)),
  retractall(storedSpell(DB, SampleS, _)),
  retractall(storedSpell(DB, SampleJ, _)),
  retractall(storedSpell(DB, Small, _)),
  retractall(storedRel(DB, DCrel, _)),
  retractall(storedIndex(DB, DCrel, _, _, _)),
  retractall(storedNoIndex(DB, DCrel, _)),
  write_list(['\nINFO:\tRetracted all information on relation \'', DCrel,
              '\' and ', 'all according small and sample objects.']),nl,
  !.

updateRel(Rel) :-
  ( not( (ground(Rel), atomic(Rel)) )
    -> ( write_list(['\nERROR:\updateRel/1 requires a concrete relation name ',
                     'as argument, but gets \'', Rel, '\'.']),
         fail
       )
    ;  ( true )
  ),
  dcName2externalName(DCRel,Rel),
  dcName2externalName(DCRel,ExtRel),
  write_list(['\nINFO:\updateRel(', Rel, ') retracts all information on \'',
              ExtRel, '\'...']),
%  retractStoredInformation(DCRel), % original code
  handleLostRelation(DCRel),        % new code - will also remove samples etc.
  write_list(['\nINFO:\updateRel(', Rel, ') re-collects basic information ',
              'on \'', ExtRel,'\'...']),
  updateCatalog,
  write_list(['\nINFO:\tUpdated all information on relation \'',ExtRel,'\'.']),
  nl,
  !.

updateDB(DB1) :-
  atomic(DB1),
  dcName2externalName(DB,DB1),
  ( ( ( not(isDatabaseOpen) ; ( databaseName(DBopen),
                                dcName2externalName(DB2,DBopen),
                                DB \= DB2
                              )
      )
    )
    *->( write_list(['\nWARNING:\tupdateDB(', DB, ') retracts all metadata on ',
                     'the given database, but will not delete any derived ',
                     'objects!']),
         retractall(storedOrderings(DB, _, _)),
         retractall(storedCard(DB, _, _)),
         retractall(storedAttrSize(DB, _, _, _, _, _, _)),
         retractall(storedTupleSize(DB, _, _)),
         retractall(storedSpell(DB, _, _)),
         retractall(storedRel(DB, _, _)),
         retractall(storedIndex(DB, _, _, _, _)),
         retractall(storedNoIndex(DB, _, _)),
         retractall(storedPET(DB, _, _, _)),
         retractall(storedSel(DB, _, _)),
         write_list(['\nINFO: All information on database \'', DB, '\' has ',
                     'been retracted.'])
       )
    ;  ( write_list(['\nERROR:\tYou may not use updateDB/1 on the currently ',
                      'opened database!']), nl
       )
  ),
  !.

/*
10 Average Size of a Tuple

---- tuplesize(Rel, Size) :-

----

The average size of a tuple in Bytes of relation ~Rel~
is ~Size~.

10.1 Get The Tuple Size

Succeed or failure of this predicate is quite similar to
predicate ~card/2~, see section about cardinalities of
relations. The predicate returns the average total tuplesize, including Flobs.

If the relation has cardinality = 0, Secondo will return a undefined tuplesize.
Therefor, nAn (not a number) is stored in the internal information database,
but 1 is returned for the tuplesize to avoid problems, e.g. when calculating
sample sizes.

*/

tupleSizeQuery(RelE, QueryAtom) :-
  secondoCatalogInfo(RelDC,RelE,_,_),
  systemTable(RelDC,_),                      % special case: trel objects
  concat_atom(['query ', RelE, ' feed tuplesize'],QueryAtom), !.

tupleSizeQuery(RelE, QueryAtom) :-
  concat_atom(['query ', RelE, ' tuplesize'],QueryAtom).

tuplesize(DCrel, TupleSize) :-
  dm(dbhandling,['\nTry: tuplesize(',DCrel,',',Size,').']),
  databaseName(DB),
  storedTupleSize(DB, DCrel, Size),
  ( ( Size = nAn )
    -> ( write_list(['\nWARNING:\tTuplesize is not a number (nAn). ', Size,
                     '\n--->\tTherefore, tuplesize is set to 1.']),
         nl,
         TupleSize is 1
       )
    ;  ( Size =:= 0
         -> ( write_list(['\nWARNING:\tTuplesize is 0. ', Size,
                     '\n--->\tTherefore, tuplesize is set to 1.']),
              nl,
              TupleSize is 1
            )
         ; TupleSize is Size
       )
  ),
  !.

tuplesize(DCrel, TupleSize) :-
  dm(dbhandling,['\nTry: tuplesize(',DCrel,',',Size,').']),
  dcName2externalName(DCrel,ExtRel),
  tupleSizeQuery(ExtRel, QueryAtom),
  secondo(QueryAtom, [real, Size]),
  databaseName(DB),
  ( ( Size \= undef )
    -> ( StoreTupleSize is Size,
         TupleSize is Size
       )
    ;  ( write_list(['\nWARNING:\Tuplesize query has strange result: ', Size,
                     '\n--->\tTherefore, tuplesize is set to \'not a number\'',
                     '(nAn).','\n--->\tPossibly card(',DCrel,')=0?']),
         nl,
         StoreTupleSize = nAn,
         TupleSize is 1
       )
  ),
  assert(storedTupleSize(DB, DCrel, StoreTupleSize)),
  !.

tuplesize(X, Y) :-
  write('ERROR:\tCannot retrieve tuplesize for relation \''),write(X),  %'
  write('\'.'),nl,                                                      %'
  throw(error_SQL(database_tuplesize(X, Y):cannotRetrieveTuplesize));
  !, fail.

/*
The following version of the predicate,

---- tupleSizeSplit(+DCrel, -Size)
----

returns the average tuplesize in a more detailed format, namely as term
~sizeTerm(CoreSize, InFlobSize, ExtFlobSize)~.

*/

tupleSizeSplit(DCrel, Size) :-
  databaseName(DB),
  relation(DCrel, AttrList),
  tupleSizeSplit2(DB, DCrel, AttrList, Size), !.

tupleSizeSplit(DCrel, X) :-
  throw(error_SQL(database_tupleSizeSplit(DCrel, X):unknownError)),
  fail, !.

tupleSizeSplit2(_, _, [], sizeTerm(0,0,0)) :- !.
tupleSizeSplit2(DB, DCRel, [Attr|Rest], TupleSize) :-
  storedAttrSize(DB, DCRel, Attr, _, Core, InFlob, ExtFlob),
  tupleSizeSplit2(DB, DCRel, Rest, RTupleSize),
  addSizeTerms([sizeTerm(Core,InFlob,ExtFlob),RTupleSize],TupleSize), !.

/*
10.2 Storing And Loading Tuple Sizes

*/
readStoredTupleSizes :-
  retractall(storedTupleSize(_, _, _)),
  [storedTupleSizes].

writeStoredTupleSizes :-
  open('storedTupleSizes.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredTupleSize(FD), _),
  close(FD).

writeStoredTupleSize(Stream) :-
  storedTupleSize(DB, X, Y),
  write(Stream, storedTupleSize(DB, X, Y)),
  write(Stream, '.\n').

:-
  dynamic(storedTupleSize/3),
  at_halt(writeStoredTupleSizes),
  readStoredTupleSizes.

% try to create/delete samples and ignore error codes.

tryCreate(QueryAtom) :-
  write('tryCreate: '), write(QueryAtom), nl,
  secondo_direct(QueryAtom), !.

tryCreate(_) :-
  write('tryCreate: Using existing object!' ), nl.

tryDelete(QueryAtom) :-
  secondo_direct(QueryAtom), !.

tryDelete(_).


/*
11 Datatype Core Tuple Sizes

To calculate the proper sizes of attributes, the optimizer needs information
on how much memory the representation of available Secondo datatypes need.
To get this information, a systemtable with this information is queried whenever
a database is opened (see file ~auxiliary.pl~). The systemtable is a relation
contaning (among others) two attributes ~Type~ (containing the name of a
datatype) and ~Size~ (containing its size in byte).

*/

:-assert(helpLine(showDatatypes,0,[],
        'List all registered Secondo data types.')).

:-   dynamic(secDatatype/2).

extractSecondoTypeSizes([]) :- !.

extractSecondoTypeSizes([X|Rest]) :-
  X = [TypeNameQuoted, TypeSize],
  sub_atom(TypeNameQuoted,1,_,1,TypeName),
  downcase_atom(TypeName, TypeNameDC),
  assert(secDatatype(TypeNameDC, TypeSize)),
  extractSecondoTypeSizes(Rest).

readSecondoTypeSizes :-
  retractall(secDatatype(_, _)),
  isDatabaseOpen, !,
  secondo('query SEC2TYPEINFO feed project[Type, Size] consume',SecList),
  SecList = [_,L],
  extractSecondoTypeSizes(L).

showOneDatatype :-
  secDatatype(X, Y),
  write(X), write(': '), write(Y), write(' byte\n').

showDatatypes :-
  findall(_, showOneDatatype, _).

/*
Deprecated version:

----
readStoredTypeSizes :-
  retractall(secDatatype(_, _)),
  [storedTypeSizes].

:-  readStoredTypeSizes.

----

*/

/*
12 Showing, Loading, Storing and Looking-Up Attribute Sizes and Types

Together with the attributes` type, this information is stored as facts
~storedAttrSize(Database, Rel, Attr, Type, CoreSize, InFlobSize, ExtFlobSize)~ in memory. between sessions information is stored in file ~storedAttrSizes.pl~.

Throughout the optimizer, attribute sizes are passed in terms ~sizeTerm(CoreSize, InFlobSize, ExtFlobSize)~.

*/

:-assert(helpLine(showStoredAttrSizes,0,[],
        'List metadata on attribute sizes in current DB.')).


attrSize(DCRel:DCAttr, sizeTerm(CoreSize, InFlobSize, ExtFlobSize)) :-
  databaseName(DBName),
  storedAttrSize(DBName, DCRel, DCAttr, _, CoreSize, InFlobSize, ExtFlobSize),
  !.

attrSize(X, Y) :-
  throw(error_SQL(database_attrSize(X, Y):missingData)),
  fail, !.


attrType(DCRel:DCAttr, Type) :-
  databaseName(DBName),
  storedAttrSize(DBName, DCRel, DCAttr, Type, _, _, _), !.

attrType(X, Y) :-
  throw(error_SQL(database_attrType(X, Y):missingData)),
  fail, !.

readStoredAttrSizes :-
  retractall(storedAttrSize(_, _, _, _, _, _, _)),
  [storedAttrSizes].

writeStoredAttrSizes :-
  open('storedAttrSizes.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredAttrSize(FD), _),
  close(FD).

writeStoredAttrSize(Stream) :-
  storedAttrSize(Database, Rel, Attr, Type, CoreSize, InFlobSize, ExtFlobSize),
  write(Stream, storedAttrSize(Database, Rel, Attr, Type, CoreSize,
                               InFlobSize, ExtFlobSize)),
  write(Stream, '.\n').

showStoredAttrSize :-
  storedAttrSize(Database, Rel, Attr, Type, CoreSize, InFlobSize, ExtFlobSize),
  write(Database), write('.'), write(Rel), write('.'),
  write(Attr), write(': \t'), write(Type),
  write(' ('), write(CoreSize), write('/'),
  write(InFlobSize), write('/'), write(ExtFlobSize), write(')\n').

showStoredAttrSizes :-
  write('Stored attribute sizes\nRel.Attr: Type '),
  write('(CoreTupleSize/Avg.InlineFlobSize/Avg.ExtFlobSize) [byte]:\n'),
  findall(_, showStoredAttrSize, _).

:-
  dynamic(storedAttrSize/7),
  at_halt(writeStoredAttrSizes),
  readStoredAttrSizes.

% sum-up a list of sizeTerms
addSizeTerms([], sizeTerm(0, 0, 0)) :- !.
addSizeTerms([sizeTerm(X1,Y1,Z1)|Rest], sizeTerm(Res1, Res2, Res3)) :-
  addSizeTerms(Rest, sizeTerm(X2,Y2,Z2)),
  Res1 is X1 + X2,
  Res2 is Y1 + Y2,
  Res3 is Z1 + Z2, !.
addSizeTerms(X, Y) :-
  throw(error_SQL(database_addSizeTerms(X, Y):unknownError)),
  fail, !.

/*
12 Ordering Information on Relations

The ``interesting orders'' extension processes information on the ordering of
stored relations to exploit orderings by using the efficient ~mergejoin~
operator.

Additionally, the ``adaptive join'' extension needs shuffled relations for
proper operation.

These information is stored in (dynamic) facts

---- storedOrder(DBName, DCrelName, DCattribute)
----

where ~DBName~ is the name of the database, ~DCrelName~ is the name of the
relation and ~DCattribute~ is a attribute of the relation, by that the relation is
ordered (by means of the Secondo standard ordering schema). If ~Attribute~ is
~none~, the relation are either not ordered, or no information on its ordering
is available.

If ~Attribute~ is ~shuffled~, the tuples of the relation have explicitly been
shuffled and can be used with the adaptive ~pjoin~ operator.

At the moment, ordering information must be maintained by hand. It will be
written and reread to/from a persistent file, but will not be inquired or
automatically updated.

*/

readStoredOrders :-
  retractall(storedOrder(_,_,_)),
  [storedOrderings].

writeStoredOrders :-
  open('storedOrderings.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredOrder(FD), _),
  close(FD).

writeStoredOrder(Stream) :-
  storedOrder(DB, Rel, Attr),
  write(Stream, storedOrder(DB, Rel, Attr)),
  write(Stream, '.\n').

/*
The order can be defined for only one of a relations' attributes.
Even if it is sorted according to more than one attributes. The
sort order is assumed to be ascending. The database, relation and
attribute names will be stored in down cased spelling.

*/

:-assert(helpLine(showStoredOrders,0,[],
        'List metadata on stored orderings in current DB.')).

hasStoredOrder(DB, DCRel, DCAttr) :-
  storedOrder(DB, DCRel, DCAttr).

changeStoredOrder(DB, DCRel, DCAttr) :-
  retract(storedOrder(DB, DCRel, DCAttr)),
  assert(storedOrder(DB, DCRel, DCAttr)).

changeStoredOrder(DB, DCRel, DCAttr) :-
  assert(storedOrder(DB, DCRel, DCAttr)).

showStoredOrders(_) :-
  storedOrder(Database, Rel, Attr),
  write(Database), write('.'), write(Rel), write(': \t\t'),
  write(Attr), nl.

showStoredOrders :-
  write('Stored orders'), nl,
  write('Database.Rel: \t\tAttr'), nl,
  write('--------------------------------------'), nl,
  findall(X, showStoredOrders(X), _).

:-
  dynamic(storedOrder/3),
  at_halt(writeStoredOrders),
  readStoredOrders.


/*
13 System Tables

System Tables are used in Secondo to retrieve information on operators,
data types, operator usage, etc.

While they can somehow be treated similar to ordinary relation objects,
they behave sligthly different. Also, the optimzer should treat them differently,
e.g. it should not create small and sample files or indexes for these objects.

To identify these objects, we use facts

---- systemTable(?SystemFileInternalName,?Description)
----

where ~SystemFileInternalName~ is the name of the according  system table in InternalSpelling,
and ~Description~ is a text describing the system table's meaning.

*/

systemTable(sec_derived_obj,
'A table of derived objects, which are dynamically created after a restore.').
systemTable(sec2cacheinfo,'A table with cache statistics').
systemTable(sec2fileinfo,'A table with file access statistics').
systemTable(sec2operatorinfo,'A table with operator descriptions').
systemTable(sec2operatorusage,'A table with statistics on operator usage').
systemTable(sec2typeinfo,'A table with data type descriptions').
systemTable(sec2commands,'A table with the kernel command history').
systemTable(sec2counters,'A table with the counter history').

/*
14 System Identifiers

Several identifiers are used by the Secondo System and are therefore invalid
for naming database objects, attributes etc.

All names of operators and data types available in the Secondo kernel are
system identifiers. Operators and data types can be retrieved from system tables
SEC2OPERATORINFO and SEC2TYPEINFO.

There are some more system identifiers, e.g. ~const~ and ~value~., wjich are
used to construct constant value expressions.

Such additional identifiers, that cannot be queried from the Secondo kernel,
are explicitly defined asserting facts

---- systemIdentifier(ExtId, DCid).
----

where ~DCid~ is the name in down cased spelling, and ~ExtId~ is the name in
external spelling.

---- validIdentifier(+ExtId)
----

Succeeds, iff ~ExtId~ (in external spelling) is not a reserved system identifier,
starts with a alphabethic character, and has a lengths of 40 characters at most.

You should use these facts and the predicate to verify, that identifiers for
database objects and attributes are valid before using them in the database.

The list of system identifiers is updated by calling

---- readSystemIdentifiers/0
----

A list of all system identifiers can be printed to screen by calling

---- showSystemIdentifiers/0
----

*/
:- dynamic(systemIdentifier/2).

:-assert(helpLine(showSystemIdentifiers,0,[],
         'List all known system identifiers.')).

validIdentifier(ExtId) :-
  ( ground(ExtId)
    -> true
    ;  ( write_list(['WARNING: Identifier \'', ExtId,
                     '\' is not ground.\n']), nl,
         !, fail
       )
  ),
  ( ( sub_atom(ExtId, 0, 1, _, First), char_type(First,alpha) )
    -> true
    ; ( write_list(['WARNING: Identifier \'', ExtId,
                    '\' does not start with a letter!\n']), nl,
         !, fail
      )
  ),
  ( ( atom_length(ExtId,Length), Length =< 40 )
    -> true                 % the kernel allows for 48, but we keep a reserve
    ;  ( write_list(['WARNING: Identifier \'', ExtId,
                     '\' is longer than 40 characters!\n']), nl,
         !, fail
       )
  ),
  ( systemIdentifier(ExtId,_)
    -> (  write_list(['WARNING: Identifier \'', ExtId,
                      '\' is a reserved system identifier!\n']), nl,
         !, fail
       )
    ;  true
  ),
  ( ( sub_atom(ExtId,_,1,_,Char), not(char_type(Char,csym)) )
    -> ( write_list(['WARNING: Identifier \'', ExtId,
                     '\' contains prohibited character \'',Char,'\'.\n']), nl,
         !, fail
       )
    ;  true
  ),
  !.

readSystemIdentifiers :-
  retractall(systemIdentifier(_,_)),
  assert(systemIdentifier(value,value)),
  assert(systemIdentifier(const,const)),
  assert(systemIdentifier(query,query)),
  assert(systemIdentifier(let,let)),
  assert(systemIdentifier(update,update)),
  assert(systemIdentifier(delete,delete)),
  assert(systemIdentifier(restore,restore)),
  assert(systemIdentifier(database,database)),
  assert(systemIdentifier(list,list)),
  assert(systemIdentifier(create,create)),
  assert(systemIdentifier(types,types)),
  assert(systemIdentifier(objects,objects)),
  assert(systemIdentifier(operators,operators)),
  assert(systemIdentifier(algebra,algebra)),
  assert(systemIdentifier(algebras,algebras)),
  assert(systemIdentifier(save,save)),
  assert(systemIdentifier(to,to)),
  assert(systemIdentifier(from,from)),
  write('\nINFO:\tNow retrieving system identifiers...'),nl,
  secondo('query SEC2OPERATORINFO feed project[Name] tconsume', OperatorList),
  OperatorList = [_, OL], flatten(OL,OLF), sort(OLF,OLS),
  secondo('query SEC2TYPEINFO feed project[Type] tconsume', TypeList),
  TypeList = [_, TL], flatten(TL,TLF), sort(TLF,TLS),
  append(OLS,TLS,JointList),
  sort(JointList,JointList2),
  storeSystemIdentifierList(JointList2),
  write('\nINFO:\tFinished retrieving system identifiers.'),nl,
  !.

storeSystemIdentifierList([]).
storeSystemIdentifierList([ExtString|Rest]) :-
  string_to_atom(ExtString,ExtQuotedAtom),
  sub_atom(ExtQuotedAtom,1,_,1,ExtAtom),
  sub_atom(ExtAtom, 0, 1, _, First),
  ( char_type(First,alpha)
    -> ( dcName2externalName(DCid,ExtAtom),
         assert(systemIdentifier(ExtAtom,DCid)), !
       )
    ;  true
  ),
  storeSystemIdentifierList(Rest).

showSystemIdentifiers :-
  nl,
  write_list(['Table of reserved system identifiers\n',
              '------------------------------------\n',
              'The following identifiers are reserved and may not be used \n',
              'for database objects or attributes:\n\n']),
  findall( X, systemIdentifier(X,_), ReservedIdList),
  sort(ReservedIdList,ReservedIdListSorted),
  findall(_,( member(Id,ReservedIdListSorted),
              write_list(['\t',(Id),'\n'])
            ),
          _),
  nl,!.

/*
15 Managing Relations Within the Database

The following predicates are used to handle relations within the currently opened
database:

---- create_relation(+ExtRelName, +AttrTypeList)
drop_relation(+ExtRelName)
insert_into_relation(+DCrelName, +TupleList)
delete_from_relation_by_value(+DCrelName, +TupleList)
----

*/

/*
---- create_relation(+ExtRelName, +AttrTypeList)
----

Creates a database object named ~ExtRelName~ (given in external spelling) as an
object of type rel(tuple((A1 T1) (A2 T2) ... (An Tn)), where (Ai Ti) are pairs
of (ExtAttributeName DCtypeName). The attributenames and types are passed as a
list [[A1, V1], [A2, T2], ..., [An, Tn]].

The relation is empty. If the creation fails, an exception is thrown.

*/

create_relation(ExtRelName,AttrTypeList) :-
  dm(dbhandling,['\nTry: create_relation(',ExtRelName,',',AttrTypeList,').']),
  ( databaseName(_)
    -> true
   ;  (  write('ERROR:\tNo database open. Cannot create relation!'),nl,
         throw(error_SQL(database_create_relation(ExtRelName,AttrTypeList)
                        :noDatabaseOpen)),
         !, fail
      )
  ),
  dcName2externalName(DCname,ExtRelName),
  ( secondoCatalogInfo(DCname,ExtRelName2,_,_)
    -> ( write_list(['\nERROR:\tFailed trying to create relation \'',
                     ExtRelName,'\'.',
                     '\n--->\tThere is already an object called \'',
                     ExtRelName2,'\' in the data base.']),nl,
         throw(error_SQL(database_create_relation(ExtRelName,AttrTypeList)
                        :fileAlreadyExists)), fail
       )
    ;  true
  ),
  ( validIdentifier(ExtRelName)
    -> true
    ;  write_list(['\nERROR:\tFailed trying to create relation \'',
                     ExtRelName,'\'.',
                     '\n--->\tThe relation name is invalid.']),nl,
         throw(error_SQL(database_create_relation(ExtRelName,AttrTypeList)
                        :invalidObjectName)), fail
  ),
  ( checkAttrTypeList(AttrTypeList)
    -> true
    ;  write_list(['\nERROR:\tFailed trying to create relation \'',
                     ExtRelName,'\'.',
                     '\n--->\tThe attribute/type list is invalid.']),nl,
         throw(error_SQL(database_create_relation(ExtRelName,AttrTypeList)
                        :invalidArgument)), fail
  ),
  getConstRelationTypeExpression(AttrTypeList,TypeExpr),
  concat_atom(['let ', ExtRelName, ' = [const ', TypeExpr, ' value ()]'],
              QueryAtom),
  ( secondo(QueryAtom)
    -> true
    ;  ( write_list(['\nERROR:\tFailed trying to create relation \'',
                     ExtRelName,'\'.',
                     '\n--->\tSecondo let command failed.']),nl,
         throw(error_SQL(database_create_relation(ExtRelName,AttrTypeList)
                        :secondoCommandFailed)), fail
       )
  ),!,
  updateCatalog,
  updateCatalog.

create_relation(ExtRelName,AttrTypeList) :-
    write_list(['\nERROR:\tFailed trying to create relation \'',
                ExtRelName,'\'.']),nl,
    throw(error_SQL(database_create_relation(ExtRelName,AttrTypeList)
                   :unknownError)),
    !, fail.

/*
---- checkAttrTypeList(+AttrTypeList)
----

Succeeds, iff ~AttrTypeList~ is a list of valid \[AttrName, Type\]-lists.
All ~AttrName~s must be valid attribute names, all ~Type~s must be
registered Secondo types. All ~AttrName~s must be pairwise different.

*/
checkAttrTypeList(AttrTypeList) :-
  ( AttrTypeList = []
    -> ( write_list(['\nERROR:\tRelation schema must contain one attribute ',
                     'at least!']), nl,
         fail
       )
    ;  true
  ),
  checkAttrTypeList2(AttrTypeList,DCattrNameList),
  ( is_set(DCattrNameList)
    -> true
    ;  ( write_list(['\nERROR:\tAttribute names are not unique in relation ',
                     'schema ',AttrTypeList,'!']), nl,
         fail
       )
  ),
  !.

checkAttrTypeList2([],[]) :- !.
checkAttrTypeList2([[ExtAttrName, Type] | MoreLists],
                  [DCattrName|MoreDCattrNames]) :-
  ( secDatatype(Type, _)
    -> true
    ;  ( write_list(['\nERROR:\tUnknown data type \'', Type,'\'.']),nl,
         fail
       )
  ),
  ( secondoCatalogInfo(_,ExtAttrName,_,_)
    -> (write_list(['\nERROR:\tAttribute name \'',ExtAttrName,'\' is shadowed',
                    '\n--->\tby the name of an existing database object',
                    '\n--->\tcalled \'',ExtAttrName,'\'.',
                    '\n--->\tThis would cause problems in queries.']),nl,
        fail
       )
    ;  true
  ),
  dcName2externalName(DCattrName,ExtAttrName),
  !,
  checkAttrTypeList2(MoreLists,MoreDCattrNames).

/*

----getConstRelationTypeExpression(+AttrTypeList,TypeExpr)
----

Create type expression rel(tuple((A1 T1) ... (An Tn))) from ~AttrTypeList~.

*/
getConstRelationTypeExpression(AttrTypeList,TypeExpr) :-
  getConstRelationTypeExpression2(AttrTypeList,TupleExpr),
  concat_atom(['rel(tuple(',TupleExpr,'))'],TypeExpr),
  !.

getConstRelationTypeExpression2([[AttrName, Type]],AttrExpr) :-
  concat_atom(['(',AttrName, ' ', Type, ')'], AttrExpr),
  !.

getConstRelationTypeExpression2([[AN,TY] | More],AttrExpr) :-
  getConstRelationTypeExpression2(More,MoreExpr),
  concat_atom(['(',AN,TY,')',MoreExpr],' ',AttrExpr),
  !.

/*
---- drop_relation(+ExtRelName)
----

Drop the relation and possibly dependent objects, delete meta data.
The relation name is passed in ~external spelling~.

*/
drop_relation(ExtRelName) :-
  dm(dbhandling,['\nTry: drop_relation(',ExtRelName,').']),
  ( databaseName(_)
    -> true
   ;  (  write('ERROR:\tNo database open. Cannot drop relation!'),nl,
         throw(error_SQL(database_drop_relation(ExtRelName):noDatabaseOpen)),
         !, fail
      )
  ),
  ( ( secondoCatalogInfo(DCrel,ExtRelName,_,Type),
      Type = [[rel, [tuple, _]]]
    )
    -> true
    ;  (  write_list(['\nERROR:\tObject \'',ExtRelName,
                      '\' unknown or not a relation.']),nl,
          throw(error_SQL(database_drop_relation(ExtRelName):typeError)), !, fail
       )
  ),
  deleteObject(ExtRelName),
  handleLostRelation(DCrel),
  updateCatalog,
  updateCatalog,
  write_list(['\nINFO:\tSuccessfully dropped relation \'',ExtRelName,'\'.']),
  nl, !.

drop_relation(ExtRelName) :-
    write_list(['\nERROR:\tFailed trying to drop \'',ExtRelName,'\'.']),nl,
    throw(error_SQL(database_drop_relation(ExtRelName):unknownError)),
    !, fail.

/*

---- insert_into_relation(+DCrelName, +TupleList)
----

Inserts ~n~ tuples into the relation ~DCrelName~, having type
rel(tuple((A1 T1)(A2 T2)...(Am Tm))). The tuples to insert are passed within
~TupleList~ as a list [[V11,V12,...,V1m],[V21,V22,...,V2m],...,[Vn1,Vn2,...,Vnm]],
where the Vij are valid value expressions of type Tj.

The tuples are also inserted into all known indexes for that relation.
Small objects, samples and stored metadata are NOT updated!

If the insertion fails, an exception is thrown.

*/


/*

---- delete_from_relation_by_value(+DCrelName, +TupleList)
----

Deletes ~n~ tuples from relation ~DCrelName~, having type
rel(tuple((A1 T1)(A2 T2)...(Am Tm))). The tuples to delete are passed within
~TupleList~ as a list [[V11,V12,...,V1m],[V21,V22,...,V2m],...,[Vn1,Vn2,...,Vnm]],
where the Vij are valid value expressions of type Tj.

The tuples are also deleted from all known indexes for that relation.
Small objects, samples and stored metadata are NOT updated!

*/
