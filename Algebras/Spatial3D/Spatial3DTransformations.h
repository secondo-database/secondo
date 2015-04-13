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


#ifndef _SPATIAL3DTRANSFORMATIONS_H
#define _SPATIAL3DTRANSFORMATIONS_H

#include "Spatial3D.h"

namespace spatial3DTransformations 
{
  void rotatePoint(
    Point3d* point,const Point3d* center, 
    const Vector3d* axis, double angle,Point3d& res
  );
  
  void rotateTriangleContainer(
    TriangleContainer* container,const Point3d* center, 
    const Vector3d* axis, double angle,TriangleContainer& res
  );
  
  void mirrorPoint(
    Point3d* point, const Plane3d* plane,Point3d& res
  );
  
  void mirrorTriangleContainer(
    TriangleContainer* container, const Plane3d* plane,TriangleContainer& res
  );
  
  void translatePoint(
    Point3d* point, const Vector3d* translation,Point3d& res
  );
  
  void translateTriangleContainer(
    TriangleContainer* container, const Vector3d* translation,
    TriangleContainer& res
  );
  
  void scaleDirPoint(
    Point3d* point, const Point3d* center, 
    const Vector3d* direction,Point3d& res
  );
  
  void scaleDirTriangleContainer(
    TriangleContainer* container, const Point3d* center, 
    const Vector3d* direction,TriangleContainer& res
  );
  
  void scalePoint(
    Point3d* point, const Point3d* center,  
    double factor,Point3d& res
  );
    
  void scaleTriangleContainer(
    TriangleContainer* container, const Point3d* center, 
    double factor,TriangleContainer& res
  );

  Operator* getRotatePtr();
  Operator* getMirrorPtr();
  Operator* getTranslatePtr();
  Operator* getScaleDirPtr();
  Operator* getScalePtr();
} 

#endif 
