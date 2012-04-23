//This file is part of SECONDO.

//Copyright (C) 2006, University in Hagen, Department of Computer Science, 
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

// Written 2012 by Jan Kristof Nidzwetzki 

package progresswatcher;

import java.io.File;

import javax.swing.JFileChooser;
import javax.swing.JOptionPane;

/**
 * Build a special JFile Chooser
 * If a existing file are selected
 * ask the user if this file should be overwritten
 */
class SaveJFileChooser extends JFileChooser {
	private static final long serialVersionUID = -6230014989327575691L;

	SaveJFileChooser(File arg0) {
		super(arg0);
	}

	public void approveSelection(){
	    final File f = getSelectedFile();
	    if(f.exists() && getDialogType() == SAVE_DIALOG){
	        int result = JOptionPane.showConfirmDialog(this,"The file exists, " +
	        		"overwrite?","Existing file",JOptionPane.YES_NO_CANCEL_OPTION);
	        switch(result){
	            case JOptionPane.YES_OPTION:
	                super.approveSelection();
	                return;
	            case JOptionPane.NO_OPTION:
	                return;
	            case JOptionPane.CANCEL_OPTION:
	                cancelSelection();
	                return;
	        }
	    }
	    super.approveSelection();
	}
}