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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//characters [1] Type: [] []
//characters [2] Type: [] []
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Creation of various tuples for testing.

[toc]

1. TestTuples class

*/

#ifndef __TESTTUPLES_H__
#define __TESTTUPLES_H__

#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"
#include "StandardTypes.h"
#include "Algebras/Record/Record.h"
#include "Algebras/Collection/CollectionAlgebra.h"
#include "VTree.h"
#include "VTuple.h"

#include <string>

namespace cstream {

    class TestTuples {
    public:
                
        // ((Plz int)(Ort string)(Id int))
        static VTuple* Create1() {
            TupleDescr* td = new TupleDescr("((Plz int)(Ort string)(Id int))");
            TupleType* tt = td->CreateTupleType();
            Tuple* t = new Tuple(tt);
            t->PutAttribute(0, new CcInt(true, 85307));
            t->PutAttribute(1, new CcString(true, "Paunzhausen"));
            t->PutAttribute(2, new CcInt(true, 42));
            tt->DeleteIfAllowed();
            VTuple* vt = new VTuple(t, td);
            return vt;
        }

        // ((Plz int)(Id int))
        static VTuple* Create1_1() {
            TupleDescr* td = new TupleDescr("((Plz int)(Id int))");
            TupleType* tt = td->CreateTupleType();
            Tuple* t = new Tuple(tt);
            t->PutAttribute(0, new CcInt(true, 85307));
            t->PutAttribute(1, new CcInt(true, 42));
            tt->DeleteIfAllowed();
            VTuple* vt = new VTuple(t, td);
            return vt;
        }

        // ((Name string)(Alter int)(Adresse (record(Strasse string)
        // (Nummer int)(Plz string)(Ort string))))
        static VTuple* Create2() {
            TupleDescr* td = new TupleDescr("((Name string)(Alter int)(Adresse "
            "(record(Strasse string)(Nummer int)(Plz string)(Ort string))))");
            TupleType* tt = td->CreateTupleType();
            Tuple* t = new Tuple(tt);
            t->PutAttribute(0, new CcString(true, "Maysami"));
            t->PutAttribute(1, new CcInt(true, 49));
            Record* rec = new Record(4);
            rec->SetElement(0, new CcString(true, "Sportplatz Str."), 
            "string", "Strasse");
            rec->SetElement(1, new CcInt(true, 34), "int", "Nummer");
            rec->SetElement(2, new CcString(true, "85307"), "string", 
            "Plz");
            rec->SetElement(3, new CcString(true, "Paunzhausen"), "string"
            ,"Ort");
            t->PutAttribute(2, rec);
            tt->DeleteIfAllowed();
            VTuple* vt = new VTuple(t, td);
            return vt;
        }

        // ((Name string)(Adresse (record(Nummer int)(Ort string))))
        static VTuple* Create2_1() {
            TupleDescr* td = new TupleDescr("((Name string)(Adresse "
            "(record(Nummer int)(Ort string))))");
            TupleType* tt = td->CreateTupleType();
            Tuple* t = new Tuple(tt);
            t->PutAttribute(0, new CcString(true, "Maysami"));
            Record* rec = new Record(2);
            rec->SetElement(0, new CcInt(true, 34), "int", "Nummer");
            rec->SetElement(1, new CcString(true, "Paunzhausen"), "string"
            , "Ort");
            t->PutAttribute(1, rec);
            tt->DeleteIfAllowed();
            VTuple* vt = new VTuple(t, td);
            return vt;
        }

        // ((Id int)(Adresse (record (Nummer int)
        // (R2 (record(Id1 int)(Id2 int)))(Ort string))))
        static VTuple* Create3() {
            TupleDescr* td = new TupleDescr("((Id int)(Adresse (record "
            "(Nummer int)(R2 (record(Id1 int)(Id2 int)))(Ort string))))");
            TupleType* tt = td->CreateTupleType();
            Tuple* t = new Tuple(tt);
                                    
            t->PutAttribute(0, new CcInt(true, 1));

            Record* r = new Record(3);
            r->SetElement(0, new CcInt(true, 42), "int", "Nummer");

            Record* r2 = new Record(2);
            r2->SetElement(0, new CcInt(true, 1), "int", "Id1");
            r2->SetElement(1, new CcInt(true, 2), "int", "Id2");
            
            r->SetElement(1, r2, "record", "R2");
            r->SetElement(2, new CcString(true, "Paunzhausen"), 
            "string", "Ort");

            t->PutAttribute(1, r);
            tt->DeleteIfAllowed();
            VTuple* vt = new VTuple(t, td);
            return vt;            
        }

        // ((Id int)(Adresse (record(R2 (record(Id2 int))))(Ort string)))
        static VTuple* Create3_1() {
            TupleDescr* td = new TupleDescr("((Id int)(Adresse (record(R2 "
            "(record(Id2 int))))(Ort string)))");
            TupleType* tt = td->CreateTupleType();
            Tuple* t = new Tuple(tt);
            
            t->PutAttribute(0, new CcInt(true, 1));

            Record* r = new Record(2);

            Record* r2 = new Record(1);
            r2->SetElement(0, new CcInt(true, 2), "int", "Id2");
            
            r->SetElement(0, r2, "record", "R2");
            r->SetElement(1, new CcString(true, "Paunzhausen"), 
            "string", "Ort");

            t->PutAttribute(1, r);
            tt->DeleteIfAllowed();
            VTuple* vt = new VTuple(t, td);
            return vt;
        }

