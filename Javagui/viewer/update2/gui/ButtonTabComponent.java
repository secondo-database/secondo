/*
 * Copyright (c) 1995, 2008, Oracle and/or its affiliates. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *   - Neither the name of Oracle or the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */ 

package viewer.update2.gui;

import javax.swing.*;
import javax.swing.*;
import javax.swing.plaf.basic.BasicButtonUI;
import java.awt.*;
import java.awt.event.*;

import javax.accessibility.AccessibleValue;

import viewer.update2.UpdateViewerController;

/**
 * Component to be used as tabComponent;
 * Contains a JLabel to show the text and 
 * a JButton to close the tab it belongs to 
 */ 
public class ButtonTabComponent extends JPanel 
{
    public ButtonTabComponent(int pIndex, String pTitle, ActionListener pActionListener) 
	{
        //unset default FlowLayout' gaps
        super(new FlowLayout(FlowLayout.LEFT, 0, 0));
        setOpaque(false);
        
        //title label
        JLabel label = new JLabel(pTitle);        
        add(label);
        label.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 5));

        //close button
        TabButton button = new TabButton(pActionListener);
		button.setCurrentAccessibleValue(pIndex);
        add(button);
    }
	

	/**
	 * Close button in right corner of tab
	 */
    private class TabButton extends JButton 
		implements AccessibleValue
	{
		private int index = -1;
		
        public TabButton(ActionListener pActionListener) {
            int size = 20;
            setPreferredSize(new Dimension(size, size));
            setActionCommand(UpdateViewerController.CMD_CLOSE_TAB);
            setToolTipText("Close tab");
            //Make the button looks the same for all Laf's
            setUI(new BasicButtonUI());
            //Make it transparent
            setContentAreaFilled(false);
            //No need to be focusable
            setFocusable(false);
            setBorder(BorderFactory.createEtchedBorder());
            setBorderPainted(false);
            //Making nice rollover effect
            //we use the same listener for all buttons
            addMouseListener(buttonMouseListener);
            setRolloverEnabled(true);
            //Close the proper tab by clicking the button
            addActionListener(pActionListener);
        }
		
		public Integer getCurrentAccessibleValue(){return index;}
		public Integer getMaximumAccessibleValue(){return index;}
		public Integer getMinimumAccessibleValue(){return index;}
		public boolean setCurrentAccessibleValue(Number i)
		{
			if (i instanceof Integer)
			{
				this.index = i.intValue();
				return true;
			}
			return false;
		}
		
		
        //we don't want to update UI for this button
        public void updateUI() {
        }
		
        //paint the cross
        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            Graphics2D g2 = (Graphics2D) g.create();
            //shift the image for pressed buttons
            if (getModel().isPressed()) {
                g2.translate(1, 1);
            }
            g2.setStroke(new BasicStroke(2));
            g2.setColor(Color.BLACK);
            if (getModel().isRollover()) {
                g2.setColor(Color.RED);
            }
            int delta = 6;
            g2.drawLine(delta, delta, getWidth() - delta - 1, getHeight() - delta - 1);
            g2.drawLine(getWidth() - delta - 1, delta, delta, getHeight() - delta - 1);
            g2.dispose();
        }
    }
	
    private final static MouseListener buttonMouseListener = new MouseAdapter() {
	public void mouseEntered(MouseEvent e) {
	Component component = e.getComponent();
	if (component instanceof AbstractButton) {
		AbstractButton button = (AbstractButton) component;
		button.setBorderPainted(true);
	}
	}

public void mouseExited(MouseEvent e) {
Component component = e.getComponent();
if (component instanceof AbstractButton) {
AbstractButton button = (AbstractButton) component;
button.setBorderPainted(false);
}
}
};
}


