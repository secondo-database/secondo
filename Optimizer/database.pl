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
//[toc] [\tableofcontents]

[toc]


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

/*
NVK ADDED NR
Nested relation support.
Applies downcase\_atom on every atom within the attribute list.
The other direction works fine with the standard predicate.

*/

dcName2internalName(DC, Intern) :-
  ground(Intern),                % second arg instantiated
  Intern=_:_,
  applyOnAttributeList(dcName2internalName, DC, Intern),
  !.
% NVK ADDED NR END

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
  secondoCatalogInfo(DC,Extern,_,_), % get stored external name from catalog
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
  throw(error_Internal(database_dcName2internalName(DC,Intern)
                    ::cannotTranslate)),
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
  throw(error_Internal(database_dcName2externalName(DC,External)
                      ::cannotTranslate)),
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
  my_concat_atom([PrefixUC,Suffix],'',Extern),
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
  my_concat_atom([PrefixDC,Suffix],'',InternDC),
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
  throw(error_Internal(database_internalName2externalName(Intern,Extern)
                 ::cannotTranslate)),
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
~storedAttrSize(DB, DCRel, DCAttr, Type, MemSize, CoreSize, LOBSize)~.

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

Prior to asserting the ~storedSpell/3~, ~storedAttrSize/7~,  ~storedTupleSize/3~,
~storedCard/3~, ~storedOrderings/3~ facts, all facts regarding the relation
and its attributes are retracted.

PROBLEM: The Secondo QueryProcessor only supports a fixed amount of function
objects, which are used by the projectextend operator. You should modify file

---- secondo/QueryProcessor/QueryProcessor.cpp
----

and change the constant

---- const int MAXFUNCTIONS = 30;
----

from 30 to at least $3\cdot(maximum number of attributes)+5$.

*/

updateRelationSchema(DCrel) :-
  dm(dbhandling,['\nTry: updateRelationSchema(',DCrel,').']),
  ( databaseName(_)
    -> true
    ;  ( my_concat_atom(['Cannot update relation schema for \'',DCrel,
                     '\': No database open.'],'',ErrMsg),
         write_list(['\nERROR:\t',ErrMsg]), nl,
         throw(error_Internal(database_updateRelationSchema(DCrel)
                              ::noDatabaseOpen::ErrMsg)),
         fail, !
       )
  ),
  retractStoredInformation(DCrel),
  ( ( secondoCatalogInfo(DCrel,ExtRel, _, TypeExpr),
      (   TypeExpr = [[rel, [tuple, _ExtAttrList]]]
        ; TypeExpr = [[trel, [tuple, _ExtAttrList]]]
        % NVK ADDED NR: receive data as well for nrel tables.
        ; 
          (
            optimizerOption(nestedRelations), 
            TypeExpr = [[nrel, [tuple, _ExtAttrList]]]
          ) 
        % NVK ADDED NR END
      )
    )
    -> true
    ;  ( my_concat_atom(['Cannot retrieve information on relation \'',
                     DCrel,
                     '\':\n','--->\tNo matching relation found in catalog.','',
                     ErrMsg]),
         write_list(['ERROR:\t',ErrMsg]),nl,
         throw(error_Internal(database_updateRelationSchema(DCrel)
                ::lostObject::ErrMsg)),
         fail, !
       )
  ),
  % The predicate getTupleInfo(+DCrel) will inquire for relation schema,
  % cardinality, tuple sizes, attribute sizes and spelling issues:
  ( getTupleInfo(DCrel)
    -> true
    ; (  my_concat_atom(['Failure retrieving meta data on ',
                     'relation \'',ExtRel,'\': .'],'',ErrMsg),
         write_list(['\nERROR:\t',ErrMsg]),nl,
         throw(error_Internal(database_updateRelationSchema(DCrel)
                        ::cannotRetrieveMetaData::ErrMsg)),
         fail, !
      )
  ),
  write_list(['\nINFO:\tUpdated information on relation \'',ExtRel,'\'.']), nl,
  !.

updateRelationSchema(DCrel) :-
  my_concat_atom(['updateRelationSchema failed for \'',DCrel,'\'.'],'',ErrMsg),
  write_list(['ERROR:\t',ErrMsg]),nl,
  throw(error_Internal(database_updateRelationSchema(DCrel)
                 ::cannotLookupRelationschema::ErrMsg)).


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
         throw(error_Internal(database_refreshSecondoCatalogInfo
                                   ::noDatabaseOpen)),
         !, fail
       )
  ), !,
  ( getSecondoList(ObjList)
    -> ( true )
    ;  (
         write('ERROR:\trefreshSecondoCatalogInfo failed!'), nl,
         write('--->\tRetrieving Catalog failed!'),
         throw(error_Internal(database_refreshSecondoCatalogInfo
                        ::cannotRetrieveCatalog)),
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
unique downcased naming convention or is named like a typeconstructor or operator.

Check, whether some relation has an attribute named exactly as a dababase
object, typeconstructor or operator.

If so, an exception is thrown. Otherwise the predicate succeeds.

*/

% getAllAttributeNameConflicts/0
%   succeeds, if no problem is found. Otherwise throws an exception!
checkForAttributeNameConflicts :-
  findall(Clashes, (
                     secondoCatalogInfo(DCrel,_,_,[[RelType, [tuple, _]]]),
                     % NVK MODIFIED
                     % The check clause has to be extended to check nested
                     % attribute names.
                     %member(RelType,[trel,rel]),
                     member(RelType,[trel,rel,nrel]),
                     % NVK MODIFIED END
                     getAttributeNameConflicts(DCrel,ConflictList),
                     ConflictList \= [],
                     secondoCatalogInfo(DCrel,ExtRel,_,_),
                     Clashes = [ExtRel,ConflictList]
                   ),
          FoundClashes),
  length(FoundClashes,NoInstances),
  ( NoInstances > 0
    -> (
         term_to_atom(FoundClashes,FoundClashedA),
         my_concat_atom(
           ['\nERROR:\tSome of the relations have attribute names ',
           '\n--->\tthat are also used for database objects, or are ',
           '\n--->\treserved identifiers, such as for keywords, type ',
           '\n--->\tconstructors, or operators. ',
           '\n--->\tThis is not allowed, because it creates problems while ',
           '\n--->\tprocessing database commands. ',
           '\n--->\tPlease rename the following attributes (or disable the ',
           '\n--->\talgebra modules defining the according types/operators):\n',
           '\n--->\t\t',FoundClashedA,'\n'
           ],'',ErrMsg),
         write_list(['\nERROR:\t',ErrMsg]), nl, nl,
         throw(error_SQL(database_checkForAttributeNameConflicts
                        ::schemaError::ErrMsg))
       )
    ; true
  ),!.

%  getAttributeNameConflicts(+DCrel,-ConflictList)
getAttributeNameConflicts(DCrel,ConflictList) :-
  secondoCatalogInfo(DCrel,ExtRel,_,[[RelType, [tuple, AttrList]]]),
  % NVK MODIFIED
  %member(RelType,[trel,rel]),
  member(RelType,[trel,rel,nrel]),
  getAttributeNameConflicts2(ExtRel,AttrList,ConflictList), !.

% returns a list of conflicting attribute names, i.e.
% attribute names that are also used as an object name in the current database.
getAttributeNameConflicts2(_,[],[]) :- !.

/* 
NVK ADDED NR
I don't know if that was really necessary, i was not able to create a relation that is violating this check.

*/
getAttributeNameConflicts2(ExtRel,[[Attr,Type]|More], ConflictList) :-
  optimizerOption(nestedRelations),
  !,
  appendAttribute(ExtRel, Attr, FQN),
  (Type = [arel,[tuple,ARelAttrList]] ->
    % Go depper into the arel structure
    getAttributeNameConflicts2(FQN, ARelAttrList, ARelConflicts)
  ;
    ARelConflicts=[]
  ),

  ((secondoCatalogInfo(_,Attr,_,_) ; systemIdentifier(Attr,_) ) ->
    AConflict=[FQN]
  ;  
    AConflict=[]
  ), 
  % check the rest
  getAttributeNameConflicts2(ExtRel, More, RestConflictList), 
  appendLists([ARelConflicts, AConflict, RestConflictList], ConflictList),
  !.
% NVK ADDED NR END

getAttributeNameConflicts2(ExtRel,[[Attr,_]|More],[ExtRel:Attr|ConflictList]) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED
  ( secondoCatalogInfo(_,Attr,_,_) ; systemIdentifier(Attr,_) ) , !,
  getAttributeNameConflicts2(ExtRel,More,ConflictList), !.

getAttributeNameConflicts2(ExtRel,[_|More],ConflictList) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED
  getAttributeNameConflicts2(ExtRel,More,ConflictList), !.


% The main predicate
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
         term_to_atom(SortedClashes,SortedClashesA),
         my_concat_atom(
           ['\nERROR:\tThe database contains objects violating the unique',
           '\n--->\tdowncased naming convention.',
           '\n--->\tPlease rename the following objects such that no two of\n',
           '\n--->\ttheir names are the same, when using lower case \n',
           '\n--->\tletters only:\n',
           '\n--->\t\t',SortedClashesA,'\n'],'',ErrMsg),
         write_list(['\nERROR:\t',ErrMsg]), nl, nl,
         throw(error_SQL(database_checkObjectNamingConvention(SortedClashes)
                        ::schemaError::ErrMsg))
       )
    ; true
  ),
  findall(ObjectExt1,(
              secondoCatalogInfo(ObjDC,ObjectExt1,_,_),
              systemIdentifier(ObjectExt1,ObjDC)
                     ),
          KeyClashes
         ),
  flatten(KeyClashes,FlatKeyClashes),
  sort(FlatKeyClashes,SortedKeyClashes),
  length(SortedKeyClashes,NoKeyInstances),
  ( NoKeyInstances > 0
    -> (
         term_to_atom(SortedKeyClashes,SortedKeyClashesA),
         my_concat_atom(
           ['\nERROR:\tThe database contains objects having reserved keyword ',
           '\n--->\tnames or names of type constructors or operators.',
           '\n--->\tPlease rename the following objects or deactivate the',
           '\n--->\algebras exporting the according types/operators:\n',
           '\n--->\t\t',SortedKeyClashesA,'\n'],'',ErrMsg),
         write_list(['\nERROR:\t',ErrMsg]), nl, nl,
         throw(error_SQL(database_checkObjectNamingConvention(SortedKeyClashes)
                        ::schemaError::ErrMsg))
       )
    ; true
  ),
  checkForAttributeNameConflicts,
  !.

/*
3.2 Identifying Added and Lost Catalog Entries

The following predicates are used to find the names (in ~down cased spelling~)
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
              % NVK ADDED NR
              % OLD:
              %secondoCatalogInfo(DCRel,_,_,[[rel, _]]),
              % NEW:
              ( secondoCatalogInfo(DCRel,_,_,[[rel, _]])
              ; 
                (
                  optimizerOption(nestedRelations),
                  secondoCatalogInfo(DCRel,_,_,[[nrel, _]])
                )
              ),
              % NVK ADDED NR END
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
    check for new objects in the database. \[DEPRECATED: For each new relation
    object, we retrieve metadata (relation schema, cardinality, attribute sizes,
    orderings). We possibly create sample relations and the small relation. \]
    For each new index, we check whether the base relation exists. If not,
    we delete the index \[DEPRECATED: (and the small index)\] and retract
    according metadata. \[DEPRECATED: If so, we check wheather we need to
    create a small index.\]

\[DEPRECATED: This sequence guarantees, that small relations (and samples) are
created before small indexes are possibly created (which rely on the existence
of small relations).\]

If a relation is removed, this will be detected when searching for lost
indexes and a list of obsolete indexes will be printed.

~updateCatalog~ must be called whenever a database object has been added or
deleted (e.g. after each 'let' or 'delete' command. You do not need to call
it after an 'update' command, as this will neither change the object identifier,
nor the object type (though it might change cardinalities, tuplesizes,
attribute sizes, etc.).

*/
:-assert(helpLine(updateCatalog,0,[],
   'Re-read the current DB\'s catalog and update the metadata.')).        %'

