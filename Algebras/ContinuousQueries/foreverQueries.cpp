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

[1] Implementation of operation foreverQueries.

[toc]

1 Operation foreverQueries implementation

Produces an optionally never ending stream of tuples of the 
format...

*/


#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/Stream/Stream.h"
#include "QueryProcessor.h"
#include "ListUtils.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "sha1.hpp"
#include "SecParser.h"
#include "Protocols.h"
#include "ForeverHelper.h"
#include "Tcp/TcpClient.h"
#include <thread>
#include <chrono>
#include <boost/algorithm/string.hpp>

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;


namespace continuousqueries {

/*
1.1 Operation foreverQueries TypeMapping

*/

ListExpr foreverQueries_TM(ListExpr args) {
    std::cout << "foreverQueries: TypeMapping" << endl;
    std::cout << "Argument: " << nl->ToString(args) << endl;

    // Check for text x int x text x int x int -> text
    if (!nl->HasLength(args, 5))
        return listutils::typeError(" five arguments are expected");
    
    ListExpr arg1Type = nl->First(args);
    ListExpr arg2Type = nl->Second(args);
    ListExpr arg3Type = nl->Third(args);
    ListExpr arg4Type = nl->Fourth(args);
    ListExpr arg5Type = nl->Fifth(args);

    // Check first argument (address/text)
    if (!nl->HasLength(arg1Type, 2))
        return listutils::typeError(" internal error (address) ");

    if (!FText::checkType(nl->First(arg1Type)))
        return listutils::typeError(" address as text is expected");

    // Check second argument (valid portnumber/int)
    if (!nl->HasLength(arg2Type, 2))
        return listutils::typeError(" internal error (portnumber) ");

    if(!CcInt::checkType(nl->First(arg2Type)))
        return listutils::typeError(" portnummer not an CcInt ");

    if(!listutils::isNumeric(nl->Second(arg2Type)))
        return listutils::typeError(
            " portnumber can't be converted to numeric "
        );

    if(listutils::getNumValue(nl->Second(arg2Type)) < 0 || 
        listutils::getNumValue(nl->Second(arg2Type)) > 65535 )
        return listutils::typeError(
            " portnumber between 0 and 65535 expected "
        );

    // Check third argument (email/text)
    if (!nl->HasLength(arg3Type, 2))
        return listutils::typeError(" internal error (email) ");

    if (!FText::checkType(nl->First(arg3Type)))
        return listutils::typeError(" email as text is expected");
    
    // Check fourth argument (volume/int)
    if (!nl->HasLength(arg4Type, 2))
        return listutils::typeError(" internal error (volume) ");

    if(!CcInt::checkType(nl->First(arg4Type)))
        return listutils::typeError(" volume not an CcInt ");

    if(!listutils::isNumeric(nl->Second(arg4Type)))
        return listutils::typeError(
            " volume can't be converted to numeric "
        );

    if(listutils::getNumValue(nl->Second(arg4Type)) < 0)
        return listutils::typeError(
            " volume greater or equal 0 expected "
        );

    // Check fifth argument (type/int)
    if (!nl->HasLength(arg5Type, 2))
        return listutils::typeError(" internal error (type) ");

    if(!CcInt::checkType(nl->First(arg5Type)))
        return listutils::typeError(" type not an CcInt ");

    if(!listutils::isNumeric(nl->Second(arg5Type)))
        return listutils::typeError(
            " type can't be converted to numeric "
        );

    if(listutils::getNumValue(nl->Second(arg5Type)) < 0 || 
        listutils::getNumValue(nl->Second(arg5Type)) > 3 )
        return listutils::typeError(
            " type should between 0 and 3 "
        );
    
    return arg1Type;
}

class foreverQueries_LI {
  public:
    foreverQueries_LI(int volume, int type):
        _volume(volume), _type(type), _count(1), _currentTuple(1001) 
        {
        }

    ~foreverQueries_LI()
    {}

    bool refreshRelation()
    {
        std::string exestring = "";
        Word exeword;

        if(_type==0||_type==2) exestring="(consume (foreverStream 1000 2 20))";
        if(_type==1||_type==3) exestring="(consume (foreverStream 1000 3 20))";

        if (!QueryProcessor::ExecuteQuery(exestring, exeword)) return false;
        _result = (Relation*) exeword.addr;

        return true;
    }

