/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

{\Large \bf Anhang D: RTree-Template }

[1] Header-File of TMRTree 

2012, June Jianqiu 

[TOC]

0 Overview

This header file implements a disk-resident representation of a R-Tree.
Setting some parameters the R-Tree-behaviour of Guttman or the R[*]-Tree
of Kriegel et al. can be selected.

The R-Tree is implemented as a template to satisfy the usage with various
dimensions. The desired dimensions are passed as a parameter to the template.

1 Defines and Includes

*/

#ifndef __MODERTREE_H__
#define __MODERTREE_H__

#include "stdarg.h"

#ifdef SECONDO_WIN32
#define Rectangle SecondoRectangle
#endif

#include <iostream>
#include <stack>
#include <limits>
#include <string.h>

using namespace std;

#include "SpatialAlgebra.h"
#include "RelationAlgebra.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "RectangleAlgebra.h"
#include "StandardTypes.h"
#include "BTreeAlgebra.h"
#include "TemporalAlgebra.h"
#include "AlmostEqual.h"
#include "TMRTree.h"

extern NestedList* nl;
extern QueryProcessor* qp;

#define MAXMODE 9 //modes are defined in GeneralType.h

class Space;
//long tm in tmrtree node, 4 bytes
const int REFBIT_NO = 4*8;

/*
a set of tmrtrees, each is responsible for movements with one mode

*/
class Mode_RTree{
  	SmiFileId m_list[MAXMODE];
	TM_RTree<3,TupleId>* m_trees[MAXMODE];
	
  public:
/*
The first constructor. Creates an empty R-tree.

*/
    Mode_RTree();
	~Mode_RTree();
	inline static const string BasicType(){
        return "modertree";
    }
    inline SmiFileId GetFileId(unsigned int i){
	  if(0 <= i && i < MAXMODE){
		return m_list[i];
	  }else{
		cout<<"illegal entry id"<<endl;
//		assert(false);
		return 0;
	  }
	}
    void SetFileId(unsigned int i, SmiFileId fileid)
    {
//	  cout<<"setfile id "<<endl;
	  assert(0 <= fileid);
	  if(0 <= i && i < MAXMODE){
		  m_list[i] = fileid;
		  ///////initialize the pointer to tmrtree
		  if(fileid == 0){
		  
		  }else{
			  ///////// open the tree structure //////////////////
			  m_trees[i] = new TM_RTree<3,TupleId>(fileid);
// 			  cout<<"RootID "<<m_trees[i]->RootRecordId()
// 			      <<" Box "<<m_trees[i]->BoundingBox()
// 			      <<" EntryCount "<<m_trees[i]->EntryCount()
// 			      <<" NodeCount "<<m_trees[i]->NodeCount()<<endl;
		  }
	  }
	
	}
	inline int TreeCount()
	{
	  int count = 0;
	  for(unsigned int i = 0;i < MAXMODE;i++){
		if(m_list[i] > 0) count++;
	  }
	  return count;
	}
	void CloseTree();
	void DeleteTree();
    bool InitializeBulkLoad(const bool &leafSkipping = BULKLOAD_LEAF_SKIPPING);
	bool FinalizeBulkLoad();
	void BulkLoad(const R_TreeEntry<3>& entry, int, int);
	bool CalculateRef(Relation*, int, Space*);
	TM_RTree<3,TupleId>* SubTree(int m);
	int ModeBitPosition(int min, int counter, int cur);
	void UpdateBusTree(TM_RTree<3,TupleId>* tm_rtre, Relation* rel,
				       int attr_pos, Space* sp);
	long CalculateNodeRef(TM_RTree<3,TupleId>* tm_rtree,
			  SmiRecordId node_id, Relation* rel,
			  int attr_pos, map<int,int>& trip_bit_ref);
	void UpdateMetroTree(TM_RTree<3,TupleId>* tm_rtre, Relation* rel,
				       int attr_pos, Space* sp);
	void UpdateRoadTree(TM_RTree<3,TupleId>* tm_rtre, Relation* rel,
				       int attr_pos, Space* sp);
	void UpdateWalkTree(TM_RTree<3,TupleId>* tm_rtre, Relation* rel,
				       int attr_pos, Space* sp);
	void UpdateIndoorTree(TM_RTree<3,TupleId>* tm_rtre, Relation* rel,
				       int attr_pos, Space* sp);
};

int SizeOfModeRTree();
void* CastModeRTree( void* addr);
Word CloneModeRTree( const ListExpr typeInfo, const Word& w );
Word InModeRTree( ListExpr typeInfo, ListExpr value,
              int errorPos, ListExpr& errorInfo, bool& correct );
ListExpr OutModeRTree(ListExpr typeInfo, Word value);
void CloseModeRTree( const ListExpr typeInfo, Word& w );
void DeleteModeRTree( const ListExpr typeInfo, Word& w );
Word CreateModeRTree( const ListExpr typeInfo );
bool OpenModeRTree( SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value );
bool SaveModeRTree( SmiRecord& valueRecord, size_t& offset,
                const ListExpr typeInfo, Word& value );

inline int BusBitPosition(int, int, int);

/*
the amount of minutes
status

*/
struct Obj_Dur{
  double min;
  bool status;
  Obj_Dur():min(0),status(false){}
  Obj_Dur(double& m):min(m), status(false){}
  Obj_Dur(const Obj_Dur& obj):min(obj.min),status(obj.status){}
  Obj_Dur& operator=(const Obj_Dur& obj)
  {
	min = obj.min;
	status = obj.status;
	return *this;
  }
  void Add(double m){ min += m;}
  double GetMin(){return min;}
  bool Status(){return status;}
  void SetStatus(bool b){status = b;}
  void Print(){cout<<min<<" "<<status<<endl;}
};

struct Q_ModeRtree{
  unsigned int count;
  TupleType* resulttype; 
  
  Q_ModeRtree(){ count = 0; resulttype = NULL;} 
  ~Q_ModeRtree(){if(resulttype != NULL) delete resulttype;}
  
  
  vector<int> oid_list;  
  vector<int> level_list;
  vector<int> ref_list1;
  vector<int> ref_list2;
  vector<int> bit_pos_list;
  
  void PrintSubTree(Mode_RTree*, int, Space*);
  void PrintBusTree(Mode_RTree* m_rtree, int m, Space* sp);
  void InitBitRef_Bus(map<int, EntryItem>& bit_ref, Space* sp);
  void GetNodes(TM_RTree<3,TupleId>* tmrtree, SmiRecordId node_id, 
				int level, map<int, EntryItem> bit_ref);
  void GetNodes2(TM_RTree<3,TupleId>* tmrtree, SmiRecordId node_id, int level);
  
  void PrintMetroTree(Mode_RTree* m_rtree, int m, Space* sp);
  void InitBitRef_Metro(map<int, EntryItem>& bit_ref, Space* sp);
  void PrintRoadTree(Mode_RTree* m_rtree, int m, Space* sp);
  void PrintWalkTree(Mode_RTree* m_rtree, int m, Space* sp);
  void PrintIndoorTree(Mode_RTree* m_rtree, int m, Space* sp);
  void InitBitRef_Road(map<int, EntryItem>& bit_ref, Space* sp);
};
#endif
