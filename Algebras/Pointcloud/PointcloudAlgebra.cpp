/*
----
This file is part of SECONDO.

Copyright (C) 2017, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#include "Stream.h"
#include "LogMsg.h"
#include "Tools/Flob/DbArray.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "Attribute.h"
#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "AlgebraManager.h"
#include "Operator.h"
#include "StandardTypes.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "PointCloud.h"
#include "ImportPointCloud.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "Algebras/Collection/CollectionAlgebra.h"

#include "OpImportLAS.h"

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;

namespace pointcloud {

/*
Algebra extracted from RoutePlanningAlgebra

Type Constructor functions for Cpoint

Property function

*/
    ListExpr CpointProperty() {
        return (nl->TwoElemList (
             nl->FourElemList (
             nl->StringAtom("Signature"),
             nl->StringAtom("Example Type List"),
             nl->StringAtom("List Rep"),
             nl->StringAtom("Example List")),
         nl->FourElemList (
             nl->StringAtom("-> DATA"),
             nl->StringAtom(Cpoint::BasicType()),
             nl->StringAtom("(real real real) = (x,y,z)"),
             nl->StringAtom("(1.0 2.0 3.0)")
        )));
    }

/*
IN function

*/
    Word InCpoint(const ListExpr typeinfo, const ListExpr instance, 
                    const int errorPos, ListExpr& errorInfo, bool& correct ) {
        Word res((void*)0);
        correct = false;
    
        // Check for undefined
        if(listutils::isSymbolUndefined(instance)) {
            correct = true;
            Cpoint* p = new Cpoint(0.0,0.0,0.0);
            p->SetDefined(false);
            res.addr = p;
            return res;
        }
    
        if (!nl->HasLength(instance, 3)) {
            return res;
        }
        if ((!listutils::isNumeric(nl->First(instance)))
            || (!listutils::isNumeric(nl->Second(instance)))
            || (!listutils::isNumeric(nl->Third(instance)))) {
            return res;
        }
        double x=listutils::getNumValue(nl->First(instance));
        double y=listutils::getNumValue(nl->Second(instance));
        double z=listutils::getNumValue(nl->Third(instance));
        correct = true;
        res.addr = new Cpoint (x,y,z);
        return res;
    }        

/*
OUT function

*/
    ListExpr OutCpoint(ListExpr typeInfo, Word value) { 
        Cpoint* k= (Cpoint*) value.addr;
    
        if(!k->IsDefined()) {
            return listutils::getUndefined();
        }
    
        return nl->ThreeElemList(
            nl->RealAtom(k->getX()),
            nl->RealAtom(k->getY()),
            nl->RealAtom(k->getZ()));
    }

/*
CreateCpoint

*/
    Word CreateCpoint(const ListExpr typeInfo) {
            Word w;
            w.addr = (new Cpoint(0.0, 0.0, 0.0));
            return w;
    }
                   
/*
DeleteCpoint

*/
    void DeleteCpoint(const ListExpr typeInfo, Word& w) {
            Cpoint *k = (Cpoint *) w.addr;
            delete k;
            w.addr = 0;
    }

/*
CloseCpoint

*/
    void CloseCpoint (const ListExpr typeInfo, Word& w ) {
        Cpoint *k = (Cpoint * ) w.addr;
        delete k;
        w.addr = 0;
    }
                   
/*
CloneCpoint

*/
    Word CloneCpoint( const ListExpr typeInfo, const Word& w) {
        Cpoint* k = (Cpoint*)w.addr;
        Word res(k->Clone());
        return res;
    }
                   
/*
CastCpoint

*/
    void* CastCpoint(void* addr) {
        return (new (addr) Cpoint);
    }
                 
/*
CpointTypeCheck

*/
    bool CpointTypeCheck(ListExpr type, ListExpr& errorInfo) {
        return nl->IsEqual(type, Cpoint::BasicType());
    }
                   
/*
SizeOfCpoint

*/
    int SizeOfCpoint() {
        return sizeof(Cpoint);
    }
    
