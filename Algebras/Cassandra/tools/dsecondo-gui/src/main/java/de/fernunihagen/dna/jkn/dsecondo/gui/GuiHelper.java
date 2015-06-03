package de.fernunihagen.dna.jkn.dsecondo.gui;

import java.awt.Dimension;
import java.awt.Frame;

public class GuiHelper {
	
	/**
     * Places the frame on the screen center.
     */
    public static void locateOnScreen(Frame frame) {
        Dimension paneSize   = frame.getSize();
        Dimension screenSize = frame.getToolkit().getScreenSize();
        frame.setLocation(
            (screenSize.width  - paneSize.width)  / 2,
            (screenSize.height - paneSize.height) / 2);
    }
}
