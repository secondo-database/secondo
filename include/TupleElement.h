#ifndef TUPLE_ELEMENT_H
#define TUPLE_ELEMENT_H

#ifndef TYPE_ADDRESS_DEFINED
#define TYPE_ADDRESS_DEFINED
typedef void* Address;
#endif

#ifndef TYPE_FLOB_DEFINED
#define TYPE_FLOB_DEFINED
typedef void FLOB;
#endif

class TupleElement // renamed, previous name: TupleElem
{
 public:
  TupleElement(){};
  virtual ~TupleElement() {};
  virtual int      NumOfFLOBs() { return (0); };
  virtual FLOB*    GetFLOB( int ){ return (0); };
  virtual ostream& Print( ostream& os ) { return (os << "??"); };
};

#endif

