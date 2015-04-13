/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[NP] [\newpage]
//[ue] [\"u]
//[e] [\'e]
//[lt] [\verb+<+]
//[gt] [\verb+>+]

----
SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----
 
[1] 01590 Fachpraktikum "Erweiterbare Datenbanksysteme" 

[1] WS 2014 / 2015

Jens Breit, Joachim Dechow, Daniel Fuchs, Simon Jacobi, G[ue]nther Milosits, 
Daijun Nagamine, Hans-Joachim Klauke.

Betreuer: Dr. Thomas Behr, Fabio Vald[e]s


[1] Implementation of a Spatial3D algebra: SetOps

[TOC]

[NP]

1 Includes and Defines

*/

#include "Spatial3DSetOps.h"
#include <iostream>
#include "ListUtils.h"
#include "Operator.h"
#include "NestedList.h"
#include "TypeMapUtils.h"
#include "geometric_algorithm.h"

extern NestedList* nl;

/*
1 Operator Specifcations

We have 3 operators:
union, intersection and minus.

All accept 2 sgeo3d objects of the same type.

*/

using namespace std;

namespace spatial3DSetOps {
  OperatorSpec unionSpec (
    "sgeo3d x sgeo3d -> sgeo3d, sgeo3d in {surface3d, volume3d}",
    "<sgeo3d_1> union <sgeo3d_2>",
    "calculates the union of 2 sgeo3d objects of the same type",
    "query mycube union mysphere;"
  );
  
  OperatorSpec intersectionSpec (
    "sgeo3d x sgeo3d -> sgeo3d, sgeo3d in {surface3d, volume3d}",
    "intersection (<sgeo3d_1> , <sgeo3d_2>)",
    "calculates the intersection of 2 sgeo3d objects of the same type",
    "query intersection (mycube , mysphere);"
  );
    
  OperatorSpec minusSpec (
    "sgeo3d x sgeo3d -> sgeo3d, sgeo3d in {surface3d, volume3d}",
    "<sgeo3d_1> minus <sgeo3d_2>",
    "calculates the difference of 2 sgeo3d objects of the same type",
    "query mycube minus mysphere;"
  );
  
/*
1 Type Mapping

All 3 operators use the same type mapping:
either two Surface3d or two Volume3d objects are acceptable.

*/
  ListExpr setOpsTM(ListExpr args) {
    if(!nl->HasLength(args,2)){
      return listutils::typeError("two arguments expected");
    }
    
    if(!nl->Equal(nl->First(args), nl->Second(args))) {
      return listutils::typeError("two arguments of same type expected");
    }
    if(Surface3d::checkType(nl->First(args))
         || Volume3d::checkType(nl->First(args))) {
      return nl->First(args);
    }
    return listutils::typeError("two arguments of sgeo3d type expected, "
                                "but got "
                                 + nl->ToString(nl->First(args)));
  }
  
/*
1 Type Selection

All operators use the same type selection:
The resulting type is identical to the type of the provided arguments.
The framework ensures we have a valid type combination.
So just checking the first type to determine the return type is sufficient.

*/
  int setOpsSelect( ListExpr args ) {
    if ( Surface3d::checkType(nl->First(args)) ) {
      return 0;
    }
    if ( Volume3d::checkType(nl->First(args)) ) {
      return 1;
    }
    return -1;  
  }
  
/*
1 Helper functions

1.1 addTriVector

Helper function to add a std::vector[lt]Triangle[gt] to a TriangleContainer.
The TriangleContainer must be properly enabled for bulk loading.
i.e. StartBulkLoad must have been called on the provided triCon.

*/
  void addTriVector (TriangleContainer& triCon,
                     const std::vector<Triangle>& triVec) {
    for (std::vector<Triangle>::const_iterator it = triVec.begin();
         it != triVec.end();
         ++it) {
      triCon.add(*it);
    }
  } 
  
  
  ostream& operator<< (ostream& os, const std::vector<Triangle>& triVec) {
    for (std::vector<Triangle>::const_iterator it = triVec.begin();
         it != triVec.end();
         ++it) {
      os << *it << endl;
    }
    return os;
  }
  
  ostream& operator<< (ostream& os, const TriangleContainer& triCon) {
    int size = triCon.size();
    for (int i=0; i< size; i++) {
      os << triCon.get(i) << endl;
    }
    return os;
  }
  
  
/*
1.1 addInvertedTriVector

Helper function to add a std::vector[lt]Triangle[gt] to a TriangleContainer.
The TriangleContainer must be properly enabled for bulk boading
i.e. StartBulkLoad must have been called on the provided triCon.
All triangles will be inverted (i.e. the outside will become the inside).

*/
  void addInvertedTriVector (TriangleContainer& triCon,
                             const std::vector<Triangle>& triVec) {
    for (std::vector<Triangle>::const_iterator it = triVec.begin();
         it != triVec.end();
         ++it) {
      triCon.add(Triangle(it->getA(),it->getC(),it->getB()));
    }
  }
  
/*
1.1 performSetOperation for surface3d objects
 
Helper to perform the actual set operation.
Choose the desired operation via set\_operation enum.
Takes two Surface3d objects as input.
For the result an initialized Surface3d object must be provided.

*/
  
