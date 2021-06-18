/*

*/
#ifndef SECONDO_PARTITIONDMAPTypeMapper_H
#define SECONDO_PARTITIONDMAPTypeMapper_H

#include <string>
#include "./DPDTypeMapper.h"


namespace distributed3 {
class PartitiondmapTypeMapper : public DPDTypeMapper {

  public:
  PartitiondmapTypeMapper(ListExpr& args);
  std::string err();
  bool rightNumberOfArgs();
  bool checkArgs();
  bool checkInterdependencies();
  ListExpr result();
  ListExpr appendPartition();
};

} // end namespace
#endif 
