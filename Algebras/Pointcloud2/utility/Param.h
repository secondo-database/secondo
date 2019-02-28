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

Represents a single parameter, encapsulating its name, type,
context, value, default value, min value, max value and meaning.

*/

#pragma once
#include <string>
#include <memory>
#include <vector>
#include <map>

namespace pointcloud2 {

typedef size_t intParam_t;

class Param {
public:
    enum TYPE { BOOL, INT, REAL };
    enum CONTEXT { analyzeRaster, analyzeGeom };

private:
    static bool NO_BOOL;
    static intParam_t NO_INT;
    static double NO_REAL;

    const std::string _name;
    const TYPE _type;
    const CONTEXT _context;

    bool* const _valueBool;
    const bool _defaultValueBool;

    intParam_t* const _valueInt;
    const intParam_t _defaultValueInt;
    const intParam_t _minValueInt;
    const intParam_t _maxValueInt;

    double* const _valueReal;
    const double _defaultValueReal;
    const double _minValueReal;
    const double _maxValueReal;

    const std::string _meaning;

public:
    Param(const std::string name, double* value, const CONTEXT context,
            const double minValue, const double maxValue,
            const std::string meaning);

    Param(const std::string name, intParam_t* value, const CONTEXT context,
            const intParam_t minValue, const intParam_t maxValue,
            const std::string meaning);

    Param(const std::string name, bool* value, const CONTEXT context,
            const std::string meaning);

    ~Param() = default;

    std::string getName() const { return _name; }
    TYPE getType() const { return _type; }
    CONTEXT getContext() const { return _context; }
    std::string getMeaning() const { return _meaning; }

    bool getValueBool() const;
    intParam_t getValueInt() const;
    double getValueReal() const;

    void setValueBool(const bool value) const;
    bool trySetValueInt(const intParam_t value) const;
    bool trySetValueReal(const double value) const;

    std::string getTypeAsString() const;

    std::string getValueAsString() const;
    std::string getDefaultValueAsString() const;
    std::string getMinValueAsString() const;
    std::string getMaxValueAsString() const;

private:
    static std::string getString(bool value);
    static std::string getString(intParam_t value);
    static std::string getString(double value);
};


/*
1.1 Params

This class provides access to the parameters for the pc2SetParam
and pc2GetParams operators.

*/
class Params {
    std::vector<std::string> _names;
    std::map<std::string, Param> _params;

public:
    Params();

    ~Params() = default;

    void add(Param param);

    bool contains(std::string paramName) const;

    size_t size() const;

    const Param& get(size_t i) const;

    const Param& get(std::string paramName) const;
};


} /* namespace pointcloud2 */
