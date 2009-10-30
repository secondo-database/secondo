/*



1 Class FlobId

This class encapsulates fileId, recordId and an offset within the 
record.

*/



#ifndef FLOBID_H
#define FLOBID_H

#include <stdint.h>



 // forward declaration of class FlobManager
class FlobManager;



class FlobId{
 friend class FlobManager;
 private:
   uint32_t fileId;  
   uint32_t recordId;
   uint32_t offset; 
};

#endif

