/*

1 Class SourceUnit2

This class is essentially a container for one ~SourceUnit~ A or B

*/

#ifndef SOURCEUNIT2_H_
#define SOURCEUNIT2_H_

#include <gmp.h>

#include "Refinement3.h"
#include "MovingRegion2Algebra.h"
#include "SetOps2.h"

namespace mregionops2 {

class PFace;
class SourceUnitPair2;


struct CycleInfo { 
         PFace* plumblineCandidate; 
         CycleStatus status;  
       };

class SourceUnit2 {

    friend class SourceUnitPair2;

public:

/*

1.1 Constructor

Note that the interval of the parameter ~uRegion~ might be changed!

*/

    SourceUnit2(const bool _isUnitA,
               MRegion2* const _mRegion,
               const int _pos,
               precTimeInterval _interval,
               SourceUnitPair2* const _parent);

/*

1.1 Destructor

*/
  
  ~SourceUnit2();

/*
1.1 Getter and setter methods

1.1.1 IsEmpty

Returns ~true~, if this ~SourceUnit~ represents an empty unit.

*/

    inline bool IsEmpty() const {
        return (pos == -1);
    }

    inline mpq_class GetStartTime() {
       return interval.start;
    }

    inline mpq_class GetEndTime() {
       return interval.end;
    }
    
    inline precTimeInterval GetTimeInterval() {
      return interval;
    }

    const SetOp GetOperation() const;

    void FinalizeIntSegs();

    inline bool IsUnitA() const {
        return isUnitA;
    }

    inline bool IsUnitB() const {
        return !isUnitA;
    }
    
    void SetCycleStatus(unsigned int pfaceNo, 
                        unsigned int cycleNo,  
                        CycleStatus status);    
    void AddTimestamp(mpq_class tim);
    unsigned int GetTimeIntervalCount();
    mpq_class GetTimeIntervalStart(unsigned int i);
    mpq_class GetTimeIntervalEnd(unsigned int i);   

private:
    void NormalizeTimeInterval();
    void ComputeBoundingRect();
    const Region GetTestRegion(const double t);

/*
1.1 Attributes

1.1.1 isUnitA

A flag, indicating which argument of the set operation
this ~SourceUnit~ represents: A or B.

*/

    const bool isUnitA;

/*
1.1.1 isEmpty

A flag, indicating that this ~SourceUnit~ is empty.

*/

    const int pos;

/*

1.1.1 uRegion

A pointer to the corresponding ~URegionEmb~.

Note that the interval of uRegion might be changed!

*/

    MRegion2* const mRegion;
    precTimeInterval interval;
/*

1.1.1 array

A pointer to the corresponding ~DBArray<MSegmentData>~.

1.1.1 partner

A pointer to  the other ~SourceUnit~ instance, which
is the other argument of the set operation.

*/

    SourceUnit2* partner;

/*

1.1.1 parent
A pointer the parent object.

*/

    SourceUnitPair2* const parent;

/*

1.1.1 pFaces
A ~std::vector~ to store all ~PFaces~.

*/

    vector<PFace*> pFaces;
    map<pair<unsigned int, unsigned int>, CycleInfo*> cycleInfo;


/*

1.1 Operators and Predicates
1.1.1 CreatePFaces

Creates a new ~PFace~ for each ~MSegmentData~ of this unit.

*/

    void CreatePFaces();

/*
1.1.1 CollectRelevantPFaces

Performs a linear scan over the list of all ~PFaces~,
collecting those which are relevant for the result and
pass them to the ~ResultUnitFactory~.

*/

    void CollectRelevantPFaces();

/*

1.1 Methods for debugging

*/

    void PrintPFaces();

/*

1.1.1 pFace

A pointer to the ~PFace~, this instance belongs to.

*/

    const PFace* pFace;


    void restrictSegtoInterval(MSegmentData2 sourceSeg, 
                            precTimeInterval sourceInt, 
                            precTimeInterval destInt, 
                            MSegmentData2* destSeg);


};
}
#endif
