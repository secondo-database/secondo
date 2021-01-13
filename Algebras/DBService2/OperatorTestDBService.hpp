#ifndef ALGEBRAS_DBSERVICE_OPERATOR_TEST_DB_SERVICE_HPP_
#define ALGEBRAS_DBSERVICE_OPERATOR_TEST_DB_SERVICE_HPP_

#include "Operator.h"

namespace DBService
{

  /*

1.1.1 Operator Specification

*/

  struct TestDBServiceInfo : OperatorInfo
  {
    TestDBServiceInfo()
    {
      name = "testdbservice";
      signature = "-> bool";
      syntax = "testdbservice()";
      meaning = "run DBService unit tests";
      example = "query testdbservice()";
      remark = "None";
      usesArgsInTypeMapping = false;
    }
  };

  class OperatorTestDBService {

  public:
    /*

1.1.1 Type Mapping Function

*/
    static ListExpr mapType(ListExpr nestedList);

    /*

1.1.1 Value Mapping Function

*/
    static int mapValue(Word *args,
                        Word &result,
                        int message,
                        Word &local,
                        Supplier s);
  };

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_OPERATORUNUNITTESTS_HPP_ */
