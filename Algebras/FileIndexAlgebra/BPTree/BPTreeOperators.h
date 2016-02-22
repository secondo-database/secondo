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

#ifndef BPTREEOPERATORS_H
#define BPTREEOPERATORS_H

#include "NestedList.h"
#include "Operator.h"
#include "BPTree.h"

namespace fialgebra {
  class BPTreeOperators
  {
  public:
    static Operator& GetCreatefbtreeOperator();
    static Operator& GetInsertfbtreeOperator();
    static Operator& GetDeletefbtreeOperator();
    static Operator& GetRebuildfbtreeOperator();
    static Operator& GetBulkloadfbtreeOperator();

    //
    // David
    //
    // Mit Hilfe dieses Operators werden alle Tupel-IDs aus dem Baum
    // extrahiert, deren dazugehoerige Keys im angegebenen Intervall
    // liegen.
    // frange: {text , string} × T × T -> stream(tid)
    // query "strassen_name_btree.bin" frange["A","B"]
    //     strassen gettuples consume
    static Operator& GetFrangeOperator();
    //
    // Auch dieser Operator beschreibt eine Intervallsuche, wobei in
    // dieser Variante lediglich das rechte Intervallende angegeben wird.
    // fleftrange: {text , string} × T -> stream(tid)
    // query "strassen_name_btree.bin" fleftrange["Syringenweg"]
    //     strassen gettuples consume
    static Operator& GetFleftrangeOperator();
    //
    // In dieser Variante der Bereichssuche werden alle Tupel-IDs ausgegeben,
    // bei denen die dazugehoerigen Objekte groesser oder gleich dem
    // uebergebenen Element sind.
    // frightrange: {text , string} × T -> stream(tid)
    // query "strassen_name_btree.bin" frightrange["Syringenweg"]
    //     strassen gettuples consume
    static Operator& GetFrightrangeOperator();
    //
    // Das Ergebnis dieses Operators entspricht einer Intervallsuche, bei
    // der das linke und rechte Intervallende gleich sind. Es werden also
    // genau diejenigen Elemente ausgewaehlt, bei denen der gespeicherte
    // Attributwert dem Suchwert entspricht. Da die gespeicherten Werte
    // nicht eindeutig sind, sind auch bei diesem Operator mehrere Ergebnisse
    // moeglich, so dass das Ergebnis ebenfalls ein Strom von Tupel-IDs ist.
    // fexactmatch: {text , string} × T -> stream(tid)
    // query "strassen_name_btree.bin" fexactmatch["Syringenweg"]
    //     strassen gettuples consume
    static Operator& GetFexactmatchOperator();

  private:
    // Used as local during Create, Insert and Delete
    struct OperatorContext{
      BPTree* tree;
      size_t skipped;

      OperatorContext(BPTree* tree, size_t skipped);
      virtual ~OperatorContext();
    };

    // Used as local during Bulkload
    struct BulkloadContext : OperatorContext{
      Attribute* prevAttr;

      BulkloadContext(BPTree* tree, Attribute* prevAttr, size_t skipped);
      virtual ~BulkloadContext();
    };

    static OperatorSpec
      // David
      frangeOperatorSpec,
      fleftrangeOperatorSpec,
      frightrangeOperatorSpec,
      fexactmatchOperatorSpec;

    static Operator createfbtreeOp,
      insertfbtreeOp,
      deletefbtreeOp,
      rebuildfbtreeOp,
      bulkloadfbtreeOp,
      // David
      frangeOp,
      fleftrangeOp,
      frightrangeOp,
      fexactmatchOp;

    static Operator CreateCreatefbtreeOp();
    static Operator CreateInsertfbtreeOp();
    static Operator CreateDeletefbtreeOp();
    static Operator CreateRebuildfbtreeOp();
    static Operator CreateBulkloadfbtreeOp();

    static ListExpr CreatefbtreeTM(ListExpr args);
    static ListExpr InsertfbtreeTM(ListExpr args);
    static ListExpr DeletefbtreeTM(ListExpr args);
    static ListExpr RebuildfbtreeTM(ListExpr args);

    static int CreatefbtreeVM (Word* args, Word& result, int message,
                         Word& local, Supplier s);
    static int InsertfbtreeVM (Word* args, Word& result, int message,
                         Word& local, Supplier s);
    static int DeletefbtreeVM (Word* args, Word& result, int message,
                         Word& local, Supplier s);
    static int RebuildfbtreeVM (Word* args, Word& result, int message,
                         Word& local, Supplier s);
    static int BulkloadfbtreeVM (Word* args, Word& result, int message,
                         Word& local, Supplier s);

    // Standard Cache-Groesse in Seiten
    static size_t defaultCacheSize;

    //
    // David
    //
    // frange Type Mapping
    static ListExpr FrangeTM( ListExpr args );
    // fleftrange Type Mapping
    static ListExpr FleftrangeTM( ListExpr args );
    // frightrange Type Mapping
    static ListExpr FrightrangeTM( ListExpr args );
    // fexactmatch Type Mapping
    static ListExpr FexactmatchTM( ListExpr args );
    // Generic f*[range|exactmatch] Type Mapping
    static ListExpr GenericFSearchTM( int searchType, ListExpr args );
    //
    // Value Mapping
    static int FrangeVM( Word* args, Word& result, int message,
                         Word& local, Supplier s );
  };
} // namespace fialgebra

#endif // BPTREEOPERATORS_H









