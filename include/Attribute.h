#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "TupleElement.h"

class Attribute : public TupleElement
{
 public:
  virtual int        Compare( Attribute *attrib ) = 0;
  virtual Attribute* Clone()     = 0;
  virtual bool       IsDefined() = 0;
  virtual int        Sizeof()    = 0;
};

#endif
