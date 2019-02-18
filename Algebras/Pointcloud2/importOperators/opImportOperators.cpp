/*
----
This file is part of SECONDO.

Copyright (C) 2019,
Faculty of Mathematics and Computer Science,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



0 Import Operators

Our Algebra supports two imports:

  * from CVS-files

    This import does not support properties, so the format must be
    lines of the form "x , y , z". Whitespace is ignored.

  * from LAS files

1 importxyz Operator a.k.a. CVS-import


Signature:

importxyz: \{string, text\} x real x real x REF -[>] pointcloud2(REF)

The real numbers are x- and y-scaling-factors

*/

#include "opImportOperators.h"

#include <vector>

#include <boost/algorithm/string.hpp>

#include "Algebras/Pointcloud/lasreader/lasreader.h"
#include "Algebras/FText/FTextAlgebra.h"

#include "../utility/StlFacet.h"
#include "../utility/ShapeGenerator.h"
#include "lasUtils.h"
#include "../tcPointcloud2.h"

using namespace pointcloud2;


ListExpr op_importxyz::importxyzTM(ListExpr args){
    const std::string err = "Filename, x/y factor and REF expected";
    // (? ? ? ?)
    if(!nl->HasLength(args,4)){
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    ListExpr argFileName = nl->First(args);
    ListExpr argXFactor = nl->Second(args);
    ListExpr argYFactor = nl->Third(args);
    ListExpr argRefSys = nl->Fourth(args);

    // has ->SetUsesArgsInTypeMapping() in Algebra, therefore:
    // ( (? ?) (? ?) (? ?) (? ?) )
    if(!nl->HasLength(argFileName,2)
            ||!nl->HasLength(argXFactor,2)
            ||!nl->HasLength(argYFactor,2)
            ||!nl->HasLength(argRefSys,2))
    {
        return listutils::typeError("internal error");
    }

    // ( (String/FText atom) (? ?) (? ?) (? ?) )
    std::string filename;
    if( CcString::checkType(nl->First(argFileName)) ) {
        filename = nl->StringValue(nl->Second(argFileName));
    } else if( FText::checkType(nl->First(argFileName)) ) {
        filename = nl->Text2String(nl->Second(argFileName));
    } else {
        return listutils::typeError(
                "invalid file name - needs to be constant string or text");
    }

    std::ifstream in(filename);
    if (!in.is_open()) {
        return listutils::typeError("Error opening file");
    }
    in.close();

    // ( (String/FText filename) (Real ?) (Real ?) (? ?) )
    if(!CcReal::checkType(nl->First(argXFactor)) ||
            !CcReal::checkType(nl->First(argYFactor))){
        return listutils::typeError(err+" factor");
    }

    // ( (String/FText filename) (Real ?) (Real ?) (RefSys ?) )
    if(!listutils::isSymbol(nl->First(argRefSys))){
        return listutils::typeError(err+" ref1");
    } else {
        try
        {
            Referencesystem::toEnum(nl->SymbolValue(nl->First(argRefSys)));
        } catch(std::invalid_argument&)
        {
            return listutils::typeError(err+" ref2");
        }
    }
    // return (APPEND filename (pointcloud2 Symbol))
    return nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()),
            nl->OneElemList(nl->TextAtom(filename)),
            Pointcloud2::cloudTypeWithParams(nl->First(argRefSys)));
}

