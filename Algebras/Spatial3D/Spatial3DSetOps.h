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

#ifndef _SPATIAL3DSETOPS_H
#define _SPATIAL3DSETOPS_H

#include "Spatial3D.h"
/*
1 Declarations

1.1 forward declaration of Operator

Will be included from "Operator.h" in .cpp

*/
class Operator;

namespace spatial3DSetOps {
/*
1.1 enum set\_operation

Used to select the desired set operation in performSetOperation.

*/

  enum set_operation {
    set_union = 1,
    set_intersection,
    set_minus
  };
  
/*
1.1 performSetOperation for surface3d

performs the set operation as specified by set\_operation
on first and second surface3d objects.
res must be initialized, but bulk load must not be enabled.
Contents of res will be deleted.

*/ 
  void performSetOperation (const set_operation operation,
                            const Surface3d& first,
                            const Surface3d& second,
                            Surface3d& res);
/*
1.1 performSetOperation for volume3d

performs the set operation as specified by set\_operation
on first and second volume3d objects.
res must be initialized, but bulk load must not be enabled.
Contents of res will be deleted.

*/  
  void performSetOperation (const set_operation operation,
                            const Volume3d& first,
                            const Volume3d& second,
                            Volume3d& res);
/*
1.1 Operator pointers

Provide Pointers to the Operations as required to include in Algebra.

*/
  Operator* getUnionPtr();
  Operator* getIntersectionPtr();
  Operator* getMinusPtr();
} //namespace spatial3DSTLfileops 

#endif