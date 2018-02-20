/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] The BTree2 Algebra Class 

[TOC]

0 Overview

*/

#include "BTree2Algebra.h"

#include "Algebra.h"
#include "AlgebraManager.h"
#include "QueryProcessor.h"
#include "NestedList.h"
#include "op_createbtree2.h"
#include "op_rangebtree2.h"
#include "op_describebtree2.h"
#include "op_getentry2.h"
#include "op_getFileInfo.h"
#include "op_treeheight.h"
#include "op_no_nodes.h"
#include "op_no_entries.h"
#include "op_getRootNode.h"
#include "op_keyrange.h"
#include "op_get_cache_size.h"
#include "op_set_cache_size.h"
#include "op_get_no_nodes_visited.h"
#include "op_get_no_cachehits.h"
#include "op_reset_counters.h"
#include "op_get_pinned_nodes.h"
#include "op_pin_nodes.h"
#include "op_unpin_nodes.h"
#include "op_insertbtree.h"
#include "op_updatebtree.h"

#include "op_deletebtree.h"

#include "op_set_maxkeysize.h"
#include "op_get_maxkeysize.h"
#include "op_set_maxvaluesize.h"
#include "op_get_maxvaluesize.h"

#include "op_get_statistics.h"
#include "op_set_debug.h"
#include "op_set_cache_limit_type.h"
#include "BTree2.h"

using namespace std;

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

namespace BTree2Algebra {

Algebra::Algebra() : ::Algebra() {
    AddTypeConstructor( &BTree2Algebra::BTree2::typeConstructor );

    AddOperator(&BTree2Algebra::Operators::createbtree2::def1);
    AddOperator(&BTree2Algebra::Operators::createbtree2::def2);
    AddOperator(&BTree2Algebra::Operators::insertbtree::insertbtree2);
    AddOperator(&BTree2Algebra::Operators::insertbtree::insertbtree1);
    AddOperator(&BTree2Algebra::Operators::deletebtree::deletebtree2);
    AddOperator(&BTree2Algebra::Operators::deletebtree::deletebtree1);
    AddOperator(&BTree2Algebra::Operators::updatebtree::updatebtree2);
    AddOperator(&BTree2Algebra::Operators::updatebtree::updatebtree1);

    AddOperator(&BTree2Algebra::Operators::rangebtree2::exactmatch2);
    AddOperator(&BTree2Algebra::Operators::rangebtree2::exactmatchS);
    AddOperator(&BTree2Algebra::Operators::rangebtree2::exactmatch);
    AddOperator(&BTree2Algebra::Operators::rangebtree2::range2);
    AddOperator(&BTree2Algebra::Operators::rangebtree2::rangeS);
    AddOperator(&BTree2Algebra::Operators::rangebtree2::range);
    AddOperator(&BTree2Algebra::Operators::rangebtree2::leftrange2);
    AddOperator(&BTree2Algebra::Operators::rangebtree2::leftrangeS);
    AddOperator(&BTree2Algebra::Operators::rangebtree2::leftrange);
    AddOperator(&BTree2Algebra::Operators::rangebtree2::rightrange2);
    AddOperator(&BTree2Algebra::Operators::rangebtree2::rightrangeS);
    AddOperator(&BTree2Algebra::Operators::rangebtree2::rightrange);

    AddOperator(&BTree2Algebra::Operators::keyrange::keyrange2);
    AddOperator(&BTree2Algebra::Operators::keyrange::keyrange1);
    AddOperator(&BTree2Algebra::Operators::getFileInfo::def);
    AddOperator(&BTree2Algebra::Operators::treeheight::def);
    AddOperator(&BTree2Algebra::Operators::no_nodes::def);
    AddOperator(&BTree2Algebra::Operators::no_entries::def);
    AddOperator(&BTree2Algebra::Operators::getRootNode::def);
    AddOperator(&BTree2Algebra::Operators::describebtree2::getnodeinfo);
    AddOperator(&BTree2Algebra::Operators::describebtree2::getnodesons);
    AddOperator(
        &BTree2Algebra::Operators::describebtree2::internalnodecapacity);
    AddOperator(&BTree2Algebra::Operators::describebtree2::leafnodecapacity);
    AddOperator(&BTree2Algebra::Operators::describebtree2::getminfilldegree);
    AddOperator(&BTree2Algebra::Operators::describebtree2::getnodesize);

    AddOperator(&BTree2Algebra::Operators::reset_counters::def);
    AddOperator(&BTree2Algebra::Operators::set_cache_size::def);
    AddOperator(&BTree2Algebra::Operators::get_cache_size::def);
    AddOperator(&BTree2Algebra::Operators::pin_nodes::def);
    AddOperator(&BTree2Algebra::Operators::unpin_nodes::def);
    AddOperator(&BTree2Algebra::Operators::get_pinned_nodes::def);
    AddOperator(&BTree2Algebra::Operators::get_no_nodes_visited::def);
    AddOperator(&BTree2Algebra::Operators::get_no_cachehits::def);
    AddOperator(&BTree2Algebra::Operators::set_cache_limit_type::def);

    AddOperator(&BTree2Algebra::Operators::set_maxkeysize::def);
    AddOperator(&BTree2Algebra::Operators::get_maxkeysize::def);
    AddOperator(&BTree2Algebra::Operators::set_maxvaluesize::def);
    AddOperator(&BTree2Algebra::Operators::get_maxvaluesize::def);
    AddOperator(&BTree2Algebra::Operators::get_statistics::def);
    AddOperator(&BTree2Algebra::Operators::set_debug::def);
    AddOperator(&BTree2Algebra::Operators::getentry2::def);
  };

} // end namespace BTree2Algebra

extern "C"
Algebra*
InitializeBTree2Algebra( NestedList* nlRef,
                        QueryProcessor* qpRef,
                        AlgebraManager* amRef )
{
  nl = nlRef;
  qp = qpRef;
  am = amRef;
  return (new BTree2Algebra::Algebra());
}

