/*

*/
#ifndef SECONDO_DPDTypeMapper_H
#define SECONDO_DPDTypeMapper_H

#include <string>


namespace distributed3 {
class DPDTypeMapper {

  public:
  
  static bool arraySubtypeEqualsFunctionArgument(ListExpr arrayType, 
                                        ListExpr dmap1Type);
  static ListExpr getResultType(ListExpr function);
  static ListExpr getLastArgumentOf(ListExpr function);
  static ListExpr replaceTypeOperator(ListExpr functionValue, 
                                      ListExpr replacement);
  
  virtual std::string err() = 0;
  void printValues();
  ListExpr typeMapping();
  
  protected:
  std::string getErrorMessage();
  ListExpr typeError();
  
  bool isRel();
  bool isStream();
  
  virtual bool rightNumberOfArgs() = 0;
  bool usesArgsInTypeMapping();
  virtual bool checkArgs() = 0;
  bool checkArrayType();
  bool checkNameType();
  //bool checkDmap1Type();
  bool checkPartitionType();
  bool checkNumberOfSlotsType();
  bool checkDmap2Type();
  
  virtual bool checkInterdependencies() = 0;
  
  ListExpr result();
  virtual ListExpr append() = 0;
  virtual ListExpr appendPartition() = 0;
  ListExpr appendDmap2();
  ListExpr resultType();
  
  ListExpr args;
  ListExpr array;  // array
  ListExpr name; // name of the result
  ListExpr dmap1; // function
  ListExpr partitionfunction; // redistribution function
  ListExpr numberOfSlots;  // size of the result
  ListExpr dmap2; 
  
  std::string msg;
  
};
}

#endif // SECONDO_DPDTypeMapper_H