updateCatalog :-
  dm(dbhandling,['\nTry: updateCatalog.']),
  % readSecondoTypeSizes,
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
   ;  (  ErrMsg = 'No database open. Cannot handle new relation!',
         write_list(['ERROR:\t',ErrMsg]),nl,
         throw(error_Internal(database_handleNewRelation(DCrel)
                              ::noDatabaseOpen::ErrMsg)),
         !, fail
      )
  ),
  ( ( secondoCatalogInfo(DCrel,ExtRel,_,Type),
      % NVK ADDED NR
      Type = [[RelType, [tuple, _]]],
      (optimizerOption(nestedRelations) ->
        member(RelType, [rel, nrel])
      ;
        RelType=rel
      )
      % NVK ADDED NR END
    )
    -> true
    ;  (  my_concat_atom(['Object \'',DCrel,
                      '\' unknown or not a relation.'],'',ErrMsg),
          write_list(['\nERROR:\t',ErrMsg]),nl,
          throw(error_Internal(database_handleNewRelation(DCrel)
                ::typeError::ErrMsg)),
          !, fail
       )
  ),
  write_list(['\nINFO:\tRelation \'',ExtRel,
              '\' has been added to the database.\n',
              '--->\tNow retrieving related metadata...']),nl,
  updateRelationSchema(DCrel),   % get spellings, schema, attr names and sizes,
                                 % tuple size, cardinality,
                                 % also retract ordering information
  ( ( optimizerOption(eagerObjectCreation),
      not(sub_atom(DCrel, _, _, 0, '_small')),
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
  throw(error_Internal(database_handleNewRelation(DCrel)::unknownError)), !,
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
          throw(error_Internal(database_handleNewIndex(DCindex)
                              ::noDatabaseOpen)),
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
         ( ( optimizerOption(entropy), optimizerOption(eagerObjectCreation) )
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
         assert(storedIndex(DB,DCrel,DCattr,LogicalIndexType,DCindex)),
         inquireIndexStatistics(DB,ExtIndex,
                                DCindex,DCrel,DCattr,LogicalIndexType)
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
          throw(error_Internal(database_handleLostRelation(DCrel)
                                ::noDatabaseOpen)),
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
                    retractall(storedIndex(DB,DCrel,_,_,DCindexSmall)),
                    retractall(storedIndexStat(DB, DCindexSmall, _, _, _))
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
          throw(error_Internal(database_handleLostIndex(DCindex)
                       ::noDatabaseOpen)),
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
  retractall(storedIndexStat(DB, DCindex, _, _, _)), !,
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
                    retractall(storedIndex(DB,DCrel,_,_,DCindexSmall)),
                    retractall(storedIndexStat(DB, DCindexSmall, _, _, _))
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

They can be used to create all these objects at once. The normal optimizer
behaviour is now not to use these predicates any more, but create these objects
only on demand, i.e. when they are reuired to perform a selectivity query
(sample objects) or if a small query is created by the entropy optimizer
(small objects).

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
            %secondoCatalogInfo(DCrel,_,_,[[rel, [tuple, _]]]),
            % NVK MODIFIED NR
            secondoCatalogInfo(DCrel,_,_,[[RelType, [tuple, _]]]),
            member(RelType, [rel, nrel]),
            % NVK MODIFIED NR END
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
  throw(error_Internal(database_ensureSmallObjectsExist::unknownError)),
  fail, !.


ensureSmallRelationsExist :-
  dm(dbhandling,['\nTry: ensureSmallRelationsExist.']),
  write_list(['\nINFO:\tEnsuring, that small relations exist...']), nl,
  databaseName(DB),
  findall(_,
          ( storedRel(DB,DCrel,_),
            not(sub_atom(DCrel,_,_,0,'_small')),
            not(sub_atom(DCrel,_,_,1,'_sample_')),
            %secondoCatalogInfo(DCrel,_,_,[[rel, [tuple, _]]]),
            % NVK MODIFIED NR
            secondoCatalogInfo(DCrel,_,_,[[RelType, [tuple, _]]]),
            member(RelType, [rel, nrel]),
            % NVK MODIFIED NR END
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
  throw(error_Internal(database_ensureSmallRelationsExist::unknownError)),
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
  throw(error_Internal(database_ensureSmallIndexesExist::unknownError)),
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
  my_concat_atom(['delete ', ExtObjName], '', QueryAtom),
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
  retractPredStats(DCrel),
  retractall(storedTupleSize(DB, DCrel, _, _, _)),
  retractall(storedCard(DB, DCrel, _)),
  retractall(storedOrderings(DB, DCrel, _)),
  retractall(storedIndex(DB, DCrel, _, _, _)),
  retractall(storedIndexStat(DB, _, DCrel, _, _)),
  retractall(storedNoIndex(DB, DCrel, _)),
  retractall(storedAttrSize(DB, DCrel, _, _, _, _, _)),
  retractall(storedRel(DB, DCrel, _)),
  retractall(storedSpell(DB, DCrel:_, _)),
  !.


/*
3.4.3 Creating Sample Relations

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
3.4.5 Creating Small Relations

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
    ; ( write_list(['ERROR:\tName must be atomic! But is: ',Name]), nl,
        throw(error_Internal(database_getSmallName(Name, NameSmall)
                       ::wrongInstantiationPattern)),
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
  buildSmallRelation(ExtRel, ExtRelSmall, MinSize, Percent),
  updateCatalog,!.

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
  atom_concat('XxxID', DCrel, IDAttr),
  % just add tuple counter...
  my_concat_atom(['derive ', ExtRelSmall, ' = ', ExtRel,
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
  atom_concat('XxxID', DCrel, IDAttr),
  my_concat_atom(['derive ', ExtRelSmall, ' = ', ExtRel,
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
  throw(error_Internal(database_buildSmallRelation(ExtRel, ExtRelSmall,
                                                MinSize, Percent)
                                                ::unknownError)).

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
         throw(error_Internal(database_createSmall(DCrel, Size)
              ::wrongSpelling)),
         fail
       )
  ),
  %( secondoCatalogInfo(DCrel,ExtRel,_,[[rel, [tuple, _]]])
  % NVK MODIFIED NR
  ( 
    (
      secondoCatalogInfo(DCrel,ExtRel,_,[[RelType, [tuple, _]]]),
      member(RelType, [rel, nrel])
    )  
  % NVK MODIFIED NR END
    -> ( getSmallName(ExtRel,ExtRelSmall),
         buildSmallRelation(ExtRel, ExtRelSmall, Size, 0.000001)
       )
    ;  ( write('ERROR:\tThat relation does not exist, or no database is open.'),
         nl,
         throw(error_Internal(database_createSmall(DCrel, Size)::lostObject)),
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

The predicates use the following constants defined in ~operators.pl~:

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
    ; ( write_list(['ERROR:\tName must be atomic! But is: ',Name]), nl,
        throw(error_Internal(database_getSampleSname(Name, SampleSname)
                       ::wrongInstantiationPattern)),
        fail
      )
  ), !.

getSampleJname(Name, SampleJname) :-
  dm(dbhandling,['\nTry: getSampleJname(',Name,',',SampleJname,').']),
  ( atomic(Name)
    -> atom_concat(Name,'_sample_j',SampleJname)
    ; ( write_list(['ERROR:\tName must be atomic! But is: ',Name]), nl,
        throw(error_Internal(database_getSampleSname(Name, SampleJname)
                       ::wrongInstantiationPattern)),
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
  ( TupleSize < 1
    -> ( write_list(['ERROR:\tTuplesize for relation ',DCRel,' < 1!']), nl,
         throw(error_Internal(database_getStandardSampleCard(DCRel, CardMin,
                      CardMax, SF, MaxMem,CardStd, MemStd, CardRec, MemRec)
                      ::invalidTupleSize)),
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
         MemRec  is MemStd                  % (recommended StandardSize in KB)
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
  dm(dbhandling,['CardStd=', CardStd, '\nMemStd=',MemStd, '\nCardRec=',CardRec,
                 '\nMemRec=', MemRec, '\n']),
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

% wrapper that only create the sample if it does not exist yet
ensureSampleSexists(DCrel) :-
  dm(dbhandling,['\nTry: ensureSampleSexists(',DCrel,').']),
  getSampleSname(DCrel, SampleName),
  ( secondoCatalogInfo(SampleName,_,_,_)
    -> true
    ;  ( createSampleS(DCrel), updateCatalog )
  ), !.

ensureSampleJexists(DCrel) :-
  dm(dbhandling,['\nTry: ensureSampleJexists(',DCrel,').']),
  getSampleJname(DCrel, SampleName),
  ( secondoCatalogInfo(SampleName,_,_,_)
    -> true
    ;  ( createSampleJ(DCrel), updateCatalog )
  ), !.


createSample(ExtSample, ExtRel, RequestedCard, ActualCard) :-
  dm(dbhandling,['\nTry: createSample(',ExtSample,',',ExtRel,',',
                 RequestedCard,',',ActualCard,').']),
  sampleQuery(ExtSample, ExtRel, RequestedCard, QueryAtom),
  tryCreate(QueryAtom),
  dcName2externalName(DCrel,ExtRel),
  card(DCrel, Card),
  % the following line implements the calculation done by the sample-operator:
  ActualCard is truncate(min(Card, RequestedCard)).

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
         secOptConstant(sampleJoinMaxDiskSize, MemMax),
         secOptConstant(sampleJoinMaxCard, CardMax),
         % secOptConstant(sampleJoinMinCard, CardMin),
         ( ( \+optimizerOption(autoSamples) ,
             CardStd =< CardMax , MemStd =< MemMax )
           -> ( SampleCard is CardStd, SampleSize is MemStd,
                write_list(['\tStandard join sample size of ',
                            SampleCard,'(',SampleSize,' KB) is used.']),nl
              )
          ; true
         ),
         ( var(SampleCard) % sample would become to large or to small
           -> ( optimizerOption(autoSamples)
                -> ( % automatically set sample size and force creation
                    SampleCard is CardRec,
                    SampleSize is MemRec,
                    write_list(['\tJoin sample size determined automatically:',
                                '\n\tStandard size would be ',(CardStd),'(',
                                MemStd,' KB)\n\tUsing recommended size of ',
                                CardRec,'(',MemRec,' KB).']),nl,
                    write_list(['\tUse \'resizeSamples/3\' to force another ',
                                'arbitrary sample size.']),nl
                   )
                ;  ( % leave sample creation to the user
                     my_concat_atom(['REQUEST FOR USER INTERACTION:\n',
                                 'Join sample is to large: ',CardStd,
                                 ' (=',MemStd,' KB).\n','Maximum size is ',
                                 CardRec,' (=',MemRec,' KB).\n\n',
                                 '\tPlease specify the concrete sample ',
                                 'cardinality\n','\tmanually and create the ',
                                 'sample e.g. by calling\n','\tcreateSamples(',
                                 DCRel,', ',CardRec,', ',CardRec,').'],
                                '',ErrMsg),
                     write_list(['ERROR\t',ErrMsg]), nl,
                     throw(error_Internal(database_createSampleJ(DCRel)
                                    ::requestUserInteraction::ErrMsg)),
                     fail
                   )
              )
          ; true
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
         secOptConstant(sampleSelMaxDiskSize, MemMax),
         secOptConstant(sampleSelMaxCard, CardMax),
         % secOptConstant(sampleSelMinCard, CardMin),
         ( ( \+optimizerOption(autoSamples) ,
             CardStd =< CardMax , MemStd =< MemMax )
           -> ( SampleCard is CardStd, SampleSize is MemStd,
                write_list(['\tStandard selection sample size of ',
                            SampleCard,' (',SampleSize,' KB) is used.']),nl
              )
          ; true
         ),
         ( var(SampleCard) % sample would become to large or to small
           -> ( optimizerOption(autoSamples)
                -> ( % automatically set sample size and force creation
                    SampleCard is CardRec,
                    SampleSize is MemRec,
                    write_list(['\tSelection sample size determined ',
                                'automatically:\n\tStandard size is ',
                                '',(CardStd),'(',MemStd,
                                ' KB)\n\tUsing recommended size of ',CardRec,
                                '(',MemRec,' KB).']),nl,
                    write_list(['\tUse \'resizeSamples/3\' to force another ',
                                'arbitrary sample size.']),nl
                   )
                ;  ( % leave sample creation to the user
                       my_concat_atom(['REQUEST FOR USER INTERACTION:\n',
                                    'Selection sample is to large: ',
                                     CardStd,
                                     ' (=',MemStd,' KB).\n','Maximum size is ',
                                     CardRec,' (=',MemRec,' KB).\n\n',
                                     '\tPlease specify the concrete sample ',
                                     'cardinality\n','\tmanually and create ',
                                     'the sample e.g. by calling\n',
                                     '\tcreateSamples(',
                                     DCRel,', ',CardRec,', ',CardRec,').\n'],
                                    '',ErrMsg),
                       write_list(['ERROR\t',ErrMsg]),nl,
                       throw(error_SQL(database_createSampleS(DCRel)
                                      ::requestUserInteraction::ErrMsg)),
                       fail
                   )
              )
          ;   true
         ),
         createSample(Sample, ExtRel, SampleCard, ActualSampleCard),
         write_list(['\tSample cardinality=',ActualSampleCard]),nl,
         databaseName(DB),
         dcName2externalName(DCSample,Sample),
         assert(storedCard(DB, DCSample, ActualSampleCard))
       )
  ), !.

% special case: trel (used for system tables)
sampleQuery(ExtSample, ExtRel, SampleSize, QueryAtom) :-
  dm(dbhandling,['\nTry: sampleQuery(',ExtSample,',',ExtRel,',',SampleSize,',',
                 QueryAtom,').']),
  % NVK MODIFIED NR 
  % Support for samples on nrel relations.
  % Because currentliy there exists no ~sample~ operator for nrel relations, 
  % and the sample operator don't work on streams, this method is used and
  % not the predicate below this that uses the sample operator.
  %secondoCatalogInfo(_,ExtRel,_,[[trel, [tuple, _]]]),
  secondoCatalogInfo(_,ExtRel,_,[[RelType, [tuple, _]]]),
  member(RelType, [trel, nrel]),
  % NVK MODIFIED NR END
  my_concat_atom(['derive ', ExtSample, ' = ', ExtRel,
    ' feed head[', SampleSize, ']
      extend[XxxNo: randint(20000)] sortby[XxxNo asc] remove[XxxNo]
      consume'], '', QueryAtom).

% standard case: rel
sampleQuery(ExtSample, ExtRel, SampleSize, QueryAtom) :-
  dm(dbhandling,['\nTry: sampleQuery(',ExtSample,',',ExtRel,',',SampleSize,',',
                 QueryAtom,').']),
  secOptConstant(sampleScalingFactor, SF),
  my_concat_atom(['derive ', ExtSample, ' = ', ExtRel,
    ' sample[', SampleSize, ',', SF, ']
      extend[XxxNo: randint(20000)] sortby[XxxNo asc] remove[XxxNo]
      consume'], '', QueryAtom).

/*

----  createSamples(+DCRel, +SelectionCard, +JoinCard) :-
----

Create samples for ~DCRel~ (in down cased spellinf) manually, speciying the
size (cardinality) of selection and join samples.

*/

:- assert(helpLine(createSamples,3,
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
                                                      % database setup
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

/* 
NVK ADDED NR
Note: this works different for arel relations.
Example:
?- relation(authordoc:details, A).
A = [year, publications] .
So returned are only the attributes of a arel relation and no attributes of 
deeper arel relations.

*/
relation(Rel:ARels, AttrList) :-
  optimizerOption(nestedRelations),
  % No reverse call allowed like the subqueries extentions
  % wan't to do this.
  ( var(Rel)  ; var(ARels) ),
  throw(error_Internal(nr_relation(Rel:ARels, AttrList)::notAllowed)).

relation(Rel:ARels, AttrList2) :-
  optimizerOption(nestedRelations),
  dm(dbhandling,['\nTry: relation(',Rel:ARels,',', AttrList2,').']),
  databaseName(DB),
  ground(Rel),
  ground(ARels),
  storedRel(DB, Rel, AttrList),
  reduceToARel(AttrList, ARels, AttrList2). % Could be improved...
% NVK ADDED NR END

relation(Rel, AttrList) :-
  dm(dbhandling,['\nTry: relation(',Rel,',',AttrList,').']),
  atomic(Rel), % NVK ADDED NR
  databaseName(DB),
  storedRel(DB, Rel, AttrList).

readStoredRels :-
  retractall(storedRel(_, _, _)),
%  [storedRels].
  load_storefiles(storedRels).

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

:-assert(helpLine(showDatabaseSchema,0,[],
                  'Display schemas of all relations in currebr DB.')).

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
  % NVK MODIFIED NR
  %Member = ['OBJECT',Rel,_ | [[[rel | [[tuple | [AttrList]]]]]]],
  Member = ['OBJECT',Rel,_ | [[[RelType | [[tuple | [AttrList]]]]]]],
  member(RelType, [rel, nrel]),
  % NVK MODIFIED NR END
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
6.1 Printing the complete Catalog Info

*/


:- string_concat('List all database objects with their ',
  'internal and external spelling and their types.', X),
  assert(helpLine(showCatalog,0,[], X)).



showCatalog :-
  findall(_,
          ( secondoCatalogInfo(A,B,C,D),
            write_list([A, ' ', B,' ', C,' ', D]),
            nl
          ),
          _), !.


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

---- spelling(DCRel:DCAttr, IntAttr) :-
----

Returns the internal spelling ~IntAttr~ of dc-spelled attribute name ~DCAttr~.

~IntAttr~ is available via the dynamic predicate ~storedSpell/3~.
Otherwise, the catalog info is searched and the spelling is stored for further
use.

*/

% return the internal spelling for a attribute name given in dc-spelling
% --- getIntSpellingFromDCattrList(+DCAttr, +ExtAttrList, -IntAttr)
getIntSpellingFromDCattrList(_, [], _) :- !, fail.

getIntSpellingFromDCattrList(DCattr,[[ExtAttr, _] | _], IntAttr) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED
  dcName2externalName(DCattr,ExtAttr),
  internalName2externalName(IntAttr,ExtAttr), !.

getIntSpellingFromDCattrList(DCattr,[[_, _] | Rest], IntAttr) :-
  getIntSpellingFromDCattrList(DCattr,Rest,IntAttr),
  !.

/*
NVK ADDED NR
Note that an attribute name is unique within a nrel relation.
If this assumption is no longer valid, this is no longer possible.
Example:
?- spelling(orteh:subrel:kennzeichen, X).
X = kennzeichen.

*/
getIntSpellingFromDCattrList(FQN, [[_, Type] | _], IntAttr) :-
  optimizerOption(nestedRelations),
  Type = [arel, [tuple, ArelTypes]],
  catch(dcName2externalName(FQN, ExtDCNRel),
    error_Internal(database_dcName2externalName(_,_)::cannotTranslate), fail),
  internalName2externalName(_, ExtDCNRel),
  getIntSpellingFromDCattrList(_, ArelTypes, IntAttr),
  !.

% FQN
% e.g.
% Orte:Ort
% OrteH:SubRel:Kennzeichen
getIntSpellingFromDCattrList(FQN, [[ExtAttr, _] | _], IntAttr) :-
  % NVK: Now catch and ignore the exception to allow test the other attributes.
  catch(dcName2externalName(FQN,ExtAttr),
    error_Internal(database_dcName2externalName(_,_)::cannotTranslate), fail),
  internalName2externalName(IntAttr,ExtAttr), 
  !.

/*
Allows to execute ~spelling(orteh:subrel:ort, S)~ (result is S = ort) calls for nested relations.

*/
spelling(Rel:Atts, Spelled) :-
  optimizerOption(nestedRelations),
  dm(dbhandling,['\nTry: spelling(',Rel,':',Atts,',',Spelled,').']),
  databaseName(DB),
  ( storedSpell(DB, Rel:Atts, Spelled) % OK but spelled dosn't contain the
                                        % arel structure!
    ; (
        secondoCatalogInfo(Rel, _, _, TypeList),
        TypeList = [[RelType, [tuple, AttrList]]],
         % trel just added because it should work, too.
        member(RelType, [rel, nrel, trel]),
        getIntSpellingFromDCattrList(Atts, AttrList, Spelled), 
        !,
        assert(storedSpell(DB, Rel:Atts, Spelled))
      )
  ),
  !.
% NVK ADDED END

spelling(Rel:Attr, Spelled) :-
  \+ optimizerOption(nestedRelations),
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
  throw(error_Internal(database_spelling(DCR:DCA, Rext)::cannotTranslate)),
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
  throw(error_Internal(database_spelling(Rel, RelExt)::cannotTranslate)),
  fail.

/*
6.5 Storing And Loading of Spelling Information

*/
readStoredSpells :-
  retractall(storedSpell(_, _, _)),
%  [storedSpells].
  load_storefiles(storedSpells).

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

If this fails, a Secondo query is issued via ~getTupleInfo/1~, which also
inquires the cardinality and stores it in local memory.

*/

/*
NVK ADDED NR

*/
card(RelTerm, Size) :-
  optimizerOption(nestedRelations),
  nrCard(RelTerm, Size), 
  !.
% NVK ADDED NR END

card(DCrel, Size) :-
  dm(dbhandling,['\nTry: card(',DCrel,',',Size,').']),
  databaseName(DB),
  storedCard(DB, DCrel, Size),
  !.

card(DCrel, Size) :-
  dm(dbhandling,['\nTry: card(',DCrel,',',Size,').']),
  databaseName(DB),
  secondoCatalogInfo(DCrel,_,_,_),
  getTupleInfo(DCrel),
  storedCard(DB, DCrel, Size), !.

card(DCrel, X) :-
  write('\nERROR:\tCardinality for relation \''),write(DCrel), %'
  write('\' cannot be retrieved.'),nl,                        %'
  throw(error_Internal(database_card(DCrel, X)::cannotRetrieveCardinality)),
  fail.

/*
7.2 Storing And Loading Cardinalities

*/
readStoredCards :-
  retractall(storedCard(_, _, _)),
%  [storedCards].
  load_storefiles(storedCards).

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

/*
NVK ADDED NR
Currently there are no indicies for arel attributes or for subqueries as far as i know. This predicates is necessary, calling the below predicates won't work because they may throwing a exception.

*/
hasIndex(RelT, _, _, _) :-
  RelT=rel(Term, _),
  Term=..[irrel|_],
  !,
  fail.
% NVK ADDED NR END

hasIndex(Rel, Attr, IndexName, IndexType) :-
  ( ( Rel = rel(RelName, _),
      Attr = attr(AttrName, _, _),
      ground(RelName),
      ground(AttrName)
    )
    -> ( dm(dbhandling,['\nTry: hasIndex(',Rel,',',Attr,',',IndexName,',',
                 IndexType,').']),
         not(optimizerOption(noIndex)),
         hasIndex2(Rel, Attr, IndexName, IndexType)
      )
    ; ( my_concat_atom(['Unsufficient parameters: Uninitialized argument in ',
                     'hasIndex/4.'],'',ErrMsg),
        write_list(['\nERROR:\t',ErrMsg]),
        throw(error_Internal(database_hasIndex(Rel, Attr, IndexName, IndexType)
                        ::missingParameter::ErrMsg)),
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

Storing and reading of the dynamic predicates ~storedIndex/5~,
~storedNoIndex/3~, and ~storedIndexStat/5~ in the file ~storedIndexes~.

*/
readStoredIndexes :-
  retractall(storedIndex(_, _, _, _, _)),
  retractall(storedNoIndex(_, _, _)),
  retractall(storedIndexStat(_, _, _, _, _)),
%  [storedIndexes].
  load_storefiles(storedIndexes).

writeStoredIndexes :-
  open('storedIndexes.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredIndex(FD), _),
  findall(_, writeStoredNoIndex(FD), _),
  findall(_, writeStoredIndexStatistics(FD), _),
  close(FD).

writeStoredIndex(Stream) :-
  storedIndex(DB, U, V, W, X),
  write(Stream, storedIndex(DB, U, V, W, X)),
  write(Stream, '.\n').

writeStoredNoIndex(Stream) :-
  storedNoIndex(DB, U, V),
  write(Stream, storedNoIndex(DB, U, V)),
  write(Stream, '.\n').

writeStoredIndexStatistics(Stream) :-
  storedIndexStat(DB, DCindex, DCrel, U, V),
  write(Stream, storedIndexStat(DB, DCindex, DCrel, U, V)),
  write(Stream, '.\n').


:-
  dynamic(storedIndex/5),
  dynamic(storedNoIndex/3),
  dynamic(storedIndexStat/5),
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
variables, if this is required to match the index type expression.

If you want to add a physical index type, you just need to add an according fact
here. After this, you may define logical index types using the added physical
index type and define optimization/translation rules using the logical index
type(s).

You may also need to define syntax (predicates secondoOp/3 in file opsyntx.pl)
and translation rules (predicate plan\_to\_atom/2 in file optimizer.pl) for
special index operators.

*/

% Section:Start:indexType_2_b
% Section:End:indexType_2_b

indexType( btree,  [[btree|_]]  ).
indexType( hash,   [[hash|_]]   ).
indexType( rtree,  [[rtree|_]]  ).
indexType( rtree3, [[rtree3|_]] ).
indexType( rtree4, [[rtree4|_]] ).
indexType( rtree8, [[rtree8|_]] ).
indexType( mtree,  [mtree]      ).
indexType( xtree,  [xtree]      ).
indexType( invfile,[invfile]    ).


% Section:Start:indexType_2_e
% Section:End:indexType_2_e


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
  format('~2|~w~34|~w~44|~w~79|~n',
         [LogicalTypeExpr,LogicalIndexTypeCode,SupportedAttributeTypeList]).

showIndexTypes :-
  write('\nAvailable index types:\n'),
  format('~2|~w~34|~w~44|~w~79|~n',
         ['Index Type','Type Code','Supported Key Types']),
  write_list(['  --------------------------------------------------',
              '----------------------------']),nl,
  findall(_,showSingleIndexType,_),
  nl,!.

% Section:Start:logicalIndexType_8_b
% Section:End:logicalIndexType_8_b

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
    [ '__REL__', ' feed addid extend[ P: point2d( deftime(.',
      '__ATTR__', ') ) ] creatertree[ P ]' ],
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
    [ '__REL__', ' feed addid extend[ T: trajectory(.',
      '__ATTR__', ') ] creatertree[ T ]' ],
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
    [ '__REL__', ' feed addid extend[ B: box3d( bbox( trajectory(.',
      '__ATTR__', ') ), deftime( .',
      '__ATTR__', ') ) ] sortby[B asc] bulkloadrtree[ B ]' ],
    undefined,
    undefined,
    undefined).

logicalIndexType(sptmpuni, spatiotemporal(rtree3, unit), rtree3,
    [mpoint,mregion],
    [ '__REL__', ' feed projectextend[ ',
      '__ATTR__', ' ; TID: tupleid(.)] projectextendstream[TID; MBR: units(.',
      '__ATTR__', ') use[fun(U: upoint) bbox(U) ]] ',
      'sortby[MBR asc] bulkloadrtree[MBR]' ],
    undefined,
    undefined,
    undefined).

logicalIndexType(constuni, constuni(btree), btree,
    [uint,ustring,ubool,mint,mstring,mbool],
    [ '__REL__', ' feed projectextend[ ',
      '__ATTR__', 
      ' ; TID: tupleid(.)] projectextendstream[TID; ConstUnit: units(.',
      '__ATTR__', 
      ')] projectextend[TID; ConstVal: val(initial(.ConstUnit))] ',
      'sortby[ConstVal asc] createbtree[ConstVal]' ],
    undefined,
    undefined,
    undefined).

logicalIndexType(invfile, invfile, invfile, [rel|[tuple|_]],
    ['__REL__', 'createtrie[', '__ATTR__', ']'],
    undefined, undefined, undefined).

% Section:Start:logicalIndexType_8_e
% Section:End:logicalIndexType_8_e


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
  my_concat_atom([Rel, Small] ,'', RelName),
  replaceInPattern(RelName,Attr,CreateIndexPattern,
                   CreatePatternInstantiated),
  my_concat_atom(CreatePatternInstantiated, '', Query),
  my_concat_atom([Rel,Attr,LogicalIndexTypeCode],'_', IndexName2),
  my_concat_atom([IndexName2,Small], '', IndexName),
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
    ; ( my_concat_atom(['No database open: Cannot create Index.'],'',ErrMsg),
        write_list(['ERROR:\t',ErrMsg]),nl, !,
        throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
           CreateSmall, IndexName)::noDatabase::ErrMsg)),
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
    ;  ( my_concat_atom(['Invalid relation or attribute name: \'',LFRel,':',
                     LFAttr,'\'.'],'',ErrMsg),
         write_list(['ERROR:\t',ErrMsg]), nl, !,
         throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
               CreateSmall, IndexName)::unknownIdentifier::ErrMsg)),
         fail
       )
  ),
  % Check, whether the relation has an according attribute
  ( storedRel(DB, LFRel, AttrList)
    -> true
    ;  ( my_concat_atom(['Invalid relation name: \'',LFRel,'\'.'],'',ErrMsg),
         write_list(['ERROR:\t',ErrMsg]),nl, !,
         throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
            CreateSmall, IndexName)::unknownRelation::ErrMsg)),
         fail
       )
  ),
  ( member(LFAttr,AttrList)
    -> true
    ;  ( term_to_atom(AttrList,AttrListA),
         my_concat_atom(['Invalid attribute name: Relation \'',LFRel,
                      '\' does not have an attribute \'',LFAttr,'\'.\n',
                      'Available attributes are: ', AttrListA, '.'],'',ErrMsg),
         write_list(['ERROR:\t',ErrMsg]), nl, !,
         throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
            CreateSmall, IndexName)::unknownAttribute::ErrMsg)),
         fail
       )
  ),
  % Check whether an index of that type already exists for that attribute
  ( ( storedIndex(DB,LFRel,LFAttr,LogicalIndexType,DColdIndexName),
      dcName2externalName(DColdIndexName,ExtOldIndexName),
      CreateSmall \= yes                         % do not check for small index
    )
    -> ( term_to_atom(LogicalIndexType,LogicalIndexTypeA),
    -    my_concat_atom([
                     'Invalid index type: There already exists an index of ',
                      'type \'',LogicalIndexTypeA,'\' for ',LFRel,':',LFAttr,
                      ' called \'', ExtOldIndexName, '\'.'],'',ErrMsg),
         write_list(['ERROR:\t',ErrMsg]),nl, !,
         throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
            CreateSmall, IndexName)::objectAlreadyExists::ErrMsg)),
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
         findall(X,
                 keyAttributeTypeMatchesIndexType(LFRel,LFAttr,X),
                 AvailIndTypes),
         term_to_atom(AvailIndTypes,AvailIndTypesA),
         my_concat_atom(['Invalid index type: The index type \'',
                      LogicalIndexType,
                      '\' is unknown or does not match the type of \'',
                       LFRel, ':',LFAttr,'\'which is \'',Type,'\'.\n',
                       'You may create the following index types for this ',
                       'key:\n\t', AvailIndTypesA, '.'],'',ErrMsg),
         write_list(['ERROR:\t',ErrMsg]), nl,
         throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
             CreateSmall, IndexName)::wrongType::ErrMsg)),
         fail
       )
  ),
  % Check, whether a DB-object with the proper name already exists
  ( (   databaseObjectExists(IndexName)
      ; databaseObjectExistsDC(IndexName)
    )
    -> ( my_concat_atom(['Invalid index name: Object \'',IndexName,
                      '\' already exists.'],'',ErrMsg),
         write_list(['ERROR:\t',ErrMsg]), nl, !,
         throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
             CreateSmall, IndexName)::objectAlreadyExists::ErrMsg)),
         fail
       )
    ;  true
  ),
  % Check whether the according base relation exists
  my_concat_atom([Rel,Small],'',RelName),  % concat _small suffix, if specified
  ( databaseObjectExists(RelName)
    -> true
    ;  ( my_concat_atom(['Invalid relation name: \'',RelName,
                      '\'. Could not create the index.'],'',ErrMsg),
         write_list(['ERROR:\t',ErrMsg]),nl,!,
         throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
            CreateSmall, IndexName)::unknownRelation::ErrMsg)),
         fail
       )
  ),
  % All preconditions checked and OK. Build the index...
  write('NOTE:\tTrying to create the requested index...'),nl,
  my_concat_atom(['derive ', IndexName, ' = ', Query],'',CreationQuery), !,
  write('CreationQuery = '),write(CreationQuery),nl,
  ( secondo_direct(CreationQuery)
    -> true
    ;  ( my_concat_atom(['Secondo command failed: Tried to create an index.'],
           '', ErrMsg),
         write_list(['ERROR:\t',ErrMsg]),nl,!,
         throw(error_Internal(database_createIndex(LFRel, LFAttr,
             LogicalIndexType, 
             CreateSmall, IndexName)::execution_failed::ErrMsg)),
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
  % inquire statistics for the new index
  inquireIndexStatistics(DB,IndexName,
                         DCindexName,LFRel,LFAttr,LogicalIndexType),
  write('\tIndex \''),write(IndexName),write('\' has been created.'),nl,
  !.

createIndex(LFRel, LFAttr, LogicalIndexType, CreateSmall, IndexName) :-
  my_concat_atom(['Unknown error. Tried to create an index.'],'',ErrMsg),
  write_list(['ERROR:\t',ErrMsg]),nl,!,
  throw(error_SQL(database_createIndex(LFRel, LFAttr, LogicalIndexType,
         CreateSmall, IndexName)::unspecifiedError::ErrMsg)),
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
    'Create an index of a specified type over a given attribute for a relation.'
    )).

