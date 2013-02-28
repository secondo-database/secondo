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

#include "importEsriRaster.h"
#include <math.h>

namespace raster2
{
	ListExpr importEsriRasterTypeMap(ListExpr args)
	{
	  const std::string error_message =
		  "expected single argument (stream text)";

	  if(!nl->HasLength(args, 1))
       return listutils::typeError(error_message);

	  ListExpr stream = nl->First(args);

      if(!Stream<FText>::checkType(stream))
       return listutils::typeError(error_message);

	  return nl->SymbolAtom(sreal::BasicType());
	}

	int importEsriRasterFun
        (Word* args, Word& result, int message, Word& local, Supplier s)
    {
     //File Stuff
     vector<string> Files, failedFiles;
     bool init = true;

     //Stream Stuff
     Stream<FText> stream(args[0]);
     stream.open();

     //build FilesVector
     FText* next = stream.request();
     while(next!=0){
      Files.push_back(next->GetValue());

      next->DeleteIfAllowed();
      next = stream.request();
     }
     stream.close();

     //get handle to result object (sint)
     result = qp->ResultStorage(s);
  	 sreal* srealObject = static_cast<sreal*>(result.addr);
     srealObject->clear();

     if(Files.size() > 0) {
   	  RasterData *EsriRasterData = new RasterData(false);

   	  //get raster storage
	  sreal::storage_type& rs = srealObject->getStorage();

   	  // sort using default comparison (operator <):
//   	  std::sort(Files.begin(), Files.end());

   	  //iterate thru the files
   	  vector<string>::const_iterator FilesIterator;

   	  int fileCounter = 0;
   	  for(FilesIterator=Files.begin();
   		  FilesIterator!=Files.end();
   		  FilesIterator++)
   	  {
       const string &filename = static_cast<string>( *FilesIterator );
   	   const char *currentEsriFile = filename.c_str();
   	   cout << "Processing file: " << currentEsriFile << endl;
   	   cout << "File           : "
            << ++fileCounter << " of " << Files.size() << endl;

   	   if(srealObject->importEsriRasterFile(filename.c_str(),
        EsriRasterData, init, rs) == 0)
   	   {
   		if(init)
   		 init = false;
   	   } else
      	failedFiles.push_back(currentEsriFile);
   	  }

   	  //print failed files
   	  if(failedFiles.size() > 0){
       vector<string>::const_iterator failedFilesIterator;
       cout << endl << failedFiles.size()
    		        << " files failed during import." << endl;
       for(failedFilesIterator=failedFiles.begin();
        failedFilesIterator!=failedFiles.end();
        failedFilesIterator++)
	   {
	   	const string &failed =
	   			static_cast<string>( *failedFilesIterator );
	   	cout << "\"" << failed.c_str() << "\"" << endl;
	   }
   	  } else
  	   cout << endl << "All files successfully processed." << endl;

   	  delete EsriRasterData;
   	  EsriRasterData = 0;
   	 } else
   	  cout << endl << "No data imported." << endl;

     return 0;
    }

