/*
----
This file is part of SECONDO.

Copyright (C) 2019,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



0 Pointcloud2 ClusterPc2 operator

Assigns each point of a pointcloud to a cluster.

*/
#include "opClusterPc2.h"

#include "ListUtils.h"

#include "../tcPointcloud2.h"

using namespace pointcloud2;

extern NestedList *nl;

ListExpr op_cluster::clusterTM(ListExpr args) {
    return clusterOrRemoveNoiseTM(args, ClusterOp::ClusterPc2);
}

int op_cluster::clusterVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {
    return clusterOrRemoveNoiseVM(args, result, message, local, s,
            ClusterOp::ClusterPc2);
}

std::string op_cluster::getOperatorSpec(){
    return OperatorSpec(
            " pointcloud2 x T x real x int -> pointcloud x T x int",
            " _ clusterPc2[_, _] ",
            " Assign every point to a cluster. ",
            " query pc2 clusterPc2[eps, minPts]"
    ).getStr();
}

std::shared_ptr<Operator> op_cluster::getOperator(){
    return std::make_shared<Operator>("clusterPc2",
                                    getOperatorSpec(),
                                    &op_cluster::clusterVM,
                                    Operator::SimpleSelect,
                                    &op_cluster::clusterTM);
}


/*
2 Pointcloud2 removeNoise operator

*/
ListExpr op_removeNoise::removeNoiseTM(ListExpr args) {
    return op_cluster::clusterOrRemoveNoiseTM(args, ClusterOp::RemoveNoise);
}

int op_removeNoise::removeNoiseVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {
    return op_cluster::clusterOrRemoveNoiseVM(args, result, message, local, s,
            ClusterOp::RemoveNoise);
}

std::string op_removeNoise::getOperatorSpec(){
    return OperatorSpec(
            " pointcloud2 x T x real x int -> pointcloud x T x int",
            " removeNoise(_, _, _) ",
            " Remove points that are not part of a cluster. ",
            " query removeNoise(pc2, eps, minPts)"
    ).getStr();
}

std::shared_ptr<Operator> op_removeNoise::getOperator(){
    return std::make_shared<Operator>("removeNoise",
                                    getOperatorSpec(),
                                    &op_removeNoise::removeNoiseVM,
                                    Operator::SimpleSelect,
                                    &op_removeNoise::removeNoiseTM);
}

