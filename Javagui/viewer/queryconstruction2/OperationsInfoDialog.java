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
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import javax.swing.JDialog;
import javax.swing.JTextPane;
import javax.swing.JButton;
import sj.lang.ListExpr;


/**
 * Dialog Window for informations about operations.
 * 
 * @author Thomas Alber
 *
 */
public class OperationsInfoDialog extends JDialog {
    
    JTextPane opText = new JTextPane();

    private JButton close = new JButton("close");

    private String opName, opNameH, opNameL     = "OperatorName:";
    private String algName, algNameH, algNameL  = "AlgebraName:";
    private String signat, signatH, signatL     = "Signature:";
    private String syntax, syntaxH, syntaxL     = "Syntax:";
    private String meaning, meaningH, meaningL  = "Meaning:";
    private String example, exampleH, exampleL  = "Example:";
    private String result, resultH, resultL     = "Result:";
    private String remark, remarkH, remarkL     = "Remark:";


    
    public OperationsInfoDialog(int x, int y, ListExpr opInfo) {
        this.setLayout(new BorderLayout());
        this.addWindowListener( new WindowAdapter() {
            public void windowClosing ( WindowEvent e) {
            }
        } );

        opText.setEditable(false);
        opText.setBackground(new Color(250, 250, 170));
        opText.setContentType("text/html");
        this.add(opText, BorderLayout.CENTER);
        this.add(close, BorderLayout.SOUTH);
        this.setTitle("Operator-Info");
        this.setAlwaysOnTop(true);
        this.setLocation(x, y);
        this.setSize(new Dimension(400,650));

        setStrings(opInfo);
        addInfo();
        setVisible(true);


        ActionListener closel = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
              dispose();
            }
        };
        close.addActionListener(closel);

    }
    

    /**
     * Set the information strings.
     * @param opInfo ListExpr
     */
    protected void setStrings(ListExpr opInfo) {
        opName  = opInfo.second().first().first().stringValue();
        algName = opInfo.second().first().second().stringValue();
        signat  = opInfo.second().first().third().textValue();
        syntax  = opInfo.second().first().fourth().textValue();
        meaning = opInfo.second().first().fifth().textValue();
        example = opInfo.second().first().sixth().textValue();
        result  = opInfo.second().first().seventh().textValue();
        remark  = opInfo.second().first().eighth().textValue();
    }


    /**
     * Add the sum of information string to the window.
     */
    protected void addInfo() {
        String sumStr = "<html>";

            // For extension "Select Info elements"
        if (true == true) {
            opNameH = "<br><font color=black>" + opNameL +
                        "<br><font color=blue>" + opName + "</font>";
            sumStr += opNameH;
        }
        if (true == true) {
            algNameH = "<br><font color=black>" + algNameL +
                         "<br><font color=blue>" + algName + "</font>";
            sumStr += algNameH;
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
            meaningH = "<br><br><font color=black><b>" + meaningL +
                         "<br><font color=blue>" + meaning + "</b><br></font>";
            sumStr += meaningH;
        }
        if (true == true) {
            exampleH = "<br><font color=black>" + exampleL +
                         "<br><font color=blue>" + example + "</font>";
            sumStr += exampleH;
        }
        if (true == true) {
            resultH = "<br><font color=black>" + resultL +
                        "<br><font color=blue>" + result + "</font>";
            sumStr += resultH;
        }
        if (true == true) {
            remarkH = "<br><font color=black>" + remarkL +
                        "<br><font color=blue>" + remark + "</font>";
            sumStr += remarkH;
        }

        sumStr += "</html>";
        opText.setText(sumStr);

        /* For extension "Select Info elements"
        if (opText.getText() == null) {
            opText.setText("No Information selected.");
        }
        */
    }
    
}
