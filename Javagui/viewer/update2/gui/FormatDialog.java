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
import java.awt.Point;
import java.awt.Rectangle;
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
import javax.swing.SwingUtilities;

import tools.Reporter;

import viewer.update2.*;


/**
 * Dialog to format the current document.	 
 */
public class FormatDialog extends JDialog
{
	private JButton btFormat;
	private JButton btClose;
	private JCheckBox chkSepPages;
	private JCheckBox chkApplyScript;
	private JLabel lbPosInfo;
	private JScrollPane scpDocument;
	private UpdateViewerController controller;
	private DocumentPanel formattedDocument;
	private RelationPosition currentPosition;

	/** Constructor */
	public FormatDialog(UpdateViewerController pController)
	{
		this.controller = pController;

		this.setDefaultCloseOperation(HIDE_ON_CLOSE);
		this.setModal(false);
		this.setSize(600,400);
		this.setTitle("Format document");		
		
		this.chkSepPages = new JCheckBox("separate pages");
		this.chkSepPages.setToolTipText("separate pages");
		this.chkApplyScript = new JCheckBox("apply script");
		this.chkApplyScript.setToolTipText("apply script on output files");
		this.lbPosInfo = new JLabel();
		this.lbPosInfo.setHorizontalAlignment(SwingConstants.CENTER);		
		this.scpDocument = new JScrollPane();
		
		// buttons
		this.btFormat = new JButton(UpdateViewerController.CMD_FORMAT);
		this.btFormat.addActionListener(pController);
		this.btFormat.setToolTipText("Generate and show formatted document");
		this.btClose = new JButton("Close");
		this.btClose.addActionListener(pController);
		this.btClose.setActionCommand(UpdateViewerController.CMD_CLOSE_FORMAT_DIALOG);
		this.btClose.setToolTipText("Close format dialog");
		
		// arrange components
		JPanel plButtons = new JPanel();
		plButtons.add(this.chkSepPages);
		plButtons.add(this.chkApplyScript);
		plButtons.add(this.btFormat);
		plButtons.add(this.btClose);

		JPanel plSouth = new JPanel(new GridLayout(2,1));
		plSouth.add(plButtons);
		plSouth.add(lbPosInfo);

		JPanel plCenter = new JPanel();
		plCenter.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));
		plCenter.add(this.scpDocument);

		this.getContentPane().setLayout(new BorderLayout());
		this.getContentPane().add(scpDocument, BorderLayout.CENTER);
		this.getContentPane().add(plSouth, BorderLayout.SOUTH);
	}
	
	/**
	 * Returns TRUE if checkbox 'apply script' is checked.
	 */
	public boolean getApplyScript()
	{
		return this.chkApplyScript.isSelected();
	}
	
	/**
	 * Returns position information.
	 */
	public RelationPosition getCurrentPosition()
	{
		if (this.formattedDocument==null)
		{
			return null;
		}
		return this.formattedDocument.getCurrentRelationPosition();
	}
	
	/**
	 * Returns TRUE if checkbox 'separate pages' is checked.
	 */
	public boolean getSeparatePages()
	{
		return this.chkSepPages.isSelected();
	}
	
	public void goTo(RelationPosition pPositionInfo)
	{
		if (this.formattedDocument != null) return;
		
		this.currentPosition = pPositionInfo;
		
		if (pPositionInfo==null)
		{
			SwingUtilities.invokeLater(new Runnable() {
									   public void run() {
											scpDocument.getViewport().setViewPosition(new Point(0,0));
											scpDocument.getViewport().scrollRectToVisible(new Rectangle(0,0,0,0));
											scpDocument.getVerticalScrollBar().getModel().setValue(0);
									   }
									   });
		}
		else
		{
			SwingUtilities.invokeLater(new Runnable() {
									   public void run() {
											//formattedDocument.getRectangle(currentPosition);
											scpDocument.getViewport().scrollRectToVisible(new Rectangle(0,0,0,0));
									   }
									   });
		}
		repaint();
	}
	
	/**
	 * Returns position information.
	 */
	public void setCurrentPosition(RelationPosition pPositionInfo)
	{
		this.currentPosition = pPositionInfo;
	}
	
	/**
	 * Shows the Panel with the formatted document in it.
	 */
	public void setFormattedDocument(DocumentPanel pDisplayComponent)
	{
		this.formattedDocument = pDisplayComponent;

		if (pDisplayComponent!=null)
		{
			this.formattedDocument.addMouseListener(this.controller);
			this.scpDocument.setViewportView(this.formattedDocument);
			this.goTo(this.currentPosition);
		}
	}
	
	public void showPositionInfo()
	{
		if (this.currentPosition == null)
		{
			this.lbPosInfo.setText("Position not in relation content.");
		}
		else
		{
			StringBuffer sb = new StringBuffer("Cursor position: ");
			sb.append("relation \"").append(this.currentPosition.getRelationName());
			sb.append("\", tuple \"").append(this.currentPosition.getTupleId());
			sb.append("\", attribute \"").append(this.currentPosition.getAttributeName());
			sb.append("\", offset ").append(this.currentPosition.getOffset());
			this.lbPosInfo.setText(sb.toString()); 
		}
	}
}

