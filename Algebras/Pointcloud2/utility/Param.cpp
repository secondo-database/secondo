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



1 The Param class

*/

#include "Param.h"
#include <assert.h>

using namespace std;

namespace pointcloud2 {


Param::Param(const string name, double* value, const CONTEXT context,
        const double minValue, const double maxValue,
        const string meaning) :
                _name(name), _type(TYPE::REAL), _context(context),
                _valueBool(nullptr), _defaultValueBool(false),
                _valueInt(nullptr), _defaultValueInt(0),
                _minValueInt(0), _maxValueInt(0),
                _valueReal(value), _defaultValueReal(*value),
                _minValueReal(minValue), _maxValueReal(maxValue),
                _meaning(meaning) { }

Param::Param(const string name, intParam_t* value, const CONTEXT context,
        const intParam_t minValue, const intParam_t maxValue,
        const string meaning) :
                _name(name), _type(TYPE::INT), _context(context),
                _valueBool(nullptr), _defaultValueBool(false),
                _valueInt(value), _defaultValueInt(*value),
                _minValueInt(minValue), _maxValueInt(maxValue),
                _valueReal(nullptr), _defaultValueReal(0.0),
                _minValueReal(0.0), _maxValueReal(0.0),
                _meaning(meaning) { }

Param::Param(const string name, bool* value, const CONTEXT context,
        const string meaning) :
                _name(name), _type(TYPE::BOOL), _context(context),
                _valueBool(value), _defaultValueBool(*value),
                _valueInt(nullptr), _defaultValueInt(0),
                _minValueInt(0), _maxValueInt(0),
                _valueReal(nullptr), _defaultValueReal(0.0),
                _minValueReal(0.0), _maxValueReal(0.0),
                _meaning(meaning) { }

bool Param::getValueBool() const {
    assert (_type == TYPE::BOOL);
    return *_valueBool;
}

intParam_t Param::getValueInt() const {
    assert (_type == TYPE::INT);
    return *_valueInt;
}

double Param::getValueReal() const {
    assert (_type == TYPE::REAL);
    return *_valueReal;
}

void Param::setValueBool(const bool value) const {
    assert (_type == TYPE::BOOL);
    *_valueBool = value;
}

bool Param::trySetValueInt(const intParam_t value) const {
    assert (_type == TYPE::INT);
    if (value < _minValueInt || value > _maxValueInt)
        return false;
    *_valueInt = value;
    return true;
}

bool Param::trySetValueReal(const double value) const {
    assert (_type == TYPE::REAL);
    if (value < _minValueReal || value > _maxValueReal)
        return false;
    *_valueReal = value;
    return true;
}

string Param::getTypeAsString() const {
    switch(_type) {
    case BOOL: return "bool";
    case INT:  return "int"; // or size_t
    case REAL: return "double";
    default:
        assert (false);
        return "";
    }
}

string Param::getValueAsString() const {
    switch(_type) {
    case BOOL: return getString(*_valueBool);
    case INT:  return getString(*_valueInt);
    case REAL: return getString(*_valueReal);
    default:
        assert (false);
        return "";
    }
}

string Param::getDefaultValueAsString() const {
    switch(_type) {
    case BOOL: return getString(_defaultValueBool);
    case INT:  return getString(_defaultValueInt);
    case REAL: return getString(_defaultValueReal);
    default:
        assert (false);
        return "";
    }
}

string Param::getMinValueAsString() const {
    switch(_type) {
    case BOOL: return getString(false);
    case INT:  return getString(_minValueInt);
    case REAL: return getString(_minValueReal);
    default:
        assert (false);
        return "";
    }
}

string Param::getMaxValueAsString() const {
    switch(_type) {
    case BOOL: return getString(true);
    case INT:  return getString(_maxValueInt);
    case REAL: return getString(_maxValueReal);
    default:
        assert (false);
        return "";
    }
}

string Param::getString(bool value) {
    return value ? "TRUE" : "FALSE";
}

string Param::getString(intParam_t value) {
    return std::to_string(value);
}

string Param::getString(double value){
    return std::to_string(value);
}


Params::Params() {
}

void Params::add(Param param) {
    this->_names.push_back(param.getName());
    _params.insert(std::pair<string, Param>(param.getName(), param));
}

bool Params::contains(string paramName) const {
    auto it = _params.find(paramName);
    return (it != _params.end());
    // TODO: ist hier "delete it;" nötig?
}

size_t Params::size() const {
    return _params.size();
}

const Param& Params::get(size_t i) const  {
    assert (i < _params.size());
    return get(_names[i]);
}


const Param& Params::get(string paramName) const {
    return _params.at(paramName);
}
} /* namespace pointcloud2 */
