/*
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
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

*/

#ifndef TILEALGEBRA_TYPES_H
#define TILEALGEBRA_TYPES_H

#include <vector>
#include <string>

namespace TileAlgebra
{

/*
declaration of GetValueWrapperTypes function

*/

void GetValueWrapperTypes(std::vector<std::string>& rValueWrapperTypes);

/*
declaration of GetMTypes function

*/

void GetMTypes(std::vector<std::string>& rMTypes);

/*
declaration of GettTypes function

*/

void GettTypes(std::vector<std::string>& rtTypes);

/*
declaration of GetmtTypes function

*/

void GetmtTypes(std::vector<std::string>& rmtTypes);

/*
declaration of GetitTypes function

*/

void GetitTypes(std::vector<std::string>& ritTypes);

/*
declaration of GetValueWrapperType function

*/

std::string GetValueWrapperType(const std::string& rType);

/*
declaration of GetMType function

*/

std::string GetMType(const std::string& rType);

/*
declaration of GettType function

*/

std::string GettType(const std::string& rType);

/*
declaration of GetmtType function

*/

std::string GetmtType(const std::string& rType);

/*
declaration of GetitType function

*/

std::string GetitType(const std::string& rType);

/*
declaration of IsValueWrapperType function

*/

bool IsValueWrapperType(const std::string& rType);

/*
declaration of IsMType function

*/

bool IsMType(const std::string& rType);

/*
declaration of IstType function

*/

bool IstType(const std::string& rType);

/*
declaration of IsmtType function

*/

bool IsmtType(const std::string& rType);

/*
declaration of IsitType function

*/

bool IsitType(const std::string& rType);

}

#endif // TILEALGEBRA_TYPES_H