/*
3 Code common to the clusterPc2 and removeNoise operators

*/
ListExpr op_cluster::clusterOrRemoveNoiseTM(ListExpr args, ClusterOp op) {
    const std::string err("pointcloud2 expected");

    if (!nl->HasLength(args,3)) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }

    //((pointcloud2 (WGS84 (tuple ((Name string) (Value real))))) real int)
    //First: (pointcloud2 (WGS84 (tuple ((Name string) (Value real)))))
    const ListExpr pointCloudExpr = nl->First(args);
    const ListExpr pointCloudSymbol = nl->First(pointCloudExpr);
    //Second: real
    const ListExpr epsSymbol = nl->Second(args);
    //Third: int
    const ListExpr minPtsSymbol = nl->Third(args);

    if (!listutils::isSymbol(pointCloudSymbol,Pointcloud2::BasicType())) {
        return listutils::typeError(err + " for the first parameter");
    }
    if (!listutils::isSymbol(epsSymbol,CcReal::BasicType())) {
        return listutils::typeError("The second parameter must be a real "
                                    "to describe the eps.");
    }
    if (!listutils::isSymbol(minPtsSymbol,CcInt::BasicType())) {
        return listutils::typeError("The third parameter must be an int "
                                    "to describe the minPts.");
    }

    bool hasTupleInformation = false;
    ListExpr typeInfo2 = nl->Second(pointCloudExpr);
    if (nl->HasLength(typeInfo2,2)) {
        hasTupleInformation = true;
    } else if (nl->ListLength(typeInfo2) != -1) {
        return listutils::typeError("Too much information at pointcloud type");
    }

    int attrIndexCluster = -1;
    ListExpr clusterAttribute = nl->TwoElemList(
            nl->SymbolAtom(CLUSTER_ATTR_NAME),listutils::basicSymbol<CcInt>());
    
    ListExpr referencesystem;
    ListExpr tupleExpr;
    if (hasTupleInformation) {
        // Get all current attributes from the type information
        ListExpr attributes = nl->Second(nl->Second(typeInfo2));

        tupleExpr = nl->OneElemList(attributes);

        if (op == ClusterOp::ClusterPc2) {
            // Find the last attribute for the append method
            attrIndexCluster = 0;
            ListExpr last = nl->TheEmptyList();
            while(!nl->IsEmpty(attributes)) {
                last = attributes;
                attributes = nl->Rest(attributes);
                ++attrIndexCluster;
            }

            // Append the cluster column as additional information
            // to the pointcloud
            last = nl->Append(last, clusterAttribute);
        }
        referencesystem = nl->First(nl->Second(pointCloudExpr));
        tupleExpr = nl->First(tupleExpr);
    } else{
        if (op == ClusterOp::ClusterPc2) {
            tupleExpr = nl->OneElemList(clusterAttribute);
            attrIndexCluster = 0;
        } else {
            assert (op == ClusterOp::RemoveNoise);
            tupleExpr = nl->TheEmptyList(); // for DEBUG output
        }
        referencesystem = typeInfo2;
    }

    ListExpr result;
    if (hasTupleInformation || (op == ClusterOp::ClusterPc2)) {
        ListExpr tuple = nl->TwoElemList(
                    nl->SymbolAtom(Tuple::BasicType()), tupleExpr);

        result = nl->TwoElemList(
            listutils::basicSymbol<Pointcloud2>(),
            nl->TwoElemList(referencesystem, tuple)
        );
    } else {
        result = nl->TwoElemList(
            listutils::basicSymbol<Pointcloud2>(),
            nl->OneElemList(referencesystem)
        );
    }

    // use the append mechanism (see GuideAlgebra.pdf, p.48, section 11.1)
    // to inform Value Mapping about attrIndexCluster
    return nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()),
            nl->OneElemList(nl->IntAtom(attrIndexCluster)),
            result);
}

int op_cluster::clusterOrRemoveNoiseVM(Word* args, Word& result, int message,
        Word& local, Supplier s, ClusterOp op) {
    // DEBUG
    // const size_t max_memory = qp->GetMemorySize(s); // test machine: 512 MB
    // cout << "Memory Size for Operator: " << max_memory << " MB" << endl;

    result = qp->ResultStorage(s);

    // get args
    Pointcloud2* pc2 = static_cast<Pointcloud2*>(args[0].addr);
    CcReal *eps = static_cast<CcReal *>(args[1].addr);
    CcInt *minPts = static_cast<CcInt *>(args[2].addr);
    int attrIndexCluster = ((CcInt*)args[3].addr)->GetValue();
    Pointcloud2* destPc2 = static_cast<Pointcloud2*>(result.addr);

    // get the attribute indices to copy tuples. Assume that source indices
    // have not changed, . The cluster attribute has the last index and was
    // already received from type mapping (see above)
    size_t sourceAttrCount = pc2->hasRelation()
            ? pc2->getTupleType()->GetNoAttributes() : 0;
    size_t destAttrCount = destPc2->hasRelation()
            ? destPc2->getTupleType()->GetNoAttributes() : 0;
    assert (destAttrCount == sourceAttrCount +
            ((op == ClusterOp::ClusterPc2) ? 1 : 0));
    std::vector<int> destAttrIndices;
    for (size_t i = 0; i < sourceAttrCount; ++i)
        destAttrIndices.push_back(i);

    // call DbScan and write the result to destPc2
    size_t minClusterSizeToCopy = (op == ClusterOp::ClusterPc2) ? 0 : 1;
    pc2->cluster(eps->GetValue(), minPts->GetValue(), minClusterSizeToCopy,
            destPc2, attrIndexCluster, destAttrIndices);

    return 0;
}

