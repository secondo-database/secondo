/*

*/
#ifndef SECONDO_DMAPPDMAPTypeMapper_H
#define SECONDO_DMAPPDMAPTypeMapper_H

#include <string>
#include "./DPDTypeMapper.h"


namespace distributed3 {
class DmapPdmapTypeMapper : public DPDTypeMapper {

  public:
  DmapPdmapTypeMapper(ListExpr& args);
  std::string err();
  int numberOfArgs();
  bool checkArgs();
  bool checkDmap1Type();
  bool checkInterdependencies();
  ListExpr append();
  ListExpr appendDmap1();
  ListExpr appendPartition();

};
} // end namespace
#endif 
