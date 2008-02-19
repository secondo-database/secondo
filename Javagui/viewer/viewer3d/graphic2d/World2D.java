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
public void insert(Figure2D F) {
  W2d.append(F);
}

public void sort(){
   W2d.sort();
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
public void paintWorld(Graphics g,
                       int width,
                       int height,
                       boolean filled,
                       boolean gradient ){

  g.clearRect(0,0,width,height);
  for (int i=W2d.getSize()-1; i>=0; i--) {
     Figure2D f = W2d.getFigure2DAt(i);
     f.paint(g,filled,gradient);  
  }
} // paint World

/** the content of the world */
private Figure2DVector W2d = new Figure2DVector();

} 
