
/*



*/


#include <vector>
#include <iostream>
#include "NestedList.h"
#include "RelationAlgebra.h"



class NMEALineImporter;



/*
2 Class NMEAIMPORTER

*/
class NMEAImporter{
  public: 

     NMEAImporter();

     ~NMEAImporter();


     bool setType(const string& type="GGA");


     bool scanFile(const string& fileName, string& errorMessage);

     string getKnownTypes() const;

     ListExpr getTupleType();
     
     Tuple* nextTuple();

  private:
     vector<NMEALineImporter*> importers;  // all available line importers
     int position;                        // current selected importer
     ifstream in;                         // current input stream
     
    
};