/*
TypeConstructor Cpoint

*/
    TypeConstructor CpointTC (
         Cpoint::BasicType(),
         CpointProperty,
         OutCpoint, InCpoint,
         0, 0,
         CreateCpoint, DeleteCpoint,
         OpenAttribute<Cpoint>, 
         SaveAttribute<Cpoint>,
         CloseCpoint, CloneCpoint,
         CastCpoint,
         SizeOfCpoint,
         CpointTypeCheck);    
    
/*
Type Constructor functions for Cpoints an DBArray
containing Cpoint elements
  

Property function

*/
    ListExpr CpointsProperty() {
        return (nl->TwoElemList (
            nl->FourElemList (
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List")),
                nl->FourElemList (
                nl->StringAtom("-> DATA"),
                nl->StringAtom(Cpoints::BasicType()),
               nl->StringAtom("((real real real)(real real real))"),
                nl->StringAtom("( (1.0 2.0 1.5) (2.0 6.0 1.5) )")
        )));
    }

/*
IN function

*/
    Word InCpoints (const ListExpr typeinfo, const ListExpr instance, 
                    const int errorPos, ListExpr& errorInfo, bool& correct ) {
        Word res((void*)0);
        correct = false;
        Cpoints* cpnts = new Cpoints(0);
        cpnts->SetDefined(true);
        // check for undefined
        if(listutils::isSymbolUndefined(instance)) {
            correct = true;
            cpnts->SetDefined(false);
            res.addr = cpnts;
            return res;
        }
        // Work through the list
        ListExpr first = nl->Empty();
        ListExpr inst = instance;
        while( !nl->IsEmpty(inst)) {
            first = nl->First(inst);
            inst = nl->Rest(inst);
            if ((nl->HasLength(first, 3)) &&
                (nl->IsAtom(nl->First(first))) &&
                (listutils::isNumeric(nl->First(first))) &&
                (nl->IsAtom(nl->Second(first))) &&
                (listutils::isNumeric(nl->Second(first))) &&           
                (nl->IsAtom(nl->Third(first))) &&
                (listutils::isNumeric(nl->Third(first))) ) {
                double x = listutils::getNumValue(nl->First(first));
                double y = 
                    listutils::getNumValue(nl->Second(first));               
                double z = listutils::getNumValue(nl->Third(first));
                cpnts->AppendCpoint(Cpoint(x, y, z));
            } else {
                    cpnts->DestroyCpoints();
                    return res;
            }
        }        
        correct = true;
        res.addr = cpnts;
        return res;
    }        

/*
OUT function

*/
    ListExpr OutCpoints(ListExpr typeInfo, Word value) {
        Cpoints* k= (Cpoints*) value.addr;
        Cpoint cpelem;
        Cpoint* pcpelem;
        if(!k->IsDefined()) {
            return listutils::getUndefined();
        }
        if (k->GetNoCpoints() > 0 ) {
            cpelem = k->GetCpoint(0);
            pcpelem = &cpelem;
            ListExpr result = nl->OneElemList(
                nl->ThreeElemList(
                nl->RealAtom(pcpelem->getX()),
                nl->RealAtom(pcpelem->getY()),                
                nl->RealAtom(pcpelem->getZ())));
            ListExpr last = result;
             
            for (int i = 1; i < k->GetNoCpoints(); i++) {
               cpelem = k->GetCpoint(i);
               pcpelem = &cpelem;
               last = nl->Append( last,
               nl->ThreeElemList(
                   nl->RealAtom(pcpelem->getX()),
                   nl->RealAtom(pcpelem->getY()),                
                   nl->RealAtom(pcpelem->getZ())));
             }
             return result;
        } else {
             return (nl->Empty());
        }
    }
                   
/*
CreateCpoints

*/
    Word CreateCpoints(const ListExpr typeInfo) {
        Word w;
        w.addr =  (new Cpoints(0));
        return w;
    }    
                   
/*
DeleteCpoints

*/
    void DeleteCpoints(const ListExpr typeInfo, Word& w) {
        Cpoints *k = (Cpoints *) w.addr;
        k->DestroyCpoints();
        delete k;
        w.addr = 0;
    }

/*
CloseCpoints

*/
    void CloseCpoints (const ListExpr typeInfo, Word& w ) {
        Cpoints *k = (Cpoints * ) w.addr;
        delete k;
        w.addr = 0;
    }
                   
/*
CloneCpoints

*/
    Word CloneCpoints( const ListExpr typeInfo, const Word& w) {
        Cpoints* k = (Cpoints*)w.addr;
        Word res(k->Clone());
        return res;
    }
                   
    /*
    CastCpoints
    */
    void* CastCpoints(void* addr) {
        return (new (addr) Cpoints);
     }

    /*
    SizeOfCpoints
    */
    int SizeOfCpoints() {
        return sizeof(Cpoints);
    }
                   
    /*
    CpointsTypeCheck
    */
    bool CpointsTypeCheck(ListExpr type, ListExpr& errorInfo) {
        return nl->IsEqual(type, Cpoints::BasicType());
    }

    /*
    TypeConstructor Cpoints
    */
    TypeConstructor CpointsTC (
        Cpoints::BasicType(),
        CpointsProperty,
        OutCpoints, InCpoints,
        0, 0,
        CreateCpoints, DeleteCpoints,
        OpenAttribute<Cpoints>, 
        SaveAttribute<Cpoints>,
        CloseCpoints, CloneCpoints,
        CastCpoints,
        SizeOfCpoints,
        CpointsTypeCheck);
    
    /*
     Type Constructor functions for Cpointnode that is a
     node of an 2D Tree that is the internal data structure 
     of the Cpoints stored within a PointCloud object. It
     contains Cpoint coordinates and indexes of left son
     and right son.
    */
    
    /*
    Property function
    */
    ListExpr CpointnodeProperty() {
        return (nl->TwoElemList (
        nl->FourElemList (
             nl->StringAtom("Signature"),
             nl->StringAtom("Example Type List"),
             nl->StringAtom("List Rep"),
             nl->StringAtom("Example List")),
        nl->FiveElemList (
             nl->StringAtom("-> DATA"),
             nl->StringAtom(Cpointnode::BasicType()),
             nl->StringAtom("(real real real int int)"),
             nl->StringAtom("=(x,y,z,leftson,rightson)"),
             nl->StringAtom("(1.0 2.0 3.0 2 -1)"))
        ));
    }

    /*
    IN function
    */
    Word InCpointnode  (const ListExpr typeinfo, 
              const ListExpr instance, const int errorPos,
              ListExpr& errorInfo, bool& correct ) {
        Word res((void*)0);
        correct = false;
    
        // Check for undefined
        if(listutils::isSymbolUndefined(instance)) {
            correct = true;
            Cpointnode* p = new Cpointnode(0.0,0.0,0.0,-1,-1);
            p->SetDefined(false);
            res.addr = p;
            return res;
        }
    
        if (!nl->HasLength(instance, 5)) {
            return res;
        }
        if ((!listutils::isNumeric(nl->First(instance)))
            || (!listutils::isNumeric(nl->Second(instance)))
            || (!listutils::isNumeric(nl->Third(instance)))
            || (!listutils::isNumeric(nl->Fourth(instance)))
            || (!listutils::isNumeric(nl->Fifth(instance)))) {
            return res;
        }
        double x=listutils::getNumValue(nl->First(instance));
        double y=listutils::getNumValue(nl->Second(instance));
        double z=listutils::getNumValue(nl->Third(instance));
        int leftson=listutils::getNumValue(nl->Fourth(instance));
        int rightson=listutils::getNumValue(nl->Fifth(instance));
        correct = true;
        res.addr = new Cpointnode (x,y,z,leftson,rightson);
        return res;
    }        

    /*
    OUT function
    */
    ListExpr OutCpointnode(ListExpr typeInfo, Word value) { 
        Cpointnode* k= (Cpointnode*) value.addr;
    
        if(!k->IsDefined()) {
            return listutils::getUndefined();
        }
    
        return nl->FiveElemList(
            nl->RealAtom(k->getX()),
            nl->RealAtom(k->getY()),
            nl->RealAtom(k->getZ()),
            nl->IntAtom(k->getLeftSon()),
            nl->IntAtom(k->getRightSon()));
    }

    /*
    CreateCpointnode
    */
    Word CreateCpointnode(const ListExpr typeInfo) {
        Word w;
        w.addr = (new Cpointnode(0.0, 0.0, 0.0, -1, -1));
        return w;
    }
                   
    /*
    DeleteCpointnode
    */
    void DeleteCpointnode(const ListExpr typeInfo, Word& w) {
            Cpointnode *k = (Cpointnode *) w.addr;
            delete k;
            w.addr = 0;
    }
                   
    /*
    CloseCpointnode
    */
    void CloseCpointnode (const ListExpr typeInfo, Word& w ) {
        Cpointnode *k = (Cpointnode * ) w.addr;
        delete k;
        w.addr = 0;
    }
                   
    /*
    CloneCpointnode
    */
    Word CloneCpointnode( const ListExpr typeInfo, const Word& w) {
        Cpointnode* k = (Cpointnode*)w.addr;
        Word res(k->Clone());
        return res;
    }
                   
    /*
    CastCpointnode
    */
    void* CastCpointnode(void* addr) {
        return (new (addr) Cpointnode);
    }
                 
    /*
    CpointnodeTypeCheck
    */
    bool CpointnodeTypeCheck(ListExpr type, ListExpr& errorInfo) {
        return nl->IsEqual(type, Cpointnode::BasicType());
    }
                   
    /*
    SizeOfCpointnode
    */
    int SizeOfCpointnode() {
        return sizeof(Cpointnode);
    }

    /*
    TypeConstructor Cpointnode
    */
    TypeConstructor CpointnodeTC (
        Cpointnode::BasicType(),
        CpointnodeProperty,
        OutCpointnode, InCpointnode,
        0, 0,
        CreateCpointnode, DeleteCpointnode,
        OpenAttribute<Cpointnode>, 
        SaveAttribute<Cpointnode>,
        CloseCpointnode, CloneCpointnode,
        CastCpointnode,
        SizeOfCpointnode,
        CpointnodeTypeCheck); 
    
    /*
     Type Constructor functions for PointCloud
     PointCloud contains a 2D Tree with Cpoints and
     the x and y-coordinates of the area (bbox) in which the
     Cpoints are located.  
    */
    
    /*
    Property function
    */
    ListExpr PointCloudProperty() {
        return (nl->TwoElemList (
             nl->SixElemList (
             nl->StringAtom("Signature"),
             nl->StringAtom("Example Type List"),
             nl->StringAtom("List Rep"),
             nl->StringAtom("Example List"),
             nl->StringAtom(" "),
             nl->StringAtom(" ")),
         nl->SixElemList (
             nl->StringAtom("-> DATA"),
             nl->StringAtom(PointCloud::BasicType()),
             nl->StringAtom("((r r r r)(r r r r r))"),
             nl->StringAtom("=((minx,maxx,miny,maxy)"),
             nl->StringAtom("(x,y,z,lson,rson))"),
             nl->StringAtom("((0.0 5.0 3.0 11.5)(1.0 2.0 3.0 1 5))")
        )));
    }

    /*
    IN function
    */
    Word InPointCloud (const ListExpr typeinfo, 
                    const ListExpr instance, const int errorPos, 
                    ListExpr& errorInfo, bool& correct ) {
        Word res((void*)0);
        correct = false;
        PointCloud* cpnts = new PointCloud(0);
        cpnts->SetDefined(true);
        // check for undefined
        if(listutils::isSymbolUndefined(instance)) {
            correct = true;
            cpnts->SetDefined(false);
            res.addr = cpnts;
            return res;
        }
        // check for empty
        if(nl->IsEmpty(instance)) {
            correct = true;
            cpnts->SetDefined(false);
            res.addr = cpnts;
            return res;
        }
        // Work through the list
        // First read the bbox values
        ListExpr first = nl->First(instance);
        ListExpr inst = nl->Rest(instance);
        if(nl->IsEmpty(inst)) {
            correct = false;
            cpnts->SetDefined(false);
            res.addr = cpnts;
            return res;
        }        
        if ((nl->HasLength(first, 4)) &&
            (nl->IsAtom(nl->First(first))) &&
            (listutils::isNumeric(nl->First(first))) &&
            (nl->IsAtom(nl->Second(first))) &&
            (listutils::isNumeric(nl->Second(first))) &&           
            (nl->IsAtom(nl->Third(first))) &&
            (listutils::isNumeric(nl->Third(first))) && 
            (nl->IsAtom(nl->Fourth(first))) &&
            (listutils::isNumeric(nl->Fourth(first)))) {
            double minX = listutils::getNumValue(nl->First(first));
            double maxX = listutils::getNumValue(nl->Second(first));
            double minY = listutils::getNumValue(nl->Third(first));
            double maxY = listutils::getNumValue(nl->Fourth(first));
            cpnts->setMinX(minX);
            cpnts->setMaxX(maxX);
            cpnts->setMinY(minY);
            cpnts->setMaxY(maxY);
            // Secondly go through the Cpointnodes        
            while( !nl->IsEmpty(inst)) {
                first = nl->First(inst);
                inst = nl->Rest(inst);
                if ((nl->HasLength(first, 5)) &&
                    (nl->IsAtom(nl->First(first))) &&
                    (listutils::isNumeric(nl->First(first))) &&
                    (nl->IsAtom(nl->Second(first))) &&
                    (listutils::isNumeric(nl->Second(first))) &&           
                    (nl->IsAtom(nl->Third(first))) &&
                    (listutils::isNumeric(nl->Third(first))) &&        
                    (nl->IsAtom(nl->Fourth(first))) &&
                    (listutils::isNumeric(nl->Fourth(first))) &&        
                    (nl->IsAtom(nl->Fifth(first))) &&
                    (listutils::isNumeric(nl->Fifth(first))) ) {
                    double x = listutils::getNumValue(nl->First(first));
                    double y = 
                        listutils::getNumValue(nl->Second(first));     
                    double z = listutils::getNumValue(nl->Third(first));
                    /*int leftson = 
                        listutils::getNumValue(nl->Fourth(first));     
                    int rightson = 
                        listutils::getNumValue(nl->Fifth(first));
                    */
                    cpnts->insert(x,y,z);
                } else {
                    cpnts->DestroyPointCloud();
                    return res;
                }
            }
            correct = true;
            res.addr = cpnts;
        }
        return res;
    }        

    /*
    OUT function
    */
        ListExpr OutPointCloud(ListExpr typeInfo, Word value) {
        PointCloud* k= (PointCloud*) value.addr;
        Cpointnode cpelem;
        Cpointnode* pcpelem;
        if(!k->IsDefined()) {
            return listutils::getUndefined();
        }
        if (k->GetNoCpointnodes() > 0 ) {
            cpelem = k->GetCpointnode(0);
            pcpelem = &cpelem;
            ListExpr result = nl->OneElemList(
                nl->FourElemList(
                nl->RealAtom(k->getMinX()),
                nl->RealAtom(k->getMaxX()),
                nl->RealAtom(k->getMinY()),
                nl->RealAtom(k->getMaxY())));
            ListExpr last = result;
            for (int i = 0; i < k->GetNoCpointnodes(); i++) {
               cpelem = k->GetCpointnode(i);
               pcpelem = &cpelem;
               last = nl->Append( last,
               nl->FiveElemList(
                   nl->RealAtom(pcpelem->getX()),
                   nl->RealAtom(pcpelem->getY()),                
                   nl->RealAtom(pcpelem->getZ()),          
                   nl->IntAtom(pcpelem->getLeftSon()),          
                   nl->IntAtom(pcpelem->getRightSon())));
             }
             return result;
        } else {
             return (nl->Empty());
        }
    }
                   
    /*
    CreatePointCloud
    */
    Word CreatePointCloud(const ListExpr typeInfo) {
        Word w;
        w.addr =  (new PointCloud(0));
        return w;
    }    
                   
    /*
    DeletePointCloud
    */
    void DeletePointCloud(const ListExpr typeInfo, Word& w) {
        PointCloud *k = (PointCloud *) w.addr;
        k->DestroyPointCloud();
        delete k;
        w.addr = 0;
    }

    /*
    ClosePointCloud
    */
    void ClosePointCloud (const ListExpr typeInfo, Word& w ) {
        PointCloud *k = (PointCloud * ) w.addr;
        delete k;
        w.addr = 0;
    }
                   
    /*
    ClonePointCloud
    */
    Word ClonePointCloud( const ListExpr typeInfo, const Word& w) {
        PointCloud* k = (PointCloud*)w.addr;
        Word res(k->Clone());
        return res;
    }
                   
    /*
    CastPointCloud
    */
    void* CastPointCloud(void* addr) {
        return (new (addr) PointCloud);
     }

    /*
    SizeOfPointCloud
    */
    int SizeOfPointCloud() {
        return sizeof(PointCloud);
    }
                   
    /*
    PointCloudTypeCheck
    */
    bool PointCloudTypeCheck(ListExpr type, ListExpr& errorInfo) {
        return nl->IsEqual(type, PointCloud::BasicType());
    }
            
    /*
    TypeConstructor PointCloud
    */
    TypeConstructor PointCloudTC (
         PointCloud::BasicType(),
         PointCloudProperty,
         OutPointCloud, InPointCloud,
         0, 0,
         CreatePointCloud, DeletePointCloud,
         OpenAttribute<PointCloud>, 
         SaveAttribute<PointCloud>,
         ClosePointCloud, ClonePointCloud,
         CastPointCloud,
         SizeOfPointCloud,
         PointCloudTypeCheck);

    /*
    Type Mappings
    */
    ListExpr bbox2dTM(ListExpr args) {
      if (!nl->HasLength(args, 1)) {
        return listutils::typeError("wrong number of arguments");
      }
      if (!PointCloud::checkType(nl->First(args))) {
        return listutils::typeError("pointcloud expected");
      }
      return nl->SymbolAtom(Rectangle<2>::BasicType());
    }
    
    ListExpr bboxTM(ListExpr args) {
      if (!nl->HasLength(args, 1)) {
        return listutils::typeError("wrong number of arguments");
      }
      if (!PointCloud::checkType(nl->First(args))) {
        return listutils::typeError("pointcloud expected");
      }
      return nl->SymbolAtom(Rectangle<3>::BasicType());
    }
    
    ListExpr no_componentsTM(ListExpr args) {
      if (!nl->HasLength(args, 1)) {
        return listutils::typeError("wrong number of arguments");
      }
      if (!PointCloud::checkType(nl->First(args))) {
        return listutils::typeError("pointcloud expected");
      }
      return nl->SymbolAtom(CcInt::BasicType());
    }
    
    ListExpr importpointcloudTM(ListExpr args){
        if(!nl->HasLength(args,1)) {
            return listutils::typeError("wrong number of arguments");
        }
        if( (!CcString::checkType(nl->First(args))) 
            &&  (!FText::checkType(nl->First(args))) ) {
            return listutils::typeError("string or text expected");
        }
        return nl->TwoElemList(nl->SymbolAtom(Stream<PointCloud>::BasicType()),
            nl->SymbolAtom(PointCloud::BasicType()));
    }
    
    ListExpr extractpointsTM(ListExpr args){
        if(!nl->HasLength(args,1)) {
            return listutils::typeError("wrong number of arguments");
        }
        if (!PointCloud::checkType(nl->First(args))) {
        return listutils::typeError("pointcloud expected");
      }
      return nl->TwoElemList(nl->SymbolAtom(Stream<Tuple>::BasicType()),
                             nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
          nl->TwoElemList(nl->TwoElemList(nl->SymbolAtom("Pos"),
                                          nl->SymbolAtom(Point::BasicType())),
                          nl->TwoElemList(nl->SymbolAtom("Height"),
                                        nl->SymbolAtom(CcReal::BasicType())))));
      
      return nl->TwoElemList(nl->SymbolAtom(Stream<Tuple>::BasicType()),
        nl->TwoElemList(
          nl->TwoElemList(nl->SymbolAtom("Pos"),
                          nl->SymbolAtom(Point::BasicType())),
          nl->TwoElemList(nl->SymbolAtom("Height"),
                          nl->SymbolAtom(CcReal::BasicType()))));
    }
    
     /*
    Value Mapping
    */
    int bbox2dVM(Word* args, Word& result, int message, Word& local, 
                 Supplier s) {
      PointCloud* source = static_cast<PointCloud*>(args[0].addr);
      result = qp->ResultStorage(s);
      Rectangle<2> *res = static_cast<Rectangle<2>* >(result.addr);
      if (source->IsDefined()) {
        *res = source->BoundingBox();
      }
      return 0;
    }
     
    int bboxVM(Word* args, Word& result, int message, Word& local, Supplier s) {
      PointCloud* source = static_cast<PointCloud*>(args[0].addr);
      result = qp->ResultStorage(s);
      Rectangle<3> *res = static_cast<Rectangle<3>* >(result.addr);
      if (source->IsDefined()) {
        *res = source->BoundingBox3d();
      }
      return 0;
    }
    
    int no_componentsVM(Word* args, Word& result, int message, Word& local,
                        Supplier s) {
      PointCloud* source = static_cast<PointCloud*>(args[0].addr);
      result = qp->ResultStorage(s);
      CcInt *res = static_cast<CcInt* >(result.addr);
      if (source->IsDefined()) {
        res->Set(true, source->GetNoCpointnodes());
      }
      return 0;
    }
   
    int importpointcloudSelect(ListExpr args) {
        if ( CcString::checkType(nl->First(args)) ) {
            return 0;
        } 
        if (FText::checkType(nl->First(args)) ) {
            return 1;
        }
        return -1;
    }
    
    template<class T> 
    int importpointcloudVMT( Word* args, Word& result, 
                int message, Word& local, Supplier s ) {
        ImportPointCloud::importpointcloudLI<T>* li =
            (ImportPointCloud::importpointcloudLI<T>*) local.addr;
        switch(message) {
            case OPEN : 
                if(li) {
                    delete li;
                }
                local.addr = new ImportPointCloud::importpointcloudLI<T>(
                    (T*) args[0].addr);
                return 0;
            case REQUEST:  
                result.addr = li?li->getNext():0;
                if (result.addr != 0) {
//                     test output of some PointCloud data
//                     PointCloud* myTestRes = (PointCloud*) result.addr;
//                     if ((myTestRes->GetNoCpointnodes()) > 0) {
//                         cout << "Amount of elements: " 
//                                 << (myTestRes->GetNoCpointnodes()) << "   "
//                                 << " MinX: " << myTestRes->getMinX()
//                                 << " MaxX: " << myTestRes->getMaxX()
//                                 << " MinY: " << myTestRes->getMinY()    
//                                 << " MaxY: " << myTestRes->getMaxY() 
//                                 << endl;
//                     }
                    return YIELD;
                } else {
                    return CANCEL;
                }
            case CLOSE: 
                if(li){
                    delete li;
                    local.addr = 0;
                }
                return 0;
        }
        return 0;
    }
    
    ValueMapping importpointcloudVM[] = {
        importpointcloudVMT<CcString>,
        importpointcloudVMT<FText>
    };
    
    class ExtractpointsLI {
     public:
      ExtractpointsLI(PointCloud *src) : pc(src), counter(0) {
        cps = pc->getAllPointsInRange(pc->getMinX(), pc->getMinY(),
                                      pc->getMaxX(), pc->getMaxY());
        tt = getTupleType();
      }

      ~ExtractpointsLI(){
        delete cps;
        tt->DeleteIfAllowed();
      }
      
      Tuple* getNextTuple() {
        if (counter >= cps->GetNoCpoints()) {
          return 0;
        }
        Cpoint cp = cps->GetCpoint(counter);
        counter++;
        Point *pt = new Point(true, cp.getX(), cp.getY());
        CcReal *height = new CcReal(true, cp.getZ());
        Tuple *result = new Tuple(tt);
        result->PutAttribute(0, pt);
        result->PutAttribute(1, height);
        return result;
      }
      
      TupleType* getTupleType() {
        ListExpr ttList = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
          nl->TwoElemList(nl->TwoElemList(nl->SymbolAtom("Pos"),
                                          nl->SymbolAtom(Point::BasicType())),
                          nl->TwoElemList(nl->SymbolAtom("Height"),
                                         nl->SymbolAtom(CcReal::BasicType()))));
        SecondoCatalog *sc = SecondoSystem::GetCatalog();
        ListExpr numttList = sc->NumericType(ttList);
        return new TupleType(numttList);
      }
      
      PointCloud *pc;
      int counter;
      Cpoints *cps;
      TupleType *tt;
    };
    
    int extractpointsVM(Word* args, Word& result, int message, Word& local,
                        Supplier s) {
      ExtractpointsLI* li = (ExtractpointsLI*)local.addr;
      switch (message) {
        case OPEN: {
          if (li) {
            delete li;
          }
          PointCloud *source = static_cast<PointCloud*>(args[0].addr);
          li = new ExtractpointsLI(source);
          local.setAddr(li);
          return 0;
        }
        case REQUEST: {
          result.addr = li ? li->getNextTuple() : 0;
          return result.addr ? YIELD : CANCEL;
        }
        case CLOSE: {
          if (li) {
            delete li;
            local.addr = 0;
          }
          return 0;
        }
      }
      return 0;
    }

    /*
    Operator Specs
    
    */
    OperatorSpec bbox2dSpec("pointcloud -> rect2",
        "bbox2d( _ )",
        "Returns the 2-dim bounding box corresponding to a pointcloud object",
        "query bbox2d(pc)");
    
    OperatorSpec bboxSpec("pointcloud -> rect3",
        "bbox( _ )",
        "Returns the 3-dim bounding box corresponding to a pointcloud object",
        "query bbox(pc)");
    
    OperatorSpec no_componentsSpec("pointcloud -> int",
        "no_components( _ )",
        "Returns the number of points of a pointcloud object",
        "query no_components(pc)");
    
    OperatorSpec importpointcloudSpec("{string, text}-> pointcloud",
        "importpointcloud( _ )",
        "Returns one or more pointclouds",
        "query importpointcloud(fileName) feed count");
    
    OperatorSpec extractpointsSpec("pointcloud -> stream(tuple(point, real))",
        "extractpoints( _ )",
        "Returns a stream of all elements of a pointcloud, 2d x 1d",
        "query extractpoints(pc) feed count");
    
    
    /*
    Operator bbox2d
    
    */
    Operator bbox2dOp(
      "bbox2d",
      bbox2dSpec.getStr(),
      bbox2dVM,
      Operator::SimpleSelect,
      bbox2dTM
    );
    
    /*
    Operator bbox
    
    */
    Operator bboxOp(
      "bbox",
      bboxSpec.getStr(),
      bboxVM,
      Operator::SimpleSelect,
      bboxTM
    );

    /*
    Operator no\_components
    
    */
    Operator no_componentsOp(
      "no_components",
      no_componentsSpec.getStr(),
      no_componentsVM,
      Operator::SimpleSelect,
      no_componentsTM
    );
    
    /*
    Operator importpointcloud
    
    */
    Operator importpointcloudOp(
        "importpointcloud",
        importpointcloudSpec.getStr(),
        2,
        importpointcloudVM,
        importpointcloudSelect,
        importpointcloudTM); 
    
    /*
    Operator extractpoints
    
    */
    Operator extractpointsOp(
        "extractpoints",
        extractpointsSpec.getStr(),
        extractpointsVM,
        Operator::SimpleSelect,
        extractpointsTM); 


    /*
    class RoutePlanningAlgebra
    
    */    
    class PointcloudAlgebra : public Algebra {
        public:
        PointcloudAlgebra() : Algebra() {

            AddTypeConstructor( &CpointTC );
            AddTypeConstructor( &CpointsTC );
            AddTypeConstructor( &CpointnodeTC );
            AddTypeConstructor( &PointCloudTC );
            CpointTC.AssociateKind( Kind::DATA() );
            CpointsTC.AssociateKind( Kind::DATA() );
            CpointnodeTC.AssociateKind( Kind::DATA() );
            PointCloudTC.AssociateKind( Kind::DATA() );
            PointCloudTC.AssociateKind( Kind::SPATIAL2D());
            AddOperator(&bbox2dOp, false);
            AddOperator(&bboxOp, false);
            AddOperator(&no_componentsOp, false);
            AddOperator(&importpointcloudOp, false);
            AddOperator(&extractpointsOp, false);
            AddOperator(getImportLASOp());

        }
        ~PointcloudAlgebra() {};
    };
}

extern "C"
Algebra *
InitializePointcloudAlgebra(NestedList *nlRef, QueryProcessor *qpRef) {
    return new pointcloud::PointcloudAlgebra;
}



