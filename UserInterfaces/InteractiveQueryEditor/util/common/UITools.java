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

package util.common;

import java.awt.Dimension;
import java.awt.Font;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsEnvironment;
import java.awt.Insets;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.util.Enumeration;

import javax.swing.UIManager;
import javax.swing.plaf.FontUIResource;

/**
 * This class realizes several utility methods related to the positioning of UI elements.
 * @author D.Merle
 */
public class UITools {

	/**
	 * Privater Konstruktor, der den Default Konstruktor überschreibt.<br>
	 */
	private UITools() {}

	/**
	 * Ermittelt für eine übergebene Größe eines Windows/Dialogs die Koordinaten,
	 * die man benutzen muss, damit das Fenster mittig dargestellt wird.
	 * @param dimension Die Abmessung des zu zentrierenden Dialogs
	 * @return {@link Point} Die ermittelten Koordinaten zur Zentrierung
	 */
	public static Point calculateCenterPosition(final Dimension dimension) {
		final Rectangle screensize = getEffectiveScreenSize();
		return new Point((int)(screensize.getWidth()-dimension.getWidth())/2, (int)(screensize.getHeight()-dimension.getHeight())/2);
	}

	public static Dimension calculateDimentsion(final int percentage) {
		if (percentage > 100 || percentage < 0) {
			throw new IllegalArgumentException("Please specify a vlaue between 0 and 100");
		}
		final Rectangle screensize = getEffectiveScreenSize();
		final int width = (int)(screensize.getWidth()*percentage/100);
		final int height = (int)(screensize.getHeight()*percentage/100);
		return new Dimension(width, height);
	}

	/**
	 * Die Methode ermittelt über die Klasse {@link GraphicsConfiguration} die zur Verfügung<br>
	 * stehende Bildschirmgröße in Pixeln. Dabei werden beispielsweise Toolbars des OS berücksichtigt.
	 * @return
	 */
	private static Rectangle getEffectiveScreenSize() {
		final GraphicsConfiguration configuration = GraphicsEnvironment.getLocalGraphicsEnvironment().getDefaultScreenDevice().getDefaultConfiguration();
		final Rectangle totalScreensize = configuration.getBounds();
		final Insets insets = Toolkit.getDefaultToolkit().getScreenInsets(configuration);
		final Rectangle effectiveScreensize = new Rectangle();
		effectiveScreensize.x = totalScreensize.x + insets.left;
		effectiveScreensize.y = totalScreensize.y + insets.top;
		effectiveScreensize.height = totalScreensize.height - insets.top - insets.bottom;
		effectiveScreensize.width = totalScreensize.width - insets.left - insets.right;
		return effectiveScreensize;
	}

	public static void scaleUI() {
		final double scalingFactor= UITools.calculateUIScaling();
		final int fontSize = (int)(12 * scalingFactor);//12 is the default Java font size
		replaceAllUIFonts(new FontUIResource(null, Font.PLAIN, fontSize));
	}

	/**
	 *
	 * @return
	 */
	public static double calculateUIScaling() {
		final Toolkit toolkit = Toolkit.getDefaultToolkit();
		final Dimension resolution = toolkit.getScreenSize();
		return resolution.getHeight()/768;
	}

	/**
	 *
	 * @param font
	 */
	private static void replaceAllUIFonts (final FontUIResource font){
		final Enumeration<Object> keys = UIManager.getDefaults().keys();
		while (keys.hasMoreElements()) {
			final Object key = keys.nextElement();
			final Object value = UIManager.get(key);
			if (value != null && value instanceof FontUIResource) {
				UIManager.put (key, font);
			}
		}
	}
}