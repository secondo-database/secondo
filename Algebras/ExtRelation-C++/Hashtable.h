/*


*/

#ifndef SEC_HASHTABLE_H 
#define SEC_HASHTABLE_H 


#include <vector>
#include <list>

#include "LogMsg.h"
#include "RelationAlgebra.h"

struct CmpTuples {

  int indA;
  int indB;

  CmpTuples(const int indexA, const int indexB ) : 
    indA( indexA ),
    indB( indexB )
  {
    std::cout << "indA:" << indA << " indB:" << indB << std::endl;	  
    assert(indA >= 0);
    assert(indB >= 0);
  }

  inline int operator()(const Tuple* ta, const Tuple* tb) const
  {
    /* tuples with NULL-Values in the join attributes
       are never matched with other tuples. */
    Attribute* a = static_cast<Attribute*>( ta->GetAttribute(indA) );
    Attribute* b = static_cast<Attribute*>( tb->GetAttribute(indB) );

    //cout << "a: " << *a << endl;
    //cout << "b: " << *b << endl;

    /* The cases below should be handled already in the Attribute's
       Compare function

    if( !a->IsDefined() ) {
      return -1;
    }

    if( !b->IsDefined() ) {
      return 1;
    }
    */

    return a->Compare(b);
  }

};

typedef unsigned int HashVal;


class HashBucket {

  public:
  typedef std::list<Tuple*> BucketList;
  typedef BucketList::const_iterator const_iterator;
  typedef BucketList::iterator iterator;

    HashBucket() : 
      numOfMatches( 0 ),
      numOfProbes( 0 ),
      length( 0 ),
      usedMem( 0 )
    {}
    ~HashBucket() {}


    inline long append(Tuple* t) 
    {
      tuples.push_back( t );
      long mem = t->GetSize();
      usedMem += mem;
      length++;

      return mem;
    }

    const_iterator begin() const { return tuples.begin(); }
    const_iterator end() const   { return tuples.end(); }

    iterator begin() { return tuples.begin(); }
    iterator end()   { return tuples.end(); }

    long clear() 
    {
      BucketList::iterator it = tuples.begin();   
      while( it != tuples.end() )
      {
        delete *it;
        it++;
      }
      tuples.clear();
      usedMem = 0;
      length = 0;
      return usedMem;
    }

    unsigned int& getMatchCtr() { return numOfMatches; }
    unsigned int& getProbeCtr() { return numOfProbes; }

    unsigned int getMatches() const { return numOfMatches; }
    unsigned int getProbes()  const { return numOfProbes; }
    unsigned int getLength()  const { return length; }
    inline   long  getUsedMem() const { return usedMem; }

  protected:
    BucketList tuples;

    unsigned int numOfMatches;
    unsigned int numOfProbes;
    unsigned int length;
    long usedMem;

};


class HashTable {
 
  public:

  typedef std::vector<HashBucket> BucketTable;
  typedef BucketTable::const_iterator const_iterator;
  typedef BucketTable::iterator iterator;

    HashTable( const int buckets, CmpTuples cmpObj ) :
      n( buckets ),
      usedMem( 0 ),
      cmp( cmpObj ),
      table( buckets ),
      probeBucket(0)
    {
      TRACE("HashTable()")
    }
 
    ~HashTable()
    {}

/*
Function ~add~ inserts tuples into bucket with hash
value h. Memory overflow situations must be handled
where ~Hashable~ objects are used, e.g. The value mapping
function of an operator.

*/

    inline bool add(Tuple* t, HashVal h) 
    {
      HashBucket& b = getBucket(h);
      usedMem += b.append(t);
      return true;
    }

    inline HashBucket& getBucket(HashVal h) 
    {  
      return table[h % n];
    }

/*
The function ~probe~ returns a matching tuple. If the end
of the bucket is reached a 0 will be returned.
 
*/

    void initProbe(const HashVal h)
    {
      probeBucket = &getBucket(h);
      probeBucket->getProbeCtr()++;
      probeIter = probeBucket->begin();
      //cout << "initProbe() for bucket " << h % n << endl;
    }

    inline Tuple* probe(const Tuple* t)
    {
      if (!probeBucket) {
        return 0;
      }	

      unsigned int& m = probeBucket->getMatchCtr();
      while ( probeIter != probeBucket->end() ) 
      {
        Tuple* result = *probeIter;
        probeIter++;
	//cout << "t     :" << *t << endl;
	//cout << "result:" << *result << endl;
        if ( cmp(result,t) == 0 ) {
          m++; 
          return result;
        } 
      }
      return 0;
    }

    inline long getUsedMem() const { return usedMem; }

    void clearBuckets()
    {
      TRACE("clearBuckets()")
      HashTable::iterator bucket = table.begin();

      while(bucket != table.end() )
      {
        bucket->clear();
        bucket++;
      }
    }


    const_iterator begin() const { return table.begin(); }
    const_iterator end() const   { return table.end(); }

    iterator begin() { return table.begin(); }
    iterator end()   { return table.end(); }

    size_t size() { return table.size(); } 

    int tuplesInMem()
    {
      int tuples = 0;
      iterator it = begin();
      while( it != end() ) 
      {
        tuples += it->getLength();
        it++;
      }
      return tuples;
    } 


    void dumpBucketStatistics(const std::string& prefix, const int seqNr) const
    {
      std::stringstream fileName;
      fileName << prefix << "-" << seqNr; 
      HashTable::const_iterator it = table.begin();
      const std::string sep("|");
      int bucketNr = 0;

      cmsg.file( fileName.str() )
        << "# Hashtable bucket info : BucketNr " << sep 
        << " length " << sep 
        << " usedMem " << sep 
        << " probed " << sep
        << " matches " << std::endl;
      cmsg.send(); 

      while ( it != table.end() )  
      { 
        cmsg.file( fileName.str() )
          << bucketNr << sep 
          << it->getLength() << sep
          << it->getUsedMem() << sep
          << it->getProbes() << sep
          << it->getMatches() << std::endl;
        cmsg.send();

        it++;
        bucketNr++;
      }
    }

  private:

    const int n;
    long usedMem;
    CmpTuples cmp;

    // members used for the memory part of the hash table 
    BucketTable table;
    HashBucket::iterator probeIter;
    //HashBucket::const_iterator probeEnd;
    HashBucket* probeBucket;
   
};

#undef TRACE_ON

#endif
