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

[1] Implementation of operation foreverStream.

[toc]

1 Operation foreverStream implementation

Produces an optionally never ending stream of tuples of the 
format...

*/


#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Stream.h"
#include "QueryProcessor.h"
#include "ListUtils.h"

#include "ForeverHelper.h"
#include "Protocols.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;


namespace continuousqueries {

/*
1.1 Operation foreverStream TypeMapping

*/

ListExpr foreverStream_TM(ListExpr args) {
    // the list is coded as ( (<type> <query part>) (<type> <query part>) )

    std::cout << "foreverStream: TypeMapping" << endl;
    std::cout << "Argument: " << nl->ToString(args) << endl;

    // Check for int x int -> stream(tuple())
    if (!nl->HasLength(args, 3)) 
        return listutils::typeError(" three arguments are expected");
    
    ListExpr arg1Type = nl->First(args);
    ListExpr arg2Type = nl->Second(args);
    ListExpr arg3Type = nl->Third(args);

    // Check first argument (tuple volume)
    if (!nl->HasLength(arg1Type, 2))
        return listutils::typeError(" internal error (tuple volume) ");

    if(!CcInt::checkType(nl->First(arg1Type)))
        return listutils::typeError(" (tuple volume) not an CcInt ");

    if(!listutils::isNumeric(nl->Second(arg1Type)))
        return listutils::typeError(
            " (tuple volume) can't be converted to numeric "
        );

    // Check second argument (tuple type)
    if (!nl->HasLength(arg2Type, 2))
        return listutils::typeError(" internal error (tuple type) ");

    if(!CcInt::checkType(nl->First(arg2Type)))
        return listutils::typeError(" (tuple type) not an CcInt ");

    if(!listutils::isNumeric(nl->Second(arg2Type)))
        return listutils::typeError(
            " (tuple type) can't be converted to numeric "
        );

    // Check third argument (chance of undefined)
    if (!nl->HasLength(arg3Type, 2))
        return listutils::typeError(" internal error (chance of undefined) ");

    if(!CcInt::checkType(nl->First(arg3Type)))
        return listutils::typeError(" (chance of undefined) not an CcInt ");

    if(!listutils::isNumeric(nl->Second(arg3Type)))
        return listutils::typeError(
            " (chance of undefined) can't be converted to numeric "
        );

    if ((listutils::getNumValue(nl->Second(arg3Type)) < -1) ||
        (listutils::getNumValue(nl->Second(arg3Type)) > 100)) {
            return listutils::typeError(
                " (chance of undefined) between -1 and 100 expected "
            );
        }

    // Return Stream based on choosen tuple type
    if (listutils::getNumValue(nl->Second(arg2Type)) == 0) 
        return  nl->TwoElemList( listutils::basicSymbol<Stream<Tuple>>(),
                    nl->TwoElemList( listutils::basicSymbol<Tuple>(),
                        nl->TwoElemList(
                            nl->TwoElemList( nl->SymbolAtom("I"),
                            listutils::basicSymbol<CcInt>()),
                            nl->TwoElemList( nl->SymbolAtom("S"),
                            listutils::basicSymbol<CcString>())
                        )
                    )
                );
    
    if (listutils::getNumValue(nl->Second(arg2Type)) == 1) 
        return  nl->TwoElemList( listutils::basicSymbol<Stream<Tuple>>(),
                    nl->TwoElemList( listutils::basicSymbol<Tuple>(),
                        nl->SixElemList(
                            nl->TwoElemList( nl->SymbolAtom("Id"),
                                listutils::basicSymbol<CcString>()),
                            nl->TwoElemList( nl->SymbolAtom("City"),
                                listutils::basicSymbol<CcString>()),
                            nl->TwoElemList( nl->SymbolAtom("Month"),
                                listutils::basicSymbol<CcString>()),
                            nl->TwoElemList( nl->SymbolAtom("Number"),
                                listutils::basicSymbol<CcInt>()),
                            nl->TwoElemList( nl->SymbolAtom("Fraction"),
                                listutils::basicSymbol<CcReal>()),
                            nl->TwoElemList( nl->SymbolAtom("Valid"),
                                listutils::basicSymbol<CcBool>())
                        )
                    )
                );

    if (listutils::getNumValue(nl->Second(arg2Type)) == 2) 
        return  nl->TwoElemList( listutils::basicSymbol<Stream<Tuple>>(),
                    nl->TwoElemList( listutils::basicSymbol<Tuple>(),
                        nl->FourElemList(
                            nl->TwoElemList( nl->SymbolAtom("I_eq"),
                            listutils::basicSymbol<CcInt>()),
                            nl->TwoElemList( nl->SymbolAtom("I_gt"),
                            listutils::basicSymbol<CcInt>()),
                            nl->TwoElemList( nl->SymbolAtom("I_lt"),
                            listutils::basicSymbol<CcInt>()),
                            nl->TwoElemList( nl->SymbolAtom("S_eq"),
                            listutils::basicSymbol<CcString>())
                        )
                    )
                );

    if (listutils::getNumValue(nl->Second(arg2Type)) == 3)
    {
        ListExpr res = nl->OneElemList(
            nl->TwoElemList( nl->SymbolAtom("Id_eq"),
                listutils::basicSymbol<CcString>()));

        ListExpr last = res;

        last = nl->Append(last, nl->TwoElemList( nl->SymbolAtom("City_eq"),
            listutils::basicSymbol<CcString>()));
        last = nl->Append(last, nl->TwoElemList( nl->SymbolAtom("Month_eq"),
            listutils::basicSymbol<CcString>()));

        last = nl->Append(last, nl->TwoElemList( nl->SymbolAtom("Number_eq"),
            listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList( nl->SymbolAtom("Number_gt"),
            listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList( nl->SymbolAtom("Number_lt"),
            listutils::basicSymbol<CcInt>()));

        last = nl->Append(last, nl->TwoElemList( nl->SymbolAtom("Fraction_eq"),
            listutils::basicSymbol<CcReal>()));
        last = nl->Append(last, nl->TwoElemList( nl->SymbolAtom("Fraction_gt"),
            listutils::basicSymbol<CcReal>()));
        last = nl->Append(last, nl->TwoElemList( nl->SymbolAtom("Fraction_lt"),
            listutils::basicSymbol<CcReal>()));

        last = nl->Append(last, nl->TwoElemList( nl->SymbolAtom("Valid_eq"),
            listutils::basicSymbol<CcBool>()));

        return  nl->TwoElemList( listutils::basicSymbol<Stream<Tuple>>(),
                    nl->TwoElemList( listutils::basicSymbol<Tuple>(),
                        res
                    )
                );
    }

    return listutils::typeError(" (tuple type) between 0 and 3 expected ");
}

std::string RelationToString(Relation* rel)
{
    std::stringstream out;

    int i = 0;
    Tuple* t = 0;
    GenericRelationIterator* it = rel->MakeScan();

    while ((t = it->GetNextTuple()) != 0)
    {
        for(int ii = 0; ii < t->GetNoAttributes(); ii++) 
        {
            if (ii>0 || i>0) out << ", ";
            t->GetAttribute(ii)->Print(out);
        }
        
        t->DeleteIfAllowed();
        t = 0;
        i++;
    }

    delete it;
    it = 0;

    return out.str();
}

class foreverStream_LI{
  public:
    foreverStream_LI(int volume, int type, int chanceOfUndefined, 
                                            TupleType* resultTupleType):
        _volume(volume), 
        _type(type),
        _chanceOfUndefined(chanceOfUndefined),
        _count(1),
        _resultTupleType(resultTupleType) {}

    ~foreverStream_LI()
    {
        _resultTupleType->DeleteIfAllowed();
    }

    Tuple* getNext()
    {
        if (_volume && _count > _volume) return 0;

        Tuple* t = new Tuple(_resultTupleType);
        int v1;
        int v2;
        double d1;
        double d2;
        int queryType;

        if (_type == 0)
        {
            t->PutAttribute(0,  new CcInt(
                ForeverHelper::getBool(_chanceOfUndefined), 
                ForeverHelper::getInt()));
            t->PutAttribute(1,  new CcString(
                ForeverHelper::getBool(_chanceOfUndefined), 
                ForeverHelper::getSmallString()));
        } else

        if (_type == 1)
        {
            t->PutAttribute(0,  new CcString(
                ForeverHelper::getBool(_chanceOfUndefined), 
                ForeverHelper::getUniqueId()));
            t->PutAttribute(1,  new CcString(
                ForeverHelper::getBool(_chanceOfUndefined), 
                ForeverHelper::getBigString()));
            t->PutAttribute(2,  new CcString(
                ForeverHelper::getBool(_chanceOfUndefined), 
                ForeverHelper::getSmallString()));
            t->PutAttribute(3,  new CcInt(
                ForeverHelper::getBool(_chanceOfUndefined), 
                ForeverHelper::getInt()));
            t->PutAttribute(4,  new CcReal(
                ForeverHelper::getBool(_chanceOfUndefined), 
                ForeverHelper::getDouble()));
            t->PutAttribute(5,  new CcBool(
                ForeverHelper::getBool(_chanceOfUndefined), 
                ForeverHelper::getBool()));
        } else

        if (_type == 2)
        {
            // There are 9 possible query types with a maximum of 2 ands:
            // I=?, I>?, I<?, S=?
            // I=? and S=?
            // I>? and I<? -- first ? has to be smaller than second -> between
            // I>? and S=?
            // I<? and S=?
            // I>? and I<? and S=?
            // chanceOfUndefined means here: chance for a easy query 
            // (where only one attribute is tested).

            if (!ForeverHelper::getBool(_chanceOfUndefined))
            {
                queryType = rand() % 4 + 1;
            } else {
                queryType = 4 + rand() % 5 + 1;
            }
            
            v1 = ForeverHelper::getInt(98);
            v2 = v1 + ForeverHelper::getInt(100 - v1);
            
            switch (queryType)
            {
                case 1: // I=?
                    t->PutAttribute(0,  new CcInt(
                        true, 
                        ForeverHelper::getInt()));
                    t->PutAttribute(1,  new CcInt(
                        false,
                        ForeverHelper::getInt()));
                    t->PutAttribute(2,  new CcInt(
                        false, 
                        ForeverHelper::getInt()));
                    t->PutAttribute(3,  new CcString(
                        false, 
                        ForeverHelper::getSmallString()));
                    break;

                case 2: // I>?
                    t->PutAttribute(0,  new CcInt(
                        false, 
                        ForeverHelper::getInt()));
                    t->PutAttribute(1,  new CcInt(
                        true,
                        ForeverHelper::getInt()));
                    t->PutAttribute(2,  new CcInt(
                        false, 
                        ForeverHelper::getInt()));
                    t->PutAttribute(3,  new CcString(
                        false, 
                        ForeverHelper::getSmallString()));
                    break;

                case 3: // I<?
                    t->PutAttribute(0,  new CcInt(
                        false, 
                        ForeverHelper::getInt()));
                    t->PutAttribute(1,  new CcInt(
                        false,
                        ForeverHelper::getInt()));
                    t->PutAttribute(2,  new CcInt(
                        true, 
                        ForeverHelper::getInt()));
                    t->PutAttribute(3,  new CcString(
                        false, 
                        ForeverHelper::getSmallString()));
                    break;

                case 4: // S=?
                    t->PutAttribute(0,  new CcInt(
                        false, 
                        ForeverHelper::getInt()));
                    t->PutAttribute(1,  new CcInt(
                        false,
                        ForeverHelper::getInt()));
                    t->PutAttribute(2,  new CcInt(
                        false, 
                        ForeverHelper::getInt()));
                    t->PutAttribute(3,  new CcString(
                        true, 
                        ForeverHelper::getSmallString()));
                    break;
                
                case 5: // I=? and S=?
                    t->PutAttribute(0,  new CcInt(
                        true, 
                        ForeverHelper::getInt()));
                    t->PutAttribute(1,  new CcInt(
                        false,
                        ForeverHelper::getInt()));
                    t->PutAttribute(2,  new CcInt(
                        false, 
                        ForeverHelper::getInt()));
                    t->PutAttribute(3,  new CcString(
                        true, 
                        ForeverHelper::getSmallString()));
                    break;

                case 6: // I>? and I<? -- 1st ? smaller than 2nd -> between
                    t->PutAttribute(0,  new CcInt(
                        false, 
                        ForeverHelper::getInt()));
                    t->PutAttribute(1,  new CcInt(
                        true,
                        v1));
                    t->PutAttribute(2,  new CcInt(
                        true, 
                        v2));
                    t->PutAttribute(3,  new CcString(
                        false, 
                        ForeverHelper::getSmallString()));
                    break;
                
                case 7: // I>? and S=?
                    t->PutAttribute(0,  new CcInt(
                        false, 
                        ForeverHelper::getInt()));
                    t->PutAttribute(1,  new CcInt(
                        true,
                        ForeverHelper::getInt()));
                    t->PutAttribute(2,  new CcInt(
                        false, 
                        ForeverHelper::getInt()));
                    t->PutAttribute(3,  new CcString(
                        true, 
                        ForeverHelper::getSmallString()));
                    break;
                
                case 8: // I<? and S=?
                    t->PutAttribute(0,  new CcInt(
                        false, 
                        ForeverHelper::getInt()));
                    t->PutAttribute(1,  new CcInt(
                        false,
                        ForeverHelper::getInt()));
                    t->PutAttribute(2,  new CcInt(
                        true, 
                        ForeverHelper::getInt()));
                    t->PutAttribute(3,  new CcString(
                        true, 
                        ForeverHelper::getSmallString()));
                    break;
                
                case 9: // I>? and I<? and S=?
                    t->PutAttribute(0,  new CcInt(
                        false, 
                        ForeverHelper::getInt()));
                    t->PutAttribute(1,  new CcInt(
                        true,
                        v1));
                    t->PutAttribute(2,  new CcInt(
                        true, 
                        v2));
                    t->PutAttribute(3,  new CcString(
                        true, 
                        ForeverHelper::getSmallString()));
                    break;

                default:
                    break;
            }

        } else

        if (_type == 3)
        {
            bool choosen[10] = {false, false, false, false, false,
                false, false, false, false, false};

            if (_chanceOfUndefined == -1) 
            {
                for(size_t i = 0; i < 10; i++)
                {
                    choosen[i] = true;
                }
            }

            v1 = ForeverHelper::getInt();
            v2 = ForeverHelper::getInt();
            d1 = ForeverHelper::getDouble();
            d2 = ForeverHelper::getDouble();

            if ((_chanceOfUndefined == -1) || 
                (!ForeverHelper::getBool(_chanceOfUndefined)))
            {                       // test one attribute
                choosen[rand() % 10] = true;
            } else {
                queryType = ForeverHelper::getInt(53);
                
                if (queryType > 420) // test two attributes
                {
                    bool invalid;
                    int test1;
                    int test2;
                    do {
                        invalid = false;
                        test1 = rand() % 10;
                        test2 = rand() % 10;
                                                
                        if (test1 == test2) invalid = true;
                        if ((test1 == 3 && (test2 == 4 || test2 == 5)) ||
                            (test2 == 3 && (test1 == 4 || test1 == 5))) {
                            invalid = true;
                        }
                        if ((test1 == 6 && (test2 == 7 || test2 == 8)) ||
                            (test2 == 6 && (test1 == 7 || test1 == 8))) {
                            invalid = true;
                        }
                    } while (invalid);
                    
                    if ((test1==4 && test2==5) || (test1==5 && test2==4)) {
                        v1 = ForeverHelper::getInt(98);
                        v2 = v1 + ForeverHelper::getInt(100 - v1);
                    }

                    if ((test1==7 && test2==8) || (test1==8 && test2==7)) {
                        d1 = ForeverHelper::getDouble(5);
                        d2 = ForeverHelper::getDouble(5) + d1;
                    }

                    choosen[test1] = true;
                    choosen[test2] = true;
                } else {            // test three attributes
                    bool invalid;
                    int test1;
                    int test2;
                    int test3;

                    do {
                        invalid = false;
                        test1 = rand() % 10;
                        test2 = rand() % 10;
                        test3 = rand() % 10;
                                                
                        if (test1 == test2) invalid = true;
                        if (test2 == test3) invalid = true;
                        if (test3 == test1) invalid = true;

                        if ((test1 == 3 && (test2 == 4 || test2 == 5 || 
                                test3 == 4 || test3 == 5)) ||
                            (test2 == 3 && (test1 == 4 || test1 == 5 || 
                                test3 == 4 || test3 == 5)) ||
                            (test3 == 3 && (test1 == 4 || test1 == 5 || 
                                test2 == 4 || test2 == 5))) {
                            invalid = true;
                        }

                        if ((test1 == 6 && (test2 == 7 || test2 == 8 || 
                                test3 == 7 || test3 == 8)) ||
                            (test2 == 6 && (test1 == 7 || test1 == 8 || 
                                test3 == 7 || test3 == 8)) ||
                            (test3 == 6 && (test1 == 7 || test1 == 8 || 
                                test2 == 7 || test2 == 8))) {
                            invalid = true;
                        }
                    } while (invalid);
                    
                    if (((test1==4 && test2==5) || (test1==5 && test2==4)) || 
                        ((test1==4 && test3==5) || (test1==5 && test3==4)) || 
                        ((test2==4 && test3==5) || (test2==5 && test3==4)))
                    {
                        v1 = ForeverHelper::getInt(98);
                        v2 = v1 + ForeverHelper::getInt(100 - v1);
                    }

                    if (((test1==7 && test2==8) || (test1==8 && test2==7)) || 
                        ((test1==7 && test3==8) || (test1==8 && test3==7)) || 
                        ((test2==7 && test3==8) || (test2==8 && test3==7)))
                    {
                        d1 = ForeverHelper::getDouble(5);
                        d2 = ForeverHelper::getDouble(5) + d1;
                    }

                    choosen[test1] = true;
                    choosen[test2] = true;
                    choosen[test3] = true;
                }
            }

            t->PutAttribute(0,  new CcString(                   // ID_eq
                choosen[0], 
                ForeverHelper::getUniqueId()));
            t->PutAttribute(1,  new CcString(                   // City_eq
                choosen[1], 
                ForeverHelper::getBigString()));
            t->PutAttribute(2,  new CcString(                   // Month_eq
                choosen[2], 
                ForeverHelper::getSmallString()));
            t->PutAttribute(3,  new CcInt(                      // Number_eq
                choosen[3], 
                ForeverHelper::getInt()));
            t->PutAttribute(4,  new CcInt(                      // Number_gt
                choosen[4], 
                v1));
            t->PutAttribute(5,  new CcInt(                      // Number_lt
                choosen[5], 
                v2));
            t->PutAttribute(6,  new CcReal(                     // Fraction_eq
                choosen[6], 
                ForeverHelper::getDouble()));
            t->PutAttribute(7,  new CcReal(                     // Fraction_gt
                choosen[7], 
                d1));
            t->PutAttribute(8,  new CcReal(                     // Fraction_lt
                choosen[8],
                d2));
            t->PutAttribute(9,  new CcBool(                     // Valid_eq
                choosen[9], 
                ForeverHelper::getBool()));

        }

        if (_volume) _count++;

        return t;
    }

  private:
    int _volume;
    int _type;
    int _chanceOfUndefined;
    int _count;
    TupleType* _resultTupleType;
};

/*
1.2 Operation foreverStream ValueMappings

*/
int foreverStream_VM( Word* args, Word& result, int message,
    Word& local, Supplier s ) 
{
    foreverStream_LI* li = (foreverStream_LI*) local.addr;
    
    ListExpr resultType;
    TupleType* resultTupleType;

    switch (message) {
    case OPEN :
        std::cout << "foreverStream: ValueMapping: OPEN" << endl;
        std::cout << "Creating an optionally never ending stream." << endl;
        std::cout << "Volume: " << ((CcInt*) args[0].addr)->GetValue() << " | "
                  << "Type: "   << ((CcInt*) args[1].addr)->GetValue() << " | "
                  << "Chance of Undefined: " 
                  << ((CcInt*) args[2].addr)->GetValue() << "%" << endl;
        
        if (li) {
            delete li;
        }
    
        resultType = GetTupleResultType( s );
        resultTupleType = new TupleType( nl->Second( resultType ) );

        local.addr = new foreverStream_LI( 
            ((CcInt*) args[0].addr)->GetValue(), 
            ((CcInt*) args[1].addr)->GetValue(), 
            ((CcInt*) args[2].addr)->GetValue(), 
            resultTupleType
        );

        return 0;

    case REQUEST:
        std::cout << "foreverStream: ValueMapping: REQUEST" << endl;
        
        result.addr = li ? li->getNext() : 0;
        return result.addr ? YIELD : CANCEL;

    case CLOSE:
        std::cout << "foreverStream: ValueMapping: CLOSE" << endl;

        if(li) {
            delete li;
            local.addr = 0;
        }

        // if (resultTupleType) resultTupleType->DeleteIfAllowed();

        return 0;
    }

    return 0;
}

/*
1.3 Operation foreverStream operator selection array

*/
            
ValueMapping foreverStream[] = {
    foreverStream_VM
};

/*
1.4 Operation foreverStream operator selection

*/

int foreverStream_Select(ListExpr args) {
    return 0;

    // if (listutils::isTupleStream(nl->First(args)))
    //     return 0;
    // if (VTHelpers::IsVTupleStream(nl->First(args)))
    //     return 1;
    // return -1;
}

/*
1.5 Operation foreverStream operator specification

*/

const std::string foreverStreamOpSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>int x int x int -> stream(tuple)</text--->"
"<text>foreverStream ( _, _, _ )</text--->"
"<text>Creates an (optionally) never ending stream"
" of tuples. Two tuple variants implemented.</text--->"
"<text>query foreverStream(100, 0, 0) count"
"</text--->"
") )";


/*
1.6 Operation foreverStream

*/

Operator foreverStream_Op(
    "foreverStream",
    foreverStreamOpSpec,
    1,
    foreverStream,
    foreverStream_Select,
    foreverStream_TM
);

} /* end of namespace */
