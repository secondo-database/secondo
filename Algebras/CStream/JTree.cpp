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

[1] Implementation of a tree made from JNodes.

[toc]

1 JNode class implementation

*/
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/TupleIdentifier/TupleIdentifier.h"
#include "RTuple.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "Algebras/Stream/Stream.h"

#include "TupleDescr.h"
#include "VTuple.h"
#include "VTHelpers.h"
#include "JTree.h"

#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

namespace cstream {

/*
1.1 Constructor
Constructs a node based on the given parameter. If it's the root node,
the values will be overwritten.

*/
JNode::JNode(const JsonType _t, const std::string _k,
             const std::string _v, JNode* _p) {
    if (_t == jROOT) {
        _type = jROOT;
        _key = "ROOT";
        _value = _v;
        _parent = NULL;
    } else {
        _type = _t;
        _key = _k;
        _value = _v;
        _parent = _p;
    }
}

/*
1.2 addChild
Adds a child JNode to the parrents vector \_children.

*/
void JNode::addChild(JNode* p) {
    _children.push_back(p);
}

/*
1.3 print
Prints the tree of JNodes in an recursive manner. 
Used for debugging.

*/
void JNode::print(int level) {
    if (_type != jROOT) for (int x=0; x<=level; x++) LOG << "   ";
    
    switch (_type) {
        case (jROOT): {
            LOG << "■──┬──[ROOT]" << ENDL;

            for (std::vector<JNode*>::iterator it = _children.begin();
                 it != _children.end(); ++it) {
                (*it)->print(level);
            }

            LOG << "└─────[END-OF-TREE]" << ENDL;            
            break;
        }

        case (jOBJECT): {
            LOG << "└──┬──[OBJT] " << _key << ENDL;

            for (std::vector<JNode*>::iterator it = _children.begin();
                 it != _children.end(); ++it) {
                (*it)->print(level+1);
            }
            break;            
        }

        case (jARRAY): {
            LOG << "└──┬──[ARRY] " << _key << ENDL;
            
            for (std::vector<JNode*>::iterator it = _children.begin();
                 it != _children.end(); ++it) {

                (*it)->print(level+1);
            }
            break;            
        }

        case (jSTRING): {
            std::string safestring;
            if (_value.length() > 136) {
                safestring = _value.substr(0, 130) + " (...)";
            } else {
                safestring = _value;                
            }

            LOG << "├─────[TEXT] " << _key << ": " 
                << safestring << ENDL;
            break;            
        }

        case (jNUMBER): {
            LOG << "├─────[NUMR] " << _key << ": " 
                << _value << ENDL;            
            break;            
        }

        case (jBOOL): {
            LOG << "├─────[BOOL] " << _key << ": " 
                << _value << ENDL;
            break;            
        }

        case (jNULL): {
            LOG << "├─────[NULL] " << _key << ": " 
                << _value << ENDL;
            break;
        }

        case (jERROR): {
            LOG << "├─────[ !!! ERROR !!! ] " << _key << ": " 
                << _value << ENDL;
            break;
        }
    }   
}

/*
1.4 deleteRec
Called by receiveStream on the root node. It deletes all
nodes of the tree in an recursive manner.

*/
void JNode::deleteRec() {
    for (std::vector<JNode*>::iterator it = _children.begin();
        it != _children.end(); ++it) {
        
        (*it)->deleteRec();
        delete (*it);
    }
}

/*
1.5 getTupleType
Returns the Secondo tuple type used in createTupleDescrString and
getAttribute.

*/
std::string JNode::getTupleType(bool forceReal) {
    std::string res = "";
    
    switch (_type) {
        case (jROOT): {
            return "root";
            break;
        }
        case (jOBJECT): {
            return "record";
            break;
        }
        case (jARRAY): {
            // res = "(" + _key + " (vector(int)))";
            if (_children.size() == 0) return "vector";

            bool allNumbers = true;
            bool allTheSame = true;
            
            std::vector<JNode*>::iterator it = _children.begin();

            std::string lastTDS = (*it)->createTupleDescrString();

            if (lastTDS.find(" ") != std::string::npos) 
                lastTDS = lastTDS.substr(lastTDS.find(" ")+1, lastTDS.length());

            for (; it != _children.end(); ++it) {
                
                std::string newTDS = (*it)->createTupleDescrString();
                std::string newTDS4Obj = newTDS;

                if (newTDS.find(" ") != std::string::npos) 
                    newTDS = newTDS.substr(newTDS.find(" ")+1, newTDS.length());
   
                if (lastTDS != newTDS) allTheSame = false;

                if ((*it)->_type != jNUMBER) allNumbers = false;

                lastTDS = newTDS;
            }

            if (allTheSame || allNumbers) return "vector";

            return "record";
            break;
        }
        case (jSTRING): {
            return "text";
            break;
        }
        case (jNUMBER): {
            if ((forceReal) || (_value.find(".") != std::string::npos))
               return "real";

            return  "int";
            break;
        }
        case (jBOOL): {
            return "bool";
            break;
        }
        case (jNULL): {
            return "string";
            break;
        }
        case (jERROR): {
            return "";
            break;
        }
    }

    return res;
}

/*
1.6 createTupleDescrString
Returns the TupleDescr String including the key. Therefor the key has
to be removed to use this return value to create a TupleDescr for the
creation of array/collections.

*/
std::string JNode::createTupleDescrString(bool forceReal) {
    std::string res = "";
    
    switch (_type) {
        case (jROOT): {
            res = "(";
            
            for (std::vector<JNode*>::iterator it = _children.begin();
                 it != _children.end(); ++it) {
                res += (*it)->createTupleDescrString();
            }
            
            res += ")";
            break;
        }
        case (jOBJECT): {
            res = "(" + _key + " (record ";
            
            for (std::vector<JNode*>::iterator it = _children.begin();
                 it != _children.end(); ++it) {

                res += (*it)->createTupleDescrString();
            }

            res += "))";
            break;
        }
        case (jARRAY): {
            // empty array (should be 'undefined', but that doesn't work...)
            if (_children.size() == 0) return "(" + _key + " (vector bool))";
            
            // all other arrays

            bool allNumbers = true;
            bool allTheSame = true;
            
            std::vector<JNode*>::iterator it = _children.begin();

            std::string resObj = "(" + _key + " (record ";
            std::string lastTDS = (*it)->createTupleDescrString();

            if (lastTDS.find(" ") != std::string::npos) {
                lastTDS = lastTDS.substr(lastTDS.find(" ")+1, lastTDS.length());
            }

            for (; it != _children.end(); ++it) {
                
                std::string newTDS = (*it)->createTupleDescrString();
                std::string newTDS4Obj = newTDS;

                if (newTDS.find(" ") != std::string::npos) {
                    newTDS = newTDS.substr(newTDS.find(" ")+1, newTDS.length());
                }
   
                if (lastTDS != newTDS) allTheSame = false;
                if ((*it)->_type != jNUMBER) allNumbers = false;

                resObj += newTDS4Obj;
                lastTDS = newTDS;
            }

            // lastTDS is e.g.: "text)"
            if (allTheSame) return "(" + _key + " (vector " + lastTDS + ")";
            if (allNumbers) return "(" + _key + " (vector real))";
            
            resObj += "))";
            return resObj;
            break;
        }
        case (jSTRING):
        case (jNUMBER):
        case (jBOOL):
        case (jNULL): {
            return "(" + _key + " " + getTupleType(forceReal) + ")";
            break;
        }
        case (jERROR): {
            return "";
            break;
        }
    }

    return res;
}


/*
1.7 buildTupleTree
Called by receiveStream on the root node. Builds the Tuple in a 
recursive manner. The sub trees for objects/records and arrays/
collections is handled by getAttribute.

*/
Tuple* JNode::buildTupleTree() {
    if (_type != jROOT) return NULL;
    
    TupleDescr* td = NULL;
    try {
        td = new TupleDescr(createTupleDescrString(false));
    } catch (SecondoException e) {
        LOG << "JNode: buildTupleTree() - Error: " << e.msg() << ENDL;
        return NULL;
    }

    TupleType* tt = td->CreateTupleType();
    Tuple* t = new Tuple(tt);
    int counter = 0;

    for (std::vector<JNode*>::iterator it = _children.begin();
            it != _children.end(); ++it) {
        
        t->PutAttribute(counter, (*it)->getAttribute());
        counter++;
    }

    tt->DeleteIfAllowed();
    
    return t;
}

/*
1.8 getAttribute
Returns the appropriate Attribute to be inserted into the Tuple,
a Record or a Collection.

----------|----------------------------------------------------
 JSON     | SECONDO
----------|----------------------------------------------------
 Object   | Record
 Array    | Collection, if all elements are the same
          | Record, otherwise.
 String   | Text, since JSON-String can have any length and
          | having differen TupleDescr just because one name
          | is longer than the other us noch reasonable.
 Number   | Int or Real, depending on the existence of a point.
 Bool     | Bool
 Null     | String with the value "null"

*/
Attribute* JNode::getAttribute(bool forceReal) {

    JsonType localType = _type;
    std::string tupledescr = "";

    // Decide if the Array will be handled as array or as object.
    if (localType == jARRAY) {
        tupledescr = createTupleDescrString(forceReal);

        if (tupledescr.substr(0, _key.length() + 9) != 
            "(" + _key + " (vector") localType = jOBJECT;
            
        if (tupledescr == "(" + _key + " (vector real))") forceReal = true;
    }

    switch (localType) {
        case (jOBJECT): {
            int counter = 0;
            Record* rec = new Record(_children.size()+1);

            for (std::vector<JNode*>::iterator it = _children.begin();
                 it != _children.end(); ++it) {

                rec->SetElement(
                    counter, 
                    (*it)->getAttribute(forceReal), 
                    (*it)->getTupleType(), 
                    (*it)->_key
                );

                counter++;
            }

            return rec;
            break;
        }
        case (jARRAY): {
            ListExpr typeinfo;
            
            // remove the key
            tupledescr = tupledescr.substr(_key.length()+1, 
                tupledescr.length()-_key.length()-2);

            // remove unneccessary parentheses in case of records
            // if (tupledescr.substr(0, 15) == "(vector((record") {
            //     tupledescr = "(vector(record" 
            //         + tupledescr.substr(15, tupledescr.length()-16);
            // }

            nl->ReadFromString( tupledescr, typeinfo);
            
            SecondoCatalog* sc = SecondoSystem::GetCatalog();
            ListExpr numtypeinfo = sc->NumericType(typeinfo);
            
            collection::Collection* coll = new collection::Collection(
                collection::vector, numtypeinfo);

            for (std::vector<JNode*>::iterator it = _children.begin();
                 it != _children.end(); ++it) {

                coll->Insert((*it)->getAttribute(forceReal), 1);
            }

            coll->SetDefined(true);

            return coll;
            break;
        }
        case (jSTRING): {
            return new FText(true, _value);
            break;
        }
        case (jNUMBER): {
            bool asReal = createTupleDescrString(forceReal).find("real") 
                       != std::string::npos;

            if (_value[0] == 'e') {
                if (asReal) return new CcReal(false, 0.0);
                return new CcInt(false, 0);
            }
            
            if (asReal) return new CcReal(true, std::atof(_value.c_str())); 

            return new CcInt(true, std::stoi(_value));

            break;
        }
        case (jBOOL): {
            if (_value[0] == 't') return new CcBool(true, true);
            if (_value[0] == 'f') return new CcBool(true, false);
            return new CcBool(false, false);
            break;
        }
        case (jNULL): {
            if (_value[0] == 'n') new CcString(true, "null");
            return new CcString(true, _value);
            break;
        }
        
        case (jROOT):
        case (jERROR): {
            return NULL;
            break;
        }
    }

    return NULL;
}

/*
1.9 buildTree
Iterates over the string in \_value and adds the JNodes to the tree.
It finds and extracts the key for the next node (or creates them for arrays), 
decides which JsonType it will be by using getValueType() and then extracts
the value by using getValue().
For arrays and objects, the \_value will be a string which can be analyzed by
buildTree, so after creating the new node and adding it to the parents 
_children vector, it calls buildTree on it.
This way it creates the tree in a recursive manner.

*/
void JNode::buildTree() {
    if (_type != jROOT && _type != jOBJECT && _type != jARRAY) return;

    try {
        int indexVal = 0;
        int maxVal = _value.length();

        int* index = &indexVal;
        int* max = &maxVal;
        
        std::string cKey;

        int arrayCounter = 0;

        while (*index < *max) {
            if (_type == jARRAY) {
                // if it's an array, an (array-wide) unique key will be
                // provided, since Attributes in Secondo Records need a key
                // it's also neccessary if the array is handled as an object
                cKey = "ArrayElem" + std::to_string(arrayCounter);
                if (*index == 0) (*index)++;
            } else {
                // this for loop looks for the next key in an object/root
                for (; *index < *max; (*index)++) {
                    if (_value[*index] == '"') break;
                }
                cKey = getString(true, _value, index, max);
            }

            if (_type == jARRAY) {
                // this for loop looks for the next value in an array
                for (; *index < *max; (*index)++) {
                    if (*index == 1) break;
                    if (_value[*index] == ',') break;
                }
            } else {
                // this for loop looks for the next value in an object/root
                for (; *index < *max; (*index)++) {
                    if (_value[*index] == ':') break;
                }
            }

            JsonType    cType  = getValueType(_value, index, max);
            std::string cValue = getValue(cType, _value, index, max);
            
            if (cType != jERROR) {
                JNode* child = new JNode(cType, cKey, cValue, this);
                addChild(child);
                child->buildTree();

                if (_type == jARRAY) arrayCounter++;
            }
            
            if (_type != jARRAY) (*index)++;
        }
    } catch (int e) {
        LOG << "CStream::JTree - an exception # " 
            << e << " # occurred." << ENDL;
    }
}

/*
1.10 getValueType
Returns the type of the JSON value for the buildTree() function.
Result will then be stored in the \_type var of the new node.

*/
JsonType JNode::getValueType(std::string jsonString, int* index, int* max) {
    
    for (; *index < *max; (*index)++) {
        if (jsonString[*index] == '"') return jSTRING;
        if (jsonString[*index] == '{') return jOBJECT;
        if (jsonString[*index] == '[') return jARRAY;
        if (toupper(jsonString[*index]) == 'T') return jBOOL;
        if (toupper(jsonString[*index]) == 'F') return jBOOL;
        if (toupper(jsonString[*index]) == 'N') return jNULL;
        if ((jsonString[*index] == '-') || 
            (std::isdigit(jsonString[*index]))) return jNUMBER;
    }

    return jERROR;
}

/*
1.11 getValue
Returns the JSON value for the buildTree() function as a string.
Uses the corresponding get<>() functions.
Result will then be stored in the \_value var of the new node.

*/
std::string JNode::getValue(JsonType valueType, std::string jsonString,
                            int* index, int* max) {
    if (valueType == jSTRING) return getString(false, jsonString, index, max);
    if (valueType == jNUMBER) return getNumber(jsonString, index, max);
    if (valueType == jBOOL)   return getBool(jsonString, index, max);
    if (valueType == jNULL)   return getNull(jsonString, index, max);
    if (valueType == jOBJECT) return getObject(jsonString, index, max);
    if (valueType == jARRAY)  return getArray(jsonString, index, max);  

    return "jERROR";
}

/*
1.12 getString
The get<>() function for getting value of JsonType String.
Called by the getValue() function.

*/
std::string JNode::getString(bool keySafe, std::string jsonString,
                             int* index, int* max) {
    
    bool doEscape = false;
    std::string res = "";

    // Current jsonString[index] equals ".
    (*index)++;

    for (; *index < *max; (*index)++) {
        
        if (jsonString[*index] == '"' && !doEscape) break;

        if (jsonString[*index] == '\\') {
            doEscape = !doEscape;
        } else {
            doEscape = false;
        }

        res += jsonString[*index];
    }

    if (keySafe) {
        boost::regex expr{R"([^A-Za-z0-9])"};
        res = boost::regex_replace(res, expr, "");

        if (res.empty()) {
            res = "X";
        } else {
            res[0] = toupper(res[0]);
            if (std::isdigit(res[0])) {
                res = "X" + res;
            }
        }

        res = res.substr(0, 39);
    }

    return res;
}

/*
1.13 getNumber
The get<>() function for getting value of JsonType Number.
Called by the getValue() function.

*/
std::string JNode::getNumber(std::string jsonString, int* index, int* max) {
    std::string res = "";

    for (; *index < *max; (*index)++) {
        if ((jsonString[*index] == '}') ||
            (jsonString[*index] == ']') ||
            (jsonString[*index] == ',')) break;
        res += jsonString[*index];
    }

    boost::trim(res);
    boost::regex expr{"-?(?:0|[1-9]\\d*)(?:\\.\\d+)?(?:[eE][+-]?\\d+)?"};
    if (!boost::regex_match(res, expr)) {
        res = "error_number (format): " + res;
    } else {
        try {
            if (res.find(".") != std::string::npos) {
                //double test = boost::lexical_cast<double>(res);
                //test=test;
            } else {
                //int test = boost::lexical_cast<int>(res);
                //test=test;
            }
        } catch(const boost::bad_lexical_cast &e) {
            res = "error_number (conversion): " + res;
        }
    }

    return res;
}

/*
1.14 getBool
The get<>() function for getting value of JsonType Bool.
Called by the getValue() function.

*/
std::string JNode::getBool(std::string jsonString, int* index, int* max) {
    std::string res = "error_bool: " + jsonString.substr(*index, 5);

    if (toupper(jsonString[*index]) == 'T') {
        std::string sub = jsonString.substr(*index, 4);
        boost::to_upper(sub);
        if (sub == "TRUE") res="true";
    } else {
        std::string sub = jsonString.substr(*index, 5);
        boost::to_upper(sub);
        if (sub == "FALSE") res="false";
    }
    
    toSeperator(jsonString, index, max);

    return res;
}

/*
1.15 getNull
The get<>() function for getting value of JsonType Null.
Called by the getValue() function.

*/
std::string JNode::getNull(std::string jsonString, int* index, int* max) {
    std::string res="";
    
    std::string sub = jsonString.substr(*index, 4);
    boost::to_upper(sub);
    
    toSeperator(jsonString, index, max);

    if (sub == "NULL") {
        res = "null";
    } else {
        res = "error_null: " + sub;
        res = res.substr(0, 47);
    }

    return res;
}

/*
1.16 getObject
The get<>() function for getting value (the full string of the object) of 
JsonType Object. Called by the getValue() function.

*/
std::string JNode::getObject(std::string jsonString, int* index, int* max) {
    std::string res = "";
    int bracketsCount = 0;
    bool inText = false;
    bool doEscape = false;

    for (; *index < *max; (*index)++) {
        switch (jsonString[*index]) {
            case '\\': {
                doEscape = !doEscape;
                break;
            }
            case '"': {
                if (!(doEscape)) inText = !inText;
                doEscape = false;
                break;
            }
            case '{': {
                if (!doEscape && !inText) bracketsCount++;
                doEscape = false;
                break;
            }
            case '}': {
                if (!doEscape && !inText) bracketsCount--;
                doEscape = false;
                break;
            }
            default: {
                doEscape = false;
            }
        }

        res += jsonString[*index];
        
        if ((res.length() > 0) && (bracketsCount == 0)) break;
    }

    return res;
}

/*
1.17 getArray
The get<>() function for getting value (the full string of the array) of 
JsonType Array. Called by the getValue() function.

*/
std::string JNode::getArray(std::string jsonString, int* index, int* max) {
    std::string res = "";
    int bracketsCount = 0;
    bool inText = false;
    bool doEscape = false;

    for (; *index < *max; (*index)++) {
        switch (jsonString[*index]) {
            case '\\': {
                doEscape = !doEscape;
                break;
            }
            case '"': {
                if (!(doEscape)) inText = !inText;
                break;
            }
            case '[': {
                if (!doEscape && !inText) bracketsCount++;
                break;
            }
            case ']': {
                if (!doEscape && !inText) bracketsCount--;
                break;
            }
            default: {
                doEscape = false;
            }
        }

        res += jsonString[*index];
        
        if ((res.length() > 0) && (bracketsCount == 0)) break;
    }

    return res;
}

/*
1.17 getArray
Moves the index to the next correct seperator - ',' or '}' or ']'.

*/
void JNode::toSeperator(std::string jsonString, int* index, int* max) {
    for (; *index < *max; (*index)++) {
        if (jsonString[*index] == ',') break;
        if (jsonString[*index] == '}') break;
        if (jsonString[*index] == ']') break;
    }
}

} /* namespace cstream */
