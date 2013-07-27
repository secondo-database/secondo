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

/*
system includes

*/

#include <string>
#include <vector>

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Method GetValueWrapperTypes returns all value wrapper types.

author: Dirk Zacher
parameters: rValueWrapperTypes - reference to a vector of strings containing
                                 all value wrapper types
return value: -
exceptions: -

*/

void GetValueWrapperTypes(std::vector<std::string>& rValueWrapperTypes);

/*
Method GetMTypes returns all moving types.

author: Dirk Zacher
parameters: rMTypes - reference to a vector of strings containing
                      all moving types
return value: -
exceptions: -

*/

void GetMTypes(std::vector<std::string>& rMTypes);

/*
Method GettTypes returns all t types of Tile Algebra.

author: Dirk Zacher
parameters: rtTypes - reference to a vector of strings containing
                      all t types of Tile Algebra
return value: -
exceptions: -

*/

void GettTypes(std::vector<std::string>& rtTypes);

/*
Method GetmtTypes returns all mt types of Tile Algebra.

author: Dirk Zacher
parameters: rmtTypes - reference to a vector of strings containing
                       all mt types of Tile Algebra
return value: -
exceptions: -

*/

void GetmtTypes(std::vector<std::string>& rmtTypes);

/*
Method GetitTypes returns all it types of Tile Algebra.

author: Dirk Zacher
parameters: ritTypes - reference to a vector of strings containing
                       all it types of Tile Algebra
return value: -
exceptions: -

*/

void GetitTypes(std::vector<std::string>& ritTypes);

/*
Method GetsTypes returns all s types of Raster2 Algebra.

author: Dirk Zacher
parameters: rsTypes - reference to a vector of strings containing
                      all s types of Raster2 Algebra
return value: -
exceptions: -

*/

void GetsTypes(std::vector<std::string>& rsTypes);

/*
Method GetmsTypes returns all ms types of Raster2 Algebra.

author: Dirk Zacher
parameters: rmsTypes - reference to a vector of strings containing
                       all ms types of Raster2 Algebra
return value: -
exceptions: -

*/

void GetmsTypes(std::vector<std::string>& rmsTypes);

/*
Method GetisTypes returns all is types of Raster2 Algebra.

author: Dirk Zacher
parameters: risTypes - reference to a vector of strings containing
                       all is types of Raster2 Algebra
return value: -
exceptions: -

*/

void GetisTypes(std::vector<std::string>& risTypes);

/*
Method GetType returns corresponding type of given type
included in given types vector.

author: Dirk Zacher
parameters: rType - reference to a type
            rTypes - reference to a vector of strings of all types
                     of the type category of the returned type
return value: corresponding type of given type included in given types vector
exceptions: -

*/

std::string GetType(const std::string& rType,
                    const std::vector<std::string>& rTypes);

/*
Method GetValueWrapperType returns corresponding value wrapper type
of given type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: corresponding value wrapper type of given type
exceptions: -

*/

std::string GetValueWrapperType(const std::string& rType);

/*
Method GetMType returns corresponding moving type of given type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: corresponding moving type of given type
exceptions: -

*/

std::string GetMType(const std::string& rType);

/*
Method GettType returns corresponding Tile Algebra t type of given type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: corresponding Tile Algebra t type of given type
exceptions: -

*/

std::string GettType(const std::string& rType);

/*
Method GetmtType returns corresponding Tile Algebra mt type of given type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: corresponding Tile Algebra mt type of given type
exceptions: -

*/

std::string GetmtType(const std::string& rType);

/*
Method GetitType returns corresponding Tile Algebra it type of given type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: corresponding Tile Algebra it type of given type
exceptions: -

*/

std::string GetitType(const std::string& rType);

/*
Method GetsType returns corresponding Raster2 Algebra s type of given type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: corresponding Raster2 Algebra s type of given type
exceptions: -

*/

std::string GetsType(const std::string& rType);

/*
Method GetmsType returns corresponding Raster2 Algebra ms type of given type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: corresponding Raster2 Algebra ms type of given type
exceptions: -

*/

std::string GetmsType(const std::string& rType);

/*
Method GetisType returns corresponding Raster2 Algebra is type of given type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: corresponding Raster2 Algebra is type of given type
exceptions: -

*/

std::string GetisType(const std::string& rType);

/*
Method IsType checks if given type is included in given types vector.

author: Dirk Zacher
parameters: rType - reference to a type
            rTypes - reference to a vector of strings of all types to check
return value: true, if given type is included in given types vector,
              otherwise false
exceptions: -

*/

bool IsType(const std::string& rType,
            const std::vector<std::string>& rTypes);

/*
Method IsValueWrapperType checks if given type is a value wrapper type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: true, if given type is a value wrapper type, otherwise false
exceptions: -

*/

bool IsValueWrapperType(const std::string& rType);

/*
Method IsMType checks if given type is a moving type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: true, if given type is a moving type, otherwise false
exceptions: -

*/

bool IsMType(const std::string& rType);

/*
Method IstType checks if given type is a Tile Algebra t type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: true, if given type is a Tile Algebra t type, otherwise false
exceptions: -

*/

bool IstType(const std::string& rType);

/*
Method IsmtType checks if given type is a Tile Algebra mt type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: true, if given type is a Tile Algebra mt type, otherwise false
exceptions: -

*/

bool IsmtType(const std::string& rType);

/*
Method IsitType checks if given type is a Tile Algebra it type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: true, if given type is a Tile Algebra it type, otherwise false
exceptions: -

*/

bool IsitType(const std::string& rType);

/*
Method IssType checks if given type is a Raster2 Algebra s type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: true, if given type is a Raster2 Algebra s type, otherwise false
exceptions: -

*/

bool IssType(const std::string& rType);

/*
Method IsmsType checks if given type is a Raster2 Algebra ms type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: true, if given type is a Raster2 Algebra ms type, otherwise false
exceptions: -

*/

bool IsmsType(const std::string& rType);

/*
Method IsisType checks if given type is a Raster2 Algebra is type.

author: Dirk Zacher
parameters: rType - reference to a type
return value: true, if given type is a Raster2 Algebra is type, otherwise false
exceptions: -

*/

bool IsisType(const std::string& rType);

}

#endif // TILEALGEBRA_TYPES_H