createIndex(DCrel,DCattr,LogicalIndexType) :-
  dm(dbhandling,['\nTry: createIndex(',DCrel,',',DCattr,',',LogicalIndexType,
                 ').']),
  ( isDatabaseOpen
    -> true
    ;  (  my_concat_atom(['No database open. Cannot create index.'],'',ErrMsg),
          throw(error_SQL(database_createIndex(DCrel,DCattr,LogicalIndexType)
                                      ::noDatabase::ErrMsg)),
         fail
       )
  ),
  ( ground(DCrel)
    -> true
    ;  ( my_concat_atom(['Unsufficient parameters: You are required to pass ',
                      'a relation name to create an index.'],'',ErrMsg),
         write_list(['\nERROR:\t',ErrMsg]), nl,
         !,
         throw(error_SQL(database_createIndex(DCrel,DCattr,LogicalIndexType)
                                      ::missingParameter::unspecifiedRelation)),
         fail
       )
  ),
  ( ground(DCattr)
    -> true
    ;  ( my_concat_atom(['Unsufficient parameters: You are required to pass ',
                      'an attribute name to create an index.'],'',ErrMsg),
         write_list(['\nERROR:\t',ErrMsg]), nl,
         !,
         throw(error_SQL(database_createIndex(DCrel,DCattr,LogicalIndexType)
                                             ::missingParameter::ErrMsg)),
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
         term_to_atom(AvailTypes,AvailTypesA),
         my_concat_atom(['Unsufficient parameters: Cannot create index.\n',
                     '--->\tYou are required to pass a logical index type.\n',
                     '--->\tAvailable logical index types for \'',ExtRel,
                     ':',ExtAttr,'\' are:\n','--->\t',AvailTypesA],'',ErrMsg),
         write_list(['\nERROR:\t',ErrMsg]), nl,
         !,
         throw(error_SQL(database_createIndex(DCrel,DCattr,LogicalIndexType)
                                             ::missingParameter::ErrMsg)),
         fail
       )
  ),
  createIndex(DCrel,DCattr,LogicalIndexType, no, _), !,
  updateCatalog.