    std::string getNext()
    {
        if (_volume && (_count > _volume)) return "";

        if (_currentTuple == 1001)
        {
            if (!refreshRelation()) return "error";
            _currentTuple = 1;
        }

        Tuple* currentTuple = _result->GetTuple(_currentTuple, true);
        std::string query = "";
                
        if (_type == 0)
        {
            std::string parts[3];
            parts[0] = "";
            parts[1] = "";
            parts[2] = "";

            int count = 0;

            if (currentTuple->GetAttribute(0)->toText() != "-")
            {
                parts[count] = "(=(attr t I)" + currentTuple
                    ->GetAttribute(0)->toText() + ")";
                count++;
            }

            if (currentTuple->GetAttribute(1)->toText() != "-")
            {
                parts[count] = "(>(attr t I)" + currentTuple
                    ->GetAttribute(1)->toText() + ")";
                count++;
            }

            if (currentTuple->GetAttribute(2)->toText() != "-")
            {
                parts[count] = "(<(attr t I)" + currentTuple
                    ->GetAttribute(2)->toText() + ")";
                count++;
            }

            if (currentTuple->GetAttribute(3)->toText() != "-")
            {
                parts[count] = "(=(attr t S)\"" + currentTuple
                    ->GetAttribute(3)->toText() + "\")";
                count++;
            }

            if (count == 1)
                query = "(fun(t(tuple((I int)(S string))))" + parts[0] + ")";

            if (count == 2)
                query = "(fun(t(tuple((I int)(S string))))(and" 
                    + parts[0] + parts[1] + "))";

            if (count == 3)
                query = "(fun(t(tuple((I int)(S string))))(and" 
                    + parts[0] + "(and" + parts[1] + parts[2] + ")))";
        }

        if (_type == 1)
        {
            std::string parts[3];
            parts[0] = "";
            parts[1] = "";
            parts[2] = "";

            int count = 0;

            if (currentTuple->GetAttribute(1)->toText() != "-")
            {
                parts[count] = "(=(attr t City)\"" + currentTuple
                    ->GetAttribute(1)->toText() + "\")";
                count++;
            }

            if (currentTuple->GetAttribute(2)->toText() != "-")
            {
                parts[count] = "(=(attr t Month)\"" + currentTuple
                    ->GetAttribute(2)->toText() + "\")";
                count++;
            }

            if (currentTuple->GetAttribute(3)->toText() != "-")
            {
                parts[count] = "(=(attr t Number)" + currentTuple
                    ->GetAttribute(3)->toText() + ")";
                count++;
            }

            if (currentTuple->GetAttribute(4)->toText() != "-")
            {
                parts[count] = "(>(attr t Number)" + currentTuple
                    ->GetAttribute(4)->toText() + ")";
                count++;
            }

            if (currentTuple->GetAttribute(5)->toText() != "-")
            {
                parts[count] = "(<(attr t Number)" + currentTuple
                    ->GetAttribute(5)->toText() + ")";
                count++;
            }

            if (currentTuple->GetAttribute(6)->toText() != "-")
            {
                parts[count] = "(=(attr t Fraction)" + currentTuple
                    ->GetAttribute(6)->toText() + ")";
                count++;
            }

            if (currentTuple->GetAttribute(7)->toText() != "-")
            {
                parts[count] = "(>(attr t Fraction)" + currentTuple
                    ->GetAttribute(7)->toText() + ")";
                count++;
            }

            if (currentTuple->GetAttribute(8)->toText() != "-")
            {
                parts[count] = "(<(attr t Fraction)" + currentTuple
                    ->GetAttribute(8)->toText() + ")";
                count++;
            }
            
            if (currentTuple->GetAttribute(9)->toText() != "-")
            {
                parts[count] = "(=(attr t Valid)" + boost::to_upper_copy(
                    currentTuple->GetAttribute(9)->toText()) + ")";
                count++;
            }

            if (count == 0) {
                count = 1;
                parts[0] = "(=(attr t City)\"" 
                    + ForeverHelper::getBigString() + "\")";
            }



            if (count == 1)
                query = "(fun(t(tuple((Id string)(City string)(Month string)"
                        "(Number int)(Fraction real)(Valid bool))))"
                        + parts[0] + ")";

            if (count == 2)
                query = "(fun(t(tuple((Id string)(City string)(Month string)"
                        "(Number int)(Fraction real)(Valid bool))))(and" 
                        + parts[0] + parts[1] + "))";

            if (count == 3)
                query = "(fun(t(tuple((Id string)(City string)(Month string)"
                        "(Number int)(Fraction real)(Valid bool))))(and" 
                        + parts[0] + "(and" + parts[1] + parts[2] + ")))";
        }

        if (_type == 2)
        {
            query += "(";
            
            if (currentTuple->GetAttribute(0)->toText() != "-")
                query += "(I_eq " + currentTuple->GetAttribute(0)
                    ->toText() + ")";
                    
            if (currentTuple->GetAttribute(1)->toText() != "-")
                query += "(I_gt " + currentTuple->GetAttribute(1)
                    ->toText() + ")";
            
            if (currentTuple->GetAttribute(2)->toText() != "-")
                query += "(I_lt " + currentTuple->GetAttribute(2)
                    ->toText() + ")";

            if (currentTuple->GetAttribute(3)->toText() != "-")
                query += "(S_eq \"" + currentTuple
                    ->GetAttribute(3)->toText() + "\")";

            query += ")";
        }

        if (_type == 3)
        {
            query += "(";
            
            if (currentTuple->GetAttribute(1)->toText() != "-")
                query += "(City_eq \"" + currentTuple
                    ->GetAttribute(1)->toText() + "\")";
                    
            if (currentTuple->GetAttribute(2)->toText() != "-")
                query += "(Month_eq \"" + currentTuple
                    ->GetAttribute(2)->toText() + "\")";
            
            if (currentTuple->GetAttribute(3)->toText() != "-")
                query += "(Number_eq " + currentTuple
                    ->GetAttribute(3)->toText() + ")";

            if (currentTuple->GetAttribute(4)->toText() != "-")
                query += "(Number_gt " + currentTuple
                    ->GetAttribute(4)->toText() + ")";

            if (currentTuple->GetAttribute(5)->toText() != "-")
                query += "(Number_lt " + currentTuple
                    ->GetAttribute(5)->toText() + ")";
                    
            if (currentTuple->GetAttribute(6)->toText() != "-")
                query += "(Fraction_eq " + currentTuple
                    ->GetAttribute(6)->toText() + ")";
            
            if (currentTuple->GetAttribute(7)->toText() != "-")
                query += "(Fraction_gt " + currentTuple
                    ->GetAttribute(7)->toText() + ")";

            if (currentTuple->GetAttribute(8)->toText() != "-")
                query += "(Fraction_lt " + currentTuple
                    ->GetAttribute(8)->toText() + ")";
            
            if (currentTuple->GetAttribute(9)->toText() != "-")
                query += "(Valid_eq " + boost::to_upper_copy(
                    currentTuple->GetAttribute(9)->toText()) + ")";

            query += ")";
        }

        // if the test was only on the QID, test instead on the city.
        if (query.length() == 2)
            query = "((City_eq \"" + ForeverHelper::getBigString() + "\"))";

        if (_volume) _count++;
        _currentTuple++;

        return query;
    }