        // ((Name string)(Adresse (record(Strasse string)(Nummer int)
        // (Plz string)(Ort string)))(Tel (vector(record(Art int)
        // (Nummer string)))))
        static VTuple* Create4() {
            TupleDescr* td = new TupleDescr("((Name string)(Adresse (record"
            "(Strasse string)(Nummer int)(Plz string)(Ort string)))(Tel "
            "(vector(record(Art int)(Nummer string)))))");
            TupleType* tt = td->CreateTupleType();
            Tuple* t = new Tuple(tt);
            t->PutAttribute(0, new CcString(true, "Maysami"));
            
            Record* rec = new Record(4);
            rec->SetElement(0, new CcString(true, "Sportplatz Str."), "string"
            , "Strasse");
            rec->SetElement(1, new CcInt(true, 34), "int", "Nummer");
            rec->SetElement(2, new CcString(true, "85307"), "string", "Plz");
            rec->SetElement(3, new CcString(true, "Paunzhausen"), "string"
            , "Ort");
            t->PutAttribute(1, rec);

            ListExpr typeinfo;            
            nl->ReadFromString("(vector(record((Art int)(Nummer string))))"
            , typeinfo);            
            SecondoCatalog* sc = SecondoSystem::GetCatalog();
            ListExpr numtypeinfo = sc->NumericType(typeinfo);
            collection::Collection* coll = new collection::Collection(
                collection::vector, numtypeinfo);
            rec = new Record(2);
            rec->SetElement(0, new CcInt(true, 42), "int", "Art");
            rec->SetElement(1, new CcString(true, "4711"), "string"
            , "Nummer");
            coll->Insert(rec, 2);
            coll->SetDefined(true);
            t->PutAttribute(2, coll);
            
            tt->DeleteIfAllowed();
            VTuple* vt = new VTuple(t, td);
            return vt;
        }

        // ((Name string)(Adresse (record(Nummer int)))
        // (Tel (vector(record(Art int)))))
        static VTuple* Create4_1() {
            TupleDescr* td = new TupleDescr("((Name string)(Adresse "
            "(record(Nummer int)))(Tel (vector(record(Art int)))))");
            TupleType* tt = td->CreateTupleType();
            Tuple* t = new Tuple(tt);
            t->PutAttribute(0, new CcString(true, "Maysami"));
            
            Record* rec = new Record(1);
            rec->SetElement(0, new CcInt(true, 34), "int", "Nummer");
            t->PutAttribute(1, rec);

            ListExpr typeinfo;
            nl->ReadFromString("(vector(record((Art int))))"
            , typeinfo);
            SecondoCatalog* sc = SecondoSystem::GetCatalog();
            ListExpr numtypeinfo = sc->NumericType(typeinfo);
            collection::Collection* coll = new collection::Collection(
                collection::vector, numtypeinfo);
            rec = new Record(1);
            rec->SetElement(0, new CcInt(true, 42), "int", "Art");
            coll->Insert(rec, 1);
            coll->SetDefined(true);
            t->PutAttribute(2, coll);
            
            tt->DeleteIfAllowed();
            VTuple* vt = new VTuple(t, td);
            return vt;
        }

        // ((Id int)(V1(vector int)))
        static VTuple* Create5() {
            TupleDescr* td = new TupleDescr("((Id int)(V1(vector int)))");
            TupleType* tt = td->CreateTupleType();
            Tuple* t = new Tuple(tt);

            t->PutAttribute(0, new CcInt(true, 42));

            ListExpr typeinfo;
            //nl->ReadFromString("(vector(int))"
            nl->ReadFromString("(vector int)"
            , typeinfo);
            SecondoCatalog* sc = SecondoSystem::GetCatalog();
            ListExpr numtypeinfo = sc->NumericType(typeinfo);
            LOG << "Create5: " << nl->ToString(numtypeinfo) << ENDL;
            collection::Collection* coll = new collection::Collection(
                collection::vector, numtypeinfo);

            coll->Insert(new CcInt(true, 1), 1);
            coll->SetDefined(true);

            t->PutAttribute(1, coll);

            tt->DeleteIfAllowed();
            VTuple* vt = new VTuple(t, td);
            return vt;
        }

        // ((V1(vector int)))
        static VTuple* Create5_1() {
            TupleDescr* td = new TupleDescr("((V1(vector int)))");
            TupleType* tt = td->CreateTupleType();
            Tuple* t = new Tuple(tt);

            ListExpr typeinfo;
            //nl->ReadFromString("(vector(int))"
            nl->ReadFromString("(vector int)"
            , typeinfo);
            SecondoCatalog* sc = SecondoSystem::GetCatalog();
            ListExpr numtypeinfo = sc->NumericType(typeinfo);
            collection::Collection* coll = new collection::Collection(
                collection::vector, numtypeinfo);

            coll->Insert(new CcInt(true, 1), 1);
            coll->SetDefined(true);

            t->PutAttribute(0, coll);

            tt->DeleteIfAllowed();
            VTuple* vt = new VTuple(t, td);
            return vt;
        }

    };

} /* end of namespace */

#endif
