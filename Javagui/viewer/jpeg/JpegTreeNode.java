/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

 * JpegTreeNode.java
 *
 * Created on 13. Februar 2004, 17:52
 */

package viewer.jpeg;

/**
 *
 * @author  sopra
 */



import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.tree.*;


public class JpegTreeNode extends javax.swing.tree.DefaultMutableTreeNode {
    
    /** Creates a new instance of JpegTreeNode with content type "text" */
    public JpegTreeNode(String text) {
        this.setUserObject(text);
    }

    /** Creates a new instance of JpegTreeNode with content type "image" */
    public JpegTreeNode(JPGViewerMetaData jpg) {
        this.setUserObject(jpg.getImgIcon().getImage().getScaledInstance(48, 48, jpg.getImgIcon().getImage().SCALE_DEFAULT));
    }
    
}
