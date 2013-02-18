/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

#ifndef RASTER2_IMPORT_ESRI_GRID_H
#define RASTER2_IMPORT_ESRI_GRID_H

#include <NList.h>

#include "../stype.h"
#include "../sint.h"
#include "../sreal.h"

namespace raster2
{
    const int INDEX_BUFFER_SIZE = 8;
    const string FILE_SEPARATOR = "/";

    const string BOUNDING_FILE_NAME = "dblbnd.adf";
    const string INDEX_FILE_NAME = "w001001x.adf";
    const string DATA_FILE_NAME = "w001001.adf";
    const string STATISTICS_FILE_NAME = "sta.adf";

    /*
    Set this to *true* in order to write lots of debug output.

    */
    const bool DEBUG_OUTPUT_ENABLED = false;

    extern ValueMapping importEsriGridFuns[];
    ListExpr importEsriGridTypeMap(ListExpr args);
    int importEsriGridSelectFun(ListExpr);
    template<class inClass> int importEsriGridFun
    (Word* args, Word& result, int message, Word& local, Supplier s);

    struct importEsriGridInfo : OperatorInfo
    {
      importEsriGridInfo()
      {
        name      = "importEsriGrid";
        signature = "{text|string} -> " + sint::BasicType();
        appendSignature("{text|string} -> " + sreal::BasicType());
        syntax    = "importEsriGrid(_)";
        meaning   = "Imports an ESRI Grid file.";
      }
    };

    struct EsriGridConfigData;

    bool readFileIntoBuffer(char* buffer, const string& fname,
                            const uint32_t len);
    void readEsriGridBounds(RasterData *EsriRasterData,
                            const string boundingFileName);
    void readEsriGridIndexConfig(RasterData *EsriRasterData,
                                 fstream *indexFile);
    void readStatisticData(RasterData *EsriRasterData,
                           const string statisticsFileName);
    void evaluateActualTilesRowAndColumnCount(RasterData *EsriRaster);
    int importEsriGridFun
    (Word* args, Word& result, int message, Word& local, Supplier s);
}

#endif /* #ifndef RASTER2_IMPORT_ESRI_GRID_H */
