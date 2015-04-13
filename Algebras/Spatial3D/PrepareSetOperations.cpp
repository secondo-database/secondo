/*
 ----
 SECONDO is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with SECONDO; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ----

 01590 Fachpraktikum "Erweiterbare Datenbanksysteme" 
 WS 2014 / 2015

 <our names here>

 //paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
 //paragraph [10] Footnote: [{\footnote{] [}}]
 //[TOC] [\tableofcontents]

 [1] Implementation of a Spatial3D algebra

 [TOC]

 1 Includes and Defines

*/

#include <vector>
#include "geometric_algorithm.h"
#include "../../include/AlmostEqual.h"
#include "Spatial3D.h"
#include "AuxiliaryTypes.h"
#include "MultiObjectTriangleContainer.h"


bool spatial3d_geometric::prepareSetOperationSurface(
                        const TriangleContainer& in_1,
                        const TriangleContainer& in_2,
                        std::vector<Triangle>& out_only_1,
                        std::vector<Triangle>& out_only_2,
                        std::vector<Triangle>& out_both)
{
  MultiObjectTriangleContainer container;
  for (int c = 0; c < in_1.size(); ++c)
  {
    bool result = container.addTriangle(in_1.get(c), 1, false);
    if (!result)
      return false;
  }
  for (int c = 0; c < in_2.size(); ++c)
  {
    bool result = container.addTriangle(in_2.get(c), 2, true);
    if (!result)
      return false;
  }
  container.prepareSetOperationSurface(1, 2, 3, 4, 5);
  container.exportObject(3, out_both);
  container.exportObject(4, out_only_1);
  container.exportObject(5, out_only_2);
  return true;
}

bool spatial3d_geometric::prepareSetOperationVolume(
                     const Volume3d& in_1,
                     const Volume3d& in_2,
                     std::vector<Triangle>& out_only_1_outside_2,
                     std::vector<Triangle>& out_only_2_outside_1,
                     std::vector<Triangle>& out_only_1_inside_2,
                     std::vector<Triangle>& out_only_2_inside_1,
                     std::vector<Triangle>& out_both_same_direction,
                     std::vector<Triangle>& out_both_opposite_direction)
{
  MultiObjectTriangleContainer container;
  for (int c = 0; c < in_1.size(); ++c)
  {
    bool result = container.addTriangle(in_1.get(c), 1, false);
    if (!result)
      return false;
  }
  for (int c = 0; c < in_2.size(); ++c)
  {
    bool result = container.addTriangle(in_2.get(c), 2, true);
    if (!result)
      return false;
  }
  container.prepareSetOperationVolume(1, 2, 3, 4, 5, 6, 7, 8);
  container.exportObject(3, out_both_same_direction);
  container.exportObject(4, out_both_opposite_direction);
  container.exportObject(5, out_only_1_outside_2);
  container.exportObject(6, out_only_2_outside_1);
  container.exportObject(7, out_only_1_inside_2);
  container.exportObject(8, out_only_2_inside_1);
  return true;
}