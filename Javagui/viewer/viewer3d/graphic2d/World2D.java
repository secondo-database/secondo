//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package viewer.viewer3d.graphic2d;

import gui.idmanager.*;

/***************************
* Autor : Thomas Behr
* Version 1.1
* Datum   16.5.2000
****************************/


import java.awt.Color;
import java.awt.image.*;
import java.awt.Graphics;

public class World2D {

/** insert a figure to the world */
public void insertFigure(Figure2D F) {
  W2d.Include(F);
}

/** removes all figures from the world */
public  void deleteAll() {
    W2d.empty();
}

/** removes all figures with given ID */
public void deleteFiguresWithID( ID PID ) {
    W2d.deleteFiguresWithID(PID);
}

/** paint the world on img */
public void paintWorld(BufferedImage img,
                        boolean border,       
                        boolean filled,
                        boolean gradient )     

                        {

  Graphics G = img.getGraphics();
  G.clearRect(0,0,img.getWidth(),img.getHeight());
  for (int i=W2d.getSize()-1; i>=0; i--) {
    if (W2d.getFigure2DAt(i).isTriangle()) {
        Triangle2D[] Trs = W2d.getFigure2DAt(i).getTriangles();
        for(int j=0; j<Trs.length; j++)
        if (Trs[j]!=null)
            Trs[j].paint(img,border,filled,gradient);
    } // if triangles
    else
      if (W2d.getFigure2DAt(i).isLine()) {
        Line2D Line = W2d.getFigure2DAt(i).getLine();
        Line.paint(img,gradient);
      }
      else {   // a Point
          IDPoint2D P = W2d.getFigure2DAt(i).getPoint();   
          P.paint(img);
      }

  } // for all figures in W2d


} // paint World

/** the content of the world */
private Figure2DVector W2d = new Figure2DVector();

} 