    template <typename T, typename Helper>
    int stype<T, Helper>::importEsriRasterFile(
    		const char *currentEsriRasterFile,
    		RasterData *EsriRasterData, bool init,
    		storage_type& rs) {
     long valuesUndef = 0;
     long valuesSkipped = 0;
  	 size_t valuesWritten = 0;

 	 if(EsriRasterData->getEsriRasterHDR(currentEsriRasterFile, init) != 0)
 	  return 1;

  	 if(!EsriRasterData->checkRasterExtend())
  	  return 1;

  	 long offsetX = EsriRasterData->getRasterXOffset();
  	 long offsetY = EsriRasterData->getRasterYOffset();
     long allValues = EsriRasterData->currentEsriHDR.nrows *
    		 EsriRasterData->currentEsriHDR.ncols;
     long unit = allValues/100;

  	 if(!EsriRasterData->checkRasterOffset(offsetX, offsetY))
  	  return 1;

	 double *buffer =
            EsriRasterData->getEsriRasterData(currentEsriRasterFile);

	 if(buffer != NULL){
	  if(init){
	   //set storage cache
	   if(EsriRasterData->currentEsriHDR.ncols >
	       EsriRasterData->currentEsriHDR.nrows) {
	    if(EsriRasterData->currentEsriHDR.ncols > storage_type::tile_size){
	     size_t cacheSize =
	      (2+EsriRasterData->currentEsriHDR.ncols/storage_type::tile_size);
	     rs.setCacheSize(cacheSize);
	    }
	   } else {
	    if(EsriRasterData->currentEsriHDR.nrows > storage_type::tile_size){
	     size_t cacheSize =
	      (2+EsriRasterData->currentEsriHDR.nrows/storage_type::tile_size);
	     rs.setCacheSize(cacheSize);
	    }
	   }

	   //initiliaze the gird
	   double Length_min = 0.0;
	   double Width_min = 0.0;

	   if(EsriRasterData->getUseCenter()){
	    Length_min = EsriRasterData->originEsriHDR.xllcenter -
	  			     EsriRasterData->originEsriHDR.cellsize/2;
	  	Width_min = EsriRasterData->originEsriHDR.yllcenter -
	  			    EsriRasterData->originEsriHDR.cellsize/2;
       } else {
	  	Length_min = EsriRasterData->originEsriHDR.xllcorner;
	  	Width_min = EsriRasterData->originEsriHDR.yllcorner;
       }
       grid = grid2(Length_min, Width_min,
				       EsriRasterData->originEsriHDR.cellsize);
       minimum = 0 - EsriRasterData->originEsriHDR.nodata_value;
       maximum = EsriRasterData->originEsriHDR.nodata_value;
	  }

      double posOriginX =
        (offsetX * (1.0 / (double) EsriRasterData->currentEsriHDR.extend))
        + grid.getOriginX();

	  double posOriginY =
        (offsetY * (1.0 / (double) EsriRasterData->currentEsriHDR.extend))
         + grid.getOriginY();

	  cout << "Grid origin at : "
		   << posOriginX << ", "
		   << posOriginY << endl;

      for(int row = EsriRasterData->currentEsriHDR.nrows - 1; row >= 0; row--){
       for(int col = 0; col < EsriRasterData->currentEsriHDR.ncols; col++){
	    RasterIndex<2> ri = (int[]){(col + offsetX), (row + offsetY)};
         double v = buffer[(((EsriRasterData->currentEsriHDR.nrows - 1) - row) *
			   EsriRasterData->currentEsriHDR.ncols) + col];

	    if(v != EsriRasterData->currentEsriHDR.nodata_value){
         const T& v1 = rs[ri];

         if(isnan(v1)){
          rs[ri] = v;

          if(v < minimum)
           minimum = v;

		  if(v > maximum)
		   maximum = v;

          valuesWritten++;
         } else
          valuesSkipped++;
        } else
         valuesUndef++;

	    int row1 = ((EsriRasterData->currentEsriHDR.nrows - 1) - row);
	    if(((row1 == 0) && (col == 0)) ||
          ((((row1 * EsriRasterData->currentEsriHDR.ncols)+(col+1)) %
           unit == 0) && (((row1 * EsriRasterData->currentEsriHDR.ncols) +
            (col+1)) >= unit))){
           cout << "\rProcessing: "
                << ((row1 * EsriRasterData->currentEsriHDR.ncols)+(col+1))
                / unit << "%";
           fflush (stdout);
        }
       }
      }

	  // print out some statistics
	  cout << "\rMinimum        : " << minimum << endl;
	  cout << "Maximum        : " << maximum << endl;
	  cout << "Voids          : " << valuesUndef << endl;
      cout << "Values skipped : " << valuesSkipped << endl;
	  cout << "Values written : " << valuesWritten << endl;
	  cout << endl;

	  delete[] buffer;
	 } else {
      cout << endl;
      return 1;
	 }

	 return 0;
    }
}