createIndex(DCrel,DCattr,LogicalIndexType) :-
  my_concat_atom(['Unknown error: Cannot create index. Check whether relation ',
               'and attribute exist.'],'',ErrMsg),
  write_list(['\nERROR:\t',ErrMsg]), nl,!,
  throw(error_SQL(database_createIndex(DCrel,DCattr,LogicalIndexType)
                                                ::unspecifiedError::ErrMsg)),
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
  my_concat_atom(FragList, '_', ObjectName2),
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
    ;  my_concat_atom(['_',ExtDisambiguator],'',DisambiguatorSuffix)
  ),
  my_concat_atom([ExtRelName,ExtAttrName,TypeCode],'_',ObjectName2),
  my_concat_atom([ObjectName2,DisambiguatorSuffix,SmallSuffix],'',
                 ExtObjectName),
  !.

/*
----
    createSmallIndex(+DCRel,+DCAttr,+LogicalIndexType)
    createSmallIndex(+DCindex)
----

Creates a '\_small'-index for ~DCRel~:~DCAttr~ having type ~LogicalIndexType~.

Create '\_small'-index for an existing index ~DCindex~ (will also create the
according small relation, if it does not yet exists).

~DCRel~, ~DCAttr~, and  +DCindex are atomic terms respecting down-cased
spelling rules. ~LogicalIndexType~ is a term.

*/

createSmallIndex(DCindex) :-
  dm(dbhandling,['\nTry: createSmallIndex(',DCindex,').']),
  not(optimizerOption(entropy)), !.

createSmallIndex(DCindex) :-
  dm(dbhandling,['\nTry: createSmallIndex(',DCindex,').']),
  optimizerOption(entropy),
  % Check if a database is opened
  ( databaseName(DB)
    -> true
    ; ( my_concat_atom(['No database open: Cannot create small index for \'',
                     DCindex,'\'.'],'',ErrMsg),
        write_list(['ERROR:\t',ErrMsg]),nl, !,
        throw(error_Internal(database_createSmallIndex(DCindex)
              ::noDatabase::ErrMsg)),
        fail
      )
  ),
  % Check whether DCindex exists and is registered
  ( ( secondoCatalogInfo(DCindex,ExtIndex,_,_),
      storedIndex(DB,DCRel,DCAttr,LogicalIndexType,DCindex)
    )
    -> true
    ;  ( my_concat_atom(['Cannot build a \'_small\'- index for \'',
                          ExtIndex,'\': No such index registered.'],'',ErrMsg),
         write_list(['ERROR:\t',ErrMsg]),nl,!,
         throw(error_Internal(database_createSmallIndex(DCindex)
              ::noSuchObject::ErrMsg)),
         fail
       )
  ),
  % Get information on index type
  splitIndexObjectName(ExtIndex,_,AttrName,IndexCode,_,IsSmall),
  dcName2externalName(DCAttr, AttrName),
  % check if index is not already a small-index
  ( IsSmall = yes
    -> ( my_concat_atom(['Cannot build a \'_small\'- index for index \'',
                       ExtIndex,'\''],'',ErrMsg),
         write_list(['ERROR:\t',ErrMsg]),nl,!,
         throw(error_Internal(database_createSmallIndex(DCindex)
              ::invalidObjectName::ErrMsg)),
         fail
       )
    ; true
  ),
  logicalIndexType(IndexCode,LogicalIndexType,_,_,_,_,_,_),
  % create according small relation if necessary
  createSmallRelationObjectForRel(DCRel),
  % create the small index
  createSmallIndex(DCRel,DCAttr,LogicalIndexType),
  !.

createSmallIndex(DCRel,DCAttr,LogicalIndexType) :-
  dm(dbhandling,['\nTry: createSmallIndex(',DCRel,',',DCAttr,',',
                 LogicalIndexType,').']),
  (optimizerOption(entropy)
   -> catch( createIndex(DCRel, DCAttr, LogicalIndexType, yes, _),
             error_SQL(X),
            ( ( (   X = (database_createIndex(DCRel, DCAttr, LogicalIndexType,
                                            IndexName)::ErrorCode::Message)
                  ; X = (database_createIndex(DCRel, DCAttr, LogicalIndexType,
                                                    IndexName)::ErrorCode)
                ),
                member(ErrorCode,[objectAlreadyExists])
              )
              -> ( write_list(['\nINFO:\tSmall index already exists.']), nl )
              ;  ( bound(Message)
                   -> throw(error_Internal(database_createIndex(DCRel, DCAttr,
                              LogicalIndexType,IndexName)::ErrorCode::Message))
                   ;  throw(error_Internal(database_createIndex(DCRel, DCAttr,
                              LogicalIndexType,IndexName)::ErrorCode))
                 )
            )
          )
   ;  true
  ),
  !.

% a wrapper that will create the index only if it is not already present:
createSmallIndexForIndex(DCindex) :-
  dm(dbhandling,['\nTry: createSmallIndexForIndex(',DCindex,').']),
  my_concat_atom([DCindex,'_small'],'',DCindexSmall),
  (   secondoCatalogInfo(DCindexSmall,_,_,_)
    ; ( createSmallIndex(DCindex), updateCatalog )
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
    ; ( my_concat_atom(['No database open: Cannot drop index \'',
                     ExtIndexName,'\'.'],'',ErrMsg),
        write_list(['ERROR:\t',ErrMsg]),nl, !,
        throw(error_SQL(database_dropIndex(ExtIndexName)::noDatabase::ErrMsg)),
        fail
      )
  ),
  splitIndexObjectName(ExtIndexName,RelName,AttrName,_,_,IsSmall),
  % check if index is not a small-index
  ( IsSmall = yes
    -> ( my_concat_atom(['Cannot drop \'_small\'- index \'',
                      ExtIndexName,'\' manually.'],'',ErrMsg),
         write_list(['ERROR:\t',ErrMsg]),nl,!,
         throw(error_SQL(database_dropIndex(ExtIndexName)
                                        ::prohibitedAction::ErrMsg)),
         fail
       )
    ; true
  ),
  % check if index exists
  ( databaseObjectExists(ExtIndexName)
    -> true
    ;  ( my_concat_atom(['Unknown object: Index \'',ExtIndexName,
                      '\' does not exist'],'',ErrMsg),
         write_list(['ERROR:\t',ErrMsg]),nl,!,
         throw(error_SQL(dropIndex(ExtIndexName)::unknownIndex::ErrMsg)),
         fail
       )
  ),
  % remove the index object
  my_concat_atom(['delete ', ExtIndexName],'',DeleteIndexAtom),
  secondo_direct(DeleteIndexAtom),
  % update storedIndex/5
  dcName2externalName(DCRel,RelName),            % get DCReal
  % dcName2externalName(DCRel:DCAttr, AttrName), % original code
  dcName2externalName(DCAttr, AttrName),         % modified code
  dcName2externalName(DCindexName,ExtIndexName),
  retractall(storedIndex(DB,DCRel,DCAttr,_,DCindexName)), % retract info
  retractall(storedIndexStat(DB,DCindexName,_,_,_)),      % retract statistics
  % update storedNoIndex/3
  ( storedIndex(DB,DCRel,DCAttr, _, _)
    -> true
    ;  assert(storedNoIndex(DB,DCRel,DCAttr))
  ),
  % possibly remove small-index
  my_concat_atom([ExtIndexName,'_small'],'',ExtSmallIndexName),
  ( databaseObjectExists(ExtSmallIndexName)
    -> ( write('NOTE:\tAlso dropping the _small-index \''),         %'
         write(ExtSmallIndexName), write('\'.'),nl,                 %'
         my_concat_atom(['delete ', ExtSmallIndexName],'',DeleteSmallIndexAtom),
         secondo_direct(DeleteSmallIndexAtom)
       )
    ;  true
  ),
  write('NOTE:\tIndex \''), write(ExtIndexName), write('\' has been dropped.'),
  nl,
  !.

dropIndex(ExtIndexName) :-
  my_concat_atom(['Unknown error: Tried to drop index \'',
               ExtIndexName,'\'.'],'',ErrMsg),
  write_list(['ERROR:\t',ErrMsg]),nl,!,
  throw(error_SQL(database_dropIndex(ExtIndexName)::unspecifiedError::ErrMsg)),
  fail.

