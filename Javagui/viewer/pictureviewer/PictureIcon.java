/*

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"{u}]
//[ae] [\"{a}]
//[oe] [\"{o}]
//[TOC] [\tableofcontents]

[1] PictureIcon: Class Definitions

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

import viewer.pictureviewer.*;
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
import tools.Reporter;






/*

3 Class PictureIcon

This class represents a single picture object by providing its picture icon
or its histogram icons, depending on which constructor has been called. Methods
are availabe to access the original list representation of the picture object
and to provide a meaningful string with the picture's meta data.

*/

public class PictureIcon
extends ImageIcon
{

/*

3.1 private Attributes

*/

    private ListExpr	listXr;
    private double[]	histo;
    private byte[]	buf;
    private String metaData;
    private int width = -1;
    private int height = -1;

    static private int	widthIcon=40;
    static private int	heightIcon=40;
    static private Image[]	histoIcon;



/*

3.1 Constructor ~PictureViewer~

The Standard-constructor with no data.
It might be used as dummy.

*/

	public PictureIcon()
	{
	    buf = null;
	    listXr = null;
	    histo = null;
	    metaData = "";
	}

/*

3.1 Constructor ~PictureViewer~

It creates an icon for a picture with data of the picture.

*/

	public PictureIcon( String base64code, ListExpr LE)
	{
		listXr = LE;
		histo = null;

		buf = getbase64fast( base64code );

		processJpegMetaData(buf);

		Image ima = Toolkit.getDefaultToolkit().createImage( buf );

		if (width == -1 || height == -1) {
		    ImageIcon icon = new ImageIcon(ima);
		    width = icon.getIconWidth();
		    height = icon.getIconHeight();
		}

		float widthFactor = ((float) width)/widthIcon;
		float heightFactor = ((float) height)/heightIcon;
		float factor =
		    widthFactor > heightFactor ? widthFactor : heightFactor;

		int scaledWidth = (int) (width/factor);
		int scaledHeight = (int) (height/factor);
		if (scaledWidth < 1) scaledWidth = 1;
		if (scaledHeight < 1) scaledHeight = 1;

		setImage(
		    ima.getScaledInstance(
			scaledWidth, scaledHeight,
//			Image.SCALE_FAST));
			Image.SCALE_DEFAULT));
	}



/*

3.1 Constructor ~PictureViewer~

It creates an icon for a histogram with data of the histogram.

*/

	public PictureIcon(double[] hist, int col, ListExpr LE)
	{

		setImage(createHistoIcon(hist, col, widthIcon, heightIcon ));
		buf = null;
		listXr = LE;
		histo = hist;

		metaData = "";

	}


/*

3.1 Constants used in ~jpeg-metadata~

Info in libjpeg.

*/

	static private byte M_SOI 	= (byte)0xd8;	// start of jpeg
	static private byte M_EOI 	= (byte)0xd9;	// end of jpeg
	static private byte M_SOS 	= (byte)0xda;	// start of compressed
	static private byte M_COM 	= (byte)0xfe;	// COMent
	static private byte M_FF  	= (byte)0xff;	// ff
	static private byte M_SOF0	= (byte)0xc0;	// start of frame N
	static private byte M_SOF15 	= (byte)0xcf;	// last SOF

	static private String[] jpeg_processtype= {
		"Baseline", "Extended sequential",
		"Progressive", "Lossles",
		"Unknown", "Differential sequential",
		"Differential progressive", "Differential lossles",
		"Unknown", "Extended sequential, arithmetic coding",
		"Progressive, arithmetic coding", "Lossles, arithmetic coding",
		"Unknown", "Differential sequential, arithmetic coding",
		"Differential progressive, arithmetic coding",
		"Differential lossles, arithmetic coding"
	};


/*

3.1 Function ~processJpegMetaData~

This methode searches for metadata in the byterepresentation for jpeg.

*/

	private void processJpegMetaData(byte[] buf)
	{
	    int len = 1;
            if(gui.Environment.DEBUG_MODE)
  	       Reporter.debug("PictureIcon jpeg meta M_COM="
	  		           + M_COM
			           + " M_SOS="
			           + M_SOS
			           + " (M_COM==0xfe)="
			           + ( M_COM==0xfe));



	    metaData =
		"Filename:         "
		+listXr.first().stringValue()
		+"\nDate:             "
		+listXr.second().stringValue()
		+"\nCategory:         "
		+listXr.third().stringValue()
		+"\nPortrait:         "
		+listXr.fourth().writeListExprToString().trim();

	    try  {
		if (buf[0]!= 0xff && buf[1]!=M_SOI)
			return; // first marker = no jpeg
		for (int i=2; ; )
		{
			// next marker
			while ( buf[i] != M_FF )
				i++;	// skip junk
			while ( buf[i] == M_FF )
				i++;	// next marker
			int marker = buf[i++];
			if (marker == M_SOS || marker==M_EOI) return;
			len = ((0xff & (int)(buf[i]))<<8)
				+ (0xff & (int)(buf[i+1]));
			if (len<2) return;
			if (marker == M_COM)
			{ // process metastring
      			  	   Reporter.debug(
				  	"    PictureIcon jpeg meta M_COM len="
					+ len + " index=" + i);
				metaData += "\n ";
				for ( i +=2; len>2; len--){
					char c = (char)buf[i++];
					if (c<' ' || c>'z')
						c = '.';	// no junk-output
					metaData += c;
				}
			}
			else if (marker>=M_SOF0 && marker<=M_SOF15)
			{ // SOF marker
      				   Reporter.debug(" PictureIcon jpeg meta M_SOF len="
				  	              + len + " index=" + i);
				int jpeg_precision 	= buf[i+2];
				int jpeg_height
					=((0xff & (int)(buf[i+3]))<<8)
						+(0xff & (int)(buf[i+4]));
				int jpeg_width
					=((0xff & (int)(buf[i+5]))<<8)
						+(0xff & (int)(buf[i+6]));
				int jpeg_components	= buf[i+7];
				if ( ((jpeg_components*3)+8)==len){
					// if meaningful
				    metaData +=
					"\nJPEG width:       "+jpeg_width
					+"\nJPEG height:      "+jpeg_height
					+"\nJPEG components:  "+jpeg_components
					+"\nJPEG precision:   "
					+jpeg_precision
					+" bits per sample"
					+"\nJPEG process:     "
					+jpeg_processtype[marker & 0xf];
				    width = jpeg_width;
				    height = jpeg_height;
				}
				i += len;
			} else {
				i += len;	// skip
			}

		}
	    } catch (IndexOutOfBoundsException e) {
		// ready
      		    Reporter.debug(" PictureIcon jpeg meta finish" );
	    }
	}


/*

3.1 Constants used in ~base64decoder~

*/

	static private int[] b64deco =
	{
	-1, -1, -1, -1, -1, -1, -1, -1,  -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,  -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,  -1, -1, -1, 62, -1, -1, -1, 63,
	52, 53, 54, 55, 56, 57, 58, 59,  60, 61, -1, -1, -1,  0, -1, -1,
	-1,  0,  1,  2,  3,  4,  5,  6,   7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22,  23, 24, 25, -1, -1, -1, -1, -1,
	-1, 26, 27, 28, 29, 30, 31, 32,  33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48,  49, 50, 51, -1, -1, -1, -1, -1,
	};



/*

3.1 Function ~getbase64fast~

This methode creates the byterepresentation for jpeg of base64 code.

*/

   	static public byte[] getbase64fast( String datap )
   	{
		int j=0;
		int n=0;
		datap += "====";
		byte[] bin = datap.getBytes();
		byte[] bufo = new byte[ ( 3*bin.length)/4 +4 ];
     		try {
			for ( int i=0; ; n=0)
			{
				while (b64deco[bin[i] & 0x7f]<0)
					i++;
				n += (b64deco[bin[i++]& 0x7f] << 18);
				while (b64deco[bin[i]& 0x7f]<0)
					i++;
				n += (b64deco[bin[i++]& 0x7f] << 12);
				while (b64deco[bin[i]& 0x7f]<0)
					i++;
				n += (b64deco[bin[i++]& 0x7f]<<6);
				while (b64deco[bin[i]& 0x7f]<0)
					i++;
				n += b64deco[bin[i++]& 0x7f];
				bufo[j++] = (byte)((n>>16) & 0xff);
				bufo[j++] = (byte)((n>>8) & 0xff);
				bufo[j++] = (byte)((n) & 0xff);
			}
      		} catch (IndexOutOfBoundsException  e ) {
           		   Reporter.debug(" PictureIcon create picture bound end " );
      		}
		return bufo;
   	}



/*

3.1 Function ~getListExpr~

This methode returns the ListExpr represented by this icon.

*/

	public ListExpr getListExpr()
	{
		return listXr;
	}



/*

3.1 Function ~getMetaData~

If available, this methode returns formated metadata represented by this icon.

*/

	public String getMetaData()
	{
	    return metaData;
	}



/*

3.1 Function ~createHistoIcon~

This methode creates an icon with histogram in it.

*/

	private Image createHistoIcon( double[] curve,
				       int color,
				       int w,
				       int h )
	{
		int len = w * h;
        	int pix[] = new int[len];
		int opac = 0xff << 24;

		if (color<3 && color>=0)
			color = ( 0xff0000 >> (color<<3)) | opac;
		else
			color = ( 0xffffff) | opac;

		for (int x=0; x<len; x++) pix[x] = 0x000000 | opac;

		for (int i = 0; i < 256; i++)
		{
		    int u = h-1-(int)(curve[i]*(h-1));
		    try {
			pix[u*w+i*w/256] = color;
		    } catch (ArrayIndexOutOfBoundsException e) {
		    }
        	}
	        return
		    Toolkit.getDefaultToolkit().createImage(
			new MemoryImageSource(w, h, pix, 0, w));

	}



}