int op_importxyz::importxyzVMT( Word* args, Word& result, int message,
        Word& local, Supplier s ){

    FText* fileNameDatabaseString = static_cast<FText*>(args[4].addr); //T
    if (!fileNameDatabaseString->IsDefined()){
        return 0;
    }
    const std::string filename = fileNameDatabaseString->GetValue();

    std::ifstream fileStream(filename);
    if (!fileStream.is_open()) {
        std::cout << "Error opening file." << endl;
        return -1;
    }

    const double xFactor = (static_cast<CcReal*>(args[1].addr))->GetRealval();
    const double yFactor = (static_cast<CcReal*>(args[2].addr))->GetRealval();

    result = qp->ResultStorage(s);
    Pointcloud2* res = static_cast<pointcloud2::Pointcloud2*>(result.addr);

    //iterate on lines and construct pointcloud
    unsigned int lines = 0; 
    unsigned int errLines = 0;
    unsigned int currentLine = 0;
    constexpr unsigned int linesInTransaction = 100000;
    unsigned int currentLinesInTransaction = linesInTransaction;

    //Attributes
    res->startInsert();
    std::string line;
    while (std::getline(fileStream, line)){
        ++lines;
        std::vector<std::string> tokens(3);
        boost::algorithm::split(tokens, line, boost::is_any_of(","));
        if(tokens.size() != 3){
            ++errLines; 
            continue;
        }
        try {
            if(currentLinesInTransaction == linesInTransaction)
            {
                currentLinesInTransaction = 0;
                std::cout << "Current Line : " << currentLine << std::endl;
                // if(currentLine != 0) SmiEnvironment::CommitTransaction();
                // bool beginSuccess = SmiEnvironment::BeginTransaction();
                // if(!beginSuccess)
                // {
                //     SmiEnvironment::CommitTransaction();
                //     SmiEnvironment::BeginTransaction();
                // }
            } 

            res->insert({
                xFactor * std::stod(tokens[0]),
                yFactor * std::stod(tokens[1]),
                std::stod(tokens[2])
            });
        }
        catch (std::invalid_argument &){
            ++errLines;
        }
        ++currentLine;
        ++currentLinesInTransaction;
    }
    fileStream.close();

    res->finalizeInsert();
    // SmiEnvironment::CommitTransaction(); //Commit the last transaction
    const unsigned int  imported = lines - errLines;

    //DEBUG lines
    std::cout << endl;
    std::cout << "Lines imported: " << imported << endl;
    std::cout << "Lines with errors: " << errLines << endl;

    if(local.addr){
        local.addr = nullptr;
    }
    return 0;
}

std::string op_importxyz::getOperatorSpec(){
    return OperatorSpec(
            "{string,text} x real x real x REF -> real",
            "importxyz(_,_,_,_)",
            "Import Pointclouds from text-file",
            "query importxyz(\"test.csv\",1.0,1.0,EUCLID)"
    ).getStr();
}

std::shared_ptr<Operator> op_importxyz::getOperator(){
    return std::make_shared<Operator>("importxyz",
                                       getOperatorSpec(),
                                       op_importxyz::importxyzVMT,
                                       //2,
                                       Operator::SimpleSelect,
                                       //OPImportxyz::importxyzSelect,
                                       &op_importxyz::importxyzTM);
}

/*
2 The Pointcloud2 importPc2FromLas Operator

importPc2FromLas: {string, text} -[>] pointcloud2(X)

2.1 tupleTypeOfLasPointFormat

Returns the tuple type depending on the point format of the las file.
(which is returned by lasreader::getPointFormat()).
Currently, can handle format 1-5.

2.2 Type Mapping

*/
ListExpr op_importPc2FromLas::importPc2FromLasTM(ListExpr args){
    if(!nl->HasLength(args, 1)){
        return listutils::typeError("expected one argument");
    }

    ListExpr arg = nl->First(args);
    // the list is coded as (<type> <query part>)
    if(!nl->HasLength(arg, 2)){
        return listutils::typeError("internal error");
    }

    std::string filename;
    if( CcString::checkType(nl->First(arg)) ) {
        filename = nl->StringValue(nl->Second(arg));
    } else if( FText::checkType(nl->First(arg)) ) {
        filename = nl->Text2String(nl->Second(arg));
    } else {
        return listutils::typeError(
                "invalid file name - needs to be constant string or text");
    }

    std::ifstream fileStream(filename);
    if (!fileStream.is_open()) {
        return listutils::typeError("Error opening file");
    }
    fileStream.close();
    
    lasreader reader(filename);
    if(!reader.isOk()){
        return listutils::typeError ("No correct LAS File");
    }

    // get the Pointcloud2 format
    int formatNumber = reader.getPointFormat();

    //List of Attributes in LAS
    // TODO: ich verstehe nicht, weshalb diese Funktion Speicherfehler
    // produziert, die andere aber nicht
    //ListExpr pc2Format = lasFormatAttrList(formatNumber);
    ListExpr pc2Format = nl->OneElemList(getAttr<CcInt>("intensity"));
    extendAttrList(pc2Format, formatNumber);

    return nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()),
            nl->OneElemList(nl->TextAtom(filename)),
            Pointcloud2::cloudTypeWithParams(
                    nl->SymbolAtom("WGS84"),
                    nl->TwoElemList(
                            listutils::basicSymbol<Tuple>(),
                            pc2Format)));

}

