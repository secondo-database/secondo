/*

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"{u}]
//[ae] [\"{a}]
//[TOC] [\tableofcontents]

[1] PictureViewable: Class Definitions

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
import gui.*;

import java.lang.*;



/*

3 Class DisplayPicture

Superclass for all PictureViewables

	Displays one more graph, histogram, picture, ...
	means add one histogram.
	Pictures and Histograms are headerless
	for Example: Histogram: ( check consistens )
	if (LE.listLength() == 3){
		int colorchannel = LE.first().intValue();
		double maxvalue = (LE.second().realValue());
		if (LE.third().listLength()==256 )
		...
	}

*/

abstract public  class PictureViewable
extends JPanel
{


/*

3.1 Function ~set~

The function ~set~ is the main function of this class. It is responsible for
the interpretation of the specified parameter ~LE~ which contains all
data of the object. Then it stores the object for painting.

*/

    abstract public  void set( ListExpr LE );


/*

3.2 Function ~removeall~

This function removes all visible objects.
Nothing will be painted anymore.

*/

     abstract public void removeall();


/*

3.3 Function ~paint~

The function ~paint~ displays the objects.

*/

    abstract public void paint ( Graphics g );


/*

3.4 Function ~init~

The method ~init()~ initializes all, which is not initialized by the Constructor.

*/

    abstract public void init();

}