dropIndex(DCRel,DCAttr,LogicalIndexType) :-
  dm(dbhandling,['\nTry: dropIndex(',DCRel,',',DCAttr,',',
                 LogicalIndexType,').']),
  % create index name
  ( not( (ground(DCRel), ground(DCAttr), ground(LogicalIndexType),
          atomic(DCRel), atomic(DCAttr) )
       )
    -> ( my_concat_atom(['Internal error: Unsufficient parameters.'],'',ErrMsg),
         write('ERROR:\tYou need to specify all 3 arguments of dropIndex/3.'),
         nl,
         throw(error_SQL(dropIndex(DCRel,DCAttr,LogicalIndexType)
                                           ::missingParameter::ErrMsg)),
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
  my_concat_atom(['Unknown error: Tried to drop an index.'],'',ErrMsg),
  write_list(['ERROR:\t',ErrMsg]),nl,!,
  throw(error_SQL(database_dropIndex(LFRel,LFAttr,LogicalIndexType)
                 ::unspecifiedError::ErrMsg)),
  fail.

/*
---- showIndexes
----

This predicate shows a list of all registered indexes present within the
currently opened database.

*/
:-assert(helpLine(showIndexes,0,[],
         'List information on available indexes within the current DB.')).

showIndexes :-
  (databaseName(DB)
   -> ( write_list(['\n\nOverview on all Indexes available in database \'',
                    DB,'\':']),nl,
        format('~w~35|~w~69|~w~n',
               ['Index Name', 'Relation:Attribute', 'Logical Index Type']),
        write_list(['---------------------------------------------------',
                    '------------------------------------']),
        nl,
        findall(_,
                ( storedIndex(DB,DCrel,DCattr,IndexType,DCindexName),
                  dcName2externalName(DCrel,ExtRelName),
                  dcName2externalName(DCrel:DCattr,ExtAttrName),
                  dcName2externalName(DCindexName,ExtIndexName),
                  format('~w~35|~w~w~w~69|~w~n',
                         [ExtIndexName, ExtRelName, ':',ExtAttrName,IndexType])
                ),
                _)
      )
    ; (write_list(['\nWARNING:\tCannot list indexes. No database open.']), nl)
  ),
  nl, nl, !.

/*
8.5 Inquiring and Managing Metadata on Indexes

The following predicates are used to inquire and manage metadata on indexes of
various types.

Metadata are stored using dynamic facts

---- storedIndexStat(DB, DCIndexName, DCRelName, KeyType, KeyData)
----

~KeyType~ is a keyword (identifier) indicating the meaning of the maintained
information, like ~height~ (height of a tree structure), ~entries~ (number of
stored entries), ~nodes~ (number of nodes in a tree structure), ~bbox~ (total MBR
for the indexed keys), ~type~ (data type of the keys), ~double~ (whether double
indexing is used with that index), ~dim~ (dimension of entry values).

~KeyData~ is the stored metadata, which may be a term (e.g. for ~bbox~) or an
atom.

Between optimizer sessions, the metadata is stored in the file
~storedIndexes.pl~ on disk.

When a database is open, predicate

---- getIndexStatistics(+DCindexName, +KeyName, ?DCrelName, ?KeyValue)
----

can be used to access the statistics.

Use predicate

---- inquireIndexStatistics(+DCindexName)
----

to inquire and update the index statistics for index ~DCindexName~.

*/

% index stats already known
getIndexStatistics(DCindexName, KeyName, DCrelName, KeyValue) :-
  ground(DCindexName),
  atomic(DCindexName),
  databaseName(DB),
  storedIndexStat(DB, DCindexName, DCrelName, KeyName, KeyValue),
  !.

% no index stats available!
getIndexStatistics(DCindexName, _, _, _) :-
  ground(DCindexName),
  atomic(DCindexName),
  databaseName(_),
  fail,
  !.

getIndexStatistics(DCindexName, KeyName, DCrelName, KeyValue) :-
  throw(error_Internal(getIndexStatistics(DCindexName, KeyName, DCrelName,
                 KeyValue)::unknownError)),
  !.

% calculateBBoxSize(+Dimension,+BBox,-BoxSize)
calculateBBoxSize([],BoxSize) :-
  BoxSize is 1.0, !.

calculateBBoxSize([Min, Max | More],BoxSize) :-
  calculateBBoxSize(More,MoreBoxSize),
  BoxSize is MoreBoxSize * (Max - Min), !.

calculateBBoxSize(Box, BoxSize) :-
  my_concat_atom(['Cannot calculate bbox size,'],'',ErrMsg),
  throw(error_Internal(calculateBBoxSize(Box, BoxSize)
                                          ::unspecifiedError::ErrMsg)).


% wrapper for call by index name only
%   inquireIndexStatistics(+DCindexName)
inquireIndexStatistics(DCindexName) :-
  databaseName(DB),
  ground(DCindexName),
  secondoCatalogInfo(DCindexName,ExtIndexName,_,_),
  storedIndex(DB, DCrel, DCattr, LogicalTypeExpr, DCindexName),
  logicalIndexType(_, LogicalTypeExpr, PhysIndexType,
                      SupportedAttributeTypeList,
                      _, _, _, _),
  % BEGIN --- REMOVE THIS - just avoid warnings
  PhysIndexType = PhysIndexType,
  SupportedAttributeTypeList = SupportedAttributeTypeList,
  % END ---- REMOVE THIS - just avoid warnings
  inquireIndexStatistics(DB,ExtIndexName,DCindexName,DCrel,
                         DCattr,LogicalTypeExpr),
  !.

inquireIndexStatistics(DCindexName) :-
  my_concat_atom(['Cannot collect index statistics,'],'',ErrMsg),
  throw(error_Internal(inquireIndexStatistics(DCindexName)
                                          ::unspecifiedError::ErrMsg)).

% Main predicate
%   inquireIndexStatistics(+DB,+ExtIndexName,+DCindexName,
%                          +DCrel,+DCattr,+LogicalTypeExpr)
inquireIndexStatistics(DB,ExtIndexName,DCindexName,
                       DCrel,DCattr,LogicalTypeExpr) :-
  ground(DB),
  ground(ExtIndexName),
  ground(DCindexName),
  ground(DCrel),
  ground(DCattr),
  ground(LogicalTypeExpr),
  logicalIndexType(_, LogicalTypeExpr, PhysIndexType, _, _, _, _, _),
  retractall(storedIndexStat(DB,DCindexName,_,_,_)), % drop old statistics
  !,
  ( memberchk([PhysIndexType,Dimension],
                [[rtree,2],[rtree3,3],[rtree4,4],[rtree8,8]])
    *-> ( % some rtree index
          my_concat_atom(['query ',ExtIndexName],'',Query),
          secondo(Query,ResList),
          ( ResList = [[PhysIndexType, [tuple, _], KeyType, Double],
                       [_, [_, Height], [_, Entries], [_, Nodes], [_, BBox]]]
            -> (
                 calculateBBoxSize(BBox,BoxSize),
                 assert(storedIndexStat(DB,DCindexName,DCrel,height,Height)),
                 assert(storedIndexStat(DB,DCindexName,DCrel,entries,Entries)),
                 assert(storedIndexStat(DB,DCindexName,DCrel,nodes,Nodes)),
                 assert(storedIndexStat(DB,DCindexName,DCrel,bbox,BBox)),
                 assert(storedIndexStat(DB,DCindexName,DCrel,type,KeyType)),
                 assert(storedIndexStat(DB,DCindexName,DCrel,double,Double)),
                 assert(storedIndexStat(DB,DCindexName,DCrel,dim,Dimension)),
                 assert(storedIndexStat(DB,DCindexName,DCrel,boxsize,BoxSize)),
                 ( ( my_concat_atom(['query getFileInfo(',ExtIndexName,')'],'',
                                 Query2),
                     secondo(Query2,[text,ValueAtom]),
                     term_to_atom([ValueList],ValueAtom),
                     % analyze the result term
                     assertFileStats(DB,DCindexName,DCrel,ValueList)
                   )
                   -> true
                   ;  (  % List error
                        dm(dbhandling,['Wrong result list format: ',ResList,
                                       '\n']),
                        my_concat_atom(['Wrong result list'],'',ErrMsg),
                        throw(error_Internal(inquireIndexStatistics(DCindexName)
                                   ::unspecifiedError::ErrMsg))
                      )
                ),
                write_list(['INFO:\tSuccessfully inquired statistics on ',
                            'index \'',ExtIndexName,'\'.']),nl
               )
            ;  (  % List error
                  dm(dbhandling,['Wrong result list format: ',ResList,'\n']),
                  my_concat_atom(['Wrong result list'],'',ErrMsg),
                  throw(error_Internal(inquireIndexStatistics(DCindexName)
                                   ::unspecifiedError::ErrMsg))
               )
          )
    ) ; true
  ),!,
  ( PhysIndexType = btree
    *-> ( % btree index
          ( my_concat_atom(['query getFileInfo(',ExtIndexName,')'],'',Query),
            secondo(Query,[text,ValueAtom]),
            term_to_atom([ValueList],ValueAtom),
            % analyze the result term
            assertFileStats(DB,DCindexName,DCrel,ValueList)
          ) -> true
            ;  (  % List error
                  dm(dbhandling,['Wrong result list format: ',ResList,'\n']),
                  my_concat_atom(['Wrong result list'],'',ErrMsg),
                  throw(error_Internal(inquireIndexStatistics(DCindexName)
                                   ::unspecifiedError::ErrMsg))
               )
       ) ; true
  ),!,
  ( PhysIndexType = hash
    *-> ( % hashtable index
          ( my_concat_atom(['query getFileInfo(',ExtIndexName,')'],'',Query),
            secondo(Query,[text,ValueAtom]),
            term_to_atom([ValueList],ValueAtom),
            % analyze the result term
            assertFileStats(DB,DCindexName,DCrel,ValueList)
          ) -> true
            ;  (  % List error
                  dm(dbhandling,['Wrong result list format: ',ResList,'\n']),
                  my_concat_atom(['Wrong result list'],'',ErrMsg),
                  throw(error_Internal(inquireIndexStatistics(DCindexName)
                                   ::unspecifiedError::ErrMsg))
               )
       ) ; true
  ),!,
  ( PhysIndexType = mtree
    *-> ( % mtree index
         true % no statistics available/ not yet implemented
       ) ; true
  ),!,
  ( PhysIndexType = xtree
    *-> ( % xtree index
         true % no statistics available/ not yet implemented
       ) ; true
  ),
  !.

inquireIndexStatistics(DB,DCindexName,DCrel,DCattr,LogicalTypeExpr) :-
  my_concat_atom(['Cannot collect index statistics.'],'',ErrMsg),
  throw(error_Internal(inquireIndexStatistics(DB,DCindexName,DCrel,DCattr,
                                  LogicalTypeExpr)::unspecifiedError::ErrMsg)).

%% assert the (key,value) pairs as storedIndexStats/5 facts
%% assertFileStats(+DB,+DCindexName,+DCrel,+PairList)
assertFileStats(_,_,_,[]) :- !.
assertFileStats(DB,DCindexName,DCrel,[[[Key],[Value]]|MoreElems]) :-
  downcase_atom(Key,KeyDC),
  ( (KeyDC = filename ; KeyDC = filepurpose)
    -> ( my_string_to_atom(ValueStr,Value) , string_to_list(ValueStr,ValueDC) )
    ; downcase_atom(Value,ValueDC)
  ),
  assert(storedIndexStat(DB,DCindexName,DCrel,KeyDC,ValueDC)), !,
  assertFileStats(DB,DCindexName,DCrel,MoreElems).


/*
---- showIndexStatistics
----

This predicate lists all index statistics available in the knowledge base

*/

:- assert(helpLine(showIndexStatistics,0,
    [],
    'List index statistics on current DB.')).

showIndexStatistics :-
  databaseName(DB),
  write_list(['\nStatistics on Indexes for Database \'',DB,'\':\n']),
  format('  ~w~40|~w~75|~w~n',['Index Name','Statistic Name',
         'Statistic Value']),
  write('------------------------------------------------------'),
  write('-----------------------------------------------------\n'),
 findall(_,showIndexStatistics1(DB),_).

showIndexStatistics1(DB) :-
  storedIndexStat(DB,DCindexName,_,Key,Entry),
  ( catch(string_to_list(EntryP, Entry), _, fail)
    -> true
    ;  EntryP = Entry
  ),
  format('  ~w~40|~w~75|~w~n',[DCindexName,Key,EntryP]).

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

% relUsedByTerm(+Rel, +Term)
% --- succeeds, iff relation Rel is used within Term.

relUsedByTerm(Rel, Rel:_) :- !.
relUsedByTerm(Rel, Rel2:_) :- Rel \= Rel2, !, fail.
relUsedByTerm(_,[]) :- !, fail.
relUsedByTerm(Rel,[X|_]) :- relUsedByTerm(Rel,X), !.
relUsedByTerm(Rel,[_|Y]) :- relUsedByTerm(Rel,Y), !.
relUsedByTerm(Rel, Term) :-
  compound(Term),
  not(is_list(Term)),
  Term =.. [_|Args],
  relUsedByTerm(Rel, Args), !.

% retract all selectivities, bbox-selectivities, avg-bbox-sizes, predicate
% signatures for a given relation ~DCRel~ in the currect database
retractPredStats(DCRel) :-
  databaseName(DB),
  findall(_,retractPredStat(DB,DCRel),_).

retractPredStat(DB,Rel) :-
  (   storedSel(DB, Term, _)                  % selectivities
    ; storedBBoxSel(DB, Term, _)              % bbox-selectivities
    ; storedBBoxSize(DB, Term, _)             % avg-bbox-sizes
    ; storedBBoxSize(DB, Term, _)             % avg-bbox-sizes
    ; storedPET(DB, Term, _, _)               % PETs
    ; storedPredicateSignature(DB, Term, _)   % predicate signatures
  ),
  relUsedByTerm(Rel, Term),
  retractall(storedSel(DB, Term, _)),         % selectivities
  retractall(storedBBoxSel(DB, Term, _)),     % bbox-selectivities
  retractall(storedBBoxSize(DB, Term, _)),    % avg-bbox-sizes
  retractall(storedPET(DB, Term, _, _)),      % PETs
  retractall(storedPredicateSignature(DB, Term, _)). % predicate signatures

%
retractStoredInformation(DCrel) :-
  dm(dbhandling,['\nTry: retractStoredInformation(',DCrel,').']),
  databaseName(DB),
  getSampleSname(DCrel, SampleS),
  getSampleJname(DCrel, SampleJ),
  getSmallName(DCrel,Small),
  retractPredStats(DCrel),
  retractall(storedOrderings(DB, DCrel, _)),
  retractall(storedCard(DB, DCrel, _)),
  retractall(storedCard(DB, DCrel:_, _)), % NVK ADDED
  retractall(storedCard(DB, SampleS, _)),
  retractall(storedCard(DB, SampleJ, _)),
  retractall(storedCard(DB, Small, _)),
  retractall(storedAttrSize(DB, DCrel, _, _, _, _, _)),
  retractall(storedTupleSize(DB, DCrel, _, _, _)),
  retractall(storedSpell(DB, DCrel, _)),
  retractall(storedSpell(DB, DCrel:_, _)),
  retractall(storedSpell(DB, SampleS, _)),
  retractall(storedSpell(DB, SampleJ, _)),
  retractall(storedSpell(DB, Small, _)),
  retractall(storedRel(DB, DCrel, _)),
  retractall(storedIndex(DB, DCrel, _, _, _)),
  retractall(storedNoIndex(DB, DCrel, _)),
  retractall(storedIndexStat(DB, _, DCrel, _, _)),
  write_list(['\nINFO:\tRetracted all information on relation \'', DCrel,
              '\' and ', 'all according small and sample objects.']),nl,
  !.

check_and_delete(File) :-
  exists_file(File),
  !,
  delete_file(File).

check_and_delete(_).

resetKnowledgeDB :-
  check_and_delete('storedAttrSizes.pl'),
  check_and_delete('storedCards.pl'),
  check_and_delete('storedIndexes.pl'),
  check_and_delete('storedOrderings.pl'),
  check_and_delete('storedPETs.pl'),
  check_and_delete('storedRels.pl'),
  check_and_delete('storedSels.pl'),
  check_and_delete('storedSpells.pl'),
  check_and_delete('storedTupleSizes.pl'),
  retractall(storedOrderings(_, _, _)),
  retractall(storedCard(_, _, _)),
  retractall(storedAttrSize(_, _, _, _, _, _, _)),
  retractall(storedTupleSize(_, _, _, _, _)),
  retractall(storedSpell(_, _, _)),
  retractall(storedRel(_, _, _)),
  retractall(storedIndex(_, _, _, _, _)),
  retractall(storedNoIndex(_, _, _)),
  retractall(storedIndexStat(_, _, _, _, _)),
  retractall(storedPET(_, _, _, _)),
  retractall(storedSel(_, _, _)),
  retractall(storedPredicateSignature(_, _, _)),
  retractall(storedBBoxSize(_, _, _)),
  (databaseName(_) -> updateCatalog ; true),
  write_list(['\nINFO: All information has been reset.']), nl.

updateRel(Rel) :-
  ( not( (ground(Rel), atomic(Rel)) )
    -> ( write_list(['\nERROR:\tupdateRel/1 requires a concrete relation name ',
                     'as argument, but gets \'', Rel, '\'.']),
         fail
       )
    ;  ( true )
  ),
  dcName2externalName(DCRel,Rel),
  dcName2externalName(DCRel,ExtRel),
  write_list(['\nINFO:\tupdateRel(', Rel, ') retracts all information on \'',
              ExtRel, '\'...']),
%  retractStoredInformation(DCRel), % original code
  handleLostRelation(DCRel),        % new code - will also remove samples etc.
  write_list(['\nINFO:\tupdateRel(', Rel, ') re-collects basic information ',
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
         retractall(storedTupleSize(DB, _, _, _, _)),
         retractall(storedSpell(DB, _, _)),
         retractall(storedRel(DB, _, _)),
         retractall(storedIndex(DB, _, _, _, _)),
         retractall(storedNoIndex(DB, _, _)),
         retractall(storedIndexStat(DB, _, _, _, _)),
         retractall(storedPET(DB, _, _, _)),
         retractall(storedSel(DB, _, _)),
         retractall(storedPredicateSignature(DB, _, _)),
         retractall(storedBBoxSize(DB, _, _)),
         write_list(['\nINFO: All information on database \'', DB, '\' has ',
                     'been retracted.'])
       )
    ;  ( write_list(['\nERROR:\tYou may not use updateDB/1 on the currently ',
                      'opened database!']), nl
       )
  ),
  !.



/*
10 Inquiring on Cardinalities, Tuple Sizes, Attribute Types and Attribute Sizes

---- getTupleInfo(+DCrel)
----

Analyses the relation schema for relation ~DCrel~ and sends a query to Secondo
to inquire the relation's cardinality, the size information for tuples,
detailed information on attribute sizes, the attribute types, the the spelling.

For each attribute, the type and size information is stored in a asserted facts

----
  storedAttrSize(Database, Rel, Attr, Type, MemSize, CoreSize, LOBSize)
----

~MemSize~ is the attribute's minimum memory size.
~CoreSize~ is the attribute's fixed size within a tuple's root record on disk,
~LOBSize~ is the attribute's average variable LOB size of the part of it's FLOB
data, that is kept within the relation's dedicated FLOB file.

Information on cardinality is asserted in facts

---- storedCard(DB, DCrel, Card)
----

Information on tuple size is stored in asserted facts

---- storedTupleSize(DB, DCrel, MemSize, CoreSize, LOBSize),
----

*/

% The main predicate:
getTupleInfo(DCrel) :-
  dm(dbhandling,['\nTry: getTupleInfo(',DCrel,').']),
  ( databaseName(DB)
    -> true
    ;  ( my_concat_atom(['\nERROR:\tCannot get tuple information for \'',DCrel,
                     '\': No database open.'],'',ErrMsg),
         write(ErrMsg), nl,
         throw(error_Internal(database_getTupleInfo(DCrel)
              ::noDatabaseOpen::ErrMsg)),
         fail, !
       )
  ),
  ( ( secondoCatalogInfo(DCrel,ExtRel, _, TypeExpr),
      (   TypeExpr = [[rel, [tuple, ExtAttrList]]]
        ; TypeExpr = [[trel, [tuple, ExtAttrList]]]
         % NVK ADDED recognize nested relations.
        ; TypeExpr = [[nrel, [tuple, ExtAttrList]]]
      ),
      internalName2externalName(IntRel,ExtRel)
    )
    -> true
    ;  ( my_concat_atom(['ERROR:\tCannot retrieve tuple information on ',
                      'relation \'', DCrel, '\':\n',
                     '--->\tNo matching relation found in catalog.'],'',ErrMsg),
         write(ErrMsg), nl,
         throw(error_Internal(database_getTupleInfoDCrel)::lostObject::ErrMsg),
         fail, !
       )
  ),
  % retract current information
  retractall(storedCard(DB,DCrel,_)),
  retractall(storedCard(DB,DCrel:_,_)), % NVK ADDED
  retractall(storedAttrSize(DB,DCrel,_,_,_,_,_)),
  retractall(storedTupleSize(DB,DCrel,_,_,_)),
  retractall(storedRel(DB,DCrel,_)),
  retractall(storedSpell(DB,DCrel,_)),     % spelling of relation
  retractall(storedSpell(DB,_,IntRel)),    % spelling of relation
  retractall(storedSpell(DB, DCrel:_, _)), % spelling of attributes
  assert(storedSpell(DB,DCrel,IntRel)),    % XRIS: could be omitted!
  % query for new information
	% NVK MODIFIED NR 
  	getTupleInfo2NR(DB, ExtRel, DCrel, ExtAttrList, DCattrList),
	% NVK MODIFIED END NR
  assert(storedRel(DB,DCrel,DCattrList)),  % Doing this as the last step avoids
                                           % problems with missing data
  !.

/*
----
getTupleInfo2(+DB, +ExtRel, +DCrel, +ExtAttrList, -DCattrList)
----

Will inquire and assert size and type data on attributes.

*/

% NVK ADDED NR
/*
The DCattrList is no longer in order like within the catalog.
This is because vor every arel and the top level attributes of the relation a
~TupleInfoQuery~ is executed. 

*/

getTupleInfo2NR(DB, ExtRel, DCrel, ExtAttrList, DCattrList) :-
  % Get the top level information
  getTupleInfo2(DB, ExtRel, DCrel, [], ExtAttrList, DCattrList1),
  % Process all arel attributes recursivly.
   getTupleInfo2ARels(DB, ExtRel, DCrel, [], ExtAttrList, DCattrList2),
   append(DCattrList1, DCattrList2, DCattrList).

getTupleInfo2ARels(_, _, _, _, [], []) :-
  !.

getTupleInfo2ARels(DB, ExtRel, DCrel, ExtARelPath, ExtAttrList, DCattrList) :-
  ExtAttrList=[[ExtAttr, TYPE]|Rest],
  getTupleInfo2ARels(DB, ExtRel, DCrel, ExtARelPath, Rest, DCattrList1),
  ( TYPE = [arel,[tuple,ExtArelAtts]] -> 
    (
    append(ExtARelPath, [ExtAttr], NewExtARelPath),
    getTupleInfo2(DB, ExtRel, DCrel, NewExtARelPath, ExtArelAtts, DCattrList2),
    getTupleInfo2ARels(DB, ExtRel, DCrel, NewExtARelPath, ExtArelAtts, 
      DCattrList3),
    appendLists([DCattrList1, DCattrList2, DCattrList3], DCattrList)
    ) 
  ;
    DCattrList1=DCattrList
  ).

getTupleInfo2(DB,ExtRel,DCrel, ExtARelPath, ExtAttrList, DCAttrList) :-
  dm(dbhandling,['\nTry: getTupleInfo2(',DB,',',ExtRel,',',DCrel,',',
    ExtARelPath,',', ExtAttrList,',',DCAttrList,').']),
  getTupleInfoQuery(ExtRel,ExtARelPath, ExtAttrList, DCAttrList,TupleInfoQuery),
  secondo(TupleInfoQuery, TupleInfoQueryResultList),
  % NVK ADDED
  ( TupleInfoQueryResultList = [[trel, [tuple, ResultTupleAtts]], [ResultTuple]]
    -> true
    ;  ( write('TupleInfoQuery = '),
         writeln(TupleInfoQuery),
         write('TupleInfoQueryResultList = '),
         writeln(TupleInfoQueryResultList),
         my_concat_atom(['Wrong result list'],'',ErrMsg),
         throw(error_Internal(database_getTupleInfo2(DB,ExtRel,DCrel,
                                  ExtAttrList,DCAttrList)::wrongType::ErrMsg))
       )
  ),
  analyseTupleInfoQueryResultList(DB, DCrel, ExtARelPath, ExtAttrList, 
    ResultTupleAtts, ResultTuple),
  !.

/*
---- getTupleInfoQuery(+ExtRel, +ARelPath, +ExtAttrList,-DCAttrList, 
       -TupleInfoQuery)
----

Creates a complete tuple info query from an ExtensionListExpression

*/

getTupleInfoQuery(ExtRel, ARelPath, ExtAttrList, DCAttrList, TupleInfoQuery):-
  dm(dbhandling,['\nTry: getTupleInfoQuery(',ExtRel,',',ARelPath,',',
    ExtAttrList,',', DCAttrList,',',TupleInfoQuery,').']),
  getTupleInfoQuery2(ExtRel, ARelPath, ExtAttrList,DCAttrList, ExtensionList),
  buildUnnestAtom(ARelPath, ARelUnnestAtom),
  secondoCatalogInfo(_, ExtRel, _, Type),
  ( Type = [[rel, _]]      % simple relation
    -> ( TupleFeed = ExtRel, 
         atomic_list_concat(
           [ExtRel, ' sample[500; 0.000001] tconsume'], '', SmallRelation)
       )
    ;                      % nested relation
       ( atomic_list_concat([ExtRel, ' feed ', ARelUnnestAtom], '', TupleFeed),
         atomic_list_concat(
           [TupleFeed, ' head[500] tconsume'], '', SmallRelation)
       )
  ),
  atomic_list_concat([
    '\nquery ', SmallRelation, ' within[fun(therelation: ANY)',
    '\n  1 feed transformstream projectextend[; ',
    'Cardi_nality: (',TupleFeed,' count), ',
    '\n  Tuple_TotalSize: (therelation feed tuplesize), ',
    '\n  Tuple_CoreSize: (therelation feed exttuplesize), ',
    '\n  Tuple_LOBSize: ((therelation feed tuplesize)', 
           ' - (therelation feed exttuplesize)), ',
    ExtensionList,' ] tconsume ]'], '', TupleInfoQuery),
  write_list(['\n\nRES: ', getTupleInfoQuery(ExtRel,ARelPath, ExtAttrList,
    DCAttrList,TupleInfoQuery),'\n\n']),
  !.

buildUnnestAtom([], '') :- !.
buildUnnestAtom([ARel|ARelPath], Out) :-
  buildUnnestAtom(ARelPath, Out1),
  my_concat_atom(['project[', ARel, '] unnest[', ARel, '] ', Out1], Out).


% getTupleInfoQuery2(+ExtRel,+ExtAttrList,-DCattrList,-Extension)
% Concatenates an ExtensionList to an ExtensionListExpression
%    getTupleInfoQuery2(+ExtRel,+ExtAttrList,-DCattrList,-Extension)
getTupleInfoQuery2(ExtRel,ARelPath, ExtAttrList,DCattrList,Extension):-
  dm(dbhandling,['\nTry: getTupleInfoQuery2(',ExtRel,',',ARelPath,',',
    ExtAttrList,',', DCattrList,',',Extension,').']),
  getTupleInfoQuery3(ExtRel,ARelPath,ExtAttrList,DCattrList, ExtensionList),
  my_concat_atom(ExtensionList,', ',Extension),
  !.

% getTupleInfoQuery(+ExtRel,+ExtAttrList,-DCattrList,-ExtensionList)
% creates an ExtensionList from the ExtensionList.
%   getTupleInfoQuery3(+ExtRel,+ExtAttrList,-AttrDClist,-AttrExtensionList)
% NVK MODIFIED
%getTupleInfoQuery3(_,[],[],[]):- !.
getTupleInfoQuery3(_,_,[],[],[]):- !.

getTupleInfoQuery3(R,ARelPath, AttrList,AttrDClist, AttrExtensionList):-
  dm(dbhandling,['\nTry: getTupleInfoQuery3(',R,',',ARelPath,',',AttrList,',',
    AttrDClist,',',AttrExtensionList,')']),
  AttrList = [[A,_]|MoreAttrs],
  getTupleInfoQuery3(R,ARelPath,MoreAttrs,MoreDCAttrs, MoreAttrExtensions),
  dcName2externalName(AttrDC, A),
  downcaseList(ARelPath, DCARelPath),
  append(DCARelPath, [AttrDC], T),
  appendAttributeList(T, AttrDCPath),
  % buildUnnestAtom(ARelPath, ARelUnnestAtom),
  % atomic_list_concat([R, ' feed', ARelUnnestAtom], '', TupleFeed),
  my_concat_atom([
     '\n  ', A,'_c: (therelation feed extattrsize[',A,']), ',
     '\n  ', A,'_l: ((therelation feed attrsize[',A,
          ']) - (therelation feed extattrsize[',A,']))'
    ],'',AttrExtension),
  AttrDClist        = [AttrDCPath|MoreDCAttrs],
  AttrExtensionList = [AttrExtension|MoreAttrExtensions],
  !.



/*
---- analyseTupleInfoQueryResultList(+DB,+DCrel,+ExtAttrList, +ResultTuple)
----

This predicate will analyse ~ExtAttrList~ and ~ResultTuple~ (which is the result
tuple of a TupleInfoQuery in Nested List format) in parallel and assert the
facts concerning attribute types, attribute sizes, and attribute spelling.

~ExtAttrList~ and ~ResultTuple~ need to cover the same sequence of attributes.
For each element in ~ExtAttrList~, ~ResultTuple~ is required to contain 3
elements for the CoreSize, InternalFlobSize and ExternalFlobSize
(in that order).

Facts are asserted *after* the query result was successfully scanned, such
that when an error occurs, nothing gets (wrongly) asserted.

Extended by further argumeents to be able to handle nested relations.

*/


analyseTupleInfoQueryResultList(DB,DCrel, ARelPath, ExtAttrList, ResListAtts, 
    ResList) :-
  dm(dbhandling,['\nTry: analyseTupleInfoQueryResultList(',DB,',',DCrel,',',
    ARelPath,',', ExtAttrList,',',ResList,').']),
  ResList = [Card,
             TupleTotalSize,
             TupleSizeCore,
             TupleSizeLOB|MoreInfos],
  ResListAtts = [_,_,_,_|MoreRestListAtts],
  ( Card = undefined % Undefined cardinality - may not happen!
    -> ( my_concat_atom(['Cardinality query for relation ',DCrel,
                     ' has strange result: ',Card,'.'],'',ErrMsg),
         write_list(['\nERROR:\t',ErrMsg]),nl,
         throw(error_Internal(analyseTupleInfoQueryResultList(DB,DCrel,
                                ExtAttrList, ResListAtts, % NVK MODIFIED
                                [Card,
                                 TupleTotalSize,
                                 TupleSizeCore,
                                 TupleSizeLOB|MoreInfos]))::wrongType::ErrMsg),
         fail
       )
    ;  true
  ),
  ( TupleTotalSize = undefined % some error may have occured
    -> ( Card < 0.5        % special case for card=0
         -> ( % undefined tuplesize due to empty relation
              write_list(['\nWARNING:\tTuplesize for relation ',DCrel,
                     ' is undefined due to a cardinality of ', Card,
                     '\n--->\tTherefore, tuplesize is set to \'not a number\'',
                     '(nAn).']),nl,
              StoreCoreSize = nAn
            )
         ;  ( % Error! - should not happen!
              my_concat_atom(['Tuplesize query for relation ',DCrel,
                     ' has strange result: ',TupleTotalSize,'.'],'',ErrMsg),
              write_list(['\nERROR:\t',ErrMsg]),nl,
              throw(error_Internal(analyseTupleInfoQueryResultList(DB,DCrel,
                                ExtAttrList,
                                [Card,
                                 TupleTotalSize,
                                 TupleSizeCore,
                                 TupleSizeLOB|MoreInfos]))::wrongType::ErrMsg),
              fail
            )
       )
    ; StoreCoreSize = TupleSizeCore % OK - no problem occured

  ),
  analyseTupleInfoQueryResultList2(DB,DCrel,ARelPath,ExtAttrList, 
    MoreRestListAtts, MoreInfos, TupleMemSize), % NVK MODIFIED

  downcaseList(ARelPath, DCARelPath),
  appendAttributeList([DCrel|DCARelPath], NewDCrel),
  % Card is the number of tuples after unnesting all previous arel attributes
  % within the path.
  assert(storedCard(DB, NewDCrel, Card)),
  ( TupleSizeLOB = undefined
    -> ( % undefined tuplesize due to empty relation
              StoreLOBsize = nAn
            )
    ; StoreLOBsize is max(0,TupleSizeLOB) % avoid rounding errors
  ),
  assert(storedTupleSize(DB, NewDCrel, TupleMemSize, 
         StoreCoreSize, StoreLOBsize)),
  !.

% NVK MODIFIED
%analyseTupleInfoQueryResultList2(+DB,+DCrel,+ARelPath,+ExtAttrList,
%  +InfoListAtts, +InfoList, -MemTotal).
analyseTupleInfoQueryResultList2(_,_,_,[],_,_,0):- !.

analyseTupleInfoQueryResultList2(DB,DCrel,ARelPath,ExtAttrList,InfoListAtts, 
    InfoList, MemTotal):-
  dm(dbhandling,['\nTry: analyseTupleInfoQueryResultList2(',DB,',',DCrel,',',
    ARelPath,',', ExtAttrList,',',InfoListAtts,',',InfoList,').']),
  ExtAttrList = [[ExtAttr,ExtType]|MoreAttrs],
  InfoList  = [SizeCore,SizeExt|MoreInfos],
  InfoListAtts  = [_,_|MoreInfosAtts],
  ( ExtType = [arel,[tuple,_]] -> 
      NExtType = arel
    ;
      NExtType=ExtType
  ),
  dcName2externalName(DCType,NExtType), % XXX won't work for arel
  dcName2externalName(DCAttr,ExtAttr),
  internalName2externalName(IntAttr,ExtAttr),

  % Use inquired average attribute sizes. Avoid problems with undefined
  % sizes, which will occur for relations with cardinalit=0.
  secDatatype(DCType, MemSize, _, _, _, _),
  ( SizeCore = undefined
    -> ( % Fallback: use typesize, but 1 byte at least
         CoreAttrSize is max(1,MemSize)
       )
    ;  CoreAttrSize is max(0,SizeCore) % avoid rounding errors
  ),
  ( SizeExt = undefined
    -> LOBSize is 0
    ;  LOBSize is max(0,SizeExt) % avoid rounding errors
  ),
  LOBSize2 is max(0,LOBSize),    % avoid rounding errors
  analyseTupleInfoQueryResultList2(DB,DCrel,ARelPath,MoreAttrs, MoreInfosAtts, 
    MoreInfos, MoreMemTotal),
  MemTotal is MemSize + MoreMemTotal,
  downcaseList(ARelPath, DCARelPath),
  append(DCARelPath, [DCAttr], TMP1),
  listToAttributeTerm(TMP1, DCFQN),
  assert(storedSpell(DB, DCrel:DCFQN, IntAttr)),
  assert(storedAttrSize(DB,DCrel,DCFQN,DCType,MemSize,CoreAttrSize,
    LOBSize2)),
  !.
analyseTupleInfoQueryResultList2(DB,DCrel,X,Y,Z,G,Q):-
  my_concat_atom(['Retrieval of tuple and attribute information failed.'],
    '',ErrMsg),
  throw(error_Internal(database_analyseTupleInfoQueryResultList2(
    DB,DCrel,X,Y,Z,G,Q)::unspecifiedError::ErrMsg)),
  !.
% NVK ADDED NR END







/*
10 Average Size of a Tuple

---- tuplesize(+Rel, ?Size)

----

The average size of a tuple in Bytes of relation ~Rel~ on disk is ~Size~ bytes.

10.1 Get The Tuple Size

Succeed or failure of this predicate is quite similar to
predicate ~card/2~, see section about cardinalities of
relations. The predicate returns the average total tuplesize, including Flobs.

If the relation has cardinality = 0, Secondo will return a undefined tuplesize.
Therefore, nAn (not a number) is stored in the internal information database,
but 1 is returned for the tuplesize to avoid problems, e.g. when calculating
sample sizes.

*/

% private auxiliary predicate:
tupleSize2(DCrel, sizeTerm(MemSize, CoreSize, LOBSize)) :-
  databaseName(DB),
  ( storedTupleSize(DB, DCrel, MemSize, CoreSize, LOBSize) % already known
    -> true
    ;  ( getTupleInfo(DCrel),   % inquire for it, theen it should be known!
         storedTupleSize(DB, DCrel, MemSize, CoreSize, LOBSize)
       )
  ),
  !.

tuplesize(DCrel, TupleSizeScalar) :-
  dm(dbhandling,['\nTry: tuplesize(',DCrel,',',TupleSizeScalar,').']),
  tupleSize2(DCrel, sizeTerm(_, CoreSize, LOBSize)),
  ( ( CoreSize = nAn )
    -> ( write_list(['\nWARNING:\tCoreTupleSize is not a number (nAn).',
                     '\n--->\tTherefore, CoreTupleSize is set to 1.']),
         nl,
         UsedCoreSize is 1
       )
    ;  ( ( CoreSize =:= 0 )
         -> ( write_list(['\nWARNING:\tCoreTupleSize is 0.',
                     '\n--->\tTherefore, CoreTupleSize is set to 1.']),
              nl,
              UsedCoreSize is 1
            )
         ;  UsedCoreSize is CoreSize
       )
  ),
  ( ( LOBSize = nAn )
    -> ( write_list(['\nWARNING:\tLOBSize is not a number (nAn).',
                     '\n--->\tTherefore, LOBSize is set to 1.']),
         nl,
         UsedLOBSize is 1
       )
    ; UsedLOBSize is LOBSize
  ),
  TupleSizeScalar is UsedCoreSize + UsedLOBSize,
  !.

tuplesize(X, Y) :-
  my_concat_atom(['Cannot retrieve tuplesize for relation \'',X,'\''],'',
                 ErrMsg),
  write_list(['ERROR:\t',ErrMsg]),
  write('.'),nl,
  throw(error_Internal(database_tuplesize(X, Y)::unspecifiedError::ErrMsg));
  !, fail.


/*
The following version of the predicate,

---- tupleSizeSplit(+DCrel, -Size)
----

returns the average tuplesize in a more detailed format, namely as term
~sizeTerm(MemSize, CoreSize, LOBSize)~, where ~MemSize~ is the minimum amount
of main memory in bytes, that is needed for a tuple (without FLOBs), ~CoreSize~
is the average size of the tuples' core data in byte, and ~LOBSize~ is the average
size of data stored in the relation's LOB file.

*/
/*
NVK ADDED NR

*/
tupleSizeSplit(RelTerm, TupleSize) :-
  nrTupleSizeSplit(RelTerm, TupleSize).
% NVK ADDED NR END

tupleSizeSplit(DCrel, sizeTerm(MemSize3,CoreSize3,LOBSize3)) :-
  dm(dbhandling,['\nTry: tupleSizeSplit(',DCrel,',',
                 sizeTerm(MemSize,CoreSize,LOBSize),').']),
  databaseName(DB),
  ( ( storedTupleSize(DB, DCrel, MemSize, CoreSize, LOBSize), % already known
      MemSize \= nAn, CoreSize \= nAn, LOBSize \= nAn )       % well defined
    -> ( MemSize2 = MemSize, CoreSize2 = CoreSize, LOBSize2 = LOBSize
       )
    ;  ( write_list(['\nINFO:\tTuplesize contains "not a number" (nAn). ',
                     '\n--->\tTherefore, I retry getting meaningful data...']),
         nl,
         getTupleInfo(DCrel),   % inquire for it, then it should be known!
         storedTupleSize(DB, DCrel, MemSize2, CoreSize2, LOBSize2)
       )
  ),
  ( MemSize2 = nAn
    -> ( write_list(['\nWARNING:\tTupleMemSize is not a number (nAn). ',
                     '\n--->\tTherefore, it is set to 1.']),
         nl,
         MemSize3 is 1
       )
    ; MemSize3 is MemSize2
  ),
  ( CoreSize2 = nAn
    -> ( write_list(['\nWARNING:\tTupleCoreSize is not a number (nAn). ',
                     '\n--->\tTherefore, it is set to 1.']),
         nl,
         CoreSize3 is 1
       )
    ; CoreSize3 is CoreSize2
  ),
  ( LOBSize2 = nAn
    -> ( write_list(['\nWARNING:\tTupleLOBSize is not a number (nAn). ',
                     '\n--->\tTherefore, it is set to 1.']),
         nl,
         LOBSize3 is 1
       )
    ; LOBSize3 is LOBSize2
  ),
  !.

tupleSizeSplit(DCrel, X) :-
  my_concat_atom(['Unknown error.'],'',ErrMsg),
  throw(error_Internal(database_tupleSizeSplit(DCrel, X)
        ::unknownError::ErrMsg)),
  fail, !.


/*

---- getRelAttrList(+DCrel, -AttrList, sizeTerm(-Mem,-Core,-LOB))
----

For given relation ~DCrel~, return a list ~AttrList~ having format

---- [[AttrName, AttrType, sizeTerm(MemSize,CoreSize,LOBSize)], [...]]
----

with a list for each of ~rel~'s attributes. Also return the total splitTupleSize.

*/
% NVK ADDED NR
getRelAttrList(DCFQN, ResAttrList, SizeTerm) :-
  optimizerOption(nestedRelations),
  databaseName(DB),
  DCFQN=_:_,
  relation(DCFQN, AttrList),
  getRelAttrList2(DB, DCFQN, AttrList, ResAttrList, SizeTerm), !.

getRelAttrList(DCrel, ResAttrList, SizeTerm) :-
  optimizerOption(nestedRelations),
  databaseName(DB),
  is_nrel(DCrel),
  relation(DCrel, AttrList),
  reduceToARel(AttrList, [], AttrList2),
  getRelAttrList2(DB, DCrel, AttrList2, ResAttrList, SizeTerm), !.
% NVK ADDED NR END

getRelAttrList(DCrel, ResAttrList, SizeTerm) :-
  databaseName(DB),
  relation(DCrel, AttrList),
  getRelAttrList2(DB, DCrel, AttrList, ResAttrList, SizeTerm), !.

getRelAttrList(DCrel, ResAttrList, SizeTerm) :-
  my_concat_atom(['Unknown error.'],'',ErrMsg),
  throw(error_Internal(database_getRelAttrList(DCrel, ResAttrList, SizeTerm)
                                                    ::unknownError::ErrMsg)),
  fail, !.

% NVK ADDED NR
getRelAttrList2(_, _, [], [], sizeTerm(0,0,0)) :- !.
getRelAttrList2(DB, DCFQN, [Attr|AttrList1], [ResAttr|ResAttrList1],
    TupleSize) :-
  optimizerOption(nestedRelations),
  DCFQN=DCrel:NRelPath,
  appendAttribute(NRelPath, Attr, Attr2),

  storedAttrSize(DB, DCrel, Attr2, Type, MemSize, Core, LOB),
  getRelAttrList2(DB, DCFQN, AttrList1, ResAttrList1, TupleSize1),
  AttrSizeTerm = sizeTerm(MemSize, Core, LOB),
  ResAttr = [Attr, Type, AttrSizeTerm],
  addSizeTerms([TupleSize1,AttrSizeTerm],TupleSize),
  !.
% NVK ADDED NR END

getRelAttrList2(_, _, [], [], sizeTerm(0,0,0)) :- !.
getRelAttrList2(DB,DCrel,[Attr|AttrList1],[ResAttr|ResAttrList1],TupleSize) :-
  storedAttrSize(DB, DCrel, Attr, Type, MemSize, Core, LOB),
  getRelAttrList2(DB, DCrel, AttrList1, ResAttrList1, TupleSize1),
  AttrSizeTerm = sizeTerm(MemSize, Core, LOB),
  ResAttr = [Attr, Type, AttrSizeTerm],
  addSizeTerms([TupleSize1,AttrSizeTerm],TupleSize),
  !.

/*

---- projectAttrList(+OrigAttrs, +ProjAttrs, -ResAttrList, -ProjTupleSize)
----

Restricts a given attribute list ~OrigAttrs~ to the attributes given in
~ProjAttrs~ and also returns the according projected tuple size ~ProjTupleSize~.

Attribute names are the current name of the attribute, respecting renames that
already have been applied to the attribute!

*/

projectAttrList(_, [], [], sizeTerm(0,0,0)) :- !.
projectAttrList([], _, [], sizeTerm(0,0,0)) :- !.
projectAttrList([[Attr,Type,AttrSZ]|MoreAttrs],ProjAttrs,ResList,ResSZ) :-
  ( memberchk(Attr,ProjAttrs)
    -> ( % copy Attr to result list
         delete(ProjAttrs,Attr,ProjAttrs1),
         projectAttrList(MoreAttrs,ProjAttrs1,ResList1,ResSZ1),
         ResList = [[Attr,Type,AttrSZ]|ResList1],
         addSizeTerms([AttrSZ,ResSZ1],ResSZ)
       )
    ;  projectAttrList(MoreAttrs,ProjAttrs,ResList,ResSZ)
  ), !.
projectAttrList(W,X,Y,Z) :-
  my_concat_atom(['Unknown error.'],'',ErrMsg),
  throw(error_Internal(database_projectAttrList(W,X,Y,Z)
        ::unknownError::ErrMsg)),
  fail, !.

/*

---- createExtendAttrList( +ExtendFields, +RelList, -ExtendAttrs, -ExtendAttrSize )
----

For a given list of extension fields, return a list with attribute-descriptors
and a sizeTerm with the according aggregated sizes.

~ExtendFields~ has format list of field(attr(Name,Arg,Case),Expr)

~RelList~ is a list of all relations occuring withinthe extendfields.
Each entry has format: (ArgNo,rel(DCrelName,Var))

~ExtendAttrs~ is a list of descriptors [Name, Type, SizeTerm] for each extended
attribute.

~ExtendAttrSize~ is a sizeterm for the combined size of extension attributes

*/

createExtendAttrList([],_,[],sizeTerm(0,0,0)) :- !.
createExtendAttrList([Field|MoreFields],
                     RelInfoList,
                     [[AttrName,AttrType,AttrSize]|MoreAttrs],
                     TotalAttrSize) :-
  /*
  NVK MODIFIED NR
  org:
  Field = newattr(attrname(attr(Name, _, _)), Expr),
  newattr terms are generated within e.g. the translate predicates, 
  but field(attr(...)) terms are generated for new attributes within
  the select clause. Compare lookupAttr(Expr as XY).
  Don't know if this should be fixed?!?
  */
  (Field = newattr(attrname(attr(Name, _, _)), Expr)
  ; Field = field(attr(Name, _, _), Expr)),
  % NVK MODIFIED NR END

%   write_list(['createExtendAttrList: Called with ',[Field|MoreFields],'.\n']),
  ( Name = Attr:Suffix
    -> ( my_concat_atom([Attr,Suffix],'_',Renamed),
         downcase_atom(Renamed,AttrName)
       )
    ; downcase_atom(Name,AttrName)
  ),
%   write_list(['createExtendAttrList: Dealing with attribute ',
%               AttrName,'.\n']),
%   write_list(['createExtendAttrList: recursing into ',MoreFields,'.\n']),
  createExtendAttrList(MoreFields, RelInfoList, MoreAttrs, MoreAttrsSize),
%   write_list(['createExtendAttrList: returned from recursion into ',
%                MoreFields,'.\n']),
  ( getTypeTree(Expr,RelInfoList,[_,_,ExprType])
    -> AttrType = ExprType  % use inferred type
    ;  ( AttrType = int,
         write_list(['WARNING:\t could not determine expression type for ',
                     'extended attribute ',Name,': ',Expr,
                     '!\n\t\tUsing \'int\' as a fallback.']),nl)
        % using 'int' as a fallback
  ),
%   write_list(['createExtendAttrList: after getTypeTree. Type is ',
%                AttrType,'.\n']),
  secDatatype(AttrType, MemSize, _ /* NoFlobs */, _ /* PersistencyMode */,_,_),
%   write_list(['createExtendAttrList: MemSize is ',MemSize,'.\n']),
  AttrSize = sizeTerm(MemSize,MemSize /* Core */ , 0 /* Lob */),
%   write_list(['createExtendAttrList: AttrSize is ',AttrSize,'.\n']),
  addSizeTerms([AttrSize,MoreAttrsSize],TotalAttrSize),
%   write_list(['createExtendAttrList: finished. TotalAttrSize = ',
%               TotalAttrSize,'\n']),
  !.
createExtendAttrList(W,X,Y,Z) :-
  my_concat_atom(['Unknown error.'],'',ErrMsg),
  throw(error_Internal(database_createExtendAttrList(W,X,Y,Z)
        ::unknownError::ErrMsg)),
  fail, !.

/*
10.2 Storing And Loading Tuple Sizes

*/
readStoredTupleSizes :-
  retractall(storedTupleSize(_, _, _, _, _)),
%  [storedTupleSizes].
  load_storefiles(storedTupleSizes).

writeStoredTupleSizes :-
  open('storedTupleSizes.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredTupleSize(FD), _),
  close(FD).

writeStoredTupleSize(Stream) :-
  storedTupleSize(DB, DCrel, Mem, Core, LOB),
  write(Stream, storedTupleSize(DB, DCrel, Mem, Core, LOB)),
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
11 Managing Datatypes

11.1 Datatype Information

For all available Secondo data types, the algebra providing the type,
all kinds the types implement and the core tuple sizes are inquired from
the database kernel.

To calculate the proper sizes of attributes, the optimizer needs information
on how much memory the representation of available Secondo datatypes need.
To get this information, a systemtable with this information is queried whenever
a database is opened (see file ~auxiliary.pl~). The systemtable is a relation
contaning (among others) two attributes ~Type~ (containing the name of a
datatype) and ~Size~ (containing its size in byte).

Type information is inquired from the database kernel and stored in facts

---- secDatatype(TypeNameDC, TypeSize, NoFlobs, PersistencyModeDC,
                                                          AlgebraDC, DCkinds)
----

where ~TypeNameDC~ is the type name, ~TypeSize~ is the in-memory size of the
according fixed part of the data type in bytes (the minimum memory required
excluding FLOB data, but inclusing other variable parts of the data)),
~NoFlobs~ is the number of FLOBS the type maintains, ~PersistencyModeDC~ is
the type of storage mechanism used to save instances of this data type to disk.
There are 4 differnt mechanisms:

  * (unspec) unspecified. This usually means, that the actual mechanism cannot
    be determined since the type is not in kind DATA.

  * (mbfc) memoryblock-fix-core. This type means, that the data type has a
    constant in-memory-size (and possible some additional LOBs) and is stored
    to disk by just copying its memory block into the tuple core file and its
    LOBs to the LOB file. The type has constant memory, disk core and possibly
    variable disk LOB sizes.

  * (szfc) serialize-fix-core. This type means, that the data type has a
    constant in-memory-size, but provides a serialization method to save itself
    into a byte string of variable length to be stored to disk. The entire
    string is saved thisin the tuple core.
    The type has  constant memory and a constant (but usually smaller) disk
    core size. LOBs are currently not supported.

  * (szve) serialize-variable-extension. This type means, that the data type
    has a constant in-memory-size, and provides a serialization method to save
    itself into a byte string of variable length to be stored to disk. At
    least a part of this string is stored with the tuple core, the remainder
    may overflow into the tuple extension part.
    The type has constant memory and a constant (but usually smaller) disk core
    size. LOBs are currently not supported.

~AlgebraDC~ is the name of the providing Algebra, and ~DCkinds~ is a list with
all kinds, the data type belongs to.


*/

:-assert(helpLine(showDatatypes,0,[],
        'List all registered Secondo data types.')).

:-   dynamic(secDatatype/6).

extractSecondoTypeSizes([]) :- !.

extractSecondoTypeSizes([X|Rest]) :-
  X = [TypeNameQuoted, AlgebraQuoted, TypeSize, NoFlobs, PersModeQuoted, Kinds],
  sub_atom(TypeNameQuoted,1,_,1,TypeName),
  sub_atom(PersModeQuoted,1,_,1,PersistencyMode),
  sub_atom(AlgebraQuoted,1,_,1,AlgebraLong),
  downcase_atom(TypeName, TypeNameDC),
  downcase_atom(PersistencyMode, PersistencyModeDC),
  (  member([PersistencyModeDC,PersCode],[['unspecified',na],
                                         ['memoryblock-fix-core',mbfc],
                                         ['serialize-fix-core',szfc],
                                         ['serialize-variable-extension',szve]])
   ; PersCode = err
  ),
  downcase_atom(AlgebraLong,AlgebraLongDC),
  ( ( sub_atom(AlgebraLongDC, _, _, 0, 'algebra'),
      sub_atom(AlgebraLongDC, 0, _, 7, Algebra)
    )
    ; Algebra = AlgebraLongDC
  ),
  term_to_atom(KindsList,Kinds),
  assert(secDatatype(TypeNameDC, TypeSize, NoFlobs, PersCode,
                     Algebra, KindsList)),!,
  extractSecondoTypeSizes(Rest).

readSecondoTypeSizes :-
  retractall(secDatatype(_, _, _, _, _, _)),
  isDatabaseOpen, !,
  my_concat_atom(['query SEC2TYPEINFO feed projectextendstream[Type,Algebra, ',
               'CppClassSize,NumOfFlobs,PersistencyMode;Kind: .Type kinds] ',
               'sortby[Type,Algebra,CppClassSize,NumOfFlobs,PersistencyMode]',
               'groupby[Type,Algebra,CppClassSize,NumOfFlobs,PersistencyMode; ',
               'Kinds: \'[\' + (group feed projectextend[;Kind: ',
               'tolower(totext(.Kind))] aggregateB[Kind;fun(K1:text, K2: text)',
               ' K1 + \',\' + K2; \'\'] ) + \']\' ] tconsume'],'',Query),
  secondo(Query,SecList),
  SecList = [_,L],
  extractSecondoTypeSizes(L).

showOneDatatype :-
  secDatatype(Type, Size, NoFlobs, Pers, Algebra, Kinds),
  findall([NullType,NullVal], nullValue(Type,NullType,NullVal), NullValues),
  format('~w~20|~w~30|~w~35|~w~40|~w~55|~w~70|~w~n',[Type,Size,NoFlobs,Pers,
                                         Algebra,Kinds,NullValues]).

showDatatypes :-
  format('~w~20|~w~30|~w~35|~w~40|~w~55|~w~70|~w~n',
         ['Type','Size','Flobs','Pers','Algebra','Kinds','NullValues']),
  findall(_, showOneDatatype, _).

/*
11.2 Kind Checking Data Types

---- isData(+T)
----
Succeeds, iff data type ~T~ is in kind DATA

---- isKind(+T,+K)
----
Succeeds, if data type ~T~ is in kind ~K~

*/


isData(T) :-
  secDatatype(T, _, _, _, _, Kinds),
  memberchk(data,Kinds), !.

% isKind(+T,+K)
% check, whether data type ~T~ is in kind K
isKind(TC,K) :- % complex type
  is_list(TC),
  TC =.. [T|_],
  secDatatype(T, _, _, _, _, Kinds),
  memberchk(K,Kinds), !.
isKind(T,K) :-
  secDatatype(T, _, _, _, _, Kinds),
  memberchk(K,Kinds), !.


/*
12 Showing, Loading, Storing and Looking-Up Attribute Sizes and Types

Together with the attributes` type, this information is stored as facts
~storedAttrSize(Database, Rel, Attr, Type, MemSize, CoreSize, LOBSize)~ in
memory. Between sessions information is stored in file ~storedAttrSizes.pl~.

Throughout the optimizer, attribute sizes are passed in terms ~sizeTerm(MemSize, CoreSize, LOBSize)~.

*/

:-assert(helpLine(showStoredAttrSizes,0,[],
        'List metadata on attribute sizes in current DB.')).


attrSize(DCRel:DCAttr, sizeTerm(MemSize, CoreSize, LOBSize)) :-
  databaseName(DBName),
  storedAttrSize(DBName, DCRel, DCAttr, _, MemSize, CoreSize, LOBSize),
  !.

attrSize(X, Y) :-
  my_concat_atom(['Missing attribute size data on attribute \'',X,'\''],'',
                 ErrMsg),
  throw(error_Internal(database_attrSize(X, Y)::unspecifiedError::ErrMsg)),
  fail, !.


attrType(DCRel:DCAttr, Type) :-
  databaseName(DBName),
  storedAttrSize(DBName, DCRel, DCAttr, Type, _, _, _), !.

attrType(X, Y) :-
  my_concat_atom(['Missing type data on attribute \'',X,'\''],'',ErrMsg),
  throw(error_Internal(database_attrType(X, Y)::unspecifiedError::ErrMsg)),
  fail, !.

readStoredAttrSizes :-
  retractall(storedAttrSize(_, _, _, _, _, _, _)),
%  [storedAttrSizes].
  load_storefiles(storedAttrSizes).

writeStoredAttrSizes :-
  open('storedAttrSizes.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredAttrSize(FD), _),
  close(FD).

writeStoredAttrSize(Stream) :-
  storedAttrSize(Database, Rel, Attr, Type, MemSize, CoreSize, LOBSize),
  write(Stream, storedAttrSize(Database, Rel, Attr, Type, MemSize,
                               CoreSize, LOBSize)),
  write(Stream, '.\n').

showStoredAttrSize :-
  storedAttrSize(Database, Rel, Attr, Type, MemSize, CoreSize, LOBSize),
  write(Database), write('.'), write(Rel), write('.'),
  write(Attr), write(': \t'), write(Type),
  write(' ('), write(MemSize), write('/'),
  write(CoreSize), write('/'), write(LOBSize), write(')\n').

showStoredAttrSizes :-
  write('Stored attribute sizes\nDb.Rel.Attr: Type '),
  write('(CoreTupleSize/Avg.InlineFlobSize/Avg.ExtFlobSize) [byte]:\n'),
  findall(_, showStoredAttrSize, _).

:-
  dynamic(storedAttrSize/7),
  at_halt(writeStoredAttrSizes),
  readStoredAttrSizes.

% sum-up a list of sizeTerms
addSizeTerms([], sizeTerm(0, 0, 0)) :- !.
addSizeTerms([sizeTerm(X1,Y1,Z1)|Rest], sizeTerm(Res1, Res2, Res3)) :-
  number(X1), number(Y1), number(Z1),
  addSizeTerms(Rest, sizeTerm(X2,Y2,Z2)),
  Res1 is X1 + X2,
  Res2 is Y1 + Y2,
  Res3 is Z1 + Z2, !.
addSizeTerms(X, Y) :-
  my_concat_atom(['Unknown error.'],'',ErrMsg),
  throw(error_Internal(database_addSizeTerms(X, Y)::unknownError::ErrMsg)),
  fail, !.

negateSizeTerm(sizeTerm(A,B,C),sizeTerm(A1,B1,C1)) :-
  number(A), number(B), number(C),
  A1 is -1 * A,
  B1 is -1 * B,
  C1 is -1 * C,
  !.
negateSizeTerm(A,B) :-
  my_concat_atom(['Unknown error.'],'',ErrMsg),
  throw(error_Internal(database_negateSizeTerm(A,B)::unknownError::ErrMsg)),
  fail, !.

% Get the total disk size from a sizeTerm or list of sizeTerms
getTotalDiskSize(sizeTerm(_,Core,LOB),Total) :-
  Total is Core + LOB.
getTotalDiskSize([],0) :- !.
getTotalDiskSize([X|More],Total) :-
  getTotalDiskSize(X,Xtotal),
  getTotalDiskSize(More,MoreTotal),
  Total is Xtotal + MoreTotal, !.



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
%  [storedOrderings].
  load_storefiles(storedOrderings).

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

There are some more system identifiers, e.g. ~const~ and ~value~., which are
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

Additionally, you should check whether an attribute name matches an existing
object name, because this might cause severe problems during type mapping and
query processing.

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
  assert(systemIdentifier(begin,begin)),
  assert(systemIdentifier(transaction,transaction)),
  assert(systemIdentifier(commit,commit)),
  assert(systemIdentifier(abort,abort)),
  assert(systemIdentifier(kill,kill)),
  assert(systemIdentifier(set,set)),
  assert(systemIdentifier(value,value)),
  assert(systemIdentifier(const,const)),
  assert(systemIdentifier(query,query)),
  assert(systemIdentifier(let,let)),
  assert(systemIdentifier(derive,derive)),
  assert(systemIdentifier(update,update)),
  assert(systemIdentifier(delete,delete)),
  assert(systemIdentifier(beginseq,beginseq)),
  assert(systemIdentifier(endseq,endseq)),
  assert(systemIdentifier(if,if)),
  assert(systemIdentifier(then,then)),
  assert(systemIdentifier(else,else)),
  assert(systemIdentifier(endif,endif)),
  assert(systemIdentifier(while,while)),
  assert(systemIdentifier(do,do)),
  assert(systemIdentifier(endwhile,endwhile)),
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
  assert(systemIdentifier(rowid,rowid)), % reference for gettuple(.)
% Section:Start:readSystemIdentifiers_0_i
% Section:End:readSystemIdentifiers_0_i
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
  my_string_to_atom(ExtString,ExtQuotedAtom),
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
   ;  ( my_concat_atom(['No database open: Cannot create relation \'',
                     ExtRelName,'\'.'],'',ErrMsg),
        write_list(['ERROR:\t',ErrMsg]),nl,
        throw(error_SQL(database_create_relation(ExtRelName,AttrTypeList)
                        ::noDatabase::ErrMsg)),
        !, fail
      )
  ),
  dcName2externalName(DCname,ExtRelName),
  ( secondoCatalogInfo(DCname,ExtRelName2,_,_)
    -> ( my_concat_atom(['Invalid relation name: Object \'',ExtRelName2,
                      '\' already exists.'],'',ErrMsg),
         write_list(['\nERROR:\t',ErrMsg]),nl,
         throw(error_SQL(database_create_relation(ExtRelName,AttrTypeList)
                        ::objectAlreadyExists::ErrMsg)), fail
       )
    ;  true
  ),
  ( validIdentifier(ExtRelName)
    -> true
    ;  ( my_concat_atom(['Invalid relation name: Tried to create relation \'',
                      ExtRelName,'\'.'],'',ErrMsg),
         write_list(['\nERROR:\t',ErrMsg]),nl,
         throw(error_SQL(database_create_relation(ExtRelName,AttrTypeList)
                        ::schemaError::ErrMsg)), fail
       )
  ),
  ( checkAttrTypeList(AttrTypeList)
    -> true
    ;  ( my_concat_atom(['Invalid attribute/type list.'],'',ErrMsg),
         write_list(['\nERROR:\t',ErrMsg]),nl,
         throw(error_SQL(database_create_relation(ExtRelName,AttrTypeList)
                        ::schemaError::ErrMsg)), fail
       )
  ),
  getConstRelationTypeExpression(AttrTypeList,TypeExpr),
  my_concat_atom(['let ', ExtRelName, ' = [const ', TypeExpr, ' value ()]'],
              QueryAtom),
  ( secondo(QueryAtom)
    -> true
    ;  ( my_concat_atom(['Secondo command failed: Tried to create relation \'',
                      ExtRelName,'\'.'],'',ErrMsg),
         write_list(['\nERROR:\t',ErrMsg]),nl,
         throw(error_SQL(database_create_relation(ExtRelName,AttrTypeList)
                        ::unspecifiedError::ErrMsg)), fail
       )
  ),!,
  updateCatalog,
  updateCatalog.

create_relation(ExtRelName,AttrTypeList) :-
    my_concat_atom(['Unknown error: Tried to create relation \'',ExtRelName,
                 '\'.'],'',ErrMsg),
    write_list(['\nERROR:\t',ErrMsg]),nl,
    throw(error_SQL(database_create_relation(ExtRelName,AttrTypeList)
                        ::unspecifiedError::ErrMsg)),
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
  ( secDatatype(Type, _, _, _, _, _)
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
  my_concat_atom(['rel(tuple(',TupleExpr,'))'],TypeExpr),
  !.

getConstRelationTypeExpression2([[AttrName, Type]],AttrExpr) :-
  my_concat_atom(['(',AttrName, ' ', Type, ')'], AttrExpr),
  !.

getConstRelationTypeExpression2([[AN,TY] | More],AttrExpr) :-
  getConstRelationTypeExpression2(More,MoreExpr),
  my_concat_atom(['(',AN,TY,')',MoreExpr],' ',AttrExpr),
  !.

/*
----
drop_relationExt(+ExtRelName)
drop_relation(+DCrel)

----

Drop the relation and possibly dependent objects, delete meta data.
The relation name is passed in ~external spelling~ (resp. ~DC-spelling~).

*/
% the internal predicate
drop_relationInt(ExtRelName) :-
  dm(dbhandling,['\nTry: drop_relationInt(',ExtRelName,').']),
  ( databaseName(_)
    -> true
   ;  (  my_concat_atom(['No database open: Cannot drop relation \'',ExtRelName,
                      '\'.'],'',ErrMsg),
         write('ERROR:\t'),write(ErrMsg),nl,
         throw(error_SQL(database_drop_relation(ExtRelName)
              ::noDatabase::ErrMsg)),
         !, fail
      )
  ),
  ( ( secondoCatalogInfo(DCrel,ExtRelName,_,Type),
      % NVK MODIFIED NR
      %Type = [[rel, [tuple, _]]]
      Type = [[RelType, [tuple, _]]],
      (optimizerOption(nestedRelations) ->
        member(RelType, [rel, nrel])
      ;
        RelType=rel
      )
      % NVK MODIFIED NR END
    )
    -> true
    ;  (  my_concat_atom(['Type error: Object \'',ExtRelName,
                       '\' unknown or not a relation.'],'',ErrMsg),
          write_list(['\nERROR:\t',ErrMsg]),nl,
          throw(error_SQL(database_drop_relation(ExtRelName)
                                   ::unknownRelation::ErrMsg)), !, fail
       )
  ),
  deleteObject(ExtRelName),
  handleLostRelation(DCrel),
  updateCatalog,
  updateCatalog,
  write_list(['\nINFO:\tSuccessfully dropped relation \'',ExtRelName,'\'.']),
  nl, !.

drop_relationInt(ExtRelName) :-
    write_list(['\nERROR:\tFailed trying to drop \'',ExtRelName,'\'.']),nl,
    my_concat_atom(['Unknown error: While trying to drop relation \'',
                  ExtRelName,
                 '\'.'],'',ErrMsg),
    throw(error_SQL(database_drop_relation(ExtRelName)
                                   ::unspecifiedError::ErrMsg)),
    !, fail.

% The user level predicate:
:- assert(helpLine(drop_relation,1,
    [[+,'DCrel','The relation to drop.']],
    'Drop a relation; delete it (and related meta data) from the current DB.')).

drop_relation(DCrel) :-
  dm(dbhandling,['\nTry: drop_relation(',DCrel,').']),
  ground(DCrel),
  atomic(DCrel),
  relation(DCrel,_),
  dcName2externalName(DCrel,ExtRelName),
  drop_relationInt(ExtRelName), !.

drop_relation(DCrel) :-
    my_concat_atom(['Unknown relation: \'',DCrel,'\'.'],'',ErrMsg),
    write_list(['\nERROR:\tFailed trying to drop \'',DCrel,'\'.']),nl,
    throw(error_SQL(database_drop_relation(DCrel)::unknownRelation::ErrMsg)),
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


/*
End of file ~database.pl~

*/
