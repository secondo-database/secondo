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

package viewer.queryconstruction2;

import java.awt.Color;
import java.awt.Dimension;
import javax.swing.JDialog;
import javax.swing.JTextPane;
import sj.lang.ListExpr;


/**
 * Dialog Window for informations about operations.
 */
public class OperationsInfoDialog extends JDialog {
    
    JTextPane opText = new JTextPane();

    private String opName, opNameH, opNameL	= "OperatorName:";
    private String opId, opIdH, opIdL		= "OperatorId:";
    private String algName, algNameH, algNameL	= "AlgebraName:";
    private String algId, algIdH, algIdL	= "AlgebraId:";
    private String resType, resTypeH, resTypeL	= "ResultType:";
    private String signat, signatH, signatL	= "Signature:";
    private String syntax, syntaxH, syntaxL	= "Syntax:";
    private String meaning, meaningH, meaningL	= "Meaning:";
    private String example, exampleH, exampleL	= "Example:";
    private String remark, remarkH, remarkL	= "Remark:";

    
    public OperationsInfoDialog(int x, int y, ListExpr opInfo) {
        opText.setEditable(false);
	opText.setBackground(new Color(250, 250, 170));
	opText.setContentType("text/html");
	this.add(opText);
        this.setTitle("Operator-Info");
        this.setAlwaysOnTop(true);
        this.setLocation(x, y);
        this.setSize(new Dimension(400,650));

	setStrings(opInfo);
	addInfo();

        setVisible(true);


    }
    

 /**
     * Set the information strings.
     * @param ListExpr opInfo
     */
    protected void setStrings(ListExpr opInfo) {
	opName	= opInfo.second().first().first().stringValue();
	opId	= opInfo.second().first().second().toString();
	algName = opInfo.second().first().third().stringValue();
	algId	= opInfo.second().first().fourth().toString();
	resType = opInfo.second().first().fifth().textValue();
	signat	= opInfo.second().first().sixth().textValue();
	syntax	= opInfo.second().first().seventh().textValue();
	meaning = opInfo.second().first().eighth().textValue();
	example = opInfo.second().first().nineth().textValue();
	remark	= opInfo.second().first().tenth().textValue();
    }


    /**
     * Add the sum of information string to the window.
     */
    protected void addInfo() {
	String sumStr = "<html>";
	if (true == true) {
	    opNameH = "<br><font color=black>" + opNameL +
		        "<br><font color=blue>" + opName + "</font>";
	    sumStr += opNameH;
	}
	if (true == true) {
	    opIdH = "<br><font color=black>" + opIdL +
		        "<br><font color=blue>" + opId + "</font>";
	    sumStr += opIdH;
	}
	if (true == true) {
	    algNameH = "<br><font color=black>" + algNameL +
		        "<br><font color=blue>" + algName + "</font>";
	    sumStr += algNameH;
	}
	if (true == true) {
	    algIdH = "<br><font color=black>" + algIdL +
		        "<br><font color=blue>" + algId + "</font>";
	    sumStr += algIdH;
	}
	if (true == true) {
	    resTypeH = "<br><font color=black>" + resTypeL +
		        "<br><font color=blue>" + resType + "</font>";
	    sumStr += resTypeH;
	}
	if (true == true) {
	    signatH = "<br><font color=black>" + signatL +
		        "<br><font color=blue>" + signat + "</font>";
	    sumStr += signatH;
	}
	if (true == true) {
	    syntaxH = "<br><font color=black>" + syntaxL +
		        "<br><font color=blue>" + syntax + "</font>";
	    sumStr += syntaxH;
	}
	if (true == true) {
	    exampleH = "<br><font color=black>" + exampleL +
		        "<br><font color=blue>" + example + "</font>";
	    sumStr += exampleH;
	}
	if (true == true) {
	    remarkH = "<br><font color=black>" + remarkL +
		        "<br><font color=blue>" + remark + "</font>";
	    sumStr += remarkH;
	}
	if (true == true) {
	    meaningH = "<br><br><font color=black><b>" + meaningL +
		        "<br><font color=blue>" + meaning + "</b></font>";
	    sumStr += meaningH;
	}
	    	
	sumStr += "</html>";
	opText.setText(sumStr);
    }
    





/// alt:

    /**
     * Set the .
     */
    protected void view() {
        if (opText.getText() == null) {
            opText.setText("No Information selected.");
        }
    }
    
}
