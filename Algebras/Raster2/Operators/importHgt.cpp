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

#include "importHgt.h"

namespace raster2
{
	ListExpr importHgtTypeMap(ListExpr args)
    {
	  const std::string error_message =
		  "expected single argument (stream text)";

	  if(!nl->HasLength(args, 1))
		return listutils::typeError(error_message);

	  ListExpr stream = nl->First(args);

      if(!Stream<FText>::checkType(stream))
       return listutils::typeError(error_message);

	  return nl->SymbolAtom(sint::BasicType());
    }

    int importHgtFun
    (Word* args, Word& result, int message, Word& local, Supplier s){
     //File Stuff
     vector<string> Files, failedFiles;
   	 bool init = true;

   	 //Stream Stuff
   	 Stream<FText> stream(args[0]);
   	 stream.open();

   	 //build HGT FilesVector
   	 FText* next = stream.request();
   	 while(next!=0){
   	  Files.push_back(next->GetValue());

   	  next->DeleteIfAllowed();
   	  next = stream.request();
   	 }
   	 stream.close();

   	 //get handle to result object (sint)
   	 result = qp->ResultStorage(s);
     sint* sintObject = static_cast<sint*>(result.addr);
     sintObject->clear();

   	 if(Files.size() > 0) {
   	  RasterData *HGTRasterData = new RasterData(false);

   	  //get raster storage
	  sint::storage_type& rs = sintObject->getStorage();

   	  // sort using default comparison (operator <) optional
//      std::sort(Files.begin(), Files.end());

   	  //iterate thru the files
   	  vector<string>::const_iterator FilesIterator;

   	  int fileCounter = 0;
   	  for(FilesIterator=Files.begin();
   		  FilesIterator!=Files.end();
   		  FilesIterator++)
   	  {

   	   const string &filename = static_cast<string>( *FilesIterator );
   	   const char *currentHGTFile = filename.c_str();
       cout << "Processing file: " << currentHGTFile << endl;
       cout << "File           : "
            << ++fileCounter << " of " << Files.size() << endl;

   	   if(sintObject->importHgtFile(filename.c_str(),
        HGTRasterData, init, rs) == 0)
   	   {
   		if(init)
   		 init = false;
   	   } else
   		failedFiles.push_back(currentHGTFile);
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

   	  delete HGTRasterData;
   	  HGTRasterData = 0;
   	 } else
      cout << "No data imported!" << endl;

   	 return 0;
    }

    template <typename T, typename Helper>
	int stype<T, Helper>::importHgtFile(const char *currentHGTFile,
		RasterData *HGTRasterData, bool init,
		storage_type& rs) {
      long valuesUndef = 0;
      long valuesSkipped = 0;
	  size_t valuesWritten = 0;

	  int currentHGTExtend =
	      HGTRasterData->getHGTExtendfromFile(currentHGTFile);

	  if( currentHGTExtend == -1)
	   return 1;

	  if(!HGTRasterData->checkHGTFile(currentHGTFile, currentHGTExtend))
       return 1;

	  //Use this setup to import the whole file
	  int lowerX = 0;
	  int upperX = currentHGTExtend;
	  int lowerY = 0;
	  int upperY = currentHGTExtend;

	  //get pattern coordinates from filename
	  string lengthOrientation = HGTRasterData->
			  getLengthOrientationFromFileName(currentHGTFile);
	  int lengthOrientationValue = HGTRasterData->
			  getLengthOrientationValueFromFileName(currentHGTFile);
	  string widthOrientation = HGTRasterData->
			 getWidthOrientationFromFileName(currentHGTFile);
	  int widthOrientationValue = HGTRasterData->
		     getWidthOrientationValueFromFileName(currentHGTFile);

	  //perfrom init of sint object according to first file
	  if(init) {
       size_t cacheSize =
       size_t(2+currentHGTExtend/storage_type::tile_size);
       rs.setCacheSize(cacheSize);

       double initLength = lengthOrientationValue;
	   double initWidth = widthOrientationValue;

	   if(lengthOrientation.compare("W") == 0)
	    initLength = 0 - initLength;

	   if(widthOrientation.compare("S") == 0)
	    initWidth = 0 - initWidth;

	   double cellsize = 1.0 / (double) (currentHGTExtend-1);
	   double Length_min = initLength - cellsize/2;
	   double Width_min = initWidth - cellsize/2;

	   grid = grid2(Length_min, Width_min, cellsize);
//	   grid = grid2(0.0, 0.0, 1.0);

	   minimum = 0 - HGTRasterData->getHGTUndef();
	   maximum = HGTRasterData->getHGTUndef();
	  }

	   //calculate coordinates in respect to first file
	   HGTRasterData->calculateCoordinates(
				lengthOrientation, lengthOrientationValue,
				widthOrientation, widthOrientationValue);

	   long offsetX = HGTRasterData->getXOffset()
				* (currentHGTExtend - 1);
	   long offsetY = HGTRasterData->getYOffset()
				* (currentHGTExtend - 1);

	   //read the file into buffer
	   int16_t* buffer =
		 HGTRasterData->getHGTData(currentHGTFile);

	   if(buffer != NULL){
		if(!(this->checkHGTConsistency(rs, offsetX, offsetY,
				currentHGTExtend, HGTRasterData, buffer)))
			return 1;
	   } else
		return 1;

	   double posOriginX =
             (offsetX * (1.0 / (double) (currentHGTExtend-1)))
              + grid.getOriginX();

	   double posOriginY =
             (offsetY * (1.0 / (double) (currentHGTExtend-1)))
              + grid.getOriginY();

	   cout << "Grid origin at : "
			<< posOriginX << ", "
			<< posOriginY << endl;

	   for(int y = 0; y < currentHGTExtend; y++){

		if(y % ((currentHGTExtend-1)/100) == 0){
         cout << "\rProcessing: " << y/((currentHGTExtend-1)/100) << "%";
		 fflush (stdout);
		}

		for(int x = 0; x < currentHGTExtend; x++){
		 int y1 = currentHGTExtend - (y + 1);
		 int pos = y1*currentHGTExtend + x;
		 RasterIndex<2> ri = (int[]){(x + offsetX), (y + offsetY)};
		 int16_t v = buffer[pos];

		 if(HGTRasterData->getEndianTypeLittle())
		  v = HGTRasterData->convertEndian(v);

		 if(v != HGTRasterData->getHGTUndef()){
		  if(rs[ri] == Helper::getUndefined()){
		   if((x >= lowerX && x < upperX) &&
             (y >= lowerY && y < upperY)){
			rs[ri] = v;

			if(v < minimum)
			 minimum = v;

			if(v > maximum)
			 maximum = v;

			valuesWritten++;
		   } else
			valuesSkipped++;
		  } else
		   valuesSkipped++;
		 } else
		  valuesUndef++;
		}// column count
	   }// row count

	   // print out some statistics
	   cout << "\rMinimum        : " << minimum << endl;
	   cout << "Maximum        : " << maximum << endl;
	   cout << "Voids          : " << valuesUndef << endl;
	   cout << "Values skipped : " << valuesSkipped << endl;
	   cout << "Values written : " << valuesWritten << endl;
	   cout << endl;

	   delete[] buffer;

	   return 0;
	}

    template <typename T, typename Helper>
	bool stype<T, Helper>::checkHGTConsistency(storage_type& rs,
            long offsetX,
            long offsetY,
            int extend,
            RasterData *HGTRasterData,
            int16_t* data) {
     bool fileOK = true;

     if((offsetX != 0) || offsetY != 0){
      for(int index = 0; index<extend; index++){

       if(index % ((extend-1)/100) == 0){
        cout << "\rConsistency    : " << index/((extend-1)/100) << "%";
  		fflush (stdout);
       }

       int row = extend - (index + 1);

	   //left column
	   int posLeftCol = row*extend;
	   RasterIndex<2> rsLeftCol =
			   (int[]){offsetX, (index + offsetY)};
	   int16_t valueLeftCol = data[posLeftCol];

	   //lower row
	   int posLowerRow = ((extend-1)*extend) + index;
	   RasterIndex<2> rsLowerRow =
			   (int[]){(index + offsetX), offsetY};
	   int16_t valueLowerRow = data[posLowerRow];

	   //upper row
	   int posUpperRow = index;
	   RasterIndex<2> rsUpperRow =
			   (int[]){(index + offsetX), (offsetY + (extend-1))};
	   int16_t valueUpperRow = data[posUpperRow];

	   //right Column
	   int posRightCol = posLeftCol + (extend-1);
	   RasterIndex<2> rsRightCol =
			   (int[]){(offsetX + (extend-1)), (index + offsetY)};
	   int16_t valueRightCol = data[posRightCol];

	   if(HGTRasterData->getEndianTypeLittle()) {
		valueLeftCol = HGTRasterData->convertEndian(valueLeftCol);
		valueLowerRow = HGTRasterData->convertEndian(valueLowerRow);
		valueUpperRow = HGTRasterData->convertEndian(valueUpperRow);
		valueRightCol = HGTRasterData->convertEndian(valueRightCol);
	   }

	   if((rs[rsLeftCol] != Helper::getUndefined()) &&
			   (rs[rsLeftCol] != valueLeftCol))
		   fileOK = false;

	   if((rs[rsLowerRow] != Helper::getUndefined()) &&
			   (rs[rsLowerRow] != valueLowerRow))
		   fileOK = false;

	   if((rs[rsUpperRow] != Helper::getUndefined()) &&
			   (rs[rsUpperRow] != valueUpperRow))
		   fileOK = false;

	   if((rs[rsRightCol] != Helper::getUndefined()) &&
			   (rs[rsRightCol] != valueRightCol))
		   fileOK = false;
      }
     }

     if(fileOK)
      cout << "\rConsistency    : OK  " << endl;
     else
      cout << "\rConsistency    : failed, skipping file" << endl;

     return fileOK;
    }
}
