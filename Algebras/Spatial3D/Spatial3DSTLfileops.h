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


[1] Implementation of a Spatial3D algebra: FileOps

[TOC]

[NP]

1 Includes and Defines

*/
#ifndef _SPATIAL3DSTLFILEOPS_H
#define _SPATIAL3DSTLFILEOPS_H

/*
1 Declarations

1.1 forward declaration of Operator

Will be included from "Operator.h" in .cpp

*/
class Operator;
/*
1.1 Operator pointers

Provide Pointers to the Operations as required to include in Algebra.

*/
namespace spatial3DSTLfileops {
  Operator* getImportSTLptr();
  Operator* getExportSTLptr();
} //namespace spatial3DSTLfileops 

#endif