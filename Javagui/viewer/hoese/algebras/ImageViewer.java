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


package  viewer.hoese.algebras;

import  javax.swing.*;
import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;


/**
 * A example to implement a separate JFrame as viewer for images. For test reasons only it is placed in algebras.
 */
public class ImageViewer
    implements SecondoFrame {

  public void show (boolean b) {
    f.setVisible(b);
  }

  public void addObject (Object o) {}
  public void removeObject (Object o) {}
  public void select (Object o) {
    if (o != null)
      f.setContentPane(((Dsplimage)o).lab);
    f.pack();
  }
  private static JFrame f = new JFrame();
}