/*
2.3 Value Mapping

*/
int op_importPc2FromLas::importPc2FromLasVMT( Word* args, Word& result,
        int message, Word& local, Supplier s ){

    FText* fileNameDatabaseString = static_cast<FText*>(args[1].addr);
    if (!fileNameDatabaseString->IsDefined()){
        return 0;
    }
    const std::string filename = fileNameDatabaseString->GetValue();

    result = qp->ResultStorage(s);
    Pointcloud2* pc2 = static_cast<pointcloud2::Pointcloud2*>(result.addr);

    lasreader reader (filename);
    // get the point format for this file
    const int pointFormat = reader.getPointFormat();

    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    bool valid = false;
    int pointCount = 0;

    pc2->startInsert();
    while(true) {
        std::unique_ptr<lasPoint> point(reader.next());
        if(!point) {
            break;
        }

        if (!reader.toLatLon(point.get(), x, y, z, valid)) {
           return 0;
        }
        if(!valid) {
            continue;
        }

        PcPoint pcPoint = {x, y, z};

        // create a tuple to store properties
        Tuple* tuple = pc2->createEmptyTuple();
        // standard attribute
        tuple->PutAttribute(0, new CcInt(point->Intensity));

        // put further properties depending on the pointFormat
        size_t offset = 1;
        switch(pointFormat){
        case 0 : 
            fillTuple0(tuple, dynamic_cast<lasPoint0*>(point.get()), offset); 
            break;
        case 1 : 
            fillTuple1(tuple, dynamic_cast<lasPoint1*>(point.get()), offset); 
            break;
        case 2 : 
            fillTuple2(tuple, dynamic_cast<lasPoint2*>(point.get()), offset); 
            break;
        case 3 : 
            fillTuple3(tuple, dynamic_cast<lasPoint3*>(point.get()), offset); 
            break;
        case 4 : 
            fillTuple4(tuple, dynamic_cast<lasPoint4*>(point.get()), offset); 
            break;
        case 5 : 
            fillTuple5(tuple, dynamic_cast<lasPoint5*>(point.get()), offset); 
            break;
        }

        //TODO: Kann hier auch was anderes ankommen? 
        //TODO: default fall??

        pc2->insert(pcPoint, tuple);
        // tuple->DeleteIfAllowed() must NOT be called here since it
        // is already being called in Pointcloud2::insert!

        //DEBUG
        ++pointCount;
        if (pointCount % 10000 == 0)
            std::cout << pointCount << " points ..." << endl;
    }
    pc2->finalizeInsert();

    //DEBUG
    std::cout << pointCount << " points imported." << endl;

    if (local.addr) {
        local.addr = nullptr;
    }
    return 0;
}

std::string op_importPc2FromLas::getOperatorSpec(){
    return OperatorSpec(
            "{string,text} -> pointcloud2(WGS84 (tuple((Intensity int))))",
            "importPc2FromLas(_)",
            "Import Pointcloud2 from a las file",
            "query importPc2FromLas(\"test.las\")"
    ).getStr();
}

std::shared_ptr<Operator> op_importPc2FromLas::getOperator(){
    return std::make_shared<Operator>("importPc2FromLas",
                                   getOperatorSpec(),
                                   op_importPc2FromLas::importPc2FromLasVMT,
                                   Operator::SimpleSelect,
                                   &op_importPc2FromLas::importPc2FromLasTM);
}



