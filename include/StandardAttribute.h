#ifndef STANDARDATTRIBUTE_H
#define STANDARDATTRIBUTE_H

#include "Attribute.h"

class StandardAttribute : public Attribute
{
 public:
  virtual void* GetValue() = 0;
};

#endif
  
