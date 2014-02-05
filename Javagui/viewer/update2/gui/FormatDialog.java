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

package  viewer.update2.gui;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.event.ActionListener;
import java.awt.GridLayout;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.List;

import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.SwingConstants;

import tools.Reporter;

import viewer.update2.*;


/**
 * Dialog to format the current document.	 
 */
public class FormatDialog extends JDialog
{
	private DocumentPanel formattedDocument;
	private JButton btFormat;
	private JButton btClose;
	private JScrollPane scpDocument;
	private JCheckBox chkSepPages;
	private JLabel lbPosInfo;
	private UpdateViewerController controller;

	/** Constructor */
	public FormatDialog(UpdateViewerController pController)
	{
		this.controller = pController;

		this.setDefaultCloseOperation(HIDE_ON_CLOSE);
		this.setModal(false);
		this.setSize(600,400);
		this.setTitle("Format document");		
		
		// buttons
		JPanel plButtons = new JPanel();
		this.chkSepPages = new JCheckBox("separate pages");
		plButtons.add(this.chkSepPages);
		this.btFormat = new JButton(UpdateViewerController.CMD_FORMAT);
		this.btFormat.addActionListener(pController);
		this.btFormat.setToolTipText("Write formatted document to files and show");
		plButtons.add(this.btFormat);
		this.btClose = new JButton("Close");
		this.btClose.addActionListener(pController);
		this.btClose.setActionCommand(UpdateViewerController.CMD_CLOSE_FORMAT_DIALOG);
		this.btClose.setToolTipText("Close format dialog");
		plButtons.add(this.btClose);
		
		JPanel plSouth = new JPanel(new GridLayout(2,1));
		plSouth.add(plButtons);
		this.lbPosInfo = new JLabel();
		this.lbPosInfo.setHorizontalAlignment(SwingConstants.CENTER);
		plSouth.add(lbPosInfo);
		
		// scrollpane
		JPanel plCenter = new JPanel();
		plCenter.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));
		this.scpDocument = new JScrollPane();
		plCenter.add(this.scpDocument);

		// content pane
		this.getContentPane().setLayout(new BorderLayout());
		this.getContentPane().add(scpDocument, BorderLayout.CENTER);
		this.getContentPane().add(plSouth, BorderLayout.SOUTH);
	}
		
	
	/**
	 * Returns position information.
	 */
	public RelationPosition getCurrentPosition()
	{
		return this.formattedDocument.getCurrentRelationPosition();
	}
	
	/**
	 * Returns TRUE if checkbox 'separate pages' is checked.
	 */
	public boolean getSeparatePages()
	{
		return this.chkSepPages.isSelected();
	}
	
	/**
	 * Shows the Panel with the formatted document in it.
	 */
	public void setFormattedDocument(DocumentPanel pDisplayComponent)
	{
		this.formattedDocument = pDisplayComponent;
		this.formattedDocument.addMouseListener(this.controller);
		this.scpDocument.setViewportView(this.formattedDocument);
		this.scpDocument.getVerticalScrollBar().setValue(0);
		this.scpDocument.getViewport().setViewPosition(new java.awt.Point(0,0));
	}
	
	public void setPositionInfo(RelationPosition pPositionInfo)
	{
		if (pPositionInfo == null)
		{
			this.lbPosInfo.setText("Position not in relation content.");
		}
		else
		{
			StringBuffer sb = new StringBuffer("Cursor position: ");
			sb.append("relation \"").append(pPositionInfo.getRelationName());
			sb.append("\", tuple \"").append(pPositionInfo.getTupleId());
			sb.append("\", attribute \"").append(pPositionInfo.getAttributeName());
			sb.append("\", offset ").append(pPositionInfo.getOffset());
			this.lbPosInfo.setText(sb.toString()); 
		}
	}
}

