/*

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"{u}]
//[ae] [\"{a}]
//[TOC] [\tableofcontents]

[1] Display Picture: Class Definitions

Dezember 2004 Christian Bohnebuck, Uwe Hartmann, Marion Langen and Holger
M[ue]nx during Prof. G[ue]ting's practical course
'Extensible Database Systems' at Fernuniversit[ae]t Hagen.

[TOC]

1 Introduction

See the documentation of ~PictureAlgebra.h~ for a general introduction to
the Picture algebra.

This module is part of a SECONDO viewer ~PictureViewer~ which shows ~picture~ objects and their ~histograms~.


2 Includes and other preparations

*/

package viewer.pictureviewer;

import sj.lang.*;
import javax.swing.*;
import javax.swing.table.*;
import java.awt.*;
import java.util.*;
import gui.*;
import java.awt.event.*;
import javax.swing.event.*;
import tools.*;
import java.io.*;
import java.awt.image.*;

import java.lang.*;






/*

3 Class DisplayPicture

*/

public class DisplayPicture
extends PictureViewable
{
	Image 	  image;
	Dimension dim00;
	Dimension dim;

    	private String[] zoomNames =
      	{ 	"Fit Window",
		"Zoom 25%",
		"Zoom 50%",
		"Zoom 100%",
		"Zoom 200%",
		"Zoom 400%" };
    	private int zoom = 0;


/*

3.1 Constructor ~DisplayPicture~

This method initialize several dimension variables. Finally, all visible objects are removed.

*/

    	public DisplayPicture()
    	{
		dim00 = new Dimension( 0, 0);
		dim = new Dimension( 0, 0);
		removeall();
    	}


/*

3.2 Function ~getZoomNames~

This method return all possible names of zoom.

*/

    	public String[] getZoomNames() {
		return zoomNames;
    	}


/*

3.3 Function ~init~

no init

*/

    	public void init() {
    	}


/*

3.4 Function ~setZoom~

This method sets the factor of zoom.

*/

    	public void setZoom(int z) {
		zoom = z;
    	}


/*

3.5 Function ~set~

The function ~set~ is the main function of this class. It is responsible for
the interpretation of the specified parameter ~LE~ which contains all
data of the picture. Then it decodes from Base64 to jpeg and sets factor of zoom.
Moreover, it sets the dimension of the graphic.

*/

    	public  void set( ListExpr LE, double panelWidth, double panelHeight )
    	{
	    System.err.println("DisplayPicture::set() called");

		removeall();

		if (LE==null)
			return;
		if (LE.listLength() != 5)
			return;

		String base64code = LE.fifth().textValue();

		byte buf[] = PictureIcon.getbase64fast( base64code);
		image = Toolkit.getDefaultToolkit().createImage( buf );
		ImageIcon icon = new ImageIcon(image);
		int wi = icon.getIconWidth();
		int hi = icon.getIconHeight();

		boolean scale = false;

		if (zoom == 0 && panelWidth > 0 && panelHeight > 0) {
		    System.err.println("Fit Window called");
		    double widthf = wi/(panelWidth-10);
		    double heightf = hi/(panelHeight-10);
		    System.err.println("widthf="+widthf);
		    System.err.println("height="+heightf);
		    if (widthf > heightf && widthf > 1) {
			wi /= widthf;
			hi /= widthf;
			scale = true;
		    } else if (heightf > widthf && heightf > 1) {
			wi /= heightf;
			hi /= heightf;
			scale = true;
		    }
		} else if (zoom == 1) {
		    wi /= 4;
		    hi /= 4;
		    scale = true;
		} else if (zoom == 2) {
		    wi /= 2;
		    hi /= 2;
		    scale = true;
		} else if (zoom == 4) {
		    wi *= 2;
		    hi *= 2;
		    scale = true;
		} else if (zoom == 5) {
		    wi *= 4;
		    hi *= 4;
		    scale = true;
		}

		if (wi < 1) wi = 1;
		if (hi < 1) hi = 1;

		if (scale)
		    image =
			image.getScaledInstance(wi, hi, Image.SCALE_DEFAULT);

		dim.setSize(wi,hi);
		setMinimumSize( dim );
		setPreferredSize( dim );

		repaint();

	    System.err.println("DisplayPicture::set() done");
	}


/*

3.5 Function ~set~

The function ~set~ is the main function of this class. It is responsible for
the interpretation of the specified parameter ~LE~ which contains all
data of the picture. Then it decodes from Base64 to jpeg and sets factor of zoom.
Moreover, it sets the dimension of the graphic.

*/

    	public void set(ListExpr LE) {
		set(LE, -1.0, -1.0);
    	}


/*

3.5 Function ~removeall~

This function removes all visible objects.

*/


	public void removeall()
	{
		setOpaque(false);
		image=null;
		setMinimumSize( dim00 );
		setPreferredSize( dim00 );

		repaint();
	}


/*

3.6 Function ~paint~

The function ~paint~ displays the ~image~.

*/

	public void paint (  Graphics g)
	{
	    if (  image == null)
	    	return;
	    g.drawImage(image,0,0,this);
	}



}




