#ifndef FLOB_H
#define FLOB_H

#include "SecondoSMI.h"

class FLOB {
private:
  static const int SWITCH_THRESHOLD;
  
  SmiRecordFile* lobFile;
  SmiRecord lob;
  SmiRecordId lobId;
  
  //char *start;
  int size;

  friend class Tuple;
  //friend class Polygon;
  //friend void *CastPolygon(void *addr); // this method needs access to lobFile
  
  bool SaveToLob();   /* Switch from Main Memory to LOB Representation */
  
public:
	char *start;
  FLOB(SmiRecordFile* inlobFile);
  FLOB(SmiRecordFile* inlobFile, int sz);   /* Create from scratch */
  ~FLOB();        /* Destroy LOB instance */
  
  void Destroy(); /* Destroy persistent representation */
 
  void Get(int offset, int length, char *target); /* Read by copying */
  void Write(int offset, int length, char *source);
   int Size();
  
  // to be worked out later
  // void Resize(int size);
  
   int Restore(char *address); /* Restore from byte string */
  bool IsLob();                /* true, if value stored in underlying LOB, otherwise false */
  
};
#endif
