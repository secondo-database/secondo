/*

*/

#ifndef ALGEBRAS_DISTRIBUTED3_TESCLIENT_H_
#define ALGEBRAS_DISTRIBUTED3_TESCLIENT_H_

#include "Algebras/Relation-C++/RelationAlgebra.h"


namespace distributed3
{

class TESClient
{
  public:
    TESClient();
    ~TESClient(); // TODO nothing to destroy, so why a Destructor
	
    void putTuple(const int eid, const int slot, const int workerNumber, 
                                                              Tuple* tuple);
    void endOfTupleStreamFor(int eid);    
    Tuple* getTuple(int eid, int slot);
    
    static TESClient& get();
	
  private:      
    static TESClient tesClient;
    //static boost::mutex lock;
};

} /* namespace distributed3 */

#endif /* ALGEBRAS_DISTRIBUTED3_TESCLIENT_H_ */
