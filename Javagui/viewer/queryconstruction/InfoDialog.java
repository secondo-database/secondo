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

package viewer.queryconstruction;

import java.awt.Dimension;
import java.awt.GridLayout;
import javax.swing.BorderFactory;
import javax.swing.JDialog;
import javax.swing.JTextArea;

/**
 * Dialog Window for informations about streams and the active query.
 */
public class InfoDialog extends JDialog {
    
    JTextArea textArea = new JTextArea();
    
    public InfoDialog(int x, int y) {
        textArea.setEditable(false);
        textArea.setBorder(BorderFactory.createEmptyBorder(5,5,5,15));
        this.add(textArea);
        this.setAlwaysOnTop(true);
        this.setLocation(x, y);
        this.setMinimumSize(new Dimension(100,100));
        setLayout(new GridLayout(0, 1));
    }
    
    /**
     * Add an information string to the window.
     * @param name line one
     * @param str line two
     */
    protected void addInfo(String name, String str) {
        str = name + "\n" + str.replace("(", "\n  ").replace(")", "");
        textArea.append("\n" + str + "\n");
    }
    
    /**
     * Set the frame visible.
     */
    protected void view() {
        if (textArea.getText() == null) {
            textArea.setText("No informationen available.");
        }
        pack();
        setVisible(true);
    }
    
    /**
     * Set the text in the window.
     * @param name title of the frame
     * @param str 
     */
    protected void viewInfo(String name, String str) {
        
        this.setTitle(name);
        str = str.replace("(", "  ").replace(")", "");
        textArea.setText(str);
        
    }
}