/*
1.3 importPc2FromStl Operator

*/
ListExpr op_importPc2FromStl::importPc2FromStlTM(ListExpr args){
    const std::string err = "arguments 'string/text filename, int pointCount, "
            "int rotationMode (0-2), real diffusion, int rndSeed' expected";

    // (? ? ? ?)
    if (!nl->HasLength(args, 5))
        return listutils::typeError(err + " (wrong number of arguments)");
    ListExpr argFileName = nl->First(args);
    ListExpr argPointCount = nl->Second(args);
    ListExpr argRotation = nl->Third(args);
    ListExpr argDiffusion = nl->Fourth(args);
    ListExpr argRndSeed = nl->Fifth(args);

    // has ->SetUsesArgsInTypeMapping() in Algebra, therefore:
    // ( (? ?) (? ?) (? ?) (? ?) (? ?))
    if ( !nl->HasLength(argFileName, 2) ||
         !nl->HasLength(argPointCount, 2) ||
         !nl->HasLength(argRotation, 2) ||
         !nl->HasLength(argDiffusion, 2) ||
        !nl->HasLength(argRndSeed, 2))
    {
        return listutils::typeError("internal error");
    }

    // ( (String/FText atom) (? ?) (? ?) (? ?) (? ?))
    std::string filename;
    if( CcString::checkType(nl->First(argFileName)) ) {
        filename = nl->StringValue(nl->Second(argFileName));
    } else if( FText::checkType(nl->First(argFileName)) ) {
        filename = nl->Text2String(nl->Second(argFileName));
    } else {
        return listutils::typeError(
                "invalid file name - needs to be constant string or text");
    }
    std::ifstream in(filename);
    if (!in.is_open())
        return listutils::typeError("Error opening file");
    in.close();

    // ( (String/FText filename) (int ?) (int ?) (real ?))
    if (!CcInt::checkType(nl->First(argPointCount)) ||
        !CcInt::checkType(nl->First(argRotation)) ||
        !CcReal::checkType(nl->First(argDiffusion)) ||
        !CcInt::checkType(nl->First(argRndSeed))) {
        return listutils::typeError(err + " (wrong argument type)");
    }

    int rotation = nl->IntValue(nl->Second(argRotation));
    if (rotation < 0 || rotation > 2) {
        return listutils::typeError(err + " (rotationMode must be "
                "0 (none), 1 (paraxial), or 2 (arbitrary)");
    }

    int rndSeed = nl->IntValue(nl->Second(argRndSeed));
    if (rndSeed < 0) {
        return listutils::typeError(err + " (rndSeed must be "
                "0 (current time) or positive (fixed seed)");
    }

    // return (APPEND filename (pointcloud2 Symbol))
    ListExpr refSys = nl->SymbolAtom(
            Referencesystem::toString(Referencesystem::Type::EUCLID));
    return nl->ThreeElemList(
                nl->SymbolAtom(Symbols::APPEND()),
                nl->OneElemList(nl->TextAtom(filename)),
                Pointcloud2::cloudTypeWithParams(refSys));
}

int op_importPc2FromStl::importPc2FromStlVM( Word* args, Word& result,
        int message, Word& local, Supplier s ){

    bool REPORT_TO_CONSOLE = true;

    // get the arguments
    const int pointCount = (static_cast<CcInt*>(args[1].addr))->GetIntval();
    const int rotationMode = (static_cast<CcInt*>(args[2].addr))->GetIntval();
    const double diffusion =
            (static_cast<CcReal*>(args[3].addr))->GetRealval();
    const int rndSeed = (static_cast<CcInt*>(args[4].addr))->GetIntval();

    // get the file name which was passed from Type Mapping
    FText* fileName = static_cast<FText*>(args[5].addr);
    if (!fileName->IsDefined())
        return 0;

    result = qp->ResultStorage(s);
    Pointcloud2* pc2 = static_cast<pointcloud2::Pointcloud2*>(result.addr);

    StlObject stlFile{};
    if (!stlFile.read(fileName->GetValue() ))
        return -1;

    std::shared_ptr<std::vector<StlFacet>> facets = stlFile.getFacets();

    //DEBUG lines
    if (REPORT_TO_CONSOLE)
        std::cout << endl << facets->size() << " facets imported" << endl;

    ShapeGenerator::Rotation rotation =
            static_cast<ShapeGenerator::Rotation>(rotationMode);
    ShapeGenerator shapeGen { pc2, facets, size_t(pointCount), rotation,
        diffusion, unsigned(rndSeed) };
    shapeGen.generate();

    if (local.addr) {
        local.addr = nullptr;
    }
    return 0;
}

std::string op_importPc2FromStl::getOperatorSpec(){
    return OperatorSpec(
            "{string,text} x int x int x real x int -> Pointcloud2",
            "importPc2FromStl(_,_,_,_,_)",
            "Import Pointcloud2 from a StL file",
            "query importPc2FromStl(\"test.stl\", 50000, 0.0, 1)"
    ).getStr();
}

std::shared_ptr<Operator> op_importPc2FromStl::getOperator(){
    return std::make_shared<Operator>(
            "importPc2FromStl",
            getOperatorSpec(),
            op_importPc2FromStl::importPc2FromStlVM,
            Operator::SimpleSelect,
            &op_importPc2FromStl::importPc2FromStlTM);
}

