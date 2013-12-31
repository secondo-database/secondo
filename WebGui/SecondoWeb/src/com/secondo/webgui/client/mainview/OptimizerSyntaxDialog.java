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

package com.secondo.webgui.client.mainview;

import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.DialogBox;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.ScrollPanel;

/**
*  This class contains the dialog for the optimizer syntax in the help menu.
*  
*  @author Kristina Steiger
*  
**/
public class OptimizerSyntaxDialog {
	
	private DialogBox helpDialogBox = new DialogBox();
    private FlowPanel dialogContents = new FlowPanel();
    private ScrollPanel scrollContent = new ScrollPanel();
	private HTML optimizerSyntax;
	private Button closeButton = new Button("Close");
	
	public OptimizerSyntaxDialog(){
		
		helpDialogBox.setText("Info about Optimizer Syntax");

	    // Create a table to layout the content
	    dialogContents.getElement().getStyle().setPadding(5, Unit.PX);
	    helpDialogBox.setWidget(dialogContents);
		
		optimizerSyntax = new HTML("<h3>Information about the Optimizer</h3>" +
				"The optimizer component of SECONDO is written in PROLOG and allows one to formulate SECONDO commands as well as queries in an SQL-like " +
				"language within a PROLOG environment. Commands are passed directly to the SECONDO kernel for execution. Queries are translated to query " +
				"plans which are then also sent to the kernel for exection. One can also experiment with the optimizer and just see how queries are translated " +
				"without executing them.<p>" +
				"<h3>An SQL-like Query Language</h3>"+
				"The optimizer implements a part of an SQL-like language by a predicate sql, to be written in prefix notation, and provides some operator " +
				"definitions and priorities, e.g. for select, from, where, that allow us to write an SQL query directly as a PROLOG term. For example, one can " +
				"write (assuming database opt is open): sql select * from staedte where bev > 500000. <p>" +
				"Note that in this environment all relation names and attribute names are written in lower case letters only. Remember that words starting " +
				"with a capital are variables in PROLOG; therefore we cannot use such words. The optimizer on its own gets information from the SECONDO kernel " +
				"about the spellings of relation and attribute names and sends query plans to SECONDO with the correct spelling.<p>" +
				"Some messages appear that tell you something about the inner workings of the optimizer. Possibly the optimizer sends by itself some small " +
				"queries to SECONDO, then it says: <p>" +
				"Destination node 1 reached at iteration 1 <br>" +
				"Height of search tree for boundary is 0 <p>" +
				"The best plan is:<p>" +
				"Staedte feed filter[.Bev > 500000] {0.20669, 3.27586} consume<br/>" +
				"Estimated Cost: 310.64 <p>" +
				"After that appear evaluation messages and the result of the query. If you are interested in understanding how the optimizer works, please read " +
				"the paper: Gueting, R.H., T. Behr, V.T. de Almeida, Z. Ding, F. Hoffmann, and M. Spiekermann, SECONDO: An Extensible DBMS Architecture and Prototype. Fernuniversitaet Hagen, Informatik-Report 313, 2004. " +
				"If you wish to understand the working of the optimizer in more detail, you can also read the source code documentation, " +
				"that is, say in the directory Optimizer:<p>" +
				" pdview optimizer.pl<p>" +
				"Almost all prolog sourcefiles (having filename extension .pl) from the Optimizer directory can be processed in this way. In the following, " +
				"we describe the currently implemented query language in detail. Whereas the syntax resembles SQL, no attempt is made to be consistent with any " +
				"particular SQL standard.<p>" +
				"<h3>Basic Queries</h3><p>" +
				"The SQL kernel implemented by the optimizer basically has the following syntax:<p>" +
				"select (attr-list)<br>" +
				"from (rel-list)<br>" +
				"where (pred-list)<p>" +
				"Each of the lists has to be written in PROLOG syntax (i.e., in square brackets, entries separated by comma). If any of the lists has only a " +
				"single element, the square brackets can be omitted. Instead of an attribute list one can also write *. Hence one can write (dont forget to " +
				"type sql before all such queries and end them with a .):<p>" +
				"select [sname, bev]<br>" +
				"from staedte<br>" +
				"where [bev > 270000, sname starts S]<p>" +
				"To avoid name conflicts, one can introduce explicit variables. In this case one refers to attributes in the form <variable>:<attr>. " +
				"For example, one can perform a join between relations Orte and plz:<p>" +
				"select * from [orte as o, plz as p]<br>" +
				"where [o:ort = p:ort, o:ort contains dorf, (p:plz mod 13) = 0]<p>" +
				"In the sequel, we define the syntax precisely by giving a grammar. For the basic queries described so far we have the following grammar rules:<p>" +
				"query -> select distinct-clause sel-clause<br>" +
				"from rel-clause<br>" +
				"where-clause<br>" +
				"distinct-clause -> all | distinct | Epsilon<br>" +
				"sel-clause -> *<br>" +
				"| result | [result-list]<br>" +
				"| count(distinct-clause *)<br>" +
				"result -> attr | attr-expr as newname<br>" +
				"result-list -> result | result, result-list<br>" +
				"attr -> attrname | var:attrname<br>" +
				"attr-list -> attr | attr, attr-list<br>" +
				"attrname -> id<br>" +
				"rel -> relname | relname as var<br>" +
				"rel-clause -> rel | [rel-list]<br>" +
				"rel-list -> rel | rel, rel-list<br>" +
				"relname -> id<br>" +
				"var -> id<br>" +
				"where-clause -> where [pred-list] | where pred | Epsilon<br>" +
				"pred -> attr-boolexpr<br>" +
				"pred-list -> pred | pred, pred-list<p>" +
				"We use the following notational conventions. Words written in normal font are grammar symbols (non-terminals), words in bold face are terminal" +
				" symbols. The symbols -> and | are meta-symbols denoting derivation in the grammar and separation of alternatives. Epsilon denotes the empty" +
				" word. Other characters like * or : are also terminals. id is any valid SECONDO identifier (spelled in lower case letters).<p>" +
				"The notation X-list refers to a non-empty PROLOG list with elements of type X; as mentioned already, the square brackets can be omitted" +
				" if the list has just one element.<p>" +
				"The notation X-expr refers to an expression built from elements of type X, constants, and operations available on X-values. Hence attr-expr " +
				"is an expression involving attributes denoted in one of the two forms attrname or var:attrname. Similarly a predicate (pred) is a boolean " +
				"expression over attributes (attr-boolexpr).<p>" +
				"X-constant denotes a SECONDO constant of type X.<p>" +
				"Finally, Epsilon denotes the empty alternative. Hence the where-clause or the distinct keyword are optional.<p>" +
				"From the grammar, one can see that it is also possible to compute derived attributes in the select-clause. For example:<p>" +
				"select [sname, bev div 1000 as bevt] from staedte");
		
		// Add the text to the dialog
	    optimizerSyntax.setSize("490px", "490px");
	    scrollContent.setSize("510px", "510px");
	    scrollContent.add(optimizerSyntax);
	    dialogContents.add(scrollContent);

	    // Add a close button at the bottom of the dialog
	    closeButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	            helpDialogBox.hide();}
	        });
	    dialogContents.add(closeButton);
	}
	
	/**Returns the dialog box containing the optimizer info text
	 * 
	 * @return The dialog box containing the optimizer info text
	 * */
	public DialogBox getHelpDialogBox() {
		return helpDialogBox;
	}
}