  void performSetOperation (const set_operation operation,
                            const Surface3d& first,
                            const Surface3d& second,
                            Surface3d& res) {
    
    std::vector<Triangle> only_1;
    std::vector<Triangle> only_2;
    std::vector<Triangle> both;
    
    bool result = spatial3d_geometric::prepareSetOperationSurface(first,
                                                                  second,
                                                                  only_1,
                                                                  only_2,
                                                                  both);
   
    if (!result) {
      res.clear();
      res.SetDefined(false);
      return;
    }
    
    if (false) {      
      cout << "first\n" << first << endl;
      cout << "second\n" << second << endl;

      cout << "only_1\n" << only_1 << endl;
      cout << "only_2\n" << only_2 << endl;
      cout << "both\n" << both << endl;
    }
    
    res.clear();
    res.startBulkLoad();
    
    switch (operation) {
      case (set_union):
        addTriVector(res,only_1);
        addTriVector(res,only_2);
        addTriVector(res,both);
        break;
      case (set_intersection):
        addTriVector(res,both);
        break;
      case (set_minus):
        addTriVector(res,only_1);
        break;
      default:
        cerr << "unknown operation" << endl;
    } 
    
    res.endBulkLoad(NO_REPAIR); // we know what we are doing ;-)
    res.SetDefined(true); // for valid objects, setops are always defined
  }

/*
1.1 performSetOperation for volume3d objects
 
Helper to perform the actual operations for Volume3d objects.
Choose the desired operation via set\_operation enum.
Takes two Volume3d objects as input.
For the result an initialized Volume3d object must be provided.

*/
  void performSetOperation (const set_operation operation,
                            const Volume3d& first,
                            const Volume3d& second,
                            Volume3d& res) {
    
    std::vector<Triangle> only_1_outside_2;
    std::vector<Triangle> only_2_outside_1;
    std::vector<Triangle> only_1_inside_2;
    std::vector<Triangle> only_2_inside_1;
    std::vector<Triangle> both_same_direction;
    std::vector<Triangle> both_opposite_direction;
    
    bool result = spatial3d_geometric::prepareSetOperationVolume(first,
                                                                 second,
                                                   only_1_outside_2,
                                                   only_2_outside_1,
                                                   only_1_inside_2,
                                                   only_2_inside_1,
                                                   both_same_direction,
                                                   both_opposite_direction);
   

    if (!result) {
      res.clear();
      res.SetDefined(false);
      return;
    }
    
    if (false) {
      cout << "first\n" << first << endl;
      cout << "second\n" << second << endl;
      cout << "only_1_outside_2\n" << only_1_outside_2 << endl;
      cout << "only_2_outside_1\n" << only_2_outside_1 << endl;
      cout << "only_1_inside_2\n" << only_1_inside_2 << endl;
      cout << "only_2_inside_1\n" << only_2_inside_1 << endl;
      cout << "both_same_direction\n" << both_same_direction << endl;
      cout << "both_opposite_direction\n" << both_opposite_direction << endl;
    }
    
    res.clear();
    res.startBulkLoad();
    
    switch (operation) {
      case (set_union):
        addTriVector(res,only_1_outside_2);
        addTriVector(res,only_2_outside_1);
        addTriVector(res,both_same_direction);
        break;
      case (set_intersection):
        addTriVector(res,only_1_inside_2);
        addTriVector(res,only_2_inside_1);
        addTriVector(res,both_same_direction);
        break;
      case (set_minus):
        addTriVector(res,only_1_outside_2);
        addInvertedTriVector(res,only_2_inside_1);
        addTriVector(res,both_opposite_direction);
        break;
      default:
        cerr << "unknown operation" << endl;
    } 
        
    res.endBulkLoad(NO_REPAIR); // we know what we are doing ;-)
    res.SetDefined(true); // for valid objects, setops are always defined
  }
  
/*
1 Value Mappings

1.1 Template Method for ValueMappings

Provide required sgeo3d type and operation as template parameters.
Avoids duplication of code to create instances of the required objects.

*/
  
  template<typename T, set_operation operation>
  int singleSetOpsVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ) {

    T* first = (T*) args[0].addr;
    T* second = (T*) args[1].addr;
    
    result = qp->ResultStorage(s);
    T* res = (T*) result.addr;
    
    if (first->IsDefined() && second->IsDefined()) {
      performSetOperation (operation, *first, *second, *res);
    } else {
      // at least one object is undefined, so the result is too.
      res->SetDefined(false);
    }   
    return 0;
  }
  
/*
1.1 The value mappings

No surprises here, they are just using the previously defined template methods

*/  
  ValueMapping unionVM[] = {
    singleSetOpsVM<Surface3d, set_union>,
    singleSetOpsVM<Volume3d, set_union>
  };
  
  ValueMapping intersectionVM[] = {
    singleSetOpsVM<Surface3d, set_intersection>,
    singleSetOpsVM<Volume3d, set_intersection>
  };
  
  ValueMapping minusVM[] = {
    singleSetOpsVM<Surface3d, set_minus>,
    singleSetOpsVM<Volume3d, set_minus>
  };
/*
1 Operator Pointers

Provide Operator Pointers for embedding in Algebra 

*/    
  Operator* getUnionPtr(){
    return new Operator(
      "union",
      unionSpec.getStr(),
      2,
      unionVM,
      setOpsSelect,
      setOpsTM
    );
  }
  
  Operator* getIntersectionPtr(){
    return new Operator(
      "intersection",
      intersectionSpec.getStr(),
      2,
      intersectionVM,
      setOpsSelect,
      setOpsTM
    );
  }
  
  Operator* getMinusPtr(){
    return new Operator(
      "minus",
      minusSpec.getStr(),
      2,
      minusVM,
      setOpsSelect,
      setOpsTM
    );
  }  
} //namespace spatial3DSetOps