    int _volume;
    int _type;
    int _count;
    int _currentTuple;
    Relation* _result;
};

/*
1.2 Operation foreverQueries ValueMappings

*/
int foreverQueries_VM( Word* args, Word& result, int message,
    Word& local, Supplier s ) 
{
    std::cout << "foreverQueries: ValueMapping" << endl;

    FText*  ftaddress = static_cast<FText*>(args[0].addr);  
    CcInt*  ccport    = (CcInt*) args[1].addr; 
    FText*  ftemail   = static_cast<FText*>(args[2].addr);  
    CcInt*  ccvolume  = (CcInt*) args[3].addr; 
    CcInt*  cctype    = (CcInt*) args[4].addr; 

    foreverQueries_LI* li = new foreverQueries_LI(
        ccvolume->GetValue(),
        cctype->GetValue()
    );

    SHA1 sha;
    sha.update(ftemail->GetValue() + "password");
    std::string hash = sha.final();

    // create tcp client for coordinator
    TcpClient client(ftaddress->GetValue(), ccport->GetValue());

    client.Initialize();

    // register user
    client.Send(CoordinatorGenP::userauth() + "|" + hash + "|" 
        + ftemail->GetValue() + "|register");
    
    std::cout << endl << "Log in to the webinterface with username '" 
              << ftemail->GetValue() << "' and password 'password'." << endl;

    std::string lastQuery = "";
    std::string newQuery = "";

    newQuery = li->getNext();
    
    while (newQuery != "")
    {
        lastQuery = newQuery;
        
        client.Send(CoordinatorGenP::addquery(0, "", false) + "|" + hash
            + "|" + newQuery);

        std::cout << "Added Query: " << newQuery << endl;

        newQuery = li->getNext();

        if (newQuery!="")
        {
            std::cout << "Now waiting for 20 milliseconds..." << endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    } // repeat until li yields ""

    std::cout << "All done!" << endl;

    client.Shutdown();

    result = qp->ResultStorage(s);
    FText* res = (FText*) result.addr;
    res->Set(true, lastQuery);
    delete li;
    return 0;
}

/*
1.3 Operation foreverQueries operator selection array

*/
            
ValueMapping foreverQueries[] = {
    foreverQueries_VM
};

/*
1.4 Operation foreverQueries operator selection

*/

int foreverQueries_Select(ListExpr args) {
    return 0;
}

/*
1.5 Operation foreverQueries operator specification

*/

const std::string foreverQueriesOpSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>text x int x text x int x int -> text</text--->"
"<text>foreverQueries ( _, _, _, _, _ )</text--->"
"<text>(address, port, email, volume (0->infinite), type)"
" Creates an (optionally) infinite number of user"
" queries for the Secondo Stream Processor. Two tuple "
"variants implemented. To see the queries in the web inter"
"face log in with the specified email and the password 'pa"
"ssword'. Returns the last added query.</text--->"
"<text>query foreverQueries('127.0.0.1', 12300, "
"'test-ssp@mailinator.com', 5, 1);</text--->"
") )";


/*
1.6 Operation foreverQueries

*/

Operator foreverQueries_Op(
    "foreverQueries",
    foreverQueriesOpSpec,
    1,
    foreverQueries,
    foreverQueries_Select,
    foreverQueries_TM
);

} /* end of namespace */
