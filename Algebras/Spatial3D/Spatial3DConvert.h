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

//the above header was modified and copied from SuffixTree.h


#ifndef _SPATIAL3DCONVERT_H
#define _SPATIAL3DCONVERT_H


namespace spatial3DConvert 
{ 
  Operator* getRegion2SurfacePtr();
  Operator* getRegion2VolumePtr();
  Operator* getMRegion2VolumePtr();
} 

#endif 

